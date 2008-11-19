#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <fcntl.h>

#include "ab_internal_v22.h"

/**
 * \param[in,out] chan channel to operate on.
 * \param[in] fcod codec for fax transmittion.
 * \retval	0 in success.
 * \retval	-1 if fail.
 * \remark
 *	Set coder type fo MLAW
 *	Set Jitter Buffer fixed
 *	Set WLEC type NE with NLP off
 *	fcodt must be ALAW or MLAW
 */ 
int 
ab_chan_fax_pass_through_start( ab_chan_t * const chan, 
		enum cod_type_e const fcodt )
{/*{{{*/
	IFX_TAPI_JB_CFG_t jbCfgData;
	IFX_TAPI_WLEC_CFG_t lecConf;
	IFX_TAPI_COD_TYPE_t encTypeData;
	int cfd = chan->rtp_fd;
	int err;
	int err_sum = 0;

	memset(&jbCfgData, 0, sizeof(jbCfgData));
	memset(&encTypeData, 0, sizeof(encTypeData));
	memset(&lecConf, 0, sizeof(lecConf));

	if       (fcodt == cod_type_MLAW){
		encTypeData = IFX_TAPI_COD_TYPE_MLAW;
	} else if(fcodt == cod_type_ALAW){
		encTypeData = IFX_TAPI_COD_TYPE_ALAW;
	} else {
		ab_err_set(AB_ERR_BAD_PARAM, "wrong fax coder");
		err_sum++;
		goto __exit;
	}
	/* Configure coder for fax/modem communications */
	err = ioctl(cfd, IFX_TAPI_ENC_TYPE_SET, encTypeData);
	if(err != IFX_SUCCESS){
		ioctl (cfd, FIO_VINETIC_LASTERR, &ab_g_err_extra_value);
		ab_err_set(AB_ERR_UNKNOWN, "IFX_TAPI_ENC_TYPE_SET");
		err_sum++;
	}

	/* Reconfigure JB for fax/modem communications */
	jbCfgData.nJbType = IFX_TAPI_JB_TYPE_FIXED;
	jbCfgData.nPckAdpt = IFX_TAPI_JB_PKT_ADAPT_DATA;
	err = ioctl(cfd, IFX_TAPI_JB_CFG_SET, &jbCfgData);
	if(err != IFX_SUCCESS){
		ioctl (cfd, FIO_VINETIC_LASTERR, &ab_g_err_extra_value);
		ab_err_set(AB_ERR_UNKNOWN, "IFX_TAPI_JB_CFG_SET");
		err_sum++;
	}
	
	/* Reconfigure LEC for fax/modem communications */
	lecConf.nType = IFX_TAPI_WLEC_TYPE_NE;
	lecConf.bNlp = IFX_TAPI_LEC_NLP_OFF;
	err =ioctl(cfd,IFX_TAPI_WLEC_PHONE_CFG_SET,&lecConf);
	if(err != IFX_SUCCESS){
		ioctl (cfd, FIO_VINETIC_LASTERR, &ab_g_err_extra_value);
		ab_err_set(AB_ERR_UNKNOWN, "IFX_TAPI_WLEC_PHONE_CFG_SET");
		err_sum++;
	}
__exit:
	err = err_sum ? -1 : 0;
	return err;
}/*}}}*/


/**
 *	Tune RTP parameters on given channel
 *
 * \param[in,out] chan channel to operate on.
 * \param[in] cod codec type and parameters.
 * \param[in] fcod fax codec type and parameters.
 * \retval	0 in success.
 * \retval	-1 if fail.
 * \remark
 * 	from \c fcod using just \c type and \c sdp_selected_payload_type.
 */ 
