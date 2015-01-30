/******************************************************************************

                               Copyright (c) 2006
                            Infineon Technologies AG
                     Am Campeon 1-12; 81726 Munich, Germany

  THE DELIVERY OF THIS SOFTWARE AS WELL AS THE HEREBY GRANTED NON-EXCLUSIVE,
  WORLDWIDE LICENSE TO USE, COPY, MODIFY, DISTRIBUTE AND SUBLICENSE THIS
  SOFTWARE IS FREE OF CHARGE.

  THE LICENSED SOFTWARE IS PROVIDED "AS IS" AND INFINEON EXPRESSLY DISCLAIMS
  ALL REPRESENTATIONS AND WARRANTIES, WHETHER EXPRESS OR IMPLIED, INCLUDING
  WITHOUT LIMITATION, WARRANTIES OR REPRESENTATIONS OF WORKMANSHIP,
  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, DURABILITY, THAT THE
  OPERATING OF THE LICENSED SOFTWARE WILL BE ERROR FREE OR FREE OF ANY THIRD
  PARTY CLAIMS, INCLUDING WITHOUT LIMITATION CLAIMS OF THIRD PARTY INTELLECTUAL
  PROPERTY INFRINGEMENT.

  EXCEPT FOR ANY LIABILITY DUE TO WILFUL ACTS OR GROSS NEGLIGENCE AND EXCEPT
  FOR ANY PERSONAL INJURY INFINEON SHALL IN NO EVENT BE LIABLE FOR ANY CLAIM
  OR DAMAGES OF ANY KIND, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
  ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
  DEALINGS IN THE SOFTWARE.

******************************************************************************/

/**
   \file drv_tapi_linux.c
   Date: 2006-01-10
   This file contains the implementation of High-Level TAPI Driver,
   Linux part

   The implementaion mainly includes the registration part, using which
   the low-level drivers register themselves. During the registration
   the data structures are initialised,appropriate device nodes are
   created and the registered with the kernel.
*/

/* ============================= */
/* Includes                      */
/* ============================= */

#include "drv_api.h"
#include "linux/init.h"
#include "linux/types.h"
#include "linux/unistd.h"
#ifdef LINUX_2_6
#include <linux/workqueue.h>  /* LINUX 2.6 We need work_struct */
#include <linux/device.h>
#undef   CONFIG_DEVFS_FS
#else
#include "linux/tqueue.h"
#endif /* LINUX_2_6 */

#ifdef CONFIG_DEVFS_FS
#include "linux/devfs_fs_kernel.h"
#endif /* CONFIG_DEVFS_FS */

#include "drv_tapi.h"
#include "drv_tapi_api.h"
#include "drv_tapi_ll_interface.h"
#include "drv_tapi_event.h"
#include "drv_tapi_errno.h"
#include "drv_tapi_stream.h"
#include "drv_tapi_polling.h"
#include "drv_tapi_fxo_ll_interface.h"
#include "drv_tapi_ioctl.h"

#ifdef KPI_SUPPORT
#include "drv_tapi_kpi.h"
#endif
#ifdef TAPI_CID
#include "drv_tapi_cid.h"
#endif
#ifndef MODULE
#include <linux/module.h>
#include <linux/kernel.h>
#endif

#undef DEBUG_TAPI_IOCTLS

#ifdef LINUX
#ifdef LINUX_2_6
#include "linux/version.h"
#ifndef UTS_RELEASE
#include "linux/utsrelease.h"
#endif /* UTC_RELEASE */
#endif /* LINUX_2_6 */

#define TAPI_IOCTL_STACKSIZE                 4000 /* allow some overhead 4 k */

/* ============================= */
/* Global Declarations           */
/* ============================= */

/* ============================= */
/* Local Functions               */
/* ============================= */
IFX_LOCAL IFX_void_t TAPI_timer_call_back (unsigned long arg);
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,20))
IFX_LOCAL IFX_void_t TAPI_tqueue (IFX_void_t *pWork);
#else /* for Kernel newer or equal 2.6.20 */
IFX_LOCAL IFX_void_t TAPI_tqueue (struct work_struct *pWork);
#endif
IFX_LOCAL int TAPI_OS_InstallPollQueue    (TAPI_DEV *pTapiDev, void *fileptr, void *poll_entry);
IFX_LOCAL IFX_int32_t TAPI_SelectCh       (TAPI_CHANNEL* pTapiCh, IFX_int32_t node, IFX_int32_t opt);

static int ifx_tapi_open (struct inode *inode, struct file *filp);
static int ifx_tapi_release (struct inode *inode, struct file *filp);
static ssize_t ifx_tapi_write(struct file *filp, const char *buf, size_t count, loff_t * ppos);
static ssize_t ifx_tapi_read(struct file * filp, char *buf, size_t length, loff_t * ppos);
static int ifx_tapi_ioctl(struct inode *inode, struct file *filp, unsigned int nCmd, unsigned long nArgument);
static unsigned int ifx_tapi_poll (struct file *filp, poll_table *table);


#if CONFIG_PROC_FS
static int proc_read_tapi(char *page, char **start, off_t off,
                              int count, int *eof, void *data);
static int proc_get_tapi_version(char *buf);
static int proc_get_tapi_status(char *buf);
static int proc_get_tapi_registered_drivers(char *buf);
static int proc_install_tapi_entries(void);
#endif /* CONFIG_PROC_FS */

/** install parameter debug_level: LOW (1), NORMAL (2), HIGH (3), OFF (4) */
#ifdef ENABLE_TRACE
#ifdef DEBUG_TAPI
static IFX_uint32_t debug_level = DBG_LEVEL_LOW;
#else
static IFX_uint32_t debug_level = DBG_LEVEL_NORMAL;
#endif /* DEBUG_TAPI */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,17))
MODULE_PARM(debug_level, "i");
#else
module_param(debug_level, uint, 0);
#endif /* < 2.6.17 */
MODULE_PARM_DESC(debug_level, "set to get more (1) or fewer (4) debug outputs");
#endif /* ENABLE_TRACE */
/** The driver callbacks which will be registered with the kernel*/
static struct file_operations tapi_fops =
{
#ifdef MODULE
   owner:
      THIS_MODULE,
#endif
   read:
      ifx_tapi_read,
   write:
      ifx_tapi_write,
   poll:
      ifx_tapi_poll,
   ioctl:
      ifx_tapi_ioctl,
   open:
      ifx_tapi_open,
   release:
      ifx_tapi_release
};


/* ============================= */
/* Global function definition    */
/* ============================= */

/**
   Register the low level driver at the system
\param
   pLLDrvCtx Pointer to device driver context passed by low-level driver
   pHLDrvCtx Pointer to high level drvier context
\return
   IFX_SUCCESS
\remarks
   This function is called when the low-level driver is inserted and registers
   the device nodes for the low-level driver being added
*/
IFX_return_t TAPI_OS_RegisterLLDrv (IFX_TAPI_DRV_CTX_t* pLLDrvCtx,
                                    IFX_TAPI_HL_DRV_CTX_t* pHLDrvCtx)
{
   IFX_int32_t ret = 0;
   unsigned int maxDevices, maxFds, majorNumber, minorBase;
   IFX_char_t   *pRegDrvName   = IFX_NULL;

#ifdef CONFIG_DEVFS_FS
   int i = 0, j = 0;
   devfs_handle_t devHndl;
#endif
   /* copy registration info from Low level driver */
   majorNumber = pLLDrvCtx->majorNumber;
   minorBase   = pLLDrvCtx->minorBase;
   maxDevices  = pLLDrvCtx->maxDevs;
   /* max file descriptors per device /dev/fd10 + /dev/fd11..1x
      at the moment most of the ll driver report a wrong value for
      pLLDrvCtx->maxChannels of 4 which doesn't reflect the 8 PCM channels
      Therefore in we use pLLDrvCtx->maxChannels only in case of
      DuSLIC-xT (2). For all others we stick to 8 till cleanup.
      This is no limitation for the system. */
   maxFds      = (pLLDrvCtx->maxChannels == 2) ?
                  pLLDrvCtx->maxChannels : (minorBase -2);
   /* get a local handle for the driver name storage (used for driver
      registration) */
   pRegDrvName = pHLDrvCtx->registeredDrvName;

#ifdef CONFIG_DEVFS_FS
#ifdef TAPI_ONE_DEVNODE
   sprintf (pRegDrvName, "%.8s", pLLDrvCtx->devNodeName);
   /* Register the device node with the kernel */
   devHndl = devfs_register (IFX_NULL, pRegDrvName,
                             DEVFS_FL_DEFAULT, majorNumber,
                             0,
                            (S_IFCHR|S_IRUGO|S_IWUGO),
                            &tapi_fops , (void *)0);
   if (devHndl == IFX_NULL)
   {
      TRACE( TAPI_DRV, DBG_LEVEL_HIGH,
           ("IFX_TAPI_Register_LL_Drv: unable to add device /dev/%s to dev_fs\n\r",
             pLLDrvCtx->drvName));
   }
   /* Store the device handle returned by kernel */
   pHLDrvCtx->TAPI_devfs_handle[0] = devHndl;
#endif /* TAPI_ONE_DEVNODE */

   for (i = 0; i < maxDevices; ++i)
   {
      for (j = 0; j <= maxFds; j++)
      {
         /* limit devNodeName to 8 characters */
         sprintf (pRegDrvName, "%.8s%d%d", pLLDrvCtx->devNodeName, i+1, j);
         /* Register the device node with the kernel */
         devHndl = devfs_register(IFX_NULL, pRegDrvName,
                                  DEVFS_FL_DEFAULT, majorNumber,
                                  (i+1)*minorBase + j,
                                  (S_IFCHR|S_IRUGO|S_IWUGO),
                                  &tapi_fops , (void *)0);
         if (devHndl == IFX_NULL)
         {
            TRACE( TAPI_DRV, DBG_LEVEL_HIGH,
                 ("IFX_TAPI_Register_LL_Drv: unable to add device /dev/%s to dev_fs\n\r",
                   pLLDrvCtx->drvName));
            return IFX_ERROR;
         }
         /* Store the device handle returned by kernel */
         pHLDrvCtx->TAPI_devfs_handle[i*minorBase + j] = devHndl;
      }
   }
#endif /* CONFIG_DEVFS_FS */
   /* limit devNodeName to 8 characters */
   sprintf (pRegDrvName, "ifx_tapi (%.8s)", pLLDrvCtx->devNodeName);

   /* Register the character device */
   ret = register_chrdev (majorNumber, pRegDrvName, &tapi_fops);
   if (ret < 0)
   {
      TRACE( TAPI_DRV, DBG_LEVEL_HIGH,
           ("IFX_TAPI_Register_LL_Drv: unable to register chrdev major number "
            "%d\n\r", majorNumber));
      return IFX_ERROR;
   }

#if 0
#ifdef LINUX_2_6
   tapi_driver.name = "VINETIC-CPE";
   tapi_driver.bus  = &platform_bus_type;
   driver_register(&tapi_driver);
   tapi_device.name = "VINETIC-CPE";
   tapi_device.id   = 0;
   tapi_device.dev.driver = &tapi_driver;
   platform_device_register(&tapi_device);
/* device_bind_driver(&tapi_device.dev); */
#endif /* LINUX_2_6 */
#endif /* 0 */

   return IFX_SUCCESS;
}

/**
   UnRegister the low level driver from the system
\param
   pLLDrvCtx Pointer to device driver context passed by low-level driver
   pHLDrvCtx Pointer to high level drvier context
\return
   IFX_ERROR on error, otherwise IFX_SUCCESS
\remarks
   This function will be called when the low-level driver is removed from the
   kernel and unregisters the device nodes from the kernel */
IFX_return_t TAPI_OS_UnregisterLLDrv (IFX_TAPI_DRV_CTX_t* pLLDrvCtx,
                                      IFX_TAPI_HL_DRV_CTX_t* pHLDrvCtx)
{
   int minorBase;
#ifdef CONFIG_DEVFS_FS
   int j, i;
#endif /* CONFIG_DEVFS_FS */

   minorBase = pLLDrvCtx->minorBase;
#ifdef CONFIG_DEVFS_FS
   for (i=0; i < pLLDrvCtx->maxDevs; ++i)
   {
      /* remove vinetic devices from dev fs */
      for (j = 0; j < minorBase-1; j++ )
      {
         if (pHLDrvCtx->TAPI_devfs_handle[i*minorBase + j])
         {
            devfs_unregister(pHLDrvCtx->TAPI_devfs_handle[i*minorBase + j]);
         }
      }
   }
#endif /* CONFIG_DEVFS_FS */
   unregister_chrdev (pLLDrvCtx->majorNumber, pHLDrvCtx->registeredDrvName);

   return IFX_SUCCESS;
}

/**
   Open the device.

   At the first time:
   - Initialize the high-level TAPI device structure
   - Call the low-level function to initialise the low-level device structure
   - Initialize the high-level TAPI channel structure
   - Call the low-level function to initialise the low-level channel structure

   \param inode pointer to the inode
   \param filp pointer to the file descriptor

   \return
   0 - if no error,
   otherwise error code
*/
static int ifx_tapi_open (struct inode *inode, struct file *filp)
{
   TAPI_DEV                *pTapiDev            = IFX_NULL;
   IFX_TAPI_DRV_CTX_t      *pDrvCtx             = IFX_NULL;
   int result, minorNum;
   unsigned int dev = 0, ch = 0;
   unsigned int majorNum;


   minorNum = MINOR(inode->i_rdev);
   majorNum = MAJOR(inode->i_rdev);
   TRACE(TAPI_DRV, DBG_LEVEL_LOW, ("ifxTAPI open %d/%d\n\r", majorNum, minorNum));

   /* Get the index for the device driver context based on major number */
   pDrvCtx = IFX_TAPI_Get_Device_Driver_Context (majorNum);

   if (pDrvCtx == IFX_NULL)
   {
      printk (KERN_INFO "tapi_open: error getting index of drvctx\n");
      goto OPEN_ERROR;
   }

   /* get device / channel number */
#ifdef TAPI_ONE_DEVNODE
   if (minorNum == 0)
      /* remember it for th poll call */
      pDrvCtx->pTapiDev[0].bSingleFd = IFX_TRUE;
   else
#endif /* TAPI_ONE_DEVNODE */
   {
      dev = (minorNum / pDrvCtx->minorBase) - 1;
      ch   = minorNum % pDrvCtx->minorBase;
   }

   /* check the device number */
   if (dev >= pDrvCtx->maxDevs)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
           ("TAPI_DRV: max. device number exceed\n\r"));
      goto OPEN_ERROR;
   }

   /* check the channel number */
   if (ch > pDrvCtx->maxChannels)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
          ("TAPI_DRV: max. channel number exceed\n\r"));
      goto OPEN_ERROR;
   }

   pTapiDev = &(pDrvCtx->pTapiDev[dev]);

    /* check if first open on this device */
   if (pTapiDev->bInitialized == IFX_FALSE)
   {
      /* also increments the use counter */
      if (TAPI_Init_Dev (pDrvCtx, dev) != TAPI_statusOk)
         goto OPEN_ERROR;
   }
   else
   {
      /* Increment the Usage counter */
      pTapiDev->nInUse++;
   }

   if (ch == 0)
   {
      /* Save the device pointer */
      filp->private_data = pTapiDev;
   }
   else
   {
      /* Save the channel pointer */
      filp->private_data = &(pTapiDev->pTapiChanelArray[ch-1]);
   }

   /* Call the Low level Device specific open routine */
   if (ptr_chk(pDrvCtx->Open, "pDrvCtx->Open"))
   {
      if (pDrvCtx->Open((IFX_int32_t)ch, IFX_NULL) != IFX_SUCCESS)
      {
         TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
               ("Open LL channel failed for ch: %d\n", ch));
         return -1;
      }
   }

   /* increment module use counter */
