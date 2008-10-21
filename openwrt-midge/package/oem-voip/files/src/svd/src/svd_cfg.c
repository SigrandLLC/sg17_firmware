/**
 * @file svd_cfg.c
 * Configuration implementation.
 * It contains startup \ref g_so and main \ref g_conf 
 * 		configuration features implementation.
 */ 

/*Includes {{{*/
#include "svd.h"
#include "libconfig.h"
#include "svd_cfg.h"
#include "svd_log.h"

#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <stdarg.h>
#include <errno.h>

#include <getopt.h>
/*}}}*/

/** @defgroup STARTUP_I Stratup internals.
 *  @ingroup STARTUP
 *  Internal startup definitons.
 *  @{*/
#define ERR_SUCCESS 0
#define ERR_MEMORY_FULL 1
#define ERR_UNKNOWN_OPTION 2
static unsigned char g_err_no;
/** @}*/ 


/** @defgroup CFG_N Config file text values.
 *  @ingroup CFG_M
 *  Text values that can be in config file.
 *  @{*/
#define CONF_CODEC_SPEED "speed"
#define CONF_CODEC_QUALITY "quality"
#define CONF_OOB_DEFAULT "default"
#define CONF_OOB_NO "in-band"
#define CONF_OOB_ONLY "out-of-band"
#define CONF_OOB_ALL "both"
#define CONF_OOB_BLOCK "block"
#define CONF_OOBPLAY_DEFAULT "default"
#define CONF_OOBPLAY_PLAY "play"
#define CONF_OOBPLAY_MUTE "mute"
#define CONF_OOBPLAY_APT_PLAY "play_diff_pt"
#define CONF_VAD_ON "on"
#define CONF_VAD_NOVAD "off"
#define CONF_VAD_G711 "g711"
#define CONF_VAD_CNG_ONLY "CNG_only"
#define CONF_VAD_SC_ONLY "SC_only"
/** @}*/ 

/** Common config.*/
static struct config_t cfg;
/** Routes config.*/
static struct config_t route_cfg;

/** @defgroup CFG_IF Config internal functions.
 *  @ingroup CFG_M
 *  This functions using while reading config file.
 *  @{*/
/** Init self router ip and number.*/
static int self_values_init (void);
/** Init logging configuration.*/
static void log_init (void);
/** Init SIP configuration.*/
static void sip_set_init (void);
/** Init addres book configuration.*/
static int address_book_init (void);
/** Init hot line configuration.*/
static int hot_line_init (void);
/** Init route table configuration.*/
static int route_table_init (void);
/** Init RTP parameters configuration.*/
static int rtp_prms_init (void);
/** Print error message if something occures.*/
static void error_message (void);
/** Print help message.*/
static void show_help (void);
/** Print version message.*/
static void show_version (void);
/** @}*/ 

/**
 * Init startup parameters structure \ref g_so.
 *
 * \param[in] argc parameters count.
 * \param[in] argv parameters values.
 * \retval 0 if etherything ok
 * \retval error_code if something nasty happens
 * 		\arg \ref ERR_MEMORY_FULL - not enough memory
 * 		\arg \ref ERR_UNKNOWN_OPTION - bad startup option.
 * \remark
 *		It sets help, version and debug tags to \ref g_so struct.
 */
int 
startup_init( int argc, char ** argv )
{/*{{{*/
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
}/*}}}*/
	
/**
 * 	Destroys startup parameters structure \ref g_so.
 *
 * \param[in] argc parameters count.
 * \param[in] argv parameters values.
 * \remark
 *		It prints necessary messages.
 */ 
void 
startup_destroy( int argc, char ** argv )
{/*{{{*/
	if( g_so.help ){
		show_help( );
	} else if( g_so.version ){
		show_version( );
	} 
	error_message( );
}/*}}}*/

/**
 *	Parses configuration file and write parsed info into \ref g_conf.
 *
 * \retval 0 success.
 * \retval -1 in error case.
 * \remark
 *		It init`s main routine configuration \ref g_conf.
 */ 
