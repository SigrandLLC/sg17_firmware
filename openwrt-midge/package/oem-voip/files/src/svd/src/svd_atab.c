#include "svd.h"
#include "svd_cfg.h"
#include "ab_api.h"
#include "svd_ua.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <assert.h>

#include "tapi/include/drv_tapi_io.h"

#define BUFF_PER_RTP_PACK_SIZE 512

static int svd_atab_handler (su_root_magic_t * root, su_wait_t * w, 
		su_wakeup_arg_t * user_data);
static int svd_handle_event_FXS_OFFHOOK
		( svd_t * const svd, int const chan_idx );
static int svd_handle_event_FXS_ONHOOK
		( svd_t * const svd, int const chan_idx );
static int svd_handle_event_FXS_DIGIT_X
		( svd_t * const svd, int const chan_idx, long const data );
static int svd_handle_event_FXO_RINGING 
		( svd_t * const svd, int const chan_idx );

static int svd_chans_init ( svd_t * const svd );
static int attach_dev_cb ( svd_t * const svd );

static int svd_handle_digit 
		( svd_t * const svd, int const chan_idx, long const digit );
static int svd_handle_START
		( ab_t * const ab, int const chan_idx, long const digit );
static int svd_handle_ROUTE_ID
		( ab_t * const ab, int const chan_idx, long const digit );
static int svd_handle_CHAN_ID
		( svd_t * const svd, int const chan_idx, long const digit );
static int svd_handle_NET_ADDR
		( svd_t * const svd, int const chan_idx, long const digit );
static int svd_handle_ADDR_BOOK
		( svd_t * const svd, int const chan_idx, long const digit );

static int svd_ab_value_set_by_id (ab_chan_t * const ab_chan);
static int svd_process_addr (svd_t * const svd, int const chan_idx, 
		char const * const value);
static int set_route_ip (svd_chan_t * const chan_c);
static int local_connection_selector
		( svd_t * const svd, int const use_ff_FXO, int const chan_idx);

static int svd_self_call
	( svd_t * const svd, int const src_chan_idx, int const use_ff_FXO );

static int svd_media_vinetic_handle_local_data (
		su_root_magic_t * root, 
		su_wait_t * w,
		su_wakeup_arg_t * user_data );
static int svd_media_vinetic_handle_remote_data (
		su_root_magic_t * root, 
		su_wait_t * w,
		su_wakeup_arg_t * user_data );
static int svd_media_vinetic_open_rtp (svd_chan_t * const chan_d);

/******************************************************************************/
/* it uses !!g_conf to get route_id_len */
/* 
 * - Creates ab structure
 * - Creates svd_channel_status structures
 * - Register waits on vinetic dev files 
 * 
 * dev1, dev2, dev3, dev4
 * dev_fd_N <-> wait_N
 * wait_N <-> dev_evts_func
 * */
int
svd_atab_create ( svd_t * const svd )
{
	int err;
DFS
	assert (svd);
	assert (!svd->ab);

	svd->ab = ab_create();
	if( !svd->ab ){
		SU_DEBUG_0 ((LOG_FNC_A(ab_g_err_str)));
		goto __exit_fail;
	}

	/** it uses !!g_conf to get route_id_len */
	err = svd_chans_init (svd);
	if (err){
		SU_DEBUG_0 ((LOG_FNC_A("channels init error")));
		goto __exit_fail;
	}

	err = attach_dev_cb (svd);
	if (err){
		SU_DEBUG_0 ((LOG_FNC_A("dev callback attach error")));
		goto __exit_fail;
	}
DFE
	return 0;
__exit_fail:
DFE
	return -1;	
}

void 
svd_atab_delete ( svd_t * const svd )
{
	int i;
	int j;
DFS
	assert (svd);

	if ( !(svd->ab) ){
		goto __exit;
	}

	/* ab_chans_magic_destroy */
	j = svd->ab->chans_num;
	for( i = 0; i < j; i++ ){
		svd_chan_t * curr_chan = svd->ab->chans[ i ].ctx;
		if (curr_chan){
			if (curr_chan->dial_status.route_id){
				free (curr_chan->dial_status.route_id);
			}
			if (curr_chan->dial_status.addrbk_id){
				free (curr_chan->dial_status.addrbk_id);
			}
			free (curr_chan);
			curr_chan = NULL;
		}
	}
	
	/* ab_destroy */
	ab_destroy (&svd->ab);

__exit:
DFE
	return;
}

int 
svd_media_register (svd_t * const svd, ab_chan_t * const chan)
{
	su_wait_t wait[1];
	int ret;
DFS
	assert (chan);
	assert (chan->ctx);

	svd_chan_t * chan_ctx = chan->ctx;

	assert (chan_ctx->local_wait_idx == -1);
	assert (chan_ctx->remote_wait_idx == -1);

	ret = su_wait_create(wait, chan->rtp_fd, SU_WAIT_IN);
	if (ret){
		SU_DEBUG_0 ((LOG_FNC_A ("su_wait_create() fails" ) ));
		goto __exit_fail;
	}

	ret = su_root_register (svd->root, wait, 
			svd_media_vinetic_handle_local_data, chan, 0);
	if (ret == -1){
		SU_DEBUG_0 ((LOG_FNC_A ("su_root_register() fails" ) ));
		goto __exit_fail;
	}
	chan_ctx->local_wait_idx = ret;

	ret = svd_media_vinetic_open_rtp (chan_ctx);
	if (ret == -1){
		goto __exit_fail;
	}
	chan_ctx->rtp_sfd = ret;

	ret = su_wait_create(wait, chan_ctx->rtp_sfd, SU_WAIT_IN);
	if (ret){
		SU_DEBUG_0 ((LOG_FNC_A ("su_wait_create() fails" ) ));
		goto __exit_fail;
	}

	ret = su_root_register (svd->root, wait, 
			svd_media_vinetic_handle_remote_data, chan, 0);
	if (ret == -1) {
		SU_DEBUG_0 ((LOG_FNC_A ("su_root_register() fails" ) ));
		goto __exit_fail;
	}
	chan_ctx->remote_wait_idx = ret;

DFE
	return 0;
__exit_fail:
DFE
	return -1;
}