#ifdef MODULE
   MOD_INC_USE_COUNT;
#endif
   return IFX_SUCCESS;

OPEN_ERROR:

   if (pDrvCtx->pTapiDev != IFX_NULL)
   {
      IFXOS_FREE (pDrvCtx->pTapiDev);
   }
   result = -ENODEV;
   return result;

}

/**
   Close a file.

   This function gets called when a close is called on the file descriptor.
   Both types device and channel file descriptors are handled here. The
   function decrements the usage count, and calls the LL release routine.

   \param inode Pointer to the inode.
   \param filp  Pointer to the file descriptor.

   \return
      -  0  on success
      - -1  on error
*/
static int ifx_tapi_release (struct inode *inode, struct file *filp)
{
   /* nChannel is the first field in both TAPI_DEV and TAPI_CHANNEL */
   IFX_uint8_t nChannel = ((TAPI_DEV *) filp->private_data)->nChannel;
   IFX_TAPI_DRV_CTX_t  *pDrvCtx;
   TAPI_DEV            *pTapiDev;

   TRACE(TAPI_DRV, DBG_LEVEL_LOW, ("ifxTAPI close %d/%d tapi-ch %d\n\r",
         MAJOR(inode->i_rdev), MINOR(inode->i_rdev), nChannel));

   if (nChannel != IFX_TAPI_DEVICE_CH_NUMBER)
   {
      /* closing channel file descriptor */
      TAPI_CHANNEL *pTapiCh = (TAPI_CHANNEL *) filp->private_data;
      pTapiDev = pTapiCh->pTapiDevice;
      pDrvCtx  = pTapiDev->pDevDrvCtx;

      /* decrement the use counters */
      if (pTapiCh->nInUse > 0)
      {
         pTapiCh->nInUse--;
      }
      if (pTapiDev->nInUse > 0)
      {
         pTapiDev->nInUse--;
      }
   }
   else
   {
      /* closing device file descriptor */
      pTapiDev = (TAPI_DEV *) filp->private_data;
      pDrvCtx = pTapiDev->pDevDrvCtx;

      /* decrement the use counter */
      if (pTapiDev->nInUse > 0)
      {
         pTapiDev->nInUse--;
      }
   }

   /* Call the Low-level Device specific release routine. */
   /* Not having such a function is not an error. */
   if (ptr_chk(pDrvCtx->Release, "pDrvCtx->Release"))
   {
      if (pDrvCtx->Release((IFX_int32_t)nChannel,
                           IFX_NULL, pTapiDev->pLLDev) != IFX_SUCCESS)
      {
         TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
               ("Release LL channel failed for ch: %d\n", nChannel));
         return -1;
      }
   }

   /* Disable the interrupt */
   /* untested and currently not used.
   if (pTapiDev->nInUse == 0)
   {
     pDrvCtx->Irq.LL_IrqDisable (pTapiDev->pLLDev);
   }
   */

   /* decrement use counter */
#ifdef MODULE
   MOD_DEC_USE_COUNT;
#endif

   return 0;
}


/**
   Writes data to the device.

   \param filp pointer to the file descriptor
   \param buf source buffer
   \param count data length
   \param ppos unused

   \return
   length or a negative error code
   \remark
   Currently the low-level write function is called transparently
*/
static ssize_t ifx_tapi_write (struct file *filp, const char *buf,
                                 size_t count, loff_t * ppos)
{
   TAPI_CHANNEL *pTapiCh = (TAPI_CHANNEL*) filp->private_data;
   IFX_TAPI_DRV_CTX_t *pDrvCtx = (IFX_TAPI_DRV_CTX_t*) pTapiCh->pTapiDevice->pDevDrvCtx;
   IFX_TAPI_LL_CH_t* pCh = pTapiCh->pLLChannel;
   ssize_t size=0;

   /* Call the Low level driver's write function */
   if (pDrvCtx->Write == IFX_NULL)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_LOW, ("TAPI_DRV: pDrvCtx->LL_Write is NULL !!!\n\r"));
      return 0;
   }
   size = pDrvCtx->Write (pCh, buf, count, (IFX_int32_t*)ppos);
   return size;
}

/**
   Reads data from the device.

   \param filp pointer to the file descriptor
   \param buf destination buffer
   \param count max size of data to read
   \param ppos unused

   \return
   len - data length
   \remark
   Currently the low-level read function is called transparently
*/
static ssize_t ifx_tapi_read(struct file *filp, char *buf, size_t count, loff_t * ppos)
{
   TAPI_CHANNEL *pTapiCh = (TAPI_CHANNEL*) filp->private_data;
   IFX_TAPI_DRV_CTX_t *pDrvCtx = (IFX_TAPI_DRV_CTX_t*) pTapiCh->pTapiDevice->pDevDrvCtx;
   IFX_TAPI_LL_CH_t* pCh = pTapiCh->pLLChannel;
   ssize_t size=0;

   if (pDrvCtx->Read == IFX_NULL)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_LOW, ("TAPI_DRV: pDrvCtx->LL_Read is NULL !!!\n\r"));
      return 0;
   }

   /* check for non blocking flag */
   if (filp->f_flags & O_NONBLOCK)
      pTapiCh->nFlags |= CF_NONBLOCK;

   /* Call the Low level driver's read function */
   size = pDrvCtx->Read (pCh, buf, count, (IFX_int32_t*)ppos);
   return size;
}

/**
   Configuration / Control for the device.

   \param inode pointer to the inode
   \param filp pointer to the file descriptor
   \param nCmd function id's
   \param nArg optional argument

   \return
   0 and positive values - success,
   negative value - ioctl failed
   \remark
   This function does the following functions:
      - If the ioctl command is device specific, low-level driver's ioctl function
      - If the ioctl command is TAPI specific, it is handled at this level
*/
static int ifx_tapi_ioctl(struct inode *inode, struct file *filp,
                          unsigned int nCmd, unsigned long nArg)
{
   IFX_TAPI_DRV_CTX_t *pDrvCtx    = IFX_NULL;
   IFX_int32_t         ret        = 0;
   IFX_TAPI_ioctlCtx_t ctx;

   /* get the device driver context */
   pDrvCtx = IFX_TAPI_Get_Device_Driver_Context (MAJOR(inode->i_rdev));
   if (pDrvCtx == IFX_NULL)
      {
         return IFX_ERROR;
      }
   /* get the ioctl context: channel, device etc. */
   IFX_TAPI_ioctlContextGet (pDrvCtx, MINOR(inode->i_rdev), &ctx);
   ctx.nParamSize = _IOC_SIZE(nCmd);

   switch (IFXOX_IO_GETMAGIC (nCmd))
   {
      case VMMC_IOC_MAGIC:
      case VINETIC_IOC_MAGIC:
      case DUS_IOC_MAGIC:
      case SVIP_IOC_MAGIC:
         /* Device specific ioctl command */
         ret = TAPI_Dev_Spec_Ioctl (pDrvCtx, &ctx, nCmd, nArg);
         break;

      case IFX_TAPI_IOC_MAGIC:
         /* TAPI specific ioctl */
         ret = TAPI_Spec_Ioctl     (pDrvCtx, &ctx, nCmd, nArg);
         if (ret != IFX_SUCCESS)
         {
            if ((nCmd != IFX_TAPI_CAP_CHECK) &&
                (nCmd != IFX_TAPI_RING))
            {
               /* check if we are in device or channel context */
               if (ctx.bDev == IFX_TRUE)
               {
                  ctx.p.pTapiDev->error.nCh = ctx.p.pTapiCh->nChannel;
                  ctx.p.pTapiDev->error.nCode = ret;
               }
               else
               {
                  /* we are in channel context -> set the device context */
                  ctx.p.pTapiDev = ctx.p.pTapiCh->pTapiDevice;
                  ctx.p.pTapiDev->error.nCh = ctx.p.pTapiCh->nChannel;
                  ctx.p.pTapiDev->error.nCode = ret;
               }
               /* set errno = ret; */
               ret = IFX_ERROR;
            }
         }
         break;

#ifdef QOS_SUPPORT
      case QOS_IOC_MAGIC:
         ret = Qos_Ctrl            ((IFX_uint32_t) ctx.p.pTapiCh, nCmd, nArg);
         break;
#endif /* QOS_SUPPORT */

      default:
      {
         TAPI_DEV* pTapiDev = ctx.p.pTapiDev;
         RETURN_DEVSTATUS (TAPI_statusNoIoctl, 0);
      }
   }
   if (ret == -1 && ctx.p.pTapiDev->error.nCode == 0)
   {
      TRACE (TAPI_DRV, DBG_LEVEL_HIGH, ("Error code not set %d\n\r",nCmd));
   }
   return ret;
}


#ifdef DEBUG_TAPI_IOCTLS
static void PrintIoctlCmd( IFX_uint32_t cmd, IFX_void_t *parg )
{
   printk("CMD = ");

   switch (cmd)
   {
#ifdef TAPI_AUDIO_CHANNEL
      case IFX_TAPI_AUDIO_VOLUME_SET:
         printk("IFX_TAPI_AUDIO_VOLUME_SET\n");
         break;
      case IFX_TAPI_AUDIO_ROOM_TYPE_SET :
         printk("IFX_TAPI_AUDIO_ROOM_TYPE_SET\n");
         break;
      case IFX_TAPI_AUDIO_MUTE_SET :
         printk("IFX_TAPI_AUDIO_MUTE_SET, mode=%lu\n", (IFX_uint32_t)parg);
         break;
      case IFX_TAPI_AUDIO_MODE_SET :
         printk("IFX_TAPI_AUDIO_MODE_SET, mode=%lu\n", (IFX_uint32_t)parg);
         break;
      case IFX_TAPI_AUDIO_RING_START :
         printk("IFX_TAPI_AUDIO_RING_START\n");
         break;
      case IFX_TAPI_AUDIO_RING_STOP :
         printk("IFX_TAPI_AUDIO_RING_STOP\n");
         break;
      case IFX_TAPI_AUDIO_RING_VOLUME_SET :
         printk("IFX_TAPI_AUDIO_RING_VOLUME_SET\n");
         printk("volume =%u\n", (IFX_uint32_t)parg);
         break;
      case IFX_TAPI_AUDIO_ICA_SET :
         printk("IFX_TAPI_AUDIO_ICA_SET, mode=%lu\n", (IFX_uint32_t)parg);
         break;
#endif     /* TAPI_AUDIO_CHANNEL */

#ifdef TAPI_VOICE
      case IFX_TAPI_MAP_DATA_ADD:
         printk("IFX_TAPI_MAP_DATA_ADD\n");
         printk("nDstCh     =%u\n",((IFX_TAPI_MAP_DATA_t*)parg)->nDstCh);
         printk("nChType    =%d\n",((IFX_TAPI_MAP_DATA_t*)parg)->nChType);
         printk("nRecStart  =%d\n",((IFX_TAPI_MAP_DATA_t*)parg)->nRecStart);
         printk("nPlayStart =%d\n",((IFX_TAPI_MAP_DATA_t*)parg)->nPlayStart);
         break;
      case IFX_TAPI_MAP_DATA_REMOVE:
         printk("IFX_TAPI_MAP_DATA_REMOVE\n");
         break;
      case IFX_TAPI_MAP_PHONE_ADD:
         printk("IFX_TAPI_MAP_PHONE_ADD\n");
         break;
      case IFX_TAPI_MAP_PHONE_REMOVE:
         printk("IFX_TAPI_MAP_PHONE_REMOVE\n");
         break;
      case IFX_TAPI_MAP_PCM_ADD:
         printk("IFX_TAPI_MAP_PCM_ADD\n");
         break;
      case IFX_TAPI_MAP_PCM_REMOVE:
         printk("IFX_TAPI_MAP_PCM_REMOVE\n");
         break;
      case IFX_TAPI_PKT_RTP_PT_CFG_SET:
         printk("IFX_TAPI_PKT_RTP_PT_CFG_SET\n");
         break;
      case IFX_TAPI_PKT_RTP_CFG_SET:
         printk("IFX_TAPI_PKT_RTP_CFG_SET\n");
         break;
      case IFX_TAPI_PKT_RTCP_STATISTICS_GET:
         printk("IFX_TAPI_PKT_RTCP_STATISTICS_GET\n");
         break;
      case IFX_TAPI_PKT_RTCP_STATISTICS_RESET:
         printk("IFX_TAPI_PKT_RTCP_STATISTICS_RESET\n");
         break;
      case IFX_TAPI_JB_CFG_SET:
         printk("IFX_TAPI_JB_CFG_SET\n");
         break;
      case IFX_TAPI_JB_STATISTICS_GET:
         printk("IFX_TAPI_JB_STATISTICS_GET\n");
         break;
      case IFX_TAPI_ENC_TYPE_SET:
         printk("IFX_TAPI_ENC_TYPE_SET\n");
/*         printk("Coder     =%u\n", (IFX_uint32_t*)parg);  */

         break;

      case IFX_TAPI_ENC_START:
         printk("IFX_TAPI_ENC_START\n");
         break;
      case IFX_TAPI_ENC_STOP:
         printk("IFX_TAPI_ENC_STOP\n");
         break;
      case IFX_TAPI_ENC_HOLD:
         printk("IFX_TAPI_ENC_HOLD\n");
         break;
      case IFX_TAPI_DEC_START:
         printk("IFX_TAPI_DEC_START\n");
         break;
      case IFX_TAPI_DEC_STOP:
         printk("IFX_TAPI_DEC_STOP\n");
         break;

#endif    /* TAPI_VOICE */

      case IFX_TAPI_VERSION_GET:
         printk("IFX_TAPI_VERSION_GET\n");
         break;
      case IFX_TAPI_CH_INIT:
         printk("IFX_TAPI_CH_INIT\n");
         break;
      case IFX_TAPI_TONE_LOCAL_PLAY:
         printk("IFX_TAPI_TONE_LOCAL_PLAY\n");
         break;
      case IFX_TAPI_TONE_TABLE_CFG_SET:
         printk("IFX_TAPI_TONE_TABLE_CFG_SET\n");
         break;
      case IFX_TAPI_CAP_NR:
         printk("IFX_TAPI_CAP_NR\n");
         break;
      case IFX_TAPI_CAP_LIST:
         printk("IFX_TAPI_CAP_LIST\n");
         break;
      case IFX_TAPI_LASTERR:
         printk("IFX_TAPI_LASTERR\n");
         break;
#ifdef ENABLE_OBSOLETE_BITFIELD_EVENT_INTERFACE
      case IFX_TAPI_CH_STATUS_GET:
         printk("IFX_TAPI_CH_STATUS_GET\n");
         break;
#endif
#ifdef TAPI_EXT_KEYPAD
      case IFX_TAPI_PKT_EV_GENERATE_CFG:
         printk("IFX_TAPI_PKT_EV_GENERATE_CFG\n");
         break;
#endif
      case IFX_TAPI_PKT_EV_OOB_SET:
         printk("IFX_TAPI_PKT_EV_OOB_SET\n");
         break;
      case IFX_TAPI_PKT_EV_GENERATE:
         printk("IFX_TAPI_PKT_EV_GENERATE\n");
         break;
      default:
         printk("%lX unhandled or unknown\n", cmd);
         break;
   }

}
#endif


