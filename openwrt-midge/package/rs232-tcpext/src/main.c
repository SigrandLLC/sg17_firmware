//#define NO_RS232 1
//#define DO_TICK  1

#define _GNU_SOURCE
#include "sys_headers.h"
#include "pidfile.h"
#include "tty.h"
#include "socket.h"
#include "misc.h"

// POLLRDHUP since Linux 2.6.17
#ifndef  POLLRDHUP
# define POLLRDHUP 0
#endif

void usage(const char *av0)
{
    syslog(LOG_INFO,
	   "Usage: %s /dev/ttyPORT {DTE|DCE} host port IPToS {listen|connect} pidfile P R\n"
	   "\tP - modem state poll interval, msec\n"
	   "\tR - connection restart delay time, msec\n"
	    , basename(av0));
    exit(EXIT_FAILURE);
}

static void sig_handler(int sig)
{
    syslog(LOG_WARNING, "%s signal catched", strsignal(sig));
    if (sig == SIGTERM)
    {
	syslog(LOG_WARNING, "exiting");
	exit(EXIT_SUCCESS);
    }
    else
	syslog(LOG_WARNING, "do nothing");
}

static int send_modem_state(socket_t *s, modem_state_t mstate)
{
#ifndef NO_RS232
    tty_log_modem_state("Send modem state: ", mstate);
    if (socket_send_all(s, (const char *)&mstate, sizeof(mstate)))
	return -1;
#else
    (void)s;
    (void)mstate;
#endif

    return 0;
}

static int get_send_new_modem_state(tty_t* t, socket_t *s, enum TTY_TYPE tty_type)
{
    modem_state_t in_mstate = 0;
#ifndef NO_RS232
    if (tty_get_modem_state(t, &in_mstate)) return -1;
#else
    (void)t;
    (void)s;
#endif
    if ( !t->last_in_mstate_valid || t->last_in_mstate != in_mstate)
    {
	modem_state_t out_mstate = 0;

	if (tty_type == TTY_DTE)
	{
	    out_mstate |= (in_mstate & TTY_MODEM_DSR);
#if 0
	    out_mstate |= (in_mstate & TTY_MODEM_CTS);
#endif
	    out_mstate |= (in_mstate & TTY_MODEM_CD);
	    out_mstate |= (in_mstate & TTY_MODEM_RI);
	}
	else	// TTY_DCE
	{
	    out_mstate |= (in_mstate & TTY_MODEM_DTR);
#if 0
	    out_mstate |= (in_mstate & TTY_MODEM_RTS);
#endif
	}

	if (send_modem_state(s, out_mstate))
	    return -1;

	t->last_in_mstate = in_mstate;
	t->last_in_mstate_valid = 1;
    }

    return 0;
}

