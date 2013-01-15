#include <dfmisc.h> // set_fd_block
#include <fcntl.h>


long fd_set_block(int fd) // returns old flags or -1 on error
{
    long flags = fcntl(fd, F_GETFL);
    if (flags >= 0)
       fcntl(fd, F_SETFL, flags & ~O_NONBLOCK);
    return flags;
}
