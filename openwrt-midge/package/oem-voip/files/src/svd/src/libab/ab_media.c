#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <fcntl.h>

#include "ab_internal_v22.h"

enum cod_pt_e {
	cod_pt_MLAW = 0,
	cod_pt_ALAW = 8,
	cod_pt_G729 = 18,
	cod_pt_G726_16 = 94,
	cod_pt_G726_24 = 95,
	cod_pt_G726_32 = 96,
	cod_pt_G726_40 = 97,
	cod_pt_ILBC_133 = 98,
};

typedef struct rtp_session_prms_s {
	enum evts_e {
		evts_OOB_DEFAULT,
		evts_OOB_NO,
		evts_OOB_ONLY,
		evts_OOB_ALL,
		evts_OOB_BLOCK
	} nEvents;
	enum play_evts_e {
		play_evts_DEFAULT,
		play_evts_PLAY,
		play_evts_MUTE
	} nPlayEvents;
} rtp_session_prms_t;

static int 
ab_chan_media_rtp_tune( ab_chan_t * const chan, 
		rtp_session_prms_t const * const rtpp);
static int 
ab_chan_media_switch( ab_chan_t * const chan,
		unsigned char const enc_on, unsigned char const dec_on );


static int 
ab_chan_media_rtp_tune( ab_chan_t * const chan, 
		rtp_session_prms_t const * const rtpp)
{
	IFX_TAPI_PKT_RTP_PT_CFG_t rtpPTConf;
	IFX_TAPI_PKT_RTP_CFG_t rtpConf;
	int err;

	memset(&rtpPTConf, 0, sizeof(rtpPTConf));

	/* Payload types table */
	rtpPTConf.nPTup [IFX_TAPI_COD_TYPE_MLAW] = 
			rtpPTConf.nPTdown [IFX_TAPI_COD_TYPE_MLAW] = 
			cod_pt_MLAW;
	rtpPTConf.nPTup [IFX_TAPI_COD_TYPE_ALAW] = 
			rtpPTConf.nPTdown [IFX_TAPI_COD_TYPE_ALAW] = 
			cod_pt_ALAW;
	rtpPTConf.nPTup [IFX_TAPI_COD_TYPE_G729] = 
			rtpPTConf.nPTdown [IFX_TAPI_COD_TYPE_G729] = 
			cod_pt_G729;
	rtpPTConf.nPTup [IFX_TAPI_COD_TYPE_G726_16] = 
			rtpPTConf.nPTdown [IFX_TAPI_COD_TYPE_G726_16] = 
			cod_pt_G726_16;
	rtpPTConf.nPTup [IFX_TAPI_COD_TYPE_G726_24] = 
			rtpPTConf.nPTdown [IFX_TAPI_COD_TYPE_G726_24] = 
			cod_pt_G726_24;
	rtpPTConf.nPTup [IFX_TAPI_COD_TYPE_G726_32] = 
			rtpPTConf.nPTdown [IFX_TAPI_COD_TYPE_G726_32] = 
			cod_pt_G726_32;
	rtpPTConf.nPTup [IFX_TAPI_COD_TYPE_G726_40] = 
			rtpPTConf.nPTdown [IFX_TAPI_COD_TYPE_G726_40] = 
			cod_pt_G726_40;
	rtpPTConf.nPTup [IFX_TAPI_COD_TYPE_ILBC_133] =
			rtpPTConf.nPTdown [IFX_TAPI_COD_TYPE_ILBC_133] =
			cod_pt_ILBC_133;

	err = 0;
	err = ioctl(chan->rtp_fd, IFX_TAPI_PKT_RTP_PT_CFG_SET, &rtpPTConf);
	if(err){
		ab_err_set(chan, AB_ERR_UNKNOWN, 
				"media rtp PT tune ioctl error");
	}

	memset(&rtpConf, 0, sizeof(rtpConf));

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
}

static int 
ab_chan_media_switch( ab_chan_t * const chan,
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
}

