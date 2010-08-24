#include "mam17h_debug.h"
#include "mam17h_main.h"
#include "mam17h_net.h"
#include "mam17h_sysfs.h"

void mam17_dsl_init( struct net_device *ndev)
{
	PDEBUG(debug_netcard, "");
	ether_setup(ndev);
	ndev->init = NULL;//mam17_probe;
	ndev->uninit = NULL;//mam17_uninit;
}

int mam17_net_init(struct mam17_card *card)
{
	struct net_device *ndev = NULL;
	struct device *dev_dev = (struct device*)&(card->pdev->dev);
	struct device_driver *dev_drv = (struct device_driver*)(dev_dev->driver);
	struct net_local *nl;
	int if_processed, i;
	int ret;


	for (if_processed = 0; if_processed < card->if_num; if_processed++)
	{
		// allocate network device 
		if (!(ndev = alloc_netdev( sizeof(struct net_local), "dsl%d", mam17_dsl_init)))
		{
			printk(KERN_NOTICE"error while alloc_netdev #%i\n", if_processed);
			goto exit_unreg_ifs;
		}
		PDEBUG(debug_netcard, "alloc_netdev - %s", ndev->name);
		// set some net device fields
//		ndev->mem_start = (unsigned long)((u8*)card->mem_base +
//										  SG17_HDLC_CH0_MEMOFFS + if_processed*SG17_HDLC_MEMSIZE);
//		ndev->mem_end = (unsigned long)((u8*)ndev->mem_start + SG17_HDLC_MEMSIZE);
//		ndev->irq = card->pdev->irq;
		// device private data initialisation
		nl = (struct net_local *)netdev_priv(ndev);
		memset(nl, 0, sizeof(struct net_local));
		nl->dev = &(card->pdev->dev);
		nl->number = if_processed;
		nl->regs = card->regs;
		nl->card = (struct mam17_card *)card;

//		if( (ch_num = sg17_sci_if2ch(&card->sci,if_processed)) < 0 ){
//			PDEBUG(debug_error,"error(%d) in sg17_sci_if2ch",ch_num);
//			free_netdev( ndev );			
//			goto exit_unreg_ifs;			
//		}
		nl->chan_cfg = &(card->channels[if_processed]);

		// network interface registration
		if ((ret = register_netdev(ndev)))
		{
			printk(KERN_NOTICE"mam17lan: error(%d) while register device %s\n", ret, ndev->name);
			free_netdev( ndev );
			goto exit_unreg_ifs;
		}
		PDEBUG(debug_netcard, "success");
		card->ndevs[if_processed] = ndev;

//		PDEBUG(debug_netcard, "mam17_sysfs_register");
		if (mam17_sysfs_register(ndev))
		{
			printk( KERN_ERR "%s: unable to create sysfs entires\n", ndev->name);
			goto exit_unreg_ifs;
		}
		// Create symlink to device in /sys/bus/pci/drivers/mr17h/
		sysfs_create_link( &(dev_drv->kobj),&(dev_dev->kobj),ndev->name);
		PDEBUG(debug_netcard,"sg17_sysfs_register - success");
		sprintf(card->channels[if_processed].name, "%s", ndev->name);
		
	}

	return 0;
exit_unreg_ifs:
	PDEBUG(debug_error, "Error, if_pocessed = %d", if_processed);
	for(i = 0; i < if_processed; i++)
	{
		sysfs_remove_link(&(dev_drv->kobj), ndev->name);
		unregister_netdev(card->ndevs[i]);
		free_netdev(card->ndevs[i]);
	}
	mam17_card_remove(card);
	return -ENODEV;
}
