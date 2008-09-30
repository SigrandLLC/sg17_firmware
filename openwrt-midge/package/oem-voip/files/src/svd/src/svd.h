#ifndef __SVD_H__
#define __SVD_H__

typedef struct svd_s svd_t;
typedef struct svd_chan_s svd_chan_t;

#include "ab_api.h"

/* define type of context pointers for callbacks */
#define NUA_IMAGIC_T    ab_chan_t
#define NUA_HMAGIC_T    ab_chan_t
#define NUA_MAGIC_T     svd_t
#define SOA_MAGIC_T     svd_t
#define SU_ROOT_MAGIC_T svd_t

/* tag__ debug logs */
#define SVD_DEBUG_LOGS 1

#include "config.h"
#include "sofia.h"
#include "svd_log.h"
#include "svd_cfg.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/unistd.h>
#include <errno.h>

#include "tapi/include/drv_tapi_io.h"

/** Channel context structure - store channel info / status / etc */
struct svd_chan_s
{
	struct dial_state_s {
		enum dial_state_e {
			dial_state_START,
			dial_state_ADDR_BOOK,
			dial_state_ROUTE_ID,
			dial_state_CHAN_ID,
			dial_state_NET_ADDR,
		} state; /**< State of dialing process */
		int tag; /**< additional info in dialing process */
		enum self_e {
			self_UNDEFINED = 0,
			self_YES,
			self_NO
		} dest_is_self; /**< is the desitnation router - self */

		char * route_id; /**< desitnation router identifier if it is not self */
		char * route_ip; /**< destination router ip - points to g_conf value */

		char chan_id [CHAN_ID_LEN]; /**< destination channel identificator */

		/* address_book values */
		char * addrbk_id;/**< address book identificator */
		char * addrbk_value; /**< address book value - points to g_conf value */

		/* sip number */
		char addr_payload [ADDR_PAYLOAD_LEN]; /**< sip number or other info */
	} dial_status; /**< dial status and values, gets in dial process */

	int payload; /**< Selected payload */
	int rtp_sfd; /**< RTP socket file descriptor */
	int rtp_port; /**< Local RTP port */

	char call_is_remote; /**< Caller in remote net */
	int remote_port; /**< Remote RTP port */
	char * remote_host; /**< Remote RTP host */

	int local_wait_idx; /**< Local wait index */
	int remote_wait_idx; /**< Remote wait index */

	/** NUA handle */
	nua_handle_t * op_handle;

	/* HOTLINE */
	unsigned char is_hotlined; /**< Is this channel hotline initiator */
	char * hotline_addr; /**< Hotline destintation address, 
			points to g_conf value*/
};

/** Routine main context structure */
struct svd_s
{
	su_root_t *root;	/**< Pointer to application root */
	su_home_t home[1];	/**< Our memory home */
	nua_t * nua;		/**< Pointer to NUA object */
	ab_t * ab;		/**< Pointer to ATA Boards object */
	nua_handle_t * op_reg; /**< Pointer NUA Handle reg object */
	char outbound_ip [IP_LEN_MAX]; /**< Outbound ip address */
	/* tag__ net hotline is not work - just init */
	unsigned char net_is_hotlined; /**< network hotline marker */
	char * net_hotline_addr;       /**< Hotline destintation address, 
			points to g_conf value */
};

#endif /* __SVD_H__ */

