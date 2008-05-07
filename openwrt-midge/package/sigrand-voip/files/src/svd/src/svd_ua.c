#include "svd.h"
#include "svd_ua.h"
#include <errno.h>
#include <assert.h>

/************************************************************************ UAC */

static int 
svd_pure_invite( svd_t * const svd, ab_chan_t * const chan, 
		char const * const from_str, char const * const to_str );


/************************************************************************ UAS */
static void 
svd_i_error(int const status, char const * const phrase);
static void 
svd_i_invite( int status, char const * phrase, svd_t * const svd, 
		nua_handle_t * nh, ab_chan_t * chan, 
		sip_t const *sip, tagi_t tags[]);
static void 
svd_i_cancel (nua_handle_t const * const nh, ab_chan_t const * const chan);
//static void 
//svd_i_ack (nua_handle_t const * const nh, ab_chan_t const * const chan);
static void
svd_i_state (int status, char const *phrase, nua_t * nua, svd_t * svd,
		nua_handle_t * const nh, ab_chan_t * chan, sip_t const *sip,
		tagi_t tags[]);
static void
svd_i_bye (nua_handle_t const * const nh, ab_chan_t const * const chan);
static void 
svd_i_prack (nua_handle_t * nh, ab_chan_t const * const chan, 
		sip_t const * const sip);
static void 
svd_i_info(int status, char const * phrase, svd_t * const svd, 
		nua_handle_t * nh, ab_chan_t * chan, sip_t const * sip);
static void 
svd_r_invite( int status, char const *phrase, nua_t * nua, svd_t * svd,
		nua_handle_t * nh, ab_chan_t * chan, sip_t const *sip,
		tagi_t tags[]);
static void 
svd_r_get_params( int status, char const *phrase, nua_t * nua, svd_t * svd, 
		nua_handle_t * nh, ab_chan_t * chan, sip_t const *sip, 
		tagi_t tags[] );
static void 
svd_r_shutdown( int status, char const *phrase, nua_t * nua, svd_t * svd, 
		nua_handle_t * nh, ab_chan_t * chan, sip_t const *sip, 
		tagi_t tags[] );
static void
svd_r_bye(int status, char const *phrase,
	  nua_handle_t const * const nh, ab_chan_t const * const chan);

static void 
svd_media_parse_sdp(svd_t * const svd, ab_chan_t * const chan, 
		char const * str);

/****************************************************************************/

