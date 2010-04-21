#ifndef  RS232_IP_EXTENDER_UTILS_H
# define RS232_IP_EXTENDER_UTILS_H

extern const char *progname;

void fail(void);

void make_pidfile(const char *pidfile);

void   lock(const char *lck_file);
void unlock(const char *lck_file);

int   open_tty(const char *device);
void close_tty(int devfd);


#endif //RS232_IP_EXTENDER_UTILS_H
