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
    close(s->fd);   s->fd   = -1;
    free (s->name); s->name = NULL;
}

static char *make_host_port( const char *host, const char *port)
{
    size_t name_size = strlen(host) + 3 + strlen(port) + 1;
    char  *name      = xmalloc(name_size);
    snprintf(name, name_size, "%s:%s", host, port);
    return name;
}

void socket_bind(socket_t *s, const char *host, const char *port)
{
    socket_close(s);

    s->name = make_host_port(host, port);

    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family    = AF_UNSPEC;	// Allow IPv4 or IPv6
    hints.ai_socktype  = SOCK_STREAM;
    hints.ai_flags     = AI_PASSIVE;	// For wildcard IP address, INADDR_ANY | IN6ADDR_ANY_INIT
    hints.ai_protocol  = IPPROTO_TCP;

    struct addrinfo *result;
    int rc = getaddrinfo(NULL/*host*/, port, &hints, &result);
    if (rc != 0)
    {
	syslog(LOG_ERR, "%s(): getaddrinfo(NULL, %s, ...): %s", __FUNCTION__, port, gai_strerror(rc));
        fail();
    }

    /* getaddrinfo() returns a list of address structures.
       Try each address until we successfully bind(2).
       If socket(2) (or bind(2)) fails, we (close the socket
       and) try the next address. */

    struct addrinfo *rp;
    s->fd = -1;
    for (rp = result; rp != NULL; rp = rp->ai_next)
    {
	s->fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
	if (s->fd < 0) continue;

	if (bind(s->fd, rp->ai_addr, rp->ai_addrlen) == 0)
	    break;                  // Success

	close(s->fd); s->fd = -1;
    }

    if (rp == NULL)	// No address succeeded
    {
	syslog(LOG_ERR, "%s(): Could not bind to %s", __FUNCTION__, s->name);
        fail();
    }

    freeaddrinfo(result);	// No longer needed

    rc = listen(s->fd, 2);
    if (rc < 0)
    {
	syslog(LOG_ERR, "%s(): listen(2) failed: %m", __FUNCTION__);
        fail();
    }
}

socket_t *socket_accept(socket_t *s)
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
	syslog(LOG_WARNING, "%s(): Connection from unknown place, getnameinfo(3) failed: %s\n",
	       __FUNCTION__, gai_strerror(rc));
    }

    socket_t *new_s = socket_create();
    new_s->fd       = newfd;
    new_s->name     = (rc == 0)
	? make_host_port( peer_host, peer_port)
	: make_host_port( "NULL"   , "NULL"   );

    return new_s;
}

void socket_connect(socket_t *s, const char *host, const char *port)
{
    socket_close(s);
    s->name = make_host_port(host, port);

    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family    = AF_UNSPEC;	// Allow IPv4 or IPv6
    hints.ai_socktype  = SOCK_STREAM;
    hints.ai_flags     = 0;
    hints.ai_protocol  = IPPROTO_TCP;

    struct addrinfo *result;
    int rc = getaddrinfo(host, port, &hints, &result);
    if (rc != 0)
    {
	syslog(LOG_ERR, "%s(): getaddrinfo: %s", __FUNCTION__, gai_strerror(rc));
        fail();
    }

    /* getaddrinfo() returns a list of address structures.
       Try each address until we successfully connect(2).
       If socket(2) (or connect(2)) fails, we (close the socket and)
       try the next address. */

    struct addrinfo *rp;
    s->fd = -1;
    for (rp = result; rp != NULL; rp = rp->ai_next)
    {
	s->fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
	if (s->fd < 0) continue;

	if (connect(s->fd, rp->ai_addr, rp->ai_addrlen) != -1)
	    break;                  // Success

	close(s->fd); s->fd = -1;
    }

    if (rp == NULL)	// No address succeeded
    {
	syslog(LOG_ERR, "%s(): Could not connect to %s", __FUNCTION__, s->name);
        fail();
    }

    freeaddrinfo(result);	// No longer needed
}


size_t socket_send(socket_t *s, const char *buf, size_t len)
{
    ssize_t rc = send(s->fd, buf, len, MSG_NOSIGNAL);
    if (rc < 0)
    {
	syslog(LOG_ERR, "Error on sending %zu bytes to %s: %m", len, s->name);
        fail();
    }
    else if (rc == 0)
    {
	syslog(LOG_ERR, "0 (?) bytes sent to %s", s->name);
        fail();
    }

    // rc > 0
    return rc;
}

size_t socket_recv(socket_t *s, char *buf, size_t len)
{
    ssize_t rc = recv(s->fd, buf, len, MSG_NOSIGNAL);
    if (rc < 0)
    {
	syslog(LOG_ERR, "Error received from %s: %m", s->name);
        fail();
    }

    // rc >= 0
    return rc;
}

void socket_send_all(socket_t *s, const char *buf, size_t len)
{
    do
    {
	size_t sent_len = socket_send(s, buf, len);
	buf += sent_len;
        len -= sent_len;
    } while(len > 0);
}

