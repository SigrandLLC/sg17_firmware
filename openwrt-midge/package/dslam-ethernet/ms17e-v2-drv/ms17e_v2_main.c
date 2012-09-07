#include "ms17e_v2_main.h"
#include "ms17e_v2_debug.h"

MODULE_DESCRIPTION( "RS232 PCI adapter driver Version "MS17E_V2_VER"\n" );
MODULE_AUTHOR( "Maintainer: Scherbakov Mihail <scherbakov.mihail@gmail.com>\n" );
MODULE_LICENSE( "GPL" );
MODULE_VERSION(MS17E_V2_VER);

// Register module init/deinit routines
static unsigned int cur_card_number = 0;
module_init(ms17e_v2_init);
module_exit(ms17e_v2_exit);

/*----------------------------------------------------------
 * Driver initialisation
 *----------------------------------------------------------*/
static struct pci_device_id  ms17e_v2_pci_tbl[] __devinitdata = {
	{PCI_DEVICE(MS17E_V2_PCI_VEN, MS17E_V2_PCI_DEV)},
	{0}
};

MODULE_DEVICE_TABLE (pci, ms17e_v2_pci_tbl);

static struct pci_driver ms17e_v2_driver = {
	name:		MS17E_V2_DRVNAME,
	probe:		ms17e_v2_init_one,
	remove:		ms17e_v2_remove_one,
	id_table:	ms17e_v2_pci_tbl
};

static struct uart_driver ms17e_v2_uartdrv = {
	.owner			= THIS_MODULE,
	.driver_name	= MS17E_V2_MODNAME,
	.dev_name		= MS17E_V2_SERIAL_NAME,
	.devfs_name		= MS17E_V2_SERIAL_NAME,
	.major			= MS17E_V2_SERIAL_MAJOR,
	.minor			= MS17E_V2_SERIAL_MINORS,
	.nr				= MS17E_V2_UART_NR,
};

static struct uart_ops ms17e_v2_uart_ops = {
	.tx_empty		= ms17e_v2_tx_empty,
	.set_mctrl		= ms17e_v2_set_mctrl,
	.get_mctrl		= ms17e_v2_get_mctrl,
	.stop_tx		= ms17e_v2_stop_tx,
	.start_tx		= ms17e_v2_start_tx,
	.stop_rx		= ms17e_v2_stop_rx,
	.enable_ms		= ms17e_v2_enable_ms,
	.break_ctl		= ms17e_v2_break_ctl,
	.startup		= ms17e_v2_startup,
	.shutdown		= ms17e_v2_shutdown,
	.set_termios	= ms17e_v2_set_termios,
	.type			= ms17e_v2_type,
	.release_port	= ms17e_v2_release_port,
	.request_port	= ms17e_v2_request_port,
	.config_port	= ms17e_v2_config_port,
	.verify_port	= ms17e_v2_verify_port,
	.ioctl			= ms17e_v2_ioctl,
};


static int  __devinit ms17e_v2_init(void) {
	int result;

	printk(KERN_NOTICE "Load "MS17E_V2_MODNAME" driver. Version "MS17E_V2_VER"\n");

	result = uart_register_driver(&ms17e_v2_uartdrv);

	if (result) {
		printk(KERN_NOTICE MS17E_V2_MODNAME": Error registering UART driver\n");
		return result;
	}
	if ((result = pci_module_init(&ms17e_v2_driver))) {
		uart_unregister_driver(&ms17e_v2_uartdrv);
		return result;
	}
	return 0;
}

static void  __devexit ms17e_v2_exit(void) {
	printk(KERN_NOTICE"Unload "MS17E_V2_MODNAME" driver\n");
	pci_unregister_driver(&ms17e_v2_driver);
	uart_unregister_driver(&ms17e_v2_uartdrv);
}

/*----------------------------------------------------------
 * PCI related functions
 *----------------------------------------------------------*/

