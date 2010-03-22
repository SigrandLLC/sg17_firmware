#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <error.h>
#include <signal.h>
#include <termios.h>

#include "./tty.h"


static void usage(void)
{
    fprintf(stderr, "Usage: sertest device\n");
    exit(EXIT_FAILURE);
}

static struct termios saved_tattr;
static void restore(void)
{
    tty_restore_attr(STDIN_FILENO, &saved_tattr);
}

int main(int ac, char *av[])
{
    if (ac != 2)
	usage();

    int rc;

    (void)av;

    rc = isatty(STDIN_FILENO);
    if (rc < 1)
	error(EXIT_FAILURE, 0, "stdin is not a tty");

    rc = tty_set_char_input_mode(STDIN_FILENO, 1, &saved_tattr);
    if (rc < 0)
	error(EXIT_FAILURE, errno, "(tcgetattr | tcsetattr)");

    int old_c = 0;

    do
    {
	int c;
	fputs("press any key ... ", stdout); fflush(stdout);

	ssize_t l = read(STDIN_FILENO, &c, 1);
	if (l < 0)
	{
            restore();
	    error(EXIT_FAILURE, errno, "read");
	}
	if (l == 0)
	{
            c = 0;
            fprintf(stderr, "EOF\n");
	}
	else
	{
	    printf("thanks: '%c' : %d\n", c, c); fflush(stdout);
	    if (c == 3 && old_c == 3)
	    {
		fprintf(stderr, "Double ^C, exit\n");
		break;
	    }
	    old_c = c;
	}

    } while (1);


    restore();
    return EXIT_SUCCESS;
}

