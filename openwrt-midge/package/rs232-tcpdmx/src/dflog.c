#include <dflog.h>
#include <syslog.h>
#include <stdio.h>
#include <string.h> // strdup
#include <stdlib.h> // free
#include <pthread.h>
#include <unistd.h> // getpid
#include <errno.h>
#include <x.h>

#define DFLOGBUF_SIZE 4096

static char *ident        = NULL;
static int   flags        = DFLOG_STDERR | DFLOG_SYS | DFLOG_PID;

static pthread_key_t  key;
static pthread_once_t key_once = PTHREAD_ONCE_INIT;

typedef struct
{
    char *buf;
    char *ptr;
} dflog_buf_t;

static dflog_buf_t *dflog_buf_create(void)
{
    dflog_buf_t *b = xmalloc(sizeof(*b));
    memset(b, 0, sizeof(*b));

    b->buf = xmalloc(DFLOGBUF_SIZE);
    memset(b->buf, 0,DFLOGBUF_SIZE);

    b->ptr = b->buf;

    return b;
}

static void dflog_buf_delete(dflog_buf_t *b)
{
    free(b->buf);
    free(b);
}


static void dflog_key_destroy(void *p)
{
    dflog_buf_delete(p);
}

static void dflog_key_init(void)
{
    pthread_key_create(&key, dflog_key_destroy);
}

// init once, return thread-specific buffer
static dflog_buf_t *dflog_init(void)
{
    dflog_buf_t *b;

    pthread_once(&key_once, dflog_key_init);

    if ((b = pthread_getspecific(key)) == NULL)
    {
	b = dflog_buf_create();
	pthread_setspecific(key, b);
    }

    return b;
}


void dflog_open(const char *_ident, int _flags)
{
    dflog_close();

    if (_ident != NULL)
	ident = strdup(_ident);

    if ((_flags & (DFLOG_STDERR | DFLOG_SYS)) == 0)
	 _flags =  DFLOG_STDERR;

    flags = _flags;

    if (_flags & DFLOG_SYS)
	openlog(_ident, _flags & DFLOG_PID ? LOG_PID : 0, LOG_USER);

    if (_flags & DFLOG_STDERR)
    {
	fflush(stderr);
	setlinebuf(stderr);
    }
}

void dflog_close(void)
{
    dflog_flush(LOG_ERR);

    if (flags & DFLOG_SYS)
	closelog();

    if (flags & DFLOG_STDERR)
	fflush(stderr);

    if (ident != NULL)
    {
	free(ident);
	ident = NULL;
    }
}

// output log buffer
void dflog_flush(int level)
{
    dflog_buf_t *b = dflog_init();

    if (b->buf[0] == '\0')
	return;

    char *p = strrchr(b->buf, '\n');
    if (p != NULL)
	*p = '\0';

    if (flags & DFLOG_SYS)
	syslog(level, b->buf);

    if (flags & DFLOG_STDERR)
    {
	if (ident != NULL && ident[0] != '\0')
	{
	    if (flags & DFLOG_PID)
		fprintf(stderr, "%s[%d]: ", ident, getpid());
	    else
		fprintf(stderr, "%s: ", ident);
	}
	else
	{
	    if (flags & DFLOG_PID)
		fprintf(stderr, "[%d]: ", getpid());
	}
	fputs(b->buf, stderr);
	fputc('\n'  , stderr);
	fflush(stderr);
    }

    b->buf[0] = '\0';
    b->ptr = b->buf;
}

// append log buffer and flush if \n
void vdflog_(int level, const char *format, va_list ap)
{
    dflog_buf_t *b = dflog_init();

    int rc = vsnprintf(b->ptr, DFLOGBUF_SIZE - (b->ptr - b->buf) - 1, format, ap);
    if (rc > 0)
    {
	b->ptr += rc;
	*b->ptr = '\0';
    }

    if (strrchr(b->buf, '\n') != NULL)
	dflog_flush(level);
}

// append log buffer and flush if \n
void dflog_(int level, const char *format, ...)
{
    int err = errno;
    va_list ap;

    va_start(ap, format);
    vdflog_(level, format, ap);
    va_end  (ap);
    errno = err;
}

// append and flush log buffer
void dflog(int level, const char *format, ...)
{
    int err = errno;
    va_list ap;

    va_start(ap, format);
    vdflog_(level, format, ap);
    va_end  (ap);

    dflog_flush(level);
    errno = err;
}
