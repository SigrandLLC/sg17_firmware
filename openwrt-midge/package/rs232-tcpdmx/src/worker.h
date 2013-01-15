#ifndef  RS232_TCPDMX_WORKER_H
# define RS232_TCPDMX_WORKER_H

# include <pipe-fork.h> // pipe2_fork
# include <sys/types.h> // pid_t
# include <socket.h>


typedef struct
{
    pid_t child_pid; // for parent only

    // common parent/child
    // local parent <-> child communication pipes
    int data_wr_p, state_wr_p;
    int data_rd_p, state_rd_p;

    // for child only
    socket_t *data_s, *state_s; // remote end communication sockets

} worker_t;


void worker_init(worker_t *w);

void worker_start(worker_t *w, const char *host, const char *port,
		  int ip_tos, size_t restart_delay);


#endif //RS232_TCPDMX_WORKER_H
