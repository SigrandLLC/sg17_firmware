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

static void usage(void)
{
    fprintf(stderr, "Usage: mset rs232-device [speed]\n");
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
        speed_t speed;

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


int main(int ac, char *av[])
{
    if (ac < 2 || ac > 3)
	usage();

    int rc;

    atexit(restore);

    devs_init();


    fds[RS232].fd = open(av[1], O_RDWR | O_NOCTTY);
    if (fds[RS232].fd < 0)
	error(EXIT_FAILURE, 0, "can't open %s", av[1]);

    if (isatty(fds[RS232].fd) < 1)
	error(EXIT_FAILURE, 0, "%s is not a tty", av[1]);

    fds[INPUT].fd = STDIN_FILENO;
    if (isatty(fds[INPUT].fd) < 1)
	error(EXIT_FAILURE, 0, "stdin is not a tty");

    if (ac == 3)
    {
	size_t speed = atoi(av[2]);
	setmode( &speed );
    }
    else
	setmode( NULL );

    do
    {
	unsigned char c;

	static const char prompt[] =
	    "a:set, A:clr; zZ:LE xX:DTR cC:DSR vV:RTS bB:CTS nN:CD mM:RI > ";

	fputs(prompt, stdout); fflush(stdout);

	rc = poll(&fds[INPUT], 1, -1);

	if (rc < 0)
	    error(EXIT_FAILURE, errno, "poll");
        else if (rc == 0)
	    error(EXIT_FAILURE, 0, "poll timeout\n");

        if (fds[INPUT].revents & POLLIN)
	{
	    ssize_t l = read(fds[INPUT].fd, &c, 1);
	    if (l < 0)
		error(EXIT_FAILURE, errno, "Input read");
	    else if (l == 0)
	    {
		c = 0;
		fprintf(stderr, "Input: EOF, exit\r\n");
		break;
	    }
	    else
	    {
		printf("Input: '%c' : %d\r\n", c<040?' ':c, c); fflush(stdout);

		int mstat;
                int what;
		switch(c)
		{
		    case 'z': mstat=TIOCM_LE ; what=TIOCMBIS; break;
		    case 'Z': mstat=TIOCM_LE ; what=TIOCMBIC; break;
		    case 'x': mstat=TIOCM_DTR; what=TIOCMBIS; break;
		    case 'X': mstat=TIOCM_DTR; what=TIOCMBIC; break;
		    case 'c': mstat=TIOCM_DSR; what=TIOCMBIS; break;
		    case 'C': mstat=TIOCM_DSR; what=TIOCMBIC; break;
		    case 'v': mstat=TIOCM_RTS; what=TIOCMBIS; break;
		    case 'V': mstat=TIOCM_RTS; what=TIOCMBIC; break;
		    case 'b': mstat=TIOCM_CTS; what=TIOCMBIS; break;
		    case 'B': mstat=TIOCM_CTS; what=TIOCMBIC; break;
		    case 'n': mstat=TIOCM_CD ; what=TIOCMBIS; break;
		    case 'N': mstat=TIOCM_CD ; what=TIOCMBIC; break;
		    case 'm': mstat=TIOCM_RI ; what=TIOCMBIS; break;
		    case 'M': mstat=TIOCM_RI ; what=TIOCMBIC; break;
		    case  3 :
			fprintf(stderr, "Input: ^C, exit\r\n");
			exit(EXIT_SUCCESS);
		    default:
			printf("Wrong input: '%c' : %d\r\n", c<040?' ':c, c); fflush(stdout);
                        continue;
		}

		if (ioctl(fds[RS232].fd, what, &mstat) < 0)
		    error(EXIT_FAILURE, errno, "TIOCMBIS | TIOCMBIC");
	    }
	}

    } while (1);


    return EXIT_SUCCESS;
}

