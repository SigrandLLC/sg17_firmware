#include "svd.h"
#include "libconfig.h"
#include "svd_cfg.h"
#include "svd_log.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <stdarg.h>
#include <errno.h>

#include <getopt.h>

#define ERR_SUCCESS 0
#define ERR_MEMORY_FULL 1
#define ERR_UNKNOWN_OPTION 2

static unsigned char g_err_no;

#define CONF_CODEC_SPEED "speed"
#define CONF_CODEC_QUALITY "quality"
#define CONF_CODEC_MEDIUM "medium"

static struct config_t cfg;
static struct config_t route_cfg;

static int self_number_init( void );
static int self_ip_init( void );
static void log_init( void );
static int address_book_init( void );
static int hot_line_init( void );
static int route_table_init( void );
static void error_message( );
static void show_help( void );
static void show_version( void );

int 
startup_init( int argc, char ** argv )
{
	int option_IDX;
	int option_rez;
	char * short_options = "hVd:";
	struct option long_options[ ] = {
		{ "help", no_argument, NULL, 'h' },
		{ "version", no_argument, NULL, 'V' },
		{ "debug", required_argument, NULL, 'd' },
		{ NULL, 0, NULL, 0 }
		};

	g_err_no = ERR_SUCCESS;

	/* INIT WITH DEFAULT VALUES */
	g_so.help = 0;
	g_so.version = 0;
	g_so.debug_level = -1;

	/* INIT FROM SYSTEM CONFIG FILE "/etc/routine" */
	/* INIT FROM SYSTEM ENVIRONMENT */
	/* INIT FROM USER DOTFILE "$HOME/.routine" */
	/* INIT FROM USER ENVIRONMENT */
	/* INIT FROM STARTUP KEYS */

	opterr = 0;
	while ((option_rez = getopt_long (
			argc, argv, short_options, 
			long_options, &option_IDX)) != -1) {
		switch( option_rez ){
			case 'h': {
				g_so.help = 1;
				return 1;
			}
			case 'V': {
				g_so.version = 1;
				return 1;
			}
			case 'd': {
				g_so.debug_level = strtol(optarg, NULL, 10);
				if(g_so.debug_level > 9){
					g_so.debug_level = 9;
				} else if(g_so.debug_level < 0){
					g_so.debug_level = 0;
				}
				return 0;
			}
			case '?' :{
				/* unknown option found */
				g_err_no = ERR_UNKNOWN_OPTION;
				//g_err_tag = optind;
				return g_err_no;
			}
		}
	}
	return 0;
};
	
void 
startup_destroy( argc, argv )
{
	if( g_so.help ){
		show_help( );
	} else if( g_so.version ){
		show_version( );
	} 

	error_message( );
};

/** 
 * Parses <svd.conf> file and 
 * write parsed info into <g_conf>
 *
 * return 
 * -1 - in error case
 *  0 - in success 
 *
 * */
