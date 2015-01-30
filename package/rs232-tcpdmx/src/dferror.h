/*
	error() via dflog

	(C) 2012 Dmitry A. Fedorov <dm.fedorov@gmail.com>
	Copying policy: GNU LGPL
*/

#ifndef  DFERROR_H
# define DFERROR_H


# include <stdlib.h>   // EXIT_SUCCESS, EXIT_FAILURE
# include <errno.h>    // errno


// if ERRNUM is nonzero, follow it with ": " and strerror (ERRNUM).
// If STATUS is nonzero, terminate the program with `exit (STATUS)'.  */
//
void dferror (int status, int errnum, const char *format, ...)
     __attribute__ ((__format__ (__printf__, 3, 4)));


#endif //DFERROR_H
