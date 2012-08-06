#include "ms17e_v2_main.h"
#include "ms17e_v2_proc_fs.h"
#include "ms17e_v2_debug.h"

// port_number from 0 to 7
// state can be: OFF BLINK FAST_BLINK ON
void led_control(u8 port_number, u8 state, struct ms17e_v2_card * card) {
	u8 tmp;
	u8 port_num = 7 - port_number;

//	printk(KERN_ERR"led_control: ");

	if (port_num < 4) {
//		printk("LCR0=%02x ", ioread8(&(card->regs->LCR0)));
		tmp = ioread8(&(card->regs->LCR0));
		iowrite8((~(3 << (port_num*2)) & tmp) | (state << (port_num*2)), &(card->regs->LCR0));
//		printk("LCR0=%02x ", ioread8(&(card->regs->LCR0)));
	} else {
		port_num -= 4;
//		printk("LCR1=%02x ", ioread8(&(card->regs->LCR1)));
		tmp = ioread8(&(card->regs->LCR1));
		iowrite8((~(3 << (port_num*2)) & tmp) | (state << (port_num*2)), &(card->regs->LCR1));
//		printk("LCR1=%02x ", ioread8(&(card->regs->LCR1)));
	}
//	printk("\n");
}

#ifdef DEBUG_ON
u8 offset = 0;
u8 num = 0;
static int read_show(char *buf, char **start, off_t offset_, int count, int *eof, void *data) {
	int i, j = 0;
	struct ms17e_v2_card * card = (struct ms17e_v2_card *)data;
//	PDEBUG(debug_read_write, "offset = %x num = %x", offset, num);
	for (i = 0; i < num; i++)
	{
		j += snprintf(&buf[j], count, "%02x ", ioread8((u8 *)card->regs + offset + i));
	}
	j += snprintf(&buf[j], count, "\n");
	return j;
}
static int read_store(struct file *file,const char *buffer,unsigned long count,void *data) {
	char *endp;

	offset = simple_strtoul(buffer, &endp, 16);
	while( *endp == ' '){
		endp++;
	}
	num = simple_strtoul(endp, &endp, 16);
//	PDEBUG(debug_read_write, "offset = %x num = %x", offset, num);
	return count;
}
static int write_store(struct file *file,const char *buffer,unsigned long count,void *data) {
	char *endp;
	u8 value = 0;
	struct ms17e_v2_card * card = (struct ms17e_v2_card *)data;

	offset = simple_strtoul(buffer, &endp, 16);
	while( *endp == ' '){
		endp++;
	}
	value = simple_strtoul(endp, &endp, 16);
	PDEBUG(debug_read_write, "write: offset = %02x value = %02x\n", offset, value);
	iowrite8(value, (u8 *)card->regs + offset);
	return count;
}
#endif

static int led_show(char *buf, char **start, off_t offset_, int count, int *eof, void *data) {
	int i, j = 0;
	struct ms17e_v2_card * card = (struct ms17e_v2_card *)data;
	u8 LCR0 = 0, LCR1 = 0;
	u16 leds = 0;

	LCR0 = ioread8(&(card->regs->LCR0));
	LCR1 = ioread8(&(card->regs->LCR1));
	leds = LCR0 + ((LCR1 << 8) & 0xFF00);
//	PDEBUG(debug_read_write, "offset = %x num = %x", offset, num);
	for (i = 0; i < card->if_num; i++)
	{
		j += snprintf(&buf[j], count, "led%i=%i; ", i, ((leds >> ((card->if_num - 1 - i)*2)) & 0x03) ? 1 : 0);
	}
	j += snprintf(&buf[j], count, "\n");
	j += snprintf(&buf[j], count, "rx=%lu, rx_error=%lu, tx=%lu, tx_error=%lu\n", card->serial_rx,
				card->serial_rx_error, card->serial_tx, card->serial_tx_error);
	return j;
}
static int led_store(struct file *file,const char *buf,unsigned long count,void *data) {
	struct ms17e_v2_card * card = (struct ms17e_v2_card *)data;
	char *endp;
	int port_num = 0, on_off = 0;

	port_num = simple_strtoul(buf, &endp, 10);
	if ((port_num < 0) || (port_num > 7)) {
		return count;
	}
	on_off = simple_strtoul(&endp[1], &endp, 10);
	
	if (on_off) {
		led_control(port_num, ON, card);
	} else {
		led_control(port_num, OFF, card);
	}
	return count;
}

