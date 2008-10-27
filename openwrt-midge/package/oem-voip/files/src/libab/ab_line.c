#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <fcntl.h>

#include "ab_internal_v22.h"

#define AB_INTER_DIGIT_TIME_DF 100
#define AB_DIGIT_PLAY_TIME_DF  100

#define CHAN_VOLUME_MIN_GAIN (-24)
#define CHAN_VOLUME_MAX_GAIN 24

/**
	Run the appropriate ioctl command and set the error if necessary. 
\param
	chan - channel to write error message and run ioctl
	request - ioctl macro
	data - ioctl data
	err_msg - error message if ioctl fails
\return 
	ioctl result
\remark
	ioctl makes on chan->rtp_fd file descriptor
*/
static int 
err_set_ioctl (ab_chan_t * const chan, int const request, int const data,
		char const * const err_msg )
{/*{{{*/
	int err = 0;
	err = ioctl(chan->rtp_fd, request, data);
	if (err){
		ab_err_set(AB_ERR_UNKNOWN, (char const *)err_msg); 
	}
	return err;
}/*}}}*/

/**
	Plays the given tone 
\param
	chan - channel to play tone
	tone - tone to play 
\return 
	ioctl result
\remark
	it just play the tone without any test of actual state.
*/
static int 
ab_FXS_line_just_play_it (ab_chan_t * const chan, enum ab_chan_tone_e tone)
{/*{{{*/
	int err = 0;
	switch(tone){
		case ab_chan_tone_MUTE: {
			err = err_set_ioctl( chan, 
					IFX_TAPI_TONE_LOCAL_PLAY, 0,
					"stop playing tone (ioctl)");
			if( !err ){
				chan->status.tone = ab_chan_tone_MUTE;
			}
			break;
		}
		case ab_chan_tone_DIAL: {
			err = err_set_ioctl( chan, 
					IFX_TAPI_TONE_DIALTONE_PLAY, 0,
					"playing dialtone (ioctl)");
			if( !err ){
				chan->status.tone = ab_chan_tone_DIAL;
			}
			break;
		}
		case ab_chan_tone_BUSY: {
			err = err_set_ioctl( chan, 
					IFX_TAPI_TONE_BUSY_PLAY, 0,
					"playing busy (ioctl)");
			if( !err ){
				chan->status.tone = ab_chan_tone_BUSY;
			}
			break;
		}
		case ab_chan_tone_RINGBACK: {
			err = err_set_ioctl( chan, 
					IFX_TAPI_TONE_RINGBACK_PLAY, 0,
					"playing ringback (ioctl)");
			if( !err ){
				chan->status.tone = 
						ab_chan_tone_RINGBACK;
			}
			break;
		}
	}
	return err;
}/*}}}*/

/**
	Ring or mute on given channel
\param
	chan - channel to ring
	ring - can be RINGING or MUTE
\return 
	ioctl result
\remark
	If the given ring state is actual - there is nothing happens
*/
int 
ab_FXS_line_ring (ab_chan_t * const chan, enum ab_chan_ring_e ring)
{/*{{{*/
	int err = 0;
	if (chan->status.ring != ring){
		if( ring == ab_chan_ring_RINGING ) {
			err = err_set_ioctl(
				chan, IFX_TAPI_RING_START, 0,
						"start ringing (ioctl)");
			if (!err){
				chan->status.ring = ab_chan_ring_RINGING;
			}
		} else {
			err = err_set_ioctl(
				chan, IFX_TAPI_RING_STOP, 0,
						"stop ringing (ioctl)");
			if (!err){
				chan->status.ring = ab_chan_ring_MUTE;
			}
		}
	}
	return err;
}/*}}}*/

/**
	Play the given tone
\param
	chan - channel to play tone
	tone - tone to play 
\return 
	ioctl result
\remark
	it test the state and do not do the unnecessary actions
*/
int 
ab_FXS_line_tone (ab_chan_t * const chan, enum ab_chan_tone_e tone)
{/*{{{*/
	int err = 0;
	if (chan->status.tone != tone){
		/* Status is not actual - should do smth */
		if(chan->status.tone == ab_chan_tone_MUTE){
			/* we should play smth */
			err = ab_FXS_line_just_play_it (chan, tone);
			if (err){
				goto __exit;
			} 
		} else {
			/* Something playing, but not that what we need */
			err = ab_FXS_line_just_play_it( chan, 
					ab_chan_tone_MUTE );
			if (err){
				goto __exit;
			} else if (tone != ab_chan_tone_MUTE){
				/* we are don`t need MUTE */
				err = ab_FXS_line_just_play_it (chan, tone);
				if (err){
					goto __exit;
				} 
			}
		}
	}
__exit:
	return err;
}/*}}}*/

