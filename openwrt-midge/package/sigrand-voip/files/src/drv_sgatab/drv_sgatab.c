/****************************************************************************
                  Copyright © 2007  SIGRAND 
                 // St. Martin Strasse 53; 81669 Munich, Germany

   THE DELIVERY OF THIS SOFTWARE AS WELL AS THE HEREBY GRANTED NON-EXCLUSIVE,
   WORLDWIDE LICENSE TO USE, COPY, MODIFY, DISTRIBUTE AND SUBLICENSE THIS
   SOFTWARE IS FREE OF CHARGE.

   THE LICENSED SOFTWARE IS PROVIDED "AS IS" AND SIGRAND EXPRESSLY DISCLAIMS
   ALL REPRESENTATIONS AND WARRANTIES, WHETHER EXPRESS OR IMPLIED, INCLUDING
   WITHOUT LIMITATION, WARRANTIES OR REPRESENTATIONS OF WORKMANSHIP,
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, DURABILITY, THAT THE
   OPERATING OF THE LICENSED SOFTWARE WILL BE ERROR FREE OR FREE OF ANY
   THIRD PARTY CLAIMS, INCLUDING WITHOUT LIMITATION CLAIMS OF THIRD PARTY
   INTELLECTUAL PROPERTY INFRINGEMENT.

   EXCEPT FOR ANY LIABILITY DUE TO WILLFUL ACTS OR GROSS NEGLIGENCE AND
   EXCEPT FOR ANY PERSONAL INJURY INFINEON SHALL IN NO EVENT BE LIABLE FOR
   ANY CLAIM OR DAMAGES OF ANY KIND, WHETHER IN AN ACTION OF CONTRACT,
   TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
   SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 ****************************************************************************
   Module      : drv_sgatab.c
   Description : This file contains the implementation of the pci callbacks and
                 init / exit driver functions.
*******************************************************************************/

/* ============================= */
/* Includes                      */
/* ============================= */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/pci.h>
#include <linux/string.h>
#include <linux/interrupt.h>
#include <linux/proc_fs.h>

#include "ab_ioctl.h"
#include "../tapi/include/ifx_types.h"
#include "../vinetic/include/vinetic_io.h"

/* ============================= */
/* Local Macros & Definitions    */
/* ============================= */

#define PCI_VENDOR_ID_SIGRAND 	0x0055
#define PCI_DEVICE_ID_MR17VOIP8 0x009C

#define MR17VOIP8_ACCESS_MODE 	VIN_ACCESS_PARINTEL_MUX8
#define MR17VOIP8_NAME 		"MR17VOIP8_0.9"
#define MR17VOIP8_REGION_SIZE	256
#define	MR17VOIP8_CHANS_PER_DEV 2


#define DEV_NAME "sgatab"

MODULE_DESCRIPTION("SGATAB driver - www.sigrand.ru");
MODULE_AUTHOR("SIGRAND, Vladimir Luchko <vlad.luch@mail.ru>");
MODULE_LICENSE("GPL");

/* ============================= */
/* Global variable definition    */
/* ============================= */

typedef struct ab_dev_s {
	struct pci_dev * pci_dev;
	struct pci_device_id const * pci_id;
	unsigned char pci_device_enabled;
	struct cdev cdev;
	unsigned long sgatab_adr;
} ab_dev_t;

/* ============================= */
/* Global function declaration   */
/* ============================= */

/* ============================= */
/* Local function declaration    */
/* ============================= */

/* operating system common methods */

/* Local helper functions */
static int  chardev_init  ( void );
static void chardev_undef ( void );
static int  pci_init      ( void );
static void pci_undef     ( void );

/* sets gpio`s proper values and read devices types */
static int SGATAB_gpio_type_init( void );
static int SGATAB_gpio_type_to_user( unsigned long user_data );

int SGATAB_Ioctl(struct inode *inode, struct file *filp,
		unsigned int nCmd, unsigned long nArgument );

/* Driver callbacks */
static struct file_operations sgatab_fops =
{
	.owner = THIS_MODULE,
	.ioctl = SGATAB_Ioctl,
};

static int  __devinit sgatab_probe (struct pci_dev *pci_dev, 
		const struct pci_device_id *pci_id);
static void __devexit sgatab_remove(struct pci_dev *pci_dev);

/* ============================= */
/* Local variable definition     */
/* ============================= */

static struct pci_device_id sgatab_pci_tbl[] __devinitdata = {
	{PCI_DEVICE (PCI_VENDOR_ID_SIGRAND, PCI_DEVICE_ID_MR17VOIP8)},
	{0,}
};

MODULE_DEVICE_TABLE(pci, sgatab_pci_tbl);