int 
svd_conf_init( void )
{
	char const * str_elem;
	int err;

	memset (&g_conf, 0, sizeof(g_conf));
	/* Initialize the configuration */
	config_init (&cfg);

	/* Load the file */
	if (!config_read_file (&cfg, SVD_CONF_NAME)){
		err = config_error_line (&cfg);
		SU_DEBUG_0(("svd_conf_init(): Config file syntax "
				"error in line %d\n", err));
		goto svd_conf_init__exit;
	} 

	/* Get values */

	/* app.self_number */
	err = self_number_init();
	if( err ){
		goto svd_conf_init__exit;
	}
	/* app.self_ip */
	err = self_ip_init();
	if( err ){
		goto svd_conf_init__exit;
	}

	/* app.log */
	log_init();

	/* app.ext_codec */
	str_elem = config_lookup_string (&cfg, "app.ext_codec");
	if( !str_elem ){
		SU_DEBUG_0((LOG_FNC_A("ext_code is not set")));
		goto svd_conf_init__exit;
	}
	if( !strcmp(str_elem, CONF_CODEC_SPEED) ){
		g_conf.ext_codec = codec_type_SPEED;
	} else if ( !strcmp(str_elem, CONF_CODEC_MEDIUM) ){
		g_conf.ext_codec = codec_type_MEDIUM;
	} else if ( !strcmp(str_elem, CONF_CODEC_QUALITY) ){
		g_conf.ext_codec = codec_type_QUALITY;
	}

	/* app.int_codec */
	str_elem = config_lookup_string (&cfg, "app.int_codec");
	if( !str_elem ){
		SU_DEBUG_0((LOG_FNC_A("int_code is not set")));
		goto svd_conf_init__exit;
	}
	if( !strcmp(str_elem, CONF_CODEC_SPEED) ){
		g_conf.int_codec = codec_type_SPEED;
	} else if ( !strcmp(str_elem, CONF_CODEC_MEDIUM) ){
		g_conf.int_codec = codec_type_MEDIUM;
	} else if ( !strcmp(str_elem, CONF_CODEC_QUALITY) ){
		g_conf.int_codec = codec_type_QUALITY;
	}

	/* app.address_book */
	err = address_book_init();
	if( err ){
		goto svd_conf_init__exit;
	}

	/* app.hot_line */
	err = hot_line_init();
	if( err ){
		goto svd_conf_init__exit;
	}

	/* app.route_table */
	err = route_table_init();
	if( err ){
		goto svd_conf_init__exit;
	}

	/* Free the configuration */
	config_destroy (&cfg);

	conf_show();
	return 0;

svd_conf_init__exit:
	/* Free the configuration */
	config_destroy (&cfg);

	conf_show();
	return -1;
};

void 
conf_show( void )
{
	struct adbk_record_s * curr_ab_rec;
	struct htln_record_s * curr_hl_rec;
	struct rttb_record_s * curr_rt_rec;
	int i;
	int j;

	SU_DEBUG_3(("=========================\n"));
	SU_DEBUG_3(("%s[%s] : ", g_conf.self_number, g_conf.self_ip));
	SU_DEBUG_3(("log["));

	if( g_conf.log_level == -1 ){
		SU_DEBUG_3(("no] : "));
	} else {
		SU_DEBUG_3(("%d] :", g_conf.log_level));
	}

	SU_DEBUG_3(("ic/ec["));
	switch(g_conf.int_codec){
		case codec_type_UNDEFINED:
			SU_DEBUG_3(("U/"));
			break;

		case codec_type_SPEED:
			SU_DEBUG_3(("S/"));
			break;
			
		case codec_type_QUALITY:
			SU_DEBUG_3(("Q/"));
			break;

		case codec_type_MEDIUM:
			SU_DEBUG_3(("M/"));
			break;
	}
	switch(g_conf.ext_codec){
		case codec_type_UNDEFINED:
			SU_DEBUG_3(("U]\n"));
			break;

		case codec_type_SPEED:
			SU_DEBUG_3(("S]\n"));
			break;
			
		case codec_type_QUALITY:
			SU_DEBUG_3(("Q]\n"));
			break;

		case codec_type_MEDIUM:
			SU_DEBUG_3(("M]\n"));
			break;
	}

	if(g_conf.address_book.records_num){
		SU_DEBUG_3(("AddressBook :\n"));
	}
	j = g_conf.address_book.records_num;
	for(i = 0; i < j; i++){
		curr_ab_rec = &g_conf.address_book.records[ i ];
		SU_DEBUG_3(("\t%d/\"%s\" : %s\n", 
				i+1, curr_ab_rec->id, curr_ab_rec->value));
	}
			
	if(g_conf.hot_line.records_num){
		SU_DEBUG_3(("HotLine :\n"));
	}
	j = g_conf.hot_line.records_num;
	for(i = 0; i < j; i++){
		curr_hl_rec = &g_conf.hot_line.records[ i ];
		SU_DEBUG_3(("\t%d/\"%s\" : %s\n",
				i+1, curr_hl_rec->id, curr_hl_rec->value ));
	}

	if(g_conf.route_table.records_num){
		SU_DEBUG_3(("RouteTable :\n"));
	}
	j = g_conf.route_table.records_num;
	for(i = 0; i < j; i++){
		curr_rt_rec = &g_conf.route_table.records[ i ];
		SU_DEBUG_3(("\t%d/\"%s\" : %s\n",
				i+1, curr_rt_rec->id, curr_rt_rec->value));
	}
	SU_DEBUG_3(("=========================\n"));
};

