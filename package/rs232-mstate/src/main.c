//#define NO_RS232 1
//#define DO_TICK  1

#define _GNU_SOURCE
#include "sys_headers.h"
#include "tty.h"
#include "misc.h"

void usage(const char *av0)
{
    fprintf(stderr,
	    "Usage: %s /dev/ttyPORT pollint\n"
	    "\tpollint - modem state poll interval, msec\n"
	    , basename(av0));
    exit(EXIT_FAILURE);
}

static void sig_handler(int sig)
{
    syslog(LOG_WARNING, "%s signal catched", strsignal(sig));
    if (sig == SIGTERM || sig == SIGINT)
    {
	syslog(LOG_WARNING, "exiting");
	exit(EXIT_SUCCESS);
    }
    else
	syslog(LOG_WARNING, "do nothing");
}

static int get_print_modem_state(tty_t* t)
{
    modem_state_t in_mstate = 0;

#ifndef NO_RS232
    if (tty_get_modem_state(t, &in_mstate)) return -1;
#else
    (void)t;
#endif

    tty_log_modem_state("", in_mstate);

    return 0;
}

int main(int ac, char *av[]/*, char *envp[]*/)
{
    if (ac != 3)
	usage(av[0]);

    size_t ai = 0;
    const char *device       = av[++ai];
    size_t mstat_intval = atoi(av[++ai]);

    static char progname[256];
    snprintf(progname, sizeof(progname), "rs232-mstate %-7s",
	     basename(device));

    openlog(progname, LOG_CONS|LOG_PERROR, LOG_USER);
    syslog(LOG_INFO, "started up");

    setup_sighandler(sig_handler, SIGPIPE);
    setup_sighandler(sig_handler, SIGHUP);
    setup_sighandler(sig_handler, SIGINT);
    setup_sighandler(sig_handler, SIGQUIT);
    setup_sighandler(sig_handler, SIGTERM);


    tty_t *tty = tty_create();

    do // restart
    {
	tty_open(tty, device);
#ifndef NO_RS232
	if (tty_set_raw(tty))
	    fail();
#endif

	enum { POLL_TTY, POLL_ITEMS };

	struct pollfd polls[POLL_ITEMS];
	memset(polls, 0, sizeof(polls));


	polls[POLL_TTY].fd = tty_fd(tty);

	do // i/o loop
	{
	    int rc = poll(polls, 0, mstat_intval);
	    if (rc < 0)
	    {
		syslog(LOG_ERR, "poll failed: %m");
		break; // restart
	    }
	    else if (rc == 0)	// timeout
	    {
		// handle modem lines state
#ifdef DO_TICK
		syslog(LOG_INFO, "tick"); //FIXME: comment off
#endif
		// get_print_modem_state(tty) called at the loop end
	    }
	    else
	    {
	    }

	    if (get_print_modem_state(tty) < 0)
                break; // restart

	} while(1); // i/o loop

	syslog(LOG_INFO, "restart");
    } while(1); // restart

    syslog(LOG_INFO, "finished");
    return EXIT_SUCCESS;
}
