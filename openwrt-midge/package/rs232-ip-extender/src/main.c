#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <termios.h>
#include "utils.h"

void usage(void)
{
    fprintf(stderr,
	    "Usage: %s /dev/ttyPORT host port {listen|connect} pidfile\n"
	    , progname);
    exit(EXIT_FAILURE);
}

enum CONN_TYPE { LISTEN, CONNECT };

int main(int ac, char *av[], char *envp[])
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


    lock(device);

    int devfd = open_tty(device);

    // getaddrinfo(3)

    unlock(device);
    close_tty(devfd);
    return EXIT_SUCCESS;
}