/**
   Tapi Specific ioctl handling

\param pChannel  - handle to TAPI_CHANNEL structure (might contain a pointer
                   to a TAPI_DEVICE structure, i.e. check inside)
\param iocmd   - ioctl command
\param ioarg   - ioctl argument

\return
   IFX_SUCCESS or IFX_ERROR
*/
IFX_int32_t  TAPI_OS_IoctlCh (IFX_TAPI_DRV_CTX_t* pDrvCtx,
                              IFX_TAPI_ioctlCtx_t* pCtx,
                              TAPI_CHANNEL* pChannel,
                              IFX_uint32_t iocmd, IFX_uint32_t ioarg)

{
   IFX_void_t   *parg      = (IFX_void_t *)ioarg;
   IFX_void_t   *p_iostack = IFX_NULL;
   IFX_int32_t   ret       = IFX_SUCCESS;
#ifdef DEBUG_TAPI_IOCTLS
   printk (KERN_INFO "\nTAPI_Ioctl called with command %x\n",iocmd);
#endif

   /* Get hold of memory to process this ioctl */
   p_iostack = IFXOS_MALLOC (TAPI_IOCTL_STACKSIZE);
   if (p_iostack == IFX_NULL)
      return IFX_ERROR;
#ifdef DEBUG_TAPI_IOCTLS
   PrintIoctlCmd( iocmd, ioarg );
   printk("pChannel->nChannel=%d\n", pChannel->nChannel);
#endif

   /* check channel */
   switch (iocmd)
   {
   /* Ringing Services */
   case IFX_TAPI_RING_CFG_SET:
   {
      IFX_TAPI_RING_CFG_t *p_tmp = (IFX_TAPI_RING_CFG_t *)p_iostack;
      IFXOS_ASSERT(sizeof(IFX_TAPI_RING_CFG_t) < TAPI_IOCTL_STACKSIZE);
      if (copy_from_user (p_tmp, parg, sizeof(IFX_TAPI_RING_CFG_t)) > 0 )
      {
         TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
              ("TAPI: cannot copy from user space\n\r"));
         ret = IFX_ERROR;
      }
      else
      {
         ret = IFX_TAPI_Ring_SetConfig(pChannel, p_tmp);
      }
      break;
   }

   case IFX_TAPI_RING_CFG_GET:
   {
      IFX_TAPI_RING_CFG_t *p_tmp = (IFX_TAPI_RING_CFG_t *)p_iostack;
      IFXOS_ASSERT(sizeof(IFX_TAPI_RING_CFG_t) < TAPI_IOCTL_STACKSIZE);
      ret =  IFX_TAPI_Ring_GetConfig(pChannel, p_tmp);
      if ((ret == IFX_SUCCESS) &&
          (copy_to_user (parg, p_tmp, sizeof(IFX_TAPI_RING_CFG_t)) > 0))
      {
         TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
              ("TAPI: cannot copy to user space\n\r"));
         ret = IFX_ERROR;
      }
      break;
   }

   case IFX_TAPI_RING_CADENCE_HR_SET:
   {
      IFX_TAPI_RING_CADENCE_t *p_tmp = (IFX_TAPI_RING_CADENCE_t *)p_iostack;
      IFXOS_ASSERT(sizeof(IFX_TAPI_RING_CADENCE_t) < TAPI_IOCTL_STACKSIZE);
      if (copy_from_user (p_tmp, parg, sizeof(IFX_TAPI_RING_CADENCE_t)) > 0 )
      {
         TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
              ("TAPI: cannot copy from user space\n\r"));
         ret = IFX_ERROR;
      }
      else
      {
         ret = IFX_TAPI_Ring_SetCadenceHighRes(pChannel, p_tmp);
      }
      break;
   }

   case IFX_TAPI_LINE_TYPE_SET:
   {
      IFX_TAPI_LINE_TYPE_CFG_t *p_tmp = (IFX_TAPI_LINE_TYPE_CFG_t *) p_iostack;
      IFXOS_ASSERT(sizeof(IFX_TAPI_LINE_TYPE_CFG_t) < TAPI_IOCTL_STACKSIZE);
      if (copy_from_user (p_tmp, parg, sizeof(IFX_TAPI_LINE_TYPE_CFG_t)) > 0 )
      {
         TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
              ("TAPI: cannot copy from user space\n\r"));
         ret = IFX_ERROR;
      }
      else
      {
         ret = TAPI_Phone_Set_LineType(pDrvCtx, pChannel, p_tmp);
      }
      break;
   }


#ifdef TAPI_VOICE
   /* Connection Services*/
   case IFX_TAPI_MAP_DATA_ADD:
   {
      IFX_TAPI_MAP_DATA_t *p_tmp = (IFX_TAPI_MAP_DATA_t *)p_iostack;
      IFXOS_ASSERT(sizeof(IFX_TAPI_MAP_DATA_t) < TAPI_IOCTL_STACKSIZE);
      if (copy_from_user (p_tmp, parg, sizeof(IFX_TAPI_MAP_DATA_t)) > 0)
      {
         TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
              ("TAPI: cannot copy from user space\n\r"));
         ret = IFX_ERROR;
      }
      else
         ret = TAPI_Data_Channel_Add (pChannel, p_tmp);
      break;
   }

   case IFX_TAPI_MAP_DATA_REMOVE:
   {
      IFX_TAPI_MAP_DATA_t *p_tmp = (IFX_TAPI_MAP_DATA_t *)p_iostack;
      IFXOS_ASSERT(sizeof(IFX_TAPI_MAP_DATA_t) < TAPI_IOCTL_STACKSIZE);
      if (copy_from_user (p_tmp, parg, sizeof(IFX_TAPI_MAP_DATA_t)) > 0)
      {
         TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
              ("TAPI: cannot copy from user space\n\r"));
         ret = IFX_ERROR;
      }
      else
         ret = TAPI_Data_Channel_Remove (pChannel, p_tmp);
      break;
   }

   case IFX_TAPI_MAP_PHONE_ADD:
   {
      IFX_TAPI_MAP_PHONE_t  *p_tmp = (IFX_TAPI_MAP_PHONE_t  *)p_iostack;
      IFXOS_ASSERT(sizeof(IFX_TAPI_MAP_PHONE_t) < TAPI_IOCTL_STACKSIZE);
      if (copy_from_user (p_tmp, parg, sizeof(IFX_TAPI_MAP_PHONE_t)) > 0)
      {
         TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
              ("TAPI: cannot copy from user space\n\r"));
         ret = IFX_ERROR;
      }
      else
         ret = TAPI_Phone_Channel_Add (pChannel, p_tmp);
      break;
   }

   case IFX_TAPI_MAP_PHONE_REMOVE:
   {
      IFX_TAPI_MAP_PHONE_t *p_tmp = (IFX_TAPI_MAP_PHONE_t *)p_iostack;
      IFXOS_ASSERT(sizeof(IFX_TAPI_MAP_PHONE_t) < TAPI_IOCTL_STACKSIZE);
      if (copy_from_user (p_tmp, parg, sizeof(IFX_TAPI_MAP_PHONE_t)) > 0)
      {
         TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
              ("TAPI: cannot copy from user space\n\r"));
         ret = IFX_ERROR;
      }
      else
         ret = TAPI_Phone_Channel_Remove (pChannel, p_tmp);
      break;
   }

   case IFX_TAPI_PCM_IF_CFG_SET:
   {
      IFX_TAPI_PCM_IF_CFG_t *p_tmp = (IFX_TAPI_PCM_IF_CFG_t *)p_iostack;
      IFXOS_ASSERT(sizeof(IFX_TAPI_PCM_IF_CFG_t) < TAPI_IOCTL_STACKSIZE);
      if (copy_from_user (p_tmp, parg, sizeof(IFX_TAPI_PCM_IF_CFG_t)) > 0)
      {
         TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
              ("TAPI: cannot copy from user space\n\r"));
         ret = IFX_ERROR;
      }
      else
      {
         ret = TAPI_Phone_PCM_IF_Set_Config(pChannel->pTapiDevice, p_tmp);
      }
      break;
   }

   case IFX_TAPI_MAP_PCM_ADD:
   {
      IFX_TAPI_MAP_PCM_t  *p_tmp = (IFX_TAPI_MAP_PCM_t *)p_iostack;
      IFXOS_ASSERT(sizeof(IFX_TAPI_MAP_PCM_t) < TAPI_IOCTL_STACKSIZE);
      if (copy_from_user (p_tmp, parg, sizeof(IFX_TAPI_MAP_PCM_t)) > 0)
      {
         TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
              ("TAPI: cannot copy from user space\n\r"));
         ret = IFX_ERROR;
      }
      else
      {
         if (pDrvCtx->CON.PCM_Channel_Add == IFX_NULL)
         {
            TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
                 ("TAPI: pDrvCtx->CON.PCM_Channel_Add   is NULL !!!!\n\r"));
            ret = IFX_ERROR;
         }
         else
            ret = pDrvCtx->CON.PCM_Channel_Add (pChannel->pLLChannel, p_tmp);
      }
      break;
   }

   case IFX_TAPI_MAP_PCM_REMOVE:
   {
      IFX_TAPI_MAP_PCM_t  *p_tmp = (IFX_TAPI_MAP_PCM_t  *)p_iostack;
      IFXOS_ASSERT(sizeof(IFX_TAPI_MAP_PCM_t) < TAPI_IOCTL_STACKSIZE);
      if (copy_from_user (p_tmp, parg, sizeof(IFX_TAPI_MAP_PCM_t)) > 0)
      {
         TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
              ("TAPI: cannot copy from user space\n\r"));
         ret = IFX_ERROR;
      }
      else
      {
         if (pDrvCtx->CON.PCM_Channel_Remove == IFX_NULL)
         {
            TRACE(TAPI_DRV,DBG_LEVEL_HIGH,
                 ("TAPI: pDrvCtx->CON.PCM_Channel_Remove is NULL !!!!\n\r"));
            ret = IFX_ERROR;
         }
         else
            ret = pDrvCtx->CON.PCM_Channel_Remove (pChannel->pLLChannel, p_tmp);
      }
      break;
   }

#ifdef DECT_SUPPORT
   case IFX_TAPI_MAP_DECT_ADD:
      if (ptr_chk( pDrvCtx->CON.DECT_Channel_Add,
                  "pDrvCtx->CON.DECT_Channel_Add"))
      {
         IFX_TAPI_MAP_DECT_t  *p_tmp = (IFX_TAPI_MAP_DECT_t  *)p_iostack;
         IFXOS_ASSERT(sizeof(IFX_TAPI_MAP_DECT_t) < TAPI_IOCTL_STACKSIZE);

         if (copy_from_user (p_tmp, parg, sizeof(IFX_TAPI_MAP_DECT_t)) > 0)
         {
            ret = IFX_ERROR;
         }
         else
         {
            ret = pDrvCtx->CON.DECT_Channel_Add (pChannel->pLLChannel, p_tmp);
         }
      }
      else
      {
         TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
               ("TAPI: DECT channel not supported by LL driver\n\r"));
         ret = IFX_ERROR;
      }
      break;


   case IFX_TAPI_MAP_DECT_REMOVE:
      if (ptr_chk( pDrvCtx->CON.DECT_Channel_Remove,
                  "pDrvCtx->CON.DECT_Channel_Remove"))
      {
         IFX_TAPI_MAP_DECT_t  *p_tmp = (IFX_TAPI_MAP_DECT_t  *)p_iostack;
         IFXOS_ASSERT(sizeof(IFX_TAPI_MAP_DECT_t) < TAPI_IOCTL_STACKSIZE);

         if (copy_from_user (p_tmp, parg, sizeof(IFX_TAPI_MAP_DECT_t)) > 0)
         {
            ret = IFX_ERROR;
         }
         else
         {
            ret = pDrvCtx->CON.DECT_Channel_Remove (pChannel->pLLChannel, p_tmp);
         }
      }
      else
      {
         TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
               ("TAPI: DECT channel not supported by LL driver\n\r"));
         ret = IFX_ERROR;
      }
      break;
