#ifndef __SVD_ATAB_H__
#define __SVD_ATAB_H__

#include "svd.h"

/** Create the ab struct and attach the callbacks to root */
int svd_atab_create (svd_t * const svd);

/** Destroy th ab struct */
void svd_atab_delete (svd_t * svd);

/** Attach the appropriate callbacks to rtp files and to root obj */
int svd_media_register (svd_t * const svd, ab_chan_t * const chan);

/** Close rtp-socket and destroy the callback timers in root */
void svd_media_unregister (svd_t * const svd, ab_chan_t * const chan);

/** Clears call params that has been set during dial process */
void svd_clear_call(svd_t * const svd, ab_chan_t * const chan);

/** Found index in ab->chans[] mass with the given abs_idx */
int get_dest_chan_idx( ab_t const * const ab, 
		char const * const chan_abs_idx_str, char const chan_abs_idx);

/** Found first free fxo channel */
int get_FF_FXO_idx ( ab_t const * const ab, char const self_chan_idx );

/** Found first free fxs channel */
int get_FF_FXS_idx ( ab_t const * const ab, char const self_chan_idx );

/** Start encoding / decoding on given channel */
int ab_chan_media_activate ( ab_chan_t * const chan );

/** Stop encoding / decoding on given channel */
int ab_chan_media_deactivate ( ab_chan_t * const chan );

#endif /* __SVD_ATAB_H__ */

