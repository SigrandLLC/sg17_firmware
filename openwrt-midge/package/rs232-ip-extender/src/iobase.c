#include "sys_headers.h"
#include "iobase.h"
#include "misc.h"


iobase_t* iobase_create(void)
{
    iobase_t *b = xzmalloc(sizeof(iobase_t));

    b->fd = -1;

    return b;
}

void iobase_delete(iobase_t* b)
{
    iobase_close(b);
    free(b);
}

void iobase_open(iobase_t* b, const char *name, int fd)
{
    iobase_close(b);
    b->name = xstrdup(name);
    b->fd   = fd;
}

void iobase_close(iobase_t* b)
{
    close(b->fd);  b->fd   = -1;
    free(b->name); b->name = NULL;
}

