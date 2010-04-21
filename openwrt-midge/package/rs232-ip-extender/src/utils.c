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
#include "utils.h"

const char *progname = "rs232-ip-extender";

void fail(void)
{
    syslog(LOG_ERR, "%s failed, exiting", progname);
    exit(EXIT_FAILURE);
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

enum LOCK_STATE lock(const char *lck_file)
{
    char buf[64];
    int fd;
    int pid = 0;

    fd = open(lck_file, O_RDONLY);
    if (fd < 0 && errno != ENOENT)
    {
	syslog(LOG_ERR, "Can't open lock file %s: %m", lck_file);
        return LOCK_ERROR;
    }

    enum LOCK_STATE lck_state = LOCK_ERROR;

    int n = read(fd, buf, sizeof(buf));
    close(fd);
    if( n == 4 ) 		/* Kermit-style lockfile. */
	pid = *(int *)buf;
    else if( n > 0 )	/* Ascii lockfile. */
    {
	buf[n] = 0;
	sscanf(buf, "%d", &pid);
    }

    if( pid > 0 && kill((pid_t)pid, 0) < 0 && errno == ESRCH )
    {
	/* death lockfile - remove it */
	unlink(lck_file);
	sleep(1);
	lck_state = LOCK_OK;
    }
    else
    {
	lck_state = LOCK_BY_OTHER;
    }

    if( lck_state == LOCK_OK )
    {
	int mask;

	mask = umask(022);
	fd = open(lck_file, O_WRONLY | O_CREAT | O_EXCL, 0666);
	umask(mask);
	if( fd >= 0 )
	{
	    snprintf( buf, sizeof(buf), "%10ld\t%s\n", (long)getpid(), progname );
	    write( fd, buf, strlen(buf) );
	    close(fd);
            lck_state = LOCK_OK;
	}
	else
	{
	    lck_state = LOCK_BY_OTHER;
	}
    }

    return lck_state;
}

void unlock(const char *lck_file)
{
    int rc = unlink(lck_file);
    if (rc < 0 && errno != ENOENT)
	syslog(LOG_WARNING, "Can't unlink lock file %s: %m", lck_file);
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

