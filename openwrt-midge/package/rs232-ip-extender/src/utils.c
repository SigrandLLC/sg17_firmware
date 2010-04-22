#include "sys_headers.h"
#include "utils.h"

const char *progname = "rs232-ip-extender";

void fail(void)
{
    syslog(LOG_ERR, "%s failed, exiting", progname);
    exit(EXIT_FAILURE);
}


void onexit(void (*function)(int, void *), void *arg)
{
    int rc = on_exit(function, arg);
    if (rc < 0)
    {
	syslog(LOG_ERR, "on_exit: %m");
        fail();
    }
}

