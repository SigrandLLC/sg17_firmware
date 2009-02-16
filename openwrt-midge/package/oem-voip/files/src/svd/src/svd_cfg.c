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
#define CONF_CODEC_G729 "g729"
#define CONF_CODEC_ALAW "aLaw"
#define CONF_CODEC_MLAW "uLaw"
#define CONF_CODEC_G723 "g723"
#define CONF_CODEC_ILBC133 "iLBC_133"
/*#define CONF_CODEC_ILBC152 "iLBC_152"*/
#define CONF_CODEC_G729E "g729e"
#define CONF_CODEC_G72616 "g726_16"
#define CONF_CODEC_G72624 "g726_24"
#define CONF_CODEC_G72632 "g726_32"
#define CONF_CODEC_G72640 "g726_40"
#define CONF_CODEC_BITPACK_RTP "rtp"
#define CONF_CODEC_BITPACK_AAL2 "aal2"
#define CONF_WIRETYPE_2 "2w"
#define CONF_WIRETYPE_4 "4w"
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

/** @defgroup CFG_IF Config internal functions.
 *  @ingroup CFG_M
 *  This functions using while reading config file.
 *  @{*/
/** Init self router ip and number.*/
static int self_values_init (void);
/** Init codec_t from rec_set. */
static void init_codec_el(struct config_setting_t const * const rec_set, 
		codec_t * const cod);
/** Init main configuration.*/
static int main_init (void);
/** Init route table configuration.*/
static int routet_init (void);
/** Init RTP parameters configuration.*/
static int rtp_init (void);
/** Init hard link configuration.*/
static int hardlink_init (void);
/** Init hot line configuration.*/
static int hotline_init (void);
/** Init addres book configuration.*/
static int addressb_init (void);
/** Init internal, external and fax codecs configuration.*/
static int quality_init (void);
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
	memset (&g_conf, 0, sizeof(g_conf));
	if(		main_init() 	||
			routet_init()	||
			rtp_init()		||
			hardlink_init() ||
			hotline_init()	||
			addressb_init()	||
			quality_init()
			){
		goto __exit_fail;
	}

	conf_show();
	return 0;