int 
svd_conf_init( void )
{/*{{{*/
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

	/* app.log */
	log_init();

	/* app.int_codec */
	str_elem = config_lookup_string (&cfg, "app.int_codec");
	if( !str_elem ){
		g_conf.int_codec = codec_type_QUALITY;
	} else if( !strcmp(str_elem, CONF_CODEC_SPEED) ){
		g_conf.int_codec = codec_type_SPEED;
	} else {
		g_conf.int_codec = codec_type_QUALITY;
	}
	
	/* app.rtp_port_first/last */
	g_conf.rtp_port_first = config_lookup_int (&cfg, "app.rtp_port_first");
	g_conf.rtp_port_last = config_lookup_int (&cfg, "app.rtp_port_last");
	if ((!g_conf.rtp_port_first) || (!g_conf.rtp_port_last) || 
			g_conf.rtp_port_first > g_conf.rtp_port_last ){
		SU_DEBUG_0(("rtp_port values was not set or set badly"));
		goto svd_conf_init__exit;
	}

	/* app.sip_set */
	sip_set_init();
	
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

	/* app.rtp_prms */
	err = rtp_prms_init();
	if( err ){
		goto svd_conf_init__exit;
	}

	/* self_number and self_ip init */
	err = self_values_init();
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
}/*}}}*/

/**
 * Show config parameters.
 *
 * \remark
 * 		It`s debug feature.
 */ 
void 
conf_show( void )
{/*{{{*/
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
		SU_DEBUG_3(("%d] : ", g_conf.log_level));
	}
	SU_DEBUG_3(("ports[%d:%d] : ", 
			g_conf.rtp_port_first,
			g_conf.rtp_port_last));
	
	switch(g_conf.int_codec){
		case codec_type_SPEED:
			SU_DEBUG_3(("[Use fast codecs]\n"));
			break;
		case codec_type_QUALITY:
			SU_DEBUG_3(("[Use best quality codecs]\n"));
			break;
	}

	SU_DEBUG_3(("SIP net : %d\n",g_conf.sip_set.all_set));
	if(g_conf.sip_set.all_set){
		SU_DEBUG_3((	"\tCodecs      : "));
		switch(g_conf.sip_set.ext_codec){
			case codec_type_SPEED:
				SU_DEBUG_3(("'Speed'\n"));
				break;
			case codec_type_QUALITY:
				SU_DEBUG_3(("'Quality'\n"));
				break;
		}
		SU_DEBUG_3((	"\tRegistRar   : '%s'\n"
				"\tUser/Pass   : '%s/%s'\n"
				"\tUser_URI    : '%s'\n"
				"\tSIP_channel : '%d'\n",
				g_conf.sip_set.registrar,
				g_conf.sip_set.user_name,
				g_conf.sip_set.user_pass,
				g_conf.sip_set.user_URI,
				g_conf.sip_set.sip_chan));
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
}/*}}}*/

/**
 * Free allocated memory for main configuration structure.
 */
void 
svd_conf_destroy (void)
{/*{{{*/
	int i;
	int j;

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
}/*}}}*/


/**
 * 	Init`s self ip and number settings in main routine configuration 
 *
 * \retval 0 success.
 * \retval -1 fail.
 */ 
static int 
self_values_init( void )
{/*{{{*/
	char ** addrmas = NULL;
	int addrs_count;
	int route_records_num;
	struct rttb_record_s * curr_rec;
	int i;
	int j = 0;
	int sock;

	/* get interfaces addresses on router */
	sock = socket (PF_INET, SOCK_STREAM, 0);
	if(sock == -1){
		SU_DEBUG_0 ((LOG_FNC_A(strerror(errno))));
		goto __exit_fail;
	}
	
	for (i=1;;i++){
		struct ifreq ifr;
		struct sockaddr_in *sin = (struct sockaddr_in *) &ifr.ifr_addr;
		char *ip;

		ifr.ifr_ifindex = i;
		if (ioctl (sock, SIOCGIFNAME, &ifr) < 0){
			break;
		}
		if (ioctl (sock, SIOCGIFADDR, &ifr) < 0){
			continue;
		}

		ip = inet_ntoa (sin->sin_addr);
		if( strcmp(ifr.ifr_name, "lo") ){
			addrmas = realloc (addrmas, sizeof(*addrmas)*(j+1));
			addrmas[j] = malloc(sizeof(char) * IP_LEN_MAX);
			memset(addrmas[j], 0, sizeof(char) * IP_LEN_MAX);
			strcpy(addrmas[j], ip);
			j++;
		}
	}
	close (sock);
	addrs_count = j;

	/* set self_ip and self_number according to route table and addrmas */
	route_records_num = g_conf.route_table.records_num;
	for (i=0; i<route_records_num; i++){
		curr_rec = &g_conf.route_table.records[i];
		for (j=0; j<addrs_count; j++){
			if (!strcmp (addrmas[j], curr_rec->value) ){
				g_conf.self_ip = curr_rec->value;
				g_conf.self_number = curr_rec->id;
			}
		}
	}

	/* free addrmas */
	for(i =0; i <addrs_count; i++){
		free (addrmas[i]);
	}
	free (addrmas);

        return 0;
__exit_fail:
        return -1;
}/*}}}*/