void 
svd_conf_destroy( void )
{
	int i;
	int j;

	if( g_conf.self_number && g_conf.self_number != g_conf.self_number_s ){
		free (g_conf.self_number);
	}

	j = g_conf.address_book.records_num;
	if (j){
		struct adbk_record_s * curr_rec; 
		for(i = 0; i < j; i++){
			curr_rec = &g_conf.address_book.records[ i ];
			if (curr_rec->id && curr_rec->id != curr_rec->id_s){
				free (curr_rec->id);
			}
			if (curr_rec->value && 
					curr_rec->value != curr_rec->value_s){
				free (curr_rec->value);
			}
		}
		free (g_conf.address_book.records);
	}

	j = g_conf.hot_line.records_num;
	if (j){
		struct htln_record_s * curr_rec; 
		for(i = 0; i < j; i++){
			curr_rec = &g_conf.hot_line.records[ i ];
			if (curr_rec->value && 
					curr_rec->value != curr_rec->value_s){
				free (curr_rec->value);
			}
		}
		free (g_conf.hot_line.records);
	}

	j = g_conf.route_table.records_num;
	if (j){
		struct rttb_record_s * curr_rec; 
		for(i = 0; i < j; i++){
			curr_rec = &g_conf.route_table.records[ i ];
			if (curr_rec->id && curr_rec->id != curr_rec->id_s){
				free (curr_rec->id);
			}
		}
		free (g_conf.route_table.records);
	}

	memset(&g_conf, 0, sizeof(g_conf));
};

/////////////////////////////////////////////////////////////////

static int 
self_number_init( void )
{
	char const * elem;
	int elem_len;

	elem = config_lookup_string (&cfg, "app.self_number");
	if( !elem ){
		SU_DEBUG_0 ((LOG_FNC_A("self_number is not set")));
		goto self_number_init__exit;
	}

	elem_len = strlen(elem);
	if(elem_len+1 > ROUTE_ID_LEN_DF){
		g_conf.self_number = malloc( (elem_len+1) * 
				sizeof(*(g_conf.self_number)));
		if( !g_conf.self_number ){
			SU_DEBUG_0 ((LOG_FNC_A(LOG_NOMEM_A("self_number"))));
		}
	} else {
		g_conf.self_number = g_conf.self_number_s;
	}
	strcpy (g_conf.self_number, elem);

	return 0;

self_number_init__exit:
	return -1;
};

static int 
self_ip_init( void )
{
	char const * elem;

	elem = config_lookup_string (&cfg, "app.self_ip");
	if( !elem ){
		SU_DEBUG_0 ((LOG_FNC_A("self_ip is not set")));
		goto __exit_fail;
	}

	strcpy (g_conf.self_ip, elem);

	return 0;

__exit_fail:
	return -1;
};

static void
log_init( void )
{
	struct config_setting_t * set;

	set = config_lookup (&cfg, "app.log" );
	if( !set ){
		/* do not log anything  */
		g_conf.log_level = -1;
		goto __exit;
	} 

	g_conf.log_level = config_setting_get_int_elem (set, 0);
	if (g_conf.log_level < 0){
		SU_DEBUG_2 (("Wrong \"log_level\" value [%d] .. set to [0]\n",
				g_conf.log_level));
		g_conf.log_level = 0;
	} else if (g_conf.log_level > 9){
		SU_DEBUG_2 (("Wrong \"log_level\" value [%d] .. set to [9]\n",
				g_conf.log_level));
		g_conf.log_level = 9;
	}

__exit:
	return;
};

