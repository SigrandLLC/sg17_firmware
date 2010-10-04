/*
	Тут инициализация socrate (начальная установка регистров, прерывания, заливка прошивок в DFE и IDC)
*/

#include "mam17h_socrate.h"
#include "mam17h_main.h"
#include "mam17h_debug.h"
#include "mam17h_pi.h"
#include "mam17h_mpair.h"
#include <linux/syscalls.h>

// функция которая периодически вызывается и следит за состоянием линков

void monitor_links_state(void * data)
{
	struct mam17_card * card = (struct mam17_card*) data;
	int i;
	ack_t ack;
	u32 buf32[32];
	
	for (i = 0; i < card->if_num; i++)
	{
		if (card->channels[i].need_reset)
		{
			if ((card->mpair_mode != 0) && (is_chan_in_mpair(card, i)))
			{
				switch (card->channels[i].mode)
				{
					case MASTER:
						mpair_master(card);
					break;
					case SLAVE:
						mpair_slave(card);
					break;
				}
				card->channels[i].need_reset = 0;
			} else {
//				PDEBUG(0,"reset channel %i", i);
				if (configure_channel(card, i))
				{
					printk(KERN_NOTICE "Error: configure_channel %i\n", i);
				}
				card->channels[i].need_reset = 0;
			}
			jiffies_to_timeval(0, &(card->channels[i].uptime));
		}
		if (card->channels[i].state == UP_DATA_MODE)
		{
			if (card->mpair_mode)
			{
				if ((i == 0) || ((card->mpair_mode == MPAIR01_23) && (i == 2)))
				{
					buf32[0] = i;
					buf32[1] = IFX_ENABLE;
					buf32[2] = IFX_ENABLE;
					mpi_cmd(card, CMD_LinkControl, buf32, 3, &ack);
				}
			} else {
				buf32[0] = i;
				buf32[1] = IFX_ENABLE;
				buf32[2] = IFX_ENABLE;
				mpi_cmd(card, CMD_LinkControl, buf32, 3, &ack);
				load_cfg(card, i);
			}
			load_cfg(card, i);
			jiffies_to_timeval((unsigned long)jiffies, &(card->channels[i].uptime));
		}
		if (card->channels[i].state == CONNECTED)
		{
			led_on(card, i);
			
			switch (i)
			{
				case 0:
					iowrite8(RXDE | ioread8(&(card->regs->CRB0)), &(card->regs->CRB0));
				break;
				case 1:
					iowrite8(RXDE | ioread8(&(card->regs->CRB1)), &(card->regs->CRB1));
				break;
				case 2:
					iowrite8(RXDE | ioread8(&(card->regs->CRB2)), &(card->regs->CRB2));
				break;
				case 3:
					iowrite8(RXDE | ioread8(&(card->regs->CRB3)), &(card->regs->CRB3));
				break;
			}
		}
	}

//	for (i = 0; i < 4; i++)
//	{
//		u32 buf[10];
//		PDEBUG(0, "param link %i:", i);
//		buf[0] = i;
//		mpi_cmd(card, CMD_HDLC_TC_LinkPmParamGet, buf, 1, &ack);
//		buf[0] = i;
//		if (card->channels[i].mode == MASTER)
//			buf[1] = 1;
//		else
//			buf[1] = 2;
//		mpi_cmd(card, 0x164, buf, 2, &ack);
//		PDEBUG(0, "CMD_DSL_PARAM_GET  ");
//		tunnel_cmd(card, 0x4802, buf, 0, i);
		
//	}
//	load_cfg(card, 0);
	schedule_delayed_work(&(card->work), 2*HZ);
}

// обработчик прерывания
irqreturn_t interrupt (int  irq,  void  * dev_id,  struct pt_regs  * regs_)
{
	struct mam17_card * card = (struct mam17_card*) dev_id;
	u8 t;

	udelay(100);
	if (ioread8(&(card->regs->SCI_INT)) & RME)
	{
		// new message from hdlc controller
		wake_up(&(card->wait));
	}

	if (ioread8(&(card->regs->MPI_EINT)) & INTIDC)
	{
		// new message from IDC
		mpi_recv(card);
	}
	t = ioread8(&(card->regs->SCI_INT));
	PDEBUG(debug_irq, "SCI_INT = %02x", t);

	iowrite8(ioread8(&(card->regs->SCI_INT)), &(card->regs->SCI_INT));

	return IRQ_HANDLED;
}

