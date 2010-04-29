#include "sys_headers.h"
#include "pidfile.h"
#include "misc.h"

static int pidfile_created = 0;

static void rm_pidfile__(int unused, void *pidfile)
{
    (void)unused;
    rm_pidfile(pidfile);
}

void make_pidfile(const char *pidfile)
{
    onexit(rm_pidfile__, deconst(pidfile));

    FILE *fpidfile = fopen(pidfile, "wx");
    if ( !fpidfile)
    {
        pidfile_created = 0;
	syslog(LOG_ERR, "Error opening pid file '%s': %m", pidfile);
	fail();
    }
    pidfile_created = 1;
    fprintf(fpidfile, "%d\n", getpid());
    fclose (fpidfile);
    syslog(LOG_INFO, "pid file '%s' created", pidfile);
}

void rm_pidfile(const char *pidfile)
{
    if (pidfile_created)
    {
	syslog(LOG_INFO, "unlinking pid file '%s'...", pidfile);
	int rc = unlink(pidfile);
	if (rc < 0)
	    syslog(LOG_WARNING, "Error unlinking pid file %s: %m", pidfile);
    }
    else
    {
	syslog(LOG_WARNING, "pid file %s was not created by this process (%d), nothing to unlink",
	       pidfile, getpid());
    }
}

