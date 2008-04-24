#include "svd.h"
#include "svd_cfg.h"
#include "svd_ua.h"
#include "svd_atab.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/unistd.h>
#include <assert.h>
#include <stdarg.h>
#include <syslog.h>
#include <signal.h>
#include <errno.h>

#define DEFAULT_SOCKET_PATH "/tmp/sv.socket"

/* Change this to whatever your daemon is called */
#define DAEMON_NAME "svd"

enum svd_io_event_e {
	SVD_EVT_RECONFIGURE
};

#define AS_DAEMON 1
//#define LOG_PID_MINE 1

/******************************************************************************/

unsigned int g_f_cnt= 0; 
unsigned int g_f_offset = 0;

static int svd_daemonize( void );

static svd_t * svd_create( void );
static int svd_destroy( svd_t ** svd );
static svd_t * svd_global_reconfigure ( svd_t * svd );

static void svd_logger(void *logarg, char const *format, va_list ap);
static void svd_log_set( int const level, char * path );

static int svd_is_runing ( char const * const s_path );
static int svd_io_init (svd_t * const svd, char const * const s_path);
static int svd_io_deinit (svd_t * const svd);
static int svd_io_handler (su_root_magic_t * root, su_wait_t * w,
		su_wakeup_arg_t * user_data);

/******************************************************************************/

int main( int argc, char ** argv )
{
	svd_t *svd;
	int err = 0;
	int nothing_to_do;
	pid_t current;
#ifdef LOG_PID_MINE
	current = getpid();
	fprintf(stderr,"started %d\n",current);
#endif
	nothing_to_do = startup_init( argc, argv );
	if( nothing_to_do ){
		goto __startup;
	}

	su_init();

	svd_log_set (5, NULL);

	/* runing test */
	err = svd_is_runing( g_so.socket_name );
	if (err == 1) { 
		/* already running */
		err = 0;
#ifdef LOG_PID_MINE
		fprintf(stderr,"%d : allready runing\n",current);
#endif
		goto __su;
	} else if (err == -1){
		/* error */
#ifdef LOG_PID_MINE
		fprintf(stderr,"%d : error on is_run test\n",current);
#endif
		goto __su;
	}

	/* read svd.conf file */
	err = svd_conf_init();
	if( err ){
		goto __conf;
	}

	/* change log level and path to config sets */
	svd_log_set (g_conf.log.log_level, g_conf.log.log_path);

	/* create svd structure */
	/* uses !!g_cnof */
	svd = svd_create( );
	if (svd == NULL) {
		goto __svd;
	}

	/* daemonization */
#if (AS_DAEMON == 1)
#ifdef LOG_PID_MINE
	fprintf(stderr,"%d : at daemonize\n",current);
#endif
	err = svd_daemonize ();
#ifdef LOG_PID_MINE
	fprintf(stderr,"demonize err = %d\n",err);
#endif
	if( err == -1 ){
		/* error - should exit and destroy io_socket */
		goto __svd;
	} else if ( err == 1 ){
		/* parent - should exit but not destroy io_socket */
		goto __conf; /* memory leaks in svd_destroy */
	}
	/* child - the daemon */
#endif
	/* run main cycle */
	su_root_run (svd->root);

__svd:
#ifdef LOG_PID_MINE
	fprintf(stderr,"1\n",err);
#endif
	svd_destroy (&svd);
__conf:
#ifdef LOG_PID_MINE
	fprintf(stderr,"2\n",err);
#endif
	svd_conf_destroy ();
__su:
#ifdef LOG_PID_MINE
	fprintf(stderr,"3\n",err);
#endif
	su_deinit ();
__startup:
#ifdef LOG_PID_MINE
	fprintf(stderr,"4\n",err);
#endif
	startup_destroy (argc, argv);

#ifdef LOG_PID_MINE
	current = getpid();
	fprintf(stderr,"%d : exiting\n",current);
#endif
	return err;
};

