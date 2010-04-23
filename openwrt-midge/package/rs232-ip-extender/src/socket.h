#ifndef  RS232_IP_EXTENDER_SOCKET_H
# define RS232_IP_EXTENDER_SOCKET_H


typedef struct socket_s
{
    int fd;
    char *host;
    char *port;
} socket_t;

socket_t *socket_create (void);
void      socket_delete (socket_t *s);

void      socket_bind   (socket_t *s, const char *host, const char *port);
socket_t *socket_accept (socket_t *s);
void      socket_connect(socket_t *s, const char *host, const char *port);
void      socket_close  (socket_t *s);


#endif //RS232_IP_EXTENDER_SOCKET_H
