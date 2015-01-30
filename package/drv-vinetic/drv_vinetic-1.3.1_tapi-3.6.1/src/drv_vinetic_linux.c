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

******************************************************************************
   Module      : drv_vinetic_linux.c
   Description : This file contains the implementation of the linux specific
                 driver functions.
*******************************************************************************/

/* ============================= */
/* Includes                      */
/* ============================= */
#include "drv_api.h"
#ifdef LINUX
#include "linux/module.h"
#include "linux/kernel.h"
#include "linux/init.h"
#include "drv_vinetic_api.h"
#include "drv_vinetic_main.h"
#include "drv_vinetic_int.h"

#include "drv_vinetic_tone.h"
#include "drv_vinetic_cod.h"
#include "drv_vinetic_con.h"
#include "drv_vinetic_pcm.h"
#include "drv_vinetic_alm.h"
#include "drv_vinetic_alm_cpe.h"
#include "drv_vinetic_init.h"

#ifdef QOS_SUPPORT
#include "drv_vinetic_qos.h"
#endif /* QOS_SUPPORT */

#if ((VIN_CFG_FEATURES & VIN_FEAT_GR909) && defined (VIN_V14_SUPPORT))
#include "drv_vinetic_gr909.h"
#endif /* VIN_CFG_FEATURES & VIN_FEAT_GR909 && VIN_V14_SUPPORT */

#ifdef LINUX_2_6
#include "linux/version.h"
#undef CONFIG_DEVFS_FS
#endif /* LINUX_2_6 */

#ifdef CONFIG_DEVFS_FS
#include "linux/devfs_fs_kernel.h"
#endif /* CONFIG_DEVFS_FS */

#include "drv_tapi_ll_interface.h"

/* ============================= */
/* Local Macros & Definitions    */
/* ============================= */
IFX_TAPI_LL_DEV_t* IFX_TAPI_LL_Prepare_Dev (TAPI_DEV *pTapiDev, IFX_uint32_t devNum);

MODULE_DESCRIPTION("VINETIC device driver - http://www.infineon.com");
MODULE_AUTHOR("Infineon Technologies AG, COM AC SD VA");
MODULE_SUPPORTED_DEVICE ("VINETIC-CPE");
MODULE_LICENSE("GPL");

/****************************************************************************
Description:
   macro to map the system function with takes care of interrupt handling
   registration.
Arguments:
   irq   -  irq number
   func  -  interrupt handler callback function
   arg   -  argument of interrupt handler callback function
Remarks:
   The macro is by default mapped to the operating system method. For systems
   integrating different routines, this macro must be adapted in the user con-
   figuration header file.
****************************************************************************/
#ifndef VIN_SYS_REGISTER_INT_HANDLER
#define VIN_SYS_REGISTER_INT_HANDLER(irq,flags,func,arg)             \
         request_irq ((irq),(func),(flags),DEV_NAME,(void*)(arg))
#endif /* VIN_SYS_REGISTER_INT_HANDLER */

/****************************************************************************
Description:
   macro to map the system function with takes care of interrupt handling
   unregistration.
Arguments:
   irq   -  irq number
Remarks:
   The macro is by default mapped to the operating system method. For systems
   integrating different routines, this macro must be adapted in the user con-
   figuration header file.
****************************************************************************/
#ifndef VIN_SYS_UNREGISTER_INT_HANDLER
#define VIN_SYS_UNREGISTER_INT_HANDLER(irq,arg)  \
         free_irq((irq), (arg))
#endif /* VIN_SYS_UNREGISTER_INT_HANDLER */

/* ============================= */
/* Global variable definition    */
/* ============================= */

VINETIC_DEVICE *sVinetic_Devices[VINETIC_MAX_DEVICES] = {0};
IFXOS_mutex_t SemDrvIF=0;

IFX_uint16_t major = VINETIC_MAJOR;
IFX_uint16_t minorBase = VINETIC_MINOR_BASE;
IFX_char_t*  devName = "vin";

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,17))
MODULE_PARM( major, "h" );
MODULE_PARM( minorBase, "h" );
MODULE_PARM( devName, "s" );
#else
module_param(major, ushort, 0);
module_param(minorBase, ushort, 0);
module_param(devName, charp, 0);
#endif

MODULE_PARM_DESC(major ,"Device Major number");
MODULE_PARM_DESC(minorBase ,"Number of devices to be created");
MODULE_PARM_DESC(devName ,"Name of the device");

/* ============================= */
/* Global function declaration   */
/* ============================= */

/* ============================= */
/* Local function declaration    */
/* ============================= */

/* operating system callbacks for polling/interrupt */
#ifdef LINUX_2_6
IFX_LOCAL irqreturn_t VINETIC_irq_handler (int irq, void *pDev, struct pt_regs *regs);
#else
IFX_LOCAL IFX_void_t VINETIC_irq_handler  (int irq, void *pDev, struct pt_regs *regs);
#endif /* LINUX_2_6 */

/* operating system proc interface */
#ifdef VINETIC_USE_PROC
static int VINETIC_Read_Proc   (char *buf, char **start, off_t offset,
                                 int len, int *eof, void *data);
#endif /* VINETIC_USE_PROC */

/* Local helper functions */
IFX_LOCAL IFX_int32_t OS_Release_Baseadress (VINETIC_DEVICE* pDev);
IFX_LOCAL IFX_void_t  ReleaseDevice         (VINETIC_DEVICE *pDev);

/* ============================= */
/* Local variable definition     */
/* ============================= */

/* IFX_LOCAL memory for Device pointer */
IFX_LOCAL VINETIC_DEVICE VDevices[VINETIC_MAX_DEVICES];
/* list of vinetic interrupts, neccessary for shared interrupt handling */
IFX_LOCAL vinetic_interrupt_t* VIN_irq_head = IFX_NULL;


/* ============================= */
/* Local function definition     */
/* ============================= */