/////////////////////////////////////////////////////////////////
static int
svd_daemonize( void )
{
	pid_t pid; 
	pid_t sid; 

	/* Fork off the parent process */
	pid = fork();
	if (pid < 0) {
		SU_DEBUG_0(( "unable to fork daemon, code=%d (%s)\n",
			errno, strerror(errno) ));
		goto __exit_fail;
	}
	/* If we got a good PID, then we can exit the parent process. */
	if (pid > 0) {
#ifdef LOG_PID_MINE
		fprintf(stderr,"forked child: %d\n",pid);
#endif
		goto __exit_parent;
	}

	/* At this point we are executing as the child process */

	/* Cancel certain signals */
	signal(SIGCHLD,SIG_DFL); /* A child process dies */
	signal(SIGTSTP,SIG_IGN); /* Various TTY signals */
	signal(SIGTTOU,SIG_IGN);
	signal(SIGTTIN,SIG_IGN);
	signal(SIGHUP, SIG_IGN); /* Ignore hangup signal */
	signal(SIGTERM,SIG_DFL); /* Die on SIGTERM */

	/* Change the file mode mask */
	umask(0);

	/* Create a new SID for the child process */
	sid = setsid();
	if (sid < 0) {
		SU_DEBUG_0(( "unable to create a new session, code %d (%s)\n",
			errno, strerror(errno) ));
		goto __exit_fail;
	}

	/* Change the current working directory.  This prevents the current
	directory from being locked; hence not being able to remove it. */
	if ((chdir("/")) < 0) {
		SU_DEBUG_0(( "unable to change directory to %s, "
				"code %d (%s)\n",
				"/", errno, strerror(errno) ));
		goto __exit_fail;
	}

	/* Redirect standard files to /dev/null */
	freopen( "/dev/null", "r", stdin);
	freopen( "/dev/null", "w", stdout);
	freopen( "/dev/null", "w", stderr);

#ifdef LOG_PID_MINE
fprintf(stderr,"exit_child\n");
#endif
	return 0;

__exit_fail:
#ifdef LOG_PID_MINE
fprintf(stderr,"exit_child_fail\n");
#endif
	return -1;

__exit_parent:
#ifdef LOG_PID_MINE
fprintf(stderr,"exit_parent\n");
#endif
	return 1;
};

/* uses g_cnof */
static svd_t * svd_create( void )
{
	svd_t * svd;
	int err;
DFS
	svd = malloc( sizeof(*svd) );
	if (!svd) {
    		SU_DEBUG_0 (("svd_create() not enough memory\n"));
		goto __exit_fail;
	}

	memset (svd, 0, sizeof(*svd));

	/* svd home initialization */
	if(su_home_init(svd->home) != 0){
    		SU_DEBUG_0 (("svd_create() su_home_init() failed\n"));
		goto __exit_fail;
	}

	/* svd root creation */
	svd->root = su_root_create (svd);
	if (svd->root == NULL) {
    		SU_DEBUG_0 (("svd_create() su_root_create() failed\n"));
		goto __exit_fail;
	}

	/* svd Input / Output system initialization */
	err = svd_io_init (svd, g_so.socket_name );
	if( err ) {
		goto __exit_fail;
	}

	/* create ab structure of svd and handle callbacks */
	/* uses g_cnof */
	err = svd_atab_create (svd);
	if( err ) {
		goto __exit_fail;
	}

	/* launch the SIP stack */
	/* uses !!gconf */
	svd->nua = nua_create (svd->root, svd_nua_callback, svd,
			/* deprecated
			TAG_IF(conf->ssc_stun_server,
				STUNTAG_SERVER(conf->ssc_stun_server)),
			*/
			/* unused yet
			TAG_IF(conf->ssc_contact, 
				NUTAG_URL(conf->ssc_contact)),
			TAG_IF(conf->ssc_media_addr,
				NUTAG_MEDIA_ADDRESS(conf->ssc_media_addr)),
			SOATAG_AF(SOA_AF_IP4_IP6), 
			TAG_IF(conf->ssc_proxy,
				NUTAG_PROXY(conf->ssc_proxy)),
			TAG_IF(conf->ssc_registrar,
				NUTAG_REGISTRAR(conf->ssc_registrar)),
			TAG_IF(conf->ssc_aor,
				SIPTAG_FROM_STR(conf->ssc_aor)),
			TAG_IF(conf->ssc_certdir,
				NUTAG_CERTIFICATE_DIR(conf->ssc_certdir)), 
			*/
			NUTAG_ALLOW("INFO"),
			NUTAG_ENABLEMESSAGE(1),
			NUTAG_ENABLEINVITE(1),
			NUTAG_AUTOALERT(1),
			TAG_NULL() );

	if (svd->nua) {
		nua_get_params(svd->nua, TAG_ANY(), TAG_NULL());
	} else {
		SU_DEBUG_0 (("Network is not initialized\n"));
		goto __exit_fail;
	}
DFE
	return svd;

__exit_fail:
DFE
	return NULL;
};