void svd_nua_callback(	nua_event_t  event,
			int          status,
			char const   *phrase,
			nua_t        *nua,
			svd_t        *svd,
			nua_handle_t *nh,
			ab_chan_t    *chan,
			sip_t const  *sip,
			tagi_t       tags[] )
{
DFS
	SU_DEBUG_3(("Event : %s\n",nua_event_name(event)));
	switch (event) {

/************ Indications ************/
		case nua_i_error: /*< 0 Error indication */
			svd_i_error(status, phrase);
			break;
		case nua_i_invite: /*< 1 Incoming call INVITE */
			svd_i_invite(status,phrase,svd,nh,chan,sip,tags);
			break;
		case nua_i_cancel: /*< 2 Incoming INVITE has been cancelled */
			svd_i_cancel(nh, chan);
			break;
		case nua_i_ack: /*< 3 Final response to INVITE has been ACKed */
			//svd_i_ack(nh, chan);
			break;
		case nua_i_fork:	/*< 4 Outgoing call has been forked */
			break;

		/* DEPRECATED *****/
		case nua_i_active:	/*< 5 A call has been activated */
		case nua_i_terminated:	/*< 6 A call has been terminated */
			break;
		/* DEPRECATED END */

		case nua_i_state: /*< 7 Call state has changed */
			svd_i_state (status, phrase, nua, svd, 
					nh, chan, sip, tags);
			break;
		case nua_i_outbound:	/*< 8 Status from outbound processing */
			break;
		case nua_i_bye: 	/*< 9 Incoming BYE call hangup */
			svd_i_bye(nh, chan);
			break;
					/* Incoming set first in comment */
		case nua_i_options:	/*< 10 OPTIONS */
		case nua_i_refer:	/*< 11 REFER call transfer */
		case nua_i_publish:	/*< 12 PUBLISH */
			break;
		case nua_i_prack:  	/*< 13 PRACK */
			svd_i_prack(nh, chan, sip);
			break;
		case nua_i_info:	/*< 14 session INFO */
			svd_i_info(status, phrase, svd, nh, chan, sip);
			break;

		case nua_i_update:	/*< 15 session UPDATE */
		case nua_i_message:	/*< 16 MESSAGE */
		case nua_i_chat:	/*< 17 chat MESSAGE  */
		case nua_i_subscribe:	/*< 18 SUBSCRIBE  */
		case nua_i_subscription:/*< 19 subscription to be authorized */
		case nua_i_notify:	/*< 20 event NOTIFY */
		case nua_i_method:	/*< 21 unknown method */
					/* NO Incoming set first in comment */
		case nua_i_media_error:	/*< 22 Offer-answer error indication */
			break;

/************ Responses ************/
		/*< 23 Answer to nua_set_params() or nua_get_hparams().*/
		case nua_r_set_params:
			break;
		/*< 24 Answer to nua_get_params() or nua_get_hparams().*/
		case nua_r_get_params: 
			svd_r_get_params(status, phrase, nua, svd, 
					nh, chan, sip, tags);
			break;
		case nua_r_shutdown:	/*< 25 Answer to nua_shutdown() */
			svd_r_shutdown(status, phrase, nua, svd, 
					nh, chan, sip, tags);
			break;
		case nua_r_notifier:	/*< 26 Answer to nua_notifier() */
		case nua_r_terminate:	/*< 27 Answer to nua_terminate() */
		case nua_r_authorize:	/*< 28 Answer to nua_authorize()  */
			break;

/************ SIP Responses ************/
		case nua_r_register:/**< 29 Answer to outgoing REGISTER */
		/*	svd_r_register(status, phrase, nua, svd, 
					nh, chan, sip, tags); 
		*/
			break;
		case nua_r_unregister:/**< 30 Answer to outgoing un-REGISTER */
		/*	svd_r_unregister(status, phrase, nua, svd, 
					nh, chan, sip, tags);
		*/
			break;
		case nua_r_invite:/**< 31 Answer to outgoing INVITE */
			svd_r_invite(status, phrase, nua, svd, 
					nh, chan, sip, tags);
			break;
		case nua_r_cancel:	/*< 32 Answer to outgoing CANCEL */
			break;
		case nua_r_bye:		/**< 33 Answer to outgoing BYE */
			svd_r_bye(status, phrase, nh, chan);
			break;
		case nua_r_options:	/*< 34 Answer to outgoing OPTIONS */
		case nua_r_refer:	/*< 35 Answer to outgoing REFER */
		case nua_r_publish:	/*< 36 Answer to outgoing PUBLISH */
		case nua_r_unpublish:/*< 37 Answer to outgoing un-PUBLISH */
			break;
		case nua_r_info:	 /*< 38 Answer to outgoing INFO */
			svd_r_info(status, phrase, svd, nh, chan, sip);
			break;
		case nua_r_prack:	/*< 39 Answer to outgoing PRACK */
		case nua_r_update:	 /*< 40 Answer to outgoing UPDATE */
		case nua_r_message:	/*< 41 Answer to outgoing MESSAGE */
		case nua_r_chat: /**< 42 Answer to outgoing chat message */
		case nua_r_subscribe:	/*< 43 Answer to outgoing SUBSCRIBE */
		case nua_r_unsubscribe:/*< 44 Answer to outgoing un-SUBSCRIBE */
		case nua_r_notify:	/*< 45 Answer to outgoing NOTIFY */
		case nua_r_method:/*< 46 Answer to unknown outgoing method */
		case nua_r_authenticate:/*< 47 Answer to nua_authenticate() */
			break;

		default:
			/* unknown event received */
		/*  если unknown event и nh неизвестен - нужно его уничтожить 
		 * (nua_handle_destroy) иначе (related to an existing call or 
		 * registration for instance). - игнорировать
		 */
			SU_DEBUG_2(("UNKNOWN EVENT RECEIVED : %d %s\n",
					status, phrase));
	}
DFE
};