#endif /* DECT_SUPPORT */

   case IFX_TAPI_PKT_RTP_CFG_SET:
   {
      IFX_TAPI_PKT_RTP_CFG_t *p_tmp = (IFX_TAPI_PKT_RTP_CFG_t *)p_iostack;
      IFXOS_ASSERT(sizeof(IFX_TAPI_PKT_RTP_CFG_t) < TAPI_IOCTL_STACKSIZE);
      if (copy_from_user (p_tmp, parg, sizeof(IFX_TAPI_PKT_RTP_CFG_t)) > 0)
      {
         TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
              ("TAPI: cannot copy from user space\n\r"));
         ret = IFX_ERROR;
      }
      else
      {
         if (pDrvCtx->COD.RTP_Cfg == IFX_NULL)
         {
            TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
                 ("TAPI: pDrvCtx->COD.RTP_Cfg is NULL !!!!\n\r"));
            ret = IFX_ERROR;
         }
         else
         {
#ifdef TAPI_EXT_KEYPAD
            /*
             * Clear lower 7 bits of status byte and
             * write passed argument to these bits.
             * Ensure that local play flag remains.
             */
            pChannel->nDtmfInfo = (pChannel->nDtmfInfo & DTMF_EV_LOCAL_PLAY) |
                                 ((IFX_uint8_t)(p_tmp->nEvents) & ~DTMF_EV_LOCAL_PLAY );
#else
            pChannel->nDtmfInfo = (IFX_uint8_t)(p_tmp->nEvents);
#endif

            ret = pDrvCtx->COD.RTP_Cfg (pChannel->pLLChannel, p_tmp);
      }
      }
      break;
   }

   case IFX_TAPI_PKT_RTP_PT_CFG_SET:
   {
      IFX_TAPI_PKT_RTP_PT_CFG_t *p_tmp =
                                     (IFX_TAPI_PKT_RTP_PT_CFG_t *)p_iostack;
      IFXOS_ASSERT(sizeof(IFX_TAPI_PKT_RTP_PT_CFG_t) < TAPI_IOCTL_STACKSIZE);
      if (copy_from_user (p_tmp, parg, sizeof(IFX_TAPI_PKT_RTP_PT_CFG_t)) > 0)
      {
         TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
              ("TAPI: cannot copy from user space\n\r"));
         ret = IFX_ERROR;
      }
      else
      {
         if (pDrvCtx->COD.RTP_PayloadTable_Cfg == IFX_NULL)
         {
            TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
                 ("TAPI: pDrvCtx->CON.RTP_PayloadTable_Cfg is NULL !!!!\n\r"));
            ret = IFX_ERROR;
         }
         else
            ret = pDrvCtx->COD.RTP_PayloadTable_Cfg (pChannel->pLLChannel, p_tmp);
      }
      break;
   }

   case IFX_TAPI_PKT_RTCP_STATISTICS_GET:
   {
      IFX_TAPI_PKT_RTCP_STATISTICS_t *p_tmp =
                                 (IFX_TAPI_PKT_RTCP_STATISTICS_t *)p_iostack;
      IFXOS_ASSERT(sizeof(IFX_TAPI_PKT_RTCP_STATISTICS_t) < TAPI_IOCTL_STACKSIZE);
      if (pDrvCtx->COD.RTCP_Get == IFX_NULL)
      {
            TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
                 ("TAPI: pDrvCtx->CON.RTCP_Get is NULL !!!!\n\r"));
            ret = IFX_ERROR;
      }
      else
         ret = pDrvCtx->COD.RTCP_Get (pChannel->pLLChannel, p_tmp);
      if ((ret == IFX_SUCCESS) &&
          (copy_to_user (parg, p_tmp, sizeof(IFX_TAPI_PKT_RTCP_STATISTICS_t)) > 0))
      {
         TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
              ("TAPI: cannot copy to user space\n\r"));
         ret = IFX_ERROR;
      }
      break;
   }

   case IFX_TAPI_JB_CFG_SET:
   {
      IFX_TAPI_JB_CFG_t *p_tmp = (IFX_TAPI_JB_CFG_t *)p_iostack;
      IFXOS_ASSERT(sizeof(IFX_TAPI_JB_CFG_t) < TAPI_IOCTL_STACKSIZE);
      if (copy_from_user (p_tmp, parg, sizeof(IFX_TAPI_JB_CFG_t)) > 0)
      {
         TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
              ("TAPI: cannot copy from user space\n\r"));
         ret = IFX_ERROR;
      }
      else
      {
         if (pDrvCtx->COD.JB_Cfg == IFX_NULL)
         {
            TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
                 ("TAPI: pDrvCtx->CON.JB_Cfg is NULL    !!!\n\r"));
            ret = IFX_ERROR;
         }
         else
            ret = pDrvCtx->COD.JB_Cfg (pChannel->pLLChannel, p_tmp);
      }
            /* cache actual jitter buffer config */
            if (ret == IFX_SUCCESS)
               memcpy (&pChannel->TapiJbData, p_tmp, sizeof (IFX_TAPI_JB_CFG_t));
      break;
   }

   case IFX_TAPI_JB_STATISTICS_GET:
   {
      IFX_TAPI_JB_STATISTICS_t *p_tmp =
                                   (IFX_TAPI_JB_STATISTICS_t *)p_iostack;
      IFXOS_ASSERT(sizeof(IFX_TAPI_JB_STATISTICS_t) < TAPI_IOCTL_STACKSIZE);
      if (pDrvCtx->COD.JB_Stat_Get == IFX_NULL)
      {
         TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
              ("TAPI: pDrvCtx->CON.JB_Stat_Get is NULL !!!\n\r"));
         ret = IFX_ERROR;
      }
      else
         ret = pDrvCtx->COD.JB_Stat_Get (pChannel->pLLChannel, p_tmp);
      if ((ret == IFX_SUCCESS) && (copy_to_user ( parg, p_tmp,
                                   sizeof(IFX_TAPI_JB_STATISTICS_t)) > 0))
      {
         TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
              ("TAPI: cannot copy to user space\n\r"));
         ret = IFX_ERROR;
      }
      break;
   }

   case IFX_TAPI_ENC_CFG_SET:
      if (ptr_chk(pDrvCtx->COD.ENC_Cfg, "pDrvCtx->COD.ENC_Cfg"))
      {
         IFX_TAPI_ENC_CFG_t *p_tmp = (IFX_TAPI_ENC_CFG_t *)p_iostack;
         IFXOS_ASSERT(sizeof(IFX_TAPI_ENC_CFG_t) < TAPI_IOCTL_STACKSIZE);
         if (copy_from_user (p_tmp, parg, sizeof(IFX_TAPI_ENC_CFG_t)) > 0 )
         {
            TRACE(TAPI_DRV,DBG_LEVEL_HIGH,
                 ("TAPI: cannot copy from user space\n\r"));
            ret = IFX_ERROR;
         }
         else
         {
            ret = pDrvCtx->COD.ENC_Cfg(pChannel->pLLChannel,
                                       p_tmp->nEncType, p_tmp->nFrameLen,
                                       p_tmp->AAL2BitPack);
         }
      }
      else
      {
         TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
               ("TAPI: Encoder config not supported by LL driver\n\r"));
         ret = IFX_ERROR;
      }
      break;

   case IFX_TAPI_DEC_CFG_SET:
      if (ptr_chk(pDrvCtx->COD.DEC_Cfg, "pDrvCtx->COD.DEC_Cfg"))
      {
         IFX_TAPI_DEC_CFG_t *p_tmp = (IFX_TAPI_DEC_CFG_t *)p_iostack;
         IFXOS_ASSERT(sizeof(IFX_TAPI_DEC_CFG_t) < TAPI_IOCTL_STACKSIZE);
         if (copy_from_user (p_tmp, parg, sizeof(IFX_TAPI_DEC_CFG_t)) > 0 )
         {
            TRACE(TAPI_DRV,DBG_LEVEL_HIGH,
                 ("TAPI: cannot copy from user space\n\r"));
            ret = IFX_ERROR;
         }
         else
         {
            ret = pDrvCtx->COD.DEC_Cfg(pChannel->pLLChannel,
                                       p_tmp->AAL2BitPack);
         }
      }
      else
      {
         TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
               ("TAPI: Decoder config not supported by LL driver\n\r"));
         ret = IFX_ERROR;
      }
      break;


   case IFX_TAPI_ENC_FRAME_LEN_GET:
   {
      IFX_int32_t length = 0;

      if (pDrvCtx->COD.ENC_FrameLength_Get == IFX_NULL)
      {
         TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
               ("TAPI: Getting the frame length not supported by LL driver\n\r"));
         ret = IFX_ERROR;
      }
      else
         ret = pDrvCtx->COD.ENC_FrameLength_Get (pChannel->pLLChannel, &length);
      if ((ret == IFX_SUCCESS) &&
          (copy_to_user (parg, &length, sizeof(IFX_int32_t)) > 0))
      {
         TRACE(TAPI_DRV,DBG_LEVEL_HIGH,("TAPI: cannot copy to user space\n\r"));
         ret = IFX_ERROR;
      }
      break;
   }

#endif /* TAPI_VOICE */

#ifdef TAPI_FAX_T38
   /* Fax Services */
   case IFX_TAPI_T38_MOD_START:
   {
      IFX_TAPI_T38_MOD_DATA_t *p_tmp = (IFX_TAPI_T38_MOD_DATA_t *)p_iostack;
      IFXOS_ASSERT(sizeof(IFX_TAPI_T38_MOD_DATA_t) < TAPI_IOCTL_STACKSIZE);
      if (copy_from_user (p_tmp, parg,  sizeof(IFX_TAPI_T38_MOD_DATA_t)) > 0)
      {
         TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
              ("TAPI: cannot copy from user space\n\r"));
         ret = IFX_ERROR;
      }
      else
      {
         if (pDrvCtx->COD.T38_Mod_Enable == IFX_NULL)
         {
            TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
                 ("TAPI:  pDrvCtx->COD.T38_Mod_Enable is NULL    !!!\n\r"));
            ret = IFX_ERROR;
         }
         else
            ret = pDrvCtx->COD.T38_Mod_Enable (pChannel->pLLChannel, p_tmp);
      }
      break;
   }

   case IFX_TAPI_T38_DEMOD_START:
   {
      IFX_TAPI_T38_DEMOD_DATA_t *p_tmp = (IFX_TAPI_T38_DEMOD_DATA_t *)p_iostack;
      IFXOS_ASSERT(sizeof(IFX_TAPI_T38_DEMOD_DATA_t) < TAPI_IOCTL_STACKSIZE);
      if (copy_from_user (p_tmp, parg, sizeof(IFX_TAPI_T38_DEMOD_DATA_t)) > 0)
      {
         TRACE(TAPI_DRV,DBG_LEVEL_HIGH,
              ("TAPI: cannot copy from user space\n\r"));
         ret = IFX_ERROR;
      }
      else
      {
         if (pDrvCtx->COD.T38_DeMod_Enable == IFX_NULL)
         {
            TRACE(TAPI_DRV,DBG_LEVEL_HIGH,
                 ("TAPI:  pDrvCtx->COD.T38_DeMod_Enable is NULL!!!\n\r"));
            ret = IFX_ERROR;
         }
         else
            ret = pDrvCtx->COD.T38_DeMod_Enable (pChannel->pLLChannel, p_tmp);
      }
      break;
   }


   case IFX_TAPI_T38_STATUS_GET:
   {
      IFX_TAPI_T38_STATUS_t *p_tmp = (IFX_TAPI_T38_STATUS_t *)p_iostack;
      IFXOS_ASSERT(sizeof(IFX_TAPI_T38_STATUS_t) < TAPI_IOCTL_STACKSIZE);

      if (pDrvCtx->COD.T38_Status_Get == IFX_NULL)
      {
         TRACE(TAPI_DRV,DBG_LEVEL_HIGH,
              ("TAPI:  pDrvCtx->COD.T38_Status_Get is NULL !!!\n\r"));
         ret = IFX_ERROR;
      }
      else
         ret = pDrvCtx->COD.T38_Status_Get (pChannel->pLLChannel, p_tmp);
      if ((ret == IFX_SUCCESS) &&
          (copy_to_user (parg, p_tmp, sizeof(IFX_TAPI_T38_STATUS_t)) > 0))
      {
        TRACE(TAPI_DRV,DBG_LEVEL_HIGH,("TAPI: cannot copy to user space\n\r"));
        ret = IFX_ERROR;
      }
      break;
   }
#endif /* TAPI_FAX_T38 */
   case IFX_TAPI_PCM_CFG_SET:
   {
      IFX_TAPI_PCM_CFG_t *p_tmp = (IFX_TAPI_PCM_CFG_t *)p_iostack;
      IFXOS_ASSERT(sizeof(IFX_TAPI_PCM_CFG_t) < TAPI_IOCTL_STACKSIZE);
      if (copy_from_user (p_tmp, parg, sizeof(IFX_TAPI_PCM_CFG_t)) > 0)
      {
         TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
              ("TAPI: cannot copy from user space\n\r"));
         ret = IFX_ERROR;
      }
      else
      {
         ret = TAPI_Phone_PCM_Set_Config(pChannel, p_tmp);
      }
      break;
   }

   case IFX_TAPI_PCM_CFG_GET:
   {
      IFX_TAPI_PCM_CFG_t *p_tmp = (IFX_TAPI_PCM_CFG_t *)p_iostack;
      IFXOS_ASSERT(sizeof(IFX_TAPI_PCM_CFG_t) < TAPI_IOCTL_STACKSIZE);
      ret = TAPI_Phone_PCM_Get_Config(pChannel, p_tmp);

      if ((ret == IFX_SUCCESS) &&
          (copy_to_user (parg, p_tmp, sizeof(IFX_TAPI_PCM_CFG_t)) > 0))
      {
         TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
              ("TAPI: cannot copy to user space\n\r"));
         ret = IFX_ERROR;
      }
      break;
   }

   /* Tone Services */
   case IFX_TAPI_TONE_STATUS_GET:
   {
      IFX_uint32_t nToneState;
      ret = TAPI_Phone_Tone_Get_State(pChannel, &nToneState);
      if ((ret == IFX_SUCCESS) &&
          (copy_to_user (parg, &nToneState, sizeof(IFX_int32_t)) > 0))
      {
         TRACE(TAPI_DRV,DBG_LEVEL_HIGH,("TAPI: cannot copy to user space\n\r"));
         ret = IFX_ERROR;
      }
      break;
   }

   case IFX_TAPI_TONE_STOP:
      ret = TAPI_Phone_Tone_Stop (pDrvCtx, pChannel, (IFX_int32_t)parg, TAPI_TONE_DST_DEFAULT);
      break;

   case IFX_TAPI_TONE_LEVEL_SET:
   {
      IFX_TAPI_PREDEF_TONE_LEVEL_t  *p_tmp =
                                    (IFX_TAPI_PREDEF_TONE_LEVEL_t *)p_iostack;
      IFXOS_ASSERT(sizeof(IFX_TAPI_PREDEF_TONE_LEVEL_t) < TAPI_IOCTL_STACKSIZE);
      if (copy_from_user (p_tmp, parg, sizeof(IFX_TAPI_PREDEF_TONE_LEVEL_t)) > 0 )
      {
         TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
              ("TAPI: cannot copy from user space\n\r"));
         ret = IFX_ERROR;
      }
      else
      {
         ret = TAPI_Phone_Tone_Set_Level(pDrvCtx, pChannel, p_tmp);
      }
      break;
   }

   case IFX_TAPI_LINE_HOOK_VT_SET:
   {
      IFX_TAPI_LINE_HOOK_VT_t *p_tmp = (IFX_TAPI_LINE_HOOK_VT_t *)p_iostack;
      IFXOS_ASSERT(sizeof(IFX_TAPI_LINE_HOOK_VT_t) < TAPI_IOCTL_STACKSIZE);
      if (copy_from_user (p_tmp, parg, sizeof(IFX_TAPI_LINE_HOOK_VT_t)) > 0)
      {
         TRACE(TAPI_DRV,DBG_LEVEL_HIGH,
              ("TAPI: cannot copy from user space\n\r"));
         ret = IFX_ERROR;
      }
      else
      {
         ret = TAPI_Phone_Validation_Time(pChannel, p_tmp);
      }
      break;
   }

#ifdef TAPI_EXT_KEYPAD
   case IFX_TAPI_PKT_EV_GENERATE_CFG:
   {
      IFX_TAPI_PKT_EV_GENERATE_CFG_t *pEvCfg;

      /* Note : DTMF_EV_LOCAL_PLAY is defined as 0x80 so that we can OR it with the other
                values set by IFX_TAPI_PKT_EV_OOB_SET without loosing that info */
      if(pChannel->nChannel > 0 )
      {
         TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
              ("TAPI:audio is not supported in this channel \n\r"));
         ret = IFX_ERROR;
      }
      else
      {
         pEvCfg = (IFX_TAPI_PKT_EV_GENERATE_CFG_t*)ioarg;
         if(pEvCfg->local) /*set local play*/
         {
            pChannel->nDtmfInfo |= DTMF_EV_LOCAL_PLAY;
         }
         else
         {
            pChannel->nDtmfInfo &= ~DTMF_EV_LOCAL_PLAY;
         }

         ret = IFX_SUCCESS;
      }
      break;
   }
#endif /*TAPI_EXT_KEYPAD*/

   case IFX_TAPI_PKT_EV_GENERATE :
   {
      IFX_TAPI_PKT_EV_GENERATE_t   *p_tmp =
                                    (IFX_TAPI_PKT_EV_GENERATE_t  *)p_iostack;
      IFXOS_ASSERT(sizeof(IFX_TAPI_PKT_EV_GENERATE_t ) < TAPI_IOCTL_STACKSIZE);
      if (copy_from_user (p_tmp, parg, sizeof(IFX_TAPI_PKT_EV_GENERATE_t )) > 0 )
      {
         TRACE(TAPI_DRV,DBG_LEVEL_HIGH,
              ("TAPI: cannot copy from user space\n\r"));
         ret = IFX_ERROR;
      }
      else
      {
         ret = TAPI_EVENT_PKT_EV_Generate(pChannel,p_tmp);
      }
      break;
   }