/*******************************************************************************
Description:
   This function initializes the VINETIC base address.
   If necessary, OS specific mapping is done here.
Arguments:
   pDev      - handle of the VINETIC device structure.
   nBaseAddr - physical address for vinetic access.
Return Value:
   IFX_SUCCESS / IFX_ERROR.
Remarks:
   none
*******************************************************************************/
IFX_int32_t OS_Init_Baseadress(VINETIC_DEVICE* pDev, IFX_uint32_t nBaseAddr)
{
   IFX_uint16_t *ptr;
   int  result;

   if (pDev->pBaseAddr != IFX_NULL)
      OS_Release_Baseadress(pDev);

   /* check the io region */
   result = check_mem_region(nBaseAddr, VINETIC_REGION_SIZE);
   if (result != 0)
   {
      TRACE (VINETIC, DBG_LEVEL_HIGH,
            ("ERROR: can't get I/O address 0x%X\n\r", (unsigned int)nBaseAddr));
      return IFX_ERROR;
   }
   /* request the io region (can not fail) */
   request_mem_region (nBaseAddr, VINETIC_REGION_SIZE, DEV_NAME);
   /* calculate the virtual address from the physical address */
   if ((ptr = ioremap_nocache(nBaseAddr, VINETIC_REGION_SIZE)) == NULL)
   {
      TRACE (VINETIC, DBG_LEVEL_HIGH,
         ("ERROR: vinetic address region could not be mapped!\n\r"));
      return IFX_ERROR;
   }
   /* remember physical address */
   pDev->nBaseAddr = nBaseAddr;

   /* and remember virtual address */
   pDev->pBaseAddr = ptr;

   return IFX_SUCCESS;
}

/*******************************************************************************
Description:
   This function initializes the VINETIC base address.
   If necessary, OS specific mapping is done here.
Arguments:
   pDev      - handle of the VINETIC device structure.
   nBaseAddr - physical address for vinetic access.
Return Value:
   IFX_SUCCESS / IFX_ERROR.
Remarks:
   none
*******************************************************************************/
IFX_LOCAL IFX_int32_t OS_Release_Baseadress(VINETIC_DEVICE* pDev)
{
   /* unmap the io region */
   if (pDev->pBaseAddr)
      iounmap((IFX_void_t*)pDev->pBaseAddr);
   /* release the io region */
   if(pDev->nBaseAddr)
      release_mem_region(pDev->nBaseAddr, VINETIC_REGION_SIZE);

   /* remember physical address */
   pDev->nBaseAddr = 0;
   /* and remember virtual address */
   pDev->pBaseAddr = IFX_NULL;

   return IFX_SUCCESS;
}


/*******************************************************************************
Description:
   This function allocates a temporary kernel buffer
      and copies the user buffer contents to it.
Arguments:
   p_buffer - pointer to the user buffer.
   size     - size of the buffer.
Return Value:
   pointer to kernel buffer or NULL
Remarks:
   Buffer has to be released with \ref OS_UnmapBuffer
*******************************************************************************/
IFX_void_t* OS_MapBuffer(IFX_void_t* p_buffer, IFX_uint32_t size)
{
   IFX_void_t* kernel_buffer;

   if (p_buffer == NULL)
   {
      return NULL;
   }
   kernel_buffer = vmalloc(size);
   if (kernel_buffer == NULL)
   {
      return NULL;
   }
   if (copy_from_user(kernel_buffer, p_buffer, size) > 0)
   {
      vfree(kernel_buffer);
      return NULL;
   }

   return kernel_buffer;
}

/*******************************************************************************
Description:
   This function releases a temporary kernel buffer.
Arguments:
   p_buffer - pointer to the kernel buffer.
Return Value:
   None.
Remarks:
  Counterpart of \ref OS_MapBuffer
*******************************************************************************/
IFX_void_t OS_UnmapBuffer(IFX_void_t* p_buffer)
{
   if (p_buffer != NULL)
   {
      vfree(p_buffer);
      p_buffer = NULL;
   }
}

/****************************************************************************
Description:
   IRQ handler

Arguments:
   irq    - number of the irq
   pDev   - pointer to the device structure
   regs   -
Return:
   none
****************************************************************************/
#ifdef LINUX_2_6
IFX_LOCAL irqreturn_t VINETIC_irq_handler(int irq, void *pDev, struct pt_regs *regs)
{
   return VINETIC_interrupt_routine((VINETIC_DEVICE*)pDev);
}
#else
IFX_LOCAL IFX_void_t VINETIC_irq_handler(int irq, void *pDev, struct pt_regs *regs)
{
   VINETIC_interrupt_routine((VINETIC_DEVICE*)pDev);
}
#endif /* LINUX_2_6 */

/*******************************************************************************
Description:
   Install the Interrupt Handler
Arguments:
   pDev  - pointer to device structure
   nIrq  - interrupt number
Return:
   none
*******************************************************************************/
IFX_void_t OS_Install_VineticIRQHandler (VINETIC_DEVICE* pDev, IFX_int32_t nIrq)
{
   vinetic_interrupt_t* pIrq;
   vinetic_interrupt_t* pIrqPrev = IFX_NULL;
   VINETIC_DEVICE* tmp_pDev;
   unsigned long intstatus;

   if (pDev->pIrq != IFX_NULL)
      OS_UnInstall_VineticIRQHandler(pDev);

   pIrq = VIN_irq_head;

   /* try to find existing Irq with same number */
   while (pIrq != IFX_NULL)
   {
      pIrqPrev = pIrq;
      if (pIrq->nIrq == nIrq)
         break;
      pIrq = pIrq->next_irq;
   }

   if (pIrq == IFX_NULL)
   {
      pIrq = kmalloc(sizeof(vinetic_interrupt_t),GFP_KERNEL);
      if (pIrq == IFX_NULL)
      {
         /** \todo set error status! */
         return;
      }
      memset(pIrq, 0, sizeof(vinetic_interrupt_t));
      IFXOS_MutexInit(pIrq->intAcc);
      IFXOS_MutexLock(pIrq->intAcc);
      IFXOS_LOCKINT(intstatus);

      /* register new dedicated or first of shared interrupt */
      pDev->pIrq = pIrq;
      pDev->pInt_NextDev = IFX_NULL;
      pIrq->pdev_head = pDev;
      pIrq->next_irq = IFX_NULL;
      pIrq->nIrq = nIrq;
      /* Install the Interrupt routine */
      if (VIN_SYS_REGISTER_INT_HANDLER (nIrq, (SA_INTERRUPT | SA_SHIRQ),
          VINETIC_irq_handler, pDev) != 0)
      {
         TRACE (VINETIC, DBG_LEVEL_HIGH, ("ERROR: request interrupt failed\n\r"));
      }
      else
      {
         pIrq->bRegistered = IFX_TRUE;
         VIN_DISABLE_IRQLINE(nIrq);
      }

      /* add to list of interrupts */
      /* this list is not used by the interrupt handler itself,
         so no additional intLock necessary! */
      if (VIN_irq_head == IFX_NULL)
         VIN_irq_head = pIrq;
      else
         pIrqPrev->next_irq = pIrq;
      IFXOS_UNLOCKINT(intstatus);
   }
   else
   {
      IFXOS_LOCKINT(intstatus);
      /* Install the Interrupt routine (add to shared irq list) */
      if (VIN_SYS_REGISTER_INT_HANDLER (nIrq, (SA_INTERRUPT | SA_SHIRQ),
          VINETIC_irq_handler, pDev) != 0)
      {
         TRACE (VINETIC, DBG_LEVEL_HIGH,
            ("ERROR: request shared interrupt failed\n\r"));
      }
      else
      {
         /* add into the list of an existing shared interrupt */
         pDev->pIrq = pIrq;
         pDev->pInt_NextDev = IFX_NULL;
         tmp_pDev = pIrq->pdev_head;
         while (tmp_pDev->pInt_NextDev != IFX_NULL)
         {
            tmp_pDev = tmp_pDev->pInt_NextDev;
         }
         tmp_pDev->pInt_NextDev = pDev;
      }
      IFXOS_UNLOCKINT(intstatus);
   }
}