static int
svd_destroy( svd_t ** svd )
{
DFS
	int err = 0;
	if(*svd){
	    	SU_DEBUG_3 (("ATAB delete.. "));
		svd_atab_delete (*svd);
	    	SU_DEBUG_3 (("OK\n"));

	    	SU_DEBUG_3 (("I/O deinit.. "));
		err = svd_io_deinit (*svd);
	    	SU_DEBUG_3 (("OK\n"));

		if((*svd)->nua){
	    		SU_DEBUG_3 (("NUA shotdown.. "));
			nua_shutdown ((*svd)->nua);
    			SU_DEBUG_3 (("OK\n"));
		}
		if((*svd)->root){
			SU_DEBUG_3 (("ROOT destroy.. "));
			su_root_destroy ((*svd)->root);
			SU_DEBUG_3 (("OK\n"));
		}
		if((*svd)->home){
			SU_DEBUG_3 (("HOME deinit.. "));
			su_home_deinit ((*svd)->home);
			SU_DEBUG_3 (("OK\n"));
		}

		SU_DEBUG_3 (("SELF destroy.. "));
		free (*svd);
		*svd = NULL;
		SU_DEBUG_3 (("OK\n"));
	}
DFE
	return err;
};

static svd_t *
svd_global_reconfigure ( svd_t * svd )
{
	svd_t * svd_new;
	int err;
DFS
	/* destroy svd */
	err = svd_destroy( &svd );
	/* destroy config */
	svd_conf_destroy();

	/* read svd.conf file */
	err = svd_conf_init();
	if( err ){
		goto __conf;
	}

	/* change log level and path to config sets */
	svd_log_set (g_conf.log.log_level, g_conf.log.log_path);

	/* create svd structure */
	/* uses !!g_cnof */
	svd_new = svd_create( );
	if (svd_new == NULL){
		goto __svd;
	}
	
DFE
	return svd_new;
__svd:
	svd_destroy (&svd_new);
__conf:
	svd_conf_destroy ();
DFE
	return NULL;
};

static void 
svd_logger(void *logarg, char const *format, va_list ap)
{
	FILE * log_stream = NULL;
	if (!logarg){
		return;
	}

#if (AS_DAEMON == 1)
	log_stream = fopen (logarg, "a");
	if( log_stream ){
		vfprintf(log_stream, format, ap);
		fclose (log_stream);
		log_stream = NULL;
	}
#else
	vfprintf(stderr, format, ap);
#endif
};

static void
svd_log_set (int const level, char * path )
{
	if (level == -1){
		/* do not log anything */
		su_log_redirect (NULL, svd_logger, NULL);
		goto __exit;
	}

	if( !path ){
		/* log to standart log file */
		path = LOG_FILE_PATH_DF;
	} 

	su_log_set_level (NULL, level);
	su_log_redirect (NULL, svd_logger, path);

__exit:
	return;
};

/* * 
 * tests if svd allready runing 
 * test by socket presence
 * ret  0 : NOT RUNING - should work as nothing happens
 * ret 1 : RUNING sends RECONFIGURE message
 * 		or somthing nasty happens
 * ret -1 : something nasty happens
 */
static int 
svd_is_runing ( char const * const s_path )
{
	struct sockaddr_un sv_addr;
	struct sockaddr_un cl_addr;
	int sock_fd;
	int err;
	int ret;
DFS
	sock_fd = socket(AF_UNIX, SOCK_DGRAM, 0);
	if(sock_fd < 0){
		su_perror("svd_is_runing() socket() on I/O descriptor");
		ret = -1; /* error */
		goto __exit;
	}

	memset (&sv_addr, 0, sizeof(sv_addr));

	sv_addr.sun_family = AF_UNIX;
	if( !s_path ) {
		strcpy(sv_addr.sun_path, DEFAULT_SOCKET_PATH);
	} else {
		strcpy(sv_addr.sun_path, s_path);
	}

	ret = 0; /* not running */

	err = bind(sock_fd, &sv_addr, SUN_LEN(&sv_addr));
	if(err == -1){
		/* svd already runing */
		enum svd_io_event_e io_evt = SVD_EVT_RECONFIGURE;

		ret = 1;/* svd already runing */

		SU_DEBUG_5(("SVD allready running, "
				"sending RECONFIGURE message\n"));

		memset(&cl_addr, 0, sizeof(cl_addr));
		cl_addr.sun_family = AF_UNIX;
		strcpy(cl_addr.sun_path, "/tmp/sv.XXXXXX");
		mktemp(cl_addr.sun_path);

		if( bind(sock_fd, &cl_addr, SUN_LEN(&cl_addr)) == -1 ){
			su_perror( "svd_is_runing() bind()" );
			ret = -1;/* error */
			goto __unlink_client;
		}

		err = sendto(sock_fd, &io_evt, sizeof(io_evt), 0, 
				&sv_addr, SUN_LEN(&sv_addr) );
		if(err == -1){
			ret = -1;/* error */
			su_perror( "svd_is_runing() sendto()" );
		}
	}

__unlink_client:
	if( close (sock_fd) ){
		su_perror( "svd_is_runing() close()" );
	}
	if( ret == 0 && unlink (sv_addr.sun_path) ){
		su_perror( "svd_is_runing() unlink() sv_addr" );
	}
	if( ret != 0 && unlink (cl_addr.sun_path) ){
		su_perror( "svd_is_runing() unlink() cl_addr" );
	}
__exit:
DFE
	return ret;
};



