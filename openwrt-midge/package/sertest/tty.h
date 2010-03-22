/*
	<dfc/tty.h>
	$Id: tty.h,v 1.14 2006-05-21 10:13:23 df Exp $

	getch(), kbhit() etc.

	(C) 1999 Dmitry A. Fedorov <dm.fedorov@gmail.com>
	Copying policy: GNU LGPL
*/

#ifndef  _DFC_TTY_H
# define _DFC_TTY_H

# ifdef __cplusplus
   extern "C" {
# endif


struct termios;


/* set noncanonical input tty mode (w/o echo and <cr> waiting) */
int tty_set_char_input_mode(int fd, int wait_flag,
	struct termios* saved_tattr);

/* restore saved tty modes */
int tty_restore_attr(int fd, struct termios* saved_tattr);


/*
returns:
	 0 - there are no characters in input queue;
	>0 - there are characters in input queue;
	<0 - error, see errno
*/
int kbhit(int fd);



int fcheck_key(int fd, int wait_flag);	/* main routine */
int check_key(int wait_flag);

extern inline int fwait_key(int fd) { return fcheck_key(fd, (~0)); }
extern inline int wait_key(void)    { return check_key(~0); }


# ifdef __cplusplus
   }
# endif

#endif	/* _DFC_TTY_H */