__exit_fail:
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
	struct hard_link_s   * curr_hk_rec;
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
	
	for (i=0; g_conf.int_codecs[i].type != cod_type_NONE; i++){
		SU_DEBUG_3(("t:%d/sz%d/pt:0x%X\n",
				g_conf.int_codecs[i].type,
				g_conf.int_codecs[i].pkt_size,
				g_conf.int_codecs[i].user_payload));
	} 

	SU_DEBUG_3(("SIP net : %d\n",g_conf.sip_set.all_set));
	if(g_conf.sip_set.all_set){
		SU_DEBUG_3((	"\tCodecs:\n"));
		for (i=0; g_conf.sip_set.ext_codecs[i].type != cod_type_NONE; i++){
			SU_DEBUG_3(("t:%d/sz%d/pt:0x%X\n",
					g_conf.sip_set.ext_codecs[i].type,
					g_conf.sip_set.ext_codecs[i].pkt_size,
					g_conf.sip_set.ext_codecs[i].user_payload));
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

	SU_DEBUG_3(("HardLinks :\n"));
	for(i=0; i<CHANS_MAX; i++){
		curr_hk_rec = &g_conf.hard_link[ i ];
		if(curr_hk_rec->type == hl_type_UNDEFINED){
			continue;
		} else if(curr_hk_rec->type == hl_type_2_WIRED){
			SU_DEBUG_3(("\t2w/"));
		} else if(curr_hk_rec->type == hl_type_4_WIRED){
			SU_DEBUG_3(("\t4w/"));
		}
		if(curr_hk_rec->hl_codec.type == cod_type_MLAW){
			SU_DEBUG_3(("uL/"));
		} else if(curr_hk_rec->hl_codec.type == cod_type_ALAW){
			SU_DEBUG_3(("aL/"));
		}
		SU_DEBUG_3(("s%d/up%d/",
				curr_hk_rec->hl_codec.pkt_size,
				curr_hk_rec->hl_codec.user_payload));

		SU_DEBUG_3(("i%d/id\"%s\":%s:%s/aic_%d\n",
				i, curr_hk_rec->id, 
				curr_hk_rec->pair_route, curr_hk_rec->pair_chan,
				curr_hk_rec->am_i_caller));
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
 * \param[out] cp codecs parameters.
 * \retval 0 Success;
 * \retval -1 Fail;
 * \remark
 * 	\c cp should be a pointer on the allocated memory for \c COD_MAS_SIZE
 * 	elements.
 */ 
int 
svd_init_cod_params( cod_prms_t * const cp )
{/*{{{*/
	int i;
	memset(cp, 0, sizeof(*cp)*COD_MAS_SIZE);

	for (i=0; i<COD_MAS_SIZE; i++){
		cp[i].type = cod_type_NONE;
	} 

	i=0;

	/* G711 ALAW parameters. */
	cp[i].type = cod_type_ALAW;
	if(strlen("PCMA") >= COD_NAME_LEN){
		goto __exit_fail;
	}
	strcpy(cp[i].sdp_name, "PCMA");
	cp[i].rate = 8000;
	i++;
	
	/* G711 MLAW parameters. */
	cp[i].type = cod_type_MLAW;
	if(strlen("PCMU") >= COD_NAME_LEN){
		goto __exit_fail;
	}
	strcpy(cp[i].sdp_name, "PCMU");
	cp[i].rate = 8000;
	i++;

	/* G729 parameters. */
	cp[i].type = cod_type_G729;
	if(strlen("G729") >= COD_NAME_LEN){
		goto __exit_fail;
	}
	strcpy(cp[i].sdp_name, "G729");
	cp[i].rate = 8000;
	i++;

	/* G729E parameters. */
	cp[i].type = cod_type_G729E;
	if(strlen("G729E") >= COD_NAME_LEN){
		goto __exit_fail;
	}
	strcpy(cp[i].sdp_name, "G729E");
	cp[i].rate = 8000;
	i++;

	/* G723 parameters. */
	cp[i].type = cod_type_G723;
	if(strlen("G723") >= COD_NAME_LEN){
		goto __exit_fail;
	}
	strcpy(cp[i].sdp_name, "G723");
	cp[i].rate = 8000;
	i++;

	/* iLBC_133 parameters. */
	cp[i].type = cod_type_ILBC_133;
	if(     strlen("iLBC") >= COD_NAME_LEN ||
			strlen("mode=30") >= FMTP_STR_LEN){
		goto __exit_fail;
	}
	strcpy(cp[i].sdp_name, "iLBC");
	strcpy(cp[i].fmtp_str, "mode=30");
	cp[i].rate = 8000;
	i++;

	/* iLBC_152 parameters.
	cp[i].type = cod_type_ILBC_152;
	if(     strlen("iLBC") >= COD_NAME_LEN ||
			strlen("mode=20") >= FMTP_STR_LEN){
		goto __exit_fail;
	}
	strcpy(cp[i].sdp_name, "iLBC");
	strcpy(cp[i].fmtp_str, "mode=20");
	cp[i].rate = 8000;
	i++;
	*/

	/* G726_16 parameters. */
	cp[i].type = cod_type_G726_16;
	if(strlen("G726-16") >= COD_NAME_LEN){
		goto __exit_fail;
	}
	strcpy(cp[i].sdp_name, "G726-16");
	cp[i].rate = 8000;
	i++;

	/* G726_ parameters. */
	cp[i].type = cod_type_G726_24;
	if(strlen("G726-24") >= COD_NAME_LEN){
		goto __exit_fail;
	}
	strcpy(cp[i].sdp_name, "G726-24");
	cp[i].rate = 8000;
	i++;

	/* G726_ parameters. */
	cp[i].type = cod_type_G726_32;
	if(strlen("G726-32") >= COD_NAME_LEN){
		goto __exit_fail;
	}
	strcpy(cp[i].sdp_name, "G726-32");
	cp[i].rate = 8000;
	i++;

	/* G726_ parameters. */
	cp[i].type = cod_type_G726_40;
	if(strlen("G726-40") >= COD_NAME_LEN){
		goto __exit_fail;
	}
	strcpy(cp[i].sdp_name, "G726-40");
	cp[i].rate = 8000;

	return 0;
__exit_fail:
	return -1;
}/*}}}*/

/**
 * 	Init`s self ip and number settings in main routine configuration 
 *
 * \retval 0 success.
 * \retval -1 fail.
 * \remark
 * 	It should be run after route table initialization because
 * 	it uses this value for self values pointed to.
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
	int self_found;

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
	self_found = 0;
	route_records_num = g_conf.route_table.records_num;
	for (i=0; i<route_records_num; i++){
		curr_rec = &g_conf.route_table.records[i];
		for (j=0; j<addrs_count; j++){
			if (!strcmp (addrmas[j], curr_rec->value) ){
				g_conf.self_ip = curr_rec->value;
				g_conf.self_number = curr_rec->id;
				self_found = 1;
			}
		}
	}

	/* free addrmas */
	for(i =0; i <addrs_count; i++){
		free (addrmas[i]);
	}
	free (addrmas);

	if( !self_found){
		SU_DEBUG_0 ((LOG_FNC_A("ERROR: "
				"No interfaces with ip from route table")));
		goto __exit_fail;
	}
	return 0;
__exit_fail:
	return -1;
}/*}}}*/

/**
 * Initilize one codec element from appropriate config setting.
 *
 * \param[in] rec_set config setting.
 * \param[out] cod codec_t element to initilize.
 */ 
static void
init_codec_el(struct config_setting_t const *const rec_set, codec_t *const cod)
{/*{{{*/
	char const * codel = NULL;
	/* codec type */
	codel = config_setting_get_string_elem (rec_set, 0);
	if       ( !strcmp(codel, CONF_CODEC_G729)){
		cod->type = cod_type_G729;
	} else if( !strcmp(codel, CONF_CODEC_ALAW)){
		cod->type = cod_type_ALAW;
	} else if( !strcmp(codel, CONF_CODEC_MLAW)){
		cod->type = cod_type_MLAW;
	} else if( !strcmp(codel, CONF_CODEC_G723)){
		cod->type = cod_type_G723;
	} else if( !strcmp(codel, CONF_CODEC_ILBC133)){
		cod->type = cod_type_ILBC_133;
		/*
	} else if( !strcmp(codel, CONF_CODEC_ILBC152)){
		cod->type = cod_type_ILBC_152;
		*/
	} else if( !strcmp(codel, CONF_CODEC_G729E)){
		cod->type = cod_type_G729E;
	} else if( !strcmp(codel, CONF_CODEC_G72616)){
		cod->type = cod_type_G726_16;
	} else if( !strcmp(codel, CONF_CODEC_G72624)){
		cod->type = cod_type_G726_24;
	} else if( !strcmp(codel, CONF_CODEC_G72632)){
		cod->type = cod_type_G726_32;
	} else if( !strcmp(codel, CONF_CODEC_G72640)){
		cod->type = cod_type_G726_40;
	}

	/* codec packet size */
	codel = config_setting_get_string_elem (rec_set, 1);
	if       ( !strcmp(codel, "2.5")){
		cod->pkt_size = cod_pkt_size_2_5;
	} else if( !strcmp(codel, "5")){
		cod->pkt_size = cod_pkt_size_5;
	} else if( !strcmp(codel, "5.5")){
		cod->pkt_size = cod_pkt_size_5_5;
	} else if( !strcmp(codel, "10")){
		cod->pkt_size = cod_pkt_size_10;
	} else if( !strcmp(codel, "11")){
		cod->pkt_size = cod_pkt_size_11;
	} else if( !strcmp(codel, "20")){
		cod->pkt_size = cod_pkt_size_20;
	} else if( !strcmp(codel, "30")){
		cod->pkt_size = cod_pkt_size_30;
	} else if( !strcmp(codel, "40")){
		cod->pkt_size = cod_pkt_size_40;
	} else if( !strcmp(codel, "50")){
		cod->pkt_size = cod_pkt_size_50;
	} else if( !strcmp(codel, "60")){
		cod->pkt_size = cod_pkt_size_60;
	}

	/* codec payload type */
	cod->user_payload = config_setting_get_int_elem (rec_set, 2);

	/* codec bitpack */
	codel = config_setting_get_string_elem (rec_set, 3);
	if( !codel){
		SU_DEBUG_2(("No BITPACK entries for some codecs!!!"));
		return;
	}
	if       ( !strcmp(codel, CONF_CODEC_BITPACK_RTP)){
		cod->bpack = bitpack_RTP;
	} else if( !strcmp(codel, CONF_CODEC_BITPACK_AAL2)){
		cod->bpack = bitpack_AAL2;
	}
}/*}}}*/

/**
 * Init`s main settings in main routine configuration \ref g_conf structure.
 *
 * \retval 0 success.
 * \retval -1 fail.
 */
static int
main_init( void )
{/*{{{*/
	struct config_t cfg;
	char const * str_elem = NULL;
	int err;

	config_init (&cfg);

	/* Load the file */
	if (!config_read_file (&cfg, MAIN_CONF_NAME)){
		err = config_error_line (&cfg);
		SU_DEBUG_0(("%s(): Config file syntax error in line %d\n",
				__func__, err));
		goto __exit_fail;
	} 

	/* log */
	g_conf.log_level = config_lookup_int (&cfg, "log");

	/* rtp_port_first/last */
	g_conf.rtp_port_first = config_lookup_int (&cfg, "rtp_port_first");
	g_conf.rtp_port_last = config_lookup_int (&cfg, "rtp_port_last");

	/* SIP settings */
	g_conf.sip_set.all_set = 0;

	/* app.sip_registrar */
	str_elem = config_lookup_string (&cfg, "sip_registrar");
	if( !str_elem ){
		goto __exit_success;
	}
	strncpy (g_conf.sip_set.registrar, str_elem, REGISTRAR_LEN);

	/* app.sip_username */
	str_elem = config_lookup_string (&cfg, "sip_username");
	if( !str_elem ){
		goto __exit_success;
	}
	strncpy (g_conf.sip_set.user_name, str_elem, USER_NAME_LEN);

	/* app.sip_password */
	str_elem = config_lookup_string (&cfg, "sip_password");
	if( !str_elem ){
		goto __exit_success;
	}
	strncpy (g_conf.sip_set.user_pass, str_elem, USER_PASS_LEN);

	/* app.sip_uri */
	str_elem = config_lookup_string (&cfg, "sip_uri");
	if( !str_elem ){
		goto __exit_success;
	}
	strncpy (g_conf.sip_set.user_URI, str_elem, USER_URI_LEN);

	/* app.sip_chan */
	g_conf.sip_set.sip_chan = config_lookup_int (&cfg, "sip_chan");
	if ( !g_conf.sip_set.sip_chan){
		goto __exit_success;
	}

	g_conf.sip_set.all_set = 1;

__exit_success:
	config_destroy (&cfg);
	return 0;
__exit_fail:
	config_destroy (&cfg);
	return err;
}/*}}}*/

/**
 * Init`s route table in main routine configuration \ref g_conf structure.
 *
 * \retval 0 success.
 * \retval -1 fail.
 */ 
static int
routet_init( void )
{/*{{{*/
	struct config_t cfg;
	struct config_setting_t * set;
	struct config_setting_t * rec_set;
	struct rttb_record_s * curr_rec;
	char const * elem;
	int rec_num;
	char use_id_local_buf = 1;
	int id_len;
	int i;
	int err;

	config_init (&cfg);

	/* Load the file */
	if (!config_read_file (&cfg, ROUTET_CONF_NAME)){
		err = config_error_line (&cfg);
		SU_DEBUG_0(("%s(): Config file syntax error in line %d\n",
				__func__, err));
		goto __exit_fail;
	} 

	/* Get values */
	set = config_lookup (&cfg, "route_table" );
	if( !set ){
		SU_DEBUG_0 ((LOG_FNC_A("no data")));
		goto __exit_fail;
	} 

	rec_num = config_setting_length (set);

	g_conf.route_table.records_num = rec_num;
	g_conf.route_table.records = malloc (
			rec_num * sizeof(*(g_conf.route_table.records)));
	if( !g_conf.route_table.records ){
		SU_DEBUG_0((LOG_FNC_A(LOG_NOMEM)));
		goto __exit_fail;
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
			curr_rec->id = malloc((id_len + 1)* sizeof(*(curr_rec->id)));
			if( !curr_rec->id ){
				SU_DEBUG_0 ((LOG_FNC_A(LOG_NOMEM)));
				goto __exit_fail;
			}
		}
		strcpy(curr_rec->id, elem);
		
		/* get value */
		elem = config_setting_get_string_elem (rec_set, 1);
		strcpy (curr_rec->value, elem);
	}

	err = self_values_init();
	if(err){
		goto __exit_fail;
	}

	config_destroy (&cfg);
	return 0;
__exit_fail:
	if(g_conf.route_table.records) {
		if( !use_id_local_buf){
			for(i=0; i<rec_num; i++){
				curr_rec = &g_conf.route_table.records[ i ];
				if( curr_rec->id && curr_rec->id != curr_rec->id_s ){
					free (curr_rec->id);
					curr_rec->id = NULL;
				}
			}
		}
		free(g_conf.address_book.records);
		g_conf.address_book.records = NULL;
	}
	config_destroy (&cfg);
	return -1;
}/*}}}*/

/**
 * Init`s hard link records in main routine configuration \ref g_conf structure.
 *
 * \retval 0 success.
 * \retval -1 fail.
 * \remark
 * 	It should be run after routes table initialization because
 * 	it uses this value for am_i_caller flag set. And after rtp_init, 
 * 	because it can reinits it`s values.
 */ 
static int 
hardlink_init( void )
{/*{{{*/
	/* ("w_type", "chan_id", "pair_route_id", "pair_chan_id",
	 * 		codec_name, pkt_sz, vol_tx, vol_rx), */
	struct config_t cfg;
	struct config_setting_t * set;
	struct config_setting_t * rec_set;
	struct hard_link_s * curr_rec; 
	char const * elem;
	int elem_len;
	int rec_num;
	int i;
	int err;

	config_init (&cfg);

	/* Load the file */
	if (!config_read_file (&cfg, HARDLINK_CONF_NAME)){
		err = config_error_line (&cfg);
		SU_DEBUG_0(("%s(): Config file syntax error in line %d\n",
				__func__, err));
		goto __exit_fail;
	} 

	set = config_lookup (&cfg, "hard_link" );
	if( !set){
		/* no hardlinked channels */
		goto __exit_success;
	} 

	rec_num = config_setting_length (set);

	if(rec_num > CHANS_MAX){
		SU_DEBUG_0(("%s(): Too many channels (%d) in config - max is %d\n",
				__func__, rec_num, CHANS_MAX));
		goto __exit_fail;
	}

	for(i=0; i<rec_num; i++){
		int chan_id;
		int pair_chan;
		int router_is_self;

		rec_set = config_setting_get_elem (set, i);

		/* get chan_id */
		elem = config_setting_get_string_elem (rec_set, 1);
		chan_id = strtol (elem, NULL, 10);

		/* get pair_chan_id */
		elem = config_setting_get_string_elem (rec_set, 3);
		pair_chan = strtol (elem, NULL, 10);

		/* get wire type */
		elem = config_setting_get_string_elem (rec_set, 0);
		if( !strcmp(elem, CONF_WIRETYPE_2)){
			curr_rec = &g_conf.hard_link[ chan_id ];
			curr_rec->type = hl_type_2_WIRED;
		} else if( !strcmp(elem, CONF_WIRETYPE_4)){
			/* we should choise proper channels (even_odd_pair) */
			if( chan_id%2){
				/* if chan is odd (0X) - make it even (X0) */
				chan_id--;
			}
			if( !(pair_chan%2)){
				/* if pair is even (X0)* - make it odd (0X) */
				pair_chan++;
			}
			curr_rec = &g_conf.hard_link[ chan_id ];
			curr_rec->type = hl_type_4_WIRED;
		}

		/* set chan_id to rec */
		fprintf(stderr,"i%d, ch=%d, pch=%d\n", i, chan_id, pair_chan);
		snprintf(curr_rec->id, CHAN_ID_LEN, "%02d", chan_id);

		/* set pair_chan chan to rec */
		snprintf(curr_rec->pair_chan, CHAN_ID_LEN, "%02d", pair_chan);

		/* get pair_route_id */
		elem = config_setting_get_string_elem (rec_set, 2);
		elem_len = strlen(elem);
		if (elem_len+1 < ROUTE_ID_LEN_DF){
			curr_rec->pair_route = curr_rec->pair_route_s;
		} else {
			curr_rec->pair_route = malloc(
					(elem_len+1)*sizeof(*(curr_rec->pair_route)));
			if( !curr_rec->pair_route){
				SU_DEBUG_0((LOG_FNC_A(LOG_NOMEM)));
				goto __exit_fail;
			}
		}
		strcpy (curr_rec->pair_route, elem);

		/* set am_i_caller flag by parsing the value string */
		if(curr_rec->pair_route[0] == SELF_MARKER){
			strcpy(curr_rec->pair_route, g_conf.self_number);
		}
		router_is_self = !strcmp(curr_rec->pair_route, g_conf.self_number);
		if (router_is_self){
			/* test what chan is greater */
			int ret = chan_id - pair_chan;
			if       (ret > 0){
				/* current is greater */
				curr_rec->am_i_caller = 1;
			} else if(ret < 0){
				/* address is greater */
				curr_rec->am_i_caller = 0;
			} else {
				/* error - can`t hardlink to self */
				SU_DEBUG_1((LOG_FNC_A("ERROR: can`t hardlink to self")));
				goto __exit_fail;
			}
		} else {
			/* test what router is greater */
			int ret = strcmp(g_conf.self_number, curr_rec->pair_route);
			if       (ret > 0){
				/* current is greater */
				curr_rec->am_i_caller = 1;
			} else if(ret < 0){
				/* address is greater */
				curr_rec->am_i_caller = 0;
			} else {
				/* error - can`t hardlink to self */
				SU_DEBUG_1((LOG_FNC_A("ERROR:internal-bad router self test")));
				goto __exit_fail;
			}
		}

		/* get codec name and payload set */
		elem = config_setting_get_string_elem (rec_set, 4);
		if( !strcmp(elem, CONF_CODEC_ALAW)){
			curr_rec->hl_codec.type = cod_type_ALAW;
		} else if( !strcmp(elem, CONF_CODEC_MLAW)){
			curr_rec->hl_codec.type = cod_type_MLAW;
		} 

		/* get packet size */
		elem = config_setting_get_string_elem (rec_set, 5);
		if       ( !strcmp(elem, "2.5")){
			curr_rec->hl_codec.pkt_size = cod_pkt_size_2_5;
		} else if( !strcmp(elem, "5")){
			curr_rec->hl_codec.pkt_size = cod_pkt_size_5;
		} else if( !strcmp(elem, "5.5")){
			curr_rec->hl_codec.pkt_size = cod_pkt_size_5_5;
		} else if( !strcmp(elem, "10")){
			curr_rec->hl_codec.pkt_size = cod_pkt_size_10;
		} else if( !strcmp(elem, "11")){
			curr_rec->hl_codec.pkt_size = cod_pkt_size_11;
		} else if( !strcmp(elem, "20")){
			curr_rec->hl_codec.pkt_size = cod_pkt_size_20;
		} else if( !strcmp(elem, "30")){
			curr_rec->hl_codec.pkt_size = cod_pkt_size_30;
		} else if( !strcmp(elem, "40")){
			curr_rec->hl_codec.pkt_size = cod_pkt_size_40;
		} else if( !strcmp(elem, "50")){
			curr_rec->hl_codec.pkt_size = cod_pkt_size_50;
		} else if( !strcmp(elem, "60")){
			curr_rec->hl_codec.pkt_size = cod_pkt_size_60;
		}

		/* bit pack set */
		curr_rec->hl_codec.bpack = bitpack_RTP;

		/* set rtp parameters */
		g_conf.rtp_prms[chan_id].is_set = 1;
		g_conf.rtp_prms[chan_id].OOB = evts_OOB_NO;
		g_conf.rtp_prms[chan_id].OOB_play = play_evts_MUTE;
		g_conf.rtp_prms[chan_id].evtPT = 0x62;
		g_conf.rtp_prms[chan_id].evtPTplay = 0x62;
		g_conf.rtp_prms[chan_id].COD_Tx_vol= 
				config_setting_get_int_elem (rec_set, 5);
		g_conf.rtp_prms[chan_id].COD_Rx_vol = 
				config_setting_get_int_elem (rec_set, 6);
		g_conf.rtp_prms[chan_id].VAD_cfg = vad_cfg_OFF;
		g_conf.rtp_prms[chan_id].HPF_is_ON = 0;

		if (curr_rec->type == hl_type_4_WIRED){
			/* add record for this channel and for the second stream */
			/* copy rtp_params */
			memcpy(&g_conf.rtp_prms[chan_id+1], &g_conf.rtp_prms[chan_id], 
					sizeof(g_conf.rtp_prms[chan_id]));

			/* copy hardlinked params */
			memcpy(&g_conf.hard_link[ chan_id+1 ], 
					&g_conf.hard_link[ chan_id ],
					sizeof(g_conf.hard_link[chan_id]));

			/* revert channels for feedback chan */
			snprintf (g_conf.hard_link[ chan_id+1 ].id,
					CHAN_ID_LEN, "%02d", chan_id+1);
			snprintf (g_conf.hard_link[ chan_id+1 ].pair_chan,
					CHAN_ID_LEN, "%02d", pair_chan-1);
		}
	}

__exit_success:
	config_destroy (&cfg);
	return 0;
__exit_fail:
	for(i=0; i<CHANS_MAX; i++){
		curr_rec = &g_conf.hard_link[ i ];
		if( curr_rec->type != hl_type_UNDEFINED && 
				curr_rec->pair_route && 
				curr_rec->pair_route != curr_rec->pair_route_s ){
			free (curr_rec->pair_route);
			curr_rec->pair_route = NULL;
		}
	}
	config_destroy (&cfg);
	return -1;
}/*}}}*/

/**
 * Init`s hot line records in main routine configuration \ref g_conf structure.
 *
 * \retval 0 success.
 * \retval -1 fail.
 */ 
static int 
hotline_init( void )
{/*{{{*/
	struct config_t cfg;
	struct config_setting_t * set;
	struct config_setting_t * rec_set;
	struct htln_record_s * curr_rec; 
	char const * elem;
	int elem_len;
	int rec_num;
	int i;
	int err;

	config_init (&cfg);

	/* Load the file */
	if (!config_read_file (&cfg, HOTLINE_CONF_NAME)){
		err = config_error_line (&cfg);
		SU_DEBUG_0(("%s(): Config file syntax error in line %d\n",
				__func__, err));
		goto __exit_fail;
	} 

	set = config_lookup (&cfg, "hot_line" );
	if( !set ){
		g_conf.hot_line.records_num = 0;
		goto __exit_success;
	} 

	rec_num = config_setting_length (set);

	g_conf.hot_line.records_num = rec_num;
	g_conf.hot_line.records = malloc (rec_num * 
			sizeof(*(g_conf.hot_line.records)));
	if( !g_conf.hot_line.records ){
		SU_DEBUG_0((LOG_FNC_A(LOG_NOMEM)));
		goto __exit_fail;
	}
	memset(g_conf.hot_line.records, 0, rec_num * 
			sizeof(*(g_conf.hot_line.records)));

	for(i = 0; i < rec_num; i++){
		curr_rec = &g_conf.hot_line.records[ i ];
		rec_set = config_setting_get_elem (set, i);
		/* get id */
		elem = config_setting_get_string_elem (rec_set, 0);
		strncpy(curr_rec->id, elem, CHAN_ID_LEN-1);
		
		/* get value */
		elem = config_setting_get_string_elem (rec_set, 1);
		elem_len = strlen(elem);
		if (elem_len+1 < VALUE_LEN_DF ){
			curr_rec->value = curr_rec->value_s;
		} else {
			curr_rec->value = malloc((elem_len+1)* sizeof(*(curr_rec->value)));
			if( !curr_rec->value ){
				SU_DEBUG_0((LOG_FNC_A(LOG_NOMEM)));
				goto __exit_fail;
			}
		}
		strcpy (curr_rec->value, elem);
	}

__exit_success:
	config_destroy (&cfg);
	return 0;
__exit_fail:
	if(g_conf.hot_line.records) {
		for(i=0; i<rec_num; i++){
			curr_rec = &g_conf.hot_line.records[ i ];
			if( curr_rec->value && curr_rec->value != curr_rec->value_s ){
				free (curr_rec->value);
				curr_rec->value = NULL;
			}
		}
		free(g_conf.hot_line.records);
		g_conf.hot_line.records = NULL;
	}
	config_destroy (&cfg);
	return -1;
}/*}}}*/

/**
 * Init`s address book records in main routine configuration 
 * 		\ref g_conf structure.
 *
 * \retval 0 success.
 * \retval -1 fail.
 */ 
static int
addressb_init( void )
{/*{{{*/
	struct config_t cfg;
	struct config_setting_t * set;
	struct config_setting_t * rec_set;
	struct adbk_record_s * curr_rec;
	char const * elem;
	int elem_len;
	int rec_num;
	char use_id_local_buf = 1;
	int id_len;
	int i;
	int err;

	config_init (&cfg);

	/* Load the file */
	if (!config_read_file (&cfg, ADDRESSB_CONF_NAME)){
		err = config_error_line (&cfg);
		SU_DEBUG_0(("%s(): Config file syntax error in line %d\n",
				__func__, err));
		goto __exit_fail;
	} 

	set = config_lookup (&cfg, "address_book" );
	if( !set ){
		g_conf.address_book.records_num = 0;
		goto __exit_success;
	} 

	rec_num = config_setting_length (set);

	g_conf.address_book.records_num = rec_num;
	g_conf.address_book.records = malloc (rec_num * 
			sizeof(*(g_conf.address_book.records)));
	if( !g_conf.address_book.records ){
		SU_DEBUG_0((LOG_FNC_A(LOG_NOMEM)));
		goto __exit_fail;
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
			curr_rec->id = malloc( (id_len + 1) * sizeof(*(curr_rec->id)));
			if( !curr_rec->id ){
				SU_DEBUG_0((LOG_FNC_A(LOG_NOMEM)));
				goto __exit_fail;
			}
		}
		strcpy(curr_rec->id, elem);
		
		/* get value */
		elem = config_setting_get_string_elem (rec_set, 1);
		elem_len = strlen(elem);
		if (elem_len+1 < VALUE_LEN_DF ){
			curr_rec->value = curr_rec->value_s;
		} else {
			curr_rec->value = malloc((elem_len+1) * sizeof(*(curr_rec->value)));
			if( !curr_rec->value){
				SU_DEBUG_0((LOG_FNC_A(LOG_NOMEM)));
				goto __exit_fail;
			}
		}
		strcpy (curr_rec->value, elem);
	}

__exit_success:
	config_destroy (&cfg);
	return 0;
__exit_fail:
	if(g_conf.address_book.records) {
		for(i=0; i<rec_num; i++){
			curr_rec = &g_conf.address_book.records[ i ];
			if( curr_rec->id && curr_rec->id != curr_rec->id_s ){
				free (curr_rec->id);
				curr_rec->id = NULL;
			}
			if( curr_rec->value && curr_rec->value != curr_rec->value_s ){
				free (curr_rec->value);
				curr_rec->value = NULL;
			}
		}
		free(g_conf.address_book.records);
		g_conf.address_book.records = NULL;
	}
	config_destroy (&cfg);
	return -1;
}/*}}}*/

