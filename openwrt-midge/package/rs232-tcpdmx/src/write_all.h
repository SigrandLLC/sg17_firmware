#ifndef  _COMMON_WRITE_ALL_H
# define _COMMON_WRITE_ALL_H

# include <sys/types.h> // size_t, ssize_t

ssize_t write_all(int fd, const void *buf, size_t count);

#endif	// _COMMON_WRITE_ALL_H