void
svd_media_unregister (svd_t * const svd, ab_chan_t * const chan)
{
	svd_chan_t * chan_c;
DFS
	assert (chan);
	assert (chan->ctx);

	chan_c = chan->ctx;

	if(chan_c->rtp_sfd != -1){
		if( close (chan_c->rtp_sfd) ){
			su_perror("svd_media_unregister() close()");
		}
		chan_c->rtp_sfd = -1;
	}
	if(chan_c->local_wait_idx != -1){
		su_root_deregister (svd->root, chan_c->local_wait_idx);
		chan_c->local_wait_idx = -1;
	}
	if(chan_c->remote_wait_idx != -1){
		su_root_deregister (svd->root, chan_c->remote_wait_idx);
		chan_c->remote_wait_idx = -1;
	}
DFE
}

/**it uses !!g_conf
 * clears call status on the channel
 * */
void 
svd_clear_call (svd_t * const svd, ab_chan_t * const chan)
{
	svd_chan_t * chan_c = chan->ctx;
	int len;
	int size;
DFS
	/* STATES */
	chan_c->dial_status.state = dial_state_START;
	
	/* TAG */
	chan_c->dial_status.tag = 0;

	/* DEST_IS_SELF */
	chan_c->dial_status.dest_is_self = self_UNDEFINED;

	/* ROUTER */
	len = g_conf.route_table.id_len;
	if( len ){
		size = sizeof(*(chan_c->dial_status.route_id));
		memset(chan_c->dial_status.route_id, 0, size * (len+1));
	}
	/* route_ip  */
	chan_c->dial_status.route_ip = NULL;

	/* CHAN */
	size = sizeof(chan_c->dial_status.chan_id);
	memset (chan_c->dial_status.chan_id, 0, size);

	/* ADDRBOOK */
	len = g_conf.address_book.id_len;
	if( len ){
		size = sizeof(*(chan_c->dial_status.addrbk_id));
		memset(chan_c->dial_status.addrbk_id, 0, size * (len+1));
	}

	/* ADDR_PAYLOAD */
	size = sizeof(chan_c->dial_status.addr_payload);
	memset (chan_c->dial_status.addr_payload, 0, size);

	/* Caller remote or not */
	chan_c->call_is_remote = 0;

	/* SDP */
	chan_c->payload = 0;
	chan_c->rtp_port = 0;

	chan_c->remote_port = 0;
	if(chan_c->remote_host){
		su_free (svd->home, chan_c->remote_host);
		chan_c->remote_host = NULL;
	}

	/* HANDLE */
	if(chan_c->op_handle){
		nua_handle_destroy (chan_c->op_handle);
		chan_c->op_handle = NULL;
	}
DFE
}

/* * 
 * return -1 on error case and 
 * other value on succsess 
 * if chan_abs_idx_str == NULL, using chan_abs_idx,
 * otherwise, using chan_abs_idx_str
 * */
int 
get_dest_chan_idx( ab_t const * const ab, 
		char const * const chan_abs_idx_str, char const chan_abs_idx )
{
	int ret_idx;
	int idx_to_find;
	int chans_num; 
	int i;
DFS
	if( !chan_abs_idx_str ){
		idx_to_find = chan_abs_idx;
	} else {
		idx_to_find = strtol (chan_abs_idx_str, NULL, 10);
	} 

	ret_idx = -1;
	chans_num = ab->chans_num;
	for( i = 0; i < chans_num; i++ ){
		if (idx_to_find == ab->chans[ i ].abs_idx){
			ret_idx = i;
			break;
		}
	}
	if (ret_idx == -1){
		SU_DEBUG_2 (("Wrong chan id [_%d_]\n", idx_to_find));
		goto __exit_fail;
	}

DFE
	return ret_idx;
__exit_fail:
DFE
	return -1;
}

int 
get_FF_FXO_idx ( ab_t const * const ab, char const self_chan_idx )
{
	int i;
	int j;
DFS
	j = ab->chans_num;
	for (i = 0; i < j; i++){
		ab_chan_t * curr_chan = &ab->chans[i];
		svd_chan_t * chan_ctx = curr_chan->ctx;
		if (curr_chan->parent->type == ab_dev_type_FXO &&
				chan_ctx->op_handle == NULL &&
				i != self_chan_idx ){
			break;
		}
	}
	if (i == j){
		SU_DEBUG_2(("No free FXO channel available\n"));
		i = -1;
	}
DFE
	return i;
}

int 
get_FF_FXS_idx ( ab_t const * const ab, char const self_chan_idx )
{
	int i;
	int j;
DFS
	j = ab->chans_num;
	for (i = 0; i < j; i++){
		ab_chan_t * curr_chan = &ab->chans[i];
		svd_chan_t * chan_ctx = curr_chan->ctx;
		if (curr_chan->parent->type == ab_dev_type_FXS &&
				chan_ctx->op_handle == NULL &&
				i != self_chan_idx ){
			break;
		}
	}
	if (i == j){
		SU_DEBUG_2(("No free FXS channel available\n"));
		i = -1;
	}
DFE
	return i;
}