int 
ab_chan_media_activate ( ab_chan_t * const chan )
{
	IFX_TAPI_ENC_CFG_t encCfg;
	IFX_TAPI_PKT_VOLUME_t codVolume;
	rtp_session_prms_t rtpp;
	int err;
	int err_summary = 0;

	/* In order to support RFC 4040, 
	 * the following configuration must be done
	 * Use G.711, disable high-pass filters, 
	 * VAD and set gains to 0 dB */

	memset (&rtpp, 0, sizeof(rtpp));
	memset (&encCfg, 0, sizeof(encCfg));
	memset (&codVolume, 0, sizeof(codVolume));

	/* out-of-band */
	rtpp.nEvents = evts_OOB_ONLY;
	rtpp.nPlayEvents = play_evts_PLAY;
	err = ab_chan_media_rtp_tune (chan, &rtpp);
	if(err){
		err_summary++;
	}

	encCfg.nFrameLen = IFX_TAPI_COD_LENGTH_20;

	/* tag__ using context - this is bad */
	switch(chan->ctx->payload){
		case cod_pt_MLAW:
			//fprintf(stderr,">>>>>>>>>>>>>MLAW\n");
			encCfg.nEncType = IFX_TAPI_COD_TYPE_MLAW;
			break;
		case cod_pt_ALAW:
			//fprintf(stderr,">>>>>>>>>>>>>ALAW\n");
			encCfg.nEncType = IFX_TAPI_COD_TYPE_ALAW;
			break;
		case cod_pt_G729:
			//fprintf(stderr,">>>>>>>>>>>>>G729\n");
			encCfg.nEncType = IFX_TAPI_COD_TYPE_G729;
			encCfg.nFrameLen = IFX_TAPI_COD_LENGTH_10;
			break;
		case cod_pt_G726_16:
			//fprintf(stderr,">>>>>>>>>>>>>G726_16\n");
			encCfg.nEncType = IFX_TAPI_COD_TYPE_G726_16;
			encCfg.AAL2BitPack = IFX_TAPI_COD_AAL2_BITPACK;
			break;
		case cod_pt_G726_24:
			//fprintf(stderr,">>>>>>>>>>>>>G726_24\n");
			encCfg.nEncType = IFX_TAPI_COD_TYPE_G726_24;
			encCfg.AAL2BitPack = IFX_TAPI_COD_AAL2_BITPACK;
			break;
		case cod_pt_G726_32:
			//fprintf(stderr,">>>>>>>>>>>>>G726_32\n");
			encCfg.nEncType = IFX_TAPI_COD_TYPE_G726_32;
			encCfg.AAL2BitPack = IFX_TAPI_COD_AAL2_BITPACK;
			break;
		case cod_pt_G726_40:
			//fprintf(stderr,">>>>>>>>>>>>>G726_40\n");
			encCfg.nEncType = IFX_TAPI_COD_TYPE_G726_40;
			encCfg.AAL2BitPack = IFX_TAPI_COD_AAL2_BITPACK;
			break;
		case cod_pt_ILBC_133:
			//fprintf(stderr,">>>>>>>>>>>>>iLBC\n");
			encCfg.nEncType = IFX_TAPI_COD_TYPE_ILBC_133;
			encCfg.nFrameLen = IFX_TAPI_COD_LENGTH_30;
			break;
	}

	/* Configure encoder and decoder gains : 0 dB */
	codVolume.nEnc = 0;
	codVolume.nDec = 0;


	/* Set the coder */ 
	err = 0;
	err = ioctl(chan->rtp_fd, IFX_TAPI_ENC_CFG_SET, &encCfg);
	if(err){
		ab_err_set(chan, AB_ERR_UNKNOWN, "coder set ioctl error");
		err_summary++;
	}
	/* Set the VAD off */
	err = 0;
	err = ioctl(chan->rtp_fd, IFX_TAPI_ENC_VAD_CFG_SET, 
			IFX_TAPI_ENC_VAD_NOVAD);
	if(err){
		ab_err_set(chan, AB_ERR_UNKNOWN, "vad set ioctl error");
		err_summary++;
	}
	/* Configure encoder and decoder gains : 0 dB */
	err = 0;
	err = ioctl(chan->rtp_fd, IFX_TAPI_COD_VOLUME_SET, &codVolume);
	if(err){
		ab_err_set(chan, AB_ERR_UNKNOWN, "volume set ioctl error");
		err_summary++;
	}
	/* Disable high-pass filter */
	err = 0;
	err = ioctl(chan->rtp_fd, IFX_TAPI_COD_DEC_HP_SET, IFX_FALSE);
	if(err){
		ab_err_set(chan, AB_ERR_UNKNOWN, "high pass set ioctl error");
		err_summary++;
	}

	err = ab_chan_media_switch (chan, 1, 1);
	if(err){
		err_summary++;
	}
	
	if(err_summary){
		err = -1;
	}

//fprintf(stderr,"MEDIA ACTIVATED!\n");
	return err;
}

int 
ab_chan_media_deactivate ( ab_chan_t * const chan )
{
	int err;
	err = ab_chan_media_switch (chan, 0, 0);
//fprintf(stderr,"MEDIA DE_ACTIVATED!\n");
	return err;
}

