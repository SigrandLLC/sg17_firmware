#include "mam17h_main.h"
#include "mam17h_debug.h"
#include "mam17h_sysfs.h"
#include "mam17h_socrate.h"
#include "mam17h_pi.h"
#include "mam17h_net.h"
#include "mam17h_mpair.h"

#define MAX_MSG_BUF 100
#define NUM_IFACES 4
#define DRIVER_VERSION "1.0"

MODULE_DESCRIPTION ( "Driver for shdsl modules platform SG-17S. Version "DRIVER_VERSION"\n" );
MODULE_AUTHOR ( "Scherbakov Mihail (scherbakovmihail@sigrand.ru)\n" );
MODULE_LICENSE ( "GPL" );
MODULE_VERSION ( DRIVER_VERSION );

static int __devinit mam17_probe_one(struct pci_dev *pdev, const struct pci_device_id *dev_id);
static void __devexit mam17_remove_one(struct pci_dev *pdev);

int configure_channel(struct mam17_card *card, int ch)
{
	u32 buf[32];
	ack_t ack;
	int i = 0;
	struct cmd_pmd_spanprofilegroupconfig *spanprofile;
	struct cmd_cfg_ghs_mode *ghs_mode;
	struct cmd_cfg_caplist_short_ver_2 *caplist;
	struct cmd_cfg_sdi_tx *sdi_tx;
	struct cmd_cfg_sdi_rx *sdi_rx;
	struct cmd_cfg_ghs_extended_pam_mode *extended_pam_mode;
	struct cmd_pmd_control *pmd_control;

	buf[0] = ch;
	if (mpi_cmd(card, CMD_PMD_Reset, buf, 1, &ack)) return -1;
	if (card->channels[ch].on != 1) {
		return 0;
	}
	i = 0;
	while ((card->channels[ch].state != DOWN_READY) && (i < 10000)) i++;
	mdelay(100);

	if (card->channels[ch].mode == MASTER)
	{
		buf[0] = ch;
		if (mpi_cmd(card, CMD_PMD_CO_PortSubTypeSelect, buf, 1, &ack)) return -1;
		mdelay(100);

		spanprofile=(struct cmd_pmd_spanprofilegroupconfig *)buf;
		memset(spanprofile,0,sizeof(*spanprofile));
		spanprofile->LinkNo=ch;
		spanprofile->wireinterface=TWO_WIRE;
		spanprofile->minlinerate=256000;
		spanprofile->maxlinerate=5696000;
		spanprofile->minlinesubrat=0x0;
		spanprofile->maxlinesubrat=0x0;
		spanprofile->psd=SYMETRIC;
		spanprofile->transmod=card->channels[ch].annex;
		spanprofile->remoteenabled=EOC_ENABLED_IDC;
		spanprofile->powerfeeding=NO_POWER;
		spanprofile->cc_targetmargindown=0x00;
		spanprofile->wc_targetmargindown=0x00;
		spanprofile->cc_targetmarginup=0x00;
		spanprofile->wc_targetmarginup=0x00;
		spanprofile->usedtargetmargins=0x00;
		spanprofile->refcloc=DATA_OR_NETWORK_CLK;
		spanprofile->lineprobe=LP_DISABLE;
		spanprofile->pam_constellation=SELECT_32_PAM;
		spanprofile->capliststyl=NEW_STYLE_CAPLIST;
		spanprofile->pbo_mode = card->channels[ch].pbo_mode;
		if (card->channels[ch].pbo_mode)
		spanprofile->pbo_mode = PBO_FORCED;
		if (card->channels[ch].pbo_mode == PWRBO_NORMAL)
			spanprofile->epl_mode=EPL_ENABLED;
		else 
			spanprofile->epl_mode=EPL_DISABLED;
		spanprofile->pbo_valu = card->channels[ch].pbo_vals[0];

		if (mpi_cmd(card, CMD_PMD_SpanProfileGroupConfig, buf, sizeof(*spanprofile)/4, &ack)) return -1;
	}	

	mdelay(200);

	pmd_control=(struct cmd_pmd_control *)buf;
	memset(pmd_control,0,sizeof(*pmd_control));
	pmd_control->LinkNo = ch;
	if (card->channels[ch].mode == MASTER) {
		pmd_control->LinkControl = ENABLE_LINK;
	} else {
		pmd_control->LinkControl = FORCE_LINK_DOWN;
	}
	pmd_control->ActivationState = STOP_AFTER_INIT;
	if (mpi_cmd(card, CMD_PMD_Control, buf, sizeof(*pmd_control)/4, &ack)) return -1;

	mdelay(200);

// ----------------Extended PMD configuration----------
// ----------------------------------------------------

	ghs_mode = (struct cmd_cfg_ghs_mode *)buf;
	memset(ghs_mode, 0, sizeof(*ghs_mode));
	if (card->channels[ch].mode == MASTER)
	{
		ghs_mode->transaction = GHS_TRNS_10;
		ghs_mode->startup_initialization=STARTUP_LOCAL;
		switch (card->channels[ch].pbo_mode)
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
	} else {
		ghs_mode->transaction = GHS_TRNS_00;
		ghs_mode->startup_initialization=STARTUP_FAREND;
		ghs_mode->pbo_mode = PBO_NORMAL;
		ghs_mode->epl_mode = EPL_ENABLED;
	}
	
	ghs_mode->pmms_margin_mode = PMMS_NORMAL;
	tunnel_cmd(card, CMD_CFG_GHS_MODE, buf, sizeof(*ghs_mode)/4, ch, &ack);

	caplist = (struct cmd_cfg_caplist_short_ver_2 *)buf;
	memset(caplist, 0, sizeof(*caplist));
	if (card->channels[ch].mode == MASTER)
	{
		switch (card->channels[ch].clkmode)
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
		if (card->channels[ch].pbo_mode == PWRBO_FORCED)
		{
			if (card->channels[ch].pbo_vnum)
				caplist->pow_backoff = card->channels[ch].pbo_vals[0];
			else
				caplist->pow_backoff = 0;
		}
		caplist->annex = card->channels[ch].annex + 1;
	} else {
		caplist->clock_mode = SHDSL_CLK_MODE_1|SHDSL_CLK_MODE_2|SHDSL_CLK_MODE_3a;
		caplist->annex = ANNEX_AB;
		caplist->pow_backoff = 0;
	}
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

	tunnel_cmd(card, CMD_CFG_CAPLIST_SHORT_VER_2, buf, sizeof(*caplist)/4, ch, &ack);

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

	tunnel_cmd(card, CMD_CFG_SDI_RX, buf, sizeof(*sdi_rx)/4, ch, &ack);

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

	tunnel_cmd(card, CMD_CFG_SDI_TX, buf, sizeof(*sdi_tx)/4, ch, &ack);

	extended_pam_mode=(struct cmd_cfg_ghs_extended_pam_mode*)buf;
	memset(extended_pam_mode,0,sizeof(*extended_pam_mode));
	extended_pam_mode->ext_pam_mode=0x1;
	if (card->channels[ch].mode != MASTER) {
		extended_pam_mode->bits_per_symbol=TCPAM128;
		extended_pam_mode->speed_rate=12680;
	} else {
		extended_pam_mode->bits_per_symbol=card->channels[ch].tcpam;
		if(card->channels[ch].tcpam==TCPAM128){
			extended_pam_mode->speed_rate=card->channels[ch].rate+8;
		} else {
			extended_pam_mode->speed_rate=card->channels[ch].rate;
		}
	}

	tunnel_cmd(card, CMD_CFG_GHS_EXTENDED_PAM_MODE, buf, sizeof(*extended_pam_mode)/4, ch, &ack);

	mdelay(100);

// ----------------------------------------------------
// ----------------------------------------------------

	pmd_control=(struct cmd_pmd_control *)buf;
	memset(pmd_control,0,sizeof(*pmd_control));
	pmd_control->LinkNo=ch;
	pmd_control->LinkControl=START_TRAINING;
	pmd_control->ActivationState=0x00;

	if (mpi_cmd(card, CMD_PMD_Control, buf, sizeof(*pmd_control)/4, &ack)) return -1;

/*
	PDEBUG(0, "*****TC loop:");
	buf[0] = ch;
	buf[1] = 3;
	if (mpi_cmd(card, CMD_TC_LayerLoopControl, buf, 2, , &ack)) return -1;
	PDEBUG(0, "*****TC loop OK!");

	buf[0] = ch;
	buf[1] = IFX_ENABLE;
	if (mpi_cmd(card, CMD_HDLC_TC_LinkCorruptPacketControl, buf, 2, , &ack)) return -1;
*/


//	buf[0] = ch;
//	buf[1] = IFX_ENABLE;
//	buf[2] = IFX_ENABLE;
//	if (mpi_cmd(card, CMD_LinkControl, buf, 3, &ack)) return -1;

// chip info
//	if (mpi_cmd(card, 0x110, buf, 0, &ack)) return -1;


//	PDEBUG(0, "*****state machine disable");
//	buf[0] = ch;
//	buf[1] = IFX_DISABLE;
//	buf[2] = IFX_DISABLE;
//	buf[3] = IFX_DISABLE;
//	if (mpi_cmd(card, 0x169, buf, 4, &ack)) return -1;

/*
	PDEBUG(0, "*****MII loop:");
	buf[0] = ch;
	buf[1] = 2; // ingress(from line to line) (2 egress)
	if (mpi_cmd(card, CMD_SystemInterfaceLoopControl, buf, 2, &ack)) return -1;
*/
/*
	PDEBUG(0, "*****MII loop OK!");
	buf[0] = ch;
	buf[1] = IFX_ENABLE;
	if (mpi_cmd(card, 0x140, buf, 2, &ack)) return -1;
*/

	return 0;
}

