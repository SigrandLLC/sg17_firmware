//#define NO_RS232 1
//#define DO_TICK  1

#define _GNU_SOURCE
#include "sys_headers.h"
#include "tty.h"
#include "misc.h"
#include "dferror.h"
#include "dflog.h"
#include "dfmisc.h"
#include <limits.h> // PIPE_BUF
#include "worker.h"


static void sig_handler(int sig)
{
    dflog(LOG_WARNING, "%s signal catched", strsignal(sig));
    if (sig == SIGTERM || sig == SIGCHLD)
    {
	dflog(LOG_WARNING, "exiting");
	exit(EXIT_SUCCESS);
    }
    else
	dflog(LOG_WARNING, "do nothing");
}

static ssize_t write_all_raw(int fd, const void *_buf, size_t len)
{
    const char* buf = _buf;

    do
    {
	ssize_t rc = write(fd, buf, len);
	if (rc < 0)
	    return rc;
	buf  += rc;
        len  -= rc;
    } while(len > 0);

    return 0;
}

static int wouldblock(void)
{
    int err = errno;
    return (err == EAGAIN || err == EWOULDBLOCK);
}

static int send_modem_state(int p, modem_state_t mstate)
{
#ifndef NO_RS232
    tty_log_modem_state("Send modem state: ", mstate);
    if (write_all_raw(p, (const char *)&mstate, sizeof(mstate)))
	return -1;
#else
    (void)s;
    (void)mstate;
#endif

    return 0;
}

static int get_send_new_modem_state(tty_t* t, int p, enum TTY_TYPE tty_type)
{
    modem_state_t in_mstate = 0;
#ifndef NO_RS232
    if (tty_get_modem_state(t, &in_mstate)) return -1;
#else
    (void)t;
    (void)s;
#endif
    if ( !t->last_in_mstate_valid || t->last_in_mstate != in_mstate)
    {
	modem_state_t out_mstate = 0;

	if (tty_type == TTY_DTE)
	{
	    out_mstate |= (in_mstate & TTY_MODEM_DSR);
#if 0
	    out_mstate |= (in_mstate & TTY_MODEM_CTS);
#endif
	    out_mstate |= (in_mstate & TTY_MODEM_CD);
	    out_mstate |= (in_mstate & TTY_MODEM_RI);
	}
	else	// TTY_DCE
	{
	    out_mstate |= (in_mstate & TTY_MODEM_DTR);
#if 0
	    out_mstate |= (in_mstate & TTY_MODEM_RTS);
#endif
	}

	if (send_modem_state(p, out_mstate))
	    return -1;

	t->last_in_mstate = in_mstate;
	t->last_in_mstate_valid = 1;
    }

    return 0;
}

static void usage(const char *bname)
{
    dflog(LOG_INFO,
	  "Usage: %s /dev/ttyPORT {DTE|DCE} P R IPToS host:port ...\n"
	  "\tIPToS - 0xNN"
	  "\tP - modem state poll interval, msec\n"
	  "\tR - connection restart delay time, msec\n"
	  , bname);
    exit(EXIT_FAILURE);
}