/**
 * Init`s log settings in main routine configuration \ref g_conf structure.
 */
static void
log_init( void )
{/*{{{*/
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
}/*}}}*/

/**
 * Init`s SIP settings in main routine configuration \ref g_conf structure.
 */
static void
sip_set_init( void )
{/*{{{*/
	char const * str_elem;

	g_conf.sip_set.all_set = 0;

	/* app.ext_codec */
	str_elem = config_lookup_string (&cfg, "app.ext_codec");
	if( !str_elem ){
		g_conf.sip_set.ext_codec = codec_type_SPEED;
	} else if( !strcmp(str_elem, CONF_CODEC_SPEED) ){
		g_conf.sip_set.ext_codec = codec_type_SPEED;
	} else {
		g_conf.sip_set.ext_codec = codec_type_QUALITY;
	} 

	/* app.sip_registrar */
	str_elem = config_lookup_string (&cfg, "app.sip_registrar");
	if( !str_elem ){
		goto __exit;
	}
	strncpy (g_conf.sip_set.registrar, str_elem, REGISTRAR_LEN);

	/* app.sip_username */
	str_elem = config_lookup_string (&cfg, "app.sip_username");
	if( !str_elem ){
		goto __exit;
	}
	strncpy (g_conf.sip_set.user_name, str_elem, USER_NAME_LEN);

	/* app.sip_password */
	str_elem = config_lookup_string (&cfg, "app.sip_password");
	if( !str_elem ){
		goto __exit;
	}
	strncpy (g_conf.sip_set.user_pass, str_elem, USER_PASS_LEN);

	/* app.sip_uri */
	str_elem = config_lookup_string (&cfg, "app.sip_uri");
	if( !str_elem ){
		goto __exit;
	}
	strncpy (g_conf.sip_set.user_URI, str_elem, USER_URI_LEN);

	/* app.sip_chan */
	g_conf.sip_set.sip_chan = config_lookup_int (&cfg, "app.sip_chan");
	if ( !g_conf.sip_set.sip_chan){
		goto __exit;
	}

	g_conf.sip_set.all_set = 1;
__exit:
	return;
}/*}}}*/

/**
 * Init`s address book records in main routine configuration 
 * 		\ref g_conf structure.
 *
 * \retval 0 success.
 * \retval -1 fail.
 */ 
