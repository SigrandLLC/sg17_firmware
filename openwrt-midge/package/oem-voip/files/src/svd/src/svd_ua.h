#ifndef __SVD_UA_H__
#define __SVD_UA_H__

#include "sofia.h"

/** Use first free fxo channel on self router - marker */
#define FIRST_FREE_FXO "fxo"

/** @defgroup UAC User Agent Client part.
  	User Agent Client actions.
  @{ */
/** make INVITE SIP request */
int  svd_invite (svd_t * const svd, int const use_ff_FXO, int const chan_idx);

/** make INVITE SIP request with given destination address */
int  svd_invite_to (svd_t * const svd, int const chan_idx, 
		char const * const to_str);

/** make answer to SIP call */
int  svd_answer (svd_t * const svd, ab_chan_t * const chan,  
		int status, char const *phrase);

/** make BYE SIP action */
void svd_bye	(svd_t * const svd, ab_chan_t * const chan);

/** make CANCEL SIP action */
void svd_cancel	(ab_chan_t * const chan);

/** make REGISTER SIP action */
void svd_register 		(svd_t * const svd);

/** make un-REGISTER and REGISTER again on SIP server */
void svd_refresh_registration 	(svd_t * const svd);

/** shutdown SIP stack */
void svd_shutdown 		(svd_t * const svd);

/** @} */


/** @defgroup UAS User Agent Server part.
  	User Agent Server actions.
  @{ */

/** Callback for react on SIP events */
void svd_nua_callback(	nua_event_t  event,
			int          status,
			char const   *phrase,
			nua_t        *nua,
			svd_t        *svd,
			nua_handle_t *nh,
			ab_chan_t    *chan,
			sip_t const  *sip,
			tagi_t       tags[] );

/** @} */

#endif /* __SVD_UA_H__ */

