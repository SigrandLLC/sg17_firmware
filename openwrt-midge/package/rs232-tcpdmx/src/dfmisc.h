/*
	(C) 2012 Dmitry A. Fedorov <dm.fedorov@gmail.com>
	Copying policy: GNU LGPL
*/

#ifndef  DFMISC_H
# define DFMISC_H


int is_fd_opened(int fd);
int is_fd_socket(int fd);

long fd_set_block    (int fd); // returns old flags or -1 on error
long fd_set_nonblock (int fd); // returns old flags or -1 on error
int  fd_restore_flags(int fd, long flags); // returns fcntl return if flags >= 0


#endif //DFMISC_H