// инициализация сократа
int mam17_socrate_init(struct mam17_card *card)
{
	struct firmware *fws;
	u8 * fw;
	int error = 0;

	// устанавливаем прерывание
	if (request_irq(card->pdev->irq, interrupt, SA_SHIRQ, card->name, (void*)card))
	{
		PDEBUG(debug_error, "%s: unable to get IRQ %i", card->name, card->pdev->irq);
		goto err1;
	} else {
		PDEBUG(debug_socrate_init, "IRQ - %i OK", card->pdev->irq);
	}
	
	// устанавливаем регистры "самой платы"
	iowrite8(0x0, &(card->regs->CRA));
	mdelay(100);//500
	iowrite8(ioread8(&(card->regs->CRA)) | XINT | XRST | PWRF0 | PWRF1, &(card->regs->CRA));
	mdelay(100);//500
	iowrite8(RXDE, &(card->regs->CRB0));
	iowrite8(RXDE, &(card->regs->CRB1));
	iowrite8(RXDE, &(card->regs->CRB2));
	iowrite8(RXDE, &(card->regs->CRB3));
	// устанавливаем регистры сократа
	iowrite8(0x00, &(card->regs->SCI_CFG_L));
	iowrite8(SRA | RAC | XCRC, &(card->regs->SCI_CFG_H));
	iowrite8(0x84, &(card->regs->SCI_CLKCFG));
	iowrite8(RRES, &(card->regs->SCI_CTRL_L));
	iowrite8(0xFF, &(card->regs->SCI_ACFG0));
	iowrite8(0xFF, &(card->regs->SCI_ACFG1));
	iowrite8(0xF3, &(card->regs->SCI_ACFG2));
	iowrite8(0x00, &(card->regs->SCI_ACFG3));
	iowrite8(INTIDC | INTSCI | MPILEV, &(card->regs->MPI_EINT_EN));
	iowrite8(0x3, &(card->regs->MPI_CON));
	iowrite8(XDOV | XPR | XMR | XDU | RPF | RME | RFO, &(card->regs->SCI_INTEN));
	iowrite8(1, &(card->regs->MPI_IINT_EN));

	spin_lock_init(&card->chip_lock);
	// инициализируем очередь ожидания
	init_waitqueue_head(&card->wait);
	// запускаем монитор состяний линков
	INIT_WORK(&(card->work), monitor_links_state, (void*)card);
	// загружаем прошиву из файла
	if (request_firmware((const struct firmware **)&fws, FW_DFE_NAME, &(card->pdev->dev)))
	{
		PDEBUG(debug_error, "DFE firmware file not found!");
		error = -ENODEV;
		goto err2;
	}
	fw = fws->data;

	if (sdfe4_download_DFE_fw(fw, card))
	{
		PDEBUG(0, "DFE firmware not download!!!");
		error = -ENODEV;
		goto err2;
	} else {
		PDEBUG(debug_socrate_init, "DFE firmware successfull download");
	}
	release_firmware(fws);
	if (request_firmware((const struct firmware **)&fws, FW_IDC_NAME, &(card->pdev->dev)))
	{
		PDEBUG(0, "IDC firmware file not found!");
		error = -ENODEV;
		goto err2;
	}
	fw = fws->data;
	if (sdfe4_download_IDC_fw(fw, card))
	{
		PDEBUG(0, "IDC firmware not download!!!");
		error = -ENODEV;
		goto err2;
	} else {
		PDEBUG(debug_socrate_init, "IDC firmware successfull download");
	}
	release_firmware(fws);
	return 0;

err2:
	release_firmware(fws);
	cancel_delayed_work(&(card->work));	
err1:
	free_irq(card->pdev->irq, (void*)card);
	return error;
}

inline void sdfe4_lock_chip(struct mam17_card *card)
{
	spin_lock(&card->chip_lock);
}
inline void sdfe4_unlock_chip(struct mam17_card *card)
{
	spin_unlock(&card->chip_lock);
}


