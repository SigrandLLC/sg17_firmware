#define _GNU_SOURCE
#include "sys_headers.h"
#include "pidfile.h"
#include "tty.h"
#include "socket.h"
#include "misc.h"

void usage(void)
{
    fprintf(stderr,
	    "Usage: %s /dev/ttyPORT host port {listen|connect} pidfile\n"
	    , progname);
    exit(EXIT_FAILURE);
}

void sig_handler(int sig)
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
    if (ac != 6)
	usage();

    const char *device   = av[1];
    const char *host     = av[2];
    const char *port     = av[3];
    const char *conntype = av[4];
    const char *pid_file = av[5];

    int listen = 1;

    if ( strcmp(conntype, "listen") == 0)
	listen = 1;
    else if( strcmp(conntype, "connect") == 0)
	listen = 0;
    else
        usage();

    openlog(progname, LOG_PID | LOG_CONS, LOG_DAEMON);

    int rc = daemon(0, 0);
    if (rc < 0)
    {
	syslog(LOG_ERR, "daemonize error: %m");
	fail();
    }
    openlog(progname, LOG_PID | LOG_CONS, LOG_DAEMON);
    syslog(LOG_NOTICE, "started up");

    make_pidfile(pid_file);

    /* Ignore SIGPIPEs so they don't kill us. */
    signal(SIGPIPE, SIG_IGN);

    setup_sighandler(sig_handler, SIGHUP);
    setup_sighandler(sig_handler, SIGINT);
    setup_sighandler(sig_handler, SIGQUIT);
    setup_sighandler(sig_handler, SIGTERM);


    tty_t       *tty = tty_create();
    tty_open    (tty, device);
    tty_set_raw (tty);


    static const size_t   DATA_BUF_SIZE = 4096;
    static const size_t STATUS_BUF_SIZE =  256;

    char *  data_buf = xmalloc(  DATA_BUF_SIZE);
    char *status_buf = xmalloc(STATUS_BUF_SIZE);


    socket_t *data_s = NULL, *stat_s = NULL;

    if (listen)
    {
	socket_t *listen_s = socket_create();
	socket_bind(listen_s, host, port);

	syslog(LOG_DEBUG, "Waiting data connection...");
	data_s = socket_accept(listen_s);
	syslog(LOG_INFO, "Data connection from %s", socket_name(data_s));

	syslog(LOG_DEBUG, "Waiting status connection...");
	stat_s = socket_accept(listen_s);
	syslog(LOG_INFO, "Status connection from %s", socket_name(stat_s));

	syslog(LOG_DEBUG, "Closing listening socket...");
	socket_close(listen_s);
    }
    else // connect
    {
	syslog(LOG_DEBUG, "Connecting (data) to %s:%s ...", host, port);
	data_s = socket_create();
	socket_connect(data_s, host, port);

	syslog(LOG_DEBUG, "Connecting (status) to %s:%s ...", host, port);
	stat_s = socket_create();
	socket_connect(stat_s, host, port);
    }


    enum { POLL_TTY, POLL_DATA, POLL_STATUS, POLL_ITEMS };

    struct pollfd polls[POLL_ITEMS];
    memset(polls, 0, sizeof(polls));


    polls[POLL_TTY   ].fd =    tty_fd(tty);
    polls[POLL_DATA  ].fd = socket_fd(data_s);
    polls[POLL_STATUS].fd = socket_fd(stat_s);

    int i;
    for (i = 0; i < POLL_ITEMS; ++i)
	polls[i].events = POLLIN | POLLRDHUP;

    do
    {
	rc = poll(polls, POLL_ITEMS, 5000);
	if (rc < 0)
	{
	    syslog(LOG_ERR, "poll failed: %m");
	    fail();
	}
	else if (rc == 0)	// timeout
	{
	    // handle modem lines status
	    syslog(LOG_INFO, "tick"); //TODO
	}
	else
	{
	    // do i/o

	    if (polls[POLL_TTY].revents & POLLIN)
	    {
		ssize_t r = read(polls[POLL_TTY].fd, data_buf, DATA_BUF_SIZE);
		if (r < 0)
		{
		    syslog(LOG_ERR, "Error reading %s: %m", tty_name(tty));
		    fail();
		}
		else if (r == 0)
		{
		    syslog(LOG_WARNING, "EOF readed from %s, ignore", tty_name(tty));
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
		    syslog(LOG_INFO, "EOF received from %s", socket_name(data_s));
		    syslog(LOG_INFO, "closing data connection");
		    break; // FIXME: restart
		}
		else
		{
		    //tty_write_all(tty, data_buf, r);
		    //FIXME: TODO
		}
	    }
	}
    } while(1);


    syslog(LOG_NOTICE, "finished");
    return EXIT_SUCCESS;
}
