/* mr17g_main.c
 *  	Sigrand MR17G E1 PCI adapter driver for linux (kernel 2.6.x)
 *
 *	Written 2008 by Artem Y. Polyakov (artpol84@gmail.com)
 *
 *	This driver presents MR17G modem to OS as common hdlc interface.
 *
 *	This software may be used and distributed according to the terms
 *	of the GNU General Public License.
 *
 */

#include "mr17g.h"
#include "mr17g_oem.h"
#include "mr17g_version.h"
#include "mr17g_main.h"
#include "mr17g_sci.h"
#include "pef22554.h"

// Debug settings
#define DEBUG_ON
#define DEFAULT_LEV 10
#include "mr17g_debug.h"


// mr17g_ioctl
#define SIOCGLRATE	(SIOCDEVPRIVATE+14)

MODULE_DESCRIPTION( "E1 PCI adapter driver Version "MR17G_VER"\n" );
MODULE_AUTHOR( "Maintainer: Polyakov Artem <artpol84@gmail.com>\n" );
MODULE_LICENSE( "GPL" );
MODULE_VERSION(MR17G_VER);

// Register module init/deinit routines 
static unsigned int cur_card_number = 0;
module_init(mr17g_init);
module_exit(mr17g_exit);


/*----------------------------------------------------------
 * Driver initialisation 
 *----------------------------------------------------------*/
static struct pci_device_id  mr17g_pci_tbl[] __devinitdata = {
{ PCI_DEVICE(MR17G_PCI_VEN,MR17G4_PCI_DEV) },
{ PCI_DEVICE(MR17G_PCI_VEN,MR17G8_PCI_DEV) },
{ 0 }
};

MODULE_DEVICE_TABLE( pci, mr17g_pci_tbl );
	
static struct pci_driver  mr17g_driver = {
 name:           MR17G_DRVNAME,
 probe:          mr17g_init_one,
 remove:         mr17g_remove_one,
 id_table:       mr17g_pci_tbl
};


static int  __devinit
mr17g_init( void )
{
	printk(KERN_NOTICE"Load "MR17G_MODNAME" E1 driver. Version "MR17G_VER"\n");
	return pci_module_init( &mr17g_driver );
}

static void  __devexit
mr17g_exit( void ){
	printk(KERN_NOTICE"Unload "MR17G_MODNAME" E1 driver\n");
	pci_unregister_driver( &mr17g_driver );
}

/*----------------------------------------------------------
 * PCI related functions 
 *----------------------------------------------------------*/

static int __devinit
mr17g_init_one( struct pci_dev *pdev,const struct pci_device_id *ent )
{
    struct mr17g_card *card = NULL;
	int err = -1;

	PDEBUG(debug_init,"start");

    // Setup PCI device 
	if( pci_enable_device( pdev ) )
		return -EIO;
	pci_set_master(pdev);
	
    // Init MR17G card
    if( !(card = mr17g_init_card(pdev)) ){
        goto pcifree;
    }
	pci_set_drvdata(pdev,card);

//!!!!!!!!!1
//debug_sci = 40;
//!!!!!!!!!!!!!1

	PDEBUG(debug_init,"end, card = %p",card);
	return 0;
pcifree:	
	pci_disable_device(pdev);
	PDEBUG(debug_init,"(!)fail");
	return err;
}
				
				
static void __devexit 
mr17g_remove_one( struct pci_dev *pdev )
{
	struct mr17g_card *card = (struct mr17g_card*)pci_get_drvdata( pdev );

    PDEBUG(debug_init,"TEST!");
    PDEBUG(debug_init,"card = %p, chip_quan=%d",card,card->chip_quan);

	PDEBUG(debug_init,"start");
    mr17g_shutdown_card(card);
    kfree(card);
	pci_disable_device(pdev);
	PDEBUG(debug_init,"end");
}