int 
ab_chan_media_activate ( ab_chan_t * const chan )
{
	int err;

	/* *
	 * tag__ cod_pt is enum and payload is int 
	 * there we should make choise more verbosity then we add dinamic
	 * codecs
	 * */
	chan->rtp_cfg.cod_pt = ((svd_chan_t *)(chan->ctx))->payload;

	err = ab_chan_media_rtp_tune (chan);
	if(err){
		goto __exit;
	}
	err = ab_chan_media_switch (chan, 1, 1);
__exit:
	return err;
}

int 
ab_chan_media_deactivate ( ab_chan_t * const chan )
{
	return ab_chan_media_switch (chan, 0, 0);
}

/******************************************************************************/
/* *
 * FXS : digits(mass per chan), hooks(actions per chan)
 * FXO : ring(soft actions per chan)
 *
 * su_root_magic_t - svd
 * su_wakeup_arg_t - dev_ptr
 *
 * TODO : should be reenterable or mutexes must be used tag__
 * */
static int
svd_atab_handler (svd_t * svd, su_wait_t * w, su_wakeup_arg_t * user_data)
{
DFS
	ab_dev_t * ab_dev = (ab_dev_t *) user_data;
	ab_dev_event_t evt;
	unsigned char chan_av;
	int chan_idx;
	int dev_idx;
	int err;

	memset(&evt, 0, sizeof(evt));

	err = ab_dev_event_get( ab_dev, &evt, &chan_av );
	if( err ){
		SU_DEBUG_1 ((LOG_FNC_A (ab_g_err_str) ));
		goto __exit_fail;
	}

	dev_idx = ab_dev->idx - 1;
	if (chan_av){
		/* in evt.ch we have proper number of the chan */
		chan_idx = dev_idx * svd->ab->chans_per_dev + 
				(svd->ab->chans_per_dev - evt.ch - 1);
	} else {
		/* in evt.ch we do not have proper number of the chan 
		 * because the event is the device event - not the chan event
		 */
		chan_idx = dev_idx * svd->ab->chans_per_dev;
	}

	/* in some errors we do not want to quit
	 * assert( evt.more == 0 ); */

	switch (evt.id){
		case ab_dev_event_FXS_OFFHOOK:{
			SU_DEBUG_0 (("Got fxs offhook event\n"));
			err = svd_handle_event_FXS_OFFHOOK(svd, chan_idx);
			break;
		}
		case ab_dev_event_FXS_ONHOOK:{
			SU_DEBUG_0 (("Got fxs onhook event\n"));
			err = svd_handle_event_FXS_ONHOOK(svd, chan_idx);
			break;
		}
		case ab_dev_event_FXS_DIGIT_TONE:
		case ab_dev_event_FXS_DIGIT_PULSE:{
			err = svd_handle_event_FXS_DIGIT_X(svd, chan_idx, 
					evt.data);
			break;
		}
		case ab_dev_event_FXO_RINGING:{
			SU_DEBUG_0 (("Got fxo ringing event\n"));
			err = svd_handle_event_FXO_RINGING (svd, chan_idx);
			break;
		}
		case ab_dev_event_UNCATCHED:{

#ifdef SVD_DEBUG_LOGS
			SU_DEBUG_2 (("Got unknown event : 0x%X "
					"on device / channel %d / %d\n",
					evt.data, dev_idx+1,evt.ch+1 ));
#endif
			break;
		}
	}
	if (err){
		goto __exit_fail;
	}

DFE
	return 0;
__exit_fail:
DFE
	return -1;
}

static int 
svd_handle_event_FXS_OFFHOOK( svd_t * const svd, int const chan_idx )
{
	ab_chan_t * ab_chan = &svd->ab->chans[chan_idx];
	int call_answered;
	int err;

DFS
	
#ifdef SVD_DEBUG_LOGS
	SU_DEBUG_3 (("OFFHOOK on channel [_%d_]\n ", ab_chan->abs_idx));
#endif
	/* stop ringing */
	err = ab_FXS_line_ring( ab_chan, ab_chan_ring_MUTE );
	if (err){
		SU_DEBUG_0(("can`t stop ring on [_%d_]\n", ab_chan->abs_idx));
		svd_answer(svd, ab_chan, SIP_500_INTERNAL_SERVER_ERROR);
		goto __exit_fail;
	}

	/* change linefeed mode to ACTIVE */
	err = ab_FXS_line_feed (ab_chan, ab_chan_linefeed_ACTIVE);
	if (err){
		SU_DEBUG_0(("can`t set linefeed to active on [_%d_]\n",
				ab_chan->abs_idx));
		svd_answer(svd, ab_chan, SIP_500_INTERNAL_SERVER_ERROR);
		goto __exit_fail;
	}

	/* answer on call if it exists */
	call_answered = svd_answer(svd, ab_chan, SIP_200_OK);
	if (call_answered) {
		goto __exit_success;
	}

	/* no call to answer */
	/* hotline test */
	if(((svd_chan_t *)(ab_chan->ctx))->is_hotlined){
		/* process the hotline sequence */
		err = svd_process_addr (svd, chan_idx, 
				((svd_chan_t *)(ab_chan->ctx))->hotline_addr);
		if(err){
			goto __exit_fail;
		}
	}

__exit_success:
DFE
	return 0;
__exit_fail:
DFE
	return -1;
}

