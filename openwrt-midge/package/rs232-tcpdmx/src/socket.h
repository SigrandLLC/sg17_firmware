#ifndef  RS232_TCPEXT_SOCKET_H
# define RS232_TCPEXT_SOCKET_H

# include "iobase.h"


typedef struct
{
    iobase_t *b;
} socket_t;

socket_t *socket_create (void);
void      socket_delete (socket_t *s);

void      socket_bind    (socket_t *s, const char *host, const char *port);
socket_t *socket_accept  (socket_t *s);
int       socket_connect (socket_t *s, const char *host, const char *port);
		// returns true if "Could not connect to ...", false on success
void      socket_close   (socket_t *s);

void    socket_set_ip_tos(socket_t *s, int tos);

extern inline const char *socket_name(socket_t *s) { return iobase_name(s->b); }
extern inline int         socket_fd  (socket_t *s) { return iobase_fd  (s->b); }


extern inline ssize_t socket_send(socket_t *s, const char *buf, size_t len)
	{ return iobase_write(s->b, buf, len); }

extern inline int socket_send_all(socket_t *s, const char *buf, size_t len)
	{ return iobase_write_all(s->b, buf, len); }

extern inline ssize_t socket_recv(socket_t *s, char *buf, size_t len)
	{ return iobase_read(s->b, buf, len); }


#endif //RS232_TCPEXT_SOCKET_H