/*******************************************************************************
Description:
   Uninstall the Interrupt Handler
Arguments:
   pDev  - pointer to device structure
Return:
   none
*******************************************************************************/
IFX_void_t OS_UnInstall_VineticIRQHandler(VINETIC_DEVICE* pDev)
{
   vinetic_interrupt_t* pIrq = pDev->pIrq;
   vinetic_interrupt_t* pIrqPrev;
   VINETIC_DEVICE* tmp_pDev = pIrq->pdev_head;

   /* remove this pDev from the list of this pIrq */
   if (tmp_pDev == pDev)
   {
      pIrq->pdev_head = pDev->pInt_NextDev;
   }
   else
   {
      /* find the element which has this pDev as next */
      while (tmp_pDev->pInt_NextDev != pDev)
      {
         if (tmp_pDev->pInt_NextDev == IFX_NULL)
         {
            TRACE (VINETIC, DBG_LEVEL_HIGH,
               ("ERROR: device not found in interrupt list, "
                "cannot uninstall!\n\r"));
            return;
         }
         tmp_pDev = tmp_pDev->pInt_NextDev;
      }
      tmp_pDev->pInt_NextDev = pDev->pInt_NextDev;
   }
   pDev->pInt_NextDev = IFX_NULL;
   pDev->pIrq = IFX_NULL;

   /* if this was the last device, so cleanup the irq */
   if (pIrq->pdev_head == IFX_NULL)
   {
      /* IFXOS_MutexLock(pIrq->intAcc); */

      if (pIrq->bIntEnabled == IFX_TRUE)
      {
         VIN_DISABLE_IRQLINE (pIrq->nIrq);
      }
      if (pIrq->bRegistered == IFX_TRUE)
      {
         VIN_SYS_UNREGISTER_INT_HANDLER(pIrq->nIrq, pDev);
      }
      /* remove this pIrq from irq-list */
      if (VIN_irq_head == pIrq)
      {
         VIN_irq_head = pIrq->next_irq;
      }
      else
      {
         pIrqPrev = VIN_irq_head;
         while (pIrqPrev->next_irq != IFX_NULL)
         {
            if (pIrqPrev->next_irq == pIrq)
               break;
            pIrqPrev = pIrqPrev->next_irq;
         }
         if (pIrqPrev->next_irq != IFX_NULL)
            pIrqPrev->next_irq = pIrq->next_irq;
      }
      IFXOS_MutexDelete(pIrq->intAcc);
      kfree(pIrq);
   }
   else
      VIN_SYS_UNREGISTER_INT_HANDLER(pIrq->nIrq, pDev);
}



/****************************************************************************
Description:
   Get device pointer
Arguments:
   nr - number of device
   pDev - returned pointer to device structure
Return:
   IFX_SUCCESS or IFX_ERROR in case of an error
Remarks:
   Used by board / main module to reset device states
****************************************************************************/
IFX_int32_t OS_GetDevice (IFX_int32_t nr, VINETIC_DEVICE** pDev)
{
   if (nr > VINETIC_MAX_DEVICES || sVinetic_Devices[nr] == NULL)
      return IFX_ERROR;
   *pDev = sVinetic_Devices[nr];
   return IFX_SUCCESS;
}

/****************************************************************************
Description:
   Init device structure
Arguments:
   nDevn - number of device to init
Return Value:
   IFX_SUCCESS on success, IFX_ERROR if init fails
Remarks:
   Called form Board_DevOpen().
****************************************************************************/
IFX_int32_t OS_InitDevice (IFX_int32_t nDevn)
{
   /* get device pointer */
   VINETIC_DEVICE* pDev = (VINETIC_DEVICE*) &VDevices[nDevn];

   /* Initialization */
   memset(pDev, 0, sizeof(VINETIC_DEVICE));
   pDev->nDevNr = nDevn;
   /* cache the pointer */
   sVinetic_Devices[nDevn] = pDev;

   /* Os and board independend initializations, resets all values */
   if (VINETIC_InitDevMember (pDev) == IFX_ERROR)
   {
       TRACE (VINETIC, DBG_LEVEL_HIGH,
             ("ERROR: Initialization of device structure failed\n\r"));
       return IFX_ERROR;
   }
   return IFX_SUCCESS;
}

