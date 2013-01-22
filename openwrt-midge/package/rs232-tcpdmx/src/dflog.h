/*
	logging to stderr and syslog

	(C) 2012 Dmitry A. Fedorov <dm.fedorov@gmail.com>
	Copying policy: GNU LGPL
*/

#ifndef  DFLOG_H
# define DFLOG_H

# include <stdarg.h> // va_list
# include <syslog.h>
# include <sys/types.h> // size_t


// dflog_open flags
enum
{
    DFLOG_STDERR = 1,	// output to stderr
    DFLOG_SYS    = 2,	// output to syslog
    DFLOG_PID    = 4,	// Include PID with each message
};

void dflog_open (const char *ident, int flags);
void dflog_close(void);

// use level LOG_NO to not change

void vdflog_(int level, const char *format, va_list ap); // append log buffer and flush if \n
void  dflog_(int level, const char *format, ...)         // append log buffer and flush if \n
     __attribute__ ((__format__ (__printf__, 2, 3)));

void dflog_flush(int level); // output log buffer

void dflog(int level, const char *format, ...) // append and flush log buffer
     __attribute__ ((__format__ (__printf__, 2, 3)));


void dflog_multiline(int level, const char *pfx, const char *s, size_t n);

char *dfbn(const char *fname); // basename


#endif //DFLOG_H
