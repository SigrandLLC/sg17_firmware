/* mr17g_sci.c
 *  Sigrand MR17G E1 PCI adapter driver for linux (kernel 2.6.x)
 *
 *	Written 2008 by Artem Y. Polyakov (artpol84@gmail.com)
 *
 *	This driver presents MR17G modem to OS as common hdlc interface.
 *
 *	This software may be used and distributed according to the terms
 *	of the GNU General Public License.
 *
 */


#include "mr17g.h"
#include "mr17g_sci.h"
#include "mr17g_net.h"
#include "pef22554.h"
// Debug settings
#define DEBUG_ON
#define DEFAULT_LEV 10
#include "mr17g_debug.h"

// DEBUG
static int gen_interrupts = 0;
static int sci_interrupts = 0;
//DEBUG

int
mr17g_sci_enable(struct mr17g_chip *chip)
{
    volatile struct mr17g_sci_iomem *sci = &chip->iomem->sci;

    // setup HDLC registers
	iowrite8( 0,&sci->regs.CRA);
	mdelay(100);
	iowrite8( (XRST | RXEN),&sci->regs.CRA);
	iowrite8(1,&sci->regs.SR);
    iowrite8((RXS | TXS | CRC | COL | OFL),&sci->regs.IMR);	

    // Init chip locking
	spin_lock_init(&chip->sci.lock );
    // Init interrupt wait queue
    init_waitqueue_head( &chip->wait_q );
    // Init monitoring work queue
	INIT_WORK( &chip->wqueue, mr17g_sci_monitor,(void*)chip);

    // setup SCI controller
    if( pef22554_setup_sci(chip) ){
        goto error;
    }
    // Display result HDLC registers content
    sci = &chip->iomem->sci;
    PDEBUG(debug_sci,"CRA=%02x, CRB=%02x, IMR=%02x, SR=%02x",
    sci->regs.CRA,sci->regs.CRB,sci->regs.IMR,sci->regs.SR);

	PDEBUG(debug_sci,"SCI enabled");
	return 0;
error:
	iowrite8( 0,&sci->regs.CRA);
    iowrite8(0,&sci->regs.IMR);	
    return -1;
}

int
mr17g_sci_disable(struct mr17g_chip *chip)
{
    volatile struct mr17g_sci_iomem *sci = &chip->iomem->sci;

	PDEBUG(debug_sci,"start");

	// shut down device
	iowrite8( 0,&sci->regs.CRA);
	iowrite8( 0,&sci->regs.IMR);	
	iowrite8( 0xff ,&sci->regs.SR );
	PDEBUG(debug_sci,"SCI disabled");
	return 0;
}

void
mr17g_sci_endmon(struct mr17g_chip *chip)
{
	PDEBUG(debug_sci,"start");
    // Cancel monitoring
	cancel_delayed_work(&chip->wqueue);
}

