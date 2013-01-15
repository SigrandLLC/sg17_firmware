#include "worker.h"
#include <socket.h>
#include <misc.h> // fail
#include <dferror.h>
#include <dflog.h>
#include <string.h> // memset
#include <poll.h>
#include <write_all.h>
#include <unistd.h> // read
#include <limits.h> // PIPE_BUF

// POLLRDHUP since Linux 2.6.17
#ifndef  POLLRDHUP
# define POLLRDHUP 0
#endif


void worker_init(worker_t *w)
{
    memset(w, 0, sizeof(*w));

    w->child_pid  = -1;

    w-> data_wr_p = -1;
    w-> data_rd_p = -1;
    w->state_wr_p = -1;
    w->state_rd_p = -1;
}

static void worker_run(worker_t *w, const char *host, const char *port,
		       int ip_tos, size_t restart_delay);

void worker_start(worker_t *w, const char *host, const char *port,
		  int ip_tos, size_t restart_delay)
{
    worker_init(w);

    pid_t pid = pipe4_fork(&w-> data_wr_p, &w-> data_rd_p,
			   &w->state_wr_p, &w->state_rd_p);
    if (pid < 0)
    {
	dferror(EXIT_SUCCESS, errno, "pipe4_fork() failed");
	fail();
    }

    if (pid != 0)   // parent
    {
	w->child_pid = pid;
    }
    else            // child
    {
	dflog_open("rs232-tcpdmx  worker", DFLOG_PID|DFLOG_SYS);
	dflog(LOG_WARNING, "started up for %s:%s", host, port);

	w-> data_s = socket_create();
	w->state_s = socket_create();

	worker_run(w, host, port, ip_tos, restart_delay);
    }
}

static void
worker_run(worker_t *w, const char *host, const char *port, int ip_tos,
	   size_t restart_delay)
{
    static const size_t   DATA_BUF_SIZE = PIPE_BUF/2;
    static const size_t  STATE_BUF_SIZE = PIPE_BUF/8;

    char  data_buf[ DATA_BUF_SIZE];
    char state_buf[STATE_BUF_SIZE];

    do // restart loop
    {
	// connect
	dflog(LOG_INFO, "Connecting (data) to %s:%s ...", host, port);
	if ( socket_connect(w->data_s, host, port) )
	    goto restart;

	dflog(LOG_INFO, "Connecting (state) to %s:%s ...", host, port);
	if ( socket_connect(w->state_s, host, port) )
	    goto restart;

	socket_set_ip_tos(w-> data_s, ip_tos);
	socket_set_ip_tos(w->state_s, ip_tos);


	enum
	{
	    data_s_idx, state_s_idx,
	    data_p_idx, state_p_idx,
	    poll_items
	};

	struct pollfd polls[poll_items];
	memset(polls, 0, sizeof(polls));

	polls[ data_s_idx].fd = socket_fd(w-> data_s);
	polls[state_s_idx].fd = socket_fd(w->state_s);
	polls[ data_p_idx].fd =           w-> data_rd_p;
	polls[state_p_idx].fd =           w->state_rd_p;

	polls[ data_s_idx].events = POLLIN | POLLRDHUP;
	polls[state_s_idx].events = POLLIN | POLLRDHUP;
	polls[ data_p_idx].events = POLLIN | POLLRDHUP;
	polls[state_p_idx].events = POLLIN | POLLRDHUP;


	do // poll loop
	{
	    int rc = poll(polls, poll_items, -1);
	    if (rc < 0)
	    {
		dferror(EXIT_SUCCESS, errno, "poll failed");
		goto restart;
	    }
	    else if (rc == 0)	// timeout
	    {
		dferror(EXIT_SUCCESS, 0, "unexpected poll timeout");
		goto restart;
	    }
	    else // poll revents handling
	    {
		if (polls[data_s_idx].revents & POLLRDHUP)
		{
		    dflog(LOG_ERR, "Data peer %s closed connection",
			  socket_name(w->data_s));
		    goto restart;
		}
		if (polls[data_s_idx].revents & POLLIN)
		{
		    ssize_t r = socket_recv(w->data_s, data_buf, DATA_BUF_SIZE);
		    if (r < 0)
			goto restart;
		    else if (r == 0)
		    {
			dflog(LOG_ERR, "Data connection: EOF received from %s",
			      socket_name(w->data_s));
			goto restart;
		    }
		    else
		    {
			if (write_all(w->data_wr_p, data_buf, r))
			{
			    dferror(EXIT_SUCCESS, errno, "Error writing to data pipe");
			    fail();
			}
		    }
		}

		if (polls[state_s_idx].revents & POLLRDHUP)
		{
		    dflog(LOG_ERR, "State peer %s closed connection",
			  socket_name(w->state_s));
		    goto restart;
		}
		if (polls[state_s_idx].revents & POLLIN)
		{
		    ssize_t r = socket_recv(w->state_s, state_buf, STATE_BUF_SIZE);
		    if (r < 0)
			goto restart;
		    else if (r == 0)
		    {
			dflog(LOG_ERR, "State connection: EOF received from %s",
			      socket_name(w->state_s));
			goto restart;
		    }
		    else
		    {
			if (write_all(w->state_wr_p, state_buf, r))
			{
			    dferror(EXIT_SUCCESS, errno, "Error writing to state pipe");
			    fail();
			}
		    }
		}

		if (polls[data_p_idx].revents & POLLIN)
		{
		    ssize_t r = read(w->data_rd_p, data_buf, DATA_BUF_SIZE);
		    if (r < 0)
		    {
			dferror(EXIT_SUCCESS, errno, "Error reading from data pipe");
			fail();
		    }
		    else if (r == 0)
		    {
			dflog(LOG_WARNING, "EOF readed from data pipe, finish");
			exit(EXIT_SUCCESS);
		    }
		    else	// r > 0
		    {
			if (socket_send_all(w->data_s, data_buf, r))
			    goto restart;
		    }
		}

		if (polls[state_p_idx].revents & POLLIN)
		{
		    ssize_t r = read(w->state_rd_p, state_buf, STATE_BUF_SIZE);
		    if (r < 0)
		    {
			dferror(EXIT_SUCCESS, errno, "State pipe");
			fail();
		    }
		    else if (r == 0)
		    {
			dflog(LOG_WARNING, "EOF readed from parent state, finish");
			exit(EXIT_SUCCESS);
		    }
		    else	// r > 0
		    {
			if (socket_send_all(w->state_s, state_buf, r))
			    goto restart;
		    }
		}

	    } // poll revents handling
	} while(1); // poll loop

    restart:

	socket_close(w-> data_s);
	socket_close(w->state_s);

	if (restart_delay != 0)
	    usleep(restart_delay * 1000);

	dflog(LOG_WARNING, "restart");

    } while(1); // restart loop
}
