#include "ms17e_main.h"
#include "ms17e_debug.h"
#include "ms17e_net.h"
#include "ms17e_sysfs.h"

int ms17e_net_remove(struct ms17e_card *card)
{
	int i;

	for(i = 0; i < card->if_num; i++)
	{
		ms17e_sysfs_remove(card->ndevs[i]);
		PDEBUG(debug_net, "remove sysfs files for %s: OK", card->ndevs[i]->name);
		unregister_netdev(card->ndevs[i]);
		free_netdev(card->ndevs[i]);
		PDEBUG(debug_net, "unregister and free netdev %s: OK", card->ndevs[i]->name);
	}
	return 0;
}

void ms17e_fe_init( struct net_device *ndev)
{
	ether_setup(ndev);
	ndev->init = NULL;
	ndev->uninit = NULL;
}

int ms17e_net_init(struct ms17e_card *card)
{
	struct net_device *ndev = NULL;
	struct device *dev_dev = (struct device*)&(card->pdev->dev);
	struct device_driver *dev_drv = (struct device_driver*)(dev_dev->driver);
	struct net_local *nl;
	int if_processed, i;
	int ret;
	char name[5];

	for (if_processed = 0; if_processed < card->if_num; if_processed++)
	{
		// allocate network device
		switch (PCI_SLOT(card->pdev->devfn)) {
			case 2:
				sprintf(name, "fe0%i", 7 - if_processed);
			break;
			case 3:
				sprintf(name, "fe1%i", 7 - if_processed);
			break;
			case 4:
				sprintf(name, "fe2%i", 7 - if_processed);
			break;
			case 5:
				sprintf(name, "fe3%i", 7 - if_processed);
			break;
		}
		if (!(ndev = alloc_netdev( sizeof(struct net_local), name, ms17e_fe_init)))
		{
			printk(KERN_NOTICE"error while alloc_netdev #%i\n", if_processed);
			goto exit_unreg_ifs;
		}
		PDEBUG(debug_net, "alloc_netdev %s - OK", ndev->name);

		nl = (struct net_local *)netdev_priv(ndev);
		memset(nl, 0, sizeof(struct net_local));

		nl->dev = &(card->pdev->dev);
		nl->number = if_processed;
		nl->regs = card->regs;
		nl->card = (struct ms17e_card *)card;
		nl->chan_cfg = &(card->channels[if_processed]);

		// network interface registration
		if ((ret = register_netdev(ndev)))
		{
			printk(KERN_NOTICE"ms17e lan: error(%d) while register device %s\n", ret, ndev->name);
			free_netdev( ndev );
			goto exit_unreg_ifs;
		}
		PDEBUG(debug_net, "register netdev - OK");
		card->ndevs[if_processed] = ndev;


		if (ms17e_sysfs_register(ndev))
		{
			printk( KERN_ERR "%s: unable to create sysfs entires for %s\n", MS17E_MODNAME, ndev->name);
			goto exit_unreg_ifs;
		}

		// Create symlink to device in /sys/bus/pci/drivers/mr17h/
		sysfs_create_link( &(dev_drv->kobj),&(dev_dev->kobj),ndev->name);

		PDEBUG(debug_net, "ms17e_sysfs_register - OK");
		sprintf(card->channels[if_processed].name, "%s", ndev->name);
	}

	return 0;
exit_unreg_ifs:
	PDEBUG(debug_error, "Error, if_pocessed = %d", if_processed);
	for(i = 0; i < if_processed; i++)
	{
		sysfs_remove_link(&(dev_drv->kobj), card->channels[i].name);//ndev->name);
		unregister_netdev(card->ndevs[i]);
		free_netdev(card->ndevs[i]);
	}
	ms17e_card_remove(card);
	return -ENODEV;
}