/********************************************************************** UAC */

/**
 * Sends an outgoing INVITE request.
 *
 * @param svd context pointer
 * @param use_ff_FXO use first free fxo port on destination router
 * @param chan_idx initiator channel index
 *
 * use use !!g_conf
 */
int
svd_invite (svd_t * const svd, int const use_ff_FXO, int const chan_idx )
{
	svd_chan_t * chan_data = svd->ab->chans[chan_idx].data;
	char to_str[100];
	char from_str[100];
	char from_idx[ CHAN_ID_LEN ];
	int err;
DFS
	/* forming from string */
	snprintf (from_idx, CHAN_ID_LEN-1, "%d",
			svd->ab->chans[chan_idx].abs_idx);
	strcpy (from_str, "sip:");
	strcat (from_str, from_idx);
	strcat (from_str, "@");
	strcat (from_str, g_conf.self_ip);

	/* forming dest string */
	strcpy (to_str, "sip:");

	if (use_ff_FXO){
		strcat (to_str, FIRST_FREE_FXO);
	} else {
		strcat (to_str, chan_data->dial_status.chan_id);
	}
	strcat (to_str, "@");
	strcat (to_str, chan_data->dial_status.route_ip);
	
	/* send invite request */
	err = svd_pure_invite (svd, &svd->ab->chans[chan_idx], 
			from_str, to_str );
	if (err){
		goto __exit_fail;
	}

DFE
	return 0;
__exit_fail:
DFE
	return -1;
};


void svd_shutdown(svd_t * svd)
{
DFS
	nua_shutdown(svd->nua);
DFE
};

/** 
 * Answers a call (processed in two phases). 
 *
 * See also svd_i_invite().
 */
int
svd_answer(svd_t * const svd, ab_chan_t * const chan,  
		int status, char const *phrase)
{
	int call_answered = 0;
DFS
	if ( chan->data->op_handle ){
		/* we have call to answer */
		char l_sdp_str[1000];

		call_answered = 1;

		/* register media waits and open sockets for rtp */
		svd_media_register (svd, chan);

		snprintf(l_sdp_str, 1000, "v=0\r\nm=audio %d "
				"RTP/AVP 18 8\r\na=rtpmap:18 "
				"G729/8000\r\na=rtpmap:8 PCMA/8000\r\n",
				chan->data->rtp_port);
#ifdef SVD_DEBUG_LOGS
		SU_DEBUG_3 ((LOG_FNC_A("Answer on call")));
		SU_DEBUG_3 (("Local SDP :\n%s\n",l_sdp_str));
#endif
		if (status < 200 || status >= 300){
			SU_DEBUG_2 (("Answer status : %d, %s\n",
					status, phrase));
		} 		
		nua_respond (chan->data->op_handle, status, phrase,
				SOATAG_USER_SDP_STR(l_sdp_str),
				SOATAG_RTP_SORT(SOA_RTP_SORT_REMOTE),
				SOATAG_RTP_SELECT(SOA_RTP_SELECT_ALL), 
				TAG_END());
	} 
DFE
	return call_answered;
};

/**
 * Sends a BYE request to an operation on the chan.
 */
void 
svd_bye (svd_t * const svd, ab_chan_t * const chan)
{
DFS
	assert( chan );
	assert( chan->data );

	if( chan->data->op_handle ){
		nua_bye(chan->data->op_handle, TAG_END());
	} else {
		/* just clear call params */
		svd_clear_call (svd, chan);
	}
DFE
};