//--------------------------------------------------------
// функции для общения с сократом (самого низкого уровня)
int sdfe4_hdlc_xmit(u8 *msg, u16 len, struct mam17_card *card) {
	int i = 0, j = 0, k;

	// reset transmitter & reciever
	iowrite8(RRES, &card->regs->SCI_CTRL_L);
	iowrite8(XRES, &card->regs->SCI_CTRL_L);

	for (k = 0; k < len; k+= 64)
	{
		while (!(ioread8(&card->regs->SCI_INT) & XPR) && j < 100000) j++;
		if (j >= 10000)
		{
//			printk(KERN_NOTICE ">>>Error: ~XPR\n");
			return -1;
		}
		for (i = k; i < k + 64; i++)
		{
			if (i >= len) break;
			iowrite8(msg[i], &card->regs->SCI_INGRESS);
		}
		if (i >= len) break;
		iowrite8(ioread8(&card->regs->SCI_CTRL_H) | XTF, &card->regs->SCI_CTRL_H);
	}
	iowrite8(ioread8(&card->regs->SCI_CTRL_H) | XME | XTF, &card->regs->SCI_CTRL_H);

	return 0;
}
int sdfe4_hdlc_wait_intr(int to, struct mam17_card *card) {
	static int c = 0;
	c++;
	if (!interruptible_sleep_on_timeout(&card->wait, to))
	{
		PDEBUG(debug_rs_cmd, ">>>Error: sleep timeout! c = %i", c);
		return -1;
	}
	return 0;
}
int sdfe4_hdlc_recv(u8 * msg, struct mam17_card *card) {
	int l = 0, i;

	l += ioread8(&card->regs->SCI_REPORT_L);
	l += (ioread8(&card->regs->SCI_REPORT_H) & 0x0F) << 8; // из за этой долбаной строчки я тупил несколько дней...

	if (l <= 0) return -1;
	if (l > 128)
	{
		PDEBUG(0, "l is too big!!!! (%i) [%02x][%02x]", l, ioread8(&card->regs->SCI_REPORT_L), ioread8(&card->regs->SCI_REPORT_H));
		return -1;
	}
	for (i = 0; i < l; i++)
	{
		msg[i] = ioread8(&card->regs->SCI_EGRESS);
	}
	iowrite8(RMC, &card->regs->SCI_CTRL_L);

	return l;
}
//--------------------------------------------------------