static int 
svd_handle_event_FXS_ONHOOK( svd_t * const svd, int const chan_idx )
{
	ab_chan_t * ab_chan = &svd->ab->chans[chan_idx];
	int err;

DFS

#ifdef SVD_DEBUG_LOGS
	SU_DEBUG_3 (("ONHOOK on channel [_%d_]\n ", ab_chan->abs_idx));
#endif
	/* say BYE on existing connection */
	svd_bye(svd, ab_chan);

	/* change linefeed mode to STANDBY */
	err = ab_FXS_line_feed (ab_chan, ab_chan_linefeed_STANDBY);
	if (err){
		SU_DEBUG_0(("can`t set linefeed to standby on [_%d_]\n",
				ab_chan->abs_idx));
		goto __exit_fail;
	}

DFE
	return 0;
__exit_fail:
DFE
	return -1;
}

static int 
svd_handle_event_FXS_DIGIT_X ( svd_t * const svd, int const chan_idx, 
		long const data )
{
	ab_chan_t * ab_chan = &svd->ab->chans[chan_idx];
	svd_chan_t * chan_ctx = ab_chan->ctx;
	char digit = data;
	int err;

DFS

#ifdef SVD_DEBUG_LOGS
	SU_DEBUG_3 (("DIGIT \'%c\', on channel [_%d_]\n ",
			digit, ab_chan->abs_idx ));
	SU_DEBUG_3 (("local : %d, network : %d\n",
			(data >> 9),
			(data >> 8) & 1 ));
	SU_DEBUG_3 (("HN : %p\n",chan_ctx->op_handle));
#endif

	if( chan_ctx->op_handle ){
		/* allready connected - send info 
		 * see rfc_2976
		 */
	} else {
		/* not connected yet - should process digits */
		err = svd_handle_digit (svd, chan_idx, digit);
		if(err){
			/* clear call params */
			svd_clear_call (svd, ab_chan);
			goto __exit_fail;
		}
	}

DFE
	return 0;
__exit_fail:
DFE
	return -1;
}

static int 
svd_handle_event_FXO_RINGING ( svd_t * const svd, int const chan_idx )
{
	ab_chan_t * ab_chan = &svd->ab->chans[chan_idx];
	svd_chan_t * chan_ctx = ab_chan->ctx;
	int err;
DFS
	if( chan_ctx->is_hotlined ){
		/* offhook */
		err = ab_FXO_line_hook( ab_chan, ab_chan_hook_OFFHOOK );
		if (err){
			SU_DEBUG_1(("can`t offhook on [_%d_]\n", 
					ab_chan->abs_idx));
			goto __exit_fail;
		}
		/* process the hotline sequence */
		err = svd_process_addr (svd, chan_idx,
				chan_ctx->hotline_addr);
		if(err){
			goto __exit_fail;
		}
	} else {
		SU_DEBUG_2 (("Got ringing event on channel [_%d_], "
				"it is not hotlined, but should be\n",
				ab_chan->abs_idx));
		goto __exit_fail;
	}
DFE
	return 0;
__exit_fail:
DFE
	return -1;
}


