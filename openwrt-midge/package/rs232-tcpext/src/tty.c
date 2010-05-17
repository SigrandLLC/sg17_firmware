#include "sys_headers.h"
#include <sys/ioctl.h>
#include "tty.h"
#include "misc.h"


static void tty_on_exit(int unused, void *arg)
{
    (void)unused;
    tty_t *t = arg;
    tty_delete(t);
}

tty_t *tty_create(void)
{
    tty_t *t = xzmalloc(sizeof(*t));
    t->b = iobase_create();

    onexit(tty_on_exit, t);

    return t;
}

void tty_delete(tty_t *t)
{
    tty_close(t);
    iobase_delete(t->b); t->b = NULL;
    free(t);

}


static void tty_lock  (const char *devname);
static void tty_unlock(const char *devname);

void tty_open(tty_t *t, const char *devname)
{
    tty_close_no_restore_attr(t);

    tty_lock(devname);

    int fd = open(devname, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd < 0)
    {
	syslog(LOG_ERR, "Can't open device %s: %m", devname);
        tty_unlock(devname);
	fail();
    }

    long rcflags = fcntl(fd, F_GETFL);
    if (rcflags < 0)
    {
	syslog(LOG_ERR, "%s: Can't get file descriptor flags: %m", devname);
	tty_close(t);
	fail();
    }

    rcflags &= ~O_NONBLOCK;
    rcflags = fcntl(fd, F_SETFL, rcflags);
    if (rcflags < 0)
    {
	syslog(LOG_ERR, "%s: Can't set file descriptor flags: %m", devname);
	tty_close(t);
	fail();
    }

    iobase_open(t->b, devname, fd);
}

static void tty_close_(tty_t *t, int restore_attr)
{
    if (tty_fd(t) >= 0)
    {
	/* To avoid blocking on close if we have written bytes and are in
	   flow-control, we flush the output queue. */
	tcflush(tty_fd(t), TCOFLUSH);

	if (restore_attr)
	    tty_restore(t);

	char *name = xstrdup(tty_name(t));
	iobase_close(t->b);
	tty_unlock(name);
	free(name);
    }

    t->last_in_mstate_valid = 0;
}

void tty_close(tty_t *t)
{
    tty_close_(t, 1);
}

void tty_close_no_restore_attr(tty_t *t)
{
    tty_close_(t, 0);
}


static void tty_lock(const char *devname)
{
    pid_t rc = dev_lock(devname);

    if (rc < 0)
	syslog(LOG_ERR, "Error while locking %s: %m", devname);
    else if (rc > 0)
	syslog(LOG_ERR, "%s is locked by %d", devname, rc);
    else
    {
	return;
    }

    fail();
}

static void tty_unlock(const char *devname)
{
    pid_t rc = dev_unlock(devname, getpid());

    if (rc < 0)
	syslog(LOG_WARNING, "Error while unlocking %s: %m", devname);
}


static int tty_set_raw_(tty_t *t, int save_termios)
{
    struct termios old_tios;
    if (tcgetattr(tty_fd(t), &old_tios) < 0)
    {
	syslog(LOG_ERR, "Can't get attributes for device %s: %m", tty_name(t));
	return -1;
    }

    if (save_termios)
	memcpy(&t->termios, &old_tios, sizeof(old_tios));

    struct termios new_tios;
    memcpy(&new_tios, &old_tios, sizeof(new_tios));

    cfmakeraw(&new_tios);

    // done in cfmakeraw but not in an old *libc:
    new_tios.c_cc[VMIN]  = 1;
    new_tios.c_cc[VTIME] = 0;

    // not done in cfmakeraw:
    new_tios.c_iflag |=  IGNBRK;	// cleared by cfmakeraw
    new_tios.c_cflag &= ~CLOCAL;	// !Ignore modem control lines
    new_tios.c_iflag &= ~HUPCL;		// !hang up (lower modem control lines) on close


    if (tcsetattr(tty_fd(t), TCSANOW, &new_tios) < 0)
    {
	syslog(LOG_ERR, "Can't set attributes for device %s: %m", tty_name(t));
	return -1;
    }

    return 0;
}