static struct pci_driver sgatab_pci_driver = {
      .name     = "sgatab",
      .id_table = sgatab_pci_tbl,
      .probe    = sgatab_probe,
      .remove   = __devexit_p (sgatab_remove),
};

static ab_init_params_t g_parms;
static enum dev_types_e * g_types = NULL;
static unsigned char g_types_GPIO_inited = 0;
static unsigned char g_slot_N = 0;
static u16 g_devices_count = 0;
static ab_dev_t g_ab_dev;

/* ============================= */
/* Local function definition     */
/* ============================= */

/****************************************************************************
Description:
   Configuration / Control for the device.
Arguments:
   inode - pointer to the inode
   filp  - file pointer to be opened
   nCmd  - IoCtrl
   nArgument - additional argument
Return:
   0 or error code
Remarks:
   None.
****************************************************************************/
int SGATAB_Ioctl(struct inode *inode, struct file *filp,
                          unsigned int nCmd, unsigned long nArgument)
{
	int err;
	switch (nCmd) {
		case SGAB_GET_INIT_PRMS:
			if(copy_to_user((void *)nArgument, &g_parms, 
					sizeof(g_parms))){
				err = -EFAULT;
			} else {
				err = 0;
			}
			break;
		case SGAB_BASIC_INIT_TYPES:
			if ( !g_types_GPIO_inited ){
				err = SGATAB_gpio_type_init( );
			}
			err = SGATAB_gpio_type_to_user( nArgument );
			break;
		default:
			printk(KERN_WARNING "%s: Unknown IOCTL command %d\n",
					DEV_NAME, nCmd);
			err = -1;
			break;
	}
	return err;
};

static int 
chardev_init( void )
{
	dev_t dev;
	int err;

	g_ab_dev.cdev.owner = THIS_MODULE;

	err = alloc_chrdev_region( &g_ab_dev.cdev.dev, 0, 1, DEV_NAME );
	dev = g_ab_dev.cdev.dev;
	if ( err ){
		goto ___exit;
	}

	cdev_init ( &g_ab_dev.cdev, &sgatab_fops );
	err = cdev_add ( &g_ab_dev.cdev, dev, 1 );
	if ( err ) {
		goto ___region;
	}
	g_ab_dev.cdev.dev = dev;

	return 0;

___region:
	unregister_chrdev_region(dev, 1);
___exit:
	return err;
};

static void chardev_undef( void )
{
	dev_t dev = g_ab_dev.cdev.dev;
	cdev_del ( &g_ab_dev.cdev );
	unregister_chrdev_region ( dev, 1 );
};

static int pci_init( void )
{
	ab_dev_t * ab_dev = &g_ab_dev;
	int err;

	printk(KERN_INFO "%s: %s(0x%x : 0x%x) board found\n", 
			DEV_NAME, MR17VOIP8_NAME, 
			ab_dev->pci_dev->vendor,
			ab_dev->pci_dev->device);

	ab_dev->sgatab_adr = pci_resource_start(ab_dev->pci_dev, 0);

	if (pci_enable_device(ab_dev->pci_dev)) {
		printk(KERN_ERR "%s: ERROR: can`t enable PCI device\n", 
				DEV_NAME);
		err = -EIO;
		goto pci_init_exit;
	}
	ab_dev->pci_device_enabled = 1;

	pci_read_config_word(ab_dev->pci_dev, PCI_SUBSYSTEM_ID, 
			&g_devices_count);

	g_slot_N = PCI_SLOT(ab_dev->pci_dev->devfn);
	g_types = kmalloc(sizeof(*g_types) * g_devices_count, GFP_KERNEL);
	if ( !g_types) {
		printk(KERN_ERR "%s: ERROR: can`t allocate memory for types\n", 
				DEV_NAME);
		err = -EIO;
		goto pci_init_exit;
	}

	g_parms.nBaseAddress = ab_dev->sgatab_adr;
	g_parms.nIrqNum = ab_dev->pci_dev->irq;
	g_parms.AccessMode = MR17VOIP8_ACCESS_MODE;
	strcpy(g_parms.name, MR17VOIP8_NAME );

	g_parms.region_size = MR17VOIP8_REGION_SIZE;
	g_parms.devs_count = g_devices_count;
	g_parms.chans_per_dev = MR17VOIP8_CHANS_PER_DEV;
	g_parms.first_chan_num = 
			( g_slot_N-1 ) * 
			g_devices_count * 
			MR17VOIP8_CHANS_PER_DEV + 1;

	printk(KERN_INFO "%s: id=%x at bus - %02x slot "
			"- %02x func - %x \n", DEV_NAME,
           		ab_dev->pci_dev->device, 
			ab_dev->pci_dev->bus->number,
           		PCI_SLOT(ab_dev->pci_dev->devfn), 
			PCI_FUNC(ab_dev->pci_dev->devfn));
	printk(KERN_INFO "%s: irq %d, devices count %d, "
			"memory address 0x%lx\n",
			DEV_NAME, ab_dev->pci_dev->irq, 
			g_devices_count, ab_dev->sgatab_adr);

	return 0;

pci_init_exit:
	ab_dev->sgatab_adr = 0;
	return err;
};