/**
 * Cancels a call operation currently in progress (if any).
 */
void 
svd_cancel (ab_chan_t * const chan)
{
DFS
	assert( chan );
	assert( chan->data );
	assert( chan->data->op_handle );

	nua_cancel(chan->data->op_handle, TAG_END());
DFE
};

#if 0 /* tag__ not realized yet UAC functions */

void svd_register(svd_t * svd, const char *registrar)
{
  svd_oper_t *op;
DFS
  if (!registrar && (op = svd_oper_find_by_method(svd, sip_method_register))) {
DMARKS("!registrar") // tag_ 
    p_rintf("REGISTER %s\n", op->op_ident);
    nua_register(op->op_handle, TAG_NULL());
DFE
    return;
  }

  if ((op =
       svd_oper_create(svd, SIP_METHOD_REGISTER, svd->conf->svd_aor,
		       TAG_END()))) {
DMARKS("nua_register") // tag_
    p_rintf("REGISTER %s\n", op->op_ident);
    nua_register(op->op_handle,
		 TAG_IF(registrar, NUTAG_REGISTRAR(registrar)), TAG_NULL());
DMARKS("nua_register end") // tag_
  }
DFE
}

void svd_unregister(svd_t * svd, const char *registrar)
{
  svd_oper_t *op;
DFS

  if (!registrar && (op = svd_oper_find_by_method(svd, sip_method_register))) {
    p_rintf("un-REGISTER %s\n", op->op_ident);
    nua_unregister(op->op_handle, TAG_NULL());
DFE
    return;
  } else {
    op =
      svd_oper_create(svd, SIP_METHOD_REGISTER, svd->conf->svd_aor, TAG_END());

    if (op) {
      p_rintf("un-REGISTER %s%s%s\n",
	     op->op_ident,
	     registrar ? " at " : "", registrar ? registrar : "");
      nua_unregister(op->op_handle,
		     TAG_IF(registrar, NUTAG_REGISTRAR(registrar)),
		     SIPTAG_CONTACT_STR("*"),
		     SIPTAG_EXPIRES_STR("0"), TAG_NULL());
DFE
      return;
    }
  }
DFE
}
#endif

/******************************************************************************/
static int 
svd_pure_invite( svd_t * const svd, ab_chan_t * const chan, 
		char const * const from_str, char const * const to_str )
{
	sip_to_t *to = NULL;
	sip_to_t *from = NULL;  
	char l_sdp_str[1000];
	nua_handle_t * nh;
	int err;

DFS
	to = sip_to_make(svd->home, to_str);
	if ( !to ) {
		SU_DEBUG_0 (("%s sip_to_make(): invalid address: %s\n",
				__func__, to_str));
		goto __exit_fail;
	}

	from  = sip_from_make(svd->home, from_str);
	if ( !from ) {
		SU_DEBUG_0 (("%s sip_form_make(): invalid address: %s\n", 
				__func__, from_str));
		goto __exit_fail;
	}

	/* Try to make sense out of the URL */
	if (url_sanitize(to->a_url) < 0) {
		SU_DEBUG_0 ((LOG_FNC_A("url_sanitize()")));
		goto __exit_fail;
	}

	nh = nua_handle (svd->nua, chan,
			NUTAG_URL(to->a_url),
			SIPTAG_TO(to), 
			SIPTAG_FROM(from), 
			TAG_NULL());

	su_free(svd->home, to);
	su_free(svd->home, from);

	chan->data->op_handle = nh;
	if( !chan->data->op_handle ){
		SU_DEBUG_0 ((LOG_FNC_A("can`t create handle")));
		goto __exit_fail;
	}

	/* register vinetic media */
	err = svd_media_register (svd, chan);
	if (err){
		SU_DEBUG_0 ((LOG_FNC_A("can`t register media")));
		goto __exit_fail;
	}

 	/* tag__ static sdp string now */
	snprintf(l_sdp_str, 1000, 
			"v=0\r\n"
			"m=audio %d RTP/AVP 18 8\r\n"
			"a=rtpmap:18 G729/8000\r\n"
			"a=rtpmap:8 PCMA/8000\r\n",
			chan->data->rtp_port);
#ifdef SVD_DEBUG_LOGS
	SU_DEBUG_3 (("SDP STRING : %s\n", l_sdp_str));
#endif

	nua_invite( nh,
			SOATAG_USER_SDP_STR(l_sdp_str),
			SOATAG_RTP_SORT (SOA_RTP_SORT_REMOTE),
			SOATAG_RTP_SELECT (SOA_RTP_SELECT_SINGLE), 
			TAG_END() );
DFE
	return 0;

__exit_fail:
	if (to){
		su_free(svd->home, to);
	}
	if (from){
		su_free(svd->home, from);
	}
DFE
	return -1;
};