#ifdef ENABLE_OBSOLETE_BITFIELD_EVENT_INTERFACE
   case IFX_TAPI_CH_STATUS_GET:
   {
      IFX_TAPI_CH_STATUS_t *p_tmp = IFX_NULL;
      IFX_int32_t ch;

      if (ioarg == 0)
      {
         ret = IFX_ERROR;
         break;
      }
      p_tmp = IFXOS_MALLOC(pDrvCtx->maxChannels * sizeof(IFX_TAPI_CH_STATUS_t));
      if (p_tmp == IFX_NULL)
      {
         ret = IFX_ERROR;
         break;
      }
      if (copy_from_user (p_tmp, parg, sizeof(IFX_TAPI_CH_STATUS_t)) > 0)
      {
         TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("TAPI: ioctl parameter error\n\r"));
         ret = IFX_ERROR;
      }
      ch = p_tmp[0].channels;
      if ((ret == IFX_SUCCESS) && (ch > TAPI_MAX_CHANNELS))
      {
         TRACE(TAPI_DRV,DBG_LEVEL_HIGH,
              ("TAPI: IFX_TAPI_CH_STATUS_GET: Ch%d: invalid parameters\n\r",
               pChannel->nChannel - 1));
         ret = IFX_ERROR;
      }
      if (ret == IFX_SUCCESS)
      {
         if (ch == 0)
            ch = 1;
         ret = TAPI_Phone_GetStatus(pChannel, p_tmp);

         if (copy_to_user (parg, p_tmp, sizeof(IFX_TAPI_CH_STATUS_t) * ch) > 0)
         {
            TRACE(TAPI_DRV,DBG_LEVEL_HIGH,
                 ("TAPI: cannot copy to user space\n\r"));
            ret = IFX_ERROR;
         }
      }
      IFXOS_FREE (p_tmp);
      break;
   }
#endif /* ENABLE_OBSOLETE_BITFIELD_EVENT_INTERFACE */

   case IFX_TAPI_CH_INIT:
   {
      IFX_TAPI_CH_INIT_t  *p_tmp = (IFX_TAPI_CH_INIT_t *)p_iostack;
      IFXOS_ASSERT(sizeof(IFX_TAPI_CH_INIT_t) < TAPI_IOCTL_STACKSIZE);
      if ((ioarg != 0) && (copy_from_user (p_tmp, parg,
                                         sizeof(IFX_TAPI_CH_INIT_t)) > 0))
      {
         TRACE(TAPI_DRV,DBG_LEVEL_HIGH,
              ("TAPI: cannot copy from user space\n\r"));
         ret = IFX_ERROR;
      }
      else
      {
         if (ioarg == 0)
         {
            ret = TAPI_Phone_Init(pDrvCtx, pChannel, IFX_NULL);
         }
         else
         {
            ret = TAPI_Phone_Init(pDrvCtx, pChannel, p_tmp);
         }
      }
      break;
   }
   /* Metering Services */
   case IFX_TAPI_METER_CFG_SET:
   {
      IFX_TAPI_METER_CFG_t *p_tmp = (IFX_TAPI_METER_CFG_t *)p_iostack;
      IFXOS_ASSERT(sizeof(IFX_TAPI_METER_CFG_t) < TAPI_IOCTL_STACKSIZE);
      if (copy_from_user (p_tmp, parg, sizeof(IFX_TAPI_METER_CFG_t)) > 0 )
      {
         TRACE(TAPI_DRV,DBG_LEVEL_HIGH,
              ("TAPI: cannot copy from user space\n\r"));
         ret = IFX_ERROR;
      }
      else
      {
         ret = TAPI_Phone_Meter_Config(pChannel, p_tmp);
      }
      break;
   }

   /* Lec Configuration */
   case IFX_TAPI_LEC_PHONE_CFG_SET:
   {
      IFX_TAPI_LEC_CFG_t *p_tmp = (IFX_TAPI_LEC_CFG_t *)p_iostack;
      IFXOS_ASSERT(sizeof(IFX_TAPI_LEC_CFG_t) < TAPI_IOCTL_STACKSIZE);
      if (copy_from_user (p_tmp, parg, sizeof(IFX_TAPI_LEC_CFG_t)) > 0 )
      {
         TRACE(TAPI_DRV,DBG_LEVEL_HIGH,
              ("TAPI: cannot copy from user space\n\r"));
         ret = IFX_ERROR;
      }
      else
      {
         ret = TAPI_Phone_LecConf_Alm (pChannel, p_tmp);
      }
      break;
   }

   case IFX_TAPI_LEC_PHONE_CFG_GET:
   {
      IFX_TAPI_LEC_CFG_t *p_tmp = (IFX_TAPI_LEC_CFG_t *)p_iostack;
      IFXOS_ASSERT(sizeof(IFX_TAPI_LEC_CFG_t) < TAPI_IOCTL_STACKSIZE);
      ret = TAPI_Phone_GetLecConf_Alm (pChannel, p_tmp);
      if ((ret == IFX_SUCCESS) &&
          (copy_to_user (parg, p_tmp, sizeof(IFX_TAPI_LEC_CFG_t)) > 0))
      {
         TRACE(TAPI_DRV,DBG_LEVEL_HIGH,
              ("TAPI: cannot copy to user space\n\r"));
         ret = IFX_ERROR;
      }
      break;
   }

   case IFX_TAPI_LEC_PCM_CFG_SET:
   {
      IFX_TAPI_LEC_CFG_t *p_tmp = (IFX_TAPI_LEC_CFG_t *)p_iostack;
      IFXOS_ASSERT(sizeof(IFX_TAPI_LEC_CFG_t) < TAPI_IOCTL_STACKSIZE);
      if (copy_from_user (p_tmp, parg, sizeof(IFX_TAPI_LEC_CFG_t)) > 0 )
      {
         TRACE(TAPI_DRV,DBG_LEVEL_HIGH,
              ("TAPI: cannot copy from user space\n\r"));
         ret = IFX_ERROR;
      }
      else
      {
         ret = TAPI_Phone_LecConf_Pcm (pChannel, p_tmp);
      }
      break;
   }

   case IFX_TAPI_LEC_PCM_CFG_GET:
   {
      IFX_TAPI_LEC_CFG_t *p_tmp = (IFX_TAPI_LEC_CFG_t *)p_iostack;
      IFXOS_ASSERT(sizeof(IFX_TAPI_LEC_CFG_t) < TAPI_IOCTL_STACKSIZE);
      ret = TAPI_Phone_GetLecConf_Pcm (pChannel, p_tmp);
      if ((ret == IFX_SUCCESS) &&
          (copy_to_user (parg, p_tmp, sizeof(IFX_TAPI_LEC_CFG_t)) > 0))
      {
         TRACE(TAPI_DRV,DBG_LEVEL_HIGH,("TAPI: cannot copy to user space\n\r"));
         ret = IFX_ERROR;
      }
      break;
   }

   case IFX_TAPI_WLEC_PHONE_CFG_SET:
   {
      IFX_TAPI_WLEC_CFG_t *p_tmp = (IFX_TAPI_WLEC_CFG_t *)p_iostack;
      IFXOS_ASSERT(sizeof(IFX_TAPI_WLEC_CFG_t) < TAPI_IOCTL_STACKSIZE);
      if (copy_from_user (p_tmp, parg, sizeof(IFX_TAPI_WLEC_CFG_t)) > 0 )
      {
         TRACE(TAPI_DRV,DBG_LEVEL_HIGH,
              ("TAPI: cannot copy from user space\n\r"));
         ret = IFX_ERROR;
      }
      else
      {
         ret = TAPI_Phone_LecMode_Alm_Set (pChannel, p_tmp);
      }
      break;
   }

   case IFX_TAPI_WLEC_PHONE_CFG_GET:
   {
      IFX_TAPI_WLEC_CFG_t *p_tmp = (IFX_TAPI_WLEC_CFG_t *)p_iostack;
      IFXOS_ASSERT(sizeof(IFX_TAPI_WLEC_CFG_t) < TAPI_IOCTL_STACKSIZE);
      ret = TAPI_Phone_LecMode_Alm_Get (pChannel, p_tmp);
      if ((ret == IFX_SUCCESS) &&
          (copy_to_user (parg, p_tmp, sizeof(IFX_TAPI_WLEC_CFG_t)) > 0))
      {
         TRACE(TAPI_DRV,DBG_LEVEL_HIGH,("TAPI: cannot copy to user space\n\r"));
         ret = IFX_ERROR;
      }
      break;
   }

   case IFX_TAPI_WLEC_PCM_CFG_SET:
   {
      IFX_TAPI_WLEC_CFG_t *p_tmp = (IFX_TAPI_WLEC_CFG_t *)p_iostack;
      IFXOS_ASSERT(sizeof(IFX_TAPI_WLEC_CFG_t) < TAPI_IOCTL_STACKSIZE);
      if (copy_from_user (p_tmp, parg, sizeof(IFX_TAPI_WLEC_CFG_t)) > 0 )
      {
         TRACE(TAPI_DRV,DBG_LEVEL_HIGH,
              ("TAPI: cannot copy from user space\n\r"));
         ret = IFX_ERROR;
      }
      else
      {
         ret = TAPI_Phone_LecMode_Pcm_Set (pChannel, p_tmp);
      }
      break;
   }

   case IFX_TAPI_WLEC_PCM_CFG_GET:
   {
      IFX_TAPI_WLEC_CFG_t *p_tmp = (IFX_TAPI_WLEC_CFG_t *)p_iostack;
      IFXOS_ASSERT(sizeof(IFX_TAPI_WLEC_CFG_t) < TAPI_IOCTL_STACKSIZE);
      ret = TAPI_Phone_LecMode_Pcm_Get (pChannel, p_tmp);
      if ((ret == IFX_SUCCESS) &&
          (copy_to_user (parg, p_tmp, sizeof(IFX_TAPI_WLEC_CFG_t)) > 0))
      {
         TRACE(TAPI_DRV,DBG_LEVEL_HIGH,
              ("TAPI: cannot copy to user space\n\r"));
         ret = IFX_ERROR;
      }
      break;
   }

   case IFX_TAPI_DTMF_RX_CFG_SET:
   {
      IFX_TAPI_DTMF_RX_CFG_t *p_tmp = (IFX_TAPI_DTMF_RX_CFG_t *)p_iostack;
      IFXOS_ASSERT(sizeof(IFX_TAPI_DTMF_RX_CFG_t) < TAPI_IOCTL_STACKSIZE);

      if (copy_from_user (p_tmp, parg, sizeof(IFX_TAPI_DTMF_RX_CFG_t)) > 0)
      {
         TRACE(TAPI_DRV,DBG_LEVEL_HIGH,
              ("TAPI: cannot copy from user space\n\r"));
         ret = IFX_ERROR;
      }
      else
      {
         ret = TAPI_Phone_Dtmf_RxCoeff_Cfg (pDrvCtx, pChannel, IFX_FALSE, p_tmp);
      }
      break;
   }

   case IFX_TAPI_DTMF_RX_CFG_GET:
   {
      IFX_TAPI_DTMF_RX_CFG_t *p_tmp = (IFX_TAPI_DTMF_RX_CFG_t*)p_iostack;
      IFXOS_ASSERT(sizeof(IFX_TAPI_DTMF_RX_CFG_t) < TAPI_IOCTL_STACKSIZE);
      memset(p_tmp, 0, sizeof(IFX_TAPI_DTMF_RX_CFG_t));

      ret = TAPI_Phone_Dtmf_RxCoeff_Cfg (pDrvCtx, pChannel, IFX_TRUE, p_tmp);

      if ((ret == IFX_SUCCESS) &&
          (copy_to_user (parg, p_tmp, sizeof(IFX_TAPI_DTMF_RX_CFG_t)) > 0))
      {
         TRACE(TAPI_DRV,DBG_LEVEL_HIGH,
              ("TAPI: cannot copy to user space\n\r"));
         ret = IFX_ERROR;
      }
      break;
   }

#ifdef TAPI_CID
   /* Caller ID Transmission service */
   case IFX_TAPI_CID_CFG_SET:
   {
      IFX_TAPI_CID_CFG_t *p_tmp = (IFX_TAPI_CID_CFG_t *)p_iostack;
      IFXOS_ASSERT(sizeof(IFX_TAPI_CID_CFG_t) < TAPI_IOCTL_STACKSIZE);
      if (copy_from_user (p_tmp, parg, sizeof(IFX_TAPI_CID_CFG_t)) > 0 )
      {
         TRACE(TAPI_DRV,DBG_LEVEL_HIGH,
              ("TAPI: cannot copy from user space\n\r"));
         ret = IFX_ERROR;
      }
      else
      {
         ret = TAPI_Phone_CID_SetConfig (pChannel, p_tmp);
      }
      break;
   }

   case IFX_TAPI_CID_TX_INFO_START:
   case IFX_TAPI_CID_TX_SEQ_START:
      if (ptr_chk(pDrvCtx->SIG.CID_TX_Start, "pDrvCtx->SIG.CID_TX_Start"))
      {
         IFX_TAPI_CID_MSG_t *p_tmp = (IFX_TAPI_CID_MSG_t *)p_iostack;
         IFXOS_ASSERT(sizeof(IFX_TAPI_CID_MSG_t) < TAPI_IOCTL_STACKSIZE);
         if (copy_from_user (p_tmp, parg, sizeof(IFX_TAPI_CID_MSG_t)) > 0 )
         {
            TRACE(TAPI_DRV,DBG_LEVEL_HIGH,
                 ("TAPI: cannot copy from user space\n\r"));
            ret = IFX_ERROR;
         }
         else
         {
            if (iocmd == IFX_TAPI_CID_TX_INFO_START)
               ret = TAPI_Phone_CID_Info_Tx (pChannel, p_tmp);
            else
               ret = TAPI_Phone_CID_Seq_Tx (pChannel, p_tmp);
         }
      }
      else
      {
         TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
               ("TAPI: CID sending not supported by LL driver\n\r"));
         ret = IFX_ERROR;
      }
      break;


   case IFX_TAPI_CID_RX_STATUS_GET:
   {
      IFX_TAPI_CID_RX_STATUS_t *p_tmp = (IFX_TAPI_CID_RX_STATUS_t *)p_iostack;
      IFXOS_ASSERT(sizeof(IFX_TAPI_CID_RX_STATUS_t) < TAPI_IOCTL_STACKSIZE);
      ret = TAPI_Phone_CidRx_Status (pChannel, p_tmp);
      if ((ret == IFX_SUCCESS) &&
          (copy_to_user (parg, p_tmp, sizeof(IFX_TAPI_CID_RX_STATUS_t)) > 0))
      {
         TRACE(TAPI_DRV,DBG_LEVEL_HIGH,("TAPI: cannot copy to user space\n\r"));
         ret = IFX_ERROR;
      }
      break;
   }

   case IFX_TAPI_CID_RX_DATA_GET:
   {
      IFX_TAPI_CID_RX_DATA_t *p_tmp = (IFX_TAPI_CID_RX_DATA_t *)p_iostack;
      IFXOS_ASSERT(sizeof(IFX_TAPI_CID_RX_DATA_t) < TAPI_IOCTL_STACKSIZE);
      ret = TAPI_Phone_Get_CidRxData (pChannel, p_tmp);
      if ((ret == IFX_SUCCESS) &&
          (copy_to_user (parg, p_tmp, sizeof(IFX_TAPI_CID_RX_DATA_t)) > 0))
      {
         TRACE(TAPI_DRV,DBG_LEVEL_HIGH,("TAPI: cannot copy to user space\n\r"));
         ret = IFX_ERROR;
      }
      break;
   }