/****************************************************************************
Description:
   Clean up the device if it is no longer open by some user space program.
Arguments:
   pDev - pointer to the device structure
Return Value:
   None.
Remarks:
   None.
****************************************************************************/
IFX_LOCAL IFX_void_t ReleaseDevice(VINETIC_DEVICE *pDev)
{
   /*check if we really have to do with device structure */
   if (pDev == NULL || pDev->nChannel != 0)
      return;

   /* free the interrupt */
   if (pDev->pIrq != IFX_NULL)
      OS_UnInstall_VineticIRQHandler (pDev);
   /*    ... and the memory mapping */
   if (pDev->pBaseAddr != IFX_NULL)
      OS_Release_Baseadress(pDev);

   /* memory cleanup */
   VINETIC_ExitDev(pDev);
   /* decrease in use count */
   if(pDev->nInUse)
      pDev->nInUse--;
   /* clear the cached  pointer */
   sVinetic_Devices [pDev->nDevNr] = NULL;
   /* free the memory  */
   pDev = NULL;
}



/** \defgroup VinKernel VINETIC Kernel API
\remarks
  Kernel interface for open/close from Linux kernel mode
*/

/* @{ */

/**
   Open the device from kernel mode.
\param
   nDev - index of the VINETIC device
\param
   nCh - index of the VINETIC channel (1 = channel 0, ...)
\return
   handle to device/channel or IFX_ERROR on error
\remarks
  If not already done this will
   - allocate internal memory for each new device
   - allocate io memory
   - initialize the device
   - set up the interrupt
*/
int VINETIC_OpenKernel(int nDev, int nCh)
{
   VINETIC_DEVICE *pDev = NULL;
   VINETIC_CHANNEL *pCh = NULL;
   IFX_int32_t ret = IFX_ERROR;

   /* check the device number for the type 0 */
   if ((nDev >= VINETIC_MAX_DEVICES) || (nDev < 0))
   {
      TRACE (VINETIC, DBG_LEVEL_NORMAL,
            ("Warning: Driver only supports %d VINETIC(s)\n\r",
             VINETIC_MAX_DEVICES));
      ret = -ENODEV;
      goto OPEN_ERROR;
   }
   /* check if first open on this device */
   if (sVinetic_Devices[nDev] == NULL)
   {
      /* Now allocate and initialize all data. */
      if (OS_InitDevice(nDev) == IFX_ERROR)
      {
         ret = -ENODEV;
         goto OPEN_ERROR;
      }
   }
   IFXOS_MutexLock(SemDrvIF);
   pDev = sVinetic_Devices[nDev];
   /* increase in use count */
   pDev->nInUse++;

   if (nCh == 0)
   {
      /* return the device pointer */
      ret = (IFX_int32_t)pDev;
   }
   else
   {
      /* return the channel pointer */
      pCh = &pDev->pChannel [nCh - 1];
      pCh->nChannel = nCh;
      if (pCh->nInUse == 0)
      {
         VINETIC_UpdateChMember (pCh);
      }
      pCh->nInUse++;
      ret = (IFX_int32_t)pCh;
   }

   /* increment module use counter */
   MOD_INC_USE_COUNT;

   TRACE (VINETIC, DBG_LEVEL_LOW,
         ("Open Kernel VINETIC device Nr. %d, channel %d\n\r", nDev, nCh));

   IFXOS_MutexUnlock (SemDrvIF);
   return ret;

OPEN_ERROR:
   if (pDev != NULL)
   {
      ReleaseDevice(pDev);
      TRACE (VINETIC,DBG_LEVEL_HIGH, ("Open Kernel Error 0x%X\n", pDev->err));
   }
   return ret;
}

/**
   Release the device from Kernel mode.
\param
   nHandle - handle returned by VINETIC_OpenKernel
\return
   IFX_SUCCESS or error code
\remarks
*/
int VINETIC_ReleaseKernel(int nHandle)
{
   VINETIC_DEVICE *pDev;
   VINETIC_CHANNEL *pCh;

   IFXOS_MutexLock(SemDrvIF);
   if (((VINETIC_DEVICE*)nHandle)->nChannel == 0)
   {
       pDev = (VINETIC_DEVICE*)nHandle;
      TRACE (VINETIC, DBG_LEVEL_LOW,
            ("Closing Kernel device %d\n\r", pDev->nDevNr));
      /* memory will be released when module is removed */
      pDev->nInUse--;
   }
   else
   {
      pCh = (VINETIC_CHANNEL*)nHandle;
      pCh->nInUse--;
      pCh->pParent->nInUse--;
#if (VIN_CFG_FEATURES & VIN_FEAT_PACKET)
      /* free Fifo memory */
#if 0
      /* Moved to high level tapi */
      if (pCh->nInUse == 0)
      {
         IFXOS_FREE (pCh->pFifo);
      }
#endif
#endif /* (VIN_CFG_FEATURES & VIN_FEAT_PACKET) */
      TRACE (VINETIC, DBG_LEVEL_LOW,
      ("Closing device channel %d (%d)\n\r",
       pCh->nChannel, pCh->nInUse));
   }

   IFXOS_MutexUnlock(SemDrvIF);
   /* decrement use counter */
   MOD_DEC_USE_COUNT;
   return IFX_SUCCESS;
}
/* @} */

#if ((VIN_CFG_FEATURES & VIN_FEAT_GR909) && defined (VIN_V14_SUPPORT))
/****************************************************************************
Description:
   gr909 task
Arguments:
   device - handle to a device (configuration or channel device)
Return Value:
   IFX_SUCCESS or IFX_ERROR
Remarks:
   will be executed by poll function as a background task.
****************************************************************************/
IFX_int32_t VINETIC_GR909_OSTask (IFX_uint32_t device)
{
   VINETIC_DEVICE *pDev = (VINETIC_DEVICE *)device;
   VINETIC_CHANNEL *pCh;
   IFX_int32_t     i;

   if (pDev->nChannel == 0)
   {
      /* call GR909 callback state machine */
      for (i = 0; i < VINETIC_MAX_ANA_CH_NR; i++)
      {
         pCh = &pDev->pChannel [i];
         if (pCh->hostCh.Gr909.gr909tskId <= 0)
            continue;
         if (pCh->hostCh.Gr909.Usr.status != GR909_TEST_RUNNING)
            continue;
         pCh->hostCh.Gr909.Gr909Call (pCh);
      }
   }
   else
   {
      pCh = (VINETIC_CHANNEL *)device;
      if (pCh->hostCh.Gr909.gr909tskId <= 0)
         return IFX_ERROR;
      if (pCh->hostCh.Gr909.Usr.status != GR909_TEST_RUNNING)
         return IFX_ERROR;
      pCh->hostCh.Gr909.Gr909Call (pCh);
   }

   return IFX_SUCCESS;
}