/** it uses !!g_conf */
static int 
svd_chans_init ( svd_t * const svd )
{
	int i;
	int j;
	int k;
	unsigned char route_id_len;
	unsigned char addrbk_id_len;
	int chans_num;
DFS
	route_id_len = g_conf.route_table.id_len;
	addrbk_id_len = g_conf.address_book.id_len;

	chans_num = svd->ab->chans_num;
	for (i=0; i<chans_num; i++){
		ab_chan_t * curr_chan;
		svd_chan_t * chan_c;
		curr_chan = &svd->ab->chans[ i ];
		curr_chan->ctx = malloc(sizeof(svd_chan_t));
		chan_c = curr_chan->ctx;
		if( !chan_c ){
			SU_DEBUG_0 ((LOG_FNC_A (LOG_NOMEM_A 
					("svd->ab->chans[i].ctx") ) ));
			goto __exit_fail;
		}
		memset (chan_c, 0, sizeof(*chan_c));

		/* ROUTER */
		/* route_id channel ctx sets */
		int route_id_sz = sizeof(*(chan_c->dial_status.route_id));

		chan_c->dial_status.route_id = 
				malloc( (route_id_len+1) * route_id_sz);
		if( !chan_c->dial_status.route_id ){
			SU_DEBUG_0 ((LOG_FNC_A (LOG_NOMEM_A 
					("chans[i].ctx->dial_status."
					"route_id") ) ));
			goto __exit_fail;
		}
		memset (chan_c->dial_status.route_id, 0, 
				route_id_sz * (route_id_len + 1));

		/* ADDRBOOK */
		/* address_book id channel data sets */
		int adbk_sz = sizeof(*(chan_c->dial_status.addrbk_id));

		chan_c->dial_status.addrbk_id = 
				malloc( (addrbk_id_len+1) * adbk_sz);
		if( !chan_c->dial_status.addrbk_id ){
			SU_DEBUG_0 ((LOG_FNC_A (LOG_NOMEM_A 
					("chans[i].ctx->dial_status."
					"addrbk_id") ) ));
			goto __exit_fail;
		}
		memset (chan_c->dial_status.addrbk_id, 0, 
				adbk_sz * (addrbk_id_len + 1));

	 	/* SDP */
		chan_c->rtp_sfd = -1;
		chan_c->remote_host = NULL;

	 	/* WAIT INDEXES */
		chan_c->local_wait_idx = -1;
		chan_c->remote_wait_idx = -1;

	 	/* HANDLE */
		chan_c->op_handle = NULL;

		/* ALL OTHER */
		svd_clear_call (svd, curr_chan);
	}
	
	/* HOTLINE */
	k = g_conf.hot_line.records_num;
	for (i=0; i<k; i++){
		struct htln_record_s * curr_rec = &g_conf.hot_line.records[ i ];
		int hl_id;

		if (curr_rec->id[0] == NET_MARKER){
			/*tag__ should not be there svd->net_is_hotlined = 1;*/
			svd->net_is_hotlined = 1;
			svd->net_hotline_addr = curr_rec->value;
			continue;
		}

		hl_id = strtol (curr_rec->id, NULL, 10);
		for (j=0; j<chans_num; j++){
			ab_chan_t * curr_chan = &svd->ab->chans [j];
			svd_chan_t * chan_ctx = curr_chan->ctx;
			if( hl_id == curr_chan->abs_idx ){
				chan_ctx->is_hotlined = 1;
				chan_ctx->hotline_addr = curr_rec->value;
			}
		}
	}

	/* RTP parameters */
	k = g_conf.rtp_prms.records_num;
	for (i=0; i<k; i++){
		struct rtp_record_s * curr_rec = &g_conf.rtp_prms.records[ i ];
		int rp_id;

		rp_id = strtol (curr_rec->id, NULL, 10);
		for (j=0; j<chans_num; j++){
			ab_chan_t * curr_chan = &svd->ab->chans [j];
			if( rp_id == curr_chan->abs_idx ){
				if ( curr_rec->OOB == evts_OOB_DEFAULT){
					curr_chan->rtp_cfg.nEvents = evts_OOB_ONLY;
				} else {
					curr_chan->rtp_cfg.nEvents = curr_rec->OOB;
				}
				if ( curr_rec->OOB_play == play_evts_DEFAULT){
					curr_chan->rtp_cfg.nPlayEvents = play_evts_PLAY;
				} else {
					curr_chan->rtp_cfg.nEvents = curr_rec->OOB_play;
				}

				curr_chan->rtp_cfg.evtPT = curr_rec->evtPT;
				curr_chan->rtp_cfg.evtPTplay = curr_rec->evtPTplay;
				curr_chan->rtp_cfg.cod_volume.enc_dB = curr_rec->COD_Tx_vol;
				curr_chan->rtp_cfg.cod_volume.dec_dB = curr_rec->COD_Rx_vol;
				curr_chan->rtp_cfg.VAD_cfg = curr_rec->VAD_cfg;
				curr_chan->rtp_cfg.HPF_is_ON = curr_rec->HPF_is_ON;
			}
		}
	}
DFE
	return 0;
__exit_fail:
DFE
	return -1;
}

/* *
 * Register waits on vinetic dev files 
 * 
 * dev1, dev2, dev3, dev4
 * dev_fd_N <-> wait_N
 * wait_N <-> dev_evts_func
 *
 * dev_evts_func - reenterable
 * 
 * FXS : digits(mass per chan), hooks(actions per chan)
 * FXO : ring(soft actions per chan)
 *
 * */
static int 
attach_dev_cb ( svd_t * const svd )
{
	su_wait_t wait[1];
	ab_dev_t * curr_dev;
	int i;
	int j;
	int err;
DFS
	j = svd->ab->devs_num;
	for(i = 0; i < j; i++){
		curr_dev = &svd->ab->devs[ i ];
		err = su_wait_create (wait, curr_dev->cfg_fd, POLLIN);
		if(err){
			SU_DEBUG_0 ((LOG_FNC_A ("su_wait_create() fails" ) ));
			goto __exit_fail;
		}

		err = su_root_register( svd->root, wait, 
				svd_atab_handler, curr_dev, 0);
		if (err == -1){
			SU_DEBUG_0 ((LOG_FNC_A ("su_root_register() fails" ) ));
			goto __exit_fail;
		}
	}
DFE
	return 0;
__exit_fail:
DFE
	return -1;
}


static int 
svd_handle_digit( svd_t * const svd, int const chan_idx, long const digit )
{
	ab_t * ab = svd->ab;
	svd_chan_t * chan_ctx = ab->chans[ chan_idx ].ctx;
	int err = 0;
DFS
	switch( chan_ctx->dial_status.state ){
		case dial_state_START:
			err = svd_handle_START (ab, chan_idx, digit);
			break;
		case dial_state_ADDR_BOOK:
			err = svd_handle_ADDR_BOOK(svd, chan_idx, digit);
			break;
		case dial_state_ROUTE_ID:
			err = svd_handle_ROUTE_ID(ab, chan_idx, digit);
			break;
		case dial_state_CHAN_ID:
			err = svd_handle_CHAN_ID(svd, chan_idx, digit);
			break;
		case dial_state_NET_ADDR:
			err = svd_handle_NET_ADDR(svd, chan_idx, digit);
			break;
	}
DFE
	return err;
}

/** uses !!g_conf */
static int 
svd_handle_START( ab_t * const ab, int const chan_idx, long const digit )
{
	svd_chan_t * chan_c = ab->chans[chan_idx].ctx;

	switch(digit){
		case ADBK_MARKER:
			chan_c->dial_status.state = dial_state_ADDR_BOOK;
			break;
		case SELF_MARKER:
			chan_c->dial_status.dest_is_self = self_YES;
			chan_c->dial_status.state = dial_state_CHAN_ID;
			break;
		default :
			if( !isdigit (digit) ){
				goto __exit_success;
			}
			chan_c->dial_status.route_id[ 0 ] = digit;
			if( g_conf.route_table.id_len != 1 ){
				chan_c->dial_status.state = dial_state_ROUTE_ID;
				chan_c->dial_status.tag = 1;
			} else {
				/* set route ip */
				int err;
				err = set_route_ip (chan_c);
				if (err){
					goto __exit_fail;
				}
				chan_c->dial_status.state = dial_state_CHAN_ID;
			}
			break;
	}

__exit_success:
	return 0;
__exit_fail:
	return -1;
}

