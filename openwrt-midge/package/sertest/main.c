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


#define TRACE() fprintf(stderr, "%s:%04d\n", __FILE__, __LINE__)

#define TRACEF(fmt, args...)					\
    do {							\
	fprintf(stderr, "%s:%04d: ", __FILE__, __LINE__);	\
	fprintf(stderr, fmt , ##args);				\
    } while(0)


static void usage(void)
{
    fprintf(stderr, "Usage: sertest rs232-device\n");
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

static void _setmode(enum DeviceIndex i)
{
    int rc = tcgetattr(fds[i].fd, &tattrs[i]);
    if (rc != 0)
    {
        restore();
	error(EXIT_FAILURE, errno, "%s tcgetattr", names[i]);
    }

    struct termios tattr;

    memcpy(&tattr, &tattrs[i], sizeof(tattr));

    cfmakeraw(&tattr);
    tattr.c_cc[VMIN]  = 1;
    tattr.c_cc[VTIME] = 0;

    rc = tcsetattr(fds[i].fd, TCSANOW, &tattr);
    if (rc != 0)
    {
        restore();
	error(EXIT_FAILURE, errno, "%s tcsetattr", names[i]);
    }
}

static void setmode(void)
{
    _setmode( INPUT );
    _setmode( RS232 );
}


int main(int ac, char *av[])
{
    if (ac != 2)
	usage();

    int rc;

    devs_init();

    fds[RS232].fd = open(av[1], O_RDWR | O_NOCTTY);
    if (fds[RS232].fd < 0)
	error(EXIT_FAILURE, 0, "can't open %s", av[1]);

    rc = isatty(fds[RS232].fd);
    if (rc < 1)
	error(EXIT_FAILURE, 0, "%s is not a tty", av[1]);

    fds[INPUT].fd = STDIN_FILENO;
    rc = isatty(fds[INPUT].fd);
    if (rc < 1)
	error(EXIT_FAILURE, 0, "stdin is not a tty");


    setmode();

    int old_c = 0;

    do
    {
	int c;

	//fputs("press any key ... ", stdout); fflush(stdout);

	rc = poll(fds, NDEVICES, -1);

	if (rc < 0)
	{
            restore();
	    error(EXIT_FAILURE, errno, "poll");
	}
        if (rc == 0)
	{
            restore();
	    error(EXIT_FAILURE, 0, "poll timeout\n");
	}

        if (fds[INPUT].revents & POLLIN)
	{
	    ssize_t l = read(fds[INPUT].fd, &c, 1);
	    if (l < 0)
	    {
		restore();
		error(EXIT_FAILURE, errno, "read");
	    }
	    if (l == 0)
	    {
		c = 0;
		fprintf(stderr, "Input: EOF\r\n");
	    }
	    else
	    {
		printf("Input: '%c' : %d\r\n", c, c); fflush(stdout);
		if (c == 3 && old_c == 3)
		{
		    fprintf(stderr, "Input: Double ^C, exit\r\n");
		    break;
		}
		old_c = c;
	    }
	}

    } while (1);


    restore();
    return EXIT_SUCCESS;
}

