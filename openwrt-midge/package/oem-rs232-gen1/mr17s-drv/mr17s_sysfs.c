/* mr17s_sysfs.c
 *  Sigrand MR17S: RS232 PCI adapter driver for linux (kernel 2.6.x)
 *
 *	Written 2008 by Artem Y. Polyakov (artpol84@gmail.com)
 *
 *	This driver presents MR17S module interfaces as serial ports for OS
 *
 *	This software may be used and distributed according to the terms
 *	of the GNU General Public License.
 *
 */

#include "mr17s.h"
#include "mr17s_sysfs.h"

//#define DEBUG_ON
#include "mr17s_debug.h"

#ifdef DEBUG_SYSFS

static ssize_t show_winread( struct device *dev, ADDIT_ATTR char *buff ); 
static ssize_t store_winread( struct device *dev, ADDIT_ATTR const char *buff, size_t size );
static ssize_t show_winwrite( struct device *dev, ADDIT_ATTR char *buff ); 
static ssize_t store_winwrite( struct device *dev, ADDIT_ATTR const char *buff, size_t size );
static ssize_t show_hdlcregs( struct device *dev, ADDIT_ATTR char *buff ); 
static ssize_t store_hdlcregs( struct device *dev, ADDIT_ATTR const char *buff, size_t size );

static DEVICE_ATTR(winread,0600,show_winread,store_winread);	
static DEVICE_ATTR(winwrite,0600,show_winwrite,store_winwrite);	
static DEVICE_ATTR(hdlcregs,0600,show_hdlcregs,store_hdlcregs);	

int mr17s_sysfs_register(struct device *dev)
{
    int err = 0;
    if( (err = device_create_file(dev,&dev_attr_winread)) )  goto err_ext;
    if( (err = device_create_file(dev,&dev_attr_winwrite)) ) goto err_ext1;
    if( (err = device_create_file(dev,&dev_attr_hdlcregs)) ) goto err_ext2;    
    return 0;
err_ext2:
    device_remove_file(dev,&dev_attr_winread);
err_ext1:
    device_remove_file(dev,&dev_attr_winwrite);
err_ext:
    return err;
}


void mr17s_sysfs_free(struct device *dev)
{
    device_remove_file(dev,&dev_attr_winread);
    device_remove_file(dev,&dev_attr_winwrite);
    device_remove_file(dev,&dev_attr_hdlcregs);
}

//-------------------- Read memory window ----------------------------------//
static u32 win_start=0,win_count=0;
static ssize_t
show_winread(struct device *dev, ADDIT_ATTR char *buf)
{                               
	struct mr17s_device  *rsdev = (struct mr17s_device *)dev_get_drvdata( dev );
	char *win = (char*)rsdev->iomem + win_start;
	int len = 0,i;

	for(i=0;i<win_count && (len < PAGE_SIZE-3);i++){
		len += sprintf(buf+len,"%02x ",ioread8(win + i) & 0xff);
	}
	len += sprintf(buf+len,"\n");
	return len;
}

static ssize_t
store_winread( struct device *dev, ADDIT_ATTR const char *buf, size_t size )
{
	char *endp;
	if( !size ) return 0;
	win_start = simple_strtoul(buf,&endp,16);
	PDEBUG(40,"buf=%p, endp=%p,*endp=%c",buf,endp,*endp);	
	while( *endp == ' '){
		endp++;
	}
	win_count = simple_strtoul(endp,&endp,16);
	PDEBUG(40,"buf=%p, endp=%p",buf,endp);		
	PDEBUG(40,"Set start=%d,count=%d",win_start,win_count);
	if( !win_count )
		win_count = 1;
	if( (win_start + win_count) > MR17S_IOMEM_SIZE ){
		if( win_start >= (MR17S_IOMEM_SIZE-1) ){
			win_start = 0;
			win_count = 1;
		} else {
			win_count = 1;
		}
	}
	PDEBUG(40,"Set start=%d,count=%d",win_start,win_count);	
	return size;
}

//------------------ Write memory window ---------------------------------------//

static u32 win_written = 0;
static ssize_t
show_winwrite( struct device *dev, ADDIT_ATTR char *buf )
{                               
	return sprintf(buf,"Byte %x is written",win_written);
}

static ssize_t
store_winwrite( struct device *dev, ADDIT_ATTR const char *buf, size_t size )
{
	struct mr17s_device  *rsdev = (struct mr17s_device *)dev_get_drvdata( dev );
	char *win = (char*)rsdev->iomem;
	int start, val;
	char *endp;
	if( !size ) return 0;
	start = simple_strtoul(buf,&endp,16);
	PDEBUG(0,"buf=%p, endp=%p,*endp=%c",buf,endp,*endp);	
	while( *endp == ' '){
		endp++;
	}
	val = simple_strtoul(endp,&endp,16);
	PDEBUG(0,"buf=%p, endp=%p",buf,endp);		
	PDEBUG(0,"Set start=%04x,val=%02x",start,val);
	if( start > MR17S_IOMEM_SIZE ){
		start = 0;
	}
	PDEBUG(0,"Set start=%04x,val=%02x",start,val);
	win_written = start;
	iowrite8((val&0xff),win+start);
	return size;
}


//------------------ Read channel registers  ---------------------------------------//

static u32 channel_num = 0;
static ssize_t
show_hdlcregs( struct device *dev, ADDIT_ATTR char *buf )
{
	struct mr17s_device *rsdev = (struct mr17s_device *)dev_get_drvdata( dev );
    struct mr17s_uart_port *hw_port = rsdev->ports + channel_num;
    struct mr17s_chan_iomem *mem = (struct mr17s_chan_iomem *)hw_port->port.membase;
    struct mr17s_hw_regs *regs = &mem->regs;
                              
	return snprintf(buf,PAGE_SIZE,"CRA=%02x CRB=%02x SR=%02x IMR=%02x\n"
                   "CTR=%02x LTR=%02x CRR=%02x LRR=%02x\n",
                   ioread8(&regs->CRA),ioread8(&regs->CRB),ioread8(&regs->SR),ioread8(&regs->IMR),
                   ioread8(&regs->CTR),ioread8(&regs->LTR),ioread8(&regs->CRR),ioread8(&regs->LRR));
}

static ssize_t
store_hdlcregs( struct device *dev, ADDIT_ATTR const char *buf, size_t size )
{
	struct mr17s_device *rsdev = (struct mr17s_device *)dev_get_drvdata( dev );
    u32 ch_num;
    char *endp;

	if( !size ) return 0;

	ch_num = simple_strtoul(buf,&endp,16);
    if( ch_num < rsdev->port_quan ){
        // apply
        channel_num = ch_num;
    }
	return size;
}

#else

int mr17s_sysfs_register(struct device *dev){}
void mr17s_sysfs_free(struct device *dev){}

#endif // DEBUG_SYSFS