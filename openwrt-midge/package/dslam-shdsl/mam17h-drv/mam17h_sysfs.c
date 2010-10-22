#include "mam17h_sysfs.h"
#include "mam17h_net.h"
#include "mam17h_main.h"
#include "mam17h_debug.h"
#include "mam17h_socrate.h"
#include "mam17h_pi.h"

#define to_net_dev(class) container_of(class, struct net_device, class_dev)

static ssize_t show_link_state(struct class_device *cdev, char *buf) 
{                                                                       
	struct net_device *ndev = to_net_dev(cdev);
	struct net_local *nl = (struct net_local *)netdev_priv(ndev);
	struct channel *chan = (struct channel *)nl->chan_cfg;

	if (chan->state == CONNECTED)
	{
		return snprintf(buf,PAGE_SIZE,"1");
	} else
		return snprintf(buf,PAGE_SIZE,"0");
}
static CLASS_DEVICE_ATTR(link_state,0444,show_link_state,NULL);

// Mode control (master/slave)
static ssize_t show_mode(struct class_device *cdev, char *buf) 
{                                                                       
	struct net_device *ndev = to_net_dev(cdev);
	struct net_local *nl = (struct net_local *)netdev_priv(ndev);
	if (nl->chan_cfg->mode == MASTER)
		return snprintf(buf,PAGE_SIZE,"master");
	else if (nl->chan_cfg->mode == SLAVE)
		return snprintf(buf,PAGE_SIZE,"slave");
	else
		return snprintf(buf,PAGE_SIZE,"unknown role");	
}

static ssize_t store_mode(struct class_device *cdev,const char *buf, size_t size)
{
	struct net_device *ndev = to_net_dev(cdev);
	struct net_local *nl = (struct net_local *)netdev_priv(ndev);
	struct channel *chan = (struct channel *)nl->chan_cfg;

	if (size > 0)
	{
		if (buf[0] == '0')
		{
			chan->mode = SLAVE;
			chan->annex = ANNEX_A_B;
		} else if (buf[0] == '1')
		{
			chan->mode = MASTER;
		}	    
    }    
    return size;
}
static CLASS_DEVICE_ATTR(mode,0644,show_mode,store_mode);

// Annex control 
static ssize_t show_annex(struct class_device *cdev, char *buf) 
{                                                                       
	struct net_device *ndev = to_net_dev(cdev);
	struct net_local *nl = (struct net_local *)netdev_priv(ndev);
	struct channel *chan = (struct channel *)nl->chan_cfg;

	if ((chan->state != CONNECTED) && (chan->mode != MASTER)) return 0;

	switch (chan->annex)
	{
	case ANNEX_A:
		return snprintf(buf,PAGE_SIZE,"A");
	case ANNEX_B:
		return snprintf(buf,PAGE_SIZE,"B");
	case ANNEX_A_B:
		return snprintf(buf,PAGE_SIZE,"AB");
	default:
		return snprintf(buf,PAGE_SIZE,"unknown annex");
	}	
}
static ssize_t store_annex(struct class_device *cdev,const char *buf, size_t size)
{
	struct net_device *ndev = to_net_dev(cdev);
	struct net_local *nl = netdev_priv(ndev);
	struct channel *chan = (struct channel *)nl->chan_cfg;

	if (!size) return size;
	
	switch (buf[0])
	{
	case '0':
		chan->annex = ANNEX_A;
		break;
	case '1':
		chan->annex = ANNEX_B;
		break;
	case '2':
		chan->annex = ANNEX_A_B;
		break;
	}
	return size;
}
static CLASS_DEVICE_ATTR(annex, 0644 ,show_annex,store_annex);