int tty_set_raw(tty_t *t)
{
    return tty_set_raw_(t, 1);
}

int tty_set_raw_no_save_attr(tty_t *t)
{
    return tty_set_raw_(t, 0);
}

void tty_restore(tty_t *t)
{
    if (tty_fd(t) >= 0 && t->termios.c_cflag & CSIZE)
    {
	if (tcsetattr(tty_fd(t), TCSANOW, &t->termios) < 0)
	    syslog(LOG_WARNING, "Can't restore attributes for device %s: %m",
		   tty_name(t));
	else
	    memset(&t->termios, 0, sizeof(t->termios));
    }
}


static int tty_get_modem_state_(tty_t *t)
{
    int val;

    if (ioctl(tty_fd(t), TIOCMGET, &val) < 0)
    {
	syslog(LOG_ERR, "Could not get modem state for device %s: %m", tty_name(t));
	return -1;
    }

    return val;
}

int tty_get_modem_state(tty_t *t, modem_state_t *mstate)
{
    int val = tty_get_modem_state_(t);
    if (val < 0) return val;

    *mstate = 0;

    if (val   &           TIOCM_DTR)
	*mstate |=    TTY_MODEM_DTR;
    if (val   & (TIOCM_LE|TIOCM_DSR))
	*mstate |=    TTY_MODEM_DSR;
    if (val   &           TIOCM_RTS)
	*mstate |=    TTY_MODEM_RTS;
    if (val   &           TIOCM_CTS)
	*mstate |=    TTY_MODEM_CTS;
    if (val   & (TIOCM_CAR|TIOCM_CD))
	*mstate |=     TTY_MODEM_CD;
    if (val   & (TIOCM_RNG|TIOCM_RI))
	*mstate |=     TTY_MODEM_RI;

    return 0;
}

int tty_set_modem_state(tty_t *t, modem_state_t  mstate)
{
    int val = tty_get_modem_state_(t);
    if (val < 0) return val;

    val &= ~(TIOCM_DTR|TIOCM_RTS|TIOCM_CD|TIOCM_RI);

    if (mstate & TTY_MODEM_DTR)
	val    |=    TIOCM_DTR;
    if (mstate & TTY_MODEM_RTS)
	val    |=    TIOCM_RTS;
    if (mstate & TTY_MODEM_CD)
	val    |=    TIOCM_CD;
    if (mstate & TTY_MODEM_RI)
	val    |=    TIOCM_RI;

    if (ioctl(tty_fd(t), TIOCMSET, &val) < 0)
    {
	syslog(LOG_ERR, "Could not set modem state for device %s: %m", tty_name(t));
	return -1;
    }

    return 0;
}

modem_state_t tty_mstate_in_to_out(modem_state_t in_state)
{
    modem_state_t out_state = 0;

    if ( in_state &  TTY_MODEM_DSR )
	out_state |= TTY_MODEM_DTR;
    if ( in_state &  TTY_MODEM_CTS )
	out_state |= TTY_MODEM_RTS;
    out_state |= in_state & (TTY_MODEM_CD | TTY_MODEM_RI);

    return out_state;
}


void tty_log_modem_state(const char *pfx, modem_state_t mstate)
{
    char buf[256];

    snprintf(buf, sizeof(buf), "DTR:%d DSR:%d RTS:%d CTS:%d CD:%d RI:%d",
             !!(mstate & TTY_MODEM_DTR),
	     !!(mstate & TTY_MODEM_DSR),
	     !!(mstate & TTY_MODEM_RTS),
	     !!(mstate & TTY_MODEM_CTS),
	     !!(mstate & TTY_MODEM_CD ),
	     !!(mstate & TTY_MODEM_RI )
	    );

    syslog(LOG_INFO, "%s0x%02X, %s", pfx?pfx:"", mstate, buf);
}

