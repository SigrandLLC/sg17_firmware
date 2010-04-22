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

    //+ network init
    //++ server part
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family    = AF_UNSPEC;	// Allow IPv4 or IPv6
    hints.ai_socktype  = SOCK_STREAM;
    hints.ai_flags     = AI_PASSIVE;	// For wildcard IP address, INADDR_ANY | IN6ADDR_ANY_INIT
    hints.ai_protocol  = IPPROTO_TCP;
    hints.ai_canonname = NULL;
    hints.ai_addr      = NULL;
    hints.ai_next      = NULL;

    struct addrinfo *result;
    rc = getaddrinfo(NULL/*host*/, port, &hints, &result);
    if (rc != 0)
    {
	syslog(LOG_ERR, "getaddrinfo: %s", gai_strerror(rc));
        fail();
    }

    /* getaddrinfo() returns a list of address structures.
       Try each address until we successfully bind(2).
       If socket(2) (or bind(2)) fails, we (close the socket
       and) try the next address. */

    struct addrinfo *rp;
    int sockfd;
    for (rp = result; rp != NULL; rp = rp->ai_next)
    {
	sockfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
	if (sockfd < 0) continue;

	if (bind(sockfd, rp->ai_addr, rp->ai_addrlen) == 0)
	    break;                  // Success

	close(sockfd);
    }

    if (rp == NULL)	// No address succeeded
    {
	syslog(LOG_ERR, "Could not bind on %s : %s", host, port);
        fail();
    }

    freeaddrinfo(result);	// No longer needed

    rc = listen(sockfd, 2);
    if (rc < 0)
    {
	syslog(LOG_ERR, "listen call error: %m");
        fail();
    }

    //-- server part
    //- network init


    unlock(device);
    close_tty(devfd);
    return EXIT_SUCCESS;
}