static int
address_book_init( void )
{
	struct config_setting_t * set;
	struct config_setting_t * rec_set;
	struct adbk_record_s * curr_rec;
	char const * elem;
	int elem_len;
	int rec_num;
	char use_id_local_buf = 1;
	char val_tmp [VAL_IN_CONF_FILE_MAX_SIZE];
	char * val_tmp_p;
	int val_len;
	int id_len;
	int i;

	set = config_lookup (&cfg, "app.address_book" );
	if( !set ){
		g_conf.address_book.records_num = 0;
		goto address_book_init__exit_success;
	} 

	rec_num = config_setting_length (set);

	g_conf.address_book.records_num = rec_num;
	g_conf.address_book.records = malloc (
			rec_num * sizeof(*(g_conf.address_book.records)));
	if( !g_conf.address_book.records ){
		SU_DEBUG_0((LOG_FNC_A(LOG_NOMEM)));
		goto address_book_init__exit;
	}
	memset(g_conf.address_book.records, 0, 
			rec_num * sizeof(*(g_conf.address_book.records)));

	rec_set = config_setting_get_elem (set, 0);
	elem = config_setting_get_string_elem (rec_set, 0);
	id_len = strlen(elem);

	g_conf.address_book.id_len = id_len;

	if (id_len + 1 > ADBK_ID_LEN_DF){
		use_id_local_buf = 0;
	}

	for(i = 0; i < rec_num; i++){
		int j;
		curr_rec = &g_conf.address_book.records[ i ];
		rec_set = config_setting_get_elem (set, i);

		/* get id */
		elem = config_setting_get_string_elem (rec_set, 0);

		if (use_id_local_buf){
			curr_rec->id = curr_rec->id_s;
		} else {
			curr_rec->id = malloc(
					(id_len + 1) *
					sizeof(*(curr_rec->id)));
			if( !curr_rec->id ){
				SU_DEBUG_0((LOG_FNC_A(LOG_NOMEM)));
				goto address_book_init__exit;
			}
		}
		strcpy(curr_rec->id, elem);
		
		/* get value with no garbage */
		elem = config_setting_get_string_elem (rec_set, 1);
		elem_len = strlen(elem);
		if(elem_len + 1 > VAL_IN_CONF_FILE_MAX_SIZE){
			val_tmp_p = malloc(
					(elem_len+1) *
					sizeof(*val_tmp_p));
			if( !val_tmp_p ){
				SU_DEBUG_0 ((LOG_FNC_A(LOG_NOMEM)));
				goto address_book_init__exit;
			}
		} else {
			val_tmp_p = val_tmp;
		}
		
		val_len = 0;
		for(j = 0; j < elem_len; j++){
			if( isdigit (elem [j]) || 
					elem [j] == WAIT_MARKER ||
					elem [j] == ADBK_MARKER ||
					elem [j] == SELF_MARKER ||
					elem [j] == FXO_MARKER ||
					elem [j] == NET_MARKER ){
				val_tmp_p [val_len] = elem [j];
				++val_len;
			}
		}
		val_tmp_p [val_len] = '\0';
		curr_rec->value_len = val_len;

		if (val_len+1 < VALUE_LEN_DF ){
			curr_rec->value = curr_rec->value_s;
		} else {
			curr_rec->value = malloc(
					(val_len+1) *
					sizeof(*(curr_rec->value)));
			if( !curr_rec->value ){
				SU_DEBUG_0((LOG_FNC_A(LOG_NOMEM)));
				goto address_book_init__tmp_alloc;
			}
		}
		strcpy (curr_rec->value, val_tmp_p );
	}

address_book_init__exit_success:
	return 0;

address_book_init__tmp_alloc:
	if( val_tmp_p && val_tmp_p != val_tmp ){
		free (val_tmp_p);
	}
address_book_init__exit:
	return -1;
};


