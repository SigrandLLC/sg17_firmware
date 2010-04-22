#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <error.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <termios.h>
#include <lockdev.h>
#include "utils.h"

const char *progname = "rs232-ip-extender";

void fail(void)
{
    syslog(LOG_ERR, "%s failed, exiting", progname);
    exit(EXIT_FAILURE);
}

void onexit(void (*function)(int, void *), void *arg)
{
    int rc = on_exit(function, arg);
    if (rc < 0)
    {
	syslog(LOG_ERR, "on_exit: %m");
        fail();
    }
}

void make_pidfile(const char *pidfile)
{
    if (  !pidfile) return;
    FILE *fpidfile = fopen(pidfile, "w");
    if ( !fpidfile)
    {
	syslog(LOG_ERR, "Error opening pidfile '%s': %m", pidfile);
        fail();
    }
    fprintf(fpidfile, "%d\n", getpid());
    fclose (fpidfile);
}

void lock(const char *device)
{
    pid_t rc = dev_lock(device);

    if (rc < 0)
	syslog(LOG_ERR, "Error while locking %s: %m", device);
    else if (rc > 0)
	syslog(LOG_ERR, "%s is locked by %d", device, rc);
    else
	return; // successfully locked

    fail();
}

void unlock(const char *device)
{
    pid_t rc = dev_unlock(device, getpid());
    if (rc < 0)
	syslog(LOG_WARNING, "Error while unlocking %s: %m", device);
}

int open_tty(const char *device)
{
    int devfd = open(device, O_RDWR | O_NOCTTY);
    if (devfd < 0)
    {
	syslog(LOG_ERR, "Can't open device %s: %m", device);
	fail();
    }

    struct termios termios;
    int rc = tcgetattr(devfd, &termios);
    if (rc < 0)
    {
	syslog(LOG_ERR, "Can't get attributes for device %s: %m", device);
	fail();
    }

    cfmakeraw(&termios);
    termios.c_cc[VMIN]  = 1;
    termios.c_cc[VTIME] = 0;

    rc = tcsetattr(devfd, TCSANOW, &termios);
    if (rc < 0)
    {
	syslog(LOG_ERR, "Can't set attributes for device %s: %m", device);
	fail();
    }

    return devfd;
}

void close_tty(int devfd)
{
    /* To avoid blocking on close if we have written bytes and are in
       flow-control, we flush the output queue. */
    tcflush(devfd, TCOFLUSH);
    close(devfd);
}

