#ifndef  RS232_IP_EXTENDER_TTY_H
# define RS232_IP_EXTENDER_TTY_H


typedef struct tty_descr
{
    const char *name;
    int fd;
    struct termios termios;
} tty_descr_t;

tty_descr_t *tty_create(const char *devname);
void         tty_delete(tty_descr_t *tty);
int          tty_open  (tty_descr_t *tty);
void         tty_close (tty_descr_t *tty);
void         tty_lock  (tty_descr_t *tty);
void         tty_unlock(tty_descr_t *tty);


#endif //RS232_IP_EXTENDER_TTY_H
