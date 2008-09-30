#ifndef __SVD_CFG_H__
#define __SVD_CFG_H__

#include "svd.h"

/** Getting parameters from startup keys */
int  startup_init( int argc, char ** argv );

/** Destroy startup parameters structures and print error messages */
void startup_desrtoy( int argc, char ** argv );

/** Reads config file and init \g_conf structure */
int  svd_conf_init( void );

/** Show the config information from \g_conf */
void conf_show( void );

/** Destroy \g_conf */
void svd_conf_destroy( void );


/*
COMMAND LINE KEYS:
  -h, --help		display this help and exit
  -V, --version		show version and exit
*/

/** Sturtup keys set */
struct _startup_options
{
	unsigned char help; /**< Show help and exit */
	unsigned char version; /**< Show version and exit */
	char debug_level; /**< Logging level in debug mode */
} g_so;

/** Config file path */
#define SVD_CONF_NAME 	"/etc/svd.conf"

/** Route config path */
#define SVD_ROUTE_NAME 	"/etc/svd_rt.conf"

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

/* Address book only */
/** Addressbook identifier standard length */
#define ADBK_ID_LEN_DF	5 /* static or dynamic */

/* Hot line only */
/** Channel identifier length */
#define CHAN_ID_LEN	3 /* static only */

/* Address book and Hot line common */
/** Full address value standard length */
#define VALUE_LEN_DF	40 /* static or dynamic */

/* Route table only */
/** Router identifier standard length */
#define ROUTE_ID_LEN_DF 4 /* static or dynamic */
/** IP number length */
#define IP_LEN_MAX	16 /* xxx.xxx.xxx.xxx\0 */
/** address payload length */
#define ADDR_PAYLOAD_LEN 40 /* sip id or address, or full PSTN phone number */
/** Registrar name in 'sip:server' form max length */
#define REGISTRAR_LEN 50
/** User name in 'user' form max length */
#define USER_NAME_LEN 50
/** password max length */
#define USER_PASS_LEN 30
/** User name in 'sip:user@server' form max length */
#define USER_URI_LEN 70

enum codec_type_e
{
	codec_type_SPEED,
	codec_type_QUALITY
};

struct adbk_record_s 
{
	char * id;
	char id_s [ADBK_ID_LEN_DF];
	char * value;
	char value_s [VALUE_LEN_DF];
};
struct htln_record_s 
{
	char id [CHAN_ID_LEN];
	char * value;
	char value_s [VALUE_LEN_DF];
};
struct rttb_record_s 
{
	char * id;
	char id_s [ROUTE_ID_LEN_DF];
	char value [IP_LEN_MAX];
};
struct rtp_record_s 
{
	char id [CHAN_ID_LEN];
	enum evts_2833_e 		OOB;
	enum play_evts_2833_e 	OOB_play;
	int evtPT;
	int evtPTplay;
	int COD_Tx_vol;
	int COD_Rx_vol;
	enum vad_cfg_e VAD_cfg;
	unsigned char HPF_is_ON;
};

struct address_book_s
{
	unsigned char id_len; /**< Just numbers cnt id_mas_len = id_len + 1 */
	unsigned int records_num;
	struct adbk_record_s * records;
};
struct hot_line_s 
{
	unsigned int records_num;
	struct htln_record_s * records;
};
struct route_table_s 
{
	unsigned char id_len; /**< Just numbers cnt id_mas_len = id_len + 1 */
	unsigned int records_num;
	struct rttb_record_s * records;
};
struct rtp_prms_s 
{
	unsigned int records_num;
	struct rtp_record_s * records;
};
struct sip_settings_s
{
	unsigned char all_set;
	enum codec_type_e ext_codec;
	char registrar [REGISTRAR_LEN];
	char user_name [USER_NAME_LEN];
	char user_pass [USER_PASS_LEN];
	char user_URI [USER_URI_LEN];
	unsigned char sip_chan;/**< FXS Channel abs_idx to catch sip call */
};
struct svd_conf_s
{
	char 	*self_number; /**< pointer to corresponding rt.rec[].id */
	char 	*self_ip; /**< pointer to corresponding rt.rec[].value */
	char 	log_level; /**< if log_level = -1 - do not log anything */
	enum codec_type_e 	int_codec;
	unsigned long 		rtp_port_first;
	unsigned long 		rtp_port_last;
	struct sip_settings_s	sip_set;
	struct address_book_s 	address_book;
	struct hot_line_s 		hot_line;
	struct route_table_s 	route_table;
	struct rtp_prms_s 		rtp_prms;
} g_conf;

#endif /* __SVD_CFG_H__ */