// Rate control
static ssize_t show_rate(struct class_device *cdev, char *buf) 
{                                                                       
	struct net_device *ndev = to_net_dev(cdev);
	struct net_local *nl = netdev_priv(ndev);
	struct channel *chan = (struct channel *)nl->chan_cfg;
	
	if ((chan->state != CONNECTED) && (chan->mode != MASTER)) return 0;
	
	return snprintf(buf,PAGE_SIZE,"%d",chan->rate);
}
static ssize_t store_rate(struct class_device *cdev,const char *buf, size_t size)
{
	struct net_device *ndev = to_net_dev(cdev);
	struct net_local *nl = netdev_priv(ndev);
	struct channel *chan = (struct channel *)nl->chan_cfg;
	char *endp;
	u16 tmp;
	
	// check parameters
	if (!size) return size;

	tmp = simple_strtoul(buf,&endp,0);
	if (!tmp)
		return size;

	if (tmp > MAX_RATE) tmp = MAX_RATE;
	// Modulo 64 Kbps!
	tmp /= 64;
	chan->rate = tmp*64;
	return size;
}
static CLASS_DEVICE_ATTR(rate, 0644 ,show_rate,store_rate);

// TCPAM control
static ssize_t show_tcpam(struct class_device *cdev, char *buf) 
{                                                                       
	struct net_device *ndev = to_net_dev(cdev);
	struct net_local *nl = (struct net_local *)netdev_priv(ndev);
	struct channel *chan = (struct channel *)nl->chan_cfg;

	if ((chan->state != CONNECTED) && (chan->mode != MASTER)) return 0;
	
	switch (chan->tcpam)
	{
	case TCPAM4:
		return snprintf(buf,PAGE_SIZE,"TCPAM4");
	case TCPAM8:
		return snprintf(buf,PAGE_SIZE,"TCPAM8");
 	case TCPAM16:
 		return snprintf(buf,PAGE_SIZE,"TCPAM16");
 	case TCPAM32:
 		return snprintf(buf,PAGE_SIZE,"TCPAM32");
	case TCPAM64:
		return snprintf(buf,PAGE_SIZE,"TCPAM64");
	case TCPAM128:
		return snprintf(buf,PAGE_SIZE,"TCPAM128");
 	default:
 		return snprintf(buf,PAGE_SIZE,"unknown TCPAM");
	}
}
static ssize_t store_tcpam(struct class_device *cdev, const char *buf, size_t size)
{
	struct net_device *ndev = to_net_dev(cdev);
	struct net_local *nl = netdev_priv(ndev);
	struct channel *chan = (struct channel *)nl->chan_cfg;
	u8 tmp;

	// if interface is up 
	if (!size)	return size;
	tmp = buf[0]-'0';

	if ((tmp > 6) || (tmp <=0)) return size;
	chan->tcpam = tmp;
	return size;
}
static CLASS_DEVICE_ATTR(tcpam, 0644 ,show_tcpam,store_tcpam);

// POWER Backoff control
static ssize_t show_pbo_mode(struct class_device *cdev, char *buf) 
{                                                                       
	struct net_device *ndev = to_net_dev(cdev);
	struct net_local *nl = (struct net_local *)netdev_priv(ndev);
	struct channel *chan = (struct channel *)nl->chan_cfg;
	
	if (chan->mode != MASTER)
	{
		return snprintf(buf,PAGE_SIZE,"Normal");
	}

	switch (chan->pbo_mode)
	{
	case PWRBO_NORMAL:
		return snprintf(buf,PAGE_SIZE,"Normal");
	case PWRBO_FORCED:
		return snprintf(buf,PAGE_SIZE,"Forced");
	}
	return snprintf(buf,PAGE_SIZE,"Unknown");
}
static ssize_t store_pbo_mode( struct class_device *cdev,const char *buf, size_t size ) 
{
	struct net_device *ndev = to_net_dev(cdev);
	struct net_local *nl = (struct net_local *)netdev_priv(ndev);
	struct channel *chan = (struct channel *)nl->chan_cfg;
	u8 tmp;

	if (!size) return size;
	tmp = buf[0]-'0';

	switch (tmp)
	{
	case 0:
		chan->pbo_mode = PWRBO_NORMAL;
		break;
	case 1:
		chan->pbo_mode = PWRBO_FORCED;
		break;
	}
	return size;
}
static CLASS_DEVICE_ATTR(pbo_mode, 0644, show_pbo_mode, store_pbo_mode);
static ssize_t show_pbo_val(struct class_device *cdev, char *buf) 
{                                                                       
	struct net_device *ndev = to_net_dev(cdev);
	struct net_local *nl = (struct net_local *)netdev_priv(ndev);
	struct channel *chan = (struct channel *)nl->chan_cfg;
	int pos = 0;
	int i;

	// Output string in format <val1>:<val2>:<val3>:<val4>...
	for (i = 0; i < chan->pbo_vnum; i++)
	{
		pos += snprintf(buf + pos, PAGE_SIZE - pos, "%d", chan->pbo_vals[i]);
		if (i != chan->pbo_vnum - 1)
		{
			pos += snprintf(buf + pos, PAGE_SIZE - pos, ":");
		}
	}
	return pos;
}

