#include "sys_headers.h"
#include "misc.h"

const char *progname = "rs232-ip-extender";

void fail(void)
{
    syslog(LOG_ERR, "failed, exiting");
    exit(EXIT_FAILURE);
}


void onexit(void (*function)(int, void *), void *arg)
{
    int rc = on_exit(function, arg);
    if (rc < 0)
    {
	syslog(LOG_ERR, "on_exit(3) failed: %m");
        fail();
    }
}

void *xmalloc(size_t size)
{
    void *ret = malloc(size);

    if (ret == NULL)
    {
	syslog(LOG_ERR, "%s(): Can't allocate %zu bytes: %m", __FUNCTION__, size);
        fail();
    }

    return ret;
}

void *xzmalloc(size_t size)
{
    void *ret = xmalloc(size);
    memset(ret, 0, size);
    return ret;
}

char *xstrdup(const char *src)
{
    char *ret = strdup(src);

    if (ret == NULL)
    {
	syslog(LOG_ERR, "strdup(3) failed: %m");
        fail();
    }

    return ret;
}

void setup_sighandler(void (*sighandler)(int), int signal)
{
    struct sigaction act;
    act.sa_handler = sighandler;
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_RESTART;
    int rc = sigaction(signal, &act, NULL);
    if (rc)
    {
	syslog(LOG_ERR, "sigaction error: %m");
        fail();
    }
}

