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
/** Config file path */
#define SVD_CONF_NAME 	"/etc/svd.conf"
/** Route config path */
#define SVD_ROUTE_NAME 	"/etc/svd_rt.conf"
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
struct _startup_options {/*{{{*/
	unsigned char help; /**< Show help and exit. */
	unsigned char version; /**< Show version and exit. */
	char debug_level; /**< Logging level in debug mode. */
} g_so;/*}}}*/

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
#define COD_MAS_SIZE 5
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

/** Reads config file and init \ref g_conf structure.*/
int  svd_conf_init( void );
/** Show the config information from \ref g_conf.*/
void conf_show( void );
/** Destroy \ref g_conf.*/
void svd_conf_destroy( void );

/** Codec rtp and sdp parameters.*/ 
typedef struct cod_prms_s {
	cod_type_t type;
	char sdp_name[COD_NAME_LEN];
	int rate;
} cod_prms_t;

/** Initilize codecs parameters structure */
int svd_init_cod_params( cod_prms_t * const cp );

/* g_conf inner structures {{{*/
/** Codec choose policy.*/
enum codec_type_e {
	codec_type_SPEED, /**< Choose fastest */
	codec_type_QUALITY /**< Choose most high-quality */
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
/** RTP record.*/
struct rtp_record_s {
	char id [CHAN_ID_LEN]; /**< Channel absolute identifier.*/
	enum evts_2833_e OOB; /**< Out-of-band events generation mode.*/
	enum play_evts_2833_e OOB_play; /**< Out-of-band events play mode.*/
	int evtPT; /**< RFC_2833 events payload type for generator side.*/
	int evtPTplay; /**< RFC_2833 events payload type for receiver side.*/
	int COD_Tx_vol; /**< Coder tx volume gain (from -24 to 24).*/
	int COD_Rx_vol; /**< Coder rx volume gain (from -24 to 24).*/
	enum vad_cfg_e VAD_cfg; /**< Voice activity detector mode.*/
	unsigned char HPF_is_ON; /**< High-pass filter (on or off).*/
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
/** Route table.*/
struct route_table_s {
	unsigned char id_len; /**< Just numbers cnt id_mas_len = id_len + 1.*/
	unsigned int records_num; /**< Number of routers records.*/
	struct rttb_record_s * records; /**< Records massive.*/
};
/** RTP parameters.*/
struct rtp_prms_s {
	unsigned int records_num; /**< Number of records (max 1 rec per channel).*/
	struct rtp_record_s * records; /**< Records massive.*/
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
/*}}}*/

/** Routine main configuration struct.*/
struct svd_conf_s {/*{{{*/
	char * self_number; /**< Pointer to corresponding rt.rec[].id.*/
	char * self_ip; /**< Pointer to corresponding rt.rec[].value.*/
	char log_level; /**< If log_level = -1 - do not log anything.*/
	codec_t int_codecs[COD_MAS_SIZE];/**< Codecs sorted by priority in local calls.*/
	struct fax_s fax;/**< Fax parameters.*/
	unsigned long rtp_port_first; /**< Min ports range bound for RTP.*/
	unsigned long rtp_port_last; /**< Max ports range bound for RTP.*/
	struct sip_settings_s sip_set; /**< SIP settings for registration.*/
	struct address_book_s address_book; /**< Address book.*/
	struct hot_line_s hot_line; /**< Hot line.*/
	struct route_table_s route_table; /**< Routes table.*/
	struct rtp_prms_s rtp_prms; /**< RTP parameters.*/
} g_conf;/*}}}*/
/** @} */

#endif /* __SVD_CFG_H__ */

