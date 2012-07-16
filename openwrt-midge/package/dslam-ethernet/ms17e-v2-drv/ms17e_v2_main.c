#include "ms17e_v2_main.h"
#include "ms17e_v2_debug.h"
#include "ms17e_v2_proc_fs.h"

MODULE_DESCRIPTION ( "Driver for ethernet modules v2 platform SG17S. Version "DRIVER_VERSION"\n" );
MODULE_AUTHOR ( "Scherbakov Mihail (scherbakovmihail@sigrand.ru)\n" );
MODULE_LICENSE ( "GPL" );
MODULE_VERSION ( DRIVER_VERSION );

static int __devinit ms17e_v2_probe(struct pci_dev *pdev, const struct pci_device_id *dev_id);
static void __devexit ms17e_v2_remove(struct pci_dev *pdev);

static struct pci_device_id ms17e_v2_pci_tbl[] __devinitdata =
{
	{ PCI_DEVICE(MS17E_V2_PCI_VENDOR,  MS17E_V2_PCI_DEVICE)  },
	{ 0 }
};
MODULE_DEVICE_TABLE(pci, ms17e_v2_pci_tbl);
static struct pci_driver ms17e_v2_driver =
{
	name:           MS17E_V2_MODNAME,
	probe:          ms17e_v2_probe,
	remove:         ms17e_v2_remove,
	id_table:       ms17e_v2_pci_tbl
};

// number of card's in system
int card_number = 0;

struct ms17e_v2_card *gcard = NULL;

// transmit byte to serial
int serial_tx(u8 data, struct ms17e_v2_card * card) {
	u8 tmp;

	if (ioread8(&(card->regs->CRA)) & TXEN) {
		PDEBUG(debug_serial_tx, "Error! CRA.TXEN == 1");
		return -1;
	}
//	PDEBUG(debug_serial_tx, "CRA.TXEN == 0");
	iowrite8(data, &(card->regs->TDR));
//	PDEBUG(debug_serial_tx, "TDR = %x", data);
	
	tmp = ioread8(&(card->regs->CRA));
	iowrite8(tmp | TXEN, &(card->regs->CRA));

	if (!interruptible_sleep_on_timeout(&card->wait_transmit, SERIAL_TRANSMIT_TIMEOUT))
	{
		card->serial_tx_error++;
		PDEBUG(debug_serial_tx, "Error! transmit sleep timeout!");
		return -1;
	}
	return 0;
}

irqreturn_t interrupt_handler (int irq, void *dev_id, struct pt_regs * regs_)
{
	struct ms17e_v2_card * card = (struct ms17e_v2_card*) dev_id;
	u8 status = ioread8(&(card->regs->SR));

	PDEBUG(debug_interrupt, "status=%02x", status);
	if (status & RXS)
	{
		PDEBUG(debug_interrupt, "RXS");
		PDEBUG(debug_interrupt, "read_pos=%lu write_pos=%lu buff_size=%lu\n",
				card->serial_buff_read_pos, card->serial_buff_write_pos, card->serial_buff_size);
		card->serial_buff[card->serial_buff_write_pos] = ioread8(&(card->regs->RDR));
		card->serial_buff_write_pos++;
		card->serial_buff_size++;
		card->serial_rx++;
		if (card->serial_buff_write_pos == SERIAL_BUFFER_SIZE) {
			card->serial_buff_write_pos = 0;
		}
		if (card->serial_buff_write_pos == card->serial_buff_read_pos) {
			card->serial_buff_read_pos++;
			card->serial_buff_size--;
			card->serial_rx--;
			card->serial_rx_error++;
			if (card->serial_buff_read_pos == SERIAL_BUFFER_SIZE) card->serial_buff_read_pos = 0;
		}
	}
	if (status & TXS)
	{
		PDEBUG(debug_interrupt, "TXS");
		wake_up(&(card->wait_transmit));
		card->serial_tx++;
	}
	if (status & RXE) {
		PDEBUG(debug_interrupt, "Error read data");
		card->serial_rx_error++;
	}

	iowrite8(status, &(card->regs->SR));
	
	return IRQ_HANDLED;
}