static ssize_t store_pbo_val( struct class_device *cdev,const char *buf, size_t size ) 
{
	struct net_device *ndev = to_net_dev(cdev);
	struct net_local *nl = (struct net_local *)netdev_priv(ndev);
	struct channel *chan = (struct channel *)nl->chan_cfg;
	char *endp;
	u32 tmp;
	u8 vals[16];
	int vnum = 0, i;
	
	// check parameters
	if (!size) return size;
	do {
		tmp = simple_strtoul(buf, &endp, 10);
		if (buf != endp)
		{
			vals[vnum] = (tmp > 31) ? 31 : tmp;
			vnum++;
		}
		if (*endp == '\0')
		{
			break; // all string is processed
		}
		buf = endp + 1;
	} while (vnum < 16);
	
	for(i = 0; i < vnum; i++)
	{
		chan->pbo_vals[i] = vals[i];
	}
	chan->pbo_vnum = vnum;
	return size;
}
static CLASS_DEVICE_ATTR(pbo_val, 0644 ,show_pbo_val,store_pbo_val);

// Clock mode control
static ssize_t show_clkmode(struct class_device *cdev, char *buf) 
{                                                                       
	struct net_device *ndev = to_net_dev(cdev);
	struct net_local *nl = (struct net_local *)netdev_priv(ndev);
	struct channel *chan = (struct channel *)nl->chan_cfg;

	if ((chan->state != CONNECTED) && (chan->mode != MASTER)) return 0;
	switch (chan->clkmode)
	{
	case 0:
		return snprintf(buf,PAGE_SIZE,"plesio");
	case 1:
		return snprintf(buf,PAGE_SIZE,"sync");
	case 2:
		return snprintf(buf,PAGE_SIZE,"plesio-ref");
	}
	return snprintf(buf,PAGE_SIZE,"unknown");
}
// Clock mode: 0 - Plesiochronuous, 1 - Sinchronuous
static ssize_t store_clkmode(struct class_device *cdev,const char *buf, size_t size)
{
	struct net_device *ndev = to_net_dev(cdev);
	struct net_local *nl = (struct net_local *)netdev_priv(ndev);
	struct channel *chan = (struct channel *)nl->chan_cfg;

	switch (buf[0])
	{
	case '0':
		chan->clkmode = 0;
		break;
	case '1':
		chan->clkmode = 1;
		break;
	case '2':
		chan->clkmode = 2;
		break;
	}
	return size;
}
static CLASS_DEVICE_ATTR(clkmode, 0644 ,show_clkmode,store_clkmode);

// Apply changes
static ssize_t store_apply_cfg(struct class_device *cdev,const char *buf, size_t size)
{
	struct net_device *ndev = to_net_dev(cdev);
	struct net_local *nl = netdev_priv(ndev);
	struct channel *chan = (struct channel *)nl->chan_cfg;

	if (!size) return size;
	if (buf[0] == '1')
	{
		chan->need_reset = 1;
	}
	return size;
}
static CLASS_DEVICE_ATTR(apply_cfg, 0200 ,NULL,store_apply_cfg);