/**
	Change current linefeed to given
\param
	chan - channel to operate on it
	feed - linefeed to set
\return 
	ioctl result
\remark
	it test the state and do not do the unnecessary actions
	if linefeed is disabled, and we want to set it to active, it will set
			it to standby first
*/
int 
ab_FXS_line_feed (ab_chan_t * const chan, enum ab_chan_linefeed_e feed) 
{/*{{{*/
	int err = 0;

	if (chan->status.linefeed != feed){
		char const * err_msg;
		int lf_to_set;
		switch (feed){
			case ab_chan_linefeed_DISABLED: {
				/* From any state */
				err_msg= "Setting linefeed to disabled (ioctl)";
				lf_to_set = IFX_TAPI_LINE_FEED_DISABLED;
				break;	
			}
			case ab_chan_linefeed_STANDBY: {
				/* From any state */
				err_msg = "Setting linefeed to standby (ioctl)";
				lf_to_set = IFX_TAPI_LINE_FEED_STANDBY;
				break;	
			}
			case ab_chan_linefeed_ACTIVE: {
				/* linefeed_STANDBY should be 
				 * set before ACTIVE */
				if( chan->status.linefeed == 
						ab_chan_linefeed_DISABLED){
					err = err_set_ioctl( chan, 
						IFX_TAPI_LINE_FEED_SET, 
						IFX_TAPI_LINE_FEED_STANDBY, 
						"Setting linefeed to "
						"standby before set "
						"it to active "
						"(ioctl)"); 
					if( err ){
						goto __exit;
					} else {
						chan->status.linefeed = 
						ab_chan_linefeed_STANDBY;
					}
				}
				err_msg = "Setting linefeed to active (ioctl)";
				lf_to_set = IFX_TAPI_LINE_FEED_ACTIVE;
				break;	
			}
		}
		err = err_set_ioctl (chan, 
				IFX_TAPI_LINE_FEED_SET, lf_to_set, err_msg);
		if ( !err){
			chan->status.linefeed = feed;
		} 
	}
__exit:
	return err;
}/*}}}*/

/**
	Do onhook or offhook
\param
	chan - channel to operate on it
	hook - desired hookstate
\return 
	ioctl result
\remark
	it test the state and do not do the unnecessary actions
\todo
	we can also test hook by ioctl there
*/
int 
ab_FXO_line_hook (ab_chan_t * const chan, enum ab_chan_hook_e hook)
{/*{{{*/
	int err = 0;

	if (chan->status.hook != hook){
		char const * err_msg;
		int h_to_set;
		switch (hook) {
			case ab_chan_hook_OFFHOOK : {
				h_to_set = IFX_TAPI_FXO_HOOK_OFFHOOK;
				err_msg = "Try to offhook (ioctl)";
				break;
			}
			case ab_chan_hook_ONHOOK: {
				h_to_set = IFX_TAPI_FXO_HOOK_ONHOOK;
				err_msg = "Try to onhook (ioctl)";
				break;
			}
		}
		err = err_set_ioctl (chan, 
				IFX_TAPI_FXO_HOOK_SET, h_to_set, err_msg);
		if( !err){
			chan->status.hook = hook;
		} 
	}
	return err;
}/*}}}*/

/**
	Dial the given sequence of numbers
\param
	chan - channel dial numbers in it
	data_length - sequence length
	data - sequence of numbers
	nInterDigitTime - interval between dialed digits
	nDigitPlayTime - interval to play digits
\return 
	ioctl result
\remark
	the digits can be: '0' - '9', '*','#','A','B','C' and 'D'
	if nInterDigitTime or nDigitPlayTime set to 0, it will be set to 
			default value (100 ms). Maximum value is 127 ms.
*/
int 
ab_FXO_line_digit (ab_chan_t * const chan, char const data_length, 
		char const * const data, char const nInterDigitTime,
		char const nDigitPlayTime)
{/*{{{*/
	IFX_TAPI_FXO_DIAL_CFG_t dialCfg;
	IFX_TAPI_FXO_DIAL_t 	dialDigits;
	int err = 0;

	memset(&dialCfg, 0, sizeof(dialCfg));
	memset(&dialDigits, 0, sizeof(dialDigits));

	/* Configure dial timing */
	dialCfg.nInterDigitTime = AB_INTER_DIGIT_TIME_DF;
	dialCfg.nDigitPlayTime = AB_DIGIT_PLAY_TIME_DF;
	if ( nInterDigitTime ) {
		dialCfg.nInterDigitTime = nInterDigitTime;
	}
	if( nInterDigitTime ) {
		dialCfg.nDigitPlayTime = nDigitPlayTime;
	}

	err = err_set_ioctl( chan, 
			IFX_TAPI_FXO_DIAL_CFG_SET, (int)&dialCfg, 
			"Try to configure dial params (ioctl)");
	if( err ){
		goto __exit;
	} 

	/* Dial sequence 
	 * it needs for some time but returns immediately
	 * */
	dialDigits.nDigits = data_length;
	memcpy(dialDigits.data, data, dialDigits.nDigits);

	err = err_set_ioctl( chan, 
			IFX_TAPI_FXO_DIAL_START, (int)(&dialDigits), 
			"Try to dial sequence (ioctl)");
	if( err ){
		goto __exit;
	} 
__exit:
	return err;
}/*}}}*/

