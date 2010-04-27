#include "sys_headers.h"
#include "tty.h"
#include "misc.h"


static void tty_on_exit(int unused, void *arg)
{
    (void)unused;
    tty_t *tty = arg;
    tty_delete(tty);
}

tty_t *tty_create(const char *devname)
{
    tty_t *tty = xzmalloc(sizeof(*tty));

    tty->fd = -1;

    onexit(tty_on_exit, tty);

    tty->name = xstrdup(devname);

    return tty;
}

void tty_delete(tty_t *tty)
{
    tty_restore(tty);
    tty_close  (tty);
    tty_unlock (tty);
    free       (tty->name);
    free       (tty);
}


void tty_open(tty_t *tty)
{
    tty->fd = open(tty->name, O_RDWR | O_NOCTTY);
    if (tty->fd < 0)
    {
	syslog(LOG_ERR, "Can't open device %s: %m", tty->name);
	fail();
    }
}

void tty_close (tty_t *tty)
{
    if (tty->fd >= 0)
    {
	/* To avoid blocking on close if we have written bytes and are in
	 flow-control, we flush the output queue. */
	tcflush(tty->fd, TCOFLUSH);

	close(tty->fd);
	tty->fd = -1;
    }
}


void tty_lock(tty_t *tty)
{
    pid_t rc = dev_lock(tty->name);

    if (rc < 0)
	syslog(LOG_ERR, "Error while locking %s: %m", tty->name);
    else if (rc > 0)
	syslog(LOG_ERR, "%s is locked by %d", tty->name, rc);
    else
    {
        tty->locked = 1;
	return;
    }

    fail();
}

void tty_unlock(tty_t *tty)
{
    if (tty->locked)
    {
	pid_t rc = dev_unlock(tty->name, getpid());

	if (rc < 0)
	    syslog(LOG_WARNING, "Error while unlocking %s: %m", tty->name);

	tty->locked = 0;
    }
}


void tty_set_raw(tty_t *tty)
{
    if (tcgetattr(tty->fd, &tty->termios) < 0)
    {
	syslog(LOG_ERR, "Can't get attributes for device %s: %m", tty->name);
	fail();
    }

    struct termios new_tios;
    memcpy(&new_tios, &tty->termios, sizeof(new_tios));

    cfmakeraw(&new_tios);
    new_tios.c_cc[VMIN]  = 1;
    new_tios.c_cc[VTIME] = 0;

    if (tcsetattr(tty->fd, TCSANOW, &new_tios) < 0)
    {
	syslog(LOG_ERR, "Can't set attributes for device %s: %m", tty->name);
	fail();
    }
}

void tty_restore(tty_t *tty)
{
    if (tty->fd >= 0 && tty->termios.c_cflag & CSIZE)
    {
	if (tcsetattr(tty->fd, TCSANOW, &tty->termios) < 0)
	    syslog(LOG_WARNING, "Can't restore attributes for device %s: %m",
		   tty->name);
	else
	    memset(&tty->termios, 0, sizeof(tty->termios));
    }
}

