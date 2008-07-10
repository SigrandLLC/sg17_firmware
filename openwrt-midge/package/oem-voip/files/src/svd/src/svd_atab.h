#ifndef __SVD_ATAB_H__
#define __SVD_ATAB_H__

#include "svd.h"

int svd_atab_create (svd_t * const svd);
void svd_atab_delete (svd_t * svd);
int svd_media_register (svd_t * const svd, ab_chan_t * const chan);
void svd_media_unregister (svd_t * const svd, ab_chan_t * const chan);

void svd_clear_call(svd_t * const svd, ab_chan_t * const chan);

int get_dest_chan_idx( ab_t const * const ab, 
		char const * const chan_abs_idx_str, char const chan_abs_idx);

int get_FF_FXO_idx ( ab_t const * const ab, char const self_chan_idx );
int get_FF_FXS_idx ( ab_t const * const ab, char const self_chan_idx );

#endif /* __SVD_ATAB_H__ */

