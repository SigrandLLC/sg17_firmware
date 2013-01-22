#include <write_all.h>
#include <unistd.h> // write
#include <errno.h>
#include <sched.h> // sched_yield


ssize_t write_all(int fd, const void *_buf, size_t count)
{
    const char* buf = _buf;
    int first_run = 1;

    do
    {
	ssize_t rc = write(fd, buf, count);
	if (rc < 0)
	{
#if defined EWOULDBLOCK && EWOULDBLOCK != EAGAIN
	    if ( errno == EAGAIN || errno == EWOULDBLOCK )
#else
	    if ( errno == EAGAIN )
#endif
		continue;
	    else
		return rc;
	}

	count -= rc;
	buf += rc;

	if (!first_run) sched_yield();
	first_run = 0;

    } while (count > 0);

    return 0;
}
