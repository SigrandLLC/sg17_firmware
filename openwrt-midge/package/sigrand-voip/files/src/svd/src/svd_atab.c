#include "svd.h"
#include "svd_cfg.h"
#include "libab/ab_api.h"
#include "svd_ua.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <assert.h>

#include "libab/tapi/include/drv_tapi_io.h"

#define RTP_PORT 8010
#define RTP_PORT_RANGE_LENGTH 1000 
#define BUFF_PER_RTP_PACK_SIZE 128 

static int svd_atab_handler (su_root_magic_t * root, su_wait_t * w, 
		su_wakeup_arg_t * user_data);

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
		( ab_t * const ab, int const chan_idx, long const digit );
static int svd_handle_ADDR_BOOK
		( ab_t * const ab, int const chan_idx, long const digit );

static int set_route_ip (svd_chan_t * const chan_data);

static int local_connection_selector
		( svd_t * const svd, int const use_ff_FXO, int const chan_idx);

static void do_self_fxs_call
	( ab_t * const ab, int const src_chan_idx, int const dest_chan_idx );
static void do_self_fxo_connect
	( ab_t * const ab, int const src_chan_idx, int const dest_chan_idx );

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
		SU_DEBUG_0 ((LOG_FNC_A(ab_err_get_str_last)));
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
};

