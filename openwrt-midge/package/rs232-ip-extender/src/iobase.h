#ifndef  RS232_IP_EXTENDER_IOBASE_H
# define RS232_IP_EXTENDER_IOBASE_H


typedef struct
{
    char *name;
    int fd;
} iobase_t;

iobase_t* iobase_create   (void);
void      iobase_delete   (iobase_t* b);
void      iobase_open     (iobase_t* b, const char *name, int fd);
void      iobase_close    (iobase_t* b);
size_t    iobase_read     (iobase_t *b,       char *buf, size_t len);
size_t    iobase_write    (iobase_t *b, const char *buf, size_t len);
void      iobase_write_all(iobase_t *b, const char *buf, size_t len);


#endif //RS232_IP_EXTENDER_IOBASE_H