static int __devinit ms17e_v2_init_one(struct pci_dev *pdev,const struct pci_device_id *ent) {
	struct device *dev = (struct device*)&(pdev->dev);
	struct device_driver *drv = (struct device_driver*)(dev->driver);
	struct ms17e_v2_card *card = NULL;
	int err = -1;
	unsigned long iomem_start, iomem_end;
	u32 len;
	char card_name[32], port_name[32];
	struct uart_port *port;

	PDEBUG(debug_init,"start");

	// Setup PCI device
	if (pci_enable_device(pdev))
		return -EIO;
	pci_set_master(pdev);

	// Create device structure & initialize
	if (!(card = kmalloc( sizeof(struct ms17e_v2_card), GFP_KERNEL))) {
		printk(KERN_ERR"%s: error allocating card, PCI device=%02x:%02x.%d\n",MS17E_V2_MODNAME,
				pdev->bus->number,PCI_SLOT(pdev->devfn),PCI_FUNC(pdev->devfn));
		err = -ENOMEM;
		goto pcifree;
	}
	memset((void*)card, 0, sizeof(struct ms17e_v2_card));
	card->pdev = pdev;
	card->number = cur_card_number++;
	snprintf(card_name, 31, "%s%d", MS17E_V2_MODNAME, card->number);
	iomem_start = pci_resource_start(card->pdev, 0);
	iomem_end = pci_resource_end(card->pdev, 0);
	len = iomem_end - iomem_start + 1;

/*
	// Detect type of the card
	switch(pdev->subsystem_device){
    case MR17S_DTE2CH:
        rsdev->type = DTE;
        rsdev->port_quan = 2;
        break;
    case MR17S_DCE2CH:
        rsdev->type = DCE;
        rsdev->port_quan = 2;
        break;
    case MR17S_DTE4CH:
        rsdev->type = DTE;
        rsdev->port_quan = 4;
        break;
    case MR17S_DCE4CH:
        rsdev->type = DCE;
        rsdev->port_quan = 4;
        break;
    default:
		printk(KERN_ERR"%s: error MR17S subtype=%d, PCI device=%02x:%02x.%d\n",MR17S_MODNAME,
                pdev->subsystem_device,
                pdev->bus->number,PCI_SLOT(pdev->devfn),PCI_FUNC(pdev->devfn));
        err = -ENODEV;
        goto rsfree;
    }
    PDEBUG(debug_hw,"Found %s(%d channels)",(rsdev->type == DTE) ? "DTE" : "DCE",rsdev->port_quan);
*/
	// Check I/O Memory window
	if (len != MS17E_V2_IOMEM_SIZE) {
		printk(KERN_ERR"%s: wrong size of I/O memory window: %d != %d, PCI device=%02x:%02x.%d\n",
					MS17E_V2_MODNAME, len, MS17E_V2_IOMEM_SIZE,
					pdev->bus->number,PCI_SLOT(pdev->devfn),PCI_FUNC(pdev->devfn));
		err = -EINVAL;
		goto rsfree;
	}
	// Request & remap memory region
	if (!request_mem_region(iomem_start, MS17E_V2_IOMEM_SIZE, MS17E_V2_MODNAME)) {
		printk(KERN_ERR"%s: error requesting io memory region, PCI device=%02x:%02x.%d\n",
				MS17E_V2_MODNAME,
				pdev->bus->number,PCI_SLOT(pdev->devfn),PCI_FUNC(pdev->devfn));
		goto rsfree;
	}
	card->regs = (void*)ioremap(iomem_start, MS17E_V2_IOMEM_SIZE);

	// Create serial ports
	card->port = (struct uart_port*) kmalloc(sizeof(struct uart_port), GFP_KERNEL);
	if (!card->port) {
		printk(KERN_ERR"%s: error allocating memory for port, PCI device=%02x:%02x.%d\n",
				MS17E_V2_MODNAME,
				pdev->bus->number,PCI_SLOT(pdev->devfn),PCI_FUNC(pdev->devfn));
		err = -ENOMEM;
		goto memfree;
	}

	// Setup port structure
	port = card->port;
	memset(port, 0, sizeof(*port));
//	atomic_set(&card->inuse_cntr,0);
	// port I/O setup
	port->iotype = UPIO_MEM;
	port->iobase = 1;
	port->mapbase = iomem_start;
	port->membase = (void *)card;
	port->irq = pdev->irq;
	// FIFO size
	port->fifosize = 1;
	port->uartclk = MS17E_V2_UARTCLK;
	// UART operations
	port->ops = &ms17e_v2_uart_ops;
	// port index
	port->line = 0;
	// port parent device
	port->dev = &pdev->dev;
	// PORT flags
	port->flags = ASYNC_BOOT_AUTOCONF; //UPF_SKIP_TEST | UPF_BOOT_AUTOCONF | UPF_SHARE_IRQ;

	if ((err = uart_add_one_port(&ms17e_v2_uartdrv, port))) {
		printk(KERN_ERR"%s: error, registering %s%d node, PCI device=%02x:%02x.%d\n",
				MS17E_V2_MODNAME, MS17E_V2_SERIAL_NAME, port->line,
				pdev->bus->number,PCI_SLOT(pdev->devfn),PCI_FUNC(pdev->devfn));
		goto porterr;
	}
	PDEBUG(debug_tty,"Add port "MS17E_V2_SERIAL_NAME"%d, uart_port addr = %p, info = %p",port->line, port, port->info);
	// Symlink to device in sysfs
	snprintf(port_name, 32, MS17E_V2_SERIAL_NAME"%d",port->line);
	if ((err = sysfs_create_link( &(drv->kobj),&(dev->kobj),port_name))) {
		printk(KERN_NOTICE"%s: error in sysfs_create_link\n",__FUNCTION__);
			goto porterr;
	}

	// Save MS17E_V2 internal structure in PCI device struct
	pci_set_drvdata(pdev, card);

	// Request IRQ line
	if ((err = request_irq(pdev->irq, ms17e_v2_interrupt, SA_SHIRQ, card_name, card))) {
		printk(KERN_ERR "Error request IRQ %i\n", pdev->irq);
		goto porterr;
	}

	PDEBUG(debug_init,"end, rsdev = %p", card);

	return 0;

porterr:
	port = card->port;
	snprintf(port_name, 32, MS17E_V2_SERIAL_NAME"%d",port->line);
	sysfs_remove_link(&(drv->kobj),port_name);
	uart_remove_one_port(&ms17e_v2_uartdrv,(struct uart_port*)card->port);
	kfree(card->port);
memfree:
	iounmap(card->regs);
	release_mem_region(iomem_start, MS17E_V2_IOMEM_SIZE);
rsfree:
	kfree(card);
pcifree:
	pci_disable_device(pdev);
	PDEBUG(debug_init,"(!)fail");
	return err;
}

