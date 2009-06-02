/**
 * @file svd_cfg.h
 * Configuration interface.
 * It contains startup \ref g_so and main \ref g_conf configuration 
 * 		structures and functions to manipulate with them.
 */ 

#ifndef __SVD_CFG_H__
#define __SVD_CFG_H__

#include "svd.h"

/** @defgroup CFG_M Main configuration.
 *  It contains cfg files names, and functions for manipulations with 
 *  main configuration.
 *  @{*/
#define MAIN_CONF_NAME      "/etc/svd/main.conf"
#define FXO_CONF_NAME       "/etc/svd/fxo.conf"
#define ROUTET_CONF_NAME    "/etc/svd/routet.conf"
#define TONALF_CONF_NAME    "/etc/svd/tonalf.conf"
#define HOTLINE_CONF_NAME   "/etc/svd/hotline.conf"
#define ADDRESSB_CONF_NAME  "/etc/svd/addressb.conf"
#define QUALITY_CONF_NAME   "/etc/svd/quality.conf"
#define RTP_CONF_NAME       "/etc/svd/rtp.conf"
#define WLEC_CONF_NAME      "/etc/svd/wlec.conf"
#define TF_CONF_NAME        "/etc/svi.conf"
/** @}*/

/** @defgroup CFG_DF Default values.
 *  @ingroup CFG_M
 *  Some default values that will be set if they will not find in config file.
 *  @{*/
#define ALAW_PT_DF 0
/** @}*/ 

/** @defgroup DIAL_MARK Dial string markers.
 * 	@ingroup DIAL_SEQ
 * 	Dialling addressbook and other markers.
 *  @{*/
/** Wait char in address book */
#define WAIT_MARKER ','
/** Marker of address book start while dial a number */
#define ADBK_MARKER '#'
/** Marker of self router while dial a number */
#define SELF_MARKER '*'
#define SELF_MARKER2 '0'
/** Marker of net address start and end in address book or while dialing */
#define NET_MARKER '#'
/** Marker of first free fxo channel while dial a number to call on */
#define FXO_MARKER '*'
/** @}*/


/** @defgroup STARTUP Startup configuration.
 *  \ref g_so struct (startup options) and functions to manipulate with it.
 *  @{*/
/*
COMMAND LINE KEYS:
  -h, --help		display this help and exit
  -V, --version		show version and exit
*/

/** Sturtup keys set. */
struct _startup_options {
	unsigned char help; /**< Show help and exit. */
	unsigned char version; /**< Show version and exit. */
	char debug_level; /**< Logging level in debug mode. */
} g_so;

/** Getting parameters from startup keys. */
int  startup_init( int argc, char ** argv );
/** Destroy startup parameters structures and print error messages. */
void startup_destroy( int argc, char ** argv );
/** @} */


/** @addtogroup CFG_M
 *  @{*/
/* g_conf inner definitions {{{*/
/** Codecs massives sizes. 
 * First unusing codec \c type will be set as \c codec_type_NONE.
 * It should be greater then codecs count because application can 
 * test the end of the list by \c == \c codec_type_NONE */
#define COD_MAS_SIZE 12
/* Address book only */
/** Addressbook identifier standard length.*/
#define ADBK_ID_LEN_DF	5 /* static or dynamic */
/* Hot line only */
/** Channel identifier length.*/
#define CHAN_ID_LEN	3 /* static only */
/* Address book and Hot line common */
/** Full address value standard length.*/
#define VALUE_LEN_DF	40 /* static or dynamic */
/* Route table only */
/** Router identifier standard length.*/
#define ROUTE_ID_LEN_DF 4 /* static or dynamic */
/** IP number length.*/
#define IP_LEN_MAX	16 /* xxx.xxx.xxx.xxx\0 */
/** Address payload length.*/
#define ADDR_PAYLOAD_LEN 40 /* sip id or address, or full PSTN phone number */
/** Registrar name in 'sip:server' form max length */
#define REGISTRAR_LEN 50
/** User name in 'user' form max length.*/
#define USER_NAME_LEN 50
/** Password max length.*/
#define USER_PASS_LEN 30
/** User name in 'sip:user\@server' form max length.*/
#define USER_URI_LEN 70
/*}}}*/

/** Reads config files and init \ref g_conf structure.*/
int  svd_conf_init( void );
/** Read ata board params and reinit some in \ref g_conf.*/
int  svd_conf_reinit( ab_t const * const ab );
/** Show the config information from \ref g_conf.*/
void conf_show( void );
/** Destroy \ref g_conf.*/
void svd_conf_destroy( void );


#define FMTP_STR_LEN 20

/** Codec rtp and sdp parameters.*/ 
typedef struct cod_prms_s {
	cod_type_t type;
	char sdp_name[COD_NAME_LEN];
	char fmtp_str[FMTP_STR_LEN];
	int rate;
} cod_prms_t;

/** Initilize codecs parameters structure */
int svd_init_cod_params( cod_prms_t * const cp );