int 
mr17g_sci_request_one(struct mr17g_chip *chip,char buf[SCI_BUF_SIZE],int size)
{
    volatile struct mr17g_sci_iomem *sci = &chip->iomem->sci;
	u8 tmp = ioread8(&sci->regs.CRA);
    int i,ret;

	PDEBUG(debug_sci,"start");	
//    PDEBUG(debug_sci,"CRA=%02x, CRB=%02x, IMR=%02x, SR=%02x",
//            sci->regs.CRA,sci->regs.CRB,sci->regs.IMR,sci->regs.SR);

    // SCI is busy now, fail to transmit
    if( tmp & TXEN ){
        printk(KERN_ERR"%s: SCI transmitter already in use\n",MR17G_MODNAME);
        return -EAGAIN;
    }
    // Check size of buffer
    if( size > SCI_BUF_SIZE ){
        printk(KERN_ERR"%s: bad message size(%d) in SCI xmit function\n",MR17G_MODNAME,size);
        return -EINVAL;
    }
    PDEBUG(debug_sci,"Fill SCI buffer");

    //---------------- Transmit message --------------------

    // Lock chipset until end of request
    spin_lock(&chip->sci.lock);

    // Move outgoing data to toransmit buffer
	for( i=0; i<size; i++)
		iowrite8( buf[i],(u8*)sci->tx_buf + i);

    // Prepare for transmission
    chip->sci.rxs = 0;
    chip->sci.crc_err = 0;
	iowrite16( 0, &sci->regs.RXLEN);
	iowrite16( size, &sci->regs.TXLEN);
    // Delay for hardware correct work
    mdelay(1);

    PDEBUG(debug_sci,"Before enable transmitter. First part of message, second - registers");
    mr17g_sci_dump((u8*)&sci->tx_buf,7);
    mr17g_sci_dump((u8*)&sci->regs,7);

    // Enable SCI transmitter
	iowrite8( (ioread8(&sci->regs.CRA) | TXEN ), &sci->regs.CRA);

    //----------------- Receive reply ------------------------

    // Wait for message
    if( !chip->sci.rxs ){
       ret = interruptible_sleep_on_timeout( &chip->wait_q, HZ/2 );
    }else{
        ret = 1;
    }
    PDEBUG(debug_sci,"After interrupt wait");	
    // Check correctness of transmission and receiving
	if( chip->sci.crc_err || !chip->sci.rxs ){
		PDEBUG(debug_error,"Collision detected, sci.rxs = %d, sci.crc_err=%d",chip->sci.rxs,chip->sci.crc_err);
		chip->sci.crc_err = 0;
        size = -1;
        goto exit;
	}
    // Read & check incoming message length
	size = ioread16(&sci->regs.RXLEN);
    PDEBUG(debug_sci,"size = %d", size);
	if( !size ){
		PDEBUG(debug_error,"Zero length");
		size = -EAGAIN; 
        goto exit;
	}else if(size > SCI_BUF_SIZE) {
        printk(KERN_ERR"%s: in SCI recv incoming size(%d) > MAX\n",MR17G_MODNAME,size);
		size = -EINVAL; 
        goto exit;
	}

    // Move incoming message to local buffer
	for( i=0; i<size; i++){
		buf[i] = ioread8((u8*)sci->rx_buf + i);
    }

//----------- DEBUG ---------------------------
    PDEBUGL(debug_sci,"Recv msg:");
    for( i=0; i<size; i++){
       PDEBUGL(debug_sci,"%02x ",buf[i] & 0xff);
    }
    PDEBUGL(debug_sci,"\n");
//----------- DEBUG ---------------------------

exit:
    // Restore receiver on error exit
	iowrite8( (ioread8( &sci->regs.CRA ) | RXEN), &sci->regs.CRA );		
    // Unlock chip
    spin_unlock(&chip->sci.lock);

    PDEBUG(debug_sci,"CRA=%02x, CRB=%02x, IMR=%02x, SR=%02x",
            sci->regs.CRA,sci->regs.CRB,sci->regs.IMR,sci->regs.SR);
	PDEBUG(debug_sci,"end");	
    return size;
}


int 
mr17g_sci_request(struct mr17g_chip *chip,char buf[SCI_BUF_SIZE],int size)
{
    int i,ret = 0;;
    
    for(i=0;i<3;i++){
        if( (ret = mr17g_sci_request_one(chip,buf,size)) >= 0 ){
            return ret;
        }
        PDEBUG(debug_error,"Iter %d, error",i);
    }
    return ret;
}