static void __devexit ms17e_v2_remove_one(struct pci_dev *pdev) {
	struct ms17e_v2_card *card = (struct ms17e_v2_card*)pci_get_drvdata(pdev);
	struct device *dev = (struct device*)&(pdev->dev);
	struct device_driver *drv = (struct device_driver*)(dev->driver);
	struct uart_port *port = (struct uart_port*)(card->port);
	char port_name[32];
	unsigned long iomem_start = pci_resource_start(card->pdev, 0);

	free_irq(pdev->irq, card);

	snprintf(port_name, 32, MS17E_V2_SERIAL_NAME"%d", port->line);
	sysfs_remove_link(&(drv->kobj), port_name);
	uart_remove_one_port(&ms17e_v2_uartdrv, port);
	kfree(card->port);
	iounmap(card->regs);
	release_mem_region(iomem_start, MS17E_V2_IOMEM_SIZE);
	kfree(card);
	pci_disable_device(pdev);
	PDEBUG(debug_init,"end");
}

static irqreturn_t ms17e_v2_interrupt(int irq,void *dev_id,struct pt_regs *ptregs) {
	struct ms17e_v2_card *card = (struct ms17e_v2_card*)dev_id;
	struct ms17e_v2_regs_struct *regs = card->regs;
	u8 mask, status;

	PDEBUG(debug_irq,"start, card=%p", card);

	// Read mask & status
	mask = ioread8(&regs->IMR);
	status = ioread8(&regs->SR) & mask;
	// Ack all interrupts
	iowrite8(status,&regs->SR);
	// If no interrupt - return immediately
	if (!status) return IRQ_NONE;

	if (status & TXS) {
		PDEBUG(debug_irq,"TXS");
		ms17e_v2_xmit_byte(card->port);
	}

	if (status & RXS) {
		PDEBUG(debug_irq,"RXS");
		ms17e_v2_recv_byte(card->port,ptregs);
	}

	if (status & RXE) {
		PDEBUG(debug_irq,"RXE");
	}

	return IRQ_HANDLED;

}

//----------------- UART operations -----------------------------------//

static void ms17e_v2_stop_tx(struct uart_port *port) {
	struct ms17e_v2_card *card = (struct ms17e_v2_card *)port->membase;
	// Nothing to do. Transmitter stops automaticaly
	PDEBUG(debug_xmit,"stop_tx");
//	card->block_start_tx = 0;
}