static int serial_show(char *buf, char **start, off_t offset_, int count, int *eof, void *data) {
	unsigned long i, j = 0;
	struct ms17e_v2_card * card = (struct ms17e_v2_card *)data;
	
	if (card->serial_buff_size > 0) {
		if (card->serial_buff_read_pos < card->serial_buff_write_pos) {
			PDEBUG(debug_serial_show, "read_pos=%lu write_pos=%lu buff_size=%lu\n",
				card->serial_buff_read_pos, card->serial_buff_write_pos, card->serial_buff_size);
			for (i = card->serial_buff_read_pos; i < card->serial_buff_write_pos; i++) {
				j += snprintf(&buf[j], count, "%c", card->serial_buff[card->serial_buff_read_pos]);
				card->serial_buff_read_pos++;
				if (card->serial_buff_read_pos == SERIAL_BUFFER_SIZE) card->serial_buff_read_pos = 0;
				card->serial_buff_size--;
			}
		} else {
			for (i = card->serial_buff_read_pos; i < SERIAL_BUFFER_SIZE; i++) {
				j += snprintf(&buf[j], count, "%c", card->serial_buff[i]);
				card->serial_buff_read_pos++;
				if (card->serial_buff_read_pos == SERIAL_BUFFER_SIZE) card->serial_buff_read_pos = 0;
				card->serial_buff_size--;
			}
			for (i = 0; i < card->serial_buff_write_pos; i++) {
				j += snprintf(&buf[j], count, "%c", card->serial_buff[i]);
				card->serial_buff_read_pos++;
				if (card->serial_buff_read_pos == SERIAL_BUFFER_SIZE) card->serial_buff_read_pos = 0;
				card->serial_buff_size--;
			}
		}
	}
	return j;
}
static int serial_store(struct file *file,const char *buf,unsigned long count,void *data) {
	struct ms17e_v2_card * card = (struct ms17e_v2_card *)data;
	unsigned long i = 0, err = 0;
	
	for (i = 0; i < count; i++) {
		if (serial_tx(buf[i], card)) err++;
	}
	printk(KERN_NOTICE"MS17EPP to serial transmitted %lu byte, %lu with error", count, err);
	return count;
}


int ms17e_v2_proc_fs_register(struct ms17e_v2_card *card) {
	struct proc_dir_entry *led_entry;
#ifdef DEBUG_ON
	char entry_dir[40];
	struct proc_dir_entry *read_entry;
	struct proc_dir_entry *write_entry;
	int error = 0;

	sprintf(entry_dir, "sys/debug/%i", PCI_SLOT(card->pdev->devfn));
	card->debug_entry = proc_mkdir((const char *)entry_dir, NULL);
	if (card->debug_entry == NULL) {
		printk(KERN_ERR"Can not create dir debug\n");
		error = -ENOMEM;
		return error;
	}
	if (!(read_entry = create_proc_entry("read", 0600, card->debug_entry)))
	{
		printk(KERN_ERR"Can not create read\n");
		error = -1;
		return error;
	}
	read_entry->owner=THIS_MODULE;
	read_entry->read_proc=read_show;
	read_entry->write_proc=read_store;
	read_entry->data=card;

	if (!(write_entry = create_proc_entry("write", 0200, card->debug_entry)))
	{
		printk(KERN_ERR"Can not create write\n");
		error = -1;
		return error;
	}
	write_entry->owner=THIS_MODULE;
	write_entry->read_proc=NULL;
	write_entry->write_proc=write_store;
	write_entry->data=card;

#endif
	sprintf(entry_dir, "sys/dev/%i", PCI_SLOT(card->pdev->devfn));
	card->dev_dir_entry = proc_mkdir((const char *)entry_dir, NULL);
	if (!(led_entry = create_proc_entry("led", 0644, card->dev_dir_entry)))
	{
		printk(KERN_ERR"Can not create led entry\n");
		error = -1;
		return error;
	}
	led_entry->owner=THIS_MODULE;
	led_entry->read_proc=led_show;
	led_entry->write_proc=led_store;
	led_entry->data=card;

	if (!(led_entry = create_proc_entry("serial", 0644, card->dev_dir_entry)))
	{
		printk(KERN_ERR"Can not create serial entry\n");
		error = -1;
		return error;
	}
	led_entry->owner=THIS_MODULE;
	led_entry->read_proc=serial_show;
	led_entry->write_proc=serial_store;
	led_entry->data=card;

	return error;
}
int ms17e_v2_proc_fs_remove(struct ms17e_v2_card *card) {
#ifdef DEBUG_ON
	char entry_dir[40];

	sprintf(entry_dir, "sys/debug/%i", PCI_SLOT(card->pdev->devfn));
	remove_proc_entry("write", card->debug_entry);
	remove_proc_entry("read", card->debug_entry);
	remove_proc_entry(entry_dir, NULL);
#endif
	remove_proc_entry("led", card->dev_dir_entry);
	remove_proc_entry("serial", card->dev_dir_entry);
	sprintf(entry_dir, "sys/dev/%i", PCI_SLOT(card->pdev->devfn));
	remove_proc_entry(entry_dir, NULL);
	return 0;
}