irqreturn_t
mr17g_sci_intr(int  irq,  void  *dev_id,  struct pt_regs  *regs )
{
	struct mr17g_chip *chip = (struct mr17g_chip *)dev_id;
    volatile struct mr17g_sci_iomem *sci = &chip->iomem->sci;
	u8 mask = ioread8(&sci->regs.IMR);
	u8 status = (ioread8(&sci->regs.SR) & mask);	

    gen_interrupts++;
	
	if( !status )
		return IRQ_NONE;
    
    sci_interrupts++;
	PDEBUG(debug_sci,"status=%02x, gen=%d, sci=%d",status,gen_interrupts,sci_interrupts);	

	iowrite8(0xff,&sci->regs.SR);   // ack all interrupts
	iowrite8(0,&sci->regs.IMR);  // disable interrupts

	if( status & TXS ){
		PDEBUG(debug_sci,"TXS");
		chip->sci.tx_packets++;
		chip->sci.tx_bytes += ioread16(&sci->regs.TXLEN);
	}

	if( status & RXS ){
		int in_len;
		PDEBUG(debug_sci,"RXS");
		in_len = ioread16(&sci->regs.RXLEN);
		chip->sci.rx_packets++;
		chip->sci.rx_bytes += in_len;
		wake_up( &chip->wait_q );
        chip->sci.rxs = 1;
	}

	if( status & CRC ){
        chip->sci.crc_errors++;
		chip->sci.crc_err = 1;
		iowrite8( (ioread8( &sci->regs.CRA ) | RXEN), &sci->regs.CRA );
		PDEBUG(debug_error,"CRC");
		wake_up( &chip->wait_q );
	}
	
	if( status & COL ){
        chip->sci.tx_collisions++;
		PDEBUG(debug_error,"COL");
		wake_up( &chip->wait_q );
	}
	
	iowrite8(mask, &sci->regs.IMR); // enable interrupts
	return IRQ_HANDLED;
}

void
mr17g_sci_monitor(void *data)
{
	struct mr17g_chip *chip = (struct mr17g_chip *)data;
    int i;
    for(i=0;i<chip->if_quan;i++){
        mr17g_net_link(chip->ifs[i]);
    }
    // Schedule next monitoring
	schedule_delayed_work(&chip->wqueue,2*HZ);
}


void
mr17g_sci_dump(u8 *addr, int size)
{
    int i;
    PDEBUGL(debug_sci,"I/O Memory window (from %p):",addr);
    for(i=0;i<size;i++){
        if(!(i%32)){
            PDEBUGL(debug_sci,"\n0x%04x-0x%04x: ",i,i+32);
        }
        PDEBUGL(debug_sci,"%02x ",addr[i]);
    }
    PDEBUGL(debug_sci,"\n");
}

void 
mr17g_sci_memcheck(u8 *addr){
    int i,j;
    int flag = 0;
return;
    for(i=0;i<4;i++){
        u8 tmp = 1;
        // Zero check
        *(addr + i) = 0;
        if( *(addr + i) ){
            PDEBUG(debug_sci,"byte %d, zero check error",i);
            flag = 1;
        }   
        for(j=0;j<8;j++,tmp*=2){
            *(addr + i) = tmp;
//            PDEBUG(debug_sci,"byte %d, val = %d, actual = %d",i,tmp,*(addr+i));
            if( *(addr + i) != tmp ){
                PDEBUG(debug_sci,"byte %d, val = %d, actual = %d",i,tmp,*(addr+i));
                flag = 1;
            }
        }
    }
    if( !flag ){
        PDEBUG(debug_sci,"Memcheck - SUCCESS");
    }
}


void 
mr17g_sci_memcheck1(u8 *addr){
    int i,j;
    int flag = 0;
return;
    for(i=0;i<4;i++){
        u8 tmp = 1;
        // Zero check
        iowrite8(0,addr + i);
        if( ioread8(addr + i) ){
            PDEBUG(debug_sci,"byte %d, zero check error",i);
            flag = 1;
        }   
        for(j=0;j<8;j++,tmp*=2){
            iowrite8(tmp,addr + i);
//            PDEBUG(debug_sci,"byte %d, val = %d, actual = %d",i,tmp,ioread8(addr+i));
            if( ioread8(addr + i) != tmp ){
                PDEBUG(debug_sci,"byte %d, val = %d, actual = %d",i,tmp,ioread8(addr+i));
                flag = 1;
            }
        }
    }
    if( !flag ){
        PDEBUG(debug_sci,"Memcheck - SUCCESS");
    }
}