/************************************************************************ UAS */

/******************************************************************************/

/**
 * Prints verbose error information to stdout.
 */
static void
svd_i_error(int const status, char const * const phrase)
{
DFS
	SU_DEBUG_2(("NUA STACK ERROR : %03d %s\n", status, phrase));
DFE
};

/**
 * Incoming INVITE request.
 */
static void
svd_i_invite( int status, char const * phrase, svd_t * const svd, 
		nua_handle_t * nh, ab_chan_t * chan, 
		sip_t const *sip, tagi_t tags[])
{
	ab_chan_t * req_chan;
	sip_to_t const *to = sip->sip_to;
	unsigned char abs_chan_idx;
	int chan_idx;
	int err;
DFS

	/* * 
	 * Get requested chan number 
	 * it can be:
	 * '$FIRST_FREE_FXO@..' - first free fxo
	 * 'xx@..'  - absolute channel number
	 * */
	if (isdigit(to->a_url->url_user[0])){
		/* 'xx@..'  - absolute channel number */
		abs_chan_idx = strtol(to->a_url->url_user, NULL, 10);
		chan_idx = get_dest_chan_idx (svd->ab, NULL, abs_chan_idx);
		if( chan_idx == -1 ){
			goto __exit;
		}
	} else if ( !strcmp(to->a_url->url_user, FIRST_FREE_FXO )){
	 	/* '$FIRST_FREE_FXO@..' - first free fxo */
		chan_idx = get_FF_FXO_idx (svd->ab, -1);
		if( chan_idx == -1 ){
			/* all chans busy */
			nua_respond(nh, SIP_486_BUSY_HERE, TAG_END());
			nua_handle_destroy(nh);
			goto __exit;
		}
	} else {
	 	/* unknown user */
		SU_DEBUG_2(("Incoming call to unknown user \"%s\" "
				"on this host\n",
				to->a_url->url_user));
		goto __exit;
	}

/* tag__ race on chan */

	req_chan = &svd->ab->chans[chan_idx];

	if( req_chan->data->op_handle ){
		/* user is busy */
		nua_respond(nh, SIP_486_BUSY_HERE, TAG_END());
		nua_handle_destroy(nh);
		goto __exit;
	}

	req_chan->data->op_handle = nh;
	nua_handle_bind (nh, req_chan);

	if (req_chan->parent->type == ab_dev_type_FXS){
		/* start ringing */
		err = ab_FXS_line_ring( req_chan, ab_chan_ring_RINGING );
		if (err){
			SU_DEBUG_1(("can`t ring to on \"%s\"\n",
					to->a_url->url_user));
		}
	} else if (req_chan->parent->type == ab_dev_type_FXO){
		/* do offhook */
		err = ab_FXO_line_hook( req_chan, ab_chan_hook_OFFHOOK );
		if (err){
			SU_DEBUG_1(("can`t offhook on \"%s\"\n",
					to->a_url->url_user));
		}
	}

__exit:
DFE
};

