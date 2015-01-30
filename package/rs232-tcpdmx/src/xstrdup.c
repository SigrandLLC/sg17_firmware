/*
	<dfc/xstrdup.c>
	$Id: xstrdup.c,v 1.12 2006-05-21 10:13:23 df Exp $

	xstrdup - strdup with xmalloc, never returns NULL.

	(C) 1999 Dmitry A. Fedorov <dm.fedorov@gmail.com>
	Copying policy: GNU LGPL
*/

#include <x.h>
#include <string.h>		/* strlen, memcpy */


char* xstrdup(const char* src)
{
    size_t len = strlen(src) + 1;
    return memcpy(xmalloc(len), src, len);
}