/**
 * Init`s codecs settings in main routine configuration \ref g_conf structure.
 *
 * \retval 0 success.
 * \retval -1 fail.
 */
static int
quality_init( void )
{/*{{{*/
	struct config_t cfg;
	struct config_setting_t * set = NULL;
	struct config_setting_t * rec_set = NULL;
	char const * str_elem = NULL;
	int i;
	int rec_num;
	int err;

	config_init (&cfg);

	/* Load the file */
	if (!config_read_file (&cfg, QUALITY_CONF_NAME)){
		err = config_error_line (&cfg);
		SU_DEBUG_0(("%s(): Config file syntax error in line %d\n",
				__func__, err));
		goto __exit_fail;
	} 

	/* set all to NONE */
	memset(g_conf.int_codecs, 0, sizeof(g_conf.int_codecs));
	memset(g_conf.sip_set.ext_codecs, 0, sizeof(g_conf.sip_set.ext_codecs));
	for (i=0; i<COD_MAS_SIZE; i++){
		g_conf.int_codecs[i].type = cod_type_NONE;
		g_conf.sip_set.ext_codecs[i].type = cod_type_NONE;
	} 

	/* CODECS FOR INTERNAL USAGE */
	set = config_lookup (&cfg, "int_codecs" );
	if( !set ){
		SU_DEBUG_0(("No int_codecs entries in config file"));
		goto __exit_fail;
	} 
	rec_num = config_setting_length (set);

	/* init one by one */
	for (i=0; i<rec_num; i++){
		rec_set = config_setting_get_elem (set, i);
		init_codec_el(rec_set, &g_conf.int_codecs[i]);
	} 

	/* CODECS FOR EXTERNAL USAGE */
	set = config_lookup (&cfg, "ext_codecs");
	if( !set){
		g_conf.sip_set.all_set = 0;
	} else {
		rec_num = config_setting_length (set);

		/* init one by one */
		for (i=0; i<rec_num; i++){
			rec_set = config_setting_get_elem (set, i);
			init_codec_el(rec_set, &g_conf.sip_set.ext_codecs[i]);
		} 
	}

	/* CODECS FOR FAX USAGE */
	str_elem = config_lookup_string (&cfg, "fax_codec");
	if( !str_elem){
		SU_DEBUG_0(("No fax_codec entry in config file"));
		goto __exit_fail;
	}

	/* set type and standart payload type values */
	if       ( !strcmp(str_elem, CONF_CODEC_ALAW)){
		g_conf.fax.codec_type = cod_type_ALAW;
		g_conf.fax.internal_pt = g_conf.fax.external_pt = ALAW_PT_DF;
	} else if( !strcmp(str_elem, CONF_CODEC_MLAW)){
		g_conf.fax.codec_type = cod_type_MLAW;
		g_conf.fax.internal_pt = g_conf.fax.external_pt = MLAW_PT_DF;
	} else {
		SU_DEBUG_2(("Wrong codec type for fax '%s'",str_elem));
		goto __exit_fail;
	}

	/* set internal fax payload type if it defined in config file */
	for (i=0; g_conf.int_codecs[i].type!=cod_type_NONE; i++){
		if(g_conf.int_codecs[i].type == g_conf.fax.codec_type){
			g_conf.fax.internal_pt = g_conf.int_codecs[i].user_payload;
			break;
		}
	} 
	/* set external fax payload type if it defined in config file */
	if(g_conf.sip_set.all_set){
		for (i=0; g_conf.sip_set.ext_codecs[i].type!=cod_type_NONE; i++){
			if(g_conf.sip_set.ext_codecs[i].type == g_conf.fax.codec_type){
				g_conf.fax.external_pt = 
						g_conf.sip_set.ext_codecs[i].user_payload;
				break;
			}
		} 
	}

	config_destroy (&cfg);
	return 0;
__exit_fail:
	config_destroy (&cfg);
	return err;
}/*}}}*/