// CRC count attribute 
static ssize_t show_crc16(struct class_device *cdev, char *buf) 
{
	struct net_device *ndev = to_net_dev(cdev);
	struct net_local *nl = netdev_priv(ndev);
	struct channel *chan = (struct channel *)nl->chan_cfg;

	if (chan->crc16)
		return snprintf(buf,PAGE_SIZE,"crc16");
	else
		return snprintf(buf,PAGE_SIZE,"crc32");    
}

static ssize_t store_crc16( struct class_device *cdev,const char *buf, size_t size ) 
{
	struct net_device *ndev = to_net_dev(cdev);
	struct net_local *nl = netdev_priv(ndev);
	struct channel *chan = (struct channel *)nl->chan_cfg;
//	u8 cfg_bt;

	if (!size) return 0;
    
	switch (buf[0])
	{
	case '1':
		if (chan->crc16)
			break;
		chan->crc16 = 1;
//		cfg_bt=ioread8( &(nl->regs->CRA)) | CMOD;
//		iowrite8( cfg_bt,&(nl->regs->CRA));
		break;
	case '0':
		if (!(chan->crc16))
			break;
		chan->crc16 = 0;
//		cfg_bt=ioread8( &(nl->regs->CRA)) & ~CMOD;
//		iowrite8( cfg_bt,&(nl->regs->CRA));
		break;
	}	
	return size;	
}
static CLASS_DEVICE_ATTR(crc16, 0644 ,show_crc16,store_crc16);

// fill byte value
static ssize_t show_fill_7e(struct class_device *cdev, char *buf) 
{
	struct net_device *ndev = to_net_dev(cdev);
	struct net_local *nl = netdev_priv(ndev);
	struct channel *chan = (struct channel *)nl->chan_cfg;

	if (chan->fill_7e == 0x7E)
		return snprintf(buf,PAGE_SIZE,"7E");
	else
		return snprintf(buf,PAGE_SIZE,"FF");    
}
static ssize_t store_fill_7e( struct class_device *cdev,const char *buf, size_t size ) 
{
	struct net_device *ndev = to_net_dev(cdev);
	struct net_local *nl = netdev_priv(ndev);
	struct channel *chan = (struct channel *)nl->chan_cfg;

	if (!size) return 0;
    
	switch(buf[0])
	{
	case '1':
		if (chan->fill_7e)
			break;
		chan->fill_7e = 0x7E;
		break;
	case '0':
		if (!(chan->fill_7e))
			break;
		chan->fill_7e = 0xFF;
		break;
	}	
	return size;	
}
static CLASS_DEVICE_ATTR(fill_7e, 0644 ,show_fill_7e,store_fill_7e);

//---------------------------- Power ------------------------------------- //

static ssize_t show_pwr_source(struct class_device *cdev, char *buf) 
{                                                                       
	struct net_device *ndev = to_net_dev(cdev);
	struct net_local *nl = netdev_priv(ndev);
	struct mam17_card *card = (struct mam17_card *)nl->card;
	
	return snprintf(buf, PAGE_SIZE, "%d", card->pwr_source);
}
static CLASS_DEVICE_ATTR(pwr_source,0444,show_pwr_source,NULL);

// PWRR rgister
// PWRON 
static ssize_t show_pwron(struct class_device *cdev, char *buf) 
{
	struct net_device *ndev = to_net_dev(cdev);
	struct net_local *nl = netdev_priv(ndev);
	struct regs_str *regs = nl->regs;

        switch (nl->number)
        {
        	case 0:
        	case 1:
			return snprintf(buf,PAGE_SIZE,"%s",(regs->PWRR0 & PWRON) ? "1" : "0");
        	break;
        	case 2:
        	case 3:
			return snprintf(buf,PAGE_SIZE,"%s",(regs->PWRR1 & PWRON) ? "1" : "0");
        	break;
        }
        return 0;
}

