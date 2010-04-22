#ifndef  RS232_IP_EXTENDER_TTY_RAW_H
# define RS232_IP_EXTENDER_TTY_RAW_H


struct termios;

void    lock_tty(const char *device);
void  unlock_tty(const char *device);

int     open_tty(const char *device);
void   close_tty(int ttyfd);

void set_raw_tty(int ttyfd, struct termios *tios);
void restore_tty(int ttyfd, struct termios *tios);


#endif //RS232_IP_EXTENDER_TTY_RAW_H
