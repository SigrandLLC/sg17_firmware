#include "ms17e_main.h"
#include "ms17e_debug.h"
#include "ms17e_net.h"
#include "ms17e_sysfs.h"
#include "ms17e_si345x.h"

MODULE_DESCRIPTION ( "Driver for ethernet modules platform SG17S. Version "DRIVER_VERSION"\n" );
MODULE_AUTHOR ( "Scherbakov Mihail (scherbakovmihail@sigrand.ru)\n" );
MODULE_LICENSE ( "GPL" );
MODULE_VERSION ( DRIVER_VERSION );

#define POE_CHIP_TIMEOUT HZ/2

static int __devinit ms17e_probe(struct pci_dev *pdev, const struct pci_device_id *dev_id);
static void __devexit ms17e_remove(struct pci_dev *pdev);

static struct pci_device_id ms17e_pci_tbl[] __devinitdata =
{
	{ PCI_DEVICE(MS17E_PCI_VENDOR,  MS17E_PCI_DEVICE)  },
	{ 0 }
};
MODULE_DEVICE_TABLE(pci, ms17e_pci_tbl);
static struct pci_driver ms17e_driver =
{
	name:           MS17E_MODNAME,
	probe:          ms17e_probe,
	remove:         ms17e_remove,
	id_table:       ms17e_pci_tbl
};

// number of card's in system
int card_number = 0;

struct ms17e_card *gcard = NULL;

