#include "./common.h"


int main(int ac, char *av[])
{
    if (ac < 2 || ac > 3)
	usage(basename(av[0]));

    int rc;

    atexit(restore);

    devs_init();


    fds[RS232].fd = open(av[1], O_RDWR | O_NOCTTY | O_NONBLOCK);
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
		//prchar2pfx_nl("Input: ", c);

		int mstat;
		int what;
		const char *wstr;
		switch(c)
		{
		    case 'z': mstat=TIOCM_LE ; wstr="set LE "; what=TIOCMBIS; break;
		    case 'Z': mstat=TIOCM_LE ; wstr="clr LE "; what=TIOCMBIC; break;
		    case 'x': mstat=TIOCM_DTR; wstr="set DTR"; what=TIOCMBIS; break;
		    case 'X': mstat=TIOCM_DTR; wstr="clr DTR"; what=TIOCMBIC; break;
		    case 'c': mstat=TIOCM_DSR; wstr="set DSR"; what=TIOCMBIS; break;
		    case 'C': mstat=TIOCM_DSR; wstr="clr DSR"; what=TIOCMBIC; break;
		    case 'v': mstat=TIOCM_RTS; wstr="set RTS"; what=TIOCMBIS; break;
		    case 'V': mstat=TIOCM_RTS; wstr="clr RTS"; what=TIOCMBIC; break;
		    case 'b': mstat=TIOCM_CTS; wstr="set CTS"; what=TIOCMBIS; break;
		    case 'B': mstat=TIOCM_CTS; wstr="clr CTS"; what=TIOCMBIC; break;
		    case 'n': mstat=TIOCM_CD ; wstr="set CD "; what=TIOCMBIS; break;
		    case 'N': mstat=TIOCM_CD ; wstr="clr CD "; what=TIOCMBIC; break;
		    case 'm': mstat=TIOCM_RI ; wstr="set RI "; what=TIOCMBIS; break;
		    case 'M': mstat=TIOCM_RI ; wstr="clr RI "; what=TIOCMBIC; break;
		    case  3 :
			fprintf(stderr, "^C, exit\r\n");
			exit(EXIT_SUCCESS);
		    default:
			prchar2pfx_nl("Wrong input: ", c);
			continue;
		}

		printf("%s\r\n", wstr);
		if (ioctl(fds[RS232].fd, what, &mstat) < 0)
		    error(EXIT_FAILURE, errno, "TIOCMBIS | TIOCMBIC");
	    }
	}

    } while (1);


    return EXIT_SUCCESS;
}

