#include "sys_headers.h"
#include "tty_raw.h"
#include "misc.h"

static void unlock_tty__(int unused, void *device)
{
    (void)unused;
    unlock_tty(device);
}

void lock_tty(const char *device)
{
    pid_t rc = dev_lock(device);

    if (rc < 0)
	syslog(LOG_ERR, "Error while locking %s: %m", device);
    else if (rc > 0)
	syslog(LOG_ERR, "%s is locked by %d", device, rc);
    else
    {	// successfully locked
	onexit(unlock_tty__, deconst(device));
	return;
    }

    fail();
}

void unlock_tty(const char *device)
{
    pid_t rc = dev_unlock(device, getpid());
    if (rc < 0)
	syslog(LOG_WARNING, "Error while unlocking %s: %m", device);
}


static void close_tty__(int unused, void* ttyfd)
{
    (void)unused;
    close_tty((int)ttyfd);
}

int open_tty(const char *device)
{
    int ttyfd = open(device, O_RDWR | O_NOCTTY);
    if (ttyfd < 0)
    {
	syslog(LOG_ERR, "Can't open device %s: %m", device);
	fail();
    }

    onexit(close_tty__, (void*)ttyfd);

    return ttyfd;
}

void close_tty(int ttyfd)
{
    /* To avoid blocking on close if we have written bytes and are in
       flow-control, we flush the output queue. */
    tcflush(ttyfd, TCOFLUSH);
    close(ttyfd);
}


void set_raw_tty(int ttyfd, struct termios *save_tios)
{
    struct termios old_tios;
    if (tcgetattr(ttyfd, &old_tios) < 0)
    {
	//syslog(LOG_ERR, "Can't get attributes for device %s: %m", device);
	syslog(LOG_ERR, "Can't get tty attributes: %m");
	fail();
    }

    if (save_tios != NULL)
	memcpy(save_tios, &old_tios, sizeof(old_tios));

    struct termios new_tios;
    memcpy(&new_tios, &old_tios, sizeof(old_tios));

    cfmakeraw(&new_tios);
    new_tios.c_cc[VMIN]  = 1;
    new_tios.c_cc[VTIME] = 0;

    if (tcsetattr(ttyfd, TCSANOW, &new_tios) < 0)
    {
	//syslog(LOG_ERR, "Can't set attributes for device %s: %m", device);
	syslog(LOG_ERR, "Can't set tty attributes: %m");
	fail();
    }
}

void restore_tty(int ttyfd, struct termios *tios)
{
    if (tios == NULL) return;

    if (tcsetattr(ttyfd, TCSANOW, tios) < 0)
    {
	//syslog(LOG_WARNING, "Can't set attributes for device %s: %m", device);
	syslog(LOG_WARNING, "Can't set tty attributes: %m");
    }
}