/** uses !!g_conf */
static int 
svd_handle_ROUTE_ID( ab_t * const ab, int const chan_idx, long const digit )
{
	svd_chan_t * chan_c = ab->chans[chan_idx].ctx;
	int * route_idx = & (chan_c->dial_status.tag);
	int route_id_len = g_conf.route_table.id_len;

	if( !isdigit (digit) ){
		goto __exit_success;
	}
	chan_c->dial_status.route_id [*route_idx] = digit;
	++(*route_idx);
	if(*route_idx == route_id_len){
		/* we got all digits of route_id */
		/* set route ip */
		int err;
		err = set_route_ip (chan_c);
		if( err ){
			goto __exit_fail;
		} 
		SU_DEBUG_3 (("Choosed router [%s]\n",
				chan_c->dial_status.route_id ));
		chan_c->dial_status.state = dial_state_CHAN_ID;
		chan_c->dial_status.tag = 0;
	}

__exit_success:
	return 0;
__exit_fail:
	return -1;
}

static int 
svd_handle_CHAN_ID( svd_t * const svd, int const chan_idx, long const digit )
{
	svd_chan_t * chan_c = svd->ab->chans[chan_idx].ctx;
	int * chan_mas_idx = & (chan_c->dial_status.tag);
	int err = 0;

	if( *chan_mas_idx == 0 ){
		if( digit == FXO_MARKER ){
			err = local_connection_selector(svd, 1, chan_idx);
			goto __exit;
		} else if( digit == NET_MARKER ){
			if (g_conf.sip_set.all_set){
				chan_c->dial_status.state = dial_state_NET_ADDR;
			} else {
				SU_DEBUG_2 (("No registration available\n"));
				goto __exit_fail;
			}
			goto __exit_success;
		}
	}

	if( !isdigit (digit) ){
		goto __exit_success;
	}
	chan_c->dial_status.chan_id [*chan_mas_idx] = digit;
	++(*chan_mas_idx);
	if (*chan_mas_idx == CHAN_ID_LEN-1){
		/* we got all digits of chan_id */
		SU_DEBUG_3 (("Choosed chan [%s]\n",
				chan_c->dial_status.chan_id ));
		err = local_connection_selector(svd, 0, chan_idx);
	}

__exit:
	return err;
__exit_success:
	return 0;
__exit_fail:
	return -1;
}

static int 
svd_handle_NET_ADDR( svd_t * const svd, int const chan_idx, long const digit )
{
	ab_chan_t * ab_chan = &svd->ab->chans[chan_idx];
	svd_chan_t * chan_c = ab_chan->ctx;
	int * net_idx = & (chan_c->dial_status.tag);

	if (digit != NET_MARKER){
		/* put input digits to buffer */
		chan_c->dial_status.addr_payload[ *net_idx ] = digit;
		++(*net_idx);
	} else {
		/* place a call */
		svd_invite_to(svd, chan_idx, chan_c->dial_status.addr_payload);
		goto __exit_success;
	}

	/* when buffer is full but we did not find terminant*/
	if(*net_idx == ADDR_PAYLOAD_LEN){
		SU_DEBUG_2 (("Too long addr_payload [%s] it should be "
				"not longer then %d chars\n", 
				chan_c->dial_status.addr_payload,
				ADDR_PAYLOAD_LEN));
		goto __exit_fail;
	}

__exit_success:
	return 0;
__exit_fail:
	return -1;
}

/** uses !!g_conf */
static int 
svd_handle_ADDR_BOOK( svd_t * const svd, int const chan_idx, long const digit )
{
	ab_chan_t * ab_chan = &svd->ab->chans[chan_idx];
	svd_chan_t * chan_c = ab_chan->ctx;
	int * ab_id_idx = & (chan_c->dial_status.tag);
	int err = 0;

	/* put input digits to buffer */
	chan_c->dial_status.addrbk_id[ *ab_id_idx ] = digit;
	++(*ab_id_idx);

	/* when buffer is full */
	if(*ab_id_idx == g_conf.address_book.id_len){
		*ab_id_idx = 0;
		/* find appropriate addrbook value */
		err = svd_ab_value_set_by_id(ab_chan);
		if(err){
			goto __exit_fail;
		}
		/* process it */
		err = svd_process_addr (svd, chan_idx,
				chan_c->dial_status.addrbk_value);
		if(err){
			goto __exit_fail;
		}
	}

	return 0;
__exit_fail:
	return -1;
}

static int
svd_ab_value_set_by_id (ab_chan_t * const ab_chan)
{
	svd_chan_t * chan_c = ab_chan->ctx;
	int i;
	int j;
	struct adbk_record_s * cur_rec;
	
	j = g_conf.address_book.records_num;
	for (i = 0; i < j; i++){
		cur_rec = &g_conf.address_book.records[ i ];
		if( !strcmp (cur_rec->id, chan_c->dial_status.addrbk_id) ){
			chan_c->dial_status.addrbk_value = cur_rec->value;
			goto __exit_success;
		}
	}
	SU_DEBUG_2(("Wrong address book id : [%s]\n", 
			chan_c->dial_status.addrbk_id));
	return -1;
__exit_success:
	return 0;
}