#endif /* TAPI_CID */


   case IFX_TAPI_PHONE_VOLUME_SET:
   {
      IFX_TAPI_LINE_VOLUME_t *p_tmp = (IFX_TAPI_LINE_VOLUME_t *)p_iostack;
      IFXOS_ASSERT(sizeof(IFX_TAPI_LINE_VOLUME_t) < TAPI_IOCTL_STACKSIZE);
      if (copy_from_user (p_tmp, parg, sizeof(IFX_TAPI_LINE_VOLUME_t)) > 0 )
      {
         ret = IFX_ERROR;
      }
      else
      {
         if (pDrvCtx->ALM.Volume_Set == IFX_NULL)
         {
            ret = IFX_ERROR;
         }
         else
            ret = pDrvCtx->ALM.Volume_Set (pChannel->pLLChannel, p_tmp);
      }
      break;
   }

   case IFX_TAPI_PCM_VOLUME_SET:
   {
      IFX_TAPI_LINE_VOLUME_t *p_tmp = (IFX_TAPI_LINE_VOLUME_t *)p_iostack;
      IFXOS_ASSERT(sizeof(IFX_TAPI_LINE_VOLUME_t) < TAPI_IOCTL_STACKSIZE);
      if (copy_from_user (p_tmp, parg, sizeof(IFX_TAPI_LINE_VOLUME_t)) > 0 )
      {
         ret = IFX_ERROR;
      }
      else
      {
         if (pDrvCtx->PCM.Volume_Set == IFX_NULL)
         {
            ret = IFX_ERROR;
         }
         else
            ret = pDrvCtx->PCM.Volume_Set (pChannel->pLLChannel, p_tmp);
      }
      break;
   }

   case IFX_TAPI_COD_VOLUME_SET:
      if (ptr_chk(pDrvCtx->COD.Volume_Set, "pDrvCtx->COD.Volume_Set"))
      {
         IFX_TAPI_PKT_VOLUME_t *p_tmp = (IFX_TAPI_PKT_VOLUME_t *)p_iostack;
         IFXOS_ASSERT(sizeof(IFX_TAPI_PKT_VOLUME_t) < TAPI_IOCTL_STACKSIZE);

         if (copy_from_user (p_tmp, parg, sizeof(IFX_TAPI_PKT_VOLUME_t)) > 0 )
         {
            ret = IFX_ERROR;
         }
         else
         {
            ret = pDrvCtx->COD.Volume_Set (pChannel->pLLChannel, p_tmp);
         }
      }
      else
      {
         TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
               ("TAPI: Setting COD volume not supported by LL driver\n\r"));
         ret = IFX_ERROR;
      }
      break;

   case IFX_TAPI_SIG_DETECT_ENABLE:
   {
      IFX_TAPI_SIG_DETECTION_t *p_tmp = (IFX_TAPI_SIG_DETECTION_t *)p_iostack;
      IFXOS_ASSERT(sizeof(IFX_TAPI_SIG_DETECTION_t) < TAPI_IOCTL_STACKSIZE);
      if (copy_from_user (p_tmp, parg, sizeof(IFX_TAPI_SIG_DETECTION_t)) > 0 )
      {
         ret = IFX_ERROR;
      }
      else
      {
         if (pDrvCtx->SIG.MFTD_Enable == IFX_NULL)
         {
            ret = IFX_ERROR;
         }
         else
            ret = pDrvCtx->SIG.MFTD_Enable (pChannel->pLLChannel, p_tmp);
      }
      break;
   }

   case IFX_TAPI_SIG_DETECT_DISABLE:
   {
      IFX_TAPI_SIG_DETECTION_t *p_tmp = (IFX_TAPI_SIG_DETECTION_t *)p_iostack;
      IFXOS_ASSERT(sizeof(IFX_TAPI_SIG_DETECTION_t) < TAPI_IOCTL_STACKSIZE);
      if (copy_from_user (p_tmp, parg, sizeof(IFX_TAPI_SIG_DETECTION_t)) > 0 )
      {
         ret = IFX_ERROR;
      }
      else
      {
         if (pDrvCtx->SIG.MFTD_Disable == IFX_NULL)
         {
            ret = IFX_ERROR;
         }
         else
            ret = pDrvCtx->SIG.MFTD_Disable (pChannel->pLLChannel, p_tmp);
      }
      break;
   }

   case IFX_TAPI_TONE_CPTD_START:
   {
      IFX_TAPI_TONE_CPTD_t *p_tmp = (IFX_TAPI_TONE_CPTD_t *)p_iostack;
      IFXOS_ASSERT(sizeof(IFX_TAPI_TONE_CPTD_t) < TAPI_IOCTL_STACKSIZE);
      if (copy_from_user (p_tmp, parg, sizeof(IFX_TAPI_TONE_CPTD_t)) > 0 )
      {
         ret = IFX_ERROR;
      }
      else
      {
         ret = TAPI_Phone_DetectToneStart(pDrvCtx, pChannel, p_tmp);
      }
      break;
   }

   case IFX_TAPI_ENC_AGC_CFG:
   {
      IFX_TAPI_ENC_AGC_CFG_t *p_tmp = (IFX_TAPI_ENC_AGC_CFG_t *) p_iostack;

      IFXOS_ASSERT(sizeof(IFX_TAPI_ENC_AGC_CFG_t) < TAPI_IOCTL_STACKSIZE);

      if (copy_from_user (p_tmp, parg, sizeof(IFX_TAPI_ENC_AGC_CFG_t)) > 0 )
      {
         ret = IFX_ERROR;
      }
      else
      {
         if (pDrvCtx->COD.AGC_Cfg == IFX_NULL)
         {
            TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
                  ("TAPI: pDrvCtx->COD.AGC_Cfg is NULL. "
                   "(File: %s, line: %d)\n", __FILE__, __LINE__));

            ret = IFX_ERROR;
         }
         else
         {
            ret = pDrvCtx->COD.AGC_Cfg(pChannel->pLLChannel, p_tmp);
         }
      }
      break;
   }

   case IFX_TAPI_ENC_ROOM_NOISE_DETECT_START:
      if (ptr_chk(pDrvCtx->COD.ENC_RoomNoise, "pDrvCtx->COD.ENC_RoomNoise"))
      {
         IFX_TAPI_ENC_ROOM_NOISE_DETECT_t *p_tmp =
            (IFX_TAPI_ENC_ROOM_NOISE_DETECT_t *) p_iostack;
         IFXOS_ASSERT(sizeof(IFX_TAPI_ENC_ROOM_NOISE_DETECT_t) < TAPI_IOCTL_STACKSIZE);

         if (copy_from_user (p_tmp, parg, sizeof(IFX_TAPI_ENC_ROOM_NOISE_DETECT_t)) > 0 )
         {
            ret = IFX_ERROR;
         }
         else
         {
            ret = pDrvCtx->COD.ENC_RoomNoise(pChannel->pLLChannel, IFX_TRUE,
                                             p_tmp->nThreshold,
                                             p_tmp->nVoicePktCnt,
                                             p_tmp->nSilencePktCnt);
         }
      }
      else
      {
         TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
               ("TAPI: Room-Noise feature not supported by LL driver\n\r"));
         ret = IFX_ERROR;
      }
      break;


#ifdef DECT_SUPPORT
   /* DECT services */
   case IFX_TAPI_DECT_ACTIVATION_SET:
      if (ptr_chk(pDrvCtx->DECT.Enable, "pDrvCtx->DECT.Enable"))
      {
         ret = pDrvCtx->DECT.Enable(pChannel->pLLChannel,
                                    (IFX_operation_t)ioarg == IFX_ENABLE ? 1 : 0);
      }
      else
      {
         TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
               ("TAPI: DECT Encoder config not supported by LL driver\n\r"));
         ret = IFX_ERROR;
      }
      break;

   case IFX_TAPI_DECT_CFG_SET:
      if (ptr_chk(pDrvCtx->DECT.Ch_Cfg, "pDrvCtx->DECT.Ch_Cfg"))
      {
         IFX_TAPI_DECT_CFG_t *p_tmp = (IFX_TAPI_DECT_CFG_t *)p_iostack;
         IFXOS_ASSERT(sizeof(IFX_TAPI_DECT_CFG_t) < TAPI_IOCTL_STACKSIZE);
         if (copy_from_user (p_tmp, parg, sizeof(IFX_TAPI_DECT_CFG_t)) > 0 )
         {
            TRACE(TAPI_DRV,DBG_LEVEL_HIGH,
                 ("TAPI: cannot copy from user space\n\r"));
            ret = IFX_ERROR;
         }
         else
         {
            ret = pDrvCtx->DECT.Ch_Cfg(pChannel->pLLChannel,
                                       p_tmp->nEncDelay, p_tmp->nDecDelay);
         }
      }
      else
      {
         TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
               ("TAPI: DECT channel config not supported by LL driver\n\r"));
         ret = IFX_ERROR;
      }
      break;

   case IFX_TAPI_DECT_ENC_CFG_SET:
      if (ptr_chk(pDrvCtx->DECT.ENC_Cfg, "pDrvCtx->DECT.ENC_Cfg"))
      {
         IFX_TAPI_DECT_ENC_CFG_t *p_tmp =
            (IFX_TAPI_DECT_ENC_CFG_t *)p_iostack;
         IFXOS_ASSERT(sizeof(IFX_TAPI_DECT_ENC_CFG_t) < TAPI_IOCTL_STACKSIZE);
         if (copy_from_user (p_tmp, parg, sizeof(IFX_TAPI_DECT_ENC_CFG_t)) > 0 )
         {
            TRACE(TAPI_DRV,DBG_LEVEL_HIGH,
                 ("TAPI: cannot copy from user space\n\r"));
            ret = IFX_ERROR;
         }
         else
         {
            ret = pDrvCtx->DECT.ENC_Cfg(pChannel->pLLChannel,
                                        p_tmp->nEncType, p_tmp->nFrameLen);
         }
      }
      else
      {
         TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
               ("TAPI: DECT Encoder config not supported by LL driver\n\r"));
         ret = IFX_ERROR;
      }
      break;

   case IFX_TAPI_DECT_VOLUME_SET:
      if (ptr_chk(pDrvCtx->DECT.Gain_Set, "pDrvCtx->DECT.Gain_Set"))
      {
         IFX_TAPI_PKT_VOLUME_t *p_tmp = (IFX_TAPI_PKT_VOLUME_t *)p_iostack;
         IFXOS_ASSERT(sizeof(IFX_TAPI_PKT_VOLUME_t) < TAPI_IOCTL_STACKSIZE);

         if (copy_from_user (p_tmp, parg, sizeof(IFX_TAPI_PKT_VOLUME_t)) > 0 )
         {
            ret = IFX_ERROR;
         }
         else
         {
            ret = pDrvCtx->DECT.Gain_Set (pChannel->pLLChannel, p_tmp);
         }
      }
      else
      {
         TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
               ("TAPI: Setting DECT volume not supported by LL driver\n\r"));
         ret = IFX_ERROR;
      }
      break;

   case IFX_TAPI_DECT_STATISTICS_GET:
      if (ptr_chk(pDrvCtx->DECT.Statistic, "pDrvCtx->DECT.Statistic"))
      {
         IFX_TAPI_DECT_STATISTICS_t *p_tmp =
            (IFX_TAPI_DECT_STATISTICS_t *)p_iostack;
         IFXOS_ASSERT(sizeof(IFX_TAPI_DECT_STATISTICS_t) < TAPI_IOCTL_STACKSIZE);

         if (copy_from_user (p_tmp, parg, sizeof(IFX_TAPI_DECT_STATISTICS_t)) > 0 )
         {
            TRACE(TAPI_DRV,DBG_LEVEL_HIGH,
                 ("TAPI: cannot copy from user space\n\r"));
            ret = IFX_ERROR;
         }
         else
         {
            ret = pDrvCtx->DECT.Statistic(pChannel->pLLChannel, p_tmp);
            if ((ret == IFX_SUCCESS) &&
                (copy_to_user (parg, p_tmp, sizeof(IFX_TAPI_DECT_STATISTICS_t)) > 0))
            {
               TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
                    ("TAPI: cannot copy to user space\n\r"));
               ret = IFX_ERROR;
            }
         }
      }
      else
      {
         TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
               ("TAPI: DECT get statistic not supported by LL driver\n\r"));
         ret = IFX_ERROR;
      }
      break;
#endif /* DECT_SUPPORT */

#ifdef KPI_SUPPORT
   /* Kernel Packet Interface configuration */
   case IFX_TAPI_KPI_CH_CFG_SET:
      {
         IFX_TAPI_KPI_CH_CFG_t *p_tmp = (IFX_TAPI_KPI_CH_CFG_t *) p_iostack;
         IFXOS_ASSERT(sizeof(IFX_TAPI_KPI_CH_CFG_t) < TAPI_IOCTL_STACKSIZE);

         if (copy_from_user (p_tmp, parg, sizeof(IFX_TAPI_KPI_CH_CFG_t)) > 0 )
         {
            ret = IFX_ERROR;
         }
         else
         {
            ret = IFX_TAPI_KPI_ChCfgSet (pChannel, p_tmp);
         }
      }
      break;
#endif /* KPI_SUPPORT */


   /* TEST services */
   case IFX_TAPI_TEST_LOOP:
      if (ptr_chk(pDrvCtx->ALM.TestLoop, "ALM.TestLoop"))
      {
         IFX_TAPI_TEST_LOOP_t *p_tmp = (IFX_TAPI_TEST_LOOP_t *)p_iostack;
         IFXOS_ASSERT(sizeof(IFX_TAPI_TEST_LOOP_t) < TAPI_IOCTL_STACKSIZE);

         if (copy_from_user (p_tmp, parg, sizeof(IFX_TAPI_TEST_LOOP_t)) > 0)
         {
            TRACE(TAPI_DRV,DBG_LEVEL_HIGH,
                 ("TAPI: cannot copy from user space\n\r"));
            ret = IFX_ERROR;
         }
         else
         {
            ret = pDrvCtx->ALM.TestLoop (pChannel->pLLChannel, p_tmp);
         }
      }
      else
      {
         TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
               ("TAPI: TEST ALM-Loop not supported by LL driver\n\r"));
         ret = IFX_ERROR;
      }
      break;

   /* to be completed... */
