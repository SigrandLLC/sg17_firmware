#include "mam17h_mpair.h"


int is_chan_in_mpair(struct mam17_card * card, int ch)
{
	if ((card->mpair_mode == MPAIR01) && ((ch == 0) || (ch == 1))) return 1;
	if ((card->mpair_mode == MPAIR021) && ((ch == 0) || (ch == 1) || (ch == 2))) return 1;
	if (card->mpair_mode == MPAIR0321) return 1;
	if ((card->mpair_mode == MPAIR03) && ((ch == 0) || (ch == 3))) return 1;
	if (card->mpair_mode == MPAIR01_23) return 1;
	return 0;
}


int mpair_master(struct mam17_card *card)
{
	u32 buf[32];
	ack_t ack;
	struct cmd_hdlc_tc_link_linkmodify *linkmodify;
	struct cmd_pmd_spanprofilegroupconfig *spanprofile;
	struct cmd_pmd_control *pmd_control;

	struct cmd_cfg_ghs_mode *ghs_mode;
	struct cmd_cfg_caplist_short_ver_2 *caplist;
	struct cmd_cfg_sdi_tx *sdi_tx;
	struct cmd_cfg_sdi_rx *sdi_rx;
	struct cmd_cfg_ghs_extended_pam_mode *extended_pam_mode;
	struct cmd_xmii_modify *xmii_modify;

	struct cmd_tc_flowmodify *flowmodify;

	int i = 0;
	int mpair_ref = 0;
	int mpair_mode = card->mpair_mode;
//	if (mpair_mode == MPAIR23) mpair_mode = 2;


	for (i = 0; i < 4; i++)
	{
		if (!is_chan_in_mpair(card, i)) continue;
		buf[0] = i;
		if (mpi_cmd(card, CMD_PMD_Reset, buf, 1, &ack))/////////////////////////
		{
			PDEBUG(0, "CMD_PMD_Reset error");
			return -1;
		}
	}
	mdelay(100);
/////////// my

	flowmodify=(struct cmd_tc_flowmodify *)buf;
	memset(flowmodify,0,sizeof(*flowmodify));

	flowmodify->dsl0_ts = HDLC_TC_LAYER;
	flowmodify->dsl1_ts = HDLC_TC_LAYER;
	flowmodify->dsl2_ts = HDLC_TC_LAYER;
	flowmodify->dsl3_ts = HDLC_TC_LAYER;

	if (mpi_cmd(card, CMD_TC_FlowModify, buf, sizeof(*flowmodify)/4, &ack))/////////////////////////
	{
		PDEBUG(0, "CMD_TC_FlowModify error");
		return -1;
	}
	mdelay(1000);

//////////

two_mpair:

	linkmodify=(struct cmd_hdlc_tc_link_linkmodify *)buf;
	memset(linkmodify,0,sizeof(*linkmodify));

	linkmodify->linkNo = mpair_ref;
	linkmodify->bitdyte = BIT_STUFFING;
	linkmodify->interframe_ch = card->channels[mpair_ref].fill_7e;
	linkmodify->sharedflags = IFX_ENABLE;
	linkmodify->fcs = IFX_DISABLE;
	linkmodify->acf_insert = IFX_DISABLE;
	linkmodify->txaddrctrl = 0xff03;
	linkmodify->li_m_pairports = mpair_mode;
	linkmodify->clockingmode = NO_CLK;

	if (mpi_cmd(card, CMD_HDLC_TC_LinkModify, buf, sizeof(*linkmodify)/4, &ack))/////////////////////////
	{
		PDEBUG(0, "CMD_HDLC_TC_LinkModify error");
		return -1;
	}

	buf[0] = mpair_ref;
	if (mpi_cmd(card, CMD_PMD_CO_PortSubTypeSelect, buf, 1, &ack))/////////////////////////
	{
		PDEBUG(0, "CMD_PMD_CO_PortSubTypeSelect error");
		return -1;
	}
	mdelay(100);
	spanprofile=(struct cmd_pmd_spanprofilegroupconfig *)buf;
	memset(spanprofile,0,sizeof(*spanprofile));

	spanprofile->LinkNo = mpair_ref;
	spanprofile->wireinterface = TWO_WIRE;
	spanprofile->minlinerate = 256000;
	spanprofile->maxlinerate = 15196000;//5696000;
	spanprofile->minlinesubrat = 0x0;
	spanprofile->maxlinesubrat = 0x0;
	spanprofile->psd = SYMETRIC;
	spanprofile->transmod = card->channels[mpair_ref].annex;
	spanprofile->remoteenabled = EOC_ENABLED_IDC;
	spanprofile->powerfeeding = NO_POWER;
	spanprofile->cc_targetmargindown = 0x00;
	spanprofile->wc_targetmargindown = 0x00;
	spanprofile->cc_targetmarginup = 0x00;
	spanprofile->wc_targetmarginup = 0x00;
	spanprofile->usedtargetmargins = 0x00;
	spanprofile->refcloc = DATA_OR_NETWORK_CLK;
	spanprofile->lineprobe = LP_DISABLE;
	spanprofile->pam_constellation = SELECT_32_PAM;
	spanprofile->capliststyl = NEW_STYLE_CAPLIST;
	spanprofile->pbo_mode = card->channels[mpair_ref].pbo_mode;
	if (card->channels[mpair_ref].pbo_mode)
		spanprofile->pbo_mode = PBO_FORCED;
	if (card->channels[mpair_ref].pbo_mode == PWRBO_NORMAL)
		spanprofile->epl_mode = EPL_ENABLED;
	else
		spanprofile->epl_mode = EPL_DISABLED;
	spanprofile->pbo_valu = card->channels[0].pbo_vals[mpair_ref];

	if (mpi_cmd(card, CMD_PMD_SpanProfileGroupConfig, buf, sizeof(*spanprofile)/4, &ack))/////////////////////////
	{
		PDEBUG(0, "CMD_PMD_SpanProfileGroupConfig error");
		return -1;
	}
/*
	buf[0] = 0;
	buf[1] = 1;
	buf[2] = 8*1024*1000;//speed
	buf[3] = TCPAM128;

	mpi_cmd(card, 0x65, buf, 4, &ack);////////////////////////////----------

	pmd_control=(struct cmd_pmd_control *)buf;
	memset(pmd_control,0,sizeof(*pmd_control));

	pmd_control->LinkNo = 0;
	pmd_control->LinkControl = ENABLE_LINK;
	pmd_control->ActivationState = START_AFTER_INIT;

	if (mpi_cmd(card, CMD_PMD_Control, buf, sizeof(*pmd_control)/4, &ack))/////////////////////////-------------
	{
		PDEBUG(0, "CMD_PMD_Control error");
		return -1;
	}
	mdelay(100);
*/



	pmd_control=(struct cmd_pmd_control *)buf;
	memset(pmd_control,0,sizeof(*pmd_control));

	pmd_control->LinkNo = mpair_ref;
	pmd_control->LinkControl = ENABLE_LINK;
	pmd_control->ActivationState = STOP_AFTER_INIT;

	if (mpi_cmd(card, CMD_PMD_Control, buf, sizeof(*pmd_control)/4, &ack))/////////////////////////
	{
		PDEBUG(0, "CMD_PMD_Control error");
		return -1;
	}
	mdelay(100);


// ----------------Extended PMD configuration----------
// ----------------------------------------------------

	for (i = 0; i < 4; i++)
	{
		if (!is_chan_in_mpair(card, i)) continue;

		ghs_mode = (struct cmd_cfg_ghs_mode *)buf;
		memset(ghs_mode, 0, sizeof(*ghs_mode));

		ghs_mode->transaction = GHS_TRNS_10;
		ghs_mode->startup_initialization=STARTUP_LOCAL;
		switch (card->channels[i].pbo_mode)
		{
			case PWRBO_NORMAL:
				ghs_mode->pbo_mode = PBO_NORMAL;
				ghs_mode->epl_mode = EPL_ENABLED;
			break;
			case PWRBO_FORCED:
				ghs_mode->pbo_mode = PBO_FORCED;
				ghs_mode->epl_mode = EPL_DISABLED;
			break;
		}
		ghs_mode->pmms_margin_mode = PMMS_NORMAL;
		tunnel_cmd(card, CMD_CFG_GHS_MODE, buf, sizeof(*ghs_mode)/4, i, &ack);

		caplist = (struct cmd_cfg_caplist_short_ver_2 *)buf;
		memset(caplist, 0, sizeof(*caplist));

		switch (card->channels[i].clkmode)
		{
			case 0:
				caplist->clock_mode = SHDSL_CLK_MODE_1;
			break;
			case 1:
				caplist->clock_mode = SHDSL_CLK_MODE_3a;
			break;
			case 2:
				caplist->clock_mode = SHDSL_CLK_MODE_2;
			break;
		}
		if (card->channels[i].pbo_mode == PWRBO_FORCED)
		{
			if (card->channels[i].pbo_vnum)
				caplist->pow_backoff = card->channels[i].pbo_vals[0];
			else
				caplist->pow_backoff = 0;
		}
		caplist->annex = card->channels[i].annex + 1;
		caplist->psd_mask = 0x00;

		caplist->base_rate_min = 192;
		caplist->base_rate_max = 2304;
		caplist->base_rate_min16 = 2368;
		caplist->base_rate_max16 = 3840;
		caplist->base_rate_min32 = 768;
		caplist->base_rate_max32 = 5696;

		caplist->sub_rate_min = 0x00;
		caplist->sub_rate_max = 0x00;
		caplist->enable_pmms = PMMS_OFF;
		caplist->pmms_margin = 0x00;

		tunnel_cmd(card, CMD_CFG_CAPLIST_SHORT_VER_2, buf, sizeof(*caplist)/4, i, &ack);

		sdi_rx = (struct cmd_cfg_sdi_rx*)buf;
		memset(sdi_rx, 0, sizeof(*sdi_rx));

		sdi_rx->data_shift = 0x00;
		sdi_rx->frame_shift = 0x00;
		sdi_rx->sp_level = SDI_HIGH;
		sdi_rx->driving_edg = SDI_RISING;
		sdi_rx->data_shift_edg = SDI_NO;
		sdi_rx->lstwr_1strd_dly = 0x64;
		sdi_rx->slip_mode = SLIP_FAST;
		sdi_rx->align = SDI_NO;

		tunnel_cmd(card, CMD_CFG_SDI_RX, buf, sizeof(*sdi_rx)/4, i, &ack);

		sdi_tx = (struct cmd_cfg_sdi_tx*)buf;
		memset(sdi_tx, 0, sizeof(*sdi_tx));

		sdi_tx->data_shift = 0x00;
		sdi_tx->frame_shift = 0x00;
		sdi_tx->sp_level = SDI_HIGH;
		sdi_tx->sp_sample_edg = SDI_FALLING;
		sdi_tx->data_sample_edg = SDI_FALLING;
		sdi_tx->lstwr_1strd_dly = 0x16;
		sdi_tx->slip_mode = SLIP_FAST;
		sdi_tx->align = SDI_NO;

		tunnel_cmd(card, CMD_CFG_SDI_TX, buf, sizeof(*sdi_tx)/4, i, &ack);


		extended_pam_mode=(struct cmd_cfg_ghs_extended_pam_mode*)buf;
		memset(extended_pam_mode,0,sizeof(*extended_pam_mode));

		extended_pam_mode->ext_pam_mode=0x1;
		extended_pam_mode->bits_per_symbol=card->channels[i].tcpam;
		if(card->channels[i].tcpam==TCPAM128){
			extended_pam_mode->speed_rate=card->channels[i].rate+8;
		} else {
			extended_pam_mode->speed_rate=card->channels[i].rate;
		}

		tunnel_cmd(card, CMD_CFG_GHS_EXTENDED_PAM_MODE, buf, sizeof(*extended_pam_mode)/4, i, &ack);


//////////
		xmii_modify=(struct cmd_xmii_modify *)buf;
		memset(xmii_modify,0,sizeof(*xmii_modify));
		xmii_modify->linkNo=i;
		xmii_modify->speed=MII_100BT;
		xmii_modify->duplex=FULL_DUPLEX;
		xmii_modify->smii_syncmode=NORMAL;
		xmii_modify->altcollision=IFX_DISABLE;
		xmii_modify->rxduringtx=IFX_DISABLE;
		xmii_modify->collisiontype=IFX_DISABLE;

		if (mpi_cmd(card, CMD_xMII_Modify, buf, sizeof(*xmii_modify)/4, &ack)) return -1;


	}

	mdelay(100);

// ----------------------------------------------------
// ----------------------------------------------------

	pmd_control=(struct cmd_pmd_control *)buf;
	memset(pmd_control,0,sizeof(*pmd_control));
	pmd_control->LinkNo=mpair_ref;
	pmd_control->LinkControl=START_TRAINING;
	pmd_control->ActivationState=0x00;

	if (mpi_cmd(card, CMD_PMD_Control, buf, sizeof(*pmd_control)/4, &ack)) return -1;


/*
	for (i = 0; i < 4; i++)
	{
		xmii_modify=(struct cmd_xmii_modify *)buf;
		memset(xmii_modify,0,sizeof(*xmii_modify));
		xmii_modify->linkNo=i;
		xmii_modify->speed=MII_100BT;
		xmii_modify->duplex=FULL_DUPLEX;
		xmii_modify->smii_syncmode=NORMAL;
		xmii_modify->altcollision=IFX_DISABLE;
		xmii_modify->rxduringtx=IFX_DISABLE;
		xmii_modify->collisiontype=IFX_DISABLE;

		if (mpi_cmd(card, CMD_xMII_Modify, buf, sizeof(*xmii_modify)/4, &ack)) return -1;
	}
*/
	if ((mpair_mode == MPAIR01_23) && (mpair_ref != 2))
	{
		mpair_ref = 2;
		goto two_mpair;
	}
	card->mpair = 3;
	card->mpair_mode = mpair_mode;

	PDEBUG(0, "-----------------------------------------\n");

	return 0;
}