int 
ab_chan_media_rtp_tune( ab_chan_t * const chan, codec_t const * const cod,
		codec_t const * const fcod)
{/*{{{*/
	IFX_TAPI_PKT_RTP_PT_CFG_t rtpPTConf;
	IFX_TAPI_PKT_RTP_CFG_t rtpConf;
	IFX_TAPI_PKT_VOLUME_t codVolume;
	IFX_TAPI_ENC_CFG_t encCfg;
	rtp_session_prms_t * rtpp = &chan->rtp_cfg;
	int vad_param;
	int hpf_param;
	int fcodt;
	int err_summary = 0;
	int err;

	memset(&rtpPTConf, 0, sizeof(rtpPTConf));
	memset(&rtpConf, 0, sizeof(rtpConf));
	memset(&codVolume, 0, sizeof(codVolume));
	memset(&encCfg, 0, sizeof(encCfg));

	/* Payload types table and encCfg */
	if       (cod->type == cod_type_MLAW){
		encCfg.nEncType = IFX_TAPI_COD_TYPE_MLAW;
	} else if(cod->type == cod_type_ALAW){
		encCfg.nEncType = IFX_TAPI_COD_TYPE_ALAW;
	} else if(cod->type == cod_type_G729){
		encCfg.nEncType = IFX_TAPI_COD_TYPE_G729;
	} else if(cod->type == cod_type_NONE){
		err_summary++;
		ab_err_set(AB_ERR_BAD_PARAM, "codec type not set");
		goto __exit;
	}
		/*
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
		*/
	/* tuning voice transmission codec */
	rtpPTConf.nPTup [encCfg.nEncType] = rtpPTConf.nPTdown [encCfg.nEncType] = 
				cod->sdp_selected_payload;

	if       (fcod->type == cod_type_MLAW){
		fcodt = IFX_TAPI_COD_TYPE_MLAW;
	} else if(fcod->type == cod_type_ALAW){
		fcodt = IFX_TAPI_COD_TYPE_ALAW;
	} else {
		err_summary++;
		ab_err_set(AB_ERR_UNKNOWN, "fax codec type must be aLaw or uLaw");
		goto __exit;
	}
	/* tuning fax transmission codec */
	rtpPTConf.nPTup [fcodt] = rtpPTConf.nPTdown [fcodt] = 
			fcod->sdp_selected_payload;

	rtpConf.nSeqNr = 0;
	rtpConf.nSsrc = 0;

	/* set out-of-band (RFC 2833 packet) transmission type */
	if       (rtpp->nEvents == evts_OOB_DEFAULT){
		rtpConf.nEvents = IFX_TAPI_PKT_EV_OOB_DEFAULT;
	} else if(rtpp->nEvents == evts_OOB_NO){
		rtpConf.nEvents = IFX_TAPI_PKT_EV_OOB_NO;
	} else if(rtpp->nEvents == evts_OOB_ONLY){
		rtpConf.nEvents = IFX_TAPI_PKT_EV_OOB_ONLY;
	} else if(rtpp->nEvents == evts_OOB_ALL){
		rtpConf.nEvents = IFX_TAPI_PKT_EV_OOB_ALL;
	} else if(rtpp->nEvents == evts_OOB_BLOCK){
		rtpConf.nEvents = IFX_TAPI_PKT_EV_OOB_BLOCK;
	}

	/* Configure payload type for RFC 2833 packets */
	rtpConf.nEventPT = rtpp->evtPT;
	rtpConf.nEventPlayPT = rtpp->evtPTplay;

	/* What to do upon RFC 2833 packets reception */
	if       (rtpp->nPlayEvents == play_evts_DEFAULT){
		rtpConf.nPlayEvents = IFX_TAPI_PKT_EV_OOBPLAY_DEFAULT;
	} else if(rtpp->nPlayEvents == play_evts_PLAY){
		rtpConf.nPlayEvents = IFX_TAPI_PKT_EV_OOBPLAY_PLAY;
	} else if(rtpp->nPlayEvents == play_evts_MUTE){
		rtpConf.nPlayEvents = IFX_TAPI_PKT_EV_OOBPLAY_MUTE;
	} else if(rtpp->nPlayEvents == play_evts_APT_PLAY){
		rtpConf.nPlayEvents = IFX_TAPI_PKT_EV_OOBPLAY_APT_PLAY;
	}

	/* tag__ df - 20 - set in cfg_file
	 * 729 - df 10 - set in cfg_file
	 * encCfg.nFrameLen = IFX_TAPI_COD_LENGTH_20; */

	if       (cod->pkt_size == cod_pkt_size_2_5){
		encCfg.nFrameLen = IFX_TAPI_COD_LENGTH_2_5;
	} else if(cod->pkt_size == cod_pkt_size_5){
		encCfg.nFrameLen = IFX_TAPI_COD_LENGTH_5;
	} else if(cod->pkt_size == cod_pkt_size_5_5){
		encCfg.nFrameLen = IFX_TAPI_COD_LENGTH_5_5;
	} else if(cod->pkt_size == cod_pkt_size_10){
		encCfg.nFrameLen = IFX_TAPI_COD_LENGTH_10;
	} else if(cod->pkt_size == cod_pkt_size_11){
		encCfg.nFrameLen = IFX_TAPI_COD_LENGTH_11;
	} else if(cod->pkt_size == cod_pkt_size_20){
		encCfg.nFrameLen = IFX_TAPI_COD_LENGTH_20;
	} else if(cod->pkt_size == cod_pkt_size_30){
		encCfg.nFrameLen = IFX_TAPI_COD_LENGTH_30;
	} else if(cod->pkt_size == cod_pkt_size_40){
		encCfg.nFrameLen = IFX_TAPI_COD_LENGTH_40;
	} else if(cod->pkt_size == cod_pkt_size_50){
		encCfg.nFrameLen = IFX_TAPI_COD_LENGTH_50;
	} else if(cod->pkt_size == cod_pkt_size_60){
		encCfg.nFrameLen = IFX_TAPI_COD_LENGTH_60;
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

#if 0/*{{{*/
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
#endif/*}}}*/

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

__exit:
	if(err_summary){
		err = -1;
	}
	return err;
}/*}}}*/

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
{/*{{{*/
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
}/*}}}*/