int main(int ac, char *av[]/*, char *envp[]*/)
{
    openlog(basename(av[0]), LOG_CONS|LOG_PERROR, LOG_DAEMON);

    if (ac != 10)
	usage(av[0]);

    size_t ai = 0;
    const char *device       = av[++ai];
    const char *devtype      = av[++ai];
    const char *host         = av[++ai];
    const char *port         = av[++ai];
    int        ip_tos = strtol(av[++ai], NULL, 0);
    const char *conntype     = av[++ai];
    const char *pid_file     = av[++ai];
    size_t mstat_intval = atoi(av[++ai]);
    size_t restart_delay= atoi(av[++ai]);


    enum TTY_TYPE tty_type = TTY_DTE; // default for PC

    if ( strcmp(devtype, "DTE") == 0)
	tty_type = TTY_DTE;
    else if( strcmp(devtype, "DCE") == 0)
	tty_type = TTY_DCE;
    else
        usage(av[0]);


    int listen = 1;

    if ( strcmp(conntype, "listen") == 0)
	listen = 1;
    else if( strcmp(conntype, "connect") == 0)
	listen = 0;
    else
        usage(av[0]);


    static char progname[256];
    snprintf(progname, sizeof(progname), "%s %-7s %-7s",
	     basename(av[0]), basename(device), conntype);

    openlog(progname, LOG_CONS, LOG_DAEMON);

    int rc = daemon(0, 0);
    if (rc < 0)
    {
	syslog(LOG_ERR, "daemonize error: %m");
	fail();
    }

    openlog(progname, LOG_CONS, LOG_DAEMON);
    syslog(LOG_WARNING, "started up");

    make_pidfile(pid_file);

    /* Ignore SIGPIPEs so they don't kill us. */
    signal(SIGPIPE, SIG_IGN);

    setup_sighandler(sig_handler, SIGHUP);
    setup_sighandler(sig_handler, SIGINT);
    setup_sighandler(sig_handler, SIGQUIT);
    setup_sighandler(sig_handler, SIGTERM);


    static const size_t   DATA_BUF_SIZE = 8192;
    static const size_t  STATE_BUF_SIZE =   64;

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
	tty_open(tty, device);

#ifndef NO_RS232
	if (tty_set_raw(tty))
	    fail();
#endif

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

        socket_set_ip_tos( data_s, ip_tos);
        socket_set_ip_tos(state_s, ip_tos);


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
#ifdef DO_TICK
		syslog(LOG_INFO, "tick"); //FIXME: comment off
#endif
		// get_send_new_modem_state(tty, state_s) called at the loop end
	    }
	    else
	    {
		// do i/o
		if (polls[POLL_DATA].revents & POLLRDHUP)
		{
		    syslog(LOG_ERR, "Data peer %s closed connection",
			   socket_name(data_s));
		    break; // restart
		}
		if (polls[POLL_STATE].revents & POLLRDHUP)
		{
		    syslog(LOG_ERR, "State peer %s closed connection",
			   socket_name(state_s));
		    break; // restart
		}

		if (polls[POLL_TTY].revents & POLLIN)
		{
		    ssize_t r = tty_read(tty, data_buf, DATA_BUF_SIZE);
		    if (r < 0)
			break; // fail, restart
		    else if (r == 0)
		    {
			syslog(LOG_WARNING, "EOF readed from %s, (DTR|DSR)==0",
			       tty_name(tty));

                        if (send_modem_state(state_s, 0/* ~(DTR|DSR|CD|RI) */))
			    break; // fail, restart

			tty_open(tty, device);
#ifndef NO_RS232
			if (tty_set_raw(tty))
			    break; // fail, restart
#endif
                        // not fail, continue with re-opened tty
		    }
		    else	// r > 0
		    {
			if (socket_send_all(data_s, data_buf, r))
			    break; // restart
		    }
		}

		if (polls[POLL_DATA].revents & POLLIN)
		{
		    ssize_t r = socket_recv(data_s, data_buf, DATA_BUF_SIZE);
		    if (r < 0)
			break; // fail, restart
		    else if (r == 0)
		    {
			syslog(LOG_INFO, "data connection: EOF received from %s",
			       socket_name(data_s));
			break; // fail, restart
		    }
		    else
		    {
			if (tty_write_all(tty, data_buf, r))
			    break; // fail, restart
		    }
		}

		if (polls[POLL_STATE].revents & POLLIN)
		{
		    ssize_t r = socket_recv(state_s, state_buf, STATE_BUF_SIZE);
		    if (r < 0)
			break; // fail, restart
		    else if (r == 0)
		    {
			syslog(LOG_INFO, "state connection: EOF received from %s",
			       socket_name(state_s));
			break; // fail, restart
		    }
		    else
		    {
			size_t i;
			for (i = 0; i < (size_t)r; ++i)
			{
			    tty_log_modem_state("Recv modem state: ", state_buf[i]);
			    if ( tty_set_modem_state(tty, state_buf[i]) )
				break; // fail, restart
			}
		    }
		}
	    }

	    if (get_send_new_modem_state(tty, state_s, tty_type))
		break; // fail, restart

	} while(1); // poll loop

    cleanup :
	socket_close(state_s);
        socket_close( data_s);
	tty_close(tty);
        if (restart_delay != 0)
	    usleep(restart_delay * 1000);
	syslog(LOG_WARNING, "restart");
    } while(1); // restart

    syslog(LOG_WARNING, "finished");
    return EXIT_SUCCESS;
}
