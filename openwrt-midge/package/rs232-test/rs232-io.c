#include "./common.h"


int main(int ac, char *av[])
{
    if (ac < 2 || ac > 3)
	usage(basename(av[0]));

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


    unsigned char old_c = 0;

    do
    {
	unsigned char c;

	//fputs("press any key ... ", stdout); fflush(stdout);

	int rc = poll(fds, NDEVICES, -1);

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
		printf("Input: EOF\r\n");
	    }
	    else
	    {
		prchar2pfx_nl("Input: ", c);

		rc = write(fds[RS232].fd, &c, 1);
                if (rc < 0)
		    error(EXIT_SUCCESS, errno, "RS232 write");
                else if (rc == 0)
		    error(EXIT_SUCCESS, errno, "RS232 write: 0 written\r\n");

		if (c == 3 && old_c == 3)
		{
		    fprintf(stderr, "Input: Double ^C, exit\r\n");
		    break;
		}
		old_c = c;
	    }
	}

	if (fds[RS232].revents & POLLIN)
	{
	    ssize_t l = read(fds[RS232].fd, &c, 1);
	    if (l < 0)
		error(EXIT_FAILURE, errno, "RS232 read");
	    else if (l == 0)
	    {
		c = 0;
		fprintf(stderr, "RS232: EOF\r\n");
	    }
	    else
	    {
		prchar2pfx_nl("RS232: ", c);
	    }
	}

    } while (1);


    return EXIT_SUCCESS;
}