/**
 * Incoming CANCEL.
 */
static void
svd_i_cancel (nua_handle_t const * const nh, ab_chan_t const * const chan)
{
DFS
	/* tag_? should destroy operation handle ?? */
	assert (chan);
	assert (chan->data->op_handle == nh);
	SU_DEBUG_3 (("CANCEL received\n"));
DFE
};

/**
 * Incoming ACK.
 */
/*
static void
svd_i_ack(nua_handle_t const * const nh, ab_chan_t const * const chan)
{
DFS
	assert (chan);
	assert (chan->data->op_handle == nh);
	SU_DEBUG_3 (("ACK received\n"));
	
	if(ab_chan_media_activate (chan)){
		SU_DEBUG_0(("media_activate error : %s\n",
				ab_err_get_str(chan)));
	}
DFE
}
*/
/**
 * Callback issued for any change in operation state.
 */
static void
svd_i_state(int status, char const *phrase, nua_t * nua, svd_t * svd,
		nua_handle_t * const nh, ab_chan_t * chan, sip_t const *sip,
		tagi_t tags[])
{
	char const * l_sdp = NULL;
	char const * r_sdp = NULL;
	int offer_recv = 0;
	int answer_recv = 0; 
	int offer_sent = 0; 
	int answer_sent = 0;
	int ss_state = nua_callstate_init;
	int err;

DFS
	tl_gets( tags,
			NUTAG_CALLSTATE_REF(ss_state),
			NUTAG_OFFER_RECV_REF(offer_recv),
			NUTAG_ANSWER_RECV_REF(answer_recv),
			NUTAG_OFFER_SENT_REF(offer_sent),
			NUTAG_ANSWER_SENT_REF(answer_sent),
			SOATAG_LOCAL_SDP_STR_REF(l_sdp),
			SOATAG_REMOTE_SDP_STR_REF(r_sdp), 
			TAG_END() );

	if( (!chan) && (nh)){
		chan = nua_handle_magic(nh);
	}

	SU_DEBUG_4(("CALLSTATE NAME : %s\n", nua_callstate_name(ss_state)));

	if (r_sdp) {
		SU_DEBUG_4(("Remote sdp:\n%s\n", r_sdp));
		svd_media_parse_sdp(svd, chan, r_sdp);
	}
	if (l_sdp) {
		SU_DEBUG_4(("Local sdp:\n%s\n", l_sdp));
	}

	switch (ss_state) {

	/* Initial state */
		case nua_callstate_init:
			break;

	/* 401/407 received */
		case nua_callstate_authenticating: 
			break;

	/* INVITE sent */
		case nua_callstate_calling:
			break;

	/* 18X received */
		case nua_callstate_proceeding:
			break;

	/* 2XX received */
		case nua_callstate_completing:
			/* In auto-ack, we get nua_callstate_ready */
			nua_ack(nh, TAG_END());
			break;

	/* INVITE received */
		case nua_callstate_received:
			nua_respond(nh, SIP_180_RINGING, TAG_END());
			break;

	/* 18X sent (w/SDP) */
		case nua_callstate_early:

			if(chan->parent->type == ab_dev_type_FXO){
				/* answer on call */
				svd_answer (svd, chan, SIP_200_OK);
			} /* if FXS - answer after offhook */
			break;

	/* 2XX sent */
		case nua_callstate_completed:
			break;

	/* 2XX received, ACK sent, or vice versa */
		case nua_callstate_ready:
			if(ab_chan_media_activate (chan)){
				SU_DEBUG_0(("media_activate error : %s\n",
						ab_err_get_str(chan)));
			}
			break;

	/* BYE sent */
		case nua_callstate_terminating:
			/* marker we_say_bye */
			break;

	/* BYE complete */
		case nua_callstate_terminated:
			SU_DEBUG_4 (("call on [%d] is terminated\n", 
					chan->abs_idx));

			/* deactivate media */
			ab_chan_media_deactivate (chan);

			/* media unregister */
			svd_media_unregister (svd, chan);

			/* clear call params */
			svd_clear_call (svd, chan);

			if (chan->parent->type == ab_dev_type_FXO){
				err = ab_FXO_line_hook (chan, 
						ab_chan_hook_ONHOOK);
				if(err){
					SU_DEBUG_2(("Can`t onhook on "
							"channel %d\n",
							chan->abs_idx));
				}
				SU_DEBUG_2(("onhook on "
						"channel %d\n",
						chan->abs_idx));
			}
			/* если не мы закончили разговор "FXS-FXS" */
			/* ioctl(ssc->fd, IFX_TAPI_TONE_BUSY_PLAY, 0);*/
			break;
	}
DFE
};

