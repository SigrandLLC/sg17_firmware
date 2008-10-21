#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <fcntl.h>

#include "ab_internal_v22.h"

/**
	Tune RTP parameters on given channel
\param
	chan - channel to operate on it
	rtpp - RTP parameters
\return 
	0 in success case and other value otherwise
\remark
	returns the ioctl error value and writes error message
*/
int 
ab_chan_media_rtp_tune( ab_chan_t * const chan )
{
	IFX_TAPI_PKT_RTP_PT_CFG_t rtpPTConf;
	IFX_TAPI_PKT_RTP_CFG_t rtpConf;
	IFX_TAPI_PKT_VOLUME_t codVolume;
	IFX_TAPI_ENC_CFG_t encCfg;
	rtp_session_prms_t * rtpp = &chan->rtp_cfg;
	int vad_param;
	int hpf_param;
	int err_summary = 0;
	int err;

	memset(&rtpPTConf, 0, sizeof(rtpPTConf));
	memset(&rtpConf, 0, sizeof(rtpConf));
	memset(&codVolume, 0, sizeof(codVolume));
	memset(&encCfg, 0, sizeof(encCfg));

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
	rtpConf.nEventPT = rtpp->evtPT;
	rtpConf.nEventPlayPT = rtpp->evtPTplay;

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
	case play_evts_APT_PLAY:
		rtpConf.nPlayEvents = IFX_TAPI_PKT_EV_OOBPLAY_APT_PLAY;
		break;
	}

	encCfg.nFrameLen = IFX_TAPI_COD_LENGTH_20;

	switch(rtpp->cod_pt){
	case cod_pt_MLAW:
		encCfg.nEncType = IFX_TAPI_COD_TYPE_MLAW;
		break;
	case cod_pt_ALAW:
		encCfg.nEncType = IFX_TAPI_COD_TYPE_ALAW;
		break;
	case cod_pt_G729:
		encCfg.nEncType = IFX_TAPI_COD_TYPE_G729;
		encCfg.nFrameLen = IFX_TAPI_COD_LENGTH_10;
		break;
	case cod_pt_G726_16:
		encCfg.nEncType = IFX_TAPI_COD_TYPE_G726_16;
		encCfg.AAL2BitPack = IFX_TAPI_COD_AAL2_BITPACK;
		break;
	case cod_pt_G726_24:
		encCfg.nEncType = IFX_TAPI_COD_TYPE_G726_24;
		encCfg.AAL2BitPack = IFX_TAPI_COD_AAL2_BITPACK;
		break;
	case cod_pt_G726_32:
		encCfg.nEncType = IFX_TAPI_COD_TYPE_G726_32;
		encCfg.AAL2BitPack = IFX_TAPI_COD_AAL2_BITPACK;
		break;
	case cod_pt_G726_40:
		encCfg.nEncType = IFX_TAPI_COD_TYPE_G726_40;
		encCfg.AAL2BitPack = IFX_TAPI_COD_AAL2_BITPACK;
		break;
	case cod_pt_ILBC_133:
		encCfg.nEncType = IFX_TAPI_COD_TYPE_ILBC_133;
		encCfg.nFrameLen = IFX_TAPI_COD_LENGTH_30;
		break;
	}

	/* Configure encoder and decoder gains */
	codVolume.nEnc = rtpp->cod_volume.enc_dB;
	codVolume.nDec = rtpp->cod_volume.dec_dB;

	/* Set the VAD configuration */
	switch(rtpp->VAD_cfg){
	case vad_cfg_ON:
		vad_param = IFX_TAPI_ENC_VAD_ON;
		break;
	case vad_cfg_OFF:
		vad_param = IFX_TAPI_ENC_VAD_NOVAD;
		break;
	case vad_cfg_G711:
		vad_param = IFX_TAPI_ENC_VAD_G711;
		break;
	case vad_cfg_CNG_only:
		vad_param = IFX_TAPI_ENC_VAD_CNG_ONLY;
		break;
	case vad_cfg_SC_only:
		vad_param = IFX_TAPI_ENC_VAD_SC_ONLY;
		break;
	}

	/* Configure high-pass filter */
	if(rtpp->HPF_is_ON){
		hpf_param = IFX_TRUE;
	} else {
		hpf_param = IFX_FALSE;
	}

/* tag__ show choose */
fprintf(stderr,"[%d]: OOB[",chan->abs_idx);
switch(rtpp->nEvents){
	case evts_OOB_DEFAULT:
fprintf(stderr,"def");
		break;
	case evts_OOB_NO:
fprintf(stderr,"no");
		break;
	case evts_OOB_ONLY:
fprintf(stderr,"only");
		break;
	case evts_OOB_ALL:
fprintf(stderr,"all");
		break;
	case evts_OOB_BLOCK:
fprintf(stderr,"block");
		break;
	}