static int 
hot_line_init( void )
{
	struct config_setting_t * set;
	struct config_setting_t * rec_set;
	char const * elem;
	int elem_len;
	int rec_num;
	char val_tmp [VAL_IN_CONF_FILE_MAX_SIZE];
	char * val_tmp_p;
	int val_len;
	int i;

	set = config_lookup (&cfg, "app.hot_line" );
	if( !set ){
		g_conf.hot_line.records_num = 0;
		goto hot_line_init__exit_success;
	} 

	rec_num = config_setting_length (set);

	g_conf.hot_line.records_num = rec_num;
	g_conf.hot_line.records = malloc (
			rec_num * sizeof(*(g_conf.hot_line.records)));
	if( !g_conf.hot_line.records ){
		SU_DEBUG_0((LOG_FNC_A(LOG_NOMEM)));
		goto hot_line_init__exit;
	}
	memset(g_conf.hot_line.records, 0, 
			rec_num * sizeof(*(g_conf.hot_line.records)));

	for(i = 0; i < rec_num; i++){
		int j;
		struct htln_record_s * curr_rec = 
				&g_conf.hot_line.records[ i ];
		rec_set = config_setting_get_elem (set, i);
		/* get id */
		elem = config_setting_get_string_elem (rec_set, 0);
		strncpy(curr_rec->id, elem, CHAN_ID_LEN-1);
		
		/* get value with no garbage */
		elem = config_setting_get_string_elem (rec_set, 1);
		elem_len = strlen(elem);
		if(elem_len + 1 > VAL_IN_CONF_FILE_MAX_SIZE){
			val_tmp_p = malloc(
					(elem_len+1) *
					sizeof(*val_tmp_p));
			if( !val_tmp_p ){
				SU_DEBUG_0((LOG_FNC_A(LOG_NOMEM)));
				goto hot_line_init__exit;
			}
		} else {
			val_tmp_p = val_tmp;
		}

		val_len = 0;
		for(j = 0; j < elem_len; j++){
			if( isdigit (elem [j]) ||
					elem [j] == WAIT_MARKER ||
					elem [j] == ADBK_MARKER ||
					elem [j] == SELF_MARKER ||
					elem [j] == FXO_MARKER ||
					elem [j] == NET_MARKER ){
				val_tmp_p [val_len] = elem [j];
				++val_len;
			}
		}
		val_tmp_p [val_len] = '\0';
		curr_rec->value_len = val_len;

		if (val_len+1 < VALUE_LEN_DF ){
			curr_rec->value = curr_rec->value_s;
		} else {
			curr_rec->value = malloc(
					(val_len+1) *
					sizeof(*(curr_rec->value)));
			if( !curr_rec->value ){
				SU_DEBUG_0((LOG_FNC_A(LOG_NOMEM)));
				goto hot_line_init__tmp_alloc;
			}
		}
		strcpy (curr_rec->value, val_tmp_p );
	}

	if( val_tmp_p && val_tmp_p != val_tmp ){
		free (val_tmp_p);
	}

hot_line_init__exit_success:
	return 0;

hot_line_init__tmp_alloc:
	if( val_tmp_p && val_tmp_p != val_tmp ){
		free (val_tmp_p);
	}
hot_line_init__exit:
	return -1;
};