void def_conf(struct mam17_card *card, int ch)
{
	card->channels[ch].mode = SLAVE;
	card->channels[ch].annex = ANNEX_A;
	card->channels[ch].rate = 15296;
	card->channels[ch].tcpam = TCPAM128;
	card->channels[ch].pbo_mode = PWRBO_NORMAL;

	card->channels[ch].pbo_vnum = 0;
	card->channels[ch].crc16 = 0;
	card->channels[ch].fill_7e = 0xFF;
	card->channels[ch].clkmode = 1;

	card->channels[ch].on = 1;

	card->mpair_mode = 0;
}

int load_cfg(struct mam17_card *card, int ch)
{
	u32 buf[32];
	message_t *msg;
	ack_t ack;
	struct ack_dsl_param_get *dsl_param;
	struct channel *chan = &(card->channels[ch]);
	struct cmd_ghs_cap_get *cmd_cap_get;
	struct ack_ghs_cap_get *ack_cap_get;

	tunnel_cmd(card, CMD_DSL_PARAM_GET, buf, 0, ch, &ack);
	msg = (message_t *)ack.buf32;
	
	dsl_param = (struct ack_dsl_param_get *)&(ack.buf32[2]);
	
	if (dsl_param->stu_mode == 1) chan->mode = MASTER;
	else if (dsl_param->stu_mode == 2) chan->mode = SLAVE;
	else chan->mode = -1;
	if (chan->mode == SLAVE) chan->annex = dsl_param->annex - 1;

	chan->tcpam = dsl_param->bits_p_symbol;
	chan->rate = dsl_param->base_rate;

	cmd_cap_get = (struct cmd_ghs_cap_get *)buf;
	memset(cmd_cap_get, 0, sizeof(* cmd_cap_get));
	cmd_cap_get->ClType = 1;

	if (chan->annex == ANNEX_A) {
		cmd_cap_get->ClParam = TPS_TC_A;
	} else {
		cmd_cap_get->ClParam = TPS_TC_B;
	}
	
	tunnel_cmd(card, CMD_GHS_CAP_GET, buf, 2, ch, &ack);

	ack_cap_get = (struct ack_ghs_cap_get *)&(ack.buf32[2]);

	switch (ack_cap_get->ClData[0])
	{
	case SHDSL_CLK_MODE_1:
		chan->clkmode = 0;  // plesiochronous
		break;
	case SHDSL_CLK_MODE_2:
		chan->clkmode = 2;  // plesiochronous with timing reference
		break;
	case SHDSL_CLK_MODE_3a:
		chan->clkmode = 1;  // synchronous
		break;
	}

	
//	PDEBUG(debug_load_cfg, "len = %i MSGID = %x", ack.len, msg->MSGID);
//	PDEBUG(debug_load_cfg, "mode = %x annex = %x tcpam = %x rate = %x", dsl_param->stu_mode, dsl_param->annex, 
//										dsl_param->bits_p_symbol, dsl_param->base_rate);
	
	return 0;
}

