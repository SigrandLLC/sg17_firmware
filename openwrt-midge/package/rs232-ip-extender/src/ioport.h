#ifndef  RS232_IP_EXTENDER_IOPORT_H
# define RS232_IP_EXTENDER_IOPORT_H


struct ioport;
struct ioport_vmt;

typedef struct ioport
{
    struct ioport_vmt *vmt;

    char *name;
    int fd;

} ioport_t;

typedef struct ioport_vmt
{
    ioport_t* (*create  )(void);
    void      (*delete  )(ioport_t *port);

    void      (*open    )(ioport_t *port, const char *name);
    void      (*close   )(ioport_t *port);

    size_t    (*send    )(ioport_t *port, const char *buf, size_t len);
    void      (*send_all)(ioport_t *port, const char *buf, size_t len);
    size_t    (*recv    )(ioport_t *port,       char *buf, size_t len);

} ioport_vmt_t;


#endif //RS232_IP_EXTENDER_IOPORT_H