static ssize_t store_pwron(struct class_device *cdev,const char *buf, size_t size )
{
	struct net_device *ndev = to_net_dev(cdev);
	struct net_local *nl = (struct net_local *)netdev_priv(ndev);
	struct regs_str *regs = nl->regs;

	// check parameters
	if( !size)
		return size;
		
        switch (nl->number)
        {
        	case 0:
        	case 1:
        		if (buf[0] == '0') regs->PWRR0 &= (~PWRON);
        		else regs->PWRR0 |= PWRON;
        	break;
        	case 2:
        	case 3:
        		if (buf[0] == '0') regs->PWRR1 &= (~PWRON);
	      		else regs->PWRR1 |= PWRON;
        	break;
        }
	return size;
}
static CLASS_DEVICE_ATTR(pwron,0644,show_pwron,store_pwron);

// OVL
static ssize_t show_pwrovl(struct class_device *cdev, char *buf) 
{
	struct net_device *ndev = to_net_dev(cdev);
	struct net_local *nl = (struct net_local *)netdev_priv(ndev);
	struct regs_str *regs = nl->regs;

        switch (nl->number)
        {
        	case 0:
        	case 1:
			return snprintf(buf,PAGE_SIZE,"%s",(regs->PWRR0 & OVL) ? "1" : "0");
        	break;
        	case 2:
        	case 3:
			return snprintf(buf,PAGE_SIZE,"%s",(regs->PWRR1 & OVL) ? "1" : "0");
        	break;
        }
        return 0;
}
static CLASS_DEVICE_ATTR(pwrovl,0444,show_pwrovl,NULL);
//UNB
static ssize_t show_pwrunb(struct class_device *cdev, char *buf) 
{
	struct net_device *ndev = to_net_dev(cdev);
	struct net_local *nl = netdev_priv(ndev);
	struct regs_str *regs = nl->regs;
	
        switch (nl->number)
        {
        	case 0:
        	case 1:
			return snprintf(buf,PAGE_SIZE,"%s",(regs->PWRR0 & UNB) ? "1" : "0");
        	break;
        	case 2:
        	case 3:
			return snprintf(buf,PAGE_SIZE,"%s",(regs->PWRR1 & UNB) ? "1" : "0");
        	break;
        }
        return 0;
}
static CLASS_DEVICE_ATTR(pwrunb,0444,show_pwrunb,NULL);

// ------------------------- Statistics ------------------------------- //

static ssize_t show_statistics(struct class_device *cdev, char *buf) 
{
	struct net_device *ndev = to_net_dev(cdev);
	struct net_local *nl = netdev_priv(ndev);
	struct mam17_card *card = nl->card;
	struct statistics stat;
	struct statistics_all *stat_all=&nl->stat;
	u32 buff[10];
	ack_t ack;

	get_statistic(nl->number, card, &stat);

	buff[0] = nl->number;
	mpi_cmd(card, CMD_HDLC_TC_LinkPmParamGet, buff, 1, &ack);

	stat_all->es = stat.ES_count - stat_all->es_old;
	stat_all->ses = stat.SES_count - stat_all->ses_old;
	stat_all->crc_anom = stat.CRC_Anomaly_count - stat_all->crc_anom_old;
	stat_all->losws = stat.LOSWS_count - stat_all->losws_old;
	stat_all->uas = stat.UAS_Count - stat_all->uas_old;

	stat_all->to_line += ack.buf32[3];
	stat_all->from_mii += ack.buf32[4];
	stat_all->tx_aborted += ack.buf32[5];
	stat_all->oversized += ack.buf32[6];
	stat_all->error_marked += ack.buf32[7];
	stat_all->from_line += ack.buf32[8];
	stat_all->crce += ack.buf32[9];
	stat_all->rx_aborted += ack.buf32[10];
	stat_all->invalid_frames += ack.buf32[11];
	stat_all->to_mii += ack.buf32[12];
	stat_all->to_miie += ack.buf32[13];
	stat_all->overflow += ack.buf32[14];


	return snprintf(buf,PAGE_SIZE,
			"\tSNR Margin=%d\n\tLoop Attenuation=%d\n"
			"\tCounters: ES=%u SES=%u CRC_Anom=%u LOSWS=%u UAS=%u\n"
			"\tTX: To line=%u Recv from mii=%u Aborted=%u Oversized=%u Error marked=%u\n"//////
			"\tRX: From line=%u CRCE=%u Aborted=%u Ivalid frames=%u to mii=%u to miiE=%u overflow=%u\n",////
			(s8)stat.SNR_Margin_dB,(s8)stat.LoopAttenuation_dB,
			stat_all->es,stat_all->ses,stat_all->crc_anom,stat_all->losws,stat_all->uas,stat_all->to_line,stat_all->from_mii,
			stat_all->tx_aborted,stat_all->oversized,stat_all->error_marked,stat_all->from_line,stat_all->crce,
			stat_all->rx_aborted,stat_all->invalid_frames,stat_all->to_mii,stat_all->to_miie,stat_all->overflow);
}