#if 0
   case IFX_TAPI_POLL_CONFIG:
   {
      IFX_TAPI_POLL_CONFIG_t *pPollCfg = (IFX_TAPI_POLL_CONFIG_t *)p_iostack;
      IFXOS_ASSERT(sizeof(IFX_TAPI_POLL_CONFIG_t) < TAPI_IOCTL_STACKSIZE);

      if (copy_from_user (pPollCfg, parg, sizeof(IFX_TAPI_POLL_CONFIG_t)) > 0 )
      {
         ret = IFX_ERROR;
      }
      else
      {
         ret = TAPI_IrqPollConf(pPollCfg);
      }
      break;
   }

   case IFX_TAPI_POLL_ADD:
   {
      ret = TAPI_AddDev_PollPkts(pDrvCtx, pTapiDev);
      ret = TAPI_AddDev_PollEvts(pDrvCtx, pTapiDev);
      break;
   }

   case IFX_TAPI_POLL_REM:
   {
      ret = TAPI_RemDev_PollPkts(pDrvCtx, pTapiDev);
      ret = TAPI_RemDev_PollEvts(pDrvCtx, pTapiDev);
      break;
   }

   case IFX_TAPI_POLL_TEST:
   {
      ret = TAPI_Poll_Test();
      break;
   }

   case IFX_TAPI_POLL_WRITE:
   {
      IFX_TAPI_POLL_DATA_t *pPollDown = (IFX_TAPI_POLL_DATA_t *)p_iostack;
      IFXOS_ASSERT(sizeof(IFX_TAPI_POLL_DATA_t) < TAPI_IOCTL_STACKSIZE);

      if (copy_from_user (pPollDown, parg, sizeof(IFX_TAPI_POLL_DATA_t)) > 0 )
      {
         ret = IFX_ERROR;
      }
      else
      {
         void *pPktsArray[IFX_TAPI_POLL_QUEUE];
         void **ppPkts;
         int pPktsNum = pPollDown->pPktsNum;

         ppPkts = pPollDown->ppPkts;
         for (i = 0; i < pPktsNum;)
         {
            /* skip any NULL pointers in the array */
            while (*ppPkts == NULL)
            {
               *ppPkts++;
            }
            pPkt = *ppPkts++;

            if (copy_from_user (pPollDown, parg,
                                sizeof(IFX_TAPI_POLL_DATA_t)) > 0 )
            {
               ret = IFX_ERROR;
            }
         }
         ret = TAPI_Poll_Down (pPollDown->ppPkts, &pPollDown->pPktsNum);
      }
      break;
   }

   case IFX_TAPI_POLL_READ:
      /* to be completed... */
      ret = TAPI_Poll_Up (pPollDown->ppPkts, &pPollDown->pPktsNum);
      break;
#endif

   default:
      TRACE(TAPI_DRV,DBG_LEVEL_HIGH,
           ("TAPI: Ioctl 0x%08X not handled by TAPI\n\r", iocmd));
      ret = IFX_ERROR;
      break;
   }

   IFXOS_FREE(p_iostack);

   return ret;
}

/**
   Poll implementation for the device.

   \param file pointer to the file descriptor
   \param wait pointer to the poll table

   \return
    status of the events occured on the device which are
    0, IFXOS_SYSEXCEPT, IFXOS_SYSREAD, IFXOS_SYSWRITE
   \remark
   This function does the following functions:
      - Put the event queue in the wait table and sleep if select
        is called on the device itself.
      - Put the read/write queue in the wait table and sleep if select
        is called on the channels
*/
static unsigned int ifx_tapi_poll (struct file *file, poll_table *wait)
{
   IFX_int32_t ret = 0, i;
#ifdef TAPI_ONE_DEVNODE
   IFX_int32_t j;
#endif /* TAPI_ONE_DEVNODE */
   TAPI_DEV     *pTapiDev  = (TAPI_DEV*) file->private_data;
   TAPI_CHANNEL *pTapiCh   = IFX_NULL;
   int res;

   /* install the poll queues of events to poll on */
   res = TAPI_OS_InstallPollQueue (pTapiDev, (IFX_void_t*) file,
                                  (IFX_void_t *)wait);
   if (res == IFX_ERROR)
      return ret;
#ifdef TAPI_ONE_DEVNODE
   if (pTapiDev->nChannel == IFX_TAPI_DEVICE_CH_NUMBER &&
       pTapiDev->bSingleFd == IFX_TRUE)
   {
      IFX_TAPI_DRV_CTX_t* pDrvCtx = IFX_TAPI_Get_Device_Driver_Context (
         pTapiDev->pDevDrvCtx->majorNumber);

      for (j= 0;j < pDrvCtx->maxDevs; ++j)
      {
         pTapiDev = &pDrvCtx->pTapiDev[j];
         pTapiDev->bNeedWakeup = IFX_TRUE;
         for (i = 0; i < pTapiDev->nMaxChannel; i++)
         {
            TAPI_CHANNEL *pTapiCh = &(pTapiDev->pTapiChanelArray[i]);
            if (pTapiCh != IFX_NULL && !IFX_TAPI_EventFifoEmpty(pTapiCh))
            {
               /* exception available so return action */
               ret |= IFXOS_SYSREAD;
               pTapiDev->bNeedWakeup = IFX_FALSE;
               break;
            }
         }
      }
   }
   else
#endif /* TAPI_ONE_DEVNODE */
   {
   if (pTapiDev->nChannel == IFX_TAPI_DEVICE_CH_NUMBER)
   {
      pTapiDev->bNeedWakeup = IFX_TRUE;
      for (i = 0; i < pTapiDev->nMaxChannel; i++)
      {
         TAPI_CHANNEL *pTapiCh = &(pTapiDev->pTapiChanelArray[i]);
         if (pTapiCh != IFX_NULL)
         if (!IFX_TAPI_EventFifoEmpty(pTapiCh))
         {
            /* exception available so return action */
            ret |= IFXOS_SYSREAD;
            pTapiDev->bNeedWakeup = IFX_FALSE;
            break;
         }
      }
   }
   else
   {
      /* if device ptr, no check for Tapi */
      pTapiCh = (TAPI_CHANNEL*)file->private_data;
      ret |= TAPI_SelectCh (pTapiCh, (IFX_int32_t)wait, (IFX_int32_t)file);
   }
   }

   return ret;
}


#if CONFIG_PROC_FS
/**
   Read the version information from the driver.

   \param buf destination buffer

   \return
   length
*/
static int proc_get_tapi_version(char *buf)
{
    int len;

    len = sprintf(buf, "%s\n\r", &TAPI_WHATVERSION[4]);
    len += sprintf(buf + len, "Compiled on %s, %s for Linux kernel %s\n\r",
                   __DATE__, __TIME__, UTS_RELEASE);

    return len;
}


/**
   Read the status information from the driver.

   \return
   length
*/
static int proc_get_tapi_status(
    char *buf      /**< destination buffer */
)
{
   IFX_TAPI_DRV_CTX_t *pDrvCtx  = IFX_NULL;
   TAPI_DEV           *pTapiDev = IFX_NULL;
   int len=0;
   int i, j;

   for (i = 0; i < TAPI_MAX_LL_DEVICES; i++)
   {
      if (gHLDrvCtx [i].bInUse == IFX_TRUE)
      {
         if (gHLDrvCtx [i].pDrvCtx != IFX_NULL)
         {
            pDrvCtx = gHLDrvCtx [i].pDrvCtx;
            for (j = 0; j < pDrvCtx->maxDevs; j++)
            {
               pTapiDev = &(pDrvCtx->pTapiDev[j]);
               len += sprintf(buf+len,
                      "************************************\n\r");
               len += sprintf(buf+len,
                      "pTapiDev[%d]->bInitialized = %d\n\r",
                       j,pTapiDev->bInitialized);
               len += sprintf(buf+len,
                      "------------------------------------\n\r");
            }
         }
      }
   }
   return len;
}


/**
  Read the registered low level driver information

  \return
  length

*/

static int proc_get_tapi_registered_drivers(char *buf)
{
   IFX_TAPI_DRV_CTX_t *pDrvCtx = IFX_NULL;
   int i, len = 0;


   for (i = 0; i < TAPI_MAX_LL_DEVICES; i++)
   {
      if (gHLDrvCtx [i].bInUse == IFX_TRUE)
      {
         if (gHLDrvCtx [i].pDrvCtx != IFX_NULL)
         {
            pDrvCtx = gHLDrvCtx [i].pDrvCtx;
            len += sprintf(buf+len,
                "Driver \t  version \t major \t devices devname \n\r");
            len += sprintf(buf+len,
               "==================================================\n\r");

            len += sprintf(buf+len, "%s    %s \t %d \t %d \t /dev/%s\n\r",
                 pDrvCtx->drvName, pDrvCtx->drvVersion,
                 pDrvCtx->majorNumber,pDrvCtx->maxDevs, pDrvCtx->devNodeName);
         }
      }
   }

   return len;
}





/**
   The proc filesystem: function to read an entry.
   This function provides information of proc files to the user space

   \return
   length
*/
static int proc_read_tapi(char *page, char **start, off_t off,
                          int count, int *eof, void *data)
{
    int len;

    int (*fn)(char *buf);

    /* write data into the page */
    if (data != IFX_NULL)
    {
        fn = data;
        len = fn(page);
    }
    else
        return 0;

    if (len <= off+count)
        *eof = 1;
    *start = page + off;
    len -= off;
    if (len>count)
        len = count;
    if (len<0)
        len = 0;
    /* return the data length  */
    return len;
}

/**
   Initialize and install the proc entry

\return
   -1 or 0 on success
\remark
   Called by the kernel.
*/
static int proc_install_tapi_entries(void)
{
   struct proc_dir_entry *driver_proc_node;

   /* install the proc entry */
   TRACE(TAPI_DRV, DBG_LEVEL_LOW, ("TAPI_DRV: using proc fs\n\r"));
   driver_proc_node = proc_mkdir( "driver/" DRV_TAPI_NAME, IFX_NULL);
   if (driver_proc_node != IFX_NULL)
   {
      create_proc_read_entry( "version" , S_IFREG|S_IRUGO,
                             driver_proc_node, proc_read_tapi,
                             (void *)proc_get_tapi_version );
      create_proc_read_entry( "status" , S_IFREG|S_IRUGO,
                             driver_proc_node, proc_read_tapi,
                             (void *)proc_get_tapi_status );
      create_proc_read_entry( "registered_drivers" , S_IFREG|S_IRUGO,
                             driver_proc_node, proc_read_tapi,
                             (void *)proc_get_tapi_registered_drivers );

   }
   else
   {
      TRACE(TAPI_DRV,DBG_LEVEL_HIGH,("TAPI_DRV: cannot create proc entry\n\r"));
      return -1;
   }
   return 0;
}
#endif /* CONFIG_PROC_FS */

/**
   Initialize the module (support device file system)

   \return
   Error code or 0 on success
   \remark
   Called by the kernel.
*/

static int __init ifx_tapi_module_init(void)
{
   int i;

   printk("%s, (c) 2007 Infineon Technologies AG\n\r", &TAPI_WHATVERSION[4]);

   SetTraceLevel(TAPI_DRV, debug_level);
   SetLogLevel  (TAPI_DRV, debug_level);

   for (i=0; i < TAPI_MAX_LL_DEVICES; i++)
   {
      gHLDrvCtx[i].pDrvCtx = IFX_NULL;
   }

   IFX_TAPI_On_Driver_Load();

#if CONFIG_PROC_FS
   proc_install_tapi_entries();
#endif

#ifndef MODULE
   vmmc_module_init();
#endif

#ifdef KPI_SUPPORT
   /* Initialise the Kernel Packet Interface */
   IFX_TAPI_KPI_Init();
#endif /* KPI_SUPPORT */

   return IFX_SUCCESS;
}

/**
   Clean up the module if unloaded.

   \remark
   Called by the kernel.
*/
static void __exit ifx_tapi_module_exit(void)
{
   int i;
   TAPI_CHANNEL *pChannel;
   int nCh;
   IFX_TAPI_DRV_CTX_t *pDrvCtx;

   printk (KERN_INFO "Removing Highlevel TAPI module\n");

#ifdef KPI_SUPPORT
   /* Cleanup the Kernel Packet Interface */
   IFX_TAPI_KPI_Cleanup();
#endif /* KPI_SUPPORT */

   /* Free the device data block */
   for (i = 0; i < TAPI_MAX_LL_DEVICES; i++)
   {
      pDrvCtx = gHLDrvCtx[i].pDrvCtx;

      if (pDrvCtx != IFX_NULL)
      {
         for (nCh = 0; nCh < pDrvCtx->maxChannels; nCh++)
         {
            pChannel = &(pDrvCtx->pTapiDev->pTapiChanelArray[nCh]);
            if (pChannel != IFX_NULL)
               IFX_TAPI_EventDispatcher_Exit(pChannel);
         }
         IFXOS_FREE (gHLDrvCtx[i].pDrvCtx);
         gHLDrvCtx[i].pDrvCtx = IFX_NULL;
      }
   }
#if CONFIG_PROC_FS
   remove_proc_entry("driver/" DRV_TAPI_NAME "/version" ,0);
   remove_proc_entry("driver/" DRV_TAPI_NAME "/status",0);
   remove_proc_entry("driver/" DRV_TAPI_NAME "/registered_drivers",0);
   remove_proc_entry("driver/" DRV_TAPI_NAME ,0);
#endif /* CONFIG_PROC_FS */

   IFX_TAPI_On_Driver_Unload();

   TRACE(TAPI_DRV,DBG_LEVEL_NORMAL,("TAPI_DRV: cleanup successful\n\r"));
}

/**
   Function create a timer.

   \param pTimerEntry - Functionpointer to the call back function
   \param nArgument   - pointer to TAPI channel structure

   \return
   Timer_ID    - pointer to internal timer structure
   \remark
   Initialize a task queue which will be scheduled once a timer interrupt occurs
   to execute the appropriate operation in a process context, process in which
   semaphores ... are allowed.
   Please notice that this task has to run under the keventd process, in which it
   can be executed thousands of times within a single timer tick.
*/
Timer_ID TAPI_Create_Timer(TIMER_ENTRY pTimerEntry, IFX_int32_t nArgument)
{
   Timer_ID pTimerData;

   /* allocate memory for the channel */
   pTimerData = kmalloc(sizeof(*pTimerData), GFP_KERNEL);
   if (pTimerData == IFX_NULL)
      return IFX_NULL;

   /* set function to be called after timer expires */
   pTimerData->pTimerEntry = pTimerEntry;
   pTimerData->nArgument = nArgument;

   init_timer(&(pTimerData->Timer_List));

   /* set timer call back function */
   pTimerData->Timer_List.function = TAPI_timer_call_back;
   pTimerData->Timer_List.data = (IFX_uint32_t) pTimerData;

   /* Initialize Timer Task */
#ifdef LINUX_2_6
   #if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,20))
   INIT_WORK(&(pTimerData->timerTask), TAPI_tqueue, (IFX_void_t *) pTimerData);
   #else
   INIT_WORK(&(pTimerData->timerTask), TAPI_tqueue);
   #endif
#else
   /* initialize tq_struct member of Timer_ID structure */
   memset(&(pTimerData->timerTask), 0, sizeof(struct tq_struct));
   pTimerData->timerTask.routine = TAPI_tqueue;
   pTimerData->timerTask.data = (IFX_void_t *) pTimerData;
#endif /* LINUX_2_6 */
   return pTimerData;
}

