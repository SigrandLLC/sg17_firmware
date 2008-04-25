#ifndef __SVD_H__
#define __SVD_H__

typedef struct svd_s svd_t;
typedef struct svd_oper_s svd_oper_t;
typedef struct svd_chan_s svd_chan_t;

#define AB_CMAGIC_T     svd_chan_t
#include "libab/ab_api.h"

/* define type of context pointers for callbacks */
#define NUA_IMAGIC_T    ab_chan_t
#define NUA_HMAGIC_T    ab_chan_t
#define NUA_MAGIC_T     svd_t
#define SOA_MAGIC_T     svd_t
#define SU_ROOT_MAGIC_T svd_t


#include "config.h"
#include "sofia.h"
#include "svd_log.h"
#include "svd_cfg.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/unistd.h>
#include <errno.h>


/* * 
 *
 * this structure will attach to every svd->ab->chans[i] 
 * to store the channel info / status / etc in svd routine 
 *
 * */
struct svd_chan_s
{
	struct dial_state_s {
		enum dial_state_e {
			dial_state_START,
			dial_state_ADDR_BOOK,
			dial_state_ROUTE_ID,
			dial_state_CHAN_ID,
			dial_state_NET_ADDR,
		} state;
		int tag;
		enum self_e {
			self_UNDEFINED = 0,
			self_YES,
			self_NO
		} dest_is_self;

		/* remote router id */
		char * route_id;
		char route_id_s [ROUTE_ID_LEN_DF];
		unsigned char route_id_len;
		char route_ip [IP_LEN_MAX];

		/* remote chan identificator */
		char chan_id [CHAN_ID_LEN];

		/* address_book values */
		char * addrbk_id;
		char addrbk_id_s [ADBK_ID_LEN_DF];
		unsigned char addrbk_id_len;
		char * addrbk_value; /* points to g_conf value */
		unsigned char addrbk_value_len;

		/* pstn or sip number */
		char pstn_sip_id [PSTN_SIP_ID_LEN];
	} dial_status;


	int payload; /**< Selected payload */
	int rtp_sfd; /**< RTP socket file descriptor */
	int rtp_port; /**< Local RTP port */

	int remote_port; /**< Remote RTP port */
	char * remote_host; /**< Remote RTP host */

	int local_wait_idx; /**< Local wait index */
	int remote_wait_idx; /**< Remote wait index */

	/** NUA handle */
	nua_handle_t * op_handle;

	/* HOTLINE */
	unsigned char is_hotlined; /**< Is this channel hotline initiator */
	/* points to g_conf value */
	char * hotline_addr;       /**< Hotline destintation address */
	unsigned char hotline_addr_len;/**< Hotline dest. address length */
};

struct svd_s
{
	su_root_t *root;	/**< Pointer to application root */
	su_home_t home[1];	/**< Our memory home */
	nua_t * nua;		/**< Pointer to NUA object */
	ab_t * ab;		/**< Pointer to ATA Boards object */

	unsigned char net_is_hotlined; /**< network hotline marker */
	/* points to g_conf value */
	char * net_hotline_addr;       /**< Hotline destintation address */
	unsigned char net_hotline_addr_len;/**< Hotline dst. address length */
};

extern unsigned int g_f_cnt; 
extern unsigned int g_f_offset;
#endif /* __SVD_H__ */

