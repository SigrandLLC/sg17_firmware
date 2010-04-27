#ifndef  RS232_IP_EXTENDER_TTY_H
# define RS232_IP_EXTENDER_TTY_H

# include "iobase.h"


typedef struct
{
    iobase_t *b;
    struct termios termios;
} tty_t;

tty_t *tty_create   (void);
void   tty_delete   (tty_t *t);

void   tty_open     (tty_t *t, const char *devname);
void   tty_close    (tty_t *t);

void   tty_set_raw  (tty_t *t);
void   tty_restore  (tty_t *t);


extern inline const char *tty_name(tty_t *t) { return iobase_name(t->b); }
extern inline int         tty_fd  (tty_t *t) { return iobase_fd  (t->b); }


extern inline size_t tty_read     (tty_t *t,       char *buf, size_t len)
	{ return iobase_read (t->b, buf, len); }

extern inline size_t tty_write    (tty_t *t, const char *buf, size_t len)
	{ return iobase_write(t->b, buf, len); }

extern inline void   tty_write_all(tty_t *t, const char *buf, size_t len)
	{        iobase_write(t->b, buf, len); }


#endif //RS232_IP_EXTENDER_TTY_H