int rs_cmd_count = 0;
// функции для послыки команды модулю сократа через SCI
int sdfe4_rs_cmd(u8 opcd, u32 *params, u16 plen, struct sdfe4_ret *ret, struct mam17_card *card, u8 emb) {
	int i, j;
	u32 buf[SDFE4_FIFO32];
	u32 *msg32 = (u32*)buf;
	u8  *msg8  = (u8*)buf;

	msg8[0] = PEF24624_ADR_HOST;

	if (emb == Embedded_DFE) {
		msg8[1] = PEF24624_ADR_DEV | PEF24624_ADR_RAMSHELL;
	} else if(emb == Embedded_IDC) {
		msg8[1] =  PEF24624_ADR_DEV| PEF24624_ADR_IDC_RAMSHELL;
	}
	msg8[2] = 0;
	msg8[3] = opcd;

	sdfe4_lock_chip(card);

 	for (i = 0; i < plen; i++)
		msg32[i + 1] = params[i];

	PDEBUG(debug_rs_cmd, "xmit start");
	if (sdfe4_hdlc_xmit(msg8, RAM_CMDHDR_SZ + plen * 4, card)){
		sdfe4_unlock_chip(card);
		return -1;
	}
	if (sdfe4_hdlc_wait_intr(10000, card))
	{
		sdfe4_unlock_chip(card);
		if (rs_cmd_count > 5)
		{
			PDEBUG(0, "Error: no ack");
			return -1;
		} else {
			rs_cmd_count++;
			return sdfe4_rs_cmd(opcd, params, plen, ret, card, emb);
		}
	}
//	PDEBUG(debug_rs_cmd, "wait OK");
	i = sdfe4_hdlc_recv(msg8, card);

	if (i == -1){
		PDEBUG(debug_rs_cmd, "Error: no ack");
		sdfe4_unlock_chip(card);
		return -1;
 	}
	if ((msg8[1] != PEF24624_ADR_HOST) || (msg8[2] != 0xAB))
	{
		PDEBUG(0, "Error: bad ack");
		sdfe4_unlock_chip(card);
		return -1;
	}
//	PDEBUG(debug_rs_cmd, "recv OK, i = %i", i);
//	if (opcd == CMD_WR_REG_RS_FWCTRL)
//	{
//		PDEBUGL(0, KERN_NOTICE "l = %i\n" KERN_NOTICE, i);
//	}
	for (j = 0; j < i; j++)
	{
		ret->val[j] = msg8[j];
//		PDEBUGL(0, "%02x ", msg8[j]);
	}
//	if (opcd == CMD_WR_REG_RS_FWCTRL)
//	{
//		PDEBUGL(0, "\n");
//	}
	ret->l = i;

	sdfe4_unlock_chip(card);

	rs_cmd_count = 0;
	return 0;
}
int sdfe4_aux_cmd (u8 opcode, u8 param_1, struct sdfe4_ret *ret, struct mam17_card *card)
{
	u8 buf[SDFE4_FIFO8];
	u8 *msg8 = (u8*)buf;
	int i, j;

	msg8[0] = PEF24624_ADR_HOST;
	msg8[1] = PEF24624_ADR_DEV | PEF24624_ADR_AUX;
	msg8[2] = opcode;
	msg8[3] = param_1;

	sdfe4_lock_chip(card);

	if (sdfe4_hdlc_xmit(msg8, AUX_CMDHDR_SZ + 1, card))
	{
		sdfe4_unlock_chip(card);
		return -1;
	}

	if (sdfe4_hdlc_wait_intr(10000, card))
	{
		sdfe4_unlock_chip(card);
		return -1;
	}
//	PDEBUG(0, "wait OK");

	i = sdfe4_hdlc_recv(msg8, card);
	if (i == -1)
	{
		sdfe4_unlock_chip(card);
		return -1;
	}

	if ((msg8[1] != PEF24624_ADR_HOST) || (msg8[2] != 0xAB) || (msg8[3] != 0xA9))
	{
		sdfe4_unlock_chip(card);
		return -1;
	}
	for (j = 0; j < i; j++)
	{
		ret->val[j] = msg8[j];
	}
	ret->l = i;

	sdfe4_unlock_chip(card);

	return 0;
}

void print_ret(struct sdfe4_ret * ret)
{
	int i;

	PDEBUG(0, "l = %i", ret->l);
	PDEBUGL(0, KERN_NOTICE);
	for (i = 0; i < ret->l; i++)
	{
		PDEBUGL(0, "[%i]%02x ", i, ret->val[i]);
	}
	PDEBUGL(0, "\n");
}

