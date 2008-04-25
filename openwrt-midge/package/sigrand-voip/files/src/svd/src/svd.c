#include "svd.h"
#include "svd_cfg.h"
#include "svd_ua.h"
#include "svd_atab.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <stdarg.h>
#include <syslog.h>
#include <signal.h>
#include <errno.h>

#define DAEMON_NAME "svd"

/******************************************************************************/

unsigned int g_f_cnt= 0; 
unsigned int g_f_offset = 0;

static int 	svd_daemonize( void );
static svd_t * 	svd_create( void );
static int 	svd_destroy( svd_t ** svd );
static void 	svd_logger(void *logarg, char const *format, va_list ap);
static void 	svd_log_set( int const level, char * path );

/******************************************************************************/

int main( int argc, char ** argv )
{
	svd_t *svd;
	int err = 0;
	int nothing_to_do;

	nothing_to_do = startup_init( argc, argv );
	if( nothing_to_do ){
		goto __startup;
	}

	su_init();

	/* preliminary log settings */
	svd_log_set (5, NULL);

	/* daemonization */
	err = svd_daemonize ();
	if( err == -1 ){
		goto __su;
	}	
	/* the daemon from now */

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

	/* run main cycle */
	su_root_run (svd->root);

__svd:
	svd_destroy (&svd);
__conf:
	svd_conf_destroy ();
__su:
	su_deinit ();
__startup:
	startup_destroy (argc, argv);
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
		exit(EXIT_SUCCESS);
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
	/* freopen( "/dev/null", "w", stderr); */

	return 0;

__exit_fail:
	return -1;
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

static void 
svd_logger(void *logarg, char const *format, va_list ap)
{
	struct log_arg_s {
		int level;
		char * path;
	} * log_arg;
	log_arg = logarg;

	if (!log_arg){
		/* do not do anything  */
	} else if ( !log_arg->path ){
		vfprintf(stderr, format, ap);
	} else {
		FILE * log_stream = fopen (log_arg->path, "a"); 
		if( log_stream ){
			vfprintf(log_stream, format, ap);
			fclose (log_stream);
		}
	}
};

static void
svd_log_set (int const level, char * path )
{
	if (level == -1){
		/* do not log anything */
		su_log_set_level (NULL, 0);
		su_log_redirect (NULL, svd_logger, NULL);
	} else {
		struct log_arg_s {
			int level;
			char * path;
		} log_arg;

		log_arg.level = level;
		log_arg.path = path;

		su_log_set_level (NULL, level);
		su_log_redirect (NULL, svd_logger, &log_arg);
	}
};