/****************************************************************************
Description:
   starts gr909 task ( in compliance with vxworks code)
Arguments:
   pCh  - handle to VINETIC_CHANNEL structure
Return Value:
   IFX_SUCCESS
Remarks:
   None.
****************************************************************************/
IFX_int32_t VINETIC_GR909_OSCreateTask (VINETIC_CHANNEL *pCh)
{
   /* set running state */
   pCh->hostCh.Gr909.Usr.status = GR909_TEST_RUNNING;
   /* set task id */
   pCh->hostCh.Gr909.gr909tskId = IFX_TRUE;
   /* wake up for state machine processing */
   wake_up_interruptible(&pCh->hostCh.Gr909.wqState);
   return 0;
}

/****************************************************************************
Description:
   deletes gr909 task ( in compliance with vxworks code
Arguments:
   pCh  - handle to VINETIC_CHANNEL structure
Return Value:
   IFX_SUCCESS
Remarks:
   None.
****************************************************************************/
IFX_int32_t VINETIC_GR909_OSDeleteTask (VINETIC_CHANNEL *pCh)
{
   pCh->hostCh.Gr909.gr909tskId = IFX_FALSE;
   return IFX_SUCCESS;
}

#endif /* (VIN_CFG_FEATURES & VIN_FEAT_GR909) && VIN_V14_SUPPORT */



/****************************************************************************
Description:
   Initialize the low level device pointer.
Arguments:
   high level pointer.
Return Value:
   None
Remarks:
   Called by the High Level TAPI module (tapi_open).
****************************************************************************/
IFX_TAPI_LL_DEV_t* IFX_TAPI_LL_Prepare_Dev (TAPI_DEV *pTapiDev, IFX_uint32_t devNum)
{
   VINETIC_DEVICE* pDev = (VINETIC_DEVICE*) &VDevices[devNum];

   /* Should return the pDev which should be stored by the HL */
   return VINETIC_Prepare_Dev(pTapiDev, pDev, devNum);
}


/****************************************************************************
Description:
   Initialize the low level channel pointer.
Arguments:
   high level channel pointer
   device number
   channel number
Return Value:
   IFX_SUCCESS / IFX_ERROR
Remarks:
   Called by the High Level TAPI module (tapi_open).
****************************************************************************/
IFX_void_t* IFX_TAPI_LL_Prepare_Ch(TAPI_CHANNEL *pTapiCh,
                                   IFX_uint32_t devNum,
                                   IFX_uint32_t chNum)
{
   VINETIC_DEVICE *pDev = &VDevices[devNum];

   /* Should return the pCh which should be stored by the HL */
   return VINETIC_Prepare_Ch(pTapiCh, pDev, chNum);
}


/**
   Open a VINETIC channel

   Just increment the linux module use count.

   \param  inode        Channel number from high level TAPI.
   \param file_p        unused pointer

   \return \ref IFX_SUCCESS

   \remarks
     - Called by the High Level TAPI module (ifx_tapi_open).
     - Channel number 0 stands for the device 1-255 is for channels.
       Please note this is different from the close function!
*/
IFX_int32_t IFX_TAPI_LL_Open(IFX_int32_t inode, IFX_void_t *file_p)
{
#ifdef MODULE
   MOD_INC_USE_COUNT;
#endif

   return IFX_SUCCESS;
}


/**
   Close a VINETIC channel

   \param  inode        Channel number from high level TAPI.
   \param filp          unused pointer
   \param pLLDev        unused pointer

   \return \ref IFX_SUCCESS

   \remarks
     - Called by the High Level TAPI module (ifx_tapi_release).
     - Channel number 0-254 is for channels, 255 is reserved for the device.
       Please note this is different from the open function!
*/
IFX_int32_t IFX_TAPI_LL_Close(IFX_int32_t inode, 
                              IFX_void_t *filp, IFX_TAPI_LL_DEV_t *pLLDev)
{
#ifdef MODULE
   MOD_DEC_USE_COUNT;
#endif

   return IFX_SUCCESS;
}


/****************************************************************************
Description:
   Reads data from the vinetic.
Arguments:
   filp  - file pointer to be opened
   buf   - destination buffer
   count - max size of data to read
   ppos  - unused
Return Value:
   len   - data length
Remarks:
   None.
****************************************************************************/
IFX_int32_t VINETIC_LL_Read(IFX_TAPI_LL_CH_t *pLLCh, IFX_char_t *buf,
                            IFX_int32_t count, IFX_int32_t *ppos)
{
#if (VIN_CFG_FEATURES & VIN_FEAT_PACKET)
   VINETIC_CHANNEL *pCh = (VINETIC_CHANNEL*) pLLCh;
   IFX_int32_t     ret = IFX_SUCCESS;

   /* protect channel against concurrent tasks */
   /** \todo is this ok, if we lock mutex for 65 seconds or more ? */
   IFXOS_MutexLock (pCh->chAcc);
   /* count in bytes */
   ret = pCh->if_read (pCh, (IFX_uint8_t*) buf, count);
   /* release lock */
   IFXOS_MutexUnlock (pCh->chAcc);

   /* count smaller than packet size */
   if (ret == IFX_ERROR)
   {
      /* return linux error code to application */
      ret = -EINVAL;
   }
   else if ((ret == 0) && (pCh->pTapiCh->nFlags & CF_NONBLOCK))
   {
       /* no data available. user should try to read again on next wakeup */
       ret = 0;
   }

   return ret;
#else
   return -EINVAL;
#endif /* (VIN_CFG_FEATURES & VIN_FEAT_PACKET) */
}

