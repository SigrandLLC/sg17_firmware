/* sg17lan.h: Sigrand SG-17PCI SHDSL modem driver for linux (kernel 2.6.x)
 *
 *	Written 2006-2007 by Artem U. Polyakov <artpol84@gmail.com>
 *
 *	This driver presents SG-17PCI modem
 *	to system as common ethernet-like netcard.
 *
 */

#ifndef SG17MAIN_H
#define SG17MAIN_H

#include "sg17lan.h"
#include "sg17sysfs.h"

//---- Driver initialisation ----//
static int  sg17_init( void );
static void sg17_exit( void );

static int sg17_init_card( struct sg17_card *card );
static void sg17_remove_card( struct sg17_card *card );

//---- PCI adapter related ----//
// First version of module
#define SG17_PCI_VENDOR 	0x55
#define SG17_PCI_DEVICE 	0x9a
// Second version of module
#define SG17v2_PCI_VENDOR 	0x55
#define SG17v2_PCI_DEVICE 	0x99
#define SIOCGLRATE	(SIOCDEVPRIVATE+14)
// Equipment of modules
#define SUBSYS_ID_1CH       0x00
#define SUBSYS_ID_1CH_PWR   0x01
#define SUBSYS_ID_2CH       0x02
#define SUBSYS_ID_2CH_PWR   0x03

static int __devinit sg17_probe_one(struct pci_dev *,
									const struct pci_device_id *);
static void __devexit sg17_remove_one(struct pci_dev *);

//---- Network interface related ----//
void sg17_dsl_init( struct net_device *ndev);
static int __init sg17_probe( struct net_device  *ndev );
static void __devexit sg17_uninit(struct net_device *ndev);
static irqreturn_t sg17_interrupt( int  irq,  void  *dev_id,  struct pt_regs  *regs );
static int sg17_open( struct net_device  *ndev );
static int sg17_close(struct net_device  *ndev);
static struct net_device_stats *sg17_get_stats(struct net_device *ndev);
static int sg17_change_mtu(struct net_device *ndev, int new_mtu);
static int sg17_ioctl(struct net_device *netdev, struct ifreq *ifr, int cmd);
static void sg17_set_mcast_list( struct net_device  *ndev);

/*TODO : correct */
static int sg17_start_xmit( struct sk_buff *skb, struct net_device *ndev );
static int sg17_start_xmit_link( struct sk_buff *skb, struct net_device *ndev );
static void xmit_free_buffs( struct net_device *dev );
static void recv_init_frames( struct net_device *ndev );
static int recv_alloc_buffs( struct net_device *ndev );
static void recv_free_buffs( struct net_device *ndev);
static void sg17_tx_timeout( struct net_device  *ndev );

static void sg17_tranceiver_down(struct net_local *nl);
static void sg17_tranceiver_up(struct net_local *nl);


#ifdef MR17H_UEVENTS
#define USERMODE_HELPER "/sbin/_linkhandler"
static int usermode_link_event(struct net_device *ndev,int link_up);
#else
static inline int
usermode_link_event(struct net_device *ndev,int link_up){ return 0; }
#endif


#endif