// TODO: переделать функцию под новый интерфейс регистров
// port_number from 0 to 7
// state can be: OFF BLINK FAST_BLINK ON
void led_control(u8 port_number, u8 state, struct ms17e_card * card) {
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

// read register from PoE chip number num with address addr (num = POE_CHIP0 | POE_CHIP1)
int read_poe_reg(u8 chip_num, u8 addr, struct ms17e_card * card) {
	u8 tmp = 0;
	iowrite8(1 | chip_num | POE_ADDR, &(card->regs->SAR));
	iowrite8(addr, &(card->regs->RAR));
	tmp = ioread8(&(card->regs->CRA));
	iowrite8((tmp & (~3)) | RD, &(card->regs->CRA));
//	PDEBUG(debug_read_poe_reg, "chip = %02x reg = %02x", num, addr);
	if (!interruptible_sleep_on_timeout(&card->wait_read, POE_CHIP_TIMEOUT))
	{
		PDEBUG(debug_read_poe_reg, "Error! sleep timeout");
		return -1;
	}
	return ioread8(&(card->regs->RDR));
}

// write data to register PoE chip number num with address addr (num = POE_CHIP0 | POE_CHIP1)
int write_poe_reg(u8 chip_num, u8 addr, u8 data, struct ms17e_card * card) {
	u8 tmp = 0;
	iowrite8(chip_num | POE_ADDR, &(card->regs->SAR));
	iowrite8(addr, &(card->regs->RAR));
	if (ioread8(&(card->regs->RAR)) != addr) PDEBUG(0, "RAR error!");
	iowrite8(data, &(card->regs->WDR));
	tmp = ioread8(&(card->regs->CRA));
	iowrite8((tmp & (~3)) | WR, &(card->regs->CRA));

	if (!interruptible_sleep_on_timeout(&card->wait_write, POE_CHIP_TIMEOUT))
	{
		PDEBUG(0, "Error! sleep timeout");
		return -1;
	}
	return 0;
}
// функция которая вызывается периодически и обрабатывает прерывания от PoE чипов
void monitor_poe_interrupt(void * data)
{
	struct ms17e_card *card = (struct ms17e_card *)data;
	u8 chip_num = 0, port_num = 0;
	int int_reg = 0, port_events_reg = 0, device_status_reg = 0;
	
	if ((!(ioread8(&(card->regs->IMR)) & PINT)) && ((ioread8(&(card->regs->SR)) & PINT)))
	{

		for (chip_num = 0; chip_num <= 2; chip_num += 2)
		{
			int_reg = read_poe_reg(chip_num, INT, card);
			if (!int_reg) continue;
			for (port_num = 0; port_num < 4; port_num++)
			{
				if (int_reg & (0x01 << port_num)) // port event
				{
					port_events_reg = read_poe_reg(chip_num, PORT1_EVENTS + port_num, card);
					PDEBUG(debug_monitor, "++++++++++ Port %i event!!!", port_num);
					PDEBUG(debug_monitor, "port_events_reg = %02x", port_events_reg);
				}
			}
			if (int_reg & 0x20) // Vee change
			{
				device_status_reg = read_poe_reg(chip_num, DEVICE_STATUS, card);
				PDEBUG(debug_monitor, "++++++++++ Vee change!!!");
			}
			if (int_reg & 0x40) // Overtemp change
			{
				device_status_reg = read_poe_reg(chip_num, DEVICE_STATUS, card);
				PDEBUG(debug_monitor, "++++++++++ Overtemp change!!!");
			}
		}
	}
	schedule_delayed_work(&(card->work), HZ);
}

irqreturn_t interrupt_handler (int irq, void *dev_id, struct pt_regs * regs_)
{
	struct ms17e_card * card = (struct ms17e_card*) dev_id;
	u8 status = ioread8(&(card->regs->SR));

	PDEBUG(debug_interrupt, "status=%02x", status);
	if (status & RDS)
	{
		PDEBUG(debug_interrupt, "RDS");
		wake_up(&(card->wait_read));
	}
	if (status & WRS)
	{
		PDEBUG(debug_interrupt, "WDS");
		wake_up(&(card->wait_write));
	}
	if (status & RDE) PDEBUG(debug_interrupt, "Error read data");
	if (status & WRE) PDEBUG(debug_interrupt, "Error write data");
	
	if (status & PINT)
	{
		PDEBUG(debug_interrupt, "PINT");
		iowrite8(ioread8(&(card->regs->IMR)) & (~PINT), &(card->regs->IMR));
	}
	iowrite8(status, &(card->regs->SR));
	
	return IRQ_HANDLED;
}

#ifdef DEBUG_ON
u8 offset = 0;
u8 num = 0;
static int read_show(char *buf, char **start, off_t offset_, int count, int *eof, void *data) {
	int i, j = 0;
	struct ms17e_card * card = (struct ms17e_card *)data;
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
	struct ms17e_card * card = (struct ms17e_card *)data;

	offset = simple_strtoul(buffer, &endp, 16);
	while( *endp == ' '){
		endp++;
	}
	value = simple_strtoul(endp, &endp, 16);
	PDEBUG(debug_read_write, "write: offset = %02x value = %02x\n", offset, value);
	iowrite8(value, (u8 *)card->regs + offset);
	return count;
}

// PoE regs read/write
u8 poe_reg_num = 0;
u8 poe_chip_num = 0;
static int poe_read_show(char *buf, char **start, off_t offset_, int count, int *eof, void *data) {
	int j = 0;
	struct ms17e_card * card = (struct ms17e_card *)data;
	j += snprintf(&buf[j], count, "%02x\n",  read_poe_reg(poe_chip_num, poe_reg_num, card));
	return j;
}
static int poe_read_store(struct file *file,const char *buffer,unsigned long count,void *data) {
	char *endp;

	poe_chip_num = simple_strtoul(buffer, &endp, 16);
	while( *endp == ' '){
		endp++;
	}
	poe_reg_num = simple_strtoul(endp, &endp, 16);
	if (poe_chip_num > 0) poe_chip_num = 2;
	PDEBUG(debug_read_write, "chip = %02x reg = %02x", poe_chip_num, poe_reg_num);
	return count;
}
static int poe_write_store(struct file *file,const char *buffer,unsigned long count,void *data) {
	char *endp;
	u8 chip = 0, reg = 0, value = 0;
	struct ms17e_card * card = (struct ms17e_card *)data;

	chip = simple_strtoul(buffer, &endp, 16);
	if (chip > 0) chip = 2;
	while( *endp == ' '){
		endp++;
	}
	reg = simple_strtoul(endp, &endp, 16);
	while( *endp == ' '){
		endp++;
	}
	value = simple_strtoul(endp, &endp, 16);
	PDEBUG(debug_read_write, "write: chip = %02x reg = %02x value = %02x\n", chip, reg, value);
	write_poe_reg(chip, reg, value, card);
	return count;
}


#endif

static int __devinit ms17e_probe(struct pci_dev * pdev, const struct pci_device_id * dev_id)
{
	struct ms17e_card *card = NULL;
	unsigned long iomem_start, iomem_end;
	int error = 0, i;
#ifdef DEBUG_ON
	char entry_dir[40];
	struct proc_dir_entry *debug_entry;
	struct proc_dir_entry *read_entry;
	struct proc_dir_entry *write_entry;
	struct proc_dir_entry *poe_read_entry;
	struct proc_dir_entry *poe_write_entry;
#endif

	PDEBUG(debug_probe, "New device (num = %i)", card_number);

	if (pci_enable_device(pdev))
	{
		error = -ENODEV;
		printk(KERN_NOTICE "%s: Cannot enable pci device\n", MS17E_MODNAME);
		goto err1;
	}
	PDEBUG(debug_probe, "Enable device - OK");
//	pci_set_master(pdev);

	card = kmalloc(sizeof(struct ms17e_card), GFP_KERNEL);
	gcard = card;
	if (card == NULL)
	{
		error = -ENODEV;
		printk(KERN_NOTICE "%s: Cannot allocate kernel memory\n", MS17E_MODNAME);
		goto err2;
	}
	PDEBUG(debug_probe, "Allocate kernel memory for struct card - OK");
	memset((void*)card, 0, sizeof(struct ms17e_card));
	pci_set_drvdata(pdev, card);
	card->number = card_number++;
	card->pdev = pdev;

	iomem_start = pci_resource_start(card->pdev, 0);
	iomem_end = pci_resource_end(card->pdev, 0);

	// set card name
	sprintf(card->card_name, CARD_NAME"%i", card->number);
	
	if ((iomem_end - iomem_start) != MS17E_IOMEM_SIZE - 1) return -ENODEV;
	if (!request_mem_region(iomem_start, MS17E_IOMEM_SIZE, (const char *)card->card_name))
	{
		release_mem_region(iomem_start, MS17E_IOMEM_SIZE);
		if (!request_mem_region(iomem_start, MS17E_IOMEM_SIZE, card->card_name))
		{
			printk(KERN_NOTICE "%s: error requets mem region (%lx-%lx)\n", MS17E_MODNAME, iomem_start, iomem_end);
			error = -EBUSY;
			goto err3;
		}
	}
	card->regs = (void *)ioremap(iomem_start, MS17E_IOMEM_SIZE);
	if (card->regs == NULL)
	{
		printk(KERN_NOTICE "%s: error ioremap (%lx-%lx)\n", MS17E_MODNAME, iomem_start, iomem_end);
		error = -ENODEV;
		goto err3;
	}
	PDEBUG(debug_probe, "Prepare io memory - OK");

	PDEBUG(debug_probe, "subsystem device = %x", card->pdev->subsystem_device);
	switch (card->pdev->subsystem_device)
	{
		case MS17E4_SUBSYSTEM_DEVICE:
			card->if_num = 4;
			card->pwr_source = 0;
		break;
		case MS17E4P_SUBSYSTEM_DEVICE:
			card->if_num = 4;
			card->pwr_source = 1;
		break;
		case MS17E8_SUBSYSTEM_DEVICE:
			card->if_num = 8;
			card->pwr_source = 0;
		break;
		case MS17E8P_SUBSYSTEM_DEVICE:
			card->if_num = 8;
			card->pwr_source = 1;
		break;
		default:
			printk(KERN_NOTICE "%s: error hardware PCI Subsystem ID for module\n", MS17E_MODNAME);
			error = -ENODEV;
			goto err3;
	}
	// request IRQ
	if (request_irq(card->pdev->irq, interrupt_handler, SA_SHIRQ, card->card_name, (void*)card))
	{
		PDEBUG(debug_error, "%s: unable to get IRQ %i", card->card_name, card->pdev->irq);
		error = -EBUSY;
		goto err4;
	} else {
		PDEBUG(debug_probe, "Request IRQ (%i) - OK", card->pdev->irq);
	}
	init_waitqueue_head(&card->wait_read);
	init_waitqueue_head(&card->wait_write);
	
	// выключаем все
	iowrite8(0x00, &(card->regs->CRA));
	// включаем PHY и питание, также устанавливаем режим оранж диода на ручной
	iowrite8(ERST | PRST | LEDM, &(card->regs->CRA));
	// гасим все диоды
	iowrite8(0, &(card->regs->LCR0));
	iowrite8(0, &(card->regs->LCR1));
	// включаем все прерывания
	iowrite8(WRS | RDS | WRE | RDE | PINT, &(card->regs->IMR));
	
	INIT_WORK(&(card->work), monitor_poe_interrupt, (void*)card);
	schedule_delayed_work(&(card->work), HZ);
	
//	PDEBUG(debug_probe, "Reset chips - ");
//	mdelay(10000);
//	PDEBUG(debug_probe, "Reset chips - ");
//	write_poe_reg(POE_CHIP0, COMMAND_REG, CMD_RESET_CHIP, card);
//	write_poe_reg(POE_CHIP1, COMMAND_REG, CMD_RESET_CHIP, card);
//	PDEBUG(debug_probe, "Reset chips - ");

	if (card->pwr_source)
	{
		for (i = 0; i < card->if_num; i++)
		{
			card->channels[i].pwr_enable = 1;
			card->channels[i].config_reg = 0x07;
			card->channels[i].icut_reg = 0x75;
		}
	}
	
	
//	printk(KERN_ERR"IMR=%02x\n", ioread8(&(card->regs->IMR)));
/*
	led_control(0, ON, card);
	led_control(1, ON, card);
	led_control(2, ON, card);
	led_control(3, ON, card);
	led_control(4, ON, card);
	led_control(5, ON, card);
	led_control(6, ON, card);
	led_control(7, ON, card);
*/

#ifdef DEBUG_ON
	sprintf(entry_dir, "sys/debug/%i", PCI_SLOT(card->pdev->devfn));
	PDEBUG(debug_read_write, "entry_dir=[%s]", entry_dir);
	debug_entry = proc_mkdir((const char *)entry_dir, NULL);
	if (debug_entry == NULL) {
		printk(KERN_ERR"Can not create dir debug\n");
		error = -ENOMEM;
		goto err4;
	}
	if (!(read_entry = create_proc_entry("read", 0600, debug_entry)))
	{
		printk(KERN_ERR"Can not create read\n");
		error = -1;
		goto derr5;
	}
	read_entry->owner=THIS_MODULE;
	read_entry->read_proc=read_show;
	read_entry->write_proc=read_store;
	read_entry->data=card;

	if (!(write_entry = create_proc_entry("write", 0200, debug_entry)))
	{
		printk(KERN_ERR"Can not create write\n");
		error = -1;
		goto derr6;
	}
	write_entry->owner=THIS_MODULE;
	write_entry->read_proc=NULL;
	write_entry->write_proc=write_store;
	write_entry->data=card;

//  PoE regs read/write
	if (!(poe_read_entry = create_proc_entry("poe_read", 0600, debug_entry)))
	{
		printk(KERN_ERR"Can not create poe_read\n");
		error = -1;
		goto derr7;
	}
	poe_read_entry->owner=THIS_MODULE;
	poe_read_entry->read_proc=poe_read_show;
	poe_read_entry->write_proc=poe_read_store;
	poe_read_entry->data=card;

	if (!(poe_write_entry = create_proc_entry("poe_write", 0200, debug_entry)))
	{
		printk(KERN_ERR"Can not create poe_write\n");
		error = -1;
		goto derr8;
	}
	poe_write_entry->owner=THIS_MODULE;
	poe_write_entry->read_proc=NULL;
	poe_write_entry->write_proc=poe_write_store;
	poe_write_entry->data=card;

#endif
	error = ms17e_net_init(card);
	if (error) {
		printk(KERN_NOTICE "%s: Error net init\n", MS17E_MODNAME);
		goto err4;
	}
	return 0;

#ifdef DEBUG_ON
derr8:
	remove_proc_entry("poe_read", debug_entry);
derr7:
	remove_proc_entry("write", debug_entry);
derr6:
	remove_proc_entry("read", debug_entry);
derr5:
	remove_proc_entry(entry_dir, NULL);
#endif
	
err4:
	free_irq(card->pdev->irq, (void*)card);
err3:
	iounmap(card->regs);
	release_mem_region(iomem_start, MS17E_IOMEM_SIZE);
err2:
	kfree(card);
err1:
	pci_disable_device(pdev);
	return error;
}

void ms17e_card_remove(struct ms17e_card *card)
{
	unsigned long iomem_start = pci_resource_start(card->pdev, 0);
	cancel_delayed_work(&(card->work));
	ms17e_net_remove(card);
	PDEBUG(debug_remove, "net: OK");
	free_irq(card->pdev->irq, (void*)card);
	PDEBUG(debug_remove, "IRQ %i: OK", card->pdev->irq);
	release_mem_region(iomem_start, MS17E_IOMEM_SIZE);
	PDEBUG(debug_remove, "release_mem_region: OK");
	if (card->regs) iounmap(card->regs);
	PDEBUG(debug_remove, "iounmap: OK");
	pci_disable_device(card->pdev);
	PDEBUG(debug_remove, "disable device: OK");
	pci_set_drvdata(card->pdev, NULL);
	kfree(card);
	PDEBUG(debug_remove, "free memory for structure card: OK");
	PDEBUG(debug_remove, "module removed");
}

static void __devexit ms17e_remove(struct pci_dev *pdev)
{
	// если probe_one вернула -1 эта функция не вызовется
	struct ms17e_card *card = pci_get_drvdata(pdev);
	ms17e_card_remove(card);
}


int __devinit ms17e_init (void)
{
	printk(KERN_NOTICE "Load "MS17E_MODNAME" driver\n");
	pci_register_driver(&ms17e_driver);
	return 0;	
}
void __devexit ms17e_exit (void)
{
	printk(KERN_NOTICE "Unload "MS17E_MODNAME" driver\n");
	pci_unregister_driver(&ms17e_driver);
}

module_init(ms17e_init);
module_exit(ms17e_exit);
