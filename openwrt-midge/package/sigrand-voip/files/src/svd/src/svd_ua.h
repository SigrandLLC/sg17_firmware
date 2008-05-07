#ifndef __SVD_UA_H__
#define __SVD_UA_H__

#include "sofia.h"

#define FIRST_FREE_FXO "fxo"

#define INFO_DIGIT_MSG_FORMAT "DIGIT:'%c'"
#define INFO_DIGIT_MSG_SIZE 10 
/* it is ugly, but it fast and it works */
#define INFO_DIGIT_MSG_DIGIT_POS 7 

/****************************************************************** UAC */

int  svd_invite (svd_t * const svd, int const use_ff_FXO, int const chan_idx);
int  svd_answer (svd_t * const svd, ab_chan_t * const chan,  
		int status, char const *phrase);
void svd_bye	(svd_t * const svd, ab_chan_t * const chan);
void svd_cancel	(ab_chan_t * const chan);

/*
void svd_register 	(svd_t * const svd, const char *registrar);
void svd_unregister 	(svd_t * const svd, const char *registrar);
*/
void svd_shutdown 	(svd_t * const svd);


/****************************************************************** UAS */

void svd_nua_callback(	nua_event_t  event,
			int          status,
			char const   *phrase,
			nua_t        *nua,
			svd_t        *svd,
			nua_handle_t *nh,
			ab_chan_t    *chan,
			sip_t const  *sip,
			tagi_t       tags[] );

#endif /* __SVD_UA_H__ */