/****************************************************************************
Description:
   Writes data to the VINETIC.
Arguments:
   filp  - file pointer to be opened
   buf   - source buffer
   count - data length
   ppos  - unused
Return Value:
   nLength - 0 if failure else the length
Remarks:
   None.
****************************************************************************/
IFX_int32_t VINETIC_LL_Write(IFX_TAPI_LL_CH_t *pLLCh, const char *buf,
                             IFX_int32_t count, IFX_int32_t *ppos)
{
#if (VIN_CFG_FEATURES & VIN_FEAT_PACKET)
   VINETIC_CHANNEL   *pCh = (VINETIC_CHANNEL *)pLLCh;
   IFX_int32_t       len;
   IFX_uint16_t      *pData = IFX_NULL;

   /* Protect channel against concurrent tasks. */
   IFXOS_MutexLock(pCh->chAcc);
   Vinetic_IrqLockDevice(pCh->pParent);

   pData = (IFX_uint16_t *)bufferPoolGet(TAPI_VoiceBufferPoolHandle_Get());
   if (IFX_NULL == pData)
   {
      /* Error getting memory for packet data. */
      TRACE(VINETIC, DBG_LEVEL_HIGH, ("ERR: Reserving memory for packet "
            "data.\n\r"));
      return -EFAULT;
   }
   memset(pData, 0, bufferPoolElementSize(TAPI_VoiceBufferPoolHandle_Get()));

   Vinetic_IrqUnlockDevice(pCh->pParent);
   IFXOS_MutexUnlock(pCh->chAcc);

#if defined(EVALUATION) && defined(VIN_2CPE)
   if (pCh->if_write == VoiceStream_Write)
   {
      /* for this variant, timestamp, commands and data are part of buf */
      if (count > ((MAX_PACKET_WORD + sizeof(IFX_uint32_t)) << 1))
         count = ((MAX_PACKET_WORD + sizeof(IFX_uint32_t)) << 1);
   }
   else
#endif /* EVALUATION && VIN_2CPE */
   {
      if (count > ((MAX_PACKET_WORD - 2) << 1))
         count = ((MAX_PACKET_WORD - 2) << 1);
   }
   /* here the copy is only necessary for linux. In VxWorks the user
      space could pass a buffer with two leading empty words */
   if (copy_from_user ((void *)&pData[2], (void *)buf, (IFX_uint32_t)count))
   {
      return -EFAULT;
   }
   /* protect channel against concurrent tasks */
   IFXOS_MutexLock (pCh->chAcc);
   /* count in words */
   len = pCh->if_write(pCh, (IFX_uint8_t *)pData, count);
   /* release lock */
   IFXOS_MutexUnlock (pCh->chAcc);

   /* Protect channel against concurrent tasks. */
   IFXOS_MutexLock(pCh->chAcc);
   Vinetic_IrqLockDevice(pCh->pParent);
   bufferPoolPut(pData);
   Vinetic_IrqUnlockDevice(pCh->pParent);
   IFXOS_MutexUnlock(pCh->chAcc);

   return len;
#else
   return -EINVAL;
#endif /* (VIN_CFG_FEATURES & VIN_FEAT_PACKET) */
}

#ifdef VINETIC_USE_PROC

/******************************************************************************
   Read the version information from the driver.
\param
   buf - destination buffer
\return
   len
******************************************************************************/
static int VINETIC_get_version_proc (char *buf)
{
   int len;

   len  = sprintf(buf, "%s\n\r", &DRV_VINETIC_WHATVERSION[4]);
   len += sprintf(buf + len, "Compiled on %s, %s for Linux kernel %s\n\r",
                  __DATE__, __TIME__, UTS_RELEASE);
   return len;
}

#ifdef HAVE_CONFIG_H
/******************************************************************************
   Read the configure parameters from the driver.
\param
   buf - destination buffer
\return
   len
******************************************************************************/
static int VINETIC_get_configure_proc (char *buf)
{
   int len;

   len  = sprintf(buf, "configure %s\n\r"
               "-----------------------------------------------------------\n\r"
                       "G.726 bit alignment enc: %s dec: %s\n\r",
                  &DRV_VINETIC_WHICHCONFIG[0],

#ifdef VIN_G726_AAL_BIT_ALIGNMENT_ENCODER
                  "AAL (reverse!)"
#else
                  "RTP (normal)"
#endif
                  ,
#ifdef VIN_G726_AAL_BIT_ALIGNMENT_DECODER
                  "AAL (reverse!)"
#else
                  "RTP (normal)"
#endif
                  );

   return len;
}
#endif /* HAVE_CONFIG_H */

#if (VIN_CFG_FEATURES & VIN_FEAT_PACKET)
/****************************************************************************
   Read the read fifo status from the driver.
\param
   buf - destination buffer
\return
   len
****************************************************************************/
static int VINETIC_readfifo_proc(char *buf)
{
   int len = 0;

#if 0
   VINETIC_DEVICE *pDev;
   IFX_int32_t    dev, ch;

   /* Fifo is moved to high level tapi */
   for (dev = 0; dev < VINETIC_MAX_DEVICES; dev++)
   {
      pDev = &VDevices[dev];
      if (pDev != IFX_NULL)
      {
         len += sprintf(buf + len, "dev %d: err=0x%lX\n\r",
            pDev->nChannel, pDev->err);

         for (ch = 0; ch < VINETIC_MAX_CH_NR; ch++)
         {
            len += sprintf(buf + len, "ch%d: ", ch);
            if (pDev->pChannel[ch].rdFifo.size)
            {
               len += sprintf(buf + len, "pStart=%p, pEnd=%p, pRead=%p, "
                  "pWrite=%p, size=%d %c\n\r",
                  pDev->pChannel[ch].rdFifo.pStart,
                  pDev->pChannel[ch].rdFifo.pEnd,
                  pDev->pChannel[ch].rdFifo.pRead,
                  pDev->pChannel[ch].rdFifo.pWrite,
                  pDev->pChannel[ch].rdFifo.size,
                  (Fifo_isEmpty(&pDev->pChannel[ch].rdFifo) ? 'E' : 'F'));
            }
            else
            {
               len += sprintf(buf + len, "not initialized\n\r");
            }
         }
      }
   }
#endif
   return len;
}
#endif /* (VIN_CFG_FEATURES & VIN_FEAT_PACKET) */

