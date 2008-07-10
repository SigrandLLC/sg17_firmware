#ifndef __SVD_CFG_H__
#define __SVD_CFG_H__

#include "svd.h"

int  startup_init( int argc, char ** argv );
void startup_desrtoy( int argc, char ** argv );
int  svd_conf_init( void );
void conf_show( void );
void svd_conf_destroy( void );


/*
COMMAND LINE KEYS:
  -h, --help		display this help and exit
  -V, --version		show version and exit
*/

struct _startup_options
{
	unsigned char help;
	unsigned char version;
	char debug_level;
} g_so;

#define SVD_CONF_NAME 	"/etc/svd.conf"
#define SVD_ROUTE_NAME 	"/etc/svd_rt.conf"

#define WAIT_MARKER ','
#define ADBK_MARKER '#'
#define SELF_MARKER '*'
#define NET_MARKER '#'
#define FXO_MARKER '*'

/* Address book only */
#define ADBK_ID_LEN_DF	5 /* static or dynamic */

/* Hot line only */
#define CHAN_ID_LEN	3 /* static only */

/* Address book and Hot line common */
#define VALUE_LEN_DF	40 /* static or dynamic */

/* Route table only */
#define ROUTE_ID_LEN_DF 4 /* static or dynamic */
#define IP_LEN_MAX	16 /* xxx.xxx.xxx.xxx\0 */

#define ADDR_PAYLOAD_LEN 40 /* sip id or address, or full PSTN phone number */

#define REGISTRAR_LEN 50
#define USER_NAME_LEN 50
#define USER_PASS_LEN 30
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
struct sip_settings_s
{
	unsigned char all_set;
	enum codec_type_e ext_codec;
	char registrar [REGISTRAR_LEN];
	char user_name [USER_NAME_LEN];
	char user_pass [USER_PASS_LEN];
	char user_URI [USER_URI_LEN];
	unsigned char sip_chan;
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
	struct hot_line_s 	hot_line;
	struct route_table_s 	route_table;
} g_conf;

#endif /* __SVD_CFG_H__ */