/* g_conf inner structures {{{*/
/** Tonal Frequency Channel type.*/
enum tf_type_e {
	tf_type_DEFAULT = 0,
	tf_type_N4 = 0,
	tf_type_N2,
	tf_type_T4,
	tf_type_T2,
};
/** Address book record.*/
struct adbk_record_s {
	char * id; /**< Short number pointer.*/
	char id_s [ADBK_ID_LEN_DF]; /**< Short number static massive.*/
	char * value; /**< Long number pointer.*/
	char value_s [VALUE_LEN_DF]; /**< Long number static massive.*/
};
/** Hot line record.*/
struct htln_record_s {
	char id [CHAN_ID_LEN]; /**< Channel absolute identifier.*/
	char * value; /**< Hotline address pointer.*/
	char value_s [VALUE_LEN_DF]; /**< Hotline static massive.*/
};
/** Route table record.*/
struct rttb_record_s {
	char * id; /**< Router identifier pointer.*/
	char id_s [ROUTE_ID_LEN_DF]; /**< Router identifier static massive.*/
	char value [IP_LEN_MAX]; /**< Router ip address.*/
};
/** Address book.*/
struct address_book_s {
	unsigned char id_len; /**< Just numbers cnt id_mas_len = id_len + 1.*/
	unsigned int records_num; /**< Number of records in address book.*/
	struct adbk_record_s * records; /**< Records massive.*/
};
/** Hot line.*/
struct hot_line_s {
	unsigned int records_num; /**< Number of hotline records.*/
	struct htln_record_s * records; /**< Records massive.*/
};
/** Tonal Frequency channels.*/
struct tonal_freq_s {
	int is_set; /**< Set to 1 if this channel has meaningful values. */
	enum tf_type_e type; /**< Channel type. */
	int am_i_caller; /**< Set to 1 if this channel should call to it`s pair. */
	char id [CHAN_ID_LEN]; /**< Channel absolute identifier.*/
	char * pair_route; /**< Channel pair router identifier or NULL if self. */
	char pair_route_s [ROUTE_ID_LEN_DF]; /**< Channel pair router massive.*/
	char pair_chan [CHAN_ID_LEN]; /**< Channel pair channel identifier.*/
	codec_t tf_codec; /**< codec parameters.*/
};
/** Route table.*/
struct route_table_s {
	unsigned char id_len; /**< Just numbers cnt id_mas_len = id_len + 1.*/
	unsigned int records_num; /**< Number of routers records.*/
	struct rttb_record_s * records; /**< Records massive.*/
};
/** RTP parameters.*/
struct rtp_prms_s {
	int COD_Tx_vol; /**< Coder tx volume gain (from -24 to 24).*/
	int COD_Rx_vol; /**< Coder rx volume gain (from -24 to 24).*/
	enum vad_cfg_e VAD_cfg; /**< Voice activity detector mode.*/
	unsigned char HPF_is_ON; /**< High-pass filter (on or off).*/
};

/** SIP registration and codec choise policy.*/
struct sip_settings_s {
	unsigned char all_set; /**< Shall we register on sip server?*/
	codec_t ext_codecs[COD_MAS_SIZE];/**< Codecs sorted by priority in internet calls.*/
	char registrar [REGISTRAR_LEN]; /**< SIP registrar address.*/
	char user_name [USER_NAME_LEN]; /**< SIP user name.*/
	char user_pass [USER_PASS_LEN]; /**< SIP user password.*/
	char user_URI [USER_URI_LEN]; /**< SIP URI.*/
	unsigned char sip_chan;/**< FXS Channel absolute id to catch sip call.*/
};
/** Fax parameters.*/
struct fax_s {
	enum cod_type_e codec_type;
	int internal_pt;
	int external_pt;
};
/** PSTN type.*/
enum pstn_type_e {
	pstn_type_TONE_AND_PULSE,
	pstn_type_PULSE_ONLY,
};
/*}}}*/

/** Routine main configuration struct.*/
struct svd_conf_s {/*{{{*/
	char * self_number; /**< Pointer to corresponding rt.rec[].id or NULL.*/
	char * self_ip; /**< Pointer to corresponding rt.rec[].value or lo_ip.*/
	char lo_ip[IP_LEN_MAX]; /**< Local Address IP. */
	char log_level; /**< If log_level = -1 - do not log anything.*/
	codec_t int_codecs[COD_MAS_SIZE];/**< Codecs sorted by priority in local calls.*/
	struct fax_s fax;/**< Fax parameters.*/
	unsigned long rtp_port_first; /**< Min ports range bound for RTP.*/
	unsigned long rtp_port_last; /**< Max ports range bound for RTP.*/
	struct sip_settings_s sip_set; /**< SIP settings for registration.*/
	struct address_book_s address_book; /**< Address book.*/
	struct hot_line_s hot_line; /**< Hot line.*/
	struct tonal_freq_s tonal_freq[CHANS_MAX]; /**< TF-channels params.*/
	struct route_table_s route_table; /**< Routes table.*/
	struct rtp_prms_s rtp_prms[CHANS_MAX]; /**< RTP channel parameters.*/
	enum pstn_type_e fxo_PSTN_type[CHANS_MAX]; /**< FXO pstn types.*/
	unsigned char sip_tos; /** Type of Service byte for sip-packets.*/
	unsigned char rtp_tos; /** Type of Service byte for rtp-packets.*/
	struct wlec_s wlec_prms[CHANS_MAX]; /**< WLEC channel parameters.*/
} g_conf;/*}}}*/

/** @} */

#endif /* __SVD_CFG_H__ */