static int
address_book_init( void )
{/*{{{*/
	struct config_setting_t * set;
	struct config_setting_t * rec_set;
	struct adbk_record_s * curr_rec;
	char const * elem;
	int elem_len;
	int rec_num;
	char use_id_local_buf = 1;
	int id_len;
	int i;

	set = config_lookup (&cfg, "app.address_book" );
	if( !set ){
		g_conf.address_book.records_num = 0;
		goto address_book_init__exit_success;
	} 

	rec_num = config_setting_length (set);

	g_conf.address_book.records_num = rec_num;
	g_conf.address_book.records = malloc (rec_num * 
			sizeof(*(g_conf.address_book.records)));
	if( !g_conf.address_book.records ){
		SU_DEBUG_0((LOG_FNC_A(LOG_NOMEM)));
		goto address_book_init__exit;
	}
	memset(g_conf.address_book.records, 0, rec_num * 
			sizeof(*(g_conf.address_book.records)));

	rec_set = config_setting_get_elem (set, 0);
	elem = config_setting_get_string_elem (rec_set, 0);
	id_len = strlen(elem);

	g_conf.address_book.id_len = id_len;

	if (id_len + 1 > ADBK_ID_LEN_DF){
		use_id_local_buf = 0;
	}

	for(i = 0; i < rec_num; i++){
		curr_rec = &g_conf.address_book.records[ i ];
		rec_set = config_setting_get_elem (set, i);

		/* get id */
		elem = config_setting_get_string_elem (rec_set, 0);

		if (use_id_local_buf){
			curr_rec->id = curr_rec->id_s;
		} else {
			curr_rec->id = malloc( (id_len + 1) *
					sizeof(*(curr_rec->id)));
			if( !curr_rec->id ){
				SU_DEBUG_0((LOG_FNC_A(LOG_NOMEM)));
				goto address_book_init__exit;
			}
		}
		strcpy(curr_rec->id, elem);
		
		/* get value */
		elem = config_setting_get_string_elem (rec_set, 1);
		elem_len = strlen(elem);
		if (elem_len+1 < VALUE_LEN_DF ){
			curr_rec->value = curr_rec->value_s;
		} else {
			curr_rec->value = malloc((elem_len+1) *
					sizeof(*(curr_rec->value)));
			if( !curr_rec->value ){
				SU_DEBUG_0((LOG_FNC_A(LOG_NOMEM)));
				goto address_book_init__id_alloc;
			}
		}
		strcpy (curr_rec->value, elem);
	}

address_book_init__exit_success:
	return 0;

address_book_init__id_alloc:
	if( curr_rec->id && curr_rec->id != curr_rec->id_s ){
		free (curr_rec->id);
	}
address_book_init__exit:
	return -1;
}/*}}}*/


/**
 * Init`s hot line records in main routine configuration \ref g_conf structure.
 *
 * \retval 0 success.
 * \retval -1 fail.
 */ 
static int 
hot_line_init( void )
{/*{{{*/
	struct config_setting_t * set;
	struct config_setting_t * rec_set;
	struct htln_record_s * curr_rec; 
	char const * elem;
	int elem_len;
	int rec_num;
	int i;

	set = config_lookup (&cfg, "app.hot_line" );
	if( !set ){
		g_conf.hot_line.records_num = 0;
		goto hot_line_init__exit_success;
	} 

	rec_num = config_setting_length (set);

	g_conf.hot_line.records_num = rec_num;
	g_conf.hot_line.records = malloc (rec_num * 
			sizeof(*(g_conf.hot_line.records)));
	if( !g_conf.hot_line.records ){
		SU_DEBUG_0((LOG_FNC_A(LOG_NOMEM)));
		goto hot_line_init__exit;
	}
	memset(g_conf.hot_line.records, 0, rec_num * 
			sizeof(*(g_conf.hot_line.records)));

	for(i = 0; i < rec_num; i++){
		curr_rec = &g_conf.hot_line.records[ i ];
		rec_set = config_setting_get_elem (set, i);
		/* get id */
		elem = config_setting_get_string_elem (rec_set, 0);
		strncpy(curr_rec->id, elem, CHAN_ID_LEN-1);
		
		/* get value with no garbage */
		elem = config_setting_get_string_elem (rec_set, 1);
		elem_len = strlen(elem);
		if (elem_len+1 < VALUE_LEN_DF ){
			curr_rec->value = curr_rec->value_s;
		} else {
			curr_rec->value = malloc((elem_len+1) *
					sizeof(*(curr_rec->value)));
			if( !curr_rec->value ){
				SU_DEBUG_0((LOG_FNC_A(LOG_NOMEM)));
				goto hot_line_init__exit;
			}
		}
		strcpy (curr_rec->value, elem);
	}

hot_line_init__exit_success:
	return 0;
hot_line_init__exit:
	return -1;
}/*}}}*/


/**
 * Init`s route table in main routine configuration \ref g_conf structure.
 *
 * \retval 0 success.
 * \retval -1 fail.
 */ 
static int
route_table_init( void )
{/*{{{*/
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
}/*}}}*/

/**
 * Init`s RTP parameters in main routine configuration \ref g_conf structure.
 *
 * \retval 0 success.
 * \retval -1 fail.
 */ 
