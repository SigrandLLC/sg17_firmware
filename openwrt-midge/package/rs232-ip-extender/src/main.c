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

enum CONN_TYPE { LISTEN, CONNECT };

int main(int ac, char *av[]/*, char *envp[]*/)
{
    if (ac != 6)
        usage();

    const char *device   = av[1];
    const char *host     = av[2];
    const char *port     = av[3];
    const char *conntype = av[4];
    const char *pid_file = av[5];

    enum CONN_TYPE conn_type;

    if ( strcmp(conntype, "listen") == 0)
	conn_type = LISTEN;
    else if( strcmp(conntype, "connect") == 0)
	conn_type = CONNECT;
    else
        usage();

    openlog(progname, LOG_PID | LOG_CONS, LOG_DAEMON);
    syslog(LOG_NOTICE, "%s startup", progname);

    int rc = daemon(0, 0);
    if (rc < 0)
    {
	syslog(LOG_ERR, "daemonize error: %m");
        fail();
    }

    make_pidfile(pid_file);

    /* Ignore SIGPIPEs so they don't kill us. */
    signal(SIGPIPE, SIG_IGN);


    tty_descr_t *tty = tty_create(device);
    tty_lock    (tty);
    tty_open    (tty);
    tty_set_raw (tty);


    socket_t *listen_s = socket_create();
    socket_bind(listen_s, host, port);

    socket_t *data_s = socket_accept(listen_s);
    syslog(LOG_INFO, "Data connection from %s : %s", data_s->host, data_s->port);

    socket_t *stat_s = socket_accept(listen_s);
    syslog(LOG_INFO, "Status connection from %s : %s", stat_s->host, stat_s->port);

    socket_close(listen_s);


    return EXIT_SUCCESS;
}
