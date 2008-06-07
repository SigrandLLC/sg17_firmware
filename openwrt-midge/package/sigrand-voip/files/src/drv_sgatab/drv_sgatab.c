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
/* *
 * tag__ what about race condition between pci_probe and ioctl ??
 *
 * */
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
#define MR17VOIP8_REGION_SIZE	256


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

static int SGATAB_boards_presence( unsigned long user_data );
static int SGATAB_init_params( unsigned long user_data );

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
static long g_slot_N = BOARD_SLOT_FREE;
static ab_dev_t g_ab_dev;
static ab_boards_presence_t g_bp;

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
		case SGAB_GET_BOARDS_PRESENCE:
			err = SGATAB_boards_presence (nArgument);
			break;
		case SGAB_GET_INIT_PARAMS:
			err = SGATAB_init_params (nArgument);
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

static int 
pci_init( void )
{
	ab_dev_t * ab_dev = &g_ab_dev;
	u16 sub_id = 0;
	int i;
	int err;

	printk(KERN_INFO "%s:(0x%x : 0x%x) board found\n", 
			DEV_NAME, 
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

	pci_read_config_word(ab_dev->pci_dev, PCI_SUBSYSTEM_ID, &sub_id);

	g_slot_N = PCI_SLOT(ab_dev->pci_dev->devfn);

	g_parms.nBaseAddress = ab_dev->sgatab_adr;
	g_parms.nIrqNum = ab_dev->pci_dev->irq;
	g_parms.AccessMode = MR17VOIP8_ACCESS_MODE;
	g_parms.region_size = MR17VOIP8_REGION_SIZE;
	g_parms.first_chan_idx = 
			g_slot_N * DEVS_PER_BOARD_MAX * CHANS_PER_DEV + 1;

	for (i=0; i<DEVS_PER_BOARD_MAX; i++){
		dev_type_t dt = (sub_id >> (i*DEV_TYPE_LENGTH)) & DEV_TYPE_MASK;
		g_parms.devices [i] = dt;
	}

	printk(KERN_INFO "%s: id=%x at bus - %02x slot - %02x func - %x\n", 
			DEV_NAME, ab_dev->pci_dev->device, 
			ab_dev->pci_dev->bus->number,
           		PCI_SLOT(ab_dev->pci_dev->devfn), 
			PCI_FUNC(ab_dev->pci_dev->devfn));
	printk(KERN_INFO "%s: irq %d, subsystem id %d, memory address 0x%lx\n",
			DEV_NAME, ab_dev->pci_dev->irq, sub_id, 
			ab_dev->sgatab_adr);

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
	}

	/* remember physical address */
	g_ab_dev.sgatab_adr = 0;

	printk(KERN_INFO "%s: (%s) Resources released!\n", DEV_NAME, __func__);
};

static int 
SGATAB_boards_presence( unsigned long user_data )
{
	int i;

	for (i=0; i<BOARDS_MAX; i++){
		g_bp.slots [i] = BOARD_SLOT_FREE;
	}

	g_bp.slots [0] = g_slot_N;

	if(copy_to_user((void *)user_data, &g_bp, sizeof(g_bp))){
		printk(KERN_ERR "%s: ERROR : copy_to_user(...) failed\n", 
				DEV_NAME );
		goto __exit_fail;
	}

	return 0;
__exit_fail:
	return -1;
};

static int 
SGATAB_init_params( unsigned long user_data )
{
	ab_init_params_t ip;

	if(copy_from_user (&ip, (void *)user_data, sizeof(ip))){
		printk(KERN_ERR "%s: ERROR : copy_from_user(...) failed\n", 
				DEV_NAME );
		goto __exit_fail;
	}

	if( ip.requested_board_slot == BOARD_SLOT_FREE ||
			ip.requested_board_slot != g_bp.slots [0]){
		printk(KERN_ERR "%s: ERROR:wrong req_slot %ld should be %ld\n", 
				DEV_NAME, ip.requested_board_slot, 
				g_bp.slots [0] );
		goto __exit_fail;
	}

	g_parms.requested_board_slot = ip.requested_board_slot;

	if(copy_to_user((void *)user_data, &g_parms, sizeof(g_parms))){
		printk(KERN_ERR "%s: ERROR : copy_to_user(...) failed\n", 
				DEV_NAME );
		goto __exit_fail;
	}

	return 0;
__exit_fail:
	return -1;
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
	int len = 0;

	chans_total = DEVS_PER_BOARD_MAX * CHANS_PER_DEV;
	start_num = g_parms.first_chan_idx;

	for (i = 0; i < chans_total; i++){
		switch(g_parms.devices[i/CHANS_PER_DEV]){
			case dev_type_ABSENT:
				break;
			case dev_type_FXO:
				len += sprintf(buf+len, "%02d:FXO\n",
						start_num + i);
				break;
			case dev_type_FXS:
				len += sprintf(buf+len, "%02d:FXS\n",
						start_num + i);
				break;
			case dev_type_RESERVED:
				len += sprintf(buf+len, "%02d:RESERVED\n",
						start_num + i);
				break;
		}
	}
	return len;
};

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