static ssize_t store_statistics( struct class_device *cdev,const char *buf, size_t size ) 
{
	struct net_device *ndev = to_net_dev(cdev);
	struct net_local *nl = netdev_priv(ndev);
	struct mam17_card *card = nl->card;
	struct statistics stat;
	struct statistics_all *stat_all=&nl->stat;

	if (!size) return 0;

	if(buf[0] == '1'){
		get_statistic(nl->number, card, &stat);
		memset(stat_all, 0, sizeof(struct statistics_all));
		stat_all->es_old = stat.ES_count;
		stat_all->ses_old = stat.SES_count;
		stat_all->crc_anom_old = stat.CRC_Anomaly_count;
		stat_all->losws_old = stat.LOSWS_count;
		stat_all->uas_old = stat.UAS_Count;
	}
	return size;	
}
static CLASS_DEVICE_ATTR(statistics,0644,show_statistics,store_statistics);

static ssize_t show_statistics_row(struct class_device *cdev, char *buf) 
{
	struct net_device *ndev = to_net_dev(cdev);
	struct net_local *nl = netdev_priv(ndev);
	struct mam17_card *card = nl->card;
	struct statistics stat;
	u32 buff[10];
	ack_t ack;

	get_statistic(nl->number, card, &stat);

	buff[0] = nl->number;
	if (mpi_cmd(card, CMD_HDLC_TC_LinkPmParamGet, buff, 1, &ack))
	{
		return snprintf(buf,PAGE_SIZE,"Error Getting statistic");
	}
	return snprintf(buf,PAGE_SIZE, "%d %d %u %u %u %u %u %u %u %u %u",
			(s8)stat.SNR_Margin_dB,(s8)stat.LoopAttenuation_dB,
			stat.ES_count,stat.SES_count,stat.CRC_Anomaly_count,stat.LOSWS_count,stat.UAS_Count,
			stat.SegmentAnomaly_Count, stat.SegmentDefectS_Count,stat.CounterOverflowInd,stat.CounterResetInd);
//			,ack.buf32[3],ack.buf32[4],ack.buf32[5],ack.buf32[6],ack.buf32[7],ack.buf32[8],ack.buf32[9],ack.buf32[10],
//			ack.buf32[11],ack.buf32[12],ack.buf32[13],ack.buf32[14]);
}
static CLASS_DEVICE_ATTR(statistics_row,0444,show_statistics_row,NULL);




// ------------------------- DEBUG ---------------------------------------- //
static ssize_t store_debug( struct class_device *cdev,const char *buf, size_t size ) 
{
	if (!size) return size;
	switch (buf[0])
	{
		case '1':
			if (debug_recv == 40)
			debug_recv = 0;
			else
			debug_recv = 40;
		break;
		default:
		break;
	}
	return size;
}
static CLASS_DEVICE_ATTR(debug, 0200 ,NULL,store_debug);

// ----------------------- mpair -------------------------------------------- //

static ssize_t show_mpair(struct class_device *cdev, char *buf) 
{
	struct net_device *ndev = to_net_dev(cdev);
	struct net_local *nl = netdev_priv(ndev);
	struct mam17_card *card = nl->card;
	
	return snprintf(buf,PAGE_SIZE, "%i\n", card->mpair_mode);
}