int get_statistic(u8 ch, struct mam17_card * card, struct statistics *stat)
{
	ack_t ack;

//	debug_recv = 0;
	tunnel_cmd(card, CMD_PERF_STATUS_GET, NULL, 0, ch, &ack);
//	debug_recv = 40;
	memcpy(stat,&(ack.buf32[2]), sizeof(*stat));
	return 0;
}


static struct pci_device_id mam17_pci_tbl[] __devinitdata = {
	{ PCI_DEVICE(MAM17H_PCI_VENDOR, MAM17H_PCI_DEVICE) },
	{ 0 }
};
MODULE_DEVICE_TABLE(pci, mam17_pci_tbl);
static struct pci_driver mam17_driver = {
 name:           MAM17H_MODNAME,
 probe:          mam17_probe_one,
 remove:         mam17_remove_one,
 id_table:       mam17_pci_tbl
};

int card_number = 0; // number of card's in system

static int __devinit mam17_init_card(struct mam17_card *card)
{
	unsigned long iomem_start = pci_resource_start(card->pdev, 0);
	unsigned long iomem_end = pci_resource_end(card->pdev, 0);
	int error = 0;

	// set card name
	sprintf(card->name, "mam17card%i", card->number);
	
	if ((iomem_end - iomem_start) != MAM17_IOMEM_SIZE - 1) return -ENODEV;
	if (!request_mem_region(iomem_start, MAM17_IOMEM_SIZE, card->name))
	{
		release_mem_region(iomem_start, MAM17_IOMEM_SIZE);
		if (!request_mem_region(iomem_start, MAM17_IOMEM_SIZE, card->name))
		{
			printk(KERN_NOTICE "%s: error requets mem region (%lx-%lx)\n", MAM17H_MODNAME, iomem_start, iomem_end);
			return -EBUSY;
		}
	}
	card->mem_base = (void *)ioremap(iomem_start, MAM17_IOMEM_SIZE);
	if (card->mem_base == NULL)
	{
		error = -ENODEV;
		goto err2;
	}
	card->regs = card->mem_base;
	card->mpair = 0;
	card->state = 0;

	switch (card->pdev->subsystem_device)
	{
		case 0:
			card->if_num = NUM_IFACES;
			card->pwr_source = 1;
		break;
		case 4:
			card->if_num = 4;
			card->pwr_source = 0;
		break;
		case 5:
			card->if_num = 4;
			card->pwr_source = 1;
		break;
		default:
			printk(KERN_NOTICE "%s: error hardware PCI Subsystem ID for module\n", MAM17H_MODNAME);
			error = -ENODEV;
			goto err1;
	}
	error = mam17_socrate_init(card);
	if (error)
	{
		PDEBUG(0, "socrate init faild");
		goto err1;
	}

	return 0;

err2:
	iounmap(card->mem_base);
err1:
	release_mem_region(iomem_start, MAM17_IOMEM_SIZE);
	return error;	

}

