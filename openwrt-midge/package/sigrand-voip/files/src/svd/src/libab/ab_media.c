#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <fcntl.h>

#include "ab_internal_v22.h"

int ab_chan_media_rtp_tune( ab_chan_t * const chan, 
		rtp_session_prms_t const * const rtpp)
{
	IFX_TAPI_PKT_RTP_CFG_t rtpConf;
	int err;

	memset(&rtpConf, 0, sizeof(IFX_TAPI_PKT_RTP_CFG_t));

	rtpConf.nSeqNr = 0;
	rtpConf.nSsrc = 0;

	/* set out-of-band (RFC 2833 packet) transmission type */
	switch(rtpp->nEvents){
		case evts_OOB_DEFAULT:
			rtpConf.nEvents = IFX_TAPI_PKT_EV_OOB_DEFAULT;
			break;
		case evts_OOB_NO:
			rtpConf.nEvents = IFX_TAPI_PKT_EV_OOB_NO;
			break;
		case evts_OOB_ONLY:
			rtpConf.nEvents = IFX_TAPI_PKT_EV_OOB_ONLY;
			break;
		case evts_OOB_ALL:
			rtpConf.nEvents = IFX_TAPI_PKT_EV_OOB_ALL;
			break;
		case evts_OOB_BLOCK:
			rtpConf.nEvents = IFX_TAPI_PKT_EV_OOB_BLOCK;
			break;
	}

	/* Configure payload type for RFC 2833 packets */
	rtpConf.nEventPT = 0x62;
	rtpConf.nEventPlayPT = 0x62;

	/* What to do upon RFC 2833 packets reception */
	switch(rtpp->nPlayEvents){
		case play_evts_DEFAULT:
			rtpConf.nPlayEvents = IFX_TAPI_PKT_EV_OOBPLAY_DEFAULT;
			break;
		case play_evts_PLAY:
			rtpConf.nPlayEvents = IFX_TAPI_PKT_EV_OOBPLAY_PLAY;
			break;
		case play_evts_MUTE:
			rtpConf.nPlayEvents = IFX_TAPI_PKT_EV_OOBPLAY_MUTE;
			break;
	}

	err = 0;
	err = ioctl(chan->rtp_fd, IFX_TAPI_PKT_RTP_CFG_SET, &rtpConf);
	if(err){
		ab_err_set(chan, AB_ERR_UNKNOWN, 
				"media rtp tune ioctl error");
	}

	return err;
};

int ab_chan_media_cod_tune( ab_chan_t * const chan, 
		cod_prms_t const * const codp)
{
	IFX_TAPI_ENC_CFG_t encCfg;
	IFX_TAPI_PKT_VOLUME_t codVolume;
	int err;
	int err_summary = 0;

	/* In order to support RFC 4040, 
	 * the following configuration must be done
	 * Use G.711, disable high-pass filters, 
	 * VAD and set gains to 0 dB */

	memset (&encCfg, 0, sizeof(encCfg));
	memset(&codVolume, 0, sizeof(IFX_TAPI_PKT_VOLUME_t));

	/* Configure encoder and decoder gains : 0 dB */
	codVolume.nEnc = 0;
	codVolume.nDec = 0;

	/* Set the encoder type */
	switch(codp->cod_type){
		case cod_type_G729:
			encCfg.nEncType = IFX_TAPI_COD_TYPE_G729;
			break;
		case cod_type_MLAW:
			encCfg.nEncType = IFX_TAPI_COD_TYPE_MLAW;
			break;
		case cod_type_ALAW:
			encCfg.nEncType = IFX_TAPI_COD_TYPE_ALAW;
			break;
		case cod_type_MLAW_VBD:
			encCfg.nEncType = IFX_TAPI_COD_TYPE_MLAW_VBD;
			break;
		case cod_type_ALAW_VBD:
			encCfg.nEncType = IFX_TAPI_COD_TYPE_ALAW_VBD;
			break;
	}

	/* Set the length (20 ms)*/
	encCfg.nFrameLen = IFX_TAPI_COD_LENGTH_10;

	/* Set the coder */ 
	err = 0;
	err = ioctl(chan->rtp_fd, IFX_TAPI_ENC_CFG_SET, &encCfg);
	if(err){
		ab_err_set(chan, AB_ERR_UNKNOWN, 
				"coder set ioctl error");
		err_summary++;
	}
	/* Set the VAD off */
	err = 0;
	err = ioctl(chan->rtp_fd, IFX_TAPI_ENC_VAD_CFG_SET, 
			IFX_TAPI_ENC_VAD_NOVAD);
	if(err){
		ab_err_set(chan, AB_ERR_UNKNOWN, 
				"vad set ioctl error");
		err_summary++;
	}
	/* Configure encoder and decoder gains : 0 dB */
	err = 0;
	err = ioctl(chan->rtp_fd, IFX_TAPI_COD_VOLUME_SET, &codVolume);
	if(err){
		ab_err_set(chan, AB_ERR_UNKNOWN, 
				"volume set ioctl error");
		err_summary++;
	}
	/* Disable high-pass filter */
	err = 0;
	err = ioctl(chan->rtp_fd, IFX_TAPI_COD_DEC_HP_SET, IFX_FALSE);
	if(err){
		ab_err_set(chan, AB_ERR_UNKNOWN, 
				"high pass set ioctl error");
		err_summary++;
	}

	if(err_summary){
		return -1;
	}
	return 0;
};