static ssize_t store_mpair( struct class_device *cdev,const char *buf, size_t size ) 
{
	struct net_device *ndev = to_net_dev(cdev);
	struct net_local *nl = netdev_priv(ndev);
	struct mam17_card *card = nl->card;

	if (!size) return 0;
	
	switch (buf[0] - '0')
	{
		case 0:
			card->mpair_mode = 0;
		break;
		case 1:
			card->mpair_mode = MPAIR01;
		break;
		case 2:
			card->mpair_mode = MPAIR021;
		break;
		case 3:
			card->mpair_mode = MPAIR0321;
		break;
		case 4:
			card->mpair_mode = MPAIR03;
		break;
		case 5:
			card->mpair_mode = MPAIR01_23;
		break;
	}

	return size;	
}
static CLASS_DEVICE_ATTR(mpair,0644,show_mpair,store_mpair);


static ssize_t show_uptime(struct class_device *cdev, char *buf) 
{
	struct net_device *ndev = to_net_dev(cdev);
	struct net_local *nl = netdev_priv(ndev);
	struct channel *chan = (struct channel *)nl->chan_cfg;
	struct timeval tv;
	time_t sec_all;
	unsigned s, m, h, d;

	jiffies_to_timeval((unsigned long)jiffies, &tv);
	if (chan->state == CONNECTED) {
		sec_all = tv.tv_sec - chan->uptime.tv_sec;
		d = sec_all / 86400;
		sec_all -= d * 86400;
		h = sec_all / 3600;
		sec_all -= h * 3600;
		m = sec_all / 60;
		sec_all -= m * 60;
		s = sec_all;
		return snprintf(buf,PAGE_SIZE, "%i d %02i:%02i:%02i\n", d, h, m, s);
	} else {
		return snprintf(buf,PAGE_SIZE, "0 d 00:00:00\n");
	}
}
static CLASS_DEVICE_ATTR(uptime,0444,show_uptime,NULL);

static ssize_t show_uptime_all(struct class_device *cdev, char *buf) 
{
	struct net_device *ndev = to_net_dev(cdev);
	struct net_local *nl = netdev_priv(ndev);
	struct channel *chan = (struct channel *)nl->chan_cfg;
	struct timeval tv;
	time_t sec_all;
	unsigned s, m, h, d;

	jiffies_to_timeval((unsigned long)jiffies, &tv);
	if (chan->state == CONNECTED) {
		sec_all = chan->uptime_all.tv_sec + tv.tv_sec - chan->uptime.tv_sec;
		d = sec_all / 86400;
		sec_all -= d * 86400;
		h = sec_all / 3600;
		sec_all -= h * 3600;
		m = sec_all / 60;
		sec_all -= m * 60;
		s = sec_all;
		return snprintf(buf,PAGE_SIZE, "%i d %02i:%02i:%02i\n", d, h, m, s);
	} else {
		sec_all = chan->uptime_all.tv_sec;
		d = sec_all / 86400;
		sec_all -= d * 86400;
		h = sec_all / 3600;
		sec_all -= h * 3600;
		m = sec_all / 60;
		sec_all -= m * 60;
		s = sec_all;
		return snprintf(buf,PAGE_SIZE, "%i d %02i:%02i:%02i\n", d, h, m, s);
	}
}
static CLASS_DEVICE_ATTR(uptime_all,0444,show_uptime_all,NULL);

