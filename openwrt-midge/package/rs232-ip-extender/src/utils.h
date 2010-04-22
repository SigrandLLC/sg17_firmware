#ifndef  RS232_IP_EXTENDER_UTILS_H
# define RS232_IP_EXTENDER_UTILS_H

extern const char *progname;

void fail(void);

void onexit(void (*function)(int, void *), void *arg);

void make_pidfile(const char *pidfile);

void   lock(const char *device);
void unlock(const char *device);

int   open_tty(const char *device);
void close_tty(int devfd);


#endif //RS232_IP_EXTENDER_UTILS_H
