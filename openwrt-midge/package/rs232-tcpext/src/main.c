#define _GNU_SOURCE
#include "sys_headers.h"
#include "pidfile.h"
#include "tty.h"
#include "socket.h"
#include "misc.h"

void usage(const char *av0)
{
    fprintf(stderr,
	    "Usage: %s /dev/ttyPORT host port {listen|connect} pidfile P R\n"
	    "\tP - modem state poll interval, msec\n"
	    "\tR - connection restart delay time, msec\n"
	    , basename(av0));
    exit(EXIT_FAILURE);
}

static void sig_handler(int sig)
{
    syslog(LOG_NOTICE, "%s signal catched", strsignal(sig));
    if (sig == SIGTERM)
    {
	syslog(LOG_NOTICE, "exiting");
	exit(EXIT_SUCCESS);
    }
    else
	syslog(LOG_NOTICE, "do nothing");
}

int main(int ac, char *av[]/*, char *envp[]*/)
{
    if (ac != 8)
	usage(av[0]);

    size_t ai = 0;
    const char *device       = av[++ai];
    const char *host         = av[++ai];
    const char *port         = av[++ai];
    const char *conntype     = av[++ai];
    const char *pid_file     = av[++ai];
    size_t mstat_intval = atoi(av[++ai]);
    size_t restart_delay= atoi(av[++ai]);

    int listen = 1;

    if ( strcmp(conntype, "listen") == 0)
	listen = 1;
    else if( strcmp(conntype, "connect") == 0)
	listen = 0;
    else
        usage(av[0]);

    static char progname[256];
    snprintf(progname, sizeof(progname), "rs232-tcpext %-7s %-7s",
	     basename(device), conntype);

    openlog(progname, LOG_CONS, LOG_DAEMON);

    int rc = daemon(0, 0);
    if (rc < 0)
    {
	syslog(LOG_ERR, "daemonize error: %m");
	fail();
    }

    openlog(progname, LOG_CONS, LOG_DAEMON);
    syslog(LOG_NOTICE, "started up");

    make_pidfile(pid_file);

    /* Ignore SIGPIPEs so they don't kill us. */
    signal(SIGPIPE, SIG_IGN);

    setup_sighandler(sig_handler, SIGHUP);
    setup_sighandler(sig_handler, SIGINT);
    setup_sighandler(sig_handler, SIGQUIT);
    setup_sighandler(sig_handler, SIGTERM);


    static const size_t   DATA_BUF_SIZE = 4096;
    static const size_t  STATE_BUF_SIZE =  256;

    char * data_buf = xmalloc( DATA_BUF_SIZE);
    char *state_buf = xmalloc(STATE_BUF_SIZE);

    socket_t *data_s = NULL, *state_s = NULL, *listen_s = NULL;

    tty_t *tty = tty_create();

    if (listen)
    {
	listen_s = socket_create();
    }
    else
    {
	 data_s = socket_create();
	state_s = socket_create();
    }


    do // restart
    {
	tty_open    (tty, device);
	tty_set_raw (tty);


	if (listen)
	{
	    socket_bind(listen_s, host, port);

	    syslog(LOG_INFO, "Waiting data connection...");
	    data_s = socket_accept(listen_s);
	    syslog(LOG_INFO, "Data connection from %s", socket_name(data_s));

	    syslog(LOG_INFO, "Waiting state connection...");
	    state_s = socket_accept(listen_s);
	    syslog(LOG_INFO, "State connection from %s", socket_name(state_s));

	    //syslog(LOG_INFO, "Closing listening socket...");
	    socket_close(listen_s);
	}
	else // connect
	{
	    syslog(LOG_INFO, "Connecting (data) to %s:%s ...", host, port);
	    if ( socket_connect(data_s, host, port) )
                goto cleanup; // restart

	    syslog(LOG_INFO, "Connecting (state) to %s:%s ...", host, port);
	    if ( socket_connect(state_s, host, port) )
                goto cleanup; // restart
	}


	enum { POLL_TTY, POLL_DATA, POLL_STATE, POLL_ITEMS };

	struct pollfd polls[POLL_ITEMS];
	memset(polls, 0, sizeof(polls));


	polls[POLL_TTY  ].fd =    tty_fd(tty);
	polls[POLL_DATA ].fd = socket_fd( data_s);
	polls[POLL_STATE].fd = socket_fd(state_s);

	int i;
	for (i = 0; i < POLL_ITEMS; ++i)
	    polls[i].events = POLLIN | POLLRDHUP;

	do
	{
	    rc = poll(polls, POLL_ITEMS, mstat_intval);
	    if (rc < 0)
	    {
		syslog(LOG_ERR, "poll failed: %m");
		fail();
	    }
	    else if (rc == 0)	// timeout
	    {
		// handle modem lines state
		syslog(LOG_INFO, "tick"); //FIXME: TODO
	    }
	    else
	    {
		// do i/o

		if (polls[POLL_TTY].revents & POLLIN)
		{
		    size_t r = tty_read(tty, data_buf, DATA_BUF_SIZE);
		    if (r == 0)
		    {
			syslog(LOG_WARNING, "EOF readed from %s, ignore",
			       tty_name(tty));
		    }
		    else	// r > 0
		    {
			socket_send_all(data_s, data_buf, r);
		    }
		}

		if (polls[POLL_DATA].revents & POLLIN)
		{
		    size_t r = socket_recv(data_s, data_buf, DATA_BUF_SIZE);
		    if (r == 0)
		    {
			syslog(LOG_INFO, "data connection: EOF received from %s",
			       socket_name(data_s));
			break; // restart
		    }
		    else
		    {
			tty_write_all(tty, data_buf, r);
		    }
		}

		if (polls[POLL_STATE].revents & POLLIN)
		{
		    size_t r = socket_recv(state_s, state_buf, STATE_BUF_SIZE);
		    if (r == 0)
		    {
			syslog(LOG_INFO, "state connection: EOF received from %s",
			       socket_name(state_s));
			break; // restart
		    }
		    else
		    {
			size_t i;
			for (i = 0; i < r; ++i)
			    tty_set_modem_state(tty, state_buf[i]);
		    }
		}
	    }
	} while(1); // poll loop

    cleanup :
	socket_close(state_s);
        socket_close( data_s);
	tty_close(tty);
        if (restart_delay != 0)
	    usleep(restart_delay * 1000);
	syslog(LOG_NOTICE, "restart");
    } while(1); // restart

    syslog(LOG_NOTICE, "finished");
    return EXIT_SUCCESS;
}