//=========================================================================
//========================Download firmware to DFE=========================
//========================================================================= 
int sdfe4_download_DFE_fw(u8 * fw, struct mam17_card *card)
{
	int i, k;
	u32 Data_U32[256];
	struct sdfe4_ret ret;
	u8 CODE_CRC[4];
	u8 DATA_CRC[4];
	u32 FWdtpnt = *((u32 *)(fw + FW_DFE_CRC_OFFSET + 8));

	CODE_CRC[0] = *(fw + FW_DFE_CRC_OFFSET + 3);
	CODE_CRC[1] = *(fw + FW_DFE_CRC_OFFSET + 2);
	CODE_CRC[2] = *(fw + FW_DFE_CRC_OFFSET + 1);
	CODE_CRC[3] = *(fw + FW_DFE_CRC_OFFSET);

	DATA_CRC[0] = *(fw + FW_DFE_CRC_OFFSET + 7);
	DATA_CRC[1] = *(fw + FW_DFE_CRC_OFFSET + 6);
	DATA_CRC[2] = *(fw + FW_DFE_CRC_OFFSET + 5);
	DATA_CRC[3] = *(fw + FW_DFE_CRC_OFFSET + 4);

	Data_U32[0] = 0;
  	if (sdfe4_aux_cmd(CMD_WR_REG_AUX_SCI_IF_MODE, 0x0A, &ret, card))////////////////
  	{
  		PDEBUG(0, "CMD_WR_REG_AUX_SCI_IF_MODE - Error!");
  		return -1;
  	}
  	mdelay(2);
	PDEBUG(debug_fw, "CMD_WR_REG_AUX_SCI_IF_MODE - OK");
  	if (sdfe4_rs_cmd(CMD_WR_REG_RS_FWSTART, Data_U32, 1, &ret, card, Embedded_DFE))//////////////////
  	{
  		PDEBUG(0, "CMD_WR_REG_RS_FWSTART - Error!");
  		return -1;
  	}
  	mdelay(2);
	PDEBUG(debug_fw, "CMD_WR_REG_RS_FWSTART - OK");
	if (sdfe4_rs_cmd(CMD_WR_REG_RS_FWCTRL, Data_U32, 1, &ret, card, Embedded_DFE))/////////////////////
	{
		PDEBUG(0, "CMD_WR_REG_RS_FWCTRL - Error!");
		return -1;
	}
	mdelay(2);
	PDEBUG(debug_fw, "CMD_WR_REG_RS_FWCTRL - OK");

//======================== CODE ==========================
	for (k = 0x0; k < FW_DFE_CODE_SIZE; k += PKG_SIZE)
	{
		Data_U32[0] = k/4;
		for (i = 0; i < PKG_SIZE/4; i++)
		{
			Data_U32[i + 1] = cpu_to_be32(*((u32*)&fw[k + i*4 + FW_DFE_CODE_OFFSET]));
		}
		if (sdfe4_rs_cmd(CMD_WR_RAM_RS, (u32*)Data_U32, PKG_SIZE/4 + 1, &ret, card, Embedded_DFE)) {/////////////
			udelay(100);
			if (sdfe4_rs_cmd(CMD_WR_RAM_RS, (u32*)Data_U32, PKG_SIZE/4 + 1, &ret, card, Embedded_DFE)) {////////////
				PDEBUG(0, "Error write CODE DFE");
				return -1;
			}
		}
	}
	PDEBUG(debug_fw, "CODE - OK");

	mdelay(2);
	Data_U32[0] = FWCTRL_CHK;
	if (sdfe4_rs_cmd(CMD_WR_REG_RS_FWCTRL, Data_U32, 1, &ret, card, Embedded_DFE))//////////////
	{
		PDEBUG(0, "CMD_WR_REG_RS_FWCTRL - Error!");
		return -1;
	}
	PDEBUG(debug_fw, "CMD_WR_REG_RS_FWCTRL - OK");
	mdelay(10);

	Data_U32[0] = 0;
	if (sdfe4_rs_cmd(CMD_RD_REG_RS_FWCRC, Data_U32, 1, &ret, card, Embedded_DFE))///////////////////
	{
		PDEBUG(0, "CMD_RD_REG_RS_FWCRC - Error!");
		return -1;
	}
	mdelay(10);
	PDEBUG(debug_fw, "CMD_RD_REG_RS_FWCRC - OK");

	for (i = 0; i < 4; i++) {
		if (CODE_CRC[i] != ret.val[i + 3]){
			PDEBUG(0, "CODE CRC: %02x %02x %02x %02x", CODE_CRC[0], CODE_CRC[1], CODE_CRC[2], CODE_CRC[3]);
			PDEBUG(0, "Recieved CODE CRC: %02x %02x %02x %02x", ret.val[3], ret.val[4], ret.val[5], ret.val[6]);
			PDEBUG(0, "Error: bad code CRC for DFE");
			return -1;
		}
	}

	mdelay(2);
	Data_U32[0] = FWCTRL_SWITCH;
	if (sdfe4_rs_cmd(CMD_WR_REG_RS_FWCTRL, Data_U32, 1, &ret, card, Embedded_DFE))///////////////
	{
		PDEBUG(0, "CMD_WR_REG_RS_FWCTRL - Error!");
		return -1;
	}
	PDEBUG(debug_fw, "CMD_WR_REG_RS_FWCTRL - OK");

	mdelay(2);
//======================== DATA ==========================
	for (k = 0x0; k < FW_DFE_DATA_SIZE; k += PKG_SIZE)
	{
		Data_U32[0] = k/4;
		for (i = 0; i < PKG_SIZE/4; i++)
		{
			Data_U32[i + 1] = cpu_to_be32(*((u32*)&fw[k + i*4 + FW_DFE_DATA_OFFSET]));
		}
		if (sdfe4_rs_cmd(CMD_WR_RAM_RS, (u32*)Data_U32, PKG_SIZE/4 + 1, &ret, card, Embedded_DFE)) {/////////////
			udelay(100);
			if (sdfe4_rs_cmd(CMD_WR_RAM_RS, (u32*)Data_U32, PKG_SIZE/4 + 1, &ret, card, Embedded_DFE)) {////////////
				PDEBUG(0, "Error write DATA DFE");
				return -1;
			}
		}
	}
	PDEBUG(debug_fw, "DATA - OK");

	mdelay(2);
	Data_U32[0] = FWCTRL_CHK | FWCTRL_SWITCH;
	if (sdfe4_rs_cmd(CMD_WR_REG_RS_FWCTRL, Data_U32, 1, &ret, card, Embedded_DFE))///////////////////
	{
		PDEBUG(0, "CMD_WR_REG_RS_FWCTRL - Error!");
		return -1;
	}
	PDEBUG(debug_fw, "CMD_WR_REG_RS_FWCTRL - OK");
	mdelay(10);
	Data_U32[0] = 0;
	if (sdfe4_rs_cmd(CMD_RD_REG_RS_FWCRC,Data_U32,1,&ret,card,Embedded_DFE))///////////////////
	{
		PDEBUG(0, "CMD_RD_REG_RS_FWCRC - Error!");
        	return -1;
	}
	mdelay(10);
	PDEBUG(debug_fw, "CMD_RD_REG_RS_FWCRC - OK");


	for (i = 0; i < 4; i++) {
		if (DATA_CRC[i] != ret.val[i + 3]){
			PDEBUG(0, "DATA CRC: %02x %02x %02x %02x", DATA_CRC[0], DATA_CRC[1], DATA_CRC[2], DATA_CRC[3]);
			PDEBUG(0, "Recieved DATA CRC: %02x %02x %02x %02x", ret.val[3], ret.val[4], ret.val[5], ret.val[6]);
			PDEBUG(0, "Error: bad data CRC for DFE");
			return -1;
		}
	}

	mdelay(2);
	Data_U32[0] = cpu_to_be32(FWdtpnt);
  	if (sdfe4_rs_cmd(CMD_WR_REG_RS_FWDTPNT, Data_U32, 1, &ret, card, Embedded_DFE))///////////////////
  	{
  		PDEBUG(0, "CMD_WR_REG_RS_FWDTPNT - Error!");
  		return -1;
  	}
	PDEBUG(debug_fw, "CMD_WR_REG_RS_FWDTPNT - OK");
	mdelay(2);
	Data_U32[0]= (FWCTRL_PROTECT|FWCTRL_VALID);//FWCTRL_PROTECT|
  	if (sdfe4_rs_cmd(CMD_WR_REG_RS_FWCTRL, Data_U32, 1, &ret, card, Embedded_DFE))///////////////////
  	{
  		PDEBUG(0, "CMD_WR_REG_RS_FWCTRL - Error!");
  		return -1;
  	}
	PDEBUG(debug_fw, "CMD_WR_REG_RS_FWCTRL - OK");
	return 0;
}