void 
svd_atab_delete ( svd_t * const svd )
{
	int i;
	int j;
DFS
	assert (svd);

	if ( !(svd->ab) ){
#ifdef SVD_DEBUG_LOGS
		SU_DEBUG_3 ((LOG_FNC_A("svd->ab already deleted")));
#endif
		goto __exit;
	}

	/* ab_chans_magic_destroy */
	j = svd->ab->chans_num;
	for( i = 0; i < j; i++ ){
		svd_chan_t * curr_chan = svd->ab->chans[ i ].data;
		if (curr_chan){
			if( 	curr_chan->dial_status.route_id && 
				curr_chan->dial_status.route_id != 
				curr_chan->dial_status.route_id_s){
				free (curr_chan->dial_status.route_id);
			}
			if( 	curr_chan->dial_status.addrbk_id && 
				curr_chan->dial_status.addrbk_id != 
				curr_chan->dial_status.addrbk_id_s){
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
};

int 
svd_media_register (svd_t * const svd, ab_chan_t * const chan)
{
	su_wait_t wait[1];
	int ret;
DFS
	assert (chan);
	assert (chan->data);
	assert (chan->data->local_wait_idx == -1);
	assert (chan->data->remote_wait_idx == -1);

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
	chan->data->local_wait_idx = ret;

	ret = svd_media_vinetic_open_rtp (chan->data);
	if (ret == -1){
		goto __exit_fail;
	}
	chan->data->rtp_sfd = ret;

	ret = su_wait_create(wait, chan->data->rtp_sfd, SU_WAIT_IN);
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
	chan->data->remote_wait_idx = ret;

DFE
	return 0;
__exit_fail:
DFE
	return -1;
};

void
svd_media_unregister (svd_t * const svd, ab_chan_t * const chan)
{
	svd_chan_t * chan_c;
DFS
	assert (chan);
	assert (chan->data);
	assert (chan->data->rtp_sfd != -1);
	assert (chan->data->local_wait_idx != -1);
	assert (chan->data->remote_wait_idx != -1);

	chan_c = chan->data;

	if( close (chan_c->rtp_sfd) ){
		su_perror("svd_media_unregister() close()");
	}
	chan_c->rtp_sfd = -1;

	su_root_deregister (svd->root, chan_c->local_wait_idx);
	chan_c->local_wait_idx = -1;

	su_root_deregister (svd->root, chan_c->remote_wait_idx);
	chan_c->remote_wait_idx = -1;
DFE
};

/* *
 * clears call status on the channel
 * */
void 
svd_clear_call (svd_t * const svd, ab_chan_t * const chan)
{
	svd_chan_t * chan_c = chan->data;
	int len;
	int size;
DFS
	/* STATE */
	chan_c->dial_status.state = dial_state_START;
	
	/* TAG */
	chan_c->dial_status.tag = 0;

	/* DEST_IS_SELF */
	chan_c->dial_status.dest_is_self = self_UNDEFINED;

	/* ROUTER */
	len = chan_c->dial_status.route_id_len;
	if( len ){
		size = sizeof(*(chan_c->dial_status.route_id));
		memset(chan_c->dial_status.route_id, 0, size * len);
	}
	/* route_ip  */
	memset (chan_c->dial_status.route_ip, 0, 
			sizeof(chan_c->dial_status.route_ip));


	/* CHAN */
	size = sizeof(chan_c->dial_status.chan_id);
	memset (chan_c->dial_status.chan_id, 0, size);

	/* ADDRBOOK */
	len = chan_c->dial_status.addrbk_id_len;
	if( len ){
		size = sizeof(*(chan_c->dial_status.addrbk_id));
		memset(chan_c->dial_status.addrbk_id, 0, size * len);
	}

	/* PSTN_SIP */
	size = sizeof(chan_c->dial_status.pstn_sip_id);
	memset (chan_c->dial_status.pstn_sip_id, 0, size);

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
};

/* * 
 * return -1 on error case and 
 * other value on succsess 
 * if chan_abs_idx_str == NULL, using chan_abs_idx,
 * otherwise, using chan_abs_idx_str
 * */
int 
get_dest_chan_idx( ab_t const * const ab, 
		char const * const chan_abs_idx_str, char const chan_abs_idx, 
		int const use_ff_FXO )
{
	int ret_idx;
	int idx_to_find;
	int chans_num; 
	int i;
DFS
	if( use_ff_FXO ){
	/* tag__ */SU_DEBUG_3 ((LOG_FNC_A("first free FXO not implemented")));
		goto __exit_fail;
	}

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
		SU_DEBUG_2 (("Wrong chan id [%d]\n", idx_to_find));
		goto __exit_fail;
	}

DFE
	return ret_idx;
__exit_fail:
DFE
	return -1;
};

/******************************************************************************/
static int
svd_atab_handler (svd_t * svd, su_wait_t * w, 
		su_wakeup_arg_t * user_data)
{
/* *
 * FXS : digits(mass per chan), hooks(actions per chan)
 * FXO : ring(soft actions per chan)
 *
 * su_root_magic_t - svd
 * su_wakeup_arg_t - dev_ptr
 *
 * TODO : should be reenterable or mutexes must be used tag__
 * */
DFS
	ab_dev_t * ab_dev = (ab_dev_t *) user_data;
	ab_chan_t * ab_chan;
	ab_dev_event_t evt;
	int chan_idx;
	int dev_idx;
	int err;

	memset(&evt, 0, sizeof(evt));
	err = ab_dev_event_get( ab_dev, &evt );
	if( err ){
		SU_DEBUG_0 ((LOG_FNC_A (ab_err_get_str(ab_dev)) ));
		goto __exit_fail;
	}

	dev_idx = ab_dev->idx - 1;
	chan_idx = dev_idx * svd->ab->chans_per_dev + evt.ch;
	ab_chan	= &svd->ab->chans[chan_idx];

	/* in some errors we do not want to quit
	 * assert( evt.more == 0 ); */

	switch (evt.id){
		case ab_dev_event_FXS_OFFHOOK:{
#ifdef SVD_DEBUG_LOGS
			SU_DEBUG_3 (("OFFHOOK on device / channel : %d / %d\n ",
					dev_idx+1,evt.ch+1 ));
#endif
			/* stop ringing */
			err = ab_FXS_line_ring( ab_chan, ab_chan_ring_MUTE );
			if (err){
				SU_DEBUG_0(("can`t stop ring on [%d]\n",
					ab_chan->abs_idx));
				svd_answer(svd, ab_chan, 
						SIP_500_INTERNAL_SERVER_ERROR);
				goto __exit_fail;
			}

			/* change linefeed mode to ACTIVE */
			err = ab_FXS_line_feed (ab_chan, 
					ab_chan_linefeed_ACTIVE);
			if (err){
				SU_DEBUG_0(("can`t set linefeed to "
						"active on [%d]\n",
						ab_chan->abs_idx));
				svd_answer(svd, ab_chan, 
						SIP_500_INTERNAL_SERVER_ERROR);
				goto __exit_fail;
			}

			/* answer on call if it exists */
			svd_answer(svd, ab_chan, SIP_200_OK);
			break;
		}
		case ab_dev_event_FXS_ONHOOK:{
#ifdef SVD_DEBUG_LOGS
			SU_DEBUG_3 (("ONHOOK on device / channel : %d / %d\n ",
					dev_idx+1,evt.ch+1 ));
#endif
			/* say BYE on existing connection */
			svd_bye(svd, ab_chan);

			/* change linefeed mode to STANDBY */
			err = ab_FXS_line_feed (ab_chan, 
					ab_chan_linefeed_STANDBY);
			if (err){
				SU_DEBUG_0(("can`t set linefeed to "
						"standby on [%d]\n",
						ab_chan->abs_idx));
				goto __exit_fail;
			}

			break;
		}
		case ab_dev_event_FXS_DIGIT_TONE:{
			char digit = evt.data;
#ifdef SVD_DEBUG_LOGS
			SU_DEBUG_3 (("DIGIT \'%c\', on device / channel : "
					"%d / %d\n ",
					digit, dev_idx+1,evt.ch+1 ));
			SU_DEBUG_3 (("local : %d, network : %d\n",
					(evt.data >> 9),
					(evt.data >> 8) & 1 ));

			SU_DEBUG_3 (("HN : %p\n",ab_chan->data->op_handle));
#endif
			if( ab_chan->data->op_handle ){
				/* allready connected - send info 
				 * see rfc_2976
				 */
			} else {
				/* not connected yet - should process digits */
				err = svd_handle_digit (svd,chan_idx, digit);
				if(err){
					/* clear call params */
					svd_clear_call (svd, ab_chan);
					goto __exit_fail;
				}
			}
			break;
		}
		case ab_dev_event_FXS_DIGIT_PULSE:{
			SU_DEBUG_2 (("Dialed in pulse mode "
					"on device / channel %d / %d\n "
					"->You must switch to tone mode\n",
					dev_idx+1,evt.ch+1 ));
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
		case ab_dev_event_FXO_RINGING:{
#ifdef SVD_DEBUG_LOGS
			SU_DEBUG_3 (("Got ringing event : 0x%X "
					"on device / channel %d / %d\n",
					evt.data, dev_idx+1,evt.ch+1 ));
#endif
			/* TODO 1 : tag__ hotline to some fxs channel */
			break;
		}
	}

DFE
	return 0;
__exit_fail:
DFE
	return -1;
};


/** it uses !!g_conf */
static int 
svd_chans_init ( svd_t * const svd )
{
	int i;
	int j;
	int k;
	unsigned char route_id_len;
	unsigned char route_id_dynamic = 0;
	unsigned char addrbk_id_len;
	unsigned char addrbk_id_dynamic = 0;
	int chans_num;
DFS
	route_id_len = g_conf.route_table.id_len;

	if( route_id_len + 1 > ROUTE_ID_LEN_DF ){
		route_id_dynamic = 1;
	}
	addrbk_id_len = g_conf.address_book.id_len;
	if( addrbk_id_len + 1 > ADBK_ID_LEN_DF ){
		addrbk_id_dynamic = 1;
	}

	chans_num = svd->ab->chans_num;
	for( i = 0; i < chans_num; i++ ){
		ab_chan_t * curr_chan;
		svd_chan_t * chan_d;
		curr_chan = &svd->ab->chans[ i ];
		curr_chan->data = malloc(sizeof(*(curr_chan->data)));
		chan_d = curr_chan->data;
		if( !chan_d ){
			SU_DEBUG_0 ((LOG_FNC_A (LOG_NOMEM_A 
					("svd->ab->chans[i].data") ) ));
			goto __exit_fail;
		}
		memset (chan_d, 0, sizeof(*(chan_d)));

		/* ROUTER */
		/* route_id channel data sets */
		if( route_id_dynamic ){
			int rem_id_sz = sizeof(
				*(chan_d->dial_status.route_id));

			chan_d->dial_status.route_id = 
					malloc( (route_id_len+1) * rem_id_sz);
			if( !chan_d->dial_status.route_id ){
				SU_DEBUG_0 ((LOG_FNC_A (LOG_NOMEM_A 
						("chans[i].data->dial_status."
						"route_id") ) ));
				goto __exit_fail;
			}
			memset (chan_d->dial_status.route_id, 0, 
					rem_id_sz * (route_id_len + 1));
		} else {
			chan_d->dial_status.route_id = 
				chan_d->dial_status.route_id_s;
		}
		chan_d->dial_status.route_id_len = route_id_len;

		/* ADDRBOOK */
		/* address_book id channel data sets */
		if( addrbk_id_dynamic ){
			int adbk_sz = sizeof(
				*(curr_chan->data->dial_status.addrbk_id));

			chan_d->dial_status.addrbk_id = 
					malloc( (addrbk_id_len+1) * adbk_sz);
			if( !chan_d->dial_status.addrbk_id ){
				SU_DEBUG_0 ((LOG_FNC_A (LOG_NOMEM_A 
						("chans[i].data->"
						"dial_status."
						"addrbk_id") ) ));
				goto __exit_fail;
			}
			memset (chan_d->dial_status.addrbk_id, 0, 
					adbk_sz * (addrbk_id_len + 1));
		} else {
			chan_d->dial_status.addrbk_id = 
				chan_d->dial_status.addrbk_id_s;
		}
		chan_d->dial_status.addrbk_id_len = addrbk_id_len;

	 	/* SDP */
		chan_d->rtp_sfd = -1;
		chan_d->remote_host = NULL;

	 	/* WAIT INDEXES */
		chan_d->local_wait_idx = -1;
		chan_d->remote_wait_idx = -1;

	 	/* HANDLE */
		chan_d->op_handle = NULL;

		/* ALL OTHER */
		svd_clear_call (svd, curr_chan);
	}
	
	/* HOTLINE */
	k = g_conf.hot_line.records_num;
	for(i = 0; i < k; i++){
		struct htln_record_s * curr_rec = &g_conf.hot_line.records[ i ];
		int hl_id;

		if (curr_rec->id[0] == NET_MARKER){
			svd->net_is_hotlined = 1;
			svd->net_hotline_addr_len = curr_rec->value_len;
			svd->net_hotline_addr = curr_rec->value;
			continue;
		}

		hl_id = strtol (curr_rec->id, NULL, 10);
		for(j = 0; j < chans_num; j++){
			ab_chan_t * curr_chan = &svd->ab->chans[ j ];
			if( hl_id == curr_chan->abs_idx ){
				curr_chan->data->is_hotlined = 1;
				curr_chan->data->hotline_addr_len = 
						curr_rec->value_len;
				curr_chan->data->hotline_addr = 
						curr_rec->value;
			}
		}
	}
DFE
	return 0;
__exit_fail:
DFE
	return -1;
};

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
static int attach_dev_cb ( svd_t * const svd )
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
};


static int 
svd_handle_digit( svd_t * const svd, int const chan_idx, long const digit )
{
	ab_t * ab = svd->ab;
	int err = 0;
DFS
	switch( ab->chans[ chan_idx ].data->dial_status.state ){
		case dial_state_START:
			err = svd_handle_START (ab, chan_idx, digit);
			break;
		case dial_state_ADDR_BOOK:
			err = svd_handle_ADDR_BOOK(ab, chan_idx, digit);
			break;
		case dial_state_ROUTE_ID:
			err = svd_handle_ROUTE_ID(ab, chan_idx, digit);
			break;
		case dial_state_CHAN_ID:
			err = svd_handle_CHAN_ID(svd, chan_idx, digit);
			break;
		case dial_state_NET_ADDR:
			err = svd_handle_NET_ADDR(ab, chan_idx, digit);
			break;
	}
DFE
	return err;
};

/** uses !!g_conf */
static int 
svd_handle_START( ab_t * const ab, int const chan_idx, long const digit )
{
	svd_chan_t * curr_data = ab->chans[chan_idx].data;

	switch(digit){
		case ADBK_MARKER:
			curr_data->dial_status.state = dial_state_ADDR_BOOK;
			break;
		case SELF_MARKER:
			curr_data->dial_status.dest_is_self = self_YES;
			curr_data->dial_status.state = dial_state_CHAN_ID;
			break;
		default :
			if( !isdigit (digit) ){
				SU_DEBUG_2((LOG_FNC_A("not a digit on input")));
				goto __exit_fail;
			}
			curr_data->dial_status.route_id[ 0 ] = digit;
			if( curr_data->dial_status.route_id_len != 1 ){
				curr_data->dial_status.state = 
						dial_state_ROUTE_ID;
				curr_data->dial_status.tag = 1;
			} else {
				/* set route ip */
				int err;
				err = set_route_ip (curr_data);
				if (err){
					goto __exit_fail;
				}
				curr_data->dial_status.state = 
						dial_state_CHAN_ID;
			}
			break;
	}

	return 0;
__exit_fail:
	return -1;
};

/** uses !!g_conf */
static int 
svd_handle_ROUTE_ID( ab_t * const ab, int const chan_idx, long const digit )
{
	svd_chan_t * curr_data = ab->chans[chan_idx].data;
	int * route_idx = & (curr_data->dial_status.tag);
	int route_id_len = curr_data->dial_status.route_id_len;

	if( !isdigit (digit) ){
		SU_DEBUG_2 ((LOG_FNC_A("not a digit on input")));
		goto __exit_fail;
	}
	curr_data->dial_status.route_id [*route_idx] = digit;
	++(*route_idx);
	if(*route_idx == route_id_len){
		/* we got all digits of route_id */
		/* set route ip */
		int err;
		err = set_route_ip (curr_data);
		if( err ){
			goto __exit_fail;
		} 
		SU_DEBUG_3 (("Choosed router [%s]\n",
				curr_data->dial_status.route_id ));
		curr_data->dial_status.state = dial_state_CHAN_ID;
		curr_data->dial_status.tag = 0;
	}

	return 0;
__exit_fail:
	return -1;
};

static int 
svd_handle_CHAN_ID( svd_t * const svd, int const chan_idx, long const digit )
{
	svd_chan_t * curr_data = svd->ab->chans[chan_idx].data;
	int * chan_mas_idx = & (curr_data->dial_status.tag);
	int err = 0;

	if( *chan_mas_idx == 0 ){
		if( digit == FXO_MARKER ){
			err = local_connection_selector(svd, 1, chan_idx);
			goto __exit;
		} else if( digit == NET_MARKER ){
			curr_data->dial_status.state = dial_state_NET_ADDR;
			goto __exit_success;
		}
	}

	if( !isdigit (digit) ){
		SU_DEBUG_2 ((LOG_FNC_A("not a digit on input")));
		goto __exit_fail;
	}
	curr_data->dial_status.chan_id [*chan_mas_idx] = digit;
	++(*chan_mas_idx);
	if (*chan_mas_idx == CHAN_ID_LEN-1){
		/* we got all digits of chan_id */
		SU_DEBUG_3 (("Choosed chan [%s]\n",
				curr_data->dial_status.chan_id ));
		err = local_connection_selector(svd, 0, chan_idx);
	}

__exit:
	return err;
__exit_success:
	return 0;
__exit_fail:
	return -1;
};

static int 
svd_handle_NET_ADDR( ab_t * const ab, int const chan_idx, long const digit )
{
	/*tag__ */SU_DEBUG_3 ((LOG_FNC_A("not implemented")));
	return -1;
};

static int 
svd_handle_ADDR_BOOK( ab_t * const ab, int const chan_idx, long const digit )
{
	/*tag__ */SU_DEBUG_3 ((LOG_FNC_A("not implemented")));
	return -1;
};

/* *
 * uses !!g_conf 
 * cleans old route_ip and sets new ip if router is not self
 * set self flag to proper value
 *
 * @param chan_data channel to act on it
 *
 * @retval -1 in error case (Wrong router id)
 * @retval  0 self flag has been set to proper value and
 * 		router ip has been set if router is not self
 * */
static int 
set_route_ip (svd_chan_t * const chan_data)
{
	char * g_conf_id;
	char * route_id = chan_data->dial_status.route_id;
	int rec_idx;
	int routers_num;
	int ip_find = 0;

	if(chan_data->dial_status.dest_is_self == self_YES){
		goto __exit_success;
	}
	if( !strcmp(g_conf.self_number, route_id) ){
		/* it is self */
		SU_DEBUG_3 (("Chosed router is self\n"));
		chan_data->dial_status.dest_is_self = self_YES;
		goto __exit_success;
	}
	chan_data->dial_status.dest_is_self = self_NO;

	routers_num = g_conf.route_table.records_num;

	for( rec_idx = 0; rec_idx < routers_num; rec_idx++ ){
		g_conf_id = g_conf.route_table.records [rec_idx].id;
		if( !strcmp( g_conf_id, route_id ) ){
			ip_find = 1;
			break;
		}
	}
	if (ip_find){
		strcpy (chan_data->dial_status.route_ip,
				g_conf.route_table.records[rec_idx].value);
	} else {
		SU_DEBUG_2(("Wrong router id [%s]\n", route_id ));
		goto __exit_fail;
	}

__exit_success:
	return 0;
__exit_fail:
	return -1;
};

static int 
local_connection_selector
		( svd_t * const svd, int const use_ff_FXO, int const chan_idx)
{
	ab_t * ab = svd->ab;
	svd_chan_t * curr_data = svd->ab->chans[chan_idx].data;
	int err;
DFS

	curr_data->dial_status.state = dial_state_START;

	if( curr_data->dial_status.dest_is_self == self_YES ){
		/* destination router is self */

		int dest_chan_ab_idx;
		ab_dev_type_t dest_dev_type;

		/* get destination channel index */
		dest_chan_ab_idx = get_dest_chan_idx( ab,
				curr_data->dial_status.chan_id, 0, use_ff_FXO );
		if( dest_chan_ab_idx == -1 ){
			goto __exit_fail;
		} 
		if( dest_chan_ab_idx == chan_idx ){
			SU_DEBUG_2 ((LOG_FNC_A("you can`t call to yourself")));
			goto __exit_fail;
		}

		/* get destination channel type */
		dest_dev_type = ab->chans[ dest_chan_ab_idx ].parent->type;
		if (dest_dev_type == ab_dev_type_FXS){
			/* self FXS call */

			/* clear call states */
			svd_clear_call(svd, &ab->chans[chan_idx]);

			/* make call to fxs channel on self router */
			do_self_fxs_call (ab, chan_idx, dest_chan_ab_idx);

			goto __exit_success;
		} else if (dest_dev_type == ab_dev_type_FXO){
			/* self FXO call */

			/* clear call states */
			svd_clear_call(svd, &ab->chans[chan_idx]);

			/* make connect to fxo channel on self router */
			do_self_fxo_connect (ab, chan_idx, dest_chan_ab_idx);

			goto __exit_success;
		}
	}
	/* Remote router local call */

	/* INVITE to remote router with index channel */
	err = svd_invite( svd, use_ff_FXO, chan_idx );
	if (err){
		goto __exit_fail;
	}

__exit_success:
DFE
	return 0;
__exit_fail:
DFE
	return -1;
};


static void 
do_self_fxs_call
	( ab_t * const ab, int const src_chan_idx, int const dest_chan_idx )
{
	/*tag__ */SU_DEBUG_3 ((LOG_FNC_A("not implemented")));
};

static void 
do_self_fxo_connect
	( ab_t * const ab, int const src_chan_idx, int const dest_chan_idx )
{
	/*tag__ */SU_DEBUG_3 ((LOG_FNC_A("not implemented")));
};


/******************************************************************************/

static int 
svd_media_vinetic_handle_local_data (su_root_magic_t * root, su_wait_t * w,
		su_wakeup_arg_t * user_data)
{
	ab_chan_t * chan = user_data;
	struct sockaddr_in target_sock_addr;
	char buf [BUFF_PER_RTP_PACK_SIZE];
	int rode; 
	int sent; 

	/*
	assert( chan->data->remote_port != 0 );
	assert( chan->data->remote_host );
	*/
	if (chan->data->remote_port == 0 ||
			chan->data->remote_host == NULL){
		SU_DEBUG_2(("HLD() ERROR : r_host = %s, r_port = %d\n",
				chan->data->remote_host,
				chan->data->remote_port));
		rode = read(chan->rtp_fd, buf, sizeof(buf));
		SU_DEBUG_2(("RODE FROM rtp_stream : %d\n", rode));
		goto __exit_fail;
	}

	memset(&target_sock_addr, 0, sizeof(target_sock_addr));

	target_sock_addr.sin_family = AF_INET;
	target_sock_addr.sin_port = htons(chan->data->remote_port);
	inet_aton (chan->data->remote_host, &target_sock_addr.sin_addr);
	/* ret */

	rode = read(chan->rtp_fd, buf, sizeof(buf));
	if (rode == 0){
		SU_DEBUG_2 ((LOG_FNC_A("wrong event")));
		goto __exit_fail;
	} 
	
	else if(rode > 0){
		sent = sendto(chan->data->rtp_sfd, buf, rode, 0, 
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
		}
	}

	/*
	while (rode > 0){
		sent = sendto(chan->data->rtp_sfd, buf, rode, 0, 
				&target_sock_addr, sizeof(target_sock_addr));
		if (sent == -1){
			su_perror("svd_media_vinetic_handle_local_data() "
					"sent()");
			goto __exit_fail;
		} else if (sent != rode){
			SU_DEBUG_2(("svd_media_vinetic_handle_local_data() "
					"RODE FROM rtp_stream : %d, but "
					"SENT TO socket : %d\n",
					rode, sent));
		}

		rode = read(chan->rtp_fd, buf, sizeof(buf));
	}
	*/
	if (rode < 0){
		SU_DEBUG_2 (("HLD() ERROR : read() : %d(%s)\n",
				errno, strerror(errno)));
		goto __exit_fail;
	} 

	return 0;
__exit_fail:
	return -1;
};

static int 
svd_media_vinetic_handle_remote_data (su_root_magic_t * root, su_wait_t * w,
		su_wakeup_arg_t * user_data)
{
	ab_chan_t * chan = user_data;
	unsigned char buf [BUFF_PER_RTP_PACK_SIZE];
	int received;
	int writed;

	assert( chan->data->rtp_sfd != -1 );

/* ONCE RECEIVED METHOD - in while - resource temporary unavailable error */
	received = recv(chan->data->rtp_sfd, buf, sizeof(buf), 0);

	/* tag__ use for testing 
	if( (buf[1] != 18) && (buf[1] != 8) ){
		int i = 12;
		SU_DEBUG_2(("%d = recv(), pd:%x[:",received, buf[1]));
		for (; i < received; i++){
			SU_DEBUG_2(("%X:",buf[i] ));
		}
		SU_DEBUG_2(("]\n"));
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
		}
	} else {
		SU_DEBUG_2 (("HRD() ERROR : recv() : %d(%s)\n",
				errno, strerror(errno)));
		goto __exit_fail;
	}

/*
	received = recv(chan->data->rtp_sfd, buf, sizeof(buf), 0);
	if (received == 0){
		SU_DEBUG_2(("svd_media_vinetic_handle_remote_data() - "
				"wrong event\n"));
		goto __exit_fail;
	}

	while (received > 0){
		writed = write(chan->rtp_fd, buf, received);
		if (writed == -1){
			su_perror("svd_media_vinetic_handle_remote_data() "
					"write()");
			goto __exit_fail;
		} else if (writed != received){
			SU_DEBUG_2(("svd_media_vinetic_handle_remote_data() "
					"RECEIVED FROM socket : %d, but "
					"WRITED TO rtp-stream : %d\n",
					received, writed));
		}
		SU_DEBUG_2(("svd_media_vinetic_handle_remote_data() "
					"RECEIVED FROM socket : %d, "
					"WRITED TO rtp-stream : %d\n",
					received, writed));

		received = recv(chan->data->rtp_sfd, buf, sizeof(buf), 0);
	}
	if (received < 0){
		su_perror("svd_media_vinetic_handle_remote_data() recv()");
		goto __exit_fail;
	} 
*/
	return 0;
__exit_fail:
	return -1;
};

static int 
svd_media_vinetic_open_rtp (svd_chan_t * const chan_d)
{
	int i;
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

	for (i = 0; i < RTP_PORT_RANGE_LENGTH; i++) {
		memset(&my_addr, 0, sizeof(my_addr));
		my_addr.sin_family = AF_INET;
		my_addr.sin_port = htons(RTP_PORT + i);
		my_addr.sin_addr.s_addr = htonl(INADDR_ANY);
		if ((bind(sock_fd, &my_addr, sizeof(my_addr))) != -1) {
			chan_d->rtp_port = RTP_PORT + i;
			rtp_binded = 1;
			break;
		}
	}
	if( !rtp_binded ){
		SU_DEBUG_1(("svd_media_vinetic_open_rtp(): "
				"could not find free port for RTP in "
				"range [%d,%d]\n",
				RTP_PORT, RTP_PORT+RTP_PORT_RANGE_LENGTH));
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
};

