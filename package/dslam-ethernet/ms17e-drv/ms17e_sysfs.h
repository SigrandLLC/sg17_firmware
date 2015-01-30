#ifndef MS17E_SYSFS_H
#define MS17E_SYSFS_H

#include <linux/netdevice.h>


#define to_net_dev(class) container_of(class, struct net_device, class_dev)

int ms17e_sysfs_register(struct net_device *ndev);
void ms17e_sysfs_remove(struct net_device *ndev);

#endif