static int
svd_process_addr (svd_t * const svd, int const chan_idx,
		char const * const value)
{
	ab_chan_t * ab_chan = &svd->ab->chans[chan_idx];
	svd_chan_t * chan_ctx = ab_chan->ctx;
	int val_len;
	int i;
	int err;
DFS
	chan_ctx->dial_status.state = dial_state_START;

	for(i = 0; value[i]; i++){
		err = svd_handle_digit ( svd, chan_idx, value[ i ] );
		if(err){
			/* clear call params */
			svd_clear_call (svd, ab_chan);
			goto __exit_fail;
		}
		if( i != 0 && chan_ctx->dial_status.state == 
				dial_state_START ){
			/* after that - just ',' and dialtones */
			i++;
			break;
		}
	}
	if(i < val_len){
		SU_DEBUG_2(("Call on hotline not implemented\n"));
		SU_DEBUG_2(("call to [%s] droped\n", &value[i]));
	}
#if 0
	nua_info (ab_chan->ctx->op_handle, 
			SIPTAG_CONTENT_TYPE_STR("text/plain"),
			SIPTAG_PAYLOAD_STR(&value[i]),
			TAG_END());
#endif

#if 0
	for(; i < val_len; i++){
		if( value[ i ] == WAIT_MARKER ){
			/* wait a second */
			sleep(1);
		} else {
			/* tag__ should play dial tone */
			SU_DEBUG_3(("CALL : %c\n", value[ i ]));
		}
	}
	err = ab_FXO_line_digit (&svd->ab->chans[3], 1, "9", 0, 0);
	SU_DEBUG_3(("ERR : %d\n", err));
#endif

#if 0
	ab_FXO_line_hook(&svd->ab->chans[3], ab_chan_hook_OFFHOOK);
	for(; i < val_len;){
		int number_length;
		for(j=i; (j<val_len) & (value[j]!=WAIT_MARKER); j++);
		number_length = j - i;
		err = ab_FXO_line_digit (&svd->ab->chans[3], number_length, 
				&value[i], 0, 0);
		SU_DEBUG_3(("ERR : %d\n", err));
		SU_DEBUG_3(("CALL %d[%d;%d...] : %s\n", j-i, i, j, &value[i]));
		i = j;
		sleep(number_length/3+1);
		SU_DEBUG_3(("CWAIT %d\n",number_length/3+1));
		while(value[i] == WAIT_MARKER){
			sleep(1);
			SU_DEBUG_3(("WAIT 1\n"));
			i++;
		}
	}
#endif
DFE
	return 0;
__exit_fail:
DFE
	return -1;
}

/* *
 * uses !!g_conf 
 * cleans old route_ip and sets new ip if router is not self
 * set self flag to proper value
 *
 * @param chan_c channel context to act on it
 *
 * @retval -1 in error case (Wrong router id)
 * @retval  0 self flag has been set to proper value and
 * 		router ip has been set if router is not self
 * */
static int 
set_route_ip (svd_chan_t * const chan_c)
{
	char * g_conf_id;
	char * route_id = chan_c->dial_status.route_id;
	int rec_idx;
	int routers_num;
	int ip_find = 0;

	if(chan_c->dial_status.dest_is_self == self_YES){
		goto __exit_success;
	}
	if( !strcmp(g_conf.self_number, route_id) ){
		/* it is self */
		SU_DEBUG_3 (("Chosed router is self\n"));
		chan_c->dial_status.dest_is_self = self_YES;
		goto __exit_success;
	}
	chan_c->dial_status.dest_is_self = self_NO;

	routers_num = g_conf.route_table.records_num;

	for( rec_idx = 0; rec_idx < routers_num; rec_idx++ ){
		g_conf_id = g_conf.route_table.records [rec_idx].id;
		if( !strcmp( g_conf_id, route_id ) ){
			ip_find = 1;
			break;
		}
	}
	if (ip_find){
		chan_c->dial_status.route_ip =
				g_conf.route_table.records[rec_idx].value;
	} else {
		SU_DEBUG_2(("Wrong router id [%s]\n", route_id ));
		goto __exit_fail;
	}

__exit_success:
	return 0;
__exit_fail:
	return -1;
}

static int 
local_connection_selector
		( svd_t * const svd, int const use_ff_FXO, int const chan_idx)
{
	svd_chan_t * chan_c = svd->ab->chans[chan_idx].ctx;
	int err;
DFS
	chan_c->dial_status.state = dial_state_START;

	if( chan_c->dial_status.dest_is_self == self_YES ){
		/* Destination router is self */
		err = svd_self_call (svd, chan_idx, use_ff_FXO);
	} else {
		/* Remote router local call */
		/* INVITE to remote router with index channel */
		err = svd_invite( svd, use_ff_FXO, chan_idx );
	}
	if (err){
		goto __exit_fail;
	}

DFE
	return 0;
__exit_fail:
DFE
	return -1;
}

static int
svd_self_call
	( svd_t * const svd, int const src_chan_idx, int const use_ff_FXO )
{
	int err;
	svd_chan_t * chan_ctx = svd->ab->chans[ src_chan_idx ].ctx;

	chan_ctx->dial_status.route_ip = g_conf.self_ip;
	/* INVITE to self router as to remote */
	err = svd_invite (svd, use_ff_FXO, src_chan_idx);
	return err;
}
/******************************************************************************/