// Initialize MR17G hardware 
struct mr17g_card * __devinit
mr17g_init_card(struct pci_dev *pdev)
{
    struct mr17g_card *card = NULL;
    int i,j;

    PDEBUG(debug_init,"start");

    // Setup card structure
	if( !(card = kmalloc( sizeof(struct mr17g_card), GFP_KERNEL ) ) ){
		printk(KERN_ERR"%s: error allocating memory for card ctructure\n",MR17G_MODNAME);
        goto error;
    }

	memset((void*)card,0,sizeof(struct mr17g_card));
	card->pdev = pdev;
    card->number = cur_card_number++;
	card->iomem_start = pci_resource_start( card->pdev, 1 );
	card->iomem_end = pci_resource_end( card->pdev, 1 );
	sprintf(card->name,MR17G_DRVNAME"_%d",card->number);
    switch( card->pdev->device ){
    case MR17G4_PCI_DEV:
        card->chip_quan = 1;
        break;
    case MR17G8_PCI_DEV:
        card->chip_quan = 2;
        break;
    default:
        card->chip_quan = 0;
    }
    if( (card->iomem_end - card->iomem_start) < (card->chip_quan * MR17G_IOMEM_SIZE) ){
		printk(KERN_ERR"%s: wrong size of I/O Memory Window for chip %s\n",MR17G_MODNAME,card->name);
        goto cardfree;
    }    
    card->chips = (struct mr17g_chip*)
            kmalloc( card->chip_quan*sizeof(struct mr17g_card), GFP_KERNEL );
    if( !card->chips ){
		printk(KERN_ERR"%s: error allocating memory for chips ctructures\n",MR17G_MODNAME);
        goto cardfree;
    }

    // Setup chipsets structures on the card
    for(i=0;i<card->chip_quan;i++){
        struct mr17g_chip *chip = (card->chips + i);
        PDEBUG(debug_init,"New chip = %p",chip);
        chip->pdev = card->pdev;
        chip->if_quan = 4; // TODO: in future may be cards vith 2 and 1 interfaces
        // Init MR17G memory window
	    PDEBUG(debug_init,"request memory region ");
        chip->iomem_start = card->iomem_start + i*MR17G_IOMEM_SIZE;
    	if( !request_mem_region(chip->iomem_start,MR17G_IOMEM_SIZE,card->name ) ){
	    	printk(KERN_ERR"%s: error requesting io memory region for %s\n",MR17G_MODNAME,card->name);
            goto memfree;
        }
    	chip->iomem = (struct mr17g_iomem*)ioremap(chip->iomem_start, MR17G_IOMEM_SIZE );
    	PDEBUG(debug_init,"request IRQ");
	    if( request_irq( pdev->irq, mr17g_sci_intr, SA_SHIRQ, card->name, (void*)chip) ){
		    printk(KERN_ERR"%s: error requesting irq(%d) for %s\n",MR17G_MODNAME,pdev->irq,card->name);
    		goto mapfree;
	    }


        // Initialise SCI interface
        if( mr17g_sci_enable(chip) ){
            goto mapfree;
        }

        // Setup PEF22554 basic general registers
        if( pef22554_basic_card(chip) ){
            goto mapfree;
        }

        // Initialise network interfaces
        if( mr17g_net_init(chip) )  
            goto mapfree;
        
        // Start chip monitoring
        mr17g_sci_monitor((void*)chip);
    }
    PDEBUG(debug_init,"Return card = %p",card);
    return card;

mapfree:
    {
        struct mr17g_chip *chip = (card->chips + i);
        PDEBUG(debug_init,"memfree: i=%d",i);
        iounmap( (void*)chip->iomem );
  	    release_mem_region( chip->iomem_start, MR17G_IOMEM_SIZE );
	    free_irq(chip->pdev->irq, chip);
    }
memfree:
    for(j=0;j<i;j++){
        struct mr17g_chip *chip = (card->chips + j);
        mr17g_net_uninit(chip);
        mr17g_sci_disable(chip);
        iounmap( (void*)chip->iomem );
    	release_mem_region( chip->iomem_start, MR17G_IOMEM_SIZE );
	    free_irq(chip->pdev->irq, chip);
    }        
    kfree(card->chips);
cardfree:
    kfree(card);
error:
    PDEBUG(debug_init,"(!)fail");
    return NULL;
}

static void  __devexit
mr17g_shutdown_card(struct mr17g_card *card)
{
    int i;
    PDEBUG(debug_init,"start, card = %p",card);
    for(i=0;i<card->chip_quan;i++){
        struct mr17g_chip *chip = (card->chips + i);
        PDEBUG(debug_init,"start, card = %p, chip = %p",card,chip);
        mr17g_sci_endmon(chip);
        mr17g_net_uninit(chip);
        mr17g_sci_disable(chip);
        iounmap( (void*)chip->iomem );
    	release_mem_region( card->iomem_start + i*MR17G_IOMEM_SIZE, MR17G_IOMEM_SIZE );
	    free_irq(chip->pdev->irq, chip);
    }
    kfree(card->chips);
    PDEBUG(debug_init,"end");
}