/**
   Function set and starts a timer with a specific time. It can be choose if the
   timer starts periodically.

   \param Timer_ID      - pointer to internal timer structure
   \param nTime         - Time in ms
   \param bPeriodically - Starts the timer periodically or not
   \param bRestart      - Restart the timer or normal start

   \return
   Returns an error code: IFX_TRUE / IFX_FALSE
*/
IFX_boolean_t TAPI_SetTime_Timer(Timer_ID Timer, IFX_uint32_t nTime,
                                 IFX_boolean_t bPeriodically,
                                 IFX_boolean_t bRestart)
{
   if (Timer == 0)
   {
     TRACE(TAPI_DRV,DBG_LEVEL_HIGH,
          ("\n\rDRV_ERROR: TAPI_SetTime_Timer (Timer == 0)\n\r"));
     return IFX_FALSE;
   }
   Timer->Periodical_Time = HZ*nTime/1000;
   /* prevent restart of driver */
   Timer->bPeriodical = IFX_FALSE;
   /* remove driver from list */
   del_timer_sync(&(Timer->Timer_List));

   Timer->bPeriodical = bPeriodically;

   Timer->Timer_List.expires = jiffies + Timer->Periodical_Time;
   add_timer(&(Timer->Timer_List));

   return IFX_TRUE;
}

/**
   Function stop a timer.

   \param Timer_ID      - pointer to internal timer structure

   \return
   Returns an error code: IFX_TRUE / IFX_FALSE
*/
IFX_boolean_t TAPI_Stop_Timer(Timer_ID Timer)
{
   /* stop timer */
   /* prevent restart of driver */
   Timer->bPeriodical = IFX_FALSE;
   /* remove driver from list */
   del_timer_sync(&(Timer->Timer_List));
   return (IFX_TRUE);
}

/**
   Function delete a timer.

   \param Timer_ID      - pointer to internal timer structure

   \return
   Returns an error code: IFX_TRUE / IFX_FALSE
*/
IFX_boolean_t TAPI_Delete_Timer(Timer_ID Timer)
{
   if (Timer)
   {
      TAPI_Stop_Timer(Timer);
      /* free memory */
      kfree(Timer);
      return IFX_TRUE;
   }
   return IFX_FALSE;
}

/**
   Helper function to get a periodical timer

   \param arg   - pointer to corresponding timer ID

   \return
      void
   \remark
   This function will be executed in  the process context, so to avoid
   scheduling in Interrupt Mode while working with semaphores etc...
   The task is always running under the keventd process and is also running
   very quickly. Even on a very heavily loaded system, the latency in the
   scheduler queue is quite small
*/
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,20))
IFX_LOCAL IFX_void_t TAPI_tqueue (IFX_void_t *pWork)
#else /* for Kernel newer or equal 2.6.20 */
IFX_LOCAL IFX_void_t TAPI_tqueue (struct work_struct *pWork)
#endif
{
   Timer_ID Timer = (Timer_ID) pWork;
   /* Call TAPI Timer function */
   Timer->pTimerEntry(Timer, Timer->nArgument);
   if (Timer->bPeriodical)
   {
      /* remove driver from list */
      del_timer_sync(&(Timer->Timer_List));
      /* start new timer, then call function to gain precision */
      Timer->Timer_List.expires = jiffies + Timer->Periodical_Time;
      add_timer(&(Timer->Timer_List));
   }
}

/**
   Helper function to get a periodical timer

   \param
    arg   - pointer to corresponding timer ID
   \return
      None
*/
IFX_LOCAL IFX_void_t TAPI_timer_call_back (unsigned long arg)
{
   Timer_ID Timer = (Timer_ID) arg;
   /* do the operation in process context,
      not in interrupt context */
#ifdef LINUX_2_6
   schedule_work (&(Timer->timerTask));
#else
   schedule_task (&(Timer->timerTask));
#endif /* LINUX_2_6 */
}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,20))
/* High Level Event Dispatcher function. */
IFX_LOCAL IFX_void_t Deferred_Worker (void *pWork)
{
   IFX_TAPI_EXT_EVENT_PARAM_t *pEvParam = (IFX_TAPI_EXT_EVENT_PARAM_t *) pWork;
   pEvParam->pFunc(pEvParam);
}
#else /* for Kernel newer or equal 2.6.20 */
IFX_LOCAL IFX_void_t Deferred_Worker (struct work_struct *pWork)
{
   IFX_TAPI_EXT_EVENT_PARAM_t *pEvParam = (IFX_TAPI_EXT_EVENT_PARAM_t *) pWork;
   pEvParam->pFunc(pEvParam);
}
#endif

/**
   Defer work to process context
\param pFunc - pointer to function to be called
\param pParam - parameter passed to the function
\return IFX_SUCCESS or IFX_ERROR in case of an error
\remarks
*/
IFX_return_t TAPI_DeferWork (void *pFunc, void *pParam)
{
   IFX_return_t ret = IFX_SUCCESS;
   IFX_TAPI_EXT_EVENT_PARAM_t *pEvParam = (IFX_TAPI_EXT_EVENT_PARAM_t *) pParam;
#ifdef LINUX_2_6
   struct work_struct         *pTapiWs;

   pTapiWs = (struct work_struct *) &pEvParam->tapiWs;
   pEvParam->pFunc   = (void *)pFunc;

   #if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,20))
   INIT_WORK(pTapiWs, Deferred_Worker, (void *) pEvParam);
   #else
   INIT_WORK(pTapiWs, Deferred_Worker);
   #endif

   if (schedule_work (pTapiWs) == 0)
   {
      TRACE (TAPI_DRV, DBG_LEVEL_HIGH,
            ("ERROR: TAPI_DeferWork schedule_work() failed\n\r"));
      ret = IFX_ERROR;
   }
#else
   struct tq_struct           *pTapiTq;

   pTapiTq           = (struct tq_struct *)&pEvParam->tapiTq;
   pTapiTq->routine  = Deferred_Worker;
   pEvParam->pFunc   = (void *)pFunc;
   pTapiTq->data     = (void *)pEvParam;
   pTapiTq->sync     = 0;
   if (schedule_task (pTapiTq) == 0)
   {
      TRACE (TAPI_DRV, DBG_LEVEL_HIGH,
            ("ERROR: TAPI_DeferWork schedule_task() failed\n\r"));
      ret = IFX_ERROR;
   }
#endif /* LINUX_2_6 */
   return ret;
}

/**
   Executes the select for the channel fd
\param
   pTapiCh  - handle to channel control structure
\param
   node   - node list
\param
   opt    - optinal argument, which contains needed information for
           IFXOS_SleepQueue
\return
   System event qualifier. Either 0 or IFXOS_SYSREAD
\remarks
   This function needs operating system services, that are hidden by
   IFXOS macros.
*/
IFX_int32_t TAPI_SelectCh (TAPI_CHANNEL* pTapiCh, IFX_int32_t node,
                              IFX_int32_t opt)
{
   IFX_TAPI_DRV_CTX_t *pDrvCtx = (IFX_TAPI_DRV_CTX_t*)
                                 pTapiCh->pTapiDevice->pDevDrvCtx;
   IFX_TAPI_LL_DEV_t  *pLLDev = pTapiCh->pTapiDevice->pLLDev;
   IFX_uint32_t  flags = 0;
   IFX_int32_t   ret = 0;
#ifdef TAPI_FAX_T38
   IFX_TAPI_T38_STATUS_t TapiFaxStatus;
#endif /* TAPI_FAX_T38 */

#ifdef TAPI_FAX_T38
   /* Get the Status from the low level driver */
   if (ptr_chk(pDrvCtx->COD.T38_Status_Get,
              "pDrvCtx->COD.T38_Status_Get"))
      pDrvCtx->COD.T38_Status_Get (pTapiCh->pLLChannel, &TapiFaxStatus);
#endif /* TAPI_FAX_T38 */

   IFXOS_SleepQueue(pTapiCh->wqRead, node, opt);
#ifdef TAPI_FAX_T38
   IFXOS_SleepQueue (pTapiCh->wqWrite, node, opt);
   if ((TapiFaxStatus.nStatus & IFX_TAPI_FAX_T38_TX_ON) &&
       (pTapiCh->bFaxDataRequest == IFX_TRUE))
   {
      /* task should write a new packet now */
      ret |= IFXOS_SYSWRITE;
   }
#endif /* TAPI_FAX_T38 */
   /* select on a voice channel -- only implemented for TAPI */
   flags |= CF_NEED_WAKEUP;
   /* clear flags first, then apply new flags */

   if (pDrvCtx->IRQ.LockDevice != IFX_NULL)
   {
   pDrvCtx->IRQ.LockDevice (pLLDev);
   }

#ifdef TAPI_PACKET
   if (!fifoEmpty(pTapiCh->pUpStreamFifo))
   {
      flags |= CF_WAKEUPSRC_STREAM;
      flags &= ~CF_NEED_WAKEUP;
      ret |= IFXOS_SYSREAD;
   }
#endif /* TAPI_PACKET */
   pTapiCh->nFlags &= ~(CF_WAKEUPSRC_GR909 | CF_WAKEUPSRC_STREAM |
                        CF_WAKEUPSRC_TAPI | CF_NEED_WAKEUP);
   pTapiCh->nFlags |= flags;

   if (pDrvCtx->IRQ.UnlockDevice != IFX_NULL)
   {
   pDrvCtx->IRQ.UnlockDevice (pLLDev);
   }

   return ret;
}


/**
   Install the Poll Queues

   \param fileptr    - ptr to file structure
   \param poll_entry - ptr to the poll table

   \return
      None
   \remark
   Call this function in the poll system call
*/
IFX_LOCAL int TAPI_OS_InstallPollQueue (TAPI_DEV *pTapiDev,
                                        void *fileptr, void *poll_entry)
{
   struct file *file = (struct file *)fileptr;
   poll_table  *wait  = (poll_table *)poll_entry;
   TAPI_CHANNEL *pTapiCh = IFX_NULL;

   /* check device ptr */
   if (pTapiDev == IFX_NULL)
   {
      TRACE (TAPI_DRV,DBG_LEVEL_HIGH,("ERROR: Install poll queue failed\n\r"));
      return -ENODEV;
   }
   if ((pTapiDev->nChannel) == IFX_TAPI_DEVICE_CH_NUMBER)
   {
      /* exception on channel 0 */
      poll_wait(file, &(pTapiDev->wqEvent), wait);
   }
   else if ((pTapiDev->nChannel < pTapiDev->nMaxChannel))
   {
      /* set channel ptr */
      pTapiCh = file->private_data;
      {
         /* for blocking voice channels a wait queue is installed to sleep
            in select/poll syscall until read data is available */
         poll_wait(file, &(pTapiCh->semReadBlock), wait);
      }
   }
   return IFX_SUCCESS;
}

#ifdef TAPI_EXT_KEYPAD
/**
   Returns the Channel context
   \param - void
   \return
      TAPI_CHANNEL pointer
   \remark
   Since when a key event is reported from hapi channel context is not known.
   This is used to get 0th channel since Audio has only one channel for INCA2
*/
TAPI_CHANNEL * TAPI_Get_Channel_Ctx()
{
   TAPI_CHANNEL *pChannel = IFX_NULL;
   int i;
   IFX_boolean_t drv_found = IFX_FALSE;

   for(i=0;i<TAPI_MAX_LL_DEVICES;i++)
   {
      if(strcmp(gHLDrvCtx[i].pDrvCtx->drvName,"vmmc") == 0)
      {
         TRACE( TAPI_DRV, DBG_LEVEL_LOW,("Found vmmc at pos. i= %d\n",i));
         drv_found = IFX_TRUE;
         break;
      }
   }

   if(drv_found)
      pChannel=gHLDrvCtx[i].pDrvCtx->pTapiDev->pTapiChanelArray;
   else
      pChannel=gHLDrvCtx[0].pDrvCtx->pTapiDev->pTapiChanelArray;

   return pChannel;
}
#endif /* TAPI_EXT_KEYPAD */


module_init (ifx_tapi_module_init);
module_exit (ifx_tapi_module_exit);

/****************************************************************************/

MODULE_AUTHOR           ("Infineon Technologies AG, COM AC SD VA");
MODULE_DESCRIPTION      ("TAPI Driver - www.infineon.com");
MODULE_SUPPORTED_DEVICE ("TAPI DEVICE");
MODULE_LICENSE          ("GPL");

EXPORT_SYMBOL (IFX_TAPI_Register_LL_Drv);
EXPORT_SYMBOL (IFX_TAPI_Unregister_LL_Drv);
EXPORT_SYMBOL (IFX_TAPI_ResetChState);

EXPORT_SYMBOL (IFX_TAPI_Event_Dispatch);

#ifdef TAPI_CID
EXPORT_SYMBOL (TAPI_Phone_GetCidRxBuf);
EXPORT_SYMBOL (TAPI_Cid_Abort);
EXPORT_SYMBOL (TAPI_Cid_IsActive);
EXPORT_SYMBOL (TAPI_Cid_UseSequence);
#endif /* TAPI_CID */

EXPORT_SYMBOL (TAPI_Create_Timer);
EXPORT_SYMBOL (TAPI_SetTime_Timer);
EXPORT_SYMBOL (TAPI_Stop_Timer);
EXPORT_SYMBOL (TAPI_Delete_Timer);

EXPORT_SYMBOL (TAPI_Tone_Set_Source);
EXPORT_SYMBOL (TAPI_ToneState);

EXPORT_SYMBOL(bufferPoolGet);
EXPORT_SYMBOL(bufferPoolPut);
EXPORT_SYMBOL(bufferPoolElementSize);

EXPORT_SYMBOL(fifoInit);
EXPORT_SYMBOL(fifoReset);
EXPORT_SYMBOL(fifoPut);
EXPORT_SYMBOL(fifoGet);
EXPORT_SYMBOL(fifoEmpty);
EXPORT_SYMBOL(fifoFree);

#ifdef QOS_SUPPORT
EXPORT_SYMBOL(irq_IFX_TAPI_Qos_PktEgressSched);
EXPORT_SYMBOL(IFX_TAPI_Qos_HL_Init);
EXPORT_SYMBOL(irq_IFX_TAPI_Qos_HL_PktEgress);
EXPORT_SYMBOL(irq_IFX_TAPI_Qos_PacketRedirection);
#endif /* QOS_SUPPORT */

#ifdef KPI_SUPPORT
EXPORT_SYMBOL(IFX_TAPI_KPI_WaitForData);
EXPORT_SYMBOL(IFX_TAPI_KPI_ReadData);
EXPORT_SYMBOL(IFX_TAPI_KPI_WriteData);
EXPORT_SYMBOL(IFX_TAPI_KPI_ChGet);
EXPORT_SYMBOL(irq_IFX_TAPI_KPI_PutToEgress);
#endif /* KPI_SUPPORT */

#ifdef TAPI_PACKET
EXPORT_SYMBOL(TAPI_VoiceBufferPoolHandle_Get);
EXPORT_SYMBOL(TAPI_UpStreamFifo_Get);
EXPORT_SYMBOL(TAPI_DownStreamFifo_Get);
EXPORT_SYMBOL(TAPI_VoiceBufferPoolGet);
EXPORT_SYMBOL(TAPI_ClearFifo);
#endif /* TAPI_PACKET */
EXPORT_SYMBOL(TAPI_Phone_Event_HookState);

/* FXO related exports */
EXPORT_SYMBOL(IFX_TAPI_Register_DAA_Drv);
EXPORT_SYMBOL(IFX_TAPI_FXO_Event_Dispatch);

#endif /* LINUX */
