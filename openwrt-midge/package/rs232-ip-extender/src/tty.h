#ifndef  RS232_IP_EXTENDER_TTY_H
# define RS232_IP_EXTENDER_TTY_H


typedef struct
{
    char *name;
    int fd;
    int locked;
    struct termios termios;
} tty_t;

tty_t *tty_create(void);
void   tty_delete(tty_t *tty);

void   tty_open   (tty_t *tty, const char *devname);
void   tty_close  (tty_t *tty);

void   tty_set_raw(tty_t *tty);
void   tty_restore(tty_t *tty);


#endif //RS232_IP_EXTENDER_TTY_H
