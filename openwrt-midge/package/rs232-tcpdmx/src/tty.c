#include "sys_headers.h"
#include <sys/ioctl.h>
#include "tty.h"
#include "misc.h"
#include <dferror.h>
#include <dflog.h>


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
    tty_close(t);

    tty_lock(devname);

    int fd = open(devname, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd < 0)
    {
	dferror(EXIT_SUCCESS, errno, "Can't open device %s", devname);
	tty_unlock(devname);
	fail();
    }

    long rcflags = fcntl(fd, F_GETFL);
    if (rcflags < 0)
    {
	dferror(EXIT_SUCCESS, errno,
		"%s: Can't get file descriptor flags", devname);
	tty_close(t);
	fail();
    }

    rcflags &= ~O_NONBLOCK;
    rcflags = fcntl(fd, F_SETFL, rcflags);
    if (rcflags < 0)
    {
	dferror(EXIT_SUCCESS, errno,
		"%s: Can't set file descriptor flags", devname);
	tty_close(t);
	fail();
    }

    iobase_open(t->b, devname, fd);
}

void tty_close(tty_t *t)
{
    if (tty_fd(t) >= 0)
    {
	/* To avoid blocking on close if we have written bytes and are in
	   flow-control, we flush the output queue. */
	tcflush(tty_fd(t), TCOFLUSH);

	tty_restore_attr(t);

	char *name = xstrdup(tty_name(t));
	iobase_close(t->b);
	tty_unlock(name);
	free(name);
    }

    t->last_in_mstate_valid = 0;
}

static void tty_lock(const char *devname)
{
    pid_t rc = dev_lock(devname);

    if (rc < 0)
	dferror(EXIT_SUCCESS, errno, "Error while locking %s", devname);
    else if (rc > 0)
	dflog(LOG_ERR, "%s is locked by %d", devname, rc);
    else
	return;

    fail();
}

static void tty_unlock(const char *devname)
{
    pid_t rc = dev_unlock(devname, getpid());

    if (rc < 0)
	dflog(LOG_WARNING, "Error while unlocking %s: %s",
	      devname, strerror(errno));
}

int tty_save_attr(tty_t *t)
{
    struct termios old_tios;

    if (tcgetattr(tty_fd(t), &old_tios) < 0)
    {
	dflog(LOG_ERR, "Can't get attributes for device %s: %s",
	      tty_name(t), strerror(errno));
	return -1;
    }

    memcpy(&t->save_attr, &old_tios, sizeof(old_tios));
    return 0;
}

int tty_restore_attr(tty_t *t)
{
    if (tty_fd(t) >= 0 && t->save_attr.c_cflag & CSIZE)
    {
	if (tcsetattr(tty_fd(t), TCSANOW, &t->save_attr) < 0)
	{
	    dflog(LOG_WARNING, "Can't restore attributes for device %s: %s",
		   tty_name(t), strerror(errno));
	    return -1;
	}
	else
	    memset(&t->save_attr, 0, sizeof(t->save_attr));
    }

    return 0;
}

int tty_set_raw(tty_t *t)
{
    struct termios old_tios;
    if (tcgetattr(tty_fd(t), &old_tios) < 0)
    {
	dflog(LOG_ERR, "Can't get attributes for device %s: %s",
	      tty_name(t), strerror(errno));
	return -1;
    }

    struct termios new_tios;
    memcpy(&new_tios, &old_tios, sizeof(new_tios));

    cfmakeraw(&new_tios);

    // done in cfmakeraw but not in an old *libc:
    new_tios.c_cc[VMIN]  = 1;
    new_tios.c_cc[VTIME] = 0;

    // not done in cfmakeraw:
    new_tios.c_iflag |=  IGNBRK;	// cleared by cfmakeraw
    new_tios.c_cflag |=  CLOCAL;	// Ignore modem control lines (default)
    new_tios.c_iflag &= ~HUPCL;		// !hang up (lower modem control lines) on close


    if (tcsetattr(tty_fd(t), TCSANOW, &new_tios) < 0)
    {
	dflog(LOG_ERR, "Can't set attributes for device %s: %s",
	      tty_name(t), strerror(errno));
	return -1;
    }

    return 0;
}

static int tty_get_modem_state_(tty_t *t)
{
    int val;

    if (ioctl(tty_fd(t), TIOCMGET, &val) < 0)
    {
	dflog(LOG_ERR, "Could not get modem state for device %s: %s",
	      tty_name(t), strerror(errno));
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

static inline
void set_bit(int *dst_val, int dst_mask, int src_val, int src_mask)
{
    if ( src_val &   src_mask )
	*dst_val |=  dst_mask;
    else
	*dst_val &= ~dst_mask;
}

int tty_set_modem_state(tty_t *t, modem_state_t  mstate)
{
    int val = tty_get_modem_state_(t);
    if (val < 0) return val;

    val &= ~(TIOCM_DTR | TIOCM_DSR | TIOCM_LE | TIOCM_RTS | TIOCM_CTS |
	     TIOCM_CD  | TIOCM_RI);

    set_bit(&val, TIOCM_DTR, mstate, TTY_MODEM_DTR);
    set_bit(&val, TIOCM_DSR, mstate, TTY_MODEM_DSR);
#if 0
    set_bit(&val, TIOCM_RTS, mstate, TTY_MODEM_RTS);
    set_bit(&val, TIOCM_CTS, mstate, TTY_MODEM_CTS);
#endif
    set_bit(&val, TIOCM_CD , mstate, TTY_MODEM_CD );
    set_bit(&val, TIOCM_RI , mstate, TTY_MODEM_RI );

    if (ioctl(tty_fd(t), TIOCMSET, &val) < 0)
    {
	dflog(LOG_ERR, "Could not set modem state for device %s: %s",
	      tty_name(t), strerror(errno));
	return -1;
    }

    return 0;
}

void tty_log_modem_state(const char *pfx, modem_state_t mstate)
{
    char buf[64];

    snprintf(buf, sizeof(buf), "DTR:%d DSR:%d RTS:%d CTS:%d CD:%d RI:%d",
             !!(mstate & TTY_MODEM_DTR),
	     !!(mstate & TTY_MODEM_DSR),
	     !!(mstate & TTY_MODEM_RTS),
	     !!(mstate & TTY_MODEM_CTS),
	     !!(mstate & TTY_MODEM_CD ),
	     !!(mstate & TTY_MODEM_RI )
	    );

    dflog(LOG_INFO, "%s0x%02X, %s", pfx?pfx:"", mstate, buf);
}