/**
 * Init`s RTP parameters in main routine configuration \ref g_conf structure.
 *
 * \retval 0 success.
 * \retval -1 fail.
 */ 
static int
rtp_init( void )
{/*{{{*/
	struct config_t cfg;
	struct config_setting_t * set;
	struct config_setting_t * rec_set;
	struct rtp_prms_s * curr_rec;
	char const * elem;
	int rec_num;
	int i;
	int err;

	config_init (&cfg);

	/* Load the file */
	if (!config_read_file (&cfg, RTP_CONF_NAME)){
		err = config_error_line (&cfg);
		SU_DEBUG_0(("%s(): Config file syntax error in line %d\n",
				__func__, err));
		goto __exit_fail;
	} 

	/* Get values */
	set = config_lookup (&cfg, "rtp_prms" );
	if( !set ){
		/* We will use standart params for all channels */
		goto __exit_success;
	} 

	rec_num = config_setting_length (set);

	if(rec_num > CHANS_MAX){
		SU_DEBUG_0(("%s(): Too many channels (%d) in config - max is %d\n",
				__func__, rec_num, CHANS_MAX));
		goto __exit_fail;
	}

	for(i=0; i<rec_num; i++){
		int abs_idx;
		rec_set = config_setting_get_elem (set, i);

		/* get chan id */
		elem = config_setting_get_string_elem (rec_set, 0);
		abs_idx = strtol(elem, NULL, 10);

		curr_rec = &g_conf.rtp_prms[ abs_idx ];
		
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
	config_destroy (&cfg);
	return 0;
__exit_fail:
	config_destroy (&cfg);
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

