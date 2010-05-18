#define _GNU_SOURCE
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <error.h>
#include <signal.h>
#include <string.h>
#include <fcntl.h>
#include <poll.h>
#include <termios.h>
#include <sys/ioctl.h>
#include "./prchar.h"


#define TRACE() fprintf(stderr, "%s:%04d\n", __FILE__, __LINE__)

#define TRACEF(fmt, args...)					\
    do {							\
	fprintf(stderr, "%s:%04d: ", __FILE__, __LINE__);	\
	fprintf(stderr, fmt , ##args);				\
    } while(0)


static void usage(const char *name)
{
    fprintf(stderr, "Usage: %s rs232-device [speed]\n", name);
    exit(EXIT_FAILURE);
}


enum DeviceIndex { INPUT=0, RS232=1 };
enum { NDEVICES = 2 };

static const char     *names[NDEVICES];
static struct termios tattrs[NDEVICES];
static struct pollfd     fds[NDEVICES];

static void devs_init(void)
{
    memset(tattrs, 0, sizeof(tattrs));
    memset(fds   , 0, sizeof(fds   ));

    names[INPUT] = "stdin";
    names[RS232] = "rs232";
      fds[INPUT].fd = -1;
      fds[RS232].fd = -1;
      fds[INPUT].events = POLLIN;
      fds[RS232].events = POLLIN;
}

static void restore(void)
{
    if (fds[INPUT].fd >= 0) tcsetattr(fds[INPUT].fd, TCSANOW, &tattrs[INPUT]);
    if (fds[RS232].fd >= 0) tcsetattr(fds[RS232].fd, TCSANOW, &tattrs[RS232]);
}

static void _setmode(enum DeviceIndex i, const size_t *speedptr)
{
    if (tcgetattr(fds[i].fd, &tattrs[i]) != 0)
	error(EXIT_FAILURE, errno, "%s tcgetattr", names[i]);

    struct termios tattr;

    memcpy(&tattr, &tattrs[i], sizeof(tattr));

    cfmakeraw(&tattr);
    tattr.c_cc[VMIN]  = 1;
    tattr.c_cc[VTIME] = 0;

    if (speedptr != NULL)
    {
        speed_t speed = B0;

	switch (*speedptr)
	{
	    case 0       : speed=B0      ; break;
	    case 50      : speed=B50     ; break;
	    case 75      : speed=B75     ; break;
	    case 110     : speed=B110    ; break;
	    case 134     : speed=B134    ; break;
	    case 150     : speed=B150    ; break;
	    case 200     : speed=B200    ; break;
	    case 300     : speed=B300    ; break;
	    case 600     : speed=B600    ; break;
	    case 1200    : speed=B1200   ; break;
	    case 1800    : speed=B1800   ; break;
	    case 2400    : speed=B2400   ; break;
	    case 4800    : speed=B4800   ; break;
	    case 9600    : speed=B9600   ; break;
	    case 19200   : speed=B19200  ; break;
	    case 38400   : speed=B38400  ; break;
	    case 57600   : speed=B57600  ; break;
	    case 115200  : speed=B115200 ; break;
	    case 230400  : speed=B230400 ; break;
	    case 460800  : speed=B460800 ; break;
	    case 500000  : speed=B500000 ; break;
	    case 576000  : speed=B576000 ; break;
	    case 921600  : speed=B921600 ; break;
	    case 1000000 : speed=B1000000; break;
	    case 1152000 : speed=B1152000; break;
	    case 1500000 : speed=B1500000; break;
	    case 2000000 : speed=B2000000; break;
	    case 2500000 : speed=B2500000; break;
	    case 3000000 : speed=B3000000; break;
	    case 3500000 : speed=B3500000; break;
	    case 4000000 : speed=B4000000; break;
	    default:
		error(EXIT_FAILURE, 0, "%s unknown speed %zu", names[i], *speedptr);
	}

	if (cfsetspeed(&tattr, speed) != 0)
	    error(EXIT_FAILURE, errno, "%s cfsetspeed %zu", names[i], *speedptr);
    }

    if (tcsetattr(fds[i].fd, TCSANOW, &tattr) != 0)
	error(EXIT_FAILURE, errno, "%s tcsetattr", names[i]);
}

static void setmode(const size_t *speedptr)
{
    _setmode( INPUT, NULL );
    _setmode( RS232, speedptr );
}

