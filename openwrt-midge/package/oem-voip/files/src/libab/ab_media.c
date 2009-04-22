#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <fcntl.h>

#include "ab_internal_v22.h"

/**
 * \param[in,out] chan channel to operate on.
 * \retval	0 in success.
 * \retval	-1 if fail.
 * \remark
 *	Set coder type fo MLAW
 *	Set Jitter Buffer fixed
 *	Set WLEC type NE with NLP off
 *	fcodt must be ALAW or MLAW
 */ 
int 
ab_chan_fax_pass_through_start( ab_chan_t * const chan ) 
{/*{{{*/
	IFX_TAPI_JB_CFG_t jbCfgData;
	IFX_TAPI_WLEC_CFG_t lecConf;
	IFX_TAPI_COD_TYPE_t encTypeData;
	IFX_TAPI_ENC_CFG_t encCfg;
	IFX_TAPI_DEC_CFG_t decCfg;
	int cfd = chan->rtp_fd;
	int err;
	int err_sum = 0;

	memset(&jbCfgData, 0, sizeof(jbCfgData));
	memset(&encTypeData, 0, sizeof(encTypeData));
	memset(&lecConf, 0, sizeof(lecConf));
	memset(&encCfg, 0, sizeof(encCfg));
	memset(&decCfg, 0, sizeof(decCfg));

	encTypeData = IFX_TAPI_COD_TYPE_ALAW;

	/* set normal encoder/decoder seq for fax */
	encCfg.AAL2BitPack = decCfg.AAL2BitPack = IFX_TAPI_COD_RTP_BITPACK;

	/* Set the encoder */ 
	err = 0;
	err = ioctl(cfd, IFX_TAPI_ENC_CFG_SET, &encCfg);
	if(err){
		ab_err_set(AB_ERR_UNKNOWN, "encoder set ioctl error");
		err_sum++;
	}
	/* Set the decoder */ 
	err = 0;
	err = ioctl(cfd, IFX_TAPI_DEC_CFG_SET, &decCfg);
	if(err){
		ab_err_set(AB_ERR_UNKNOWN, "decoder set ioctl error");
		err_sum++;
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
	IFX_TAPI_DEC_CFG_t decCfg;
	rtp_session_prms_t * rtpp = &chan->rtp_cfg;
	int vad_param;
	int hpf_param;
	int fcodt;
	int check_bitpack = 0;
	int err_summary = 0;
	int err;

	memset(&rtpPTConf, 0, sizeof(rtpPTConf));
	memset(&rtpConf, 0, sizeof(rtpConf));
	memset(&codVolume, 0, sizeof(codVolume));
	memset(&encCfg, 0, sizeof(encCfg));
	memset(&decCfg, 0, sizeof(decCfg));

	/* frame len {{{*/
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
	/*}}}*/
	/* PTypes table and [enc,dec]Cfg, correct frame len and set bitpack{{{*/
	if(cod->type == cod_type_ALAW){
		encCfg.nEncType = IFX_TAPI_COD_TYPE_ALAW;
		rtpPTConf.nPTup   [encCfg.nEncType] = 
		rtpPTConf.nPTdown [encCfg.nEncType] = 
				cod->sdp_selected_payload;
	} else if(cod->type == cod_type_G729){
		encCfg.nEncType = IFX_TAPI_COD_TYPE_G729;
		rtpPTConf.nPTup   [encCfg.nEncType] = 
		rtpPTConf.nPTdown [encCfg.nEncType] = 
				cod->sdp_selected_payload;
	} else if(cod->type == cod_type_G729E){
		encCfg.nEncType = IFX_TAPI_COD_TYPE_G729_E;
		rtpPTConf.nPTup   [encCfg.nEncType] = 
		rtpPTConf.nPTdown [encCfg.nEncType] = 
				cod->sdp_selected_payload;
	} else if(cod->type == cod_type_ILBC_133){
		encCfg.nFrameLen = IFX_TAPI_COD_LENGTH_30;
		encCfg.nEncType = IFX_TAPI_COD_TYPE_ILBC_133;
		rtpPTConf.nPTup   [IFX_TAPI_COD_TYPE_ILBC_152] = 
		rtpPTConf.nPTdown [IFX_TAPI_COD_TYPE_ILBC_152] = 
		rtpPTConf.nPTup   [IFX_TAPI_COD_TYPE_ILBC_133] = 
		rtpPTConf.nPTdown [IFX_TAPI_COD_TYPE_ILBC_133] = 
				cod->sdp_selected_payload;
	} else if(cod->type == cod_type_G723){
		if((encCfg.nFrameLen != IFX_TAPI_COD_LENGTH_30) &&
			(encCfg.nFrameLen != IFX_TAPI_COD_LENGTH_60)){
			/* can be set 30 or 60 */
			encCfg.nFrameLen = IFX_TAPI_COD_LENGTH_30;
		}
		encCfg.nEncType = IFX_TAPI_COD_TYPE_G723_53;
		rtpPTConf.nPTup   [IFX_TAPI_COD_TYPE_G723_63] = 
		rtpPTConf.nPTdown [IFX_TAPI_COD_TYPE_G723_63] = 
		rtpPTConf.nPTup   [IFX_TAPI_COD_TYPE_G723_53] = 
		rtpPTConf.nPTdown [IFX_TAPI_COD_TYPE_G723_53] = 
				cod->sdp_selected_payload;
	} else if(cod->type == cod_type_G726_16){
		encCfg.nEncType = IFX_TAPI_COD_TYPE_G726_16;
		rtpPTConf.nPTup   [IFX_TAPI_COD_TYPE_G726_16] = 
		rtpPTConf.nPTdown [IFX_TAPI_COD_TYPE_G726_16] = 
				cod->sdp_selected_payload;
		check_bitpack = 1;
	} else if(cod->type == cod_type_G726_24){
		encCfg.nEncType = IFX_TAPI_COD_TYPE_G726_24;
		rtpPTConf.nPTup   [IFX_TAPI_COD_TYPE_G726_24] = 
		rtpPTConf.nPTdown [IFX_TAPI_COD_TYPE_G726_24] = 
				cod->sdp_selected_payload;
		check_bitpack = 1;
	} else if(cod->type == cod_type_G726_32){
		encCfg.nEncType = IFX_TAPI_COD_TYPE_G726_32;
		rtpPTConf.nPTup   [IFX_TAPI_COD_TYPE_G726_32] = 
		rtpPTConf.nPTdown [IFX_TAPI_COD_TYPE_G726_32] = 
				cod->sdp_selected_payload;
		check_bitpack = 1;
	} else if(cod->type == cod_type_G726_40){
		encCfg.nEncType = IFX_TAPI_COD_TYPE_G726_40;
		rtpPTConf.nPTup   [IFX_TAPI_COD_TYPE_G726_40] = 
		rtpPTConf.nPTdown [IFX_TAPI_COD_TYPE_G726_40] = 
				cod->sdp_selected_payload;
		check_bitpack = 1;
	} else if(cod->type == cod_type_NONE){
		err_summary++;
		ab_err_set(AB_ERR_BAD_PARAM, "codec type not set");
		goto __exit;
	}

	if(check_bitpack){
		if(cod->bpack == bitpack_RTP){
			encCfg.AAL2BitPack = 
			decCfg.AAL2BitPack = IFX_TAPI_COD_RTP_BITPACK;
		} else {
			encCfg.AAL2BitPack = 
			decCfg.AAL2BitPack = IFX_TAPI_COD_AAL2_BITPACK;
		}
	} else {
		encCfg.AAL2BitPack = 
		decCfg.AAL2BitPack = IFX_TAPI_COD_RTP_BITPACK;
	}
	/*}}}*/
	/* FAX {{{*/
	fcodt = IFX_TAPI_COD_TYPE_ALAW;
	/* tuning fax transmission codec */
	rtpPTConf.nPTup [fcodt] = rtpPTConf.nPTdown [fcodt] = 
			fcod->sdp_selected_payload;
	/*}}}*/
	rtpConf.nSeqNr = 0;
	rtpConf.nSsrc = 0;
	/* set out-of-band (RFC 2833 packet) configuratoin {{{*/
	rtpConf.nEvents = IFX_TAPI_PKT_EV_OOB_NO;
	rtpConf.nEventPT = 0x62;
	rtpConf.nEventPlayPT = 0x62;
	rtpConf.nPlayEvents = IFX_TAPI_PKT_EV_OOBPLAY_MUTE;
	/*}}}*/
	/* Configure encoder and decoder gains {{{*/
	codVolume.nEnc = rtpp->cod_volume.enc_dB;
	codVolume.nDec = rtpp->cod_volume.dec_dB;
	/*}}}*/
	/* Set the VAD configuration {{{*/
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
	/*}}}*/
	/* Configure high-pass filter {{{ */
	if(rtpp->HPF_is_ON){
		hpf_param = IFX_TRUE;
	} else {
		hpf_param = IFX_FALSE;
	}
	/*}}}*/
#if 0/*{{{*/
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
	/*********** ioctl seq {{{*/

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

	/* Set the encoder */ 
	err = 0;
	err = ioctl(chan->rtp_fd, IFX_TAPI_ENC_CFG_SET, &encCfg);
	if(err){
		ab_err_set(AB_ERR_UNKNOWN, "encoder set ioctl error");
		err_summary++;
	}
	/* Set the decoder */ 
	err = 0;
	err = ioctl(chan->rtp_fd, IFX_TAPI_DEC_CFG_SET, &decCfg);
	if(err){
		ab_err_set(AB_ERR_UNKNOWN, "decoder set ioctl error");
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
	/*}}}*/
__exit:
	if(err_summary){
		err = -1;
	}
	return err;
}/*}}}*/

/**
 *	Tune WLEC (Window-based Line Echo Canceller) parameters on given channel
 *
 * \param[in,out] chan channel to operate on.
 * \param[in] wp wlec parameters.
 * \retval	0 in success.
 * \retval	-1 if fail.
 */ 
int 
ab_chan_media_wlec_tune( ab_chan_t * const chan, wlec_t const * const wp )
{/*{{{*/
	IFX_TAPI_WLEC_CFG_t lecConf;
	int err;

	memset(&lecConf, 0, sizeof(lecConf));

	/* WLEC mode */
	if        (wp->mode == wlec_mode_OFF){
		lecConf.nType = IFX_TAPI_WLEC_TYPE_OFF;
	} else if (wp->mode == wlec_mode_NE){
		lecConf.nType = IFX_TAPI_WLEC_TYPE_NE;
	} else if (wp->mode == wlec_mode_NFE){
		lecConf.nType = IFX_TAPI_WLEC_TYPE_NFE;
	}

	/* NLP */
	if        (wp->nlp == wlec_nlp_DEFAULT){
		lecConf.bNlp = IFX_TAPI_WLEC_NLP_DEFAULT;
	} else if (wp->nlp == wlec_nlp_ON){
		lecConf.bNlp = IFX_TAPI_WLEC_NLP_ON;
	} else if (wp->nlp == wlec_nlp_OFF){
		lecConf.bNlp = IFX_TAPI_WLEC_NLP_OFF;
	}

	/* Windows */
	if        (wp->ne_nb == wlec_window_size_4){
		lecConf.nNBNEwindow = IFX_TAPI_WLEN_WSIZE_4;
	} else if (wp->ne_nb == wlec_window_size_6){
		lecConf.nNBNEwindow = IFX_TAPI_WLEN_WSIZE_6;
	} else if (wp->ne_nb == wlec_window_size_8){
		lecConf.nNBNEwindow = IFX_TAPI_WLEN_WSIZE_8;
	} else if (wp->ne_nb == wlec_window_size_16){
		lecConf.nNBNEwindow = IFX_TAPI_WLEN_WSIZE_16;
	}

	if        (wp->fe_nb == wlec_window_size_4){
		lecConf.nNBFEwindow = IFX_TAPI_WLEN_WSIZE_4;
	} else if (wp->fe_nb == wlec_window_size_6){
		lecConf.nNBFEwindow = IFX_TAPI_WLEN_WSIZE_6;
	} else if (wp->fe_nb == wlec_window_size_8){
		lecConf.nNBFEwindow = IFX_TAPI_WLEN_WSIZE_8;
	} else if (wp->fe_nb == wlec_window_size_16){
		lecConf.nNBFEwindow = IFX_TAPI_WLEN_WSIZE_16;
	}

	if        (wp->ne_wb == wlec_window_size_4){
		lecConf.nWBNEwindow = IFX_TAPI_WLEN_WSIZE_4;
	} else if (wp->ne_wb == wlec_window_size_6){
		lecConf.nWBNEwindow = IFX_TAPI_WLEN_WSIZE_6;
	} else if (wp->ne_wb == wlec_window_size_8){
		lecConf.nWBNEwindow = IFX_TAPI_WLEN_WSIZE_8;
	} else if (wp->ne_wb == wlec_window_size_16){
		lecConf.nWBNEwindow = IFX_TAPI_WLEN_WSIZE_16;
	}

	/* Set configuration */
	err = ioctl(chan->rtp_fd, IFX_TAPI_WLEC_PHONE_CFG_SET, &lecConf);

	if (err){
		ab_err_set(AB_ERR_UNKNOWN, "wlec_phone_cfg_set ioctl error");
		err = -1;
	}
	return err;
}/*}}}*/

/**
	Switch media on / off on the given channel
\param chan - channel to operate on it
\param enc_on - on (1) or off (0) encoding
\param dec_on - on (1) or off (0) decoding
\return 
	0 in success case and other value otherwise
\remark
	returns the ioctl error value and writes error message
*/
int 
ab_chan_media_switch( ab_chan_t * const chan,
		unsigned char const enc_on, unsigned char const dec_on )
{/*{{{*/
	int err = 0;
	int err_summary = 0;

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

