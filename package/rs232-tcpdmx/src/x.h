/*
	<x.h>

	malloc(3),realloc(3) wrappers.
	Never returns NULL.

	xstrdup - strdup with xmalloc, never returns NULL.

	xsigaction - sigaction wrapper.

	xopen - open(2) wrapper

	(C) 1999-2012 Dmitry A. Fedorov <dm.fedorov@gmail.com>
	Copying policy: GNU LGPL
*/

#ifndef  _DFC_X_H
# define _DFC_X_H

# include <stddef.h>	// size_t
# include <stdlib.h>	// free
# include <signal.h>	// struct sigaction
# include <fcntl.h>	// open, flags, mode_t


void*  xmalloc(size_t size);
void* xzmalloc(size_t size);
void* xrealloc(void* ptr, size_t size);

/* User's no-memory exception handler.            */
/* Calls in loop until suscessful memory request. */
typedef void (*vfp)(void);

/* Sets new no-memory exception handler.         */
/* Returns old handler.                          */
/* If new handler is NULL, sets default handler, */
/* its write message to stderr and do exit(2)    */
vfp set_xmalloc_handler(vfp xmalloc_handler);

char* xstrdup(const char* src);


void xsigaction(int signum, const struct sigaction *act);


int xopen(const char *path, int flags, mode_t mode);


void xioctl(const char *pfx, int fd, int request, void *arg);


#endif	// _DFC_X_H