int ab_chan_media_switch( ab_chan_t * const chan,
		unsigned char const enc_on, unsigned char const dec_on )
{
	int err;
	int err_summary = 0;

	err = 0;
	if (enc_on){
		err = ioctl(chan->rtp_fd, IFX_TAPI_ENC_START, 0);
	} else {
		err = ioctl(chan->rtp_fd, IFX_TAPI_ENC_STOP, 0);
	}
	if(err){
		ab_err_set(chan, AB_ERR_UNKNOWN, 
				"encoder start/stop ioctl error");
		err_summary++;
	}

	err = 0;
	if (dec_on){
		err = ioctl(chan->rtp_fd, IFX_TAPI_DEC_START, 0);
	} else {
		err = ioctl(chan->rtp_fd, IFX_TAPI_DEC_STOP, 0);
	}
	if(err){
		ab_err_set(chan, AB_ERR_UNKNOWN, 
				"decoder start/stop ioctl error");
		err_summary++;
	}

	if(err_summary){
		return -1;
	}
	return 0;
};

int 
ab_chan_media_activate ( ab_chan_t * const chan )
{
	rtp_session_prms_t rtpp;
	cod_prms_t codp;
	int err;
	int err_summary = 0;

	memset (&rtpp, 0, sizeof(rtpp));
	memset (&codp, 0, sizeof(codp));

	/* in-band *
	rtpp.nEvents = IFX_TAPI_PKT_EV_OOB_NO;
	rtpp.nPlayEvents = IFX_TAPI_PKT_EV_OOBPLAY_MUTE;
	codp.cod_type = cod_type_ALAW;
	**/

	/* out-of-band */
	rtpp.nEvents = IFX_TAPI_PKT_EV_OOB_ONLY;
	rtpp.nPlayEvents = IFX_TAPI_PKT_EV_OOBPLAY_PLAY;
	codp.cod_type = cod_type_G729;
	/**/

 	/* via info *
	rtpp.nEvents = IFX_TAPI_PKT_EV_OOB_BLOCK;
	rtpp.nPlayEvents = IFX_TAPI_PKT_EV_OOBPLAY_MUTE;
	codp.cod_type = cod_type_G729;
	**/

	err = ab_chan_media_rtp_tune (chan, &rtpp);
	if(err){
		err_summary++;
	}
	err = ab_chan_media_cod_tune (chan, &codp);
	if(err){
		err_summary++;
	}
	err = ab_chan_media_switch (chan, 1, 1);
	if(err){
		err_summary++;
	}
	
	if(err_summary){
		err = -1;
	}

	return err;
};

int 
ab_chan_media_deactivate ( ab_chan_t * const chan )
{
	int err;
	err = ab_chan_media_switch (chan, 0, 0);
	return err;
};

