#include "sys_headers.h"
#include "pidfile.h"
#include "utils.h"

static void rm_pidfile__(int unused, void *pidfile)
{
    (void)unused;
    rm_pidfile(pidfile);
}

void make_pidfile(const char *pidfile)
{
    onexit(rm_pidfile__, deconst(pidfile));

    FILE *fpidfile = fopen(pidfile, "w");
    if ( !fpidfile)
    {
	syslog(LOG_ERR, "Error opening pidfile '%s': %m", pidfile);
        fail();
    }
    fprintf(fpidfile, "%d\n", getpid());
    fclose (fpidfile);
}

void rm_pidfile(const char *pidfile)
{
    int rc = unlink(pidfile);
    if (rc < 0)
	syslog(LOG_WARNING, "Error unlinking pid file %s: %m", pidfile);
}

