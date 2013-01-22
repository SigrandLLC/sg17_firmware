#ifndef  RS232_TCPEXT_MISC_H
# define RS232_TCPEXT_MISC_H

# include <x.h> // xmalloc, xrealloc, xstrdup, xsigaction, xopen

void fail(void);

void onexit(void (*function)(int, void *), void *arg);

static inline void* deconst(const void* arg)
{
    union { const void *cvp; void *vp; } u;
    u.cvp = arg;
    return u.vp;
}

void setup_sighandler(void (*sighandler)(int), int signal);


#endif //RS232_TCPEXT_MISC_H
