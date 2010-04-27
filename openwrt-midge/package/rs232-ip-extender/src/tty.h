#ifndef  RS232_IP_EXTENDER_TTY_H
# define RS232_IP_EXTENDER_TTY_H


typedef struct
{
    char *name;
    int fd;
    int locked;
    struct termios termios;
} tty_t;

tty_t *tty_create(const char *devname);
void   tty_delete(tty_t *tty);

void   tty_lock   (tty_t *tty);
void   tty_unlock (tty_t *tty);

void   tty_open   (tty_t *tty);
void   tty_close  (tty_t *tty);

void   tty_set_raw(tty_t *tty);
void   tty_restore(tty_t *tty);


#endif //RS232_IP_EXTENDER_TTY_H