static int
route_table_init( void )
{
	struct config_setting_t * set;
	struct config_setting_t * rec_set;
	struct rttb_record_s * curr_rec;
	char const * elem;
	int rec_num;
	char use_id_local_buf = 1;
	int id_len;
	int i;
	int err;

	/* Initialize the configuration */
	config_init (&route_cfg);

	/* Load the file */
	if (!config_read_file (&route_cfg, SVD_ROUTE_NAME)){
		err = config_error_line (&route_cfg);
		SU_DEBUG_0 (("route_table_init(): Route config file syntax "
				"error in line %d\n", err));
		goto route_table_init__exit;
	} 

	/* Get values */
	set = config_lookup (&route_cfg, "route_table" );
	if( !set ){
		SU_DEBUG_0 ((LOG_FNC_A("no data")));
		goto route_table_init__exit;
	} 

	rec_num = config_setting_length (set);

	g_conf.route_table.records_num = rec_num;
	g_conf.route_table.records = malloc (
			rec_num * sizeof(*(g_conf.route_table.records)));
	if( !g_conf.route_table.records ){
		SU_DEBUG_0((LOG_FNC_A(LOG_NOMEM)));
		goto route_table_init__exit;
	}
	memset(g_conf.route_table.records, 0, 
			rec_num * sizeof(*(g_conf.route_table.records)));

	rec_set = config_setting_get_elem (set, 0);
	elem = config_setting_get_string_elem (rec_set, 0);
	id_len = strlen(elem);

	g_conf.route_table.id_len = id_len;

	if (id_len + 1 > ROUTE_ID_LEN_DF){
		use_id_local_buf = 0;
	}

	for(i = 0; i < rec_num; i++){
		curr_rec = &g_conf.route_table.records[ i ];
		rec_set = config_setting_get_elem (set, i);

		/* get id */
		elem = config_setting_get_string_elem (rec_set, 0);

		if (use_id_local_buf){
			curr_rec->id = curr_rec->id_s;
		} else {
			curr_rec->id = malloc(
					(id_len + 1) *
					sizeof(*(curr_rec->id)));
			if( !curr_rec->id ){
				SU_DEBUG_0 ((LOG_FNC_A(LOG_NOMEM)));
				goto route_table_init__exit;
			}
		}
		strcpy(curr_rec->id, elem);
		
		/* get value */
		elem = config_setting_get_string_elem (rec_set, 1);
		strcpy (curr_rec->value, elem);
	}

	/* Free the configuration */
	config_destroy (&route_cfg);

	return 0;

route_table_init__exit:
	/* Free the configuration */
	config_destroy (&route_cfg);

	return -1;
};

static void 
error_message( void )
{
	switch( g_err_no ){
		case ERR_SUCCESS :{
			return;	
		}	
		case ERR_MEMORY_FULL :{
			fprintf( stderr, "%s : not enough memory\n", 
					PACKAGE_NAME );
			break;	
		}	
		case ERR_UNKNOWN_OPTION :{
			fprintf( stderr, "%s : invalid option\n", 
					PACKAGE_NAME );
			break;	
		}	
	}
	fprintf( stderr,"Try '%s --help' for more information.\n", 
			PACKAGE_NAME );
};

//------------------------------------------------------------

static void 
show_help( void )
{
	fprintf( stdout, 
"\
Usage: %s [OPTION]\n\
SIP VoIP User agent Daemon.\n\
\n\
Mandatory arguments to long options are mandatory for short options too.\n\
  -h, --help         display this help and exit\n\
  -V, --version      displey current version and license info\n\
  -d, --debug        set the debug level (form 0 to 9)\n\
\n\
	Execution example :\n\
	%s -d9\n\
	Means, that you wont start daemon in debug mode with \n\
			maximum debug output.\n\
\n\
Report bugs to <%s>.\n\
"
		, PACKAGE_NAME, PACKAGE_NAME, PACKAGE_NAME, PACKAGE_BUGREPORT );
};

//------------------------------------------------------------

static void 
show_version( void )
{
	fprintf( stdout, 
"\
%s-%s, built [%s]-[%s]\n\n\
Copyright (C) 2007 Free Software Foundation, Inc.\n\
This is free software.  You may redistribute copies of it under the terms of\n\
the GNU General Public License <http://www.gnu.org/licenses/gpl.html>.\n\
There is NO WARRANTY, to the extent permitted by law.\n\
\n\
Written by Vladimir Luchko. <%s>\n\
"
		, PACKAGE_NAME, PACKAGE_VERSION, 
		__DATE__, __TIME__, PACKAGE_BUGREPORT);
};

//------------------------------------------------------------

