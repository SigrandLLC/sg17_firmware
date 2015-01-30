#include <dferror.h>
#include <stdarg.h> // va_list
#include <dflog.h>  // dflog*
#include <string.h> // strerror
#include <stdlib.h> // exit

// if ERRNUM is nonzero, follow it with ": " and strerror (ERRNUM).
// If STATUS is nonzero, terminate the program with `exit (STATUS)'.  */
//
void dferror (int status, int errnum, const char *format, ...)
{
    va_list ap;

    va_start(ap, format);
    vdflog_(LOG_ERR, format, ap);
    va_end  (ap);

    if (errnum)
	dflog_(LOG_ERR, ": %s", strerror(errnum));

    dflog_flush(LOG_ERR);

    if (status)
	exit(status);
}
