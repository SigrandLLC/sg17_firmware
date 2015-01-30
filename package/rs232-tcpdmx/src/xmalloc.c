/*
	<dfc/xmalloc.c>
	$Id: xmalloc.c,v 1.13 2006-05-21 10:13:23 df Exp $

	malloc(3),realloc(3) wrappers.
	Never returns NULL.

	(C) 1999 Dmitry A. Fedorov <dm.fedorov@gmail.com>
	Copying policy: GNU LGPL
*/

#include <x.h>
#include <unistd.h>	/* STDERR_FILENO        */


static void __default_xmalloc_handler(void) __attribute__((noreturn));

static void __default_xmalloc_handler(void)
{
    static const char msg[] = "\nx{m,re}alloc(): No memory\n";
    write(STDERR_FILENO, msg, sizeof(msg));
    exit(EXIT_FAILURE);	/* exit(3) can be called from here */
}

static vfp __xmalloc_handler = __default_xmalloc_handler;


void* xmalloc(size_t size)
{
    void* ptr;

    if (size==0) return NULL;

    while ((ptr=malloc(size))==NULL)
	__xmalloc_handler();

    return ptr;
}


void* xrealloc(void* oldptr, size_t size)
{
    void* ptr;

    if (size==0)
    {
	free(oldptr);
	return NULL;
    }

    while ((ptr=realloc(oldptr,size))==NULL)
	__xmalloc_handler();

    return ptr;
}


vfp set_xmalloc_handler(vfp xmalloc_handler)
{
    vfp old_handler = __xmalloc_handler;

    __xmalloc_handler = xmalloc_handler ? : __default_xmalloc_handler;

    return old_handler;
}
