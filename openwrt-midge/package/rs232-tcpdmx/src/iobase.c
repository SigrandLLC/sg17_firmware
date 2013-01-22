#include "sys_headers.h"
#include "iobase.h"
#include "misc.h"
#include <dferror.h>


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


ssize_t iobase_read(iobase_t *b, char *buf, size_t len)
{
    ssize_t rc = read(b->fd, buf, len);
    if (rc < 0)
	dferror(EXIT_SUCCESS, errno, "Error readed from %s", b->name);

    return rc;
}

ssize_t iobase_write(iobase_t *b, const char *buf, size_t len)
{
    ssize_t rc = write(b->fd, buf, len);
    if (rc < 0)
	dferror(EXIT_SUCCESS, errno, "Error on writing %zu bytes to %s", len, b->name);
    else if (rc == 0)
    {
	dferror(EXIT_SUCCESS, 0, "0 (?) bytes written to %s", b->name);
	rc = -1;
    }

    return rc;
}

int iobase_write_all(iobase_t *b, const char *buf, size_t len)
{
    do
    {
	ssize_t wrote_len = iobase_write(b, buf, len);
	if (wrote_len < 0) return wrote_len;
	buf  += wrote_len;
        len  -= wrote_len;
    } while(len > 0);

    return 0;
}