//=========================================================================
//========================Download firmware to IDC=========================
//========================================================================= 

int sdfe4_download_IDC_fw(u8 * fw, struct mam17_card *card)// EFM FW
{
	int i,k;
	u32 Data_U32[256];
	u8 CODE_CRC[4];
	u8 DATA_CRC[4];
	struct sdfe4_ret ret;

	CODE_CRC[0] = *(fw + FW_IDC_CRC_OFFSET + 3);
	CODE_CRC[1] = *(fw + FW_IDC_CRC_OFFSET + 2);
	CODE_CRC[2] = *(fw + FW_IDC_CRC_OFFSET + 1);
	CODE_CRC[3] = *(fw + FW_IDC_CRC_OFFSET);

	DATA_CRC[0] = *(fw + FW_IDC_CRC_OFFSET + 7);
	DATA_CRC[1] = *(fw + FW_IDC_CRC_OFFSET + 6);
	DATA_CRC[2] = *(fw + FW_IDC_CRC_OFFSET + 5);
	DATA_CRC[3] = *(fw + FW_IDC_CRC_OFFSET + 4);

	mdelay(2);
	Data_U32[0] = (FWGP1_IFETCH | FWGP1_MWAIT);
  	if (sdfe4_rs_cmd(CMD_WR_REG_RS_FWDTPNT, Data_U32, 1, &ret, card, Embedded_IDC))///////////////////
  	{
  		PDEBUG(0, "CMD_WR_REG_RS_FWDTPNT - Error!");
  		return -1;
  	}
	PDEBUG(debug_fw, "CMD_WR_REG_RS_FWDTPNT - OK");
	mdelay(2);
	Data_U32[0] = 0;	
	if (sdfe4_rs_cmd(CMD_WR_REG_RS_FWCTRL, Data_U32, 1, &ret, card, Embedded_IDC))///////////////////
	{
		PDEBUG(0, "CMD_WR_REG_RS_FWCTRL - Error!");
		return -1;
	}
	PDEBUG(debug_fw, "CMD_WR_REG_RS_FWCTRL - OK");
	mdelay(2);

//==================== boot ================================
	for (k = 0x0; k < 0x40; k += PKG_SIZE)
	{
		Data_U32[0] = k/4;
		for (i = 0; i < PKG_SIZE/4; i++)
		{
			Data_U32[i + 1] = cpu_to_be32(*((u32*)&fw[k + i*4]));
		}
		if (sdfe4_rs_cmd(CMD_WR_RAM_RS, (u32*)Data_U32, PKG_SIZE/4 + 1, &ret, card, Embedded_IDC)) {///////////////////
			udelay(100);
			if (sdfe4_rs_cmd(CMD_WR_RAM_RS, (u32*)Data_U32, PKG_SIZE/4 + 1, &ret, card, Embedded_IDC)) {///////////////////
				PDEBUG(0, "Error write boot IDC");
				return -1;
			}
		}
	}
	PDEBUG(debug_fw, "BOOT - OK");
//=================== CODE =============================
	mdelay(2);
	for (k = 0x0; k < FW_IDC_CODE_SIZE; k += PKG_SIZE)
	{
		Data_U32[0] = k/4 + 0x8000/4;
		for (i = 0; i < PKG_SIZE/4; i++)
		{
			Data_U32[i + 1] = cpu_to_be32(*((u32*)&fw[k + i*4 + FW_IDC_CODE_OFFSET]));
		}
		if (sdfe4_rs_cmd(CMD_WR_RAM_RS, (u32*)Data_U32, PKG_SIZE/4 + 1, &ret, card, Embedded_IDC)) {///////////////////
			udelay(100);
			if (sdfe4_rs_cmd(CMD_WR_RAM_RS, (u32*)Data_U32, PKG_SIZE/4 + 1, &ret, card, Embedded_IDC)) {///////////////////
				PDEBUG(0, "Error write code fw IDC");
				return -1;
			}
		}
	}
	PDEBUG(debug_fw, "CODE - OK");
	mdelay(2);
	Data_U32[0] = FWCTRL_CHK;
	if (sdfe4_rs_cmd(CMD_WR_REG_RS_FWCTRL, Data_U32, 1, &ret, card, Embedded_IDC))///////////////////
	{
		PDEBUG(0, "CMD_WR_REG_RS_FWCTRL - Error!");
		return -1;
	}
	PDEBUG(debug_fw, "CMD_WR_REG_RS_FWCTRL - OK");
	mdelay(10);

	Data_U32[0] = 0;
	if (sdfe4_rs_cmd(CMD_RD_REG_RS_FWCRC, Data_U32, 1, &ret, card, Embedded_IDC))///////////////////
	{
		PDEBUG(0, "CMD_WR_REG_RS_FWCRC - Error!");
		return -1;
	}
	mdelay(10);
	PDEBUG(debug_fw, "CMD_WR_REG_RS_FWCRC - OK");


	for (i = 0; i < 4; i++) {
		if (CODE_CRC[i] != ret.val[i + 3]){
			PDEBUG(0, "IDC CODE CRC: %02x %02x %02x %02x", CODE_CRC[0], CODE_CRC[1], CODE_CRC[2], CODE_CRC[3]);
			PDEBUG(0, "Recieved IDC CODE CRC: %02x %02x %02x %02x", ret.val[3], ret.val[4], ret.val[5], ret.val[6]);
			PDEBUG(0, "Error: bad code CRC for IDC");
			return -1;
		}
	}
	Data_U32[0] = FWCTRL_SWITCH;
	if (sdfe4_rs_cmd(CMD_WR_REG_RS_FWCTRL, Data_U32, 1, &ret, card, Embedded_IDC))///////////////////
	{
		PDEBUG(0, "CMD_WR_REG_RS_FWCTRL - Error!");
		return -1;
	}
	PDEBUG(debug_fw, "CMD_WR_REG_RS_FWCTRL - OK");
	mdelay(2);
//======================== DATA ==========================
	for (k = 0x0; k < FW_IDC_DATA_SIZE; k += PKG_SIZE)
	{
		Data_U32[0] = k/4;
		for (i = 0; i < PKG_SIZE/4; i++)
		{
			Data_U32[i + 1] = cpu_to_be32(*((u32*)&fw[k + i*4 + FW_IDC_DATA_OFFSET]));
		}
		if (sdfe4_rs_cmd(CMD_WR_RAM_RS, (u32*)Data_U32, PKG_SIZE/4 + 1, &ret, card, Embedded_IDC)) {///////////////////
			udelay(100);
			if (sdfe4_rs_cmd(CMD_WR_RAM_RS, (u32*)Data_U32, PKG_SIZE/4 + 1, &ret, card, Embedded_IDC)) {///////////////////
				PDEBUG(0, "Error write data fw IDC");
				return -1;
			}
		}
	}
	PDEBUG(debug_fw, "DATA - OK");
	mdelay(2);
	Data_U32[0] = FWCTRL_CHK | FWCTRL_SWITCH;
	if (sdfe4_rs_cmd(CMD_WR_REG_RS_FWCTRL, Data_U32, 1, &ret, card, Embedded_IDC))///////////////////
	{
		PDEBUG(0, "CMD_WR_REG_RS_FWCTRL - Error!");
		return -1;
	}
	PDEBUG(debug_fw, "CMD_WR_REG_RS_FWCTRL - OK");
	mdelay(10);
	Data_U32[0] = 0;
	if (sdfe4_rs_cmd(CMD_RD_REG_RS_FWCRC,Data_U32,1,&ret,card,Embedded_IDC))///////////////////
	{
		PDEBUG(0, "CMD_RD_REG_RS_FWCRC - Error!");
		return -1;
	}
	mdelay(10);
	PDEBUG(debug_fw, "CMD_RD_REG_RS_FWCRC - OK");


	for (i = 0; i < 4; i++) {
		if (DATA_CRC[i] != ret.val[i + 3]){
			PDEBUG(0, "IDC DATA CRC: %02x %02x %02x %02x", DATA_CRC[0], DATA_CRC[1], DATA_CRC[2], DATA_CRC[3]);
			PDEBUG(0, "Recieved IDC DATA CRC: %02x %02x %02x %02x", ret.val[3], ret.val[4], ret.val[5], ret.val[6]);
			PDEBUG(0, "Error: bad data CRC for IDC");
			return -1;
		}
	}
	mdelay(2);
	Data_U32[0] = (FWCTRL_VALID);
  	if (sdfe4_rs_cmd(CMD_WR_REG_RS_FWCTRL, Data_U32, 1, &ret, card, Embedded_IDC))///////////////////
  	{
  		PDEBUG(0, "CMD_WR_REG_RS_FWCTRL - Error!");
  		return -1;
  	}
	PDEBUG(debug_fw, "CMD_WR_REG_RS_FWCTRL - OK");
	mdelay(2);
	Data_U32[0] = FWGP1_RAMLOADED;
	if (sdfe4_rs_cmd(CMD_WR_REG_RS_FWDTPNT, Data_U32, 1, &ret, card, Embedded_IDC))///////////////////
	{
		PDEBUG(0, "CMD_WR_REG_RS_FWDTPNT - Error!");
		return -1;
	}
	PDEBUG(debug_fw, "CMD_WR_REG_RS_FWDTPNT - OK");
	mdelay(2);
	return 0;
}
