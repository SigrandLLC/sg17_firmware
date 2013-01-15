#define _GNU_SOURCE
#include <signal.h> // sigaction
#include <dferror.h>
#include <errno.h>  // errno
#include <string.h> // strsignal
#include <stdlib.h> // EXIT_FAILURE
#include <x.h>

void xsigaction(int signum, const struct sigaction *act)
{
    int rc = sigaction(signum, act, NULL);
    if (rc < 0)
	dferror(EXIT_FAILURE, errno, "sigaction failed for %s", strsignal(signum));
}