static int
rtp_prms_init( void )
{/*{{{*/
	struct config_setting_t * set;
	struct config_setting_t * rec_set;
	struct rtp_record_s * curr_rec;
	char const * elem;
	int rec_num;
	int i;

	/* Get values */
	set = config_lookup (&cfg, "app.rtp_prms" );
	if( !set ){
		/* We will use statndart params for all channels */
		goto __exit_success;
	} 

	rec_num = config_setting_length (set);

	g_conf.rtp_prms.records_num = rec_num;
	g_conf.rtp_prms.records = malloc (rec_num * 
			sizeof(*(g_conf.rtp_prms.records)));
	if( !g_conf.rtp_prms.records ){
		SU_DEBUG_0((LOG_FNC_A(LOG_NOMEM)));
		goto __exit_fail;
	}
	memset(g_conf.rtp_prms.records, 0, rec_num * 
			sizeof(*(g_conf.rtp_prms.records)));

	for(i=0; i<rec_num; i++){
		curr_rec = &g_conf.rtp_prms.records[ i ];
		rec_set = config_setting_get_elem (set, i);

		/* get chan id */
		elem = config_setting_get_string_elem (rec_set, 0);
		strcpy(curr_rec->id, elem);
		
		/* get rtp params */
		elem = config_setting_get_string_elem (rec_set, 1);
		if( !strcmp(elem, CONF_OOB_DEFAULT)){
			curr_rec->OOB = evts_OOB_DEFAULT;
		} else if( !strcmp(elem, CONF_OOB_NO)){
			curr_rec->OOB = evts_OOB_NO;
		} else if( !strcmp(elem, CONF_OOB_ONLY)){
			curr_rec->OOB = evts_OOB_ONLY;
		} else if( !strcmp(elem, CONF_OOB_ALL)){
			curr_rec->OOB = evts_OOB_ALL;
		} else if( !strcmp(elem, CONF_OOB_BLOCK)){
			curr_rec->OOB = evts_OOB_BLOCK;
		}

		elem = config_setting_get_string_elem (rec_set, 2);
		if( !strcmp(elem, CONF_OOBPLAY_DEFAULT)){
			curr_rec->OOB_play = play_evts_DEFAULT;
		} else if( !strcmp(elem, CONF_OOBPLAY_PLAY)){
			curr_rec->OOB_play = play_evts_PLAY;
		} else if( !strcmp(elem, CONF_OOBPLAY_MUTE)){
			curr_rec->OOB_play = play_evts_MUTE;
		} else if( !strcmp(elem, CONF_OOBPLAY_APT_PLAY)){
			curr_rec->OOB_play = play_evts_APT_PLAY;
		}

		curr_rec->evtPT = config_setting_get_int_elem(rec_set, 3);
		curr_rec->evtPTplay = config_setting_get_int_elem(rec_set, 4);
		curr_rec->COD_Tx_vol = config_setting_get_int_elem(rec_set, 5);
		curr_rec->COD_Rx_vol = config_setting_get_int_elem(rec_set, 6);
		elem = config_setting_get_string_elem (rec_set, 7);
		if( !strcmp(elem, CONF_VAD_NOVAD)){
			curr_rec->VAD_cfg = vad_cfg_OFF;
		} else if( !strcmp(elem, CONF_VAD_ON)){
			curr_rec->VAD_cfg = vad_cfg_ON;
		} else if( !strcmp(elem, CONF_VAD_G711)){
			curr_rec->VAD_cfg = vad_cfg_G711;
		} else if( !strcmp(elem, CONF_VAD_CNG_ONLY)){
			curr_rec->VAD_cfg = vad_cfg_CNG_only;
		} else if( !strcmp(elem, CONF_VAD_SC_ONLY)){
			curr_rec->VAD_cfg = vad_cfg_SC_only;
		}
		curr_rec->HPF_is_ON = config_setting_get_int_elem(rec_set, 8);
	}
__exit_success:
	return 0;
__exit_fail:
	return -1;
}/*}}}*/

/**
 * Show error if something nasty happens.
 */
static void 
error_message( void )
{/*{{{*/
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
}/*}}}*/

//------------------------------------------------------------

/**
 * Show help message.
 */
static void 
show_help( void )
{/*{{{*/
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
		, PACKAGE_NAME, PACKAGE_NAME, PACKAGE_BUGREPORT );
}/*}}}*/

//------------------------------------------------------------

/**
 * Show program version, built info and license.
 */
static void 
show_version( void )
{/*{{{*/
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
}/*}}}*/

//------------------------------------------------------------

