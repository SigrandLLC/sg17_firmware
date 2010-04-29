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


size_t iobase_read(iobase_t *b, char *buf, size_t len)
{
    ssize_t rc = read(b->fd, buf, len);
    if (rc < 0)
    {
	syslog(LOG_ERR, "Error readed from %s: %m", b->name);
        fail();
    }

    // rc >= 0
    return rc;
}

size_t iobase_write(iobase_t *b, const char *buf, size_t len)
{
    ssize_t rc = write(b->fd, buf, len);
    if (rc < 0)
    {
	syslog(LOG_ERR, "Error on writing %zu bytes to %s: %m", len, b->name);
        fail();
    }
    else if (rc == 0)
    {
	syslog(LOG_ERR, "0 (?) bytes written to %s", b->name);
        fail();
    }

    // rc > 0
    return rc;
}

void iobase_write_all(iobase_t *b, const char *buf, size_t len)
{
    do
    {
	size_t wrote_len = iobase_write(b, buf, len);
	buf += wrote_len;
        len -= wrote_len;
    } while(len > 0);
}

