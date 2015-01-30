#ifndef  _COMMON_PIPE_FORK_H
# define _COMMON_PIPE_FORK_H


// fork new process assigning descriptors of two communicating pipes
// for parent and child.
// returns pid from fork(2) or -1 on error.
int pipe2_fork(int *pipe_wr, int *pipe_rd);

// fork new process assigning descriptors of four communicating pipes
// for parent and child.
// returns pid from fork(2) or -1 on error.
int pipe4_fork(int *pipe1_wr, int *pipe1_rd, int *pipe2_wr, int *pipe2_rd);


#endif	// _COMMON_PIPE_FORK_H
