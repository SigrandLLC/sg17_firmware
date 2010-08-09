/*
	mam17h_pi.c
	
	тут функции для общения с сократом через параллельный интерфейс
*/

#include "mam17h_main.h"
#include "mam17h_pi.h"
#include "mam17h_socrate.h"
#include "mam17h_debug.h"

ack_t gack;
int ack_wait = -1;

const char * opcode2string(int opcode)
{
	switch (opcode)
	{
		case CMD_PMD_Reset + 0x200:
			return "ACK_PMD_Reset";
		case CMD_PMD_CO_PortSubTypeSelect + 0x200:
			return "ACK_PMD_CO_PortSubTypeSelect";
		case CMD_PMD_SpanProfileGroupConfig + 0x200:
			return "ACK_PMD_SpanProfileGroupConfig";
		case CMD_PMD_Control + 0x200:
			return "ACK_PMD_Control";
		case CMD_LinkControl + 0x200:
			return "ACK_LinkControl";
		case CMD_TC_FlowModify + 0x200:
			return "ACK_TC_FlowModify";
		case CMD_HDLC_TC_LinkModify + 0x200:
			return "ACK_HDLC_TC_LinkModify";
		case CMD_xMII_Modify + 0x200:
			return "ACK_xMII_Modify";
		case 0x690:
			return "EVT_PMD_LinkState";
		case ALM_PMD_TC_LayerMismatch:
			return "___ALM_PMD_TC_LayerMismatch____";
		case EVT_InitializationComplete:
			return "EVT_InitializationComplete";
		case EVT_EOC_LinkState:
			return "EVT_EOC_LinkState";
		case CMD_PMD_StatusGet + 0x200:
			return ">>>>>ACK_PMD_StatusGet<<<<<<";
		case CMD_PMD_AlarmControl + 0x200:
			return "ACK_PMD_AlarmControl";
		case CMD_TNL_PMD_0_Message + 0x200:
			return "ACK_TNL_PMD_0_Message";
		case CMD_TNL_PMD_0_Message + 0x201:
			return "ACK_TNL_PMD_1_Message";
		case CMD_TNL_PMD_0_Message + 0x202:
			return "ACK_TNL_PMD_2_Message";
		case CMD_TNL_PMD_0_Message + 0x203:
			return "ACK_TNL_PMD_3_Message";
		case CMD_HDLC_TC_LinkPmParamGet + 0x200:
			return "ACK_HDLC_TC_LinkPMParamGet";
		case EVT_PMD_MultiWireMapping:
			return "EVT_PMD_MultiWireMapping";
		case EVT_PMD_MpairStatus:
			return "!!!!!!!!!!!EVT_PMD_MpairStatus!!!!!!!!!!!!!";
		case EVT_PMD_Channel_0_Down:
			return "EVT_PMD_Channel_0_Down";
		case ALM_DelayMeasurementAborted:
			return "ALM_DelayMeasurementAborted";
		case EVT_PMD_DelayCompState:
			return "EVT_PMD_DelayCompState";

	}
	return "<Not defined>";
}

