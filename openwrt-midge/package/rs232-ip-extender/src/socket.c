#include "sys_headers.h"
#include "socket.h"
#include "misc.h"


static void socket_on_exit(int unused, void *arg)
{
    (void)unused;
    socket_t *s = arg;
    socket_delete(s);
}

socket_t *socket_create(void)
{
    socket_t *s = xzmalloc(sizeof(*s));
    s->fd = -1;

    onexit(socket_on_exit, s);

    return s;
}

void socket_delete(socket_t *s)
{
    socket_close(s);
    free(s);
}

void socket_close(socket_t *s)
{
    close(s->fd);  s->fd   = -1;
    free(s->host); s->host = NULL;
    free(s->port); s->port = NULL;
}

void socket_bind(socket_t *s, const char *host, const char *port)
{
    free(s->host); s->host = xstrdup(host);
    free(s->port); s->port = xstrdup(port);

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
    int rc = getaddrinfo(NULL/*host*/, port, &hints, &result);
    if (rc != 0)
    {
	syslog(LOG_ERR, "%s(): getaddrinfo: %s", __FUNCTION__, gai_strerror(rc));
        fail();
    }

    /* getaddrinfo() returns a list of address structures.
       Try each address until we successfully bind(2).
       If socket(2) (or bind(2)) fails, we (close the socket
       and) try the next address. */

    struct addrinfo *rp;
    int sockfd = -1;
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
	syslog(LOG_ERR, "%s(): Could not bind on %s : %s", __FUNCTION__, host, port);
        fail();
    }

    freeaddrinfo(result);	// No longer needed

    s->fd = sockfd;

    rc = listen(sockfd, 2);
    if (rc < 0)
    {
	syslog(LOG_ERR, "%s(): listen(2) failed: %m", __FUNCTION__);
        fail();
    }
}

socket_t *socket_accept (socket_t *s)
{
    struct sockaddr peer_addr;
    socklen_t       peer_addrlen;
    int newfd = accept(s->fd, &peer_addr, &peer_addrlen);
    if (newfd < 0)
    {
	syslog(LOG_ERR, "%s(): accept(2) failed: %m", __FUNCTION__);
        fail();
    }

    char peer_host[NI_MAXHOST], peer_port[NI_MAXSERV];
    int rc = getnameinfo(&peer_addr, peer_addrlen,
		     peer_host, sizeof(peer_host),
		     peer_port, sizeof(peer_port),
		     NI_NOFQDN | NI_NUMERICHOST | NI_NUMERICSERV);
    if (rc != 0)
    {
	syslog(LOG_ERR, "%s(): Connection from unknown place, getnameinfo(3) failed: %s\n",
	       __FUNCTION__, gai_strerror(rc));
        fail();
    }

    socket_t *new_s = socket_create();
    new_s->fd       = newfd;
    new_s->host     = xstrdup(peer_host);
    new_s->port     = xstrdup(peer_port);
    return new_s;
}

