#include "sys_headers.h"
#include "socket.h"
#include "misc.h"
#include <netinet/ip.h> // IPTOS_TOS_MASK
#include <netinet/tcp.h> // TCP_CORK


static void socket_on_exit(int unused, void *arg)
{
    (void)unused;
    socket_t *s = arg;
    socket_delete(s);
}

socket_t *socket_create(void)
{
    socket_t *s = xzmalloc(sizeof(*s));
    s->b = iobase_create();

    onexit(socket_on_exit, s);

    return s;
}

void socket_delete(socket_t *s)
{
    socket_close(s);
    iobase_delete(s->b); s->b = NULL;
    free(s);
}

void socket_close(socket_t *s)
{
    shutdown(socket_fd(s), SHUT_RDWR);
    iobase_close(s->b);
}

static char *make_host_port( const char *host, const char *port)
{
    size_t name_size = strlen(host) + 3 + strlen(port) + 1;
    char  *name      = xmalloc(name_size);
    snprintf(name, name_size, "%s:%s", host, port);
    return name;
}

static void set_sock_opt_int(int fd, int opt, int optval, const char *optname, const char *pfx)
{
    int rc = setsockopt(fd, SOL_SOCKET, opt, &optval, sizeof(optval));
    if (rc < 0)
    {
	syslog(LOG_ERR, "%sCould not set %s socket option: %m", pfx, optname);
	fail();
    }
}

static void set_sock_opt_bool(int fd, int opt, const char *optname, const char *pfx)
{
    set_sock_opt_int(fd, opt, 1, optname, pfx);
}

static void set_sock_linger(int fd, int timeout, const char *pfx)
{
    struct linger linger = { 1, timeout };
    int rc = setsockopt(fd, SOL_SOCKET, SO_LINGER, &linger, sizeof(linger));
    if (rc < 0)
    {
	syslog(LOG_ERR, "%sCould not set linger: %m", pfx);
	fail();
    }
}

void socket_bind(socket_t *s, const char *host, const char *port)
{
    socket_close(s);

    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family    = AF_UNSPEC;	// Allow IPv4 or IPv6
    hints.ai_socktype  = SOCK_STREAM;
    hints.ai_flags     = AI_PASSIVE;	// For wildcard IP address, INADDR_ANY | IN6ADDR_ANY_INIT
    hints.ai_protocol  = IPPROTO_TCP;

    struct addrinfo *result;
    int rc = getaddrinfo(host, port, &hints, &result);
    if (rc != 0)
    {
	syslog(LOG_ERR, "%s(): getaddrinfo(%s, %s, ...): %s", __FUNCTION__, host, port, gai_strerror(rc));
	fail();
    }

    /* getaddrinfo() returns a list of address structures.
       Try each address until we successfully bind(2).
       If socket(2) (or bind(2)) fails, we (close the socket
       and) try the next address. */

    int errno_save = 0;
    struct addrinfo *rp;
    int fd = -1;
    for (rp = result; rp != NULL; rp = rp->ai_next)
    {
	fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
	if (fd < 0) continue;

	set_sock_opt_bool(fd, SO_REUSEADDR, "reuse address", "socket_bind(): ");

	if (bind(fd, rp->ai_addr, rp->ai_addrlen) == 0)
	    break;                  // Success
        errno_save = errno;

	close(fd); fd = -1;
    }

    if (rp == NULL)	// No address succeeded
    {
        errno = errno_save;
	syslog(LOG_ERR, "%s(): Could not bind to %s:%s: %m", __FUNCTION__, host, port);
	fail();
    }

    freeaddrinfo(result);	// No longer needed

    rc = listen(fd, 2);
    if (rc < 0)
    {
	syslog(LOG_ERR, "%s(): listen(2) failed: %m", __FUNCTION__);
	fail();
    }

    iobase_open(s->b, make_host_port(host, port), fd);
}

socket_t *socket_accept(socket_t *s)
{
    struct sockaddr peer_addr;
    socklen_t       peer_addrlen = sizeof(peer_addr);
    int newfd = accept(socket_fd(s), &peer_addr, &peer_addrlen);
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
    char *name = (rc == 0)
	? make_host_port( peer_host, peer_port)
	: make_host_port( "NULL"   , "NULL"   );
    iobase_open(new_s->b, name, newfd);

    // keepalive freeze the system
    //set_sock_opt_bool(newfd, SO_KEEPALIVE, "keepalive", "socket_accept(): ");
    set_sock_linger  (newfd, 3, "socket_accept(): "); //FIXME: linger timeout

    return new_s;
}

// returns true if "Could not connect to ...", false on success
int socket_connect(socket_t *s, const char *host, const char *port)
{
    socket_close(s);

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
    int fd = -1;
    for (rp = result; rp != NULL; rp = rp->ai_next)
    {
	fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
	if (fd < 0) continue;

	if (connect(fd, rp->ai_addr, rp->ai_addrlen) != -1)
	    break;                  // Success

	close(fd); fd = -1;
    }

    freeaddrinfo(result);	// No longer needed

    if (rp == NULL)	// No address succeeded
    {
	syslog(LOG_ERR, "%s(): Could not connect to %s:%s", __FUNCTION__, host, port);
	return 1;
    }

    iobase_open(s->b, make_host_port(host, port), fd);

    // keepalive freeze the system
    //set_sock_opt_bool(fd, SO_KEEPALIVE, "keepalive", "socket_connect(): ");
    set_sock_linger  (fd, 3, "socket_connect(): "); //FIXME: linger timeout

    return 0;
}

void socket_set_ip_tos(socket_t *s, int ip_tos)
{
    //ip_tos &= IPTOS_TOS_MASK;
    ip_tos &= 0xFF;
    int rc = setsockopt(socket_fd(s), IPPROTO_IP, IP_TOS, &ip_tos, sizeof(ip_tos));
    if (rc < 0)
    {
	syslog(LOG_ERR, "Could not set IP ToS option: %m");
	fail();
    }
}

void socket_cork(socket_t *s, int onoff)
{
    int rc = setsockopt(socket_fd(s), IPPROTO_TCP, TCP_CORK, &onoff, sizeof(onoff));
    if (rc < 0)
    {
	syslog(LOG_ERR, "Could not set TCP_CORK option %s: %m",
	       onoff?"on":"off");
	fail();
    }
}