static int __devinit ms17e_v2_probe(struct pci_dev * pdev, const struct pci_device_id * dev_id)
{
	struct ms17e_v2_card *card = NULL;
	unsigned long iomem_start, iomem_end;
	int error = 0;//, i;
//	u8 chip_num = 0, port_num = 0;

	PDEBUG(debug_probe, "New device (num = %i)", card_number);

	if (pci_enable_device(pdev))
	{
		error = -ENODEV;
		printk(KERN_NOTICE "%s: Cannot enable pci device\n", MS17E_V2_MODNAME);
		goto err1;
	}
	PDEBUG(debug_probe, "Enable device - OK");
//	pci_set_master(pdev);

	card = kmalloc(sizeof(struct ms17e_v2_card), GFP_KERNEL);
	gcard = card;
	if (card == NULL)
	{
		error = -ENODEV;
		printk(KERN_NOTICE "%s: Cannot allocate kernel memory\n", MS17E_V2_MODNAME);
		goto err2;
	}
	PDEBUG(debug_probe, "Allocate kernel memory for struct card - OK");
	memset((void*)card, 0, sizeof(struct ms17e_v2_card));
	pci_set_drvdata(pdev, card);
	card->number = card_number++;
	card->pdev = pdev;

	iomem_start = pci_resource_start(card->pdev, 0);
	iomem_end = pci_resource_end(card->pdev, 0);

	// set card name
	sprintf(card->card_name, CARD_NAME"%i", card->number);

	if ((iomem_end - iomem_start) != MS17E_V2_IOMEM_SIZE - 1) return -ENODEV;
	if (!request_mem_region(iomem_start, MS17E_V2_IOMEM_SIZE, (const char *)card->card_name))
	{
		release_mem_region(iomem_start, MS17E_V2_IOMEM_SIZE);
		if (!request_mem_region(iomem_start, MS17E_V2_IOMEM_SIZE, card->card_name))
		{
			printk(KERN_NOTICE "%s: error requets mem region (%lx-%lx)\n", MS17E_V2_MODNAME, iomem_start, iomem_end);
			error = -EBUSY;
			goto err3;
		}
	}
	card->regs = (void *)ioremap(iomem_start, MS17E_V2_IOMEM_SIZE);
	if (card->regs == NULL)
	{
		printk(KERN_NOTICE "%s: error ioremap (%lx-%lx)\n", MS17E_V2_MODNAME, iomem_start, iomem_end);
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
			printk(KERN_NOTICE "%s: error hardware PCI Subsystem ID for module\n", MS17E_V2_MODNAME);
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
	init_waitqueue_head(&card->wait_receive);
	init_waitqueue_head(&card->wait_transmit);

	card->serial_buff = kmalloc(SERIAL_BUFFER_SIZE, GFP_KERNEL);
	card->serial_buff_write_pos = 0;
	card->serial_buff_read_pos  = 0;
	card->serial_buff_size  = 0;
	card->serial_tx = 0;
	card->serial_tx_error = 0;
	card->serial_rx = 0;
	card->serial_rx_error = 0;

	// выключаем все
	iowrite8(0x00, &(card->regs->CRA));
	// включаем PHY и питание, также устанавливаем режим оранж диода на ручной
	iowrite8(ERST | PRST | LEDM | RXEN, &(card->regs->CRA));
	// гасим все диоды
	iowrite8(0xFF, &(card->regs->LCR0));
	iowrite8(0xFF, &(card->regs->LCR1));
	// включаем все прерывания
	iowrite8(TXS | RXS | RXE, &(card->regs->IMR));

/*
	if (card->pwr_source)
	{
		for (i = 0; i < card->if_num; i++)
		{
			card->channels[i].pwr_enable = 0;
			card->channels[i].config_reg = 0x07;
			card->channels[i].icut_reg = 0x75;
		}
	}
// выключаем питание на всех портах
// включать будем в соответсвии с полученными настройками
	for (chip_num = 0; chip_num <= 2; chip_num += 2)
	{
		for (port_num = 0; port_num < 4; port_num++)
		{
			write_poe_reg(chip_num, PORT1_CONFIG + port_num, 0x00, card);
			write_poe_reg(chip_num, COMMAND_REG, CMD_PORT_RESET | port_num, card);
			led_control(chip_num ? 3 - port_num : 7 - port_num, OFF, card);
		}
	}
*/
	error = ms17e_v2_proc_fs_register(card);
	return 0;

err4:
	free_irq(card->pdev->irq, (void*)card);
err3:
	iounmap(card->regs);
	release_mem_region(iomem_start, MS17E_V2_IOMEM_SIZE);
err2:
	kfree(card);
err1:
	pci_disable_device(pdev);
	return error;
}

void ms17e_v2_card_remove(struct ms17e_v2_card *card)
{
	unsigned long iomem_start = pci_resource_start(card->pdev, 0);
	free_irq(card->pdev->irq, (void*)card);
	ms17e_v2_proc_fs_remove(card);
	PDEBUG(debug_remove, "IRQ %i: OK", card->pdev->irq);
	release_mem_region(iomem_start, MS17E_V2_IOMEM_SIZE);
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

static void __devexit ms17e_v2_remove(struct pci_dev *pdev)
{
	// если probe_one вернула -1 эта функция не вызовется
	struct ms17e_v2_card *card = pci_get_drvdata(pdev);
	ms17e_v2_card_remove(card);
}


int __devinit ms17e_v2_init (void)
{
	printk(KERN_NOTICE "Load "MS17E_V2_MODNAME" driver\n");
	pci_register_driver(&ms17e_v2_driver);
	return 0;
}
void __devexit ms17e_v2_exit (void)
{
	printk(KERN_NOTICE "Unload "MS17E_V2_MODNAME" driver\n");
	pci_unregister_driver(&ms17e_v2_driver);
}

module_init(ms17e_v2_init);
module_exit(ms17e_v2_exit);
