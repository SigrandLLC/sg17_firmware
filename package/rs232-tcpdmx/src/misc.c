#include "sys_headers.h"
#include "misc.h"
#include <dferror.h>

void fail(void)
{
    dferror(EXIT_SUCCESS, 0, "failed, exiting");
    exit(EXIT_FAILURE);
}


void onexit(void (*function)(int, void *), void *arg)
{
    int rc = on_exit(function, arg);
    if (rc < 0)
    {
	dferror(EXIT_SUCCESS, errno, "on_exit(3) failed");
        fail();
    }
}

void setup_sighandler(void (*sighandler)(int), int signal)
{
    struct sigaction act;
    act.sa_handler = sighandler;
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_RESTART;
    xsigaction(signal, &act);
}