/**
 * Incoming BYE request. Note, call state related actions are
 * done in the svd_i_state() callback.
 */
static void
svd_i_bye(nua_handle_t const * const nh, ab_chan_t const * const chan)
{
DFS
	assert(chan);
	assert(chan->data);
	assert(chan->data->op_handle == nh);
	SU_DEBUG_3 (("BYE received\n"));

DFE
};

static void 
svd_i_prack (nua_handle_t * nh, ab_chan_t const * const chan, 
		sip_t const * const sip)
{
	sip_rack_t const * rack;
DFS
	rack = sip->sip_rack;
	SU_DEBUG_3 (("received PRACK %u\n", rack ? rack->ra_response : 0));
	if (chan == NULL)
	nua_handle_destroy(nh);
DFE
};

static void 
svd_i_info(int status, char const * phrase, svd_t * const svd, 
		nua_handle_t * nh, ab_chan_t * chan, sip_t const * sip)
{
DFS
	int err;
	char digit [2];

	digit[0] = sip->sip_payload->pl_data[INFO_DIGIT_MSG_DIGIT_POS];
	digit[1] = '\0';

	/* ab_chan_media_switch(chan, 0, 0); */
	err = ab_FXO_line_digit (chan, 1, digit, 0, 0);
	if (err){
		SU_DEBUG_1(("Cold`not dial digit '%s' "
				"on channel [%d] : %s\n",
				digit, chan->abs_idx, ab_err_get_str(chan) ));
	} 	
	/* sleep(1);
	ab_chan_media_switch(chan, 1, 1); */
DFE
};

/**
 * Result callback for nua_r_get_params request.
 */
void
svd_r_get_params(int status, char const *phrase, nua_t * nua, svd_t * svd,
		 nua_handle_t * nh, ab_chan_t * chan, sip_t const *sip,
		 tagi_t tags[])
{
	char buff [256];
DFS
	while(tags){
		t_snprintf(tags, buff, 256);
		SU_DEBUG_3 (("%s\n",buff));
		tags = tl_next(tags);
	};
DFE
};

static void 
svd_r_shutdown( int status, char const *phrase, nua_t * nua, svd_t * svd, 
		nua_handle_t * nh, ab_chan_t * chan, sip_t const *sip, 
		tagi_t tags[] )
{
DFS
	/*
	 * 100 - shutdown started
	 * 101 - shutdown in progress
	 * 200 - shutdown successful
	 * 500 - shutdown timeout after 30 seconds
	 */
	if(status == 200){
		nua_destroy(svd->nua);
		svd->nua = NULL;
	}
DFE
};

#if 0
void
ssc_r_register(int status, char const *phrase,
	       nua_t * nua, ssc_t * ssc,
	       nua_handle_t * nh, ssc_oper_t * op, sip_t const *sip,
	       tagi_t tags[])
{
	sip_contact_t *m = sip ? sip->sip_contact : NULL;
DFS
	p_rintf("REGISTER: %03d %s\n", status, phrase);
	if (status < 200){
DFE
		return;
	}

	if (status == 401 || status == 407)
		ssc_authenticate(ssc, op, sip, tags);
	else if (status >= 300)
		ssc_oper_destroy(ssc, op);
	else if (status == 200)
		for (m = sip ? sip->sip_contact : NULL; m; m = m->m_next)
			sl_header_print(stdout, "\tContact: %s\n", 
					(sip_header_t *) m);
DFE
}