static int __devinit mam17_probe_one(struct pci_dev * pdev, const struct pci_device_id * dev_id) {
	struct mam17_card *card = NULL;
	int error = 0;
	int i, ch;

	struct cmd_tc_flowmodify *flowmodify;
	struct cmd_hdlc_tc_link_linkmodify *linkmodify ;
	struct cmd_xmii_modify *xmii_modify;
	ack_t ack;
	u32 buf[32];

	PDEBUG(debug_init, "New device (num = %i)", card_number);

	if (pci_enable_device(pdev))
	{
		error = -ENODEV;
		goto err1;
	}
//	pci_set_master(pdev);

	card = kmalloc(sizeof(struct mam17_card), GFP_KERNEL);
	if (card == NULL)
	{
		printk(KERN_NOTICE "%s: Cannot allocate kernel memory\n", MAM17H_MODNAME);
		error = -ENODEV;
		goto err1;
	}
	memset((void*)card, 0, sizeof(struct mam17_card));
	pci_set_drvdata(pdev, card);
	card->number = card_number++;
	card->pdev = pdev;

	error = mam17_init_card(card);
	if (error)
	{
		PDEBUG(debug_error, "init card faild");
		goto err1;
	}
	PDEBUG(debug_init, "init card OK");

	error = mam17_net_init(card);
	if (error)
	{
		PDEBUG(debug_error, "net_init Error!");
		goto err1;
	}
	PDEBUG(debug_init, "net_init OK");
	
	if (card->state == 0)
	{
		ack_wait = 0x680;
		if (!interruptible_sleep_on_timeout(&card->wait, 50000))
		{
			printk(KERN_NOTICE "to fast!!!!!\n");
		}
	}

	for (ch = 0; ch < card->if_num; ch++)
	{
		def_conf(card, ch);

	flowmodify=(struct cmd_tc_flowmodify *)buf;
	memset(flowmodify,0,sizeof(*flowmodify));
	flowmodify->dsl0_ts=HDLC_TC_LAYER;
	flowmodify->dsl1_ts=HDLC_TC_LAYER;
	flowmodify->dsl2_ts=HDLC_TC_LAYER;
	flowmodify->dsl3_ts=HDLC_TC_LAYER;

	mpi_cmd(card, CMD_TC_FlowModify, buf, sizeof(*flowmodify)/4, &ack);
	
	mdelay(10);

	linkmodify=(struct cmd_hdlc_tc_link_linkmodify *)buf;
	memset(linkmodify,0,sizeof(*linkmodify));
	linkmodify->linkNo=ch;
	linkmodify->bitdyte=BIT_STUFFING;
	linkmodify->interframe_ch=card->channels[ch].fill_7e;
	linkmodify->sharedflags=IFX_ENABLE;
	linkmodify->fcs=IFX_DISABLE;
	linkmodify->acf_insert=IFX_DISABLE;
	linkmodify->txaddrctrl=0xff03;
	linkmodify->li_m_pairports=card->mpair_mode;
	linkmodify->clockingmode=NO_CLK;

	mpi_cmd(card, CMD_HDLC_TC_LinkModify, buf, sizeof(*linkmodify)/4, &ack);

	mdelay(10);

	xmii_modify=(struct cmd_xmii_modify *)buf;
	memset(xmii_modify,0,sizeof(*xmii_modify));
	xmii_modify->linkNo=ch;
	xmii_modify->speed=MII_100BT;
	xmii_modify->duplex=FULL_DUPLEX;
	xmii_modify->smii_syncmode=NORMAL;
	xmii_modify->altcollision=IFX_DISABLE;
	xmii_modify->rxduringtx=IFX_DISABLE;
	xmii_modify->collisiontype=IFX_DISABLE;
	
	mpi_cmd(card, CMD_xMII_Modify, buf, sizeof(*xmii_modify)/4, &ack);

	mdelay(10);
	
	}

	for (i = 0; i < card->if_num; i++)
		configure_channel(card, i);

	PDEBUG(debug_init, "Configure - OK\n");
//	printk(KERN_NOTICE "Card %i - OK\n", card->number);


/*
	card->mpair_mode = MPAIR0321;
	if (card->number == 0)
	{
		if (mpair_slave(card))
		{
			PDEBUG(0, "mpair_slave - faild!");
			goto error;
			return -1;
		}
		PDEBUG(debug_init, "mpair_slave - OK");
	}

	if (card->number == 1)
	{
		if (mpair_master(card))
		{
			PDEBUG(0, "mpair_master - faild!");
			goto error;
			return -1;
		}
		PDEBUG(debug_init, "mpair_master - OK");
		
//		int buf32[32];
//		ack_t ack;
//		buf32[0] = 0;
//		buf32[1] = 3;
//		if (mpi_cmd(card, CMD_TC_LayerLoopControl, buf32, 2, &ack)) return -1;

	}



//	for (i = 0; i < 4; i++)
//	{
//		int buf32[32];
//		ack_t ack;

//		buf32[0] = i;
//		buf32[1] = IFX_ENABLE;
	
//		if (mpi_cmd(card, CMD_HDLC_TC_LinkCorruptPacketControl, buf32, 2, &ack)) return -1;
		
//	}
*/
	schedule_delayed_work(&(card->work), 2*HZ);

	return 0;

err1:
	pci_disable_device(pdev);
	kfree(card);
	return error;
}

