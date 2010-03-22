/*
	<dfc/tty.c>
	$Id: tty.c,v 1.16 2006-05-21 10:13:23 df Exp $

	getch() etc.

	(C) 1999 Dmitry A. Fedorov <dm.fedorov@gmail.com>
	Copying policy: GNU LGPL
*/

#include <unistd.h>	/* isatty(), STDIN_FILENO      */
#include <stdio.h>	/* fflush(stdout), EOF         */
#include <errno.h>	/* errno                       */
#include <termios.h>	/* struct termios, tc?etattr() */
#include <sys/time.h>	/* struct timeval, select()    */
#include <string.h>	/* memcpy                      */
#include "./tty.h"


/* set noncanonical input tty mode (w/o echo and <cr> waiting) */
int tty_set_char_input_mode(int fd, int wait_flag,
			    struct termios* saved_tattr)
{
    struct termios tattr;
    int rc=0;

    if (isatty(fd))
    {
	rc=tcgetattr(fd, &tattr);
	if (rc!=0) return rc;

	if (saved_tattr!=NULL)
	    memcpy(saved_tattr, &tattr, sizeof(tattr));

	tattr.c_lflag &= ~(ICANON|ECHO|ISIG);	//Clear ICANON and ECHO
	tattr.c_cc[VMIN] = wait_flag?1:0;
	tattr.c_cc[VTIME] = 0;

	rc=tcsetattr(fd, TCSANOW, &tattr);
    }

    return rc;
}


/* restore saved tty modes */
int tty_restore_attr(int fd, struct termios* saved_tattr)
{
    return isatty(fd) ? tcsetattr(fd, TCSANOW, saved_tattr) : 0;
}


/*
returns:
	 0 - there are no characters in input queue;
	>0 - there are characters in input queue;
	<0 - error, see errno
*/
int kbhit(int fd)
{
    struct termios saved_tattr;
    fd_set fdset;
    struct timeval tv  = { 0, 0 };
    int rc;

    rc=tty_set_char_input_mode(fd, 0, &saved_tattr);
    if (rc!=0) return rc;

    FD_ZERO(&fdset); FD_SET(fd, &fdset);

    rc = select(fd+1, &fdset, NULL, NULL, &tv);

    tty_restore_attr(fd, &saved_tattr);
    return rc;
}


int fcheck_key(int fd, int wait_flag)
{
    struct termios saved_tattr;
    char buf[16];
    ssize_t l;
    int rc=0;
    size_t size=isatty(fd)?sizeof(buf):1;

    rc=tty_set_char_input_mode(fd, wait_flag, &saved_tattr);
    if (rc!=0) return rc;

    l=read(fd, &buf, size);
    if (-1==l)	{ rc = -1; goto restore; }	/* error */

    if (0==l)
    {
	if (wait_flag) { errno=0; rc = -1; goto restore; }
	else           {          rc =  0; goto restore; } /* no wait && no key */
    }
    else
	rc = buf[0] & 0xFF;

restore:
    tty_restore_attr(fd, &saved_tattr);
    return rc;
}

int check_key(int wait_flag)
{
    fflush(stdout);
    return fcheck_key(STDIN_FILENO, wait_flag);
}