static ssize_t show_downtime_all(struct class_device *cdev, char *buf) 
{
	struct net_device *ndev = to_net_dev(cdev);
	struct net_local *nl = netdev_priv(ndev);
	struct channel *chan = (struct channel *)nl->chan_cfg;
	struct timeval tv;
	time_t sec_all;
	unsigned s, m, h, d;

	jiffies_to_timeval((unsigned long)jiffies, &tv);
	if (chan->state == CONNECTED) {
		sec_all = chan->uptime.tv_sec - chan->downtime_all.tv_sec - chan->uptime_all.tv_sec;
		d = sec_all / 86400;
		sec_all -= d * 86400;
		h = sec_all / 3600;
		sec_all -= h * 3600;
		m = sec_all / 60;
		sec_all -= m * 60;
		s = sec_all;
		return snprintf(buf,PAGE_SIZE, "%i d %02i:%02i:%02i\n", d, h, m, s);
	} else {
		sec_all = tv.tv_sec - chan->downtime_all.tv_sec - chan->uptime_all.tv_sec;
		d = sec_all / 86400;
		sec_all -= d * 86400;
		h = sec_all / 3600;
		sec_all -= h * 3600;
		m = sec_all / 60;
		sec_all -= m * 60;
		s = sec_all;
		return snprintf(buf,PAGE_SIZE, "%i d %02i:%02i:%02i\n", d, h, m, s);
	}
}
static CLASS_DEVICE_ATTR(downtime_all,0444,show_downtime_all,NULL);

static ssize_t show_on_off(struct class_device *cdev, char *buf) 
{
	struct net_device *ndev = to_net_dev(cdev);
	struct net_local *nl = netdev_priv(ndev);
	struct channel *chan = (struct channel *)nl->chan_cfg;
	
	if (chan->on == 1) {
		return snprintf(buf,PAGE_SIZE, "on\n");
	} else {
		return snprintf(buf,PAGE_SIZE, "off\n");
	}
}
static ssize_t store_on_off( struct class_device *cdev,const char *buf, size_t size ) 
{
	struct net_device *ndev = to_net_dev(cdev);
	struct net_local *nl = netdev_priv(ndev);
	struct channel *chan = (struct channel *)nl->chan_cfg;
	char *endp;
	unsigned on = 0;
	
	on = simple_strtoul(buf, &endp, 10);
	
	if (on == 1) {
		chan->on = 1;
	} else {
		chan->on = 0;
	}
	chan->need_reset = 1;
	return size;
}
static CLASS_DEVICE_ATTR(on_off,0644,show_on_off,store_on_off);

static struct attribute *mam17_attr[] = {
// shdsl
&class_device_attr_link_state.attr,
&class_device_attr_mode.attr,
&class_device_attr_annex.attr,
&class_device_attr_rate.attr,
&class_device_attr_tcpam.attr,
&class_device_attr_pbo_mode.attr,
&class_device_attr_pbo_val.attr,
&class_device_attr_clkmode.attr,
&class_device_attr_apply_cfg.attr,
&class_device_attr_crc16.attr,
&class_device_attr_fill_7e.attr,
&class_device_attr_statistics.attr,
&class_device_attr_statistics_row.attr,
&class_device_attr_debug.attr,
// power supply
&class_device_attr_pwron.attr,
&class_device_attr_pwrovl.attr,
&class_device_attr_pwrunb.attr,
&class_device_attr_pwr_source.attr,

&class_device_attr_mpair.attr,

&class_device_attr_uptime.attr,
&class_device_attr_uptime_all.attr,
&class_device_attr_downtime_all.attr,
&class_device_attr_on_off.attr,

NULL
};

static struct attribute_group mam17_group = {
	.name  = "ms_private",
	.attrs  = mam17_attr,
};

int mam17_sysfs_register(struct net_device *ndev)
{
	struct class_device *class_dev = &(ndev->class_dev);
	struct net_local *nl=(struct net_local *)netdev_priv(ndev);
	static char fname[10] = "device";

	int ret = sysfs_create_group(&class_dev->kobj, &mam17_group);
	ret = sysfs_create_link( &(class_dev->kobj),&(nl->dev->kobj),fname);
	return ret;
}

void mam17_sysfs_remove(struct net_device *ndev)
{
	struct class_device *class_dev = &(ndev->class_dev);
	sysfs_remove_group(&class_dev->kobj, &mam17_group);
	sysfs_remove_link(&(class_dev->kobj),"device");
}
