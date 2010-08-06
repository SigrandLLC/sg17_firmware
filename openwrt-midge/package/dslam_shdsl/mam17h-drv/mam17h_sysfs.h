#ifndef MAM_17H_SYSFS_H
#define MAM_17H_SYSFS_H

#define MAX_RATE 15296


#define to_net_dev(class) container_of(class, struct net_device, class_dev)

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/config.h>
#include <linux/vermagic.h>
#include <linux/kobject.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>


#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/pci.h>
#include <linux/interrupt.h>
#include <linux/firmware.h>
#include <asm/io.h>

#include <linux/ioport.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/config.h>
#include <linux/vermagic.h>
#include <linux/version.h>

#include <asm/types.h>
#include <asm/byteorder.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/uaccess.h>

#include <linux/ioport.h>
#include <linux/interrupt.h>
#include <linux/delay.h>

#include <asm/io.h>
#include <asm/types.h>
#include <linux/kernel.h>
#include <linux/ioport.h>
#include <linux/wait.h>
#include <linux/interrupt.h>
#include <linux/kobject.h>

int mam17_sysfs_register(struct net_device *ndev);
void mam17_sysfs_remove(struct net_device *ndev);

#endif