fprintf(stderr,"/");
switch(rtpp->nPlayEvents){
	case play_evts_DEFAULT:
fprintf(stderr,"def");
		break;
	case play_evts_PLAY:
fprintf(stderr,"play");
		break;
	case play_evts_MUTE:
fprintf(stderr,"mute");
		break;
	case play_evts_APT_PLAY:
fprintf(stderr,"apt");
		break;
	}
fprintf(stderr,"] |0x%X/0x%X| ",rtpConf.nEventPT,rtpConf.nEventPlayPT);
switch(rtpp->cod_pt){
	case cod_pt_MLAW:
fprintf(stderr,"mlaw");
		break;
	case cod_pt_ALAW:
fprintf(stderr,"alaw");
		break;
	case cod_pt_G729:
fprintf(stderr,"g729");
		break;
	default:
fprintf(stderr,"cod_");
}
fprintf(stderr,"[e%d/d%d] ",codVolume.nEnc,codVolume.nDec);
switch(rtpp->VAD_cfg){
	case vad_cfg_ON:
fprintf(stderr,"vad on");
		break;
	case vad_cfg_OFF:
fprintf(stderr,"vad off");
		break;
	case vad_cfg_G711:
fprintf(stderr,"vad g711");
		break;
	case vad_cfg_CNG_only:
fprintf(stderr,"vad cng_only");
		break;
	case vad_cfg_SC_only:
fprintf(stderr,"vad sc_only");
		break;
	}

if(rtpp->HPF_is_ON){
fprintf(stderr," hpf on\n");
	} else {
fprintf(stderr," hpf off\n");
	}

	/*********** ioctl seq */

	/* Set the coder payload table */ 
	err = 0;
	err = ioctl(chan->rtp_fd, IFX_TAPI_PKT_RTP_PT_CFG_SET, &rtpPTConf);
	if(err){
		err_summary++;
		ab_err_set(AB_ERR_UNKNOWN, "media rtp PT tune ioctl error");
	}

	/* Set the rtp configuration (OOB, pkt params, etc.) */ 
	err = 0;
	err = ioctl(chan->rtp_fd, IFX_TAPI_PKT_RTP_CFG_SET, &rtpConf);
	if(err){
		err_summary++;
		ab_err_set(AB_ERR_UNKNOWN, "media rtp tune ioctl error");
	}

	/* Set the coder */ 
	err = 0;
	err = ioctl(chan->rtp_fd, IFX_TAPI_ENC_CFG_SET, &encCfg);
	if(err){
		ab_err_set(AB_ERR_UNKNOWN, "coder set ioctl error");
		err_summary++;
	}
	/* Set the VAD configuration */
	err = 0;
	err = ioctl(chan->rtp_fd, IFX_TAPI_ENC_VAD_CFG_SET, vad_param);
	if(err){
		ab_err_set(AB_ERR_UNKNOWN, "vad set ioctl error");
		err_summary++;
	}

	/* Configure encoder and decoder gains */
	err = 0;
	err = ioctl(chan->rtp_fd, IFX_TAPI_COD_VOLUME_SET, &codVolume);
	if(err){
		ab_err_set(AB_ERR_UNKNOWN, "volume set ioctl error");
		err_summary++;
	}

	/* Configure high-pass filter */
	err = 0;
	err = ioctl(chan->rtp_fd, IFX_TAPI_COD_DEC_HP_SET, hpf_param);
	if(err){
		ab_err_set(AB_ERR_UNKNOWN, "high pass set ioctl error");
		err_summary++;
	}

	if(err_summary){
		err = -1;
	}
	return err;
}

/**
	Switch media on / off on the given channel
\param
	chan - channel to operate on it
	enc_on - on (1) or off (0) encoding
	dec_on - on (1) or off (0) decoding
\return 
	0 in success case and other value otherwise
\remark
	returns the ioctl error value and writes error message
*/
int 
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
		ab_err_set(AB_ERR_UNKNOWN, "encoder start/stop ioctl error");
		err_summary++;
	}

	err = 0;
	if (dec_on){
		err = ioctl(chan->rtp_fd, IFX_TAPI_DEC_START, 0);
	} else {
		err = ioctl(chan->rtp_fd, IFX_TAPI_DEC_STOP, 0);
	}
	if(err){
		ab_err_set(AB_ERR_UNKNOWN, "decoder start/stop ioctl error");
		err_summary++;
	}

	if(err_summary){
		return -1;
	}
	return 0;
}