int main(int ac, char *av[]/*, char *envp[]*/)
{
    const char *bname = basename(av[0]);
    dflog_open( bname, DFLOG_PID|DFLOG_SYS|DFLOG_STDERR );
    dflog(LOG_INFO, "starting ...");

    if (ac < 7)
	usage(bname);

    size_t ai = 0;
    const char *device   =        av[++ai];
    const char *devtype  =        av[++ai];
    size_t mstat_intval  =   atoi(av[++ai]);
    size_t restart_delay =   atoi(av[++ai]);
    int ip_tos           = strtol(av[++ai], NULL, 0);


    const size_t targets = ac - ai - 1;


    char * hosts[targets]; memset(hosts, 0, sizeof(hosts));
    char * ports[targets]; memset(ports, 0, sizeof(ports));

    for (size_t t=0; t < targets; ++t)
    {
	hosts[t] = av[++ai];
	char *cp = strchr(hosts[t], ':');
	if (cp == NULL)
	    dferror(EXIT_FAILURE, 0,
		    "No ':' separator detected in host:port[%d]: %s",
		    t, hosts[t]);
	*cp = '\0';
	ports[t] = ++cp;
    }


    enum TTY_TYPE tty_type = TTY_DCE;

    if ( strcmp(devtype, "DTE") == 0)
	tty_type = TTY_DTE;
    else if( strcmp(devtype, "DCE") == 0)
	tty_type = TTY_DCE;
    else
        usage(av[0]);


    static char progname[128];
    snprintf(progname, sizeof(progname), "%s %-7s",
	     bname, basename(device));

    dflog_open(progname, DFLOG_PID|DFLOG_SYS|DFLOG_STDERR);

    ssize_t rc = daemon(0, 0);
    if (rc < 0)
    {
	dferror(EXIT_SUCCESS, errno, "daemonize error");
	fail();
    }

    dflog_open(progname, DFLOG_PID|DFLOG_SYS);
    dflog(LOG_WARNING, "started up");


    /* Ignore SIGPIPEs so they don't kill us. */
    signal(SIGPIPE, SIG_IGN);

    setup_sighandler(sig_handler, SIGHUP);
    setup_sighandler(sig_handler, SIGINT);
    setup_sighandler(sig_handler, SIGQUIT);
    setup_sighandler(sig_handler, SIGTERM);
    setup_sighandler(sig_handler, SIGCHLD);


    static const size_t   DATA_BUF_SIZE = PIPE_BUF/2;
    static const size_t  STATE_BUF_SIZE = PIPE_BUF/8;

    char  data_buf[ DATA_BUF_SIZE];
    char state_buf[STATE_BUF_SIZE];

    worker_t workers[targets];
    for (size_t t=0; t < targets; ++t)
    {
	worker_start  (&workers[t], hosts[t], ports[t], ip_tos, restart_delay);
	fd_set_nonblock(workers[t]. data_wr_p);
	fd_set_nonblock(workers[t].state_wr_p);
    }

    tty_t *tty = tty_create();


    const size_t targetpolls  = targets * 2;
    const size_t poll_items   = targetpolls + 1; // +1 for tty poll
    const size_t tty_poll_idx = targetpolls; // last poll for tty
    struct pollfd polls[poll_items];
    memset(polls, 0, sizeof(polls));


    do // restart loop
    {
	tty_open(tty, device);

#ifndef NO_RS232
	if (tty_set_raw(tty))
	    fail();
#endif


#define WORKER_INDEX(i)    (                             (i) / 2  )
#define IS_DATA_POLL(i)    ( (i != tty_poll_idx) && !!! ((i) % 2) )
#define IS_STATE_POLL(i)   ( (i != tty_poll_idx) &&  !! ((i) % 2) )
#define IS_TTY_POLL(i)     (  i == tty_poll_idx                   )

	for (size_t p=0;  p < targetpolls; ++p)
	{
	    polls[p].fd = IS_STATE_POLL(p)
		? workers[WORKER_INDEX(p)].state_rd_p
		: workers[WORKER_INDEX(p)]. data_rd_p;
	    polls[p].events = POLLIN;
	}

	polls[tty_poll_idx].fd = tty_fd(tty);
	polls[tty_poll_idx].events = POLLIN;

	do // poll loop
	{
	    rc = poll(polls, poll_items, mstat_intval);
	    if (rc < 0)
	    {
		dferror(EXIT_SUCCESS, errno, "poll failed");
		fail();
	    }
	    else if (rc == 0)	// timeout
	    {
		// handle modem lines state
#ifdef DO_TICK
		dflog(LOG_INFO, "tick");
#endif
		// get_send_new_modem_state(tty, workers[target].state_wr_p)
		// called at the loop end for each target
	    }
	    else // poll revents handling
	    {
		for (size_t p=0; p < poll_items; ++p) // polls loop
		{
		    size_t target = WORKER_INDEX(p);

		    if ( IS_DATA_POLL(p) )
		    {
			if (polls[p].revents & POLLIN)
			{
			    ssize_t r = read(workers[target].data_rd_p,
					     data_buf, DATA_BUF_SIZE);
			    if (r < 0)
			    {
				dferror(EXIT_SUCCESS, errno,
					"Data pipe: Error reading from %s:%s",
					hosts[target], ports[target]);
				fail();
			    }
			    else if (r == 0)
			    {
				dflog(LOG_ERR, "Data pipe: EOF received from %s:%s",
				      hosts[target], ports[target]);
				fail();
			    }
			    else
			    {
				if (tty_write_all(tty, data_buf, r))
				    goto restart;
			    }
			}
		    } // IS_DATA_POLL(p)

		    if ( IS_STATE_POLL(p) )
		    {
			if (polls[p].revents & POLLIN)
			{
			    ssize_t r = read(workers[target].state_rd_p,
					     state_buf, STATE_BUF_SIZE);
			    if (r < 0)
			    {
				dferror(EXIT_SUCCESS, errno,
					"State pipe: Error reading from %s:%s",
					hosts[target], ports[target]);
				fail();
			    }
			    else if (r == 0)
			    {
				dflog(LOG_ERR, "State pipe: EOF received from %s:%s",
				      hosts[target], ports[target]);
				fail();
			    }
			    else
			    {
				for (size_t i = 0; i < (size_t)r; ++i)
				{
				    tty_log_modem_state("Recv modem state: ", state_buf[i]);
				    if ( tty_set_modem_state(tty, state_buf[i]) )
					goto restart;
				}
			    }
			}
		    } // IS_STATE_POLL(p)

		    if ( IS_TTY_POLL(p) )
		    {
			if (polls[p].revents & POLLIN)
			{
			    ssize_t r = tty_read(tty, data_buf, DATA_BUF_SIZE);
			    if (r < 0)
				goto restart;
			    else if (r == 0)
			    {
				dflog(LOG_WARNING, "EOF readed from %s, (DTR|DSR)==0",
				      tty_name(tty));

				for (size_t t=0; t < targets; ++t)
				    if (send_modem_state(workers[t].state_wr_p,
							 0/* ~(DTR|DSR|CD|RI) */))
				    {
					if (wouldblock())
					    continue; // skip failed target

					dferror(EXIT_SUCCESS, errno,
						"State pipe: Error writing to %s:%s",
						hosts[t], ports[t]);
					fail();
				    }

				tty_open(tty, device); // tty reopen
#ifndef NO_RS232
				if (tty_set_raw(tty))
				    goto restart;
#endif
				// not fail, continue with re-opened tty
			    }
			    else	// r > 0
			    {
				for (size_t t=0; t < targets; ++t)
				    if (write_all_raw(workers[t].data_wr_p, data_buf, r))
				    {
					if (wouldblock())
					    continue; // skip failed target

					dferror(EXIT_SUCCESS, errno,
						"Data pipe: Error writing to %s:%s",
						hosts[target], ports[target]);
					fail();
				    }
			    }
			} // polls[p].revents & POLLIN
		    } // IS_TTY_POLL(p)
		} // polls[p].revents loop
	    } // poll revents handling

	    for (size_t t=0; t < targets; ++t)
		if (get_send_new_modem_state(tty, workers[t].state_wr_p, tty_type))
		{
		    if (wouldblock())
			continue; // skip failed target

		    goto restart;
		}

	} while(1); // poll loop

restart:

	tty_close(tty);
        if (restart_delay != 0)
	    usleep(restart_delay * 1000);
	dflog(LOG_WARNING, "restart");
    } while(1); // restart loop

    dflog(LOG_WARNING, "finished");
    return EXIT_SUCCESS;
}
