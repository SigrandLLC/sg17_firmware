#ifndef  RS232_TCPEXT_IOBASE_H
# define RS232_TCPEXT_IOBASE_H


typedef struct
{
    char *name;
    int fd;
} iobase_t;

iobase_t* iobase_create   (void);
void      iobase_delete   (iobase_t* b);
void      iobase_open     (iobase_t* b, const char *name, int fd);
void      iobase_close    (iobase_t* b);
ssize_t   iobase_read     (iobase_t *b,       char *buf, size_t len);
ssize_t   iobase_write    (iobase_t *b, const char *buf, size_t len);
int       iobase_write_all(iobase_t *b, const char *buf, size_t len);

extern inline const char *iobase_name(iobase_t *b) { return b->name; }
extern inline int         iobase_fd  (iobase_t *b) { return b->fd  ; }


#endif //RS232_TCPEXT_IOBASE_H