static void 
pci_undef ( void )
{
	/* disable the PCI device if it was enabled */
	if( g_ab_dev.pci_device_enabled ) {
		pci_disable_device ( g_ab_dev.pci_dev );
		g_ab_dev.pci_device_enabled = 0;
		if(g_types){
			kfree(g_types);
			g_types = NULL;
		}
	}

	/* remember physical address */
	g_ab_dev.sgatab_adr = 0;

	printk(KERN_INFO "%s: (%s) Resources released!\n", DEV_NAME, __func__);
};

static int SGATAB_gpio_type_init( void )
{
	VINETIC_GPIO_CONFIG ioCfg;
	unsigned short get;
	int io_handle;
	int vin_dev_handle;
	int i;
	int err = 0;

	/* try to set gpio`s proper values */
	for (i = 0; i < g_devices_count; i++){
		vin_dev_handle = VINETIC_OpenKernel ( i, 0 );
		if (vin_dev_handle == -1) {
			printk(KERN_ERR "%s: ERROR : VINETIC_OpenKernel(%d,0) "
					"failed\n", DEV_NAME, i);
			goto SGATAB_gpio_type_init__exit;
		}

		io_handle = VINETIC_GpioReserve (vin_dev_handle,
				 VINETIC_IO_DEV_GPIO_3 | VINETIC_IO_DEV_GPIO_7);
		if (io_handle == 0){
			printk(KERN_ERR "%s: ERROR : VINETIC_GpioReserve(...) "
					"failed\n", DEV_NAME);
			goto SGATAB_gpio_type_init__exit;
		}

		memset (&ioCfg, 0, sizeof(ioCfg));

		ioCfg.nMode = GPIO_MODE_INPUT;
		ioCfg.nGpio = VINETIC_IO_DEV_GPIO_3 | VINETIC_IO_DEV_GPIO_7;
		err = VINETIC_GpioConfig (io_handle, &ioCfg);
		if (err) {
			printk(KERN_ERR "%s: ERROR : VINETIC_GpioConfig(...) "
					"failed\n", DEV_NAME);
			goto SGATAB_gpio_type_init__exit;
		}

		err = VINETIC_GpioGet (io_handle, &get, VINETIC_IO_DEV_GPIO_3);
		if (err) {
			printk(KERN_ERR "%s: ERROR : VINETIC_GpioGet(...) "
					"failed\n", DEV_NAME);
			goto SGATAB_gpio_type_init__exit;
		}
		if(get) {
			g_types[i] = dev_type_FXS;
		} else {
			g_types[i] = dev_type_FXO;
		}
	}

	g_types_GPIO_inited = 1;

	return 0;

SGATAB_gpio_type_init__exit:
	return -EFAULT;
};

static int SGATAB_gpio_type_to_user( unsigned long user_data )
{
	ab_dev_types_t user_types;

	memset (&user_types, 0, sizeof(user_types));

	if(copy_from_user (&user_types, (void *)user_data, sizeof(user_types))){
		printk(KERN_ERR "%s: ERROR : copy_from_user(...) failed\n", 
				DEV_NAME );
		goto SGATAB_gpio_type_to_user__exit;
	}
	
	user_types.devs_count = g_devices_count;

	if(copy_to_user((void *)user_data, &user_types, sizeof(user_types))){
		printk(KERN_ERR "%s: ERROR : copy_to_user(...) failed\n", 
				DEV_NAME );
		goto SGATAB_gpio_type_to_user__exit;
	}
	if(copy_to_user((void *)(user_types.dev_type), g_types, 
			sizeof(*g_types) * g_devices_count)){
		printk(KERN_ERR "%s: ERROR : copy_to_user(...) failed\n", 
				DEV_NAME );
		goto SGATAB_gpio_type_to_user__exit;
	} 
	
	return 0;

SGATAB_gpio_type_to_user__exit:
	return -EFAULT;
};

/**
   The proc filesystem: function to read an entry.
   This function provides information of proc files to the user space

   \return
   length
*/
static int proc_read_sgatab (char *page, char **start, off_t off,
                          int count, int *eof, void *data)
{
	int len;

	int (*fn)(char *buf);

	/* write data into the page */
	if( !data){
		len = 0;
		goto ___exit;
	}

	fn = data;
	len = fn(page);

	if (len <= off+count){
		*eof = 1;
	}
	*start = page + off;
	len -= off;
	if (len > count){
		len = count;
	}
	if (len < 0){
		len = 0;
	}

___exit:
	/* return the data length  */
	return len;
}

