#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <error.h>
#include <signal.h>
#include <string.h>
#include <fcntl.h>
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


static int infd = -1, rsfd = -1;
static struct termios saved_stdin_tattr;
static struct termios saved_rs232_tattr;

static void restore(void)
{
    if (infd >= 0) tcsetattr(infd, TCSANOW, &saved_stdin_tattr);
    if (rsfd >= 0) tcsetattr(rsfd, TCSANOW, &saved_rs232_tattr);
}

static void _setmode(const char *name, int fd, struct termios *save_tattr)
{
    int rc = tcgetattr(fd, save_tattr);
    if (rc != 0)
    {
        restore();
	error(EXIT_FAILURE, errno, "%s tcgetattr", name);
    }

    struct termios tattr;

    memcpy(&tattr, save_tattr, sizeof(tattr));

    cfmakeraw(&tattr);
    tattr.c_cc[VMIN]  = 1;
    tattr.c_cc[VTIME] = 0;

    rc = tcsetattr(fd, TCSANOW, &tattr);
    if (rc != 0)
    {
        restore();
	error(EXIT_FAILURE, errno, "%s tcsetattr", name);
    }
}

static void setmode(void)
{
    _setmode("stdin", infd, &saved_stdin_tattr);
    _setmode("rs232", rsfd, &saved_rs232_tattr);
}


int main(int ac, char *av[])
{
    if (ac != 2)
	usage();

    int rc;

    rsfd = open(av[1], O_RDWR | O_NOCTTY);
    if (rsfd < 0)
	error(EXIT_FAILURE, 0, "can't open %s", av[1]);

    rc = isatty(rsfd);
    if (rc < 1)
	error(EXIT_FAILURE, 0, "%s is not a tty", av[1]);

    infd = STDIN_FILENO;
    rc = isatty(infd);
    if (rc < 1)
	error(EXIT_FAILURE, 0, "stdin is not a tty");


    setmode();

    int old_c = 0;

    do
    {
	int c;
	fputs("press any key ... ", stdout); fflush(stdout);

	ssize_t l = read(infd, &c, 1);
	if (l < 0)
	{
            restore();
	    error(EXIT_FAILURE, errno, "read");
	}
	if (l == 0)
	{
            c = 0;
            fprintf(stderr, "EOF\r\n");
	}
	else
	{
	    printf("thanks: '%c' : %d\r\n", c, c); fflush(stdout);
	    if (c == 3 && old_c == 3)
	    {
		fprintf(stderr, "Double ^C, exit\r\n");
		break;
	    }
	    old_c = c;
	}

    } while (1);


    restore();
    return EXIT_SUCCESS;
}