static int 
svd_io_init (svd_t * const svd, char const * const s_path)
{
	su_wait_t wait[1];
	int err = 0;
DFS
	svd->io_fd = socket(AF_UNIX, SOCK_DGRAM, 0);
	if(svd->io_fd < 0){
		su_perror("svd_io_init() socket() on I/O descriptor");
		goto  __exit_fail;
	}

	memset (&svd->io_addr, 0, sizeof(svd->io_addr));

	svd->io_addr.sun_family = AF_UNIX;
	if( !s_path ) {
		strcpy(svd->io_addr.sun_path, DEFAULT_SOCKET_PATH);
	} else {
		strcpy(svd->io_addr.sun_path, s_path);
	}

	err = bind(svd->io_fd, &svd->io_addr, SUN_LEN(&svd->io_addr));
	if(err == -1){

		printf ("bind to : %s\n", svd->io_addr.sun_path);
		su_perror("svd_io_init() bind() on I/O descriptor");
		goto  __socket_unlink;
	}

	err = su_wait_create(wait, svd->io_fd, SU_WAIT_IN);
	if (err){
		SU_DEBUG_0(("svd_io_init() su_wait_create() "
				"on I/O descriptor\n"));
		goto  __socket_unlink;
	}

	err = su_root_register(svd->root, wait, svd_io_handler, svd, 0);
	if (err == -1) {
		SU_DEBUG_0(("svd_io_init() su_root_register() "
				"on I/O descriptor\n"));
		goto  __socket_unlink;
	}

DFE
	return 0;

__socket_unlink:
	if( close(svd->io_fd) ){
		su_perror("svd_io_init() close()");
	}
	if( unlink(svd->io_addr.sun_path) ){
		su_perror("svd_io_init() unlink()");
	}
__exit_fail:
DFE
	return -1;
};

static int
svd_io_deinit (svd_t * const svd)
{
	int err = 0;
DFS
	err = close (svd->io_fd);
	if( err ){
		su_perror("svd_io_deinit() close()");
	}
	err = unlink (svd->io_addr.sun_path);
	if( err ){
		su_perror("svd_io_deinit() unlink()");
	}
	return err;
DFE
};


static int 
svd_io_handler (su_root_magic_t * root, su_wait_t * w,
		su_wakeup_arg_t * user_data)
{
	socklen_t cl_addr_len;
	struct sockaddr_un cl_addr;
	enum svd_io_event_e io_evt;
	int err = 0;
DFS
	svd_t *svd = (svd_t *) user_data;
	
	memset(&cl_addr, 0, sizeof(cl_addr));

	cl_addr_len = sizeof(cl_addr);

	err = recvfrom (svd->io_fd, &io_evt, sizeof(io_evt), 0, 
			&cl_addr, &cl_addr_len );
	if (err == -1){
		su_perror("svd_io_handler() recvfrom()");
		goto __exit_fail;
	}
	
	switch (io_evt){
		case SVD_EVT_RECONFIGURE:{
			svd_t * svd_new;
			SU_DEBUG_5(("got reconfigure message\n"));

			svd_new = svd_global_reconfigure (svd);
			if (!svd_new){
				goto __exit_fail;
			}

			/* run main cycle */
			su_root_run(svd_new->root);
			break;
		}
		default :{
			SU_DEBUG_5(("got unknown message\n"));
			goto __exit_fail;
			break;
		}
	}

DFE
	return 0;

__exit_fail:
DFE
	return err;
};