int mpi_recv(struct mam17_card *card)
{
	u32 buf32[32];
	u8 *buf8 = (u8 *)buf32;
	message_t * msg = (message_t *) buf32;
	int tunnel_msg = 0;
	int i;
	int len = ioread8(&card->regs->MPI_EFSTAT);
	if (len <= 0)
	{
		return -1;
	}


//	PDEBUGL(debug_recv, KERN_NOTICE );//">>>Incoming message: ");
	for (i = 0; i < len; i++)
	{
		buf8[i*4] = ioread8(&card->regs->MPI_EGRESS);
		buf8[i*4+1] = ioread8(&card->regs->MPI_EGRESS);
		buf8[i*4+2] = ioread8(&card->regs->MPI_EGRESS);
		buf8[i*4+3] = ioread8(&card->regs->MPI_EGRESS);
		if (i == 0)
		{
			if ((msg->MSGID == 0x580) || (msg->MSGID == 0x581) || (msg->MSGID == 0x582) || (msg->MSGID == 0x583))
			{
				tunnel_msg = 1;
				PDEBUGL(debug_recv, KERN_NOTICE /*%08x len = %i */"%i RC = %x MSGID = %x from DFE %i:  ",/* buf32[i], len,*/ card->number, msg->RC, msg->MSGID, msg->MSGID - 0x580);
			} else {
//				if (msg->MSGID > 0x400)
				PDEBUGL(debug_recv, KERN_NOTICE /*%08x len = %i*/ "%i RC = %x MSGID = %x %s ", /*buf32[i], len,*/ card->number, msg->RC, msg->MSGID, opcode2string(msg->MSGID));
			}
			
		} else {
			if ((i == 1) && (tunnel_msg))
			{
				PDEBUGL(debug_recv, "%i RC = %x MSGID = %x ", card->number, msg[1].RC, msg[1].MSGID);
			} else {
//				if (msg->MSGID > 0x400)
				PDEBUGL(debug_recv, "%08x ", buf32[i]);
			}
		}
		if (i == 8)
		{
//			PDEBUGL(debug_recv, "\n");
//			PDEBUGL(debug_recv, KERN_NOTICE ">>> ");
		}
	}
	// по адресу buf8 у нас сообщение длинной len байт
	if (msg->MSGID == ack_wait)
	{
		gack.len = len;
		for (i = 0; i < len; i++)
		{
			gack.buf32[i] = buf32[i];
		}
		wake_up(&card->wait);
		ack_wait = -1;
	}
//	if (msg->MSGID > 0x400)
	PDEBUGL(debug_recv, "\n");
	if (msg->MSGID == 0x690)
	{
		card->channels[buf32[1]].state = buf32[2];
		switch (buf32[2])
		{
			case INITIALIZING:
//				PDEBUG(0, "INITIALIZING");
				led_blink(card, buf32[1]);
			break;
			case UP_DATA_MODE:
//				PDEBUG(0, "UP_DATA_MODE");
				led_fast_blink(card, buf32[1]);
			break;
			case DOWN_NOT_READY:
//				PDEBUG(0, "DOWN_NOT_READY");
				if ((buf32[3] == 0x4) && (buf32[4] = 0xc0))
				{
					card->channels[buf32[1]].need_reset = 1;
				}
				led_off(card, buf32[1]);
			break;
			case DOWN_READY:
//				PDEBUG(0, "DOWN_READY");
				led_off(card, buf32[1]);
			break;
			case STOP_DOWN_READY:
//				PDEBUG(0, "STOP_DOWN_READY");
				led_off(card, buf32[1]);
			break;
		}
	}
	if (msg->MSGID == 0x688)
	{
		card->channels[buf32[1]].state = CONNECTED;
	}
	// mpair up
	if ((msg->MSGID == 0x692) && (buf32[2] == 1))
	{
		card->mpair = 4;
	}
	// mpair down
	if ((msg->MSGID == 0x692) && (buf32[2] == 0))
	{
		if (card->channels[buf32[1]].mode == MASTER) card->mpair = 1;
		if (card->channels[buf32[1]].mode == SLAVE) card->mpair = 2;
	}

	if (msg->MSGID == 0x680) card->state = 1;

	iowrite8(INTIDC, &card->regs->MPI_EINT);
	return 0;
}
int mpi_cmd_count = 0;
/*
	opcode - message id
	payload - pointer to payload
	plen - length of payload (numer of 32-byte words)
*/
int mpi_cmd(struct mam17_card *card, u16 opcode, u32 * payload, int plen, ack_t * ack)
{
	message_t msg;
	int i;

/*	
	PDEBUG(0, "opcode = %08x", opcode);
	PDEBUG(0, "plen = %i", plen);
	PDEBUGL(0, KERN_NOTICE);
	for (i = 0; i < plen; i++)
		PDEBUGL(0, "%08x ", payload[i]);
	PDEBUGL(0, "\n");
*/

	PDEBUG(debug_mpi_cmd, "we send %x %s", opcode, opcode2string(opcode));
	
	msg.LENGTH = plen;
	msg.MSGID = opcode;
	msg.TCID = 0;
	msg.RC = 0;
	msg.M = 0;

	iowrite8(((u8*)&msg)[0], &card->regs->MPI_INGRESS);
	iowrite8(((u8*)&msg)[1], &card->regs->MPI_INGRESS);
	iowrite8(((u8*)&msg)[2], &card->regs->MPI_INGRESS);
	iowrite8(((u8*)&msg)[3], &card->regs->MPI_INGRESS);

	for (i = 0; i < plen*4; i+=4)
	{
		iowrite8(((u8 *)payload)[i], &card->regs->MPI_INGRESS);
		iowrite8(((u8 *)payload)[i+1], &card->regs->MPI_INGRESS);
		iowrite8(((u8 *)payload)[i+2], &card->regs->MPI_INGRESS);
		iowrite8(((u8 *)payload)[i+3], &card->regs->MPI_INGRESS);
	}
	iowrite8(1, &card->regs->MPI_IINT);
	i = 0;
	while ((ioread8(&card->regs->MPI_IINT) & 1) && (i < 10000)) i++;
	if (i >= 10000)
	{
		printk(KERN_NOTICE "IDC don't read data from INGRESS FIFO\n");
		return -1;
	}
	gack.len = 0;
	ack_wait = opcode + 0x200;
	if (!interruptible_sleep_on_timeout(&card->wait, 30000))
	{
		printk(KERN_NOTICE ">>>Error: no %x %s\n", opcode, opcode2string(opcode + 0x200));
		return -1;
	}
	if (gack.len != 0)
	{
		ack->len = gack.len;
		for (i = 0; i < gack.len; i++)
		{
			ack->buf32[i] = gack.buf32[i];
		}
	}
	
	if (((message_t *)gack.buf32)->RC != 0) PDEBUG(0, "Bad RC (%i) in %s", ((message_t *)gack.buf32)->RC, opcode2string(((message_t *)gack.buf32)->MSGID));
	
	PDEBUG(debug_mpi_cmd, "-----------we recv %x %s len %i", ((message_t *)gack.buf32)->MSGID, opcode2string(((message_t *)gack.buf32)->MSGID), gack.len);
	ack_wait = -1;

	return 0;
}