static void ms17e_v2_start_tx(struct uart_port *port) {
	struct ms17e_v2_card *card = (struct ms17e_v2_card *)port->membase;
	PDEBUG(debug_xmit,"start_tx");
//	if (!(card->block_start_tx)) {
//		card->block_start_tx = 1;
		ms17e_v2_xmit_byte(port);
//	} else {
//		PDEBUG(0/*debug_xmit*/,"start_tx second launch!!!");
//	}
}

// ?? Stop receiver when shutting down ??
static void ms17e_v2_stop_rx(struct uart_port *port) {
	// Nothing to do at this moment
	PDEBUG(debug_xmit,"stop_rx");
}

static void ms17e_v2_enable_ms(struct uart_port *port) {
    // Nothing to do at this moment
}

static unsigned int ms17e_v2_get_mctrl(struct uart_port *port) {
	return TIOCM_CAR | TIOCM_DSR | TIOCM_CTS;
}

static void ms17e_v2_set_mctrl(struct uart_port *port, unsigned int mctrl) {
    // Nothing to do at this moment
}

static void ms17e_v2_break_ctl(struct uart_port *port, int break_state) {
    // Nothing to do at this moment
}

static void ms17e_v2_set_termios(struct uart_port *port, struct termios *new, struct termios *old) {
	u32 baud, quot;
	unsigned long flags;

	PDEBUG(debug_hw,"");

	new->c_lflag &= ~ICANON;
	new->c_lflag &= ~(ECHO | ECHOCTL | ECHONL);
	new->c_cc[VMIN] = 1;
	new->c_cc[VTIME] = 0;
	new->c_oflag |= ONLCR;
	new->c_iflag &= ~ICRNL;

	baud = uart_get_baud_rate(port, new, old, 0, port->uartclk);
	quot = uart_get_divisor(port, baud);
//	printk(KERN_ERR "termios new = %p termios old = %p\n", new, old);
	PDEBUG(debug_hw, "BAUD=%u, QUOTE=%u", baud, quot);

	// Locked area - configure hardware
	spin_lock_irqsave(&port->lock, flags);
	uart_update_timeout(port, new->c_cflag, baud);

	port->ignore_status_mask = 0;
	port->read_status_mask   = 0;

	spin_unlock_irqrestore(&port->lock, flags);
}

inline static void ms17e_v2_transceiver_up(struct uart_port *port) {
	struct ms17e_v2_card *card = (struct ms17e_v2_card *)port->membase;

	// enable all interrupts
//	printk(KERN_ERR MS17E_V2_DRVNAME": Enable all interrupts\n");
	iowrite8(TXS | RXS | RXE, &(card->regs->IMR));
//	card->block_start_tx = 0;
}

inline static void ms17e_v2_transceiver_down(struct uart_port *port) {
//	struct ms17e_v2_card *card = (struct ms17e_v2_card *)port->membase;
	// disable all interrupts
//	iowrite8(TXS | RXS | RXE, &(card->regs->IMR));
}

static int ms17e_v2_startup(struct uart_port *port) {
	unsigned long flags;

//	printk(KERN_ERR MS17E_V2_DRVNAME": startup\n");
//	atomic_inc(&card->inuse_cntr);
	spin_lock_irqsave(&port->lock, flags);
	ms17e_v2_transceiver_up(port);
	spin_unlock_irqrestore(&port->lock, flags);

	return 0;
}

static void ms17e_v2_shutdown(struct uart_port *port) {
	unsigned long flags;

//	printk(KERN_ERR MS17E_V2_DRVNAME": shutdown\n");
//	if(atomic_dec_and_test(&card->inuse_cntr) ){
		// shutdown port only if it is last shutdown
		spin_lock_irqsave(&port->lock, flags);
		ms17e_v2_transceiver_down(port);
		spin_unlock_irqrestore(&port->lock, flags);
//	}
}

static const char *ms17e_v2_type(struct uart_port *port) {
	return MS17E_V2_MODNAME;
}

static void ms17e_v2_release_port(struct uart_port *port) {
    // Nothing to do at this moment
}

static int ms17e_v2_request_port(struct uart_port *port) {
    // Nothing to do at this moment
    return 0;
}