/****************************************************************************
Description:
   The proc filesystem: function to read an entry.
Arguments:
   page     - pointer to page buffer
   start    - pointer to start address pointer
   off      - offset
   count    - length to read
   eof      - end of file
   data     - data (used as function pointer)
Return Value:
   length
Remarks:
   None.
****************************************************************************/
static int VINETIC_Read_Proc (char *page, char **start, off_t off,
   int count, int *eof, void *data)
{
   int (*fn)(char *buf), len;

   if (data != NULL)
   {
      fn = data;
      len = fn (page);
   }
   else
   {
      return 0;
   }
   if (len <= off+count)
   {
      *eof = 1;
   }
   *start = page + off;
   len -= off;
   if (len > count)
   {
      len = count;
   }
   if (len < 0)
   {
      len = 0;
   }

   return len;
}



/******************************************************************************/
/**
   Initialize and install the proc entry

\return
   IFX_SUCCESS or IFX_ERROR
\remark
   Called by the kernel.
*/
/******************************************************************************/
static int VINETIC_install_proc_entry (void)
{
   struct proc_dir_entry *driver_proc_node;

   /* install the proc entry */
   TRACE(VINETIC, DBG_LEVEL_NORMAL, ("vinetic: using proc fs\n\r"));

   driver_proc_node = proc_mkdir( "driver/" DEV_NAME, NULL);
   if (driver_proc_node != NULL)
   {
      create_proc_read_entry("version" , 0, driver_proc_node,
                             VINETIC_Read_Proc, (void *)VINETIC_get_version_proc);
#ifdef HAVE_CONFIG_H
      create_proc_read_entry("configure", 0, driver_proc_node,
                             VINETIC_Read_Proc, (void *)VINETIC_get_configure_proc);
#endif /* HAVE_CONFIG_H */
#if (VIN_CFG_FEATURES & VIN_FEAT_PACKET)
      create_proc_read_entry("readfifo" , 0, driver_proc_node,
                             VINETIC_Read_Proc, (void *)VINETIC_readfifo_proc);
#endif /* (VIN_CFG_FEATURES & VIN_FEAT_PACKET) */
   }
   else
   {
      TRACE(VINETIC, DBG_LEVEL_HIGH,("vinetic: cannot create proc entry\n\r"));
      return IFX_ERROR;
   }

   return IFX_SUCCESS;
}

/******************************************************************************/
/**
   Remove proc entries

\remark
   Called by the kernel.
*/
/******************************************************************************/
static void VINETIC_remove_proc_entry(void)
{
#if (VIN_CFG_FEATURES & VIN_FEAT_PACKET)
   remove_proc_entry("driver/" DEV_NAME "/readfifo",0);
#endif /* (VIN_CFG_FEATURES & VIN_FEAT_PACKET) */
   remove_proc_entry("driver/" DEV_NAME "/version" ,0);
#ifdef HAVE_CONFIG_H
   remove_proc_entry("driver/" DEV_NAME "/configure", 0);
#endif /* HAVE_CONFIG_H */
   remove_proc_entry("driver/" DEV_NAME ,0);

   return;
}

#endif /* VINETIC_USE_PROC */


/* ============================= */
/* Global function definition    */
/* ============================= */

/*******************************************************************************
Description:
Arguments:
   pText - text to be logged
Return:
Remarks:
   implemented to avoid the unresolved symbol problem when loading the module
*******************************************************************************/
IFX_void_t Drv_Log(const IFX_char_t *pText)
{
   printk (pText);
}

/****************************************************************************
Description:
   disables irq line if the driver is in interrupt mode and irq line is
   actually enabled according to device flag bIntEnabled.
   Disable the global interrupt if the device is not in polling mode and no
   interrupt line is connected.
Arguments:
   pDev - vinetic device handle
Return Value:
   none
Remarks
   If the driver works in Polling mode, nothing is done.
   If the driver works in interrupt mode and the irq was already disabled
   (flag bIntEnabled is IFX_FALSE ), the os disable function will
   not be called.

   Very important: It is assumed that disable and enable irq are done
   subsequently and that no routine calling disable/enable is executed
   inbetween as stated in following code example:

Allowed :

   Vinetic_IrqLockDevice;
   .. some instructions
   Vinetic_IrqUnlockDevice

Not allowed:

   routineX (IFX_int32_t x)
   {
      Vinetic_IrqLockDevice;
      .. some instructions;
      Vinetic_IrqUnlockDevice
   }

   routineY (IFX_int32_t y)
   {
      Vinetic_IrqLockDevice;
      routine (x);    <---------------- routineX unlocks interrupts..
      be carefull!
      ... some more instructions;
      Vinetic_IrqUnlockDevice;
   }
****************************************************************************/
IFX_void_t Vinetic_IrqLockDevice (IFX_TAPI_LL_DEV_t *pLLDev)
{
   VINETIC_DEVICE *pDev = (VINETIC_DEVICE *) pLLDev;
   vinetic_interrupt_t* pIrq = pDev->pIrq;

   /* driver used in polling mode : exit */
   if (pDev->IrqPollMode & VIN_EVENT_POLL)
      return;

   if (pIrq != IFX_NULL)
   {
      /* device specific interrupt routine is initialized */
      /* interrupt line was disabled already: exit */
      if (pIrq->bIntEnabled == IFX_FALSE)
         return;
      /* it must be possible to take the mutex before disabling the irq */
      if (!in_interrupt())
          IFXOS_MutexLock(pIrq->intAcc);
      /* invoke board or os routine to disable irq */
      VIN_DISABLE_IRQLINE (pIrq->nIrq);
      /* reset enable flag to signalize that interrupt line is disabled */
      pIrq->bIntEnabled = IFX_FALSE;
   }
   else
   {
      /* there is no specific interrupt service routine configured for that
         device. Disable the global interrupts instead */
      if (pDev->nIrqMask == 0)
      {
         VIN_DISABLE_IRQGLOBAL(pDev->nIrqMask);
      }
   }

#ifdef DEBUG_INT
   pIrq->nCount++;
#endif /* DEBUG_INT */
}

/****************************************************************************
Description:
   enables irq line if the driver is in interrupt mode and irq line is
   actually disabled according to device flag bIntEnabled.
   Enable the global interrupt, if this was disabled by 'Vinetic_IrqDisable'
Arguments:
   pDev - vinetic device handle
Return Value:
   none
Remarks
****************************************************************************/
IFX_void_t Vinetic_IrqEnable (IFX_TAPI_LL_DEV_t *pLLDev)
{
   VINETIC_DEVICE *pDev = (VINETIC_DEVICE *) pLLDev;

   vinetic_interrupt_t* pIrq = pDev->pIrq;

   /* invoke board or os routine to enable irq */
   VIN_ENABLE_IRQLINE(pIrq->nIrq);
}