int tunnel_cmd(struct mam17_card *card, u16 opcode, u32 * payload, int plen, int dfe_num, ack_t * ack)
{
	static u32 buf[31];
	message_t *msg = (message_t*)buf;
	int i;

	msg[0].LENGTH = plen;
	msg[0].MSGID = opcode;
	msg[0].TCID = 0;
	msg[0].RC = 0;
	msg[0].M = 0;
	
	buf[0] = opcode;
		
	for (i = 0; i < plen; i++)
	{
		buf[i + 1] = payload[i];
	}
	
	if (mpi_cmd(card, CMD_TNL_PMD_0_Message + dfe_num, buf, plen + 1, ack)) return -1;
	return 0;
}

void led_off(struct mam17_card * card, int ch)
{
	switch (ch)
	{
		case 0:
			iowrite8(RXDE, &(card->regs->CRB0));
		break;
		case 1:
			iowrite8(RXDE, &(card->regs->CRB1));
		break;
		case 2:
			iowrite8(RXDE, &(card->regs->CRB2));
		break;
		case 3:
			iowrite8(RXDE, &(card->regs->CRB3));
		break;
	}
}
void led_blink(struct mam17_card * card, int ch)
{
	switch (ch)
	{
		case 0:
			iowrite8(RXDE | LLED0, &(card->regs->CRB0));
		break;
		case 1:
			iowrite8(RXDE | LLED0, &(card->regs->CRB1));
		break;
		case 2:
			iowrite8(RXDE | LLED0, &(card->regs->CRB2));
		break;
		case 3:
			iowrite8(RXDE | LLED0, &(card->regs->CRB3));
		break;
	}
}
void led_fast_blink(struct mam17_card * card, int ch)
{
	switch (ch)
	{
		case 0:
			iowrite8(RXDE | LLED1, &(card->regs->CRB0));
		break;
		case 1:
			iowrite8(RXDE | LLED1, &(card->regs->CRB1));
		break;
		case 2:
			iowrite8(RXDE | LLED1, &(card->regs->CRB2));
		break;
		case 3:
			iowrite8(RXDE | LLED1, &(card->regs->CRB3));
		break;
	}
}
void led_on(struct mam17_card * card, int ch)
{
	switch (ch)
	{
		case 0:
			iowrite8(RXDE | LLED0 | LLED1, &(card->regs->CRB0));
		break;
		case 1:
			iowrite8(RXDE | LLED0 | LLED1, &(card->regs->CRB1));
		break;
		case 2:
			iowrite8(RXDE | LLED0 | LLED1, &(card->regs->CRB2));
		break;
		case 3:
			iowrite8(RXDE | LLED0 | LLED1, &(card->regs->CRB3));
		break;
	}
}