int mpair_slave(struct mam17_card *card)
{
	u32 buf[32];
	ack_t ack;
	struct cmd_hdlc_tc_link_linkmodify *linkmodify;
	struct cmd_pmd_spanprofilegroupconfig *spanprofile;
	struct cmd_pmd_control *pmd_control;

	struct cmd_cfg_ghs_mode *ghs_mode;
	struct cmd_cfg_caplist_short_ver_2 *caplist;
	struct cmd_cfg_sdi_tx *sdi_tx;
	struct cmd_cfg_sdi_rx *sdi_rx;
	struct cmd_cfg_ghs_extended_pam_mode *extended_pam_mode;
	struct cmd_xmii_modify *xmii_modify;

	struct cmd_tc_flowmodify *flowmodify;

	int i = 0;

	int mpair_ref = 0;
	int mpair_mode = card->mpair_mode;


	PDEBUG(0, "----------------MPAIR_SLAVE-------------\n""----------------MPAIR_SLAVE-------------\n");

	for (i = 0; i < 4; i++)
	{
		if (!is_chan_in_mpair(card, i)) continue;

		buf[0] = i;
		if (mpi_cmd(card, CMD_PMD_Reset, buf, 1, &ack))/////////////////////////
		{
			PDEBUG(0, "CMD_PMD_Reset error");
			return -1;
		}
	}
	mdelay(100);
/////////// my

	flowmodify=(struct cmd_tc_flowmodify *)buf;
	memset(flowmodify,0,sizeof(*flowmodify));
	flowmodify->dsl0_ts = HDLC_TC_LAYER;
	flowmodify->dsl1_ts = HDLC_TC_LAYER;
	flowmodify->dsl2_ts = HDLC_TC_LAYER;
	flowmodify->dsl3_ts = HDLC_TC_LAYER;

	if (mpi_cmd(card, CMD_TC_FlowModify, buf, sizeof(*flowmodify)/4, &ack))/////////////////////////
	{
		PDEBUG(0, "CMD_TC_FlowModify error");
		return -1;
	}
	mdelay(1000);

//////////

two_mpair:


	linkmodify=(struct cmd_hdlc_tc_link_linkmodify *)buf;
	memset(linkmodify,0,sizeof(*linkmodify));

	linkmodify->linkNo = mpair_ref;
	linkmodify->bitdyte = BIT_STUFFING;
	linkmodify->interframe_ch = card->channels[mpair_ref].fill_7e;
	linkmodify->sharedflags = IFX_ENABLE;
	linkmodify->fcs = IFX_DISABLE;
	linkmodify->acf_insert = IFX_DISABLE;
	linkmodify->txaddrctrl = 0xff03;
	linkmodify->li_m_pairports = mpair_mode;
	linkmodify->clockingmode = NO_CLK;

	if (mpi_cmd(card, CMD_HDLC_TC_LinkModify, buf, sizeof(*linkmodify)/4, &ack))/////////////////////////
	{
		PDEBUG(0, "CMD_HDLC_TC_LinkModify error");
		return -1;
	}

	spanprofile=(struct cmd_pmd_spanprofilegroupconfig *)buf;
	memset(spanprofile,0,sizeof(*spanprofile));

	spanprofile->LinkNo = mpair_ref;
	spanprofile->wireinterface = TWO_WIRE;
	spanprofile->minlinerate = 256000;
	spanprofile->maxlinerate = 15196000;//5696000;
	spanprofile->minlinesubrat = 0x0;
	spanprofile->maxlinesubrat = 0x0;
	spanprofile->psd = SYMETRIC;
	spanprofile->transmod = card->channels[0].annex;
	spanprofile->remoteenabled = EOC_ENABLED_IDC;
	spanprofile->powerfeeding = NO_POWER;
	spanprofile->cc_targetmargindown = 0x00;
	spanprofile->wc_targetmargindown = 0x00;
	spanprofile->cc_targetmarginup = 0x00;
	spanprofile->wc_targetmarginup = 0x00;
	spanprofile->usedtargetmargins = 0x00;
	spanprofile->refcloc = DATA_OR_NETWORK_CLK;
	spanprofile->lineprobe = LP_DISABLE;
	spanprofile->pam_constellation = SELECT_32_PAM;
	spanprofile->capliststyl = NEW_STYLE_CAPLIST;
	spanprofile->pbo_mode = card->channels[mpair_ref].pbo_mode;
	if (card->channels[mpair_ref].pbo_mode)
		spanprofile->pbo_mode = PBO_FORCED;
	if (card->channels[mpair_ref].pbo_mode == PWRBO_NORMAL)
		spanprofile->epl_mode = EPL_ENABLED;
	else
		spanprofile->epl_mode = EPL_DISABLED;
	spanprofile->pbo_valu = card->channels[mpair_ref].pbo_vals[mpair_ref];

	if (mpi_cmd(card, CMD_PMD_SpanProfileGroupConfig, buf, sizeof(*spanprofile)/4, &ack))/////////////////////////
	{
		PDEBUG(0, "CMD_PMD_SpanProfileGroupConfig error");
		return -1;
	}

	pmd_control=(struct cmd_pmd_control *)buf;
	memset(pmd_control,0,sizeof(*pmd_control));

	pmd_control->LinkNo = mpair_ref;
	pmd_control->LinkControl = FORCE_LINK_DOWN;
	pmd_control->ActivationState = STOP_AFTER_INIT;

	if (mpi_cmd(card, CMD_PMD_Control, buf, sizeof(*pmd_control)/4, &ack))/////////////////////////
	{
		PDEBUG(0, "CMD_PMD_Control error");
		return -1;
	}
	mdelay(100);


// ----------------Extended PMD configuration----------
// ----------------------------------------------------

	for (i = 0; i < 4; i++)
	{
		if (!is_chan_in_mpair(card, i)) continue;

		ghs_mode = (struct cmd_cfg_ghs_mode *)buf;
		memset(ghs_mode, 0, sizeof(*ghs_mode));

		ghs_mode->transaction = GHS_TRNS_00;
		ghs_mode->startup_initialization=STARTUP_FAREND;
		ghs_mode->pbo_mode = PBO_NORMAL;
		ghs_mode->epl_mode = EPL_ENABLED;
		ghs_mode->pmms_margin_mode = PMMS_NORMAL;

		tunnel_cmd(card, CMD_CFG_GHS_MODE, buf, sizeof(*ghs_mode)/4, i, &ack);

		caplist = (struct cmd_cfg_caplist_short_ver_2 *)buf;
		memset(caplist, 0, sizeof(*caplist));

		caplist->clock_mode = SHDSL_CLK_MODE_1|SHDSL_CLK_MODE_2|SHDSL_CLK_MODE_3a;
		caplist->annex = 3;//ANNEX_A_B;
		caplist->pow_backoff = 0;
		caplist->psd_mask = 0x00;

		caplist->base_rate_min = 192;
		caplist->base_rate_max = 2304;
		caplist->base_rate_min16 = 2368;
		caplist->base_rate_max16 = 3840;
		caplist->base_rate_min32 = 768;
		caplist->base_rate_max32 = 5696;

		caplist->sub_rate_min = 0x00;
		caplist->sub_rate_max = 0x00;
		caplist->enable_pmms = PMMS_OFF;
		caplist->pmms_margin = 0x00;

		tunnel_cmd(card, CMD_CFG_CAPLIST_SHORT_VER_2, buf, sizeof(*caplist)/4, i, &ack);

		sdi_rx = (struct cmd_cfg_sdi_rx*)buf;
		memset(sdi_rx, 0, sizeof(*sdi_rx));

		sdi_rx->data_shift = 0x00;
		sdi_rx->frame_shift = 0x00;
		sdi_rx->sp_level = SDI_HIGH;
		sdi_rx->driving_edg = SDI_RISING;
		sdi_rx->data_shift_edg = SDI_NO;
		sdi_rx->lstwr_1strd_dly = 0x64;
		sdi_rx->slip_mode = SLIP_FAST;
		sdi_rx->align = SDI_NO;

		tunnel_cmd(card, CMD_CFG_SDI_RX, buf, sizeof(*sdi_rx)/4, i, &ack);

		sdi_tx = (struct cmd_cfg_sdi_tx*)buf;
		memset(sdi_tx, 0, sizeof(*sdi_tx));
		sdi_tx->data_shift = 0x00;
		sdi_tx->frame_shift = 0x00;
		sdi_tx->sp_level = SDI_HIGH;
		sdi_tx->sp_sample_edg = SDI_FALLING;
		sdi_tx->data_sample_edg = SDI_FALLING;
		sdi_tx->lstwr_1strd_dly = 0x16;
		sdi_tx->slip_mode = SLIP_FAST;
		sdi_tx->align = SDI_NO;

		tunnel_cmd(card, CMD_CFG_SDI_TX, buf, sizeof(*sdi_tx)/4, i, &ack);

		extended_pam_mode=(struct cmd_cfg_ghs_extended_pam_mode*)buf;
		memset(extended_pam_mode,0,sizeof(*extended_pam_mode));

		extended_pam_mode->ext_pam_mode=0x1;
		extended_pam_mode->bits_per_symbol=TCPAM128;
		extended_pam_mode->speed_rate=12680;

		tunnel_cmd(card, CMD_CFG_GHS_EXTENDED_PAM_MODE, buf, sizeof(*extended_pam_mode)/4, i, &ack);
	}

	mdelay(100);

// ----------------------------------------------------
// ----------------------------------------------------

	pmd_control=(struct cmd_pmd_control *)buf;
	memset(pmd_control,0,sizeof(*pmd_control));
	pmd_control->LinkNo=mpair_ref;
	pmd_control->LinkControl=START_TRAINING;
	pmd_control->ActivationState=0x00;

	if (mpi_cmd(card, CMD_PMD_Control, buf, sizeof(*pmd_control)/4, &ack)) return -1;




/*
	buf[0] = 0;
	buf[1] = 1;
	buf[2] = 10*1024*1024;//speed
	buf[3] = 6;//tcpam

	mpi_cmd(card, 0x65, buf, 4, &ack);

	pmd_control=(struct cmd_pmd_control *)buf;
	memset(pmd_control,0,sizeof(*pmd_control));//////////////////////------

	pmd_control->LinkNo = 0;
	pmd_control->LinkControl = FORCE_LINK_DOWN;
	pmd_control->ActivationState = START_AFTER_INIT;

	if (mpi_cmd(card, CMD_PMD_Control, buf, sizeof(*pmd_control)/4, &ack))/////////////////////////-------
	{
		PDEBUG(0, "CMD_PMD_Control error");
		return -1;
	}
	mdelay(100);
*/


	for (i = 0; i < 4; i++)
	{
		xmii_modify=(struct cmd_xmii_modify *)buf;
		memset(xmii_modify,0,sizeof(*xmii_modify));
		xmii_modify->linkNo=i;
		xmii_modify->speed=MII_100BT;
		xmii_modify->duplex=FULL_DUPLEX;
		xmii_modify->smii_syncmode=NORMAL;
		xmii_modify->altcollision=IFX_DISABLE;
		xmii_modify->rxduringtx=IFX_DISABLE;
		xmii_modify->collisiontype=IFX_DISABLE;

		if (mpi_cmd(card, CMD_xMII_Modify, buf, sizeof(*xmii_modify)/4, &ack)) return -1;
	}

	if ((mpair_mode == MPAIR01_23) && (mpair_ref != 2))
	{
		mpair_ref = 2;
		goto two_mpair;
	}

	card->mpair = 3;
	card->mpair_mode = mpair_mode;

	PDEBUG(0, "----------------------------------------\n");

	return 0;
}