/****************************************************************************
Description:
   enables irq line if the driver is in interrupt mode and irq line is
   actually disabled according to device flag bIntEnabled.
   Enable the global interrupt, if this was disabled by 'Vinetic_IrqEnable'
Arguments:
   pDev - vinetic device handle
Return Value:
   none
Remarks
****************************************************************************/
IFX_void_t Vinetic_IrqDisable (IFX_TAPI_LL_DEV_t *pLLDev)
{
   VINETIC_DEVICE *pDev = (VINETIC_DEVICE *) pLLDev;
   vinetic_interrupt_t* pIrq = pDev->pIrq;

   /* invoke board or os routine to disable irq */
   VIN_DISABLE_IRQLINE(pIrq->nIrq);
}

/****************************************************************************
Description:
   enables irq line if the driver is in interrupt mode and irq line is
   actually disabled according to device flag bIntEnabled.
   Enable the global interrupt, if this was disabled by 'Vinetic_IrqLockDevice'
Arguments:
   pDev - vinetic device handle
Return Value:
   none
Remarks
   cf Remarks of Vinetic_IrqLockDevice () in this file.
****************************************************************************/
IFX_void_t Vinetic_IrqUnlockDevice (IFX_TAPI_LL_DEV_t *pLLDev)
{
   VINETIC_DEVICE * pDev = (VINETIC_DEVICE *) pLLDev;
   vinetic_interrupt_t* pIrq = pDev->pIrq;

   /* driver used in polling mode : exit */
   if (pDev->IrqPollMode & VIN_EVENT_POLL)
      return;
   if (pIrq != IFX_NULL)
   {
      /* device specific interrupt routine is initialized */
      /* interrupt line was enabled already: exit */
      if (pIrq->bIntEnabled == IFX_TRUE)
         return;
      /* set enable flag to signalize that interrupt line is enabled */
      pIrq->bIntEnabled = IFX_TRUE;
      /* invoke board or os routine to enable irq */
      VIN_ENABLE_IRQLINE(pIrq->nIrq);
      IFXOS_MutexUnlock(pIrq->intAcc);
   }
   else
   {
      /* there is no specific interrupt service routine configured for that
         device. Enable the global interrupts again */
      if (pDev->nIrqMask != 0)
      {
         IFX_uint32_t nMask = pDev->nIrqMask;
         pDev->nIrqMask = 0;
         VIN_ENABLE_IRQGLOBAL(nMask);
      }
   }

#ifdef DEBUG_INT
   pIrq->nCount--;
#endif /* DEBUG_INT */
}

/****************************************************************************
Description:
   Enable the global interrupt if it was disabled by 'Vinetic_IrqLockDevice'.
   Do not enable the interrupt line if it is configured on the device.
   This function has to be called in error case inside the driver to unblock
   global interrupts but do not device interrupt. The interrupt line for that
   specific device is not enable in case of an error.
Arguments:
   pDev - vinetic device handle
Return Value:
   none
Remarks
   cf Remarks of Vinetic_IrqLockDevice () in this file.
****************************************************************************/
IFX_void_t Vinetic_IrqUnlockGlobal (VINETIC_DEVICE *pDev)
{
   if (pDev->nIrqMask != 0)
   {
      IFX_uint32_t nMask = pDev->nIrqMask;
      pDev->nIrqMask = 0;
      VIN_ENABLE_IRQGLOBAL(nMask);
   }

#ifdef DEBUG_INT
   pIrq->nCount--;
#endif /* DEBUG_INT */
}



/****************************************************************************
Description:
   Initialize the module.
   Allocate the major number and register the dev nodes (for DEVFS).
Arguments:
   None.
Return Value:
   None
Remarks:
   Called by the kernel.
   Initializes global mutex, sets trace level and registers the module.
   It calls also functions to initialize the hardware which is used.
****************************************************************************/
static int __init vinetic_module_init(void)
{
   IFX_int32_t result = IFX_SUCCESS;
   IFX_int32_t i = 0;


   IFXOS_MutexInit (SemDrvIF);

   result = VINETIC_DeviceDriverInit();

   for (i = 0; i < VINETIC_MAX_DEVICES; ++i)
   {
      sVinetic_Devices[i] = IFX_NULL;
   }

#ifdef VINETIC_USE_PROC
   VINETIC_install_proc_entry();
#endif /* VINETIC_USE_PROC */

   return result;
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
****************************************************************************/
static void __exit vinetic_module_exit(void)
{
   IFX_int32_t i;

   IFX_TAPI_DRV_CTX_t DrvCtx;

   memset (&(DrvCtx), 0x00, sizeof (IFX_TAPI_DRV_CTX_t));

   DrvCtx.majorNumber = major;
   DrvCtx.minorBase = minorBase;
   DrvCtx.devNodeName = devName;
   DrvCtx.maxDevs = VINETIC_MAX_DEVICES;

   printk(KERN_INFO "vinetic_module_exit: Unregistering LL dev with major number %d and minor base %d\n",DrvCtx.majorNumber, DrvCtx.minorBase);

   /* notify HL Tapi when a LL dev unregisters */
   IFX_TAPI_Unregister_LL_Drv (major);

   TRACE (VINETIC, DBG_LEVEL_LOW, ("cleaning up %s module ...\n\r", DEV_NAME));

   for (i=0; i < VINETIC_MAX_DEVICES; i++)
   {
     if (sVinetic_Devices[i] != NULL)
      {
         ReleaseDevice (sVinetic_Devices[i]);
      }
   }

#if 0
#ifdef DEBUG
   dbgResult();
#endif /* DEBUG */
#endif

#ifndef CONFIG_DEVFS_FS
   /* unregister the module, free the major number */
   unregister_chrdev (VINETIC_MAJOR, DEV_NAME);
#endif /* CONFIG_DEVFS_FS */
#ifdef VINETIC_USE_PROC
   VINETIC_remove_proc_entry();
#endif /* VINETIC_USE_PROC */
}


module_init(vinetic_module_init);
module_exit(vinetic_module_exit);


#endif /* LINUX */