void
ssc_r_unregister(int status, char const *phrase,
		 nua_t * nua, ssc_t * ssc,
		 nua_handle_t * nh, ssc_oper_t * op, sip_t const *sip,
		 tagi_t tags[])
{
	sip_contact_t *m;
DFS
	p_rintf("un-REGISTER: %03d %s\n", status, phrase);
	if (status < 200){
DFE
		return;
	}

	if (status == 200)
		for (m = sip ? sip->sip_contact : NULL; m; m = m->m_next)
			sl_header_print(stdout, "\tContact: %s\n", 
					(sip_header_t *) m);

	if (status == 401 || status == 407)
		ssc_authenticate(ssc, op, sip, tags);
	else
		ssc_oper_destroy(ssc, op);
DFE
}
#endif

/**
 * Callback for an outgoing INVITE request.
 */
static void 
svd_r_invite( int status, char const *phrase, nua_t * nua, svd_t * svd,
		nua_handle_t * nh, ab_chan_t * chan, sip_t const *sip,
		tagi_t tags[])
{
DFS
	SU_DEBUG_3(("got answer on INVITE: %03d %s\n", status, phrase));

	if (status >= 300) {
		/* op->op_callstate &= ~opc_sent; */
		if (status == 401 || status == 407) {
			SU_DEBUG_2(("should AUTHENTICATE - "
					"not implemented yet\n"));
			/* ssc_authenticate(ssc, op, sip, tags); */
		}
	}
DFE
};

/**
 * Callback for an outgoing BYE request.
 */
static void
svd_r_bye(int status, char const *phrase,
	  nua_handle_t const * const nh, ab_chan_t const * const chan)
{
DFS
	assert(chan);
	assert(chan->data->op_handle == nh);
	SU_DEBUG_3(("got answer on BYE: %03d %s\n", status, phrase));
DFE
};

svd_r_info(int status, char const * phrase, svd_t * const svd, 
		nua_handle_t * nh, ab_chan_t * chan, sip_t const * sip)
{
DFS
	SU_DEBUG_3(("got answer on INFO: %d, %s\n",status,phrase));
DFE
};

void 
svd_media_parse_sdp(svd_t * const svd, ab_chan_t * const chan, 
		char const *str)
{
	su_home_t *home = svd->home;
	sdp_parser_t *remote_sdp;
	sdp_session_t *sdp_sess;
	sdp_connection_t *sdp_connection;
	const char *pa_error;

	remote_sdp = sdp_parse (home, str, strlen(str), sdp_f_insane);
	pa_error = sdp_parsing_error (remote_sdp);
	if (pa_error) {
		SU_DEBUG_0(("%s: error parsing SDP: %s\n", __func__, pa_error));
		return;
	}

	sdp_sess = sdp_session (remote_sdp);
	sdp_connection = sdp_media_connections (sdp_sess->sdp_media);
	if (sdp_sess && sdp_sess->sdp_media->m_port && sdp_connection
			&& sdp_connection->c_address) {
		chan->data->remote_port = sdp_sess->sdp_media->m_port;
		chan->data->remote_host = su_strdup(
				home, sdp_connection->c_address);
	}

	chan->data->payload = sdp_sess->sdp_media->m_rtpmaps->rm_pt;

#ifdef SVD_DEBUG_LOGS
	SU_DEBUG_3(("I've got remote %s:%d with payload %d\n",
		chan->data->remote_host, chan->data->remote_port, 
		chan->data->payload));
#endif
	sdp_parser_free(remote_sdp);
	return;
};