void mam17_card_remove(struct mam17_card *card)
{
	struct device *dev_dev = (struct device*)&(card->pdev->dev);
	struct device_driver *dev_drv = (struct device_driver*)(dev_dev->driver);
	unsigned long iomem_start = pci_resource_start(card->pdev, 0);
	int i;
	
	for (i = 0; i < card->if_num; i++)
	{
		mam17_sysfs_remove(card->ndevs[i]);
		sysfs_remove_link(&(dev_drv->kobj), card->ndevs[i]->name);
		unregister_netdev(card->ndevs[i]);
		free_netdev(card->ndevs[i]);
	}


	PDEBUG(debug_remove, "IRQ %i", card->pdev->irq);
	free_irq(card->pdev->irq, (void*)card);
//	PDEBUG(debug_remove, "IRQ %i (%p)", card->pdev->irq, card);

	cancel_delayed_work(&(card->work));
	PDEBUG(debug_remove, "cancel_delayed_work: OK\n");
//	device_remove_file(&(pdev->dev), &dev_attr_regs);
//	device_remove_file(&(pdev->dev), &dev_attr_param);
//	attr_files_remove(pdev);
	PDEBUG(debug_remove, "device_remove_file: OK\n");
	release_mem_region(iomem_start, MAM17_IOMEM_SIZE);
	PDEBUG(debug_remove, "release_mem_region: OK\n");
	if (card->mem_base) iounmap(card->mem_base);
	PDEBUG(debug_remove, "iounmap: OK\n");
	PDEBUG(debug_remove, "module removed\n");

	pci_disable_device(card->pdev);
	pci_set_drvdata(card->pdev, NULL);
	kfree(card);

}

static void __devexit mam17_remove_one(struct pci_dev *pdev)
{
	// если probe_one вернула -1 эта функция не вызовется
	struct mam17_card *card = pci_get_drvdata(pdev);
	mam17_card_remove(card);
}


int __devinit mam17_init (void)
{
	printk(KERN_NOTICE "Load "MAM17H_MODNAME" driver\n");
	pci_register_driver(&mam17_driver);
	return 0;	
}
void __devexit mam17_exit (void)
{
	printk(KERN_NOTICE "Unload "MAM17H_MODNAME" driver\n");
	pci_unregister_driver(&mam17_driver);
}

module_init(mam17_init);
module_exit(mam17_exit);
