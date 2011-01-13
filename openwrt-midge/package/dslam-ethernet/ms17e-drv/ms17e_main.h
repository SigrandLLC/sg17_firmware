#ifndef MS17E_H
#define MS17E_H

#include <linux/pci.h>
#include <linux/proc_fs.h>

#define DRIVER_VERSION "0.1"
#define MS17E_MODNAME "ms17e"
#define CARD_NAME "ms17e_card" // + card number (card->number)

// TODO: поставить реальные цифры
#define MS17E_PCI_VENDOR  0x55
#define MS17E_PCI_DEVICE  0x94
#define MS17E4_SUBSYSTEM_DEVICE  1
#define MS17E4P_SUBSYSTEM_DEVICE 5
#define MS17E8_SUBSYSTEM_DEVICE  2
#define MS17E8P_SUBSYSTEM_DEVICE 7

#define MS17E_IOMEM_SIZE 0x1000

/* bits descriptions of ms17e resiters */
// CRA
#define WR   0x01
#define RD   0x02
#define LEDM 0x10
#define PRST 0x40
#define ERST 0x80
// SR && IMR
#define WRS  0x01
#define RDS  0x02
#define WRE  0x04
#define RDE  0x08
#define PINT 0x40
// SAR
#define RW        0x01
#define POE_CHIP0 0x00
#define POE_CHIP1 0x02
#define POE_ADDR  0x40

// defines for led_control function
#define LINK          0
#define POWER         1
#define OFF           0
#define BLINK         1
#define FAST_BLINK    2
#define ON            3

#ifndef IO_READ_WRITE
#       define iowrite8(val,addr)  writeb(val,addr)
#       define iowrite16(val,addr)  writeb(val,addr)
#       define iowrite32(val,addr)  writel(val,addr)
#       define ioread8(addr) readb(addr)
#       define ioread16(addr) readb(addr)
#       define ioread32(addr) readl(addr)
#endif

struct regs_struct {
	u8 CRA, SR, IMR, reserved, SAR, RAR, WDR, RDR, LCR0, LCR1;
};

struct channel {
	unsigned on;
	char name[5]; // имя интерфейса (feXX)
	u8 pwr_src:1; // есть источник питания или нет
};

struct ms17e_card {
	u8 number:3; // номер pci карты
	struct pci_dev *pdev;
	char card_name[12];  // имя карты (ms17e_cardX)
	u8 if_num:4;
	u8 pwr_source:1;
	struct channel channels[8];
	struct net_device *ndevs[8];
	struct regs_struct *regs;
	wait_queue_head_t wait_read;
	wait_queue_head_t wait_write;
	struct work_struct work;
};

void ms17e_card_remove(struct ms17e_card *card);

#endif