static int 
svd_media_vinetic_handle_local_data (su_root_magic_t * root, su_wait_t * w,
		su_wakeup_arg_t * user_data)
{
	ab_chan_t * chan = user_data;
	svd_chan_t * chan_ctx = chan->ctx;
	struct sockaddr_in target_sock_addr;
	char buf [BUFF_PER_RTP_PACK_SIZE];
	int rode; 
	int sent; 

	/*
	assert( chan->ctx->remote_port != 0 );
	assert( chan->ctx->remote_host );
	*/
	if (chan_ctx->remote_port == 0 ||
			chan_ctx->remote_host == NULL){
		SU_DEBUG_2(("HLD() ERROR : r_host = %s, r_port = %d\n",
				chan_ctx->remote_host,
				chan_ctx->remote_port));
		rode = read(chan->rtp_fd, buf, sizeof(buf));
		SU_DEBUG_2(("RODE FROM rtp_stream : %d\n", rode));
		goto __exit_fail;
	}

	memset(&target_sock_addr, 0, sizeof(target_sock_addr));

	target_sock_addr.sin_family = AF_INET;
	target_sock_addr.sin_port = htons(chan_ctx->remote_port);
	inet_aton (chan_ctx->remote_host, &target_sock_addr.sin_addr);

	rode = read(chan->rtp_fd, buf, sizeof(buf));
	if (rode == 0){
		SU_DEBUG_2 ((LOG_FNC_A("wrong event")));
		goto __exit_fail;
	} else if(rode > 0){
		sent = sendto(chan_ctx->rtp_sfd, buf, rode, 0, 
				&target_sock_addr, sizeof(target_sock_addr));
		if (sent == -1){
			SU_DEBUG_2 (("HLD() ERROR : sent() : %d(%s)\n",
					errno, strerror(errno)));
			goto __exit_fail;
		} else if (sent != rode){
			SU_DEBUG_2(("HLD() ERROR :"
					"RODE FROM rtp_stream : %d, but "
					"SENT TO socket : %d\n",
					rode, sent));
			goto __exit_fail;
		}
	} else {
		SU_DEBUG_2 (("HLD() ERROR : read() : %d(%s)\n",
				errno, strerror(errno)));
		goto __exit_fail;
	} 

	return 0;
__exit_fail:
	return -1;
}

static int 
svd_media_vinetic_handle_remote_data (su_root_magic_t * root, su_wait_t * w,
		su_wakeup_arg_t * user_data)
{
	ab_chan_t * chan = user_data;
	svd_chan_t * chan_ctx = chan->ctx;
	unsigned char buf [BUFF_PER_RTP_PACK_SIZE];
	int received;
	int writed;

	assert( chan_ctx->rtp_sfd != -1 );

	received = recv(chan_ctx->rtp_sfd, buf, sizeof(buf), 0);

	/* tag__ use for testing oob
	if (buf[1] == 0x62 || buf[1] == 0xe2){
		int i = 12;
		SU_DEBUG_3(("%d = recv(), pd:%x[:",received, buf[1]));
		for (; i < received; i++){
			SU_DEBUG_3(("%X:",buf[i] ));
		}
		SU_DEBUG_3(("]\n"));
	}
	*/
	if (received == 0){
		SU_DEBUG_2 ((LOG_FNC_A("wrong event")));
		goto __exit_fail;
	} else if (received > 0){
		writed = write(chan->rtp_fd, buf, received);
		if (writed == -1){
			SU_DEBUG_2 (("HRD() ERROR : write() : %d(%s)\n",
					errno, strerror(errno)));
			goto __exit_fail;
		} else if (writed != received){
			SU_DEBUG_2(("HRD() ERROR :"
					"RECEIVED FROM socket : %d, but "
					"WRITED TO rtp-stream : %d\n",
					received, writed));
			goto __exit_fail;
		}
	} else {
		SU_DEBUG_2 (("HRD() ERROR : recv() : %d(%s)\n",
				errno, strerror(errno)));
		goto __exit_fail;
	} 
	return 0;
__exit_fail:
	return -1;
}

static int 
svd_media_vinetic_open_rtp (svd_chan_t * const chan_d)
{
	int i;
	long ports_count;
	struct sockaddr_in my_addr;
	int sock_fd;
	int rtp_binded = 0;
DFS
	sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock_fd == -1) {
		SU_DEBUG_1 (("OPEN_RTP() ERROR : socket() : %d(%s)\n",
				errno, strerror(errno)));
		goto __exit_fail;
	}

	ports_count = g_conf.rtp_port_last - g_conf.rtp_port_first + 1;
	for (i = 0; i < ports_count; i++) {
		memset(&my_addr, 0, sizeof(my_addr));
		my_addr.sin_family = AF_INET;
		my_addr.sin_port = htons(g_conf.rtp_port_first + i);
		my_addr.sin_addr.s_addr = htonl(INADDR_ANY);
		if ((bind(sock_fd, &my_addr, sizeof(my_addr))) != -1) {
			chan_d->rtp_port =g_conf.rtp_port_first + i;
			rtp_binded = 1;
			break;
		}
	}
	if( !rtp_binded ){
		SU_DEBUG_1(("svd_media_vinetic_open_rtp(): "
				"could not find free port for RTP in "
				"range [%d,%d]\n",
				g_conf.rtp_port_first, g_conf.rtp_port_last));
		goto __sock_opened;
	}

DFE
	return sock_fd;

__sock_opened:
	if(close (sock_fd)){
		SU_DEBUG_2 (("OPEN_RTP() ERROR : close() : %d(%s)\n",
				errno, strerror(errno)));
	}
__exit_fail:
DFE
	return -1;
}

