#ifndef  RS232_IP_EXTENDER_TTY_H
# define RS232_IP_EXTENDER_TTY_H

# include "iobase.h"


typedef struct
{
    iobase_t *b;
    struct termios termios;
} tty_t;

tty_t *tty_create (void);
void   tty_delete (tty_t *t);

void   tty_open   (tty_t *t, const char *devname);
void   tty_close  (tty_t *t);

void   tty_set_raw(tty_t *t);
void   tty_restore(tty_t *t);

extern inline const char *tty_name(tty_t *t) { return iobase_name(t->b); }
extern inline int         tty_fd  (tty_t *t) { return iobase_fd  (t->b); }


#endif //RS232_IP_EXTENDER_TTY_H