static void ms17e_v2_config_port(struct uart_port *port, int flags) {
	struct ms17e_v2_card *card = (struct ms17e_v2_card *)port->membase;

//	printk(KERN_ERR MS17E_V2_DRVNAME": config_port\n");

	if (flags & UART_CONFIG_TYPE) {
		port->type = 100;
	} else {
		port->type = PORT_UNKNOWN;
		return;
	}

	// turn all off
	iowrite8(0x00, &(card->regs->CRA));
	// enable all PHY and power/ also set orange LED mode to manual
	iowrite8(ERST | PRST | LEDM | RXEN, &(card->regs->CRA));
	// turn on all LEDs
	iowrite8(0xFF, &(card->regs->LCR0));
	iowrite8(0xFF, &(card->regs->LCR1));
}

static int ms17e_v2_verify_port(struct uart_port *port, struct serial_struct *ser) {
	// Nothing to do at this moment
	return 0;
}

static unsigned int ms17e_v2_tx_empty(struct uart_port *port) {
	// always empty
//	printk(KERN_ERR MS17E_V2_DRVNAME": tx_empty\n");
	return TIOCSER_TEMT;
}

static int ms17e_v2_ioctl(struct uart_port *port, unsigned int cmd, unsigned long arg) {
//	printk(KERN_ERR MS17E_V2_DRVNAME": ioctl\n");
	return 0;
}

// Hardware related functions

void ms17e_v2_recv_byte(struct uart_port *port, struct pt_regs *ptregs) {
	struct ms17e_v2_card *card = (struct ms17e_v2_card *)port->membase;
	struct tty_struct *tty = port->info->tty;
	unsigned int ch;
	unsigned long flags;

//	printk(KERN_ERR MS17E_V2_DRVNAME": recv_byte\n");

	// Block port. This function can run both
	// in process and interrupt context
	spin_lock_irqsave(&port->lock, flags);
	ch = ioread8(&(card->regs->RDR));
	port->icount.rx++;
	if (uart_handle_sysrq_char(port,ch,ptregs)) return;
//	printk(KERN_ERR MS17E_V2_DRVNAME": recv_byte [%x]\n", ch);
	tty_insert_flip_char(tty, ch, TTY_NORMAL);
	tty_flip_buffer_push(tty);
	spin_unlock_irqrestore(&port->lock,flags);
	return;
}


static void ms17e_v2_xmit_byte(struct uart_port *port) {
	struct ms17e_v2_card *card = (struct ms17e_v2_card *)port->membase;
	struct circ_buf *xmit = &port->info->xmit;
	unsigned long flags;
	u8 tmp = 0;

//	printk(KERN_ERR MS17E_V2_DRVNAME": xmit_byte\n");

	// Block port. This function can run both
	// in process and interrupt context
	spin_lock_irqsave(&port->lock, flags);

	// check that TX buffer is empty
	if (ioread8(&(card->regs->CRA)) & TXEN) {
		PDEBUG(0, "TX buffer not empty!");
		goto exit;
	}

	if (port->x_char) {
		PDEBUG(debug_xmit,"Xmit X CHAR");
		iowrite8(port->x_char, &(card->regs->TDR));
		tmp = ioread8(&(card->regs->CRA));
		iowrite8(tmp | TXEN, &(card->regs->CRA));
		port->icount.tx++;
		port->x_char = 0;
		goto exit;
	}

	// Nothing to xmit OR xmit stopped
	if (uart_circ_empty(xmit) || uart_tx_stopped(port)) {
		PDEBUG(debug_xmit,"CIRC_EMPTY=%d, TX_STOPPED=%d",uart_circ_empty(xmit),uart_tx_stopped(port));
		goto exit;
	}


	iowrite8(xmit->buf[xmit->tail], &(card->regs->TDR));
//	printk(KERN_ERR MS17E_V2_DRVNAME": xmit_byte [%x]\n", xmit->buf[xmit->tail]);
	tmp = ioread8(&(card->regs->CRA));
	iowrite8(tmp | TXEN, &(card->regs->CRA));
	xmit->tail = (xmit->tail + 1) & (UART_XMIT_SIZE - 1);
	port->icount.tx++;

	// Ask for new bytes
	if (uart_circ_chars_pending(xmit) < WAKEUP_CHARS){
	PDEBUG(debug_xmit,"Ack for new characters");
		uart_write_wakeup(port);
	}

exit:
	spin_unlock_irqrestore(&port->lock,flags);
}