/**
  Read the channels information

  \return
  length

*/

static int proc_get_sgatab_channels(char *buf)
{
	int i;
	int start_num;
	int chans_total;
	int dev_id;
	int len = 0;

	chans_total = g_devices_count * MR17VOIP8_CHANS_PER_DEV;
	start_num = ( g_slot_N-1 ) * chans_total + 1;

	for (i = 0; i < chans_total; i++){
		len += sprintf(buf+len, "%02d:",start_num + i);
		if( g_types_GPIO_inited ){
			dev_id = i / MR17VOIP8_CHANS_PER_DEV;
			if(g_types [dev_id] == dev_type_FXS){
				len += sprintf(buf+len, "FXS\n");
			} else if(g_types [dev_id] == dev_type_FXO){
				len += sprintf(buf+len, "FXO\n");
			}
		} else {
			len += sprintf(buf+len, "UNDEFINED\n");
		}
	}

	return len;
}

/**
   Initialize and install the proc entry

\return
   -1 or 0 on success
\remark
   Called by the kernel.
*/
static int proc_install_sgatab_entries( void )
{
	struct proc_dir_entry *driver_proc_node;

	/* install the proc entry */
	driver_proc_node = proc_mkdir( "driver/" DEV_NAME, NULL);
	if (driver_proc_node != NULL){
		create_proc_read_entry( "channels" , S_IFREG|S_IRUGO,
				     driver_proc_node, proc_read_sgatab,
				     (void *)proc_get_sgatab_channels );
	} else {
		return -1;
	}

	return 0;
}

/*******************************************************************************
Description:
   This function calling then PCI device probing.
Arguments:
   pci_dev   - pointer to pci_dev structure of probing device
   pci_id    - pointer to pci_device_id structure of probing device
Return Value:
   0 if ok / -x in error case
Remarks:
   Called by the PCI core.
*******************************************************************************/
static int __devinit sgatab_probe(
		struct pci_dev *pci_dev, 
		const struct pci_device_id *pci_id ) 
{
	int err;

	g_ab_dev.pci_dev = pci_dev;
	g_ab_dev.pci_id = pci_id;
	g_ab_dev.pci_device_enabled = 0;

	err = pci_init ();
	if( err ) {
		printk(KERN_ERR "%s: ERROR : Could not allocate memory for "
				"0x%x:0x%x. EXIT.\n",
				DEV_NAME, pci_id->vendor, pci_id->device);
		goto ___exit;
	}

	return 0;

___exit:
	pci_undef ();
	return err;
};

/*******************************************************************************
Description:
   This function clean up all data and memory after removing PCI device.
Arguments:
   pci_dev   - pointer to pci_dev structure of removed device
Return Value:
   none
Remarks:
   Called by the PCI core.
*******************************************************************************/
static void __devexit sgatab_remove(struct pci_dev *pci_dev)
{
	pci_undef ();
};

/****************************************************************************
Description:
   Initialize the module.
Arguments:
   None.
Return Value:
   0 if ok / -x in error case
Remarks:
   Called by the kernel.
   Register the PCI driver.
****************************************************************************/
static int __init sgatab_module_init(void)
{
	int err;

	/* Register the PCI driver */
	err = pci_register_driver ( &sgatab_pci_driver );
	if ( err < 0 ){
		printk(KERN_ERR "%s: ERROR : Loading module ERROR: can`t "
				"register PCI driver!\n", DEV_NAME);
		goto ___exit;
	}

	err = proc_install_sgatab_entries ();
	if (err){
		printk(KERN_ERR "%s: ERROR : Loading module ERROR: can`t "
				"register PROC entries!\n", DEV_NAME);
		goto ___exit;
	}

	err = chardev_init ();
	if (err) {
		printk(KERN_ERR "%s: ERROR : Could not allocate "
				"character device number. EXIT\n", DEV_NAME);
		goto ___exit;
	}

___exit:
	return err;
}

/****************************************************************************
Description:
   Clean up the module if unloaded.
Arguments:
   None.
Return Value:
   None.
Remarks:
   Called by the kernel.
   Unregister the PCI driver.
****************************************************************************/
static void __exit sgatab_module_exit(void)
{
	pci_unregister_driver( &sgatab_pci_driver );
	printk(KERN_INFO "%s: Unloading module : PCI driver "
			"unregistered!\n", DEV_NAME);

	remove_proc_entry("driver/" DEV_NAME "/channels" ,0);
	remove_proc_entry("driver/" DEV_NAME, 0);

	chardev_undef ();
}

module_init(sgatab_module_init);
module_exit(sgatab_module_exit);

