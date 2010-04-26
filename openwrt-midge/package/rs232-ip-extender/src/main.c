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


    tty_descr_t *tty = tty_create(device);
    tty_lock    (tty);
    tty_open    (tty);
    tty_set_raw (tty);


    socket_t *data_s = NULL, *stat_s = NULL;

    if (listen)
    {
	socket_t *listen_s = socket_create();
	socket_bind(listen_s, host, port);

	syslog(LOG_DEBUG, "Waiting data connection...");
	data_s = socket_accept(listen_s);
	syslog(LOG_INFO, "Data connection from %s : %s", data_s->host, data_s->port);

	syslog(LOG_DEBUG, "Waiting status connection...");
	stat_s = socket_accept(listen_s);
	syslog(LOG_INFO, "Status connection from %s : %s", stat_s->host, stat_s->port);

	syslog(LOG_DEBUG, "Closing listening socket...");
	socket_close(listen_s);
	syslog(LOG_DEBUG, "");
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


    syslog(LOG_NOTICE, "finished");
    return EXIT_SUCCESS;
}
