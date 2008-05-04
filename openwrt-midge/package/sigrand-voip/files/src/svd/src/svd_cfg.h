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
#define VAL_IN_CONF_FILE_MAX_SIZE 50

/* Route table only */
#define ROUTE_ID_LEN_DF 4 /* static or dynamic */
#define IP_LEN_MAX	16 /* xxx.xxx.xxx.xxx\0 */

#define PSTN_SIP_ID_LEN	15 /* sip id of full PSTN phone number */


enum codec_type_e
{
	codec_type_UNDEFINED,
	codec_type_SPEED,
	codec_type_MEDIUM,
	codec_type_QUALITY
};

struct adbk_record_s 
{
	char * id;
	char id_s [ADBK_ID_LEN_DF];
	char * value;
	unsigned char value_len;/**< Just numbers cnt id_mas_len = id_len + 1 */
	char value_s [VALUE_LEN_DF];
};
struct htln_record_s 
{
	char id [CHAN_ID_LEN];
	char * value;
	unsigned char value_len;/**< Just numbers cnt id_mas_len = id_len + 1 */
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
struct log_params_s
{
};
struct svd_conf_s
{
	char 	*self_number;
	char 	self_number_s [ROUTE_ID_LEN_DF];
	char 	self_ip [IP_LEN_MAX];
	char 	log_level; /**< if log_level = -1 - do not log anything */
	enum codec_type_e 	ext_codec;
	enum codec_type_e 	int_codec;
	struct address_book_s 	address_book;
	struct hot_line_s 	hot_line;
	struct route_table_s 	route_table;
} g_conf;

#endif /* __SVD_CFG_H__ */

