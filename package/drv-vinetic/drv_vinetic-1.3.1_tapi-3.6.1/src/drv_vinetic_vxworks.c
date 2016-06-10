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

/* ============================= */
/* Includes                      */
/* ============================= */

#include "drv_api.h"
#include "drv_vinetic_api.h"
#include "drv_vinetic_vxworks.h"

#include "drv_vinetic_main.h"
#include "drv_vinetic_int.h"

#include "drv_vinetic_tone.h"
#include "drv_vinetic_cod.h"
#include "drv_vinetic_con.h"
#include "drv_vinetic_pcm.h"
#include "drv_vinetic_alm.h"
#include "drv_vinetic_alm_cpe.h"
#include "drv_vinetic_init.h"

#ifndef VIN_2CPE

#if ((VIN_CFG_FEATURES & VIN_FEAT_GR909) && (VIN_V14_SUPPORT))
#include "drv_vinetic_gr909.h"
#endif /* ((VIN_CFG_FEATURES & VIN_FEAT_GR909) && (VIN_V14_SUPPORT)) */

#endif /* VIN_2CPE */

#if (CPU_FAMILY==PPC)
/* for INUM_TO_IVEC / IVEC_TO_INUM */
#include <arch/ppc/ivPpc.h>
#endif

#include "drv_tapi_ll_interface.h"

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
#define VIN_SYS_REGISTER_INT_HANDLER(irq,func,arg)             \
            intConnect(INUM_TO_IVEC(irq), (VOIDFUNCPTR)(func), \
            (IFX_int32_t)(arg))
#endif /* VIN_SYS_REGISTER_INT_HANDLER */

/****************************************************************************
Description:
   macro to map the system function with takes care of interrupt handling
   unregistration.
Arguments:
   irq   - irq number
Remarks:
   The macro is by default mapped to the operating system method. For systems
   integrating different routines, this macro must be adapted in the user con-
   figuration header file.
****************************************************************************/
#ifndef VIN_SYS_UNREGISTER_INT_HANDLER
#define VIN_SYS_UNREGISTER_INT_HANDLER(irq)  \
         VIN_SYS_REGISTER_INT_HANDLER((irq), OS_IRQHandler_Dummy, (irq))
#endif /* VIN_SYS_UNREGISTER_INT_HANDLER */

/* ============================= */
/* Local variable definition     */
/* ============================= */

/* list of vinetic interrupts, neccessary for shared interrupt handling */
IFX_LOCAL vinetic_interrupt_t* VIN_irq_head                         = IFX_NULL;
VINETIC_DEVICE*               sVinetic_Devices[VINETIC_MAX_DEVICES] = {0};
IFXOS_mutex_t                 SemDrvIF                              = 0;
/* IFX_LOCAL memory for Device pointer */
IFX_LOCAL VINETIC_DEVICE      VDevices [VINETIC_MAX_DEVICES];

IFX_uint16_t major = VINETIC_MAJOR;
IFX_uint16_t minorBase = VINETIC_MINOR_BASE;
IFX_char_t*  devName = "vin";


/* ============================= */
/* Local function definition     */
/* ============================= */

/* Local helper functions */
IFX_LOCAL IFX_void_t OS_IRQHandler(vinetic_interrupt_t* pIrq);
IFX_LOCAL IFX_void_t OS_IRQHandler_Dummy(int i);
/* Device Init. Called when the device has been opened for
   the first time. The device structure will be cached */
#if 0 /* Not used */
IFX_LOCAL IFX_int32_t OS_InitDevice (IFX_int32_t nDevn);
#endif /* Not used */

IFX_TAPI_LL_DEV_t* IFX_TAPI_LL_Prepare_Dev (TAPI_DEV *pTapiDev,
                                            IFX_uint32_t devNum);
IFX_void_t* IFX_TAPI_LL_Prepare_Ch (TAPI_CHANNEL *pTapiCh,
                                    IFX_uint32_t devNum,
                                    IFX_uint32_t chNum);
IFX_void_t Vinetic_IrqEnable (IFX_TAPI_LL_DEV_t *pLLDev);
IFX_void_t Vinetic_IrqDisable (IFX_TAPI_LL_DEV_t *pLLDev);




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
                             IFX_int32_t count, IFX_int32_t* ppos)
{
#if (VIN_CFG_FEATURES & VIN_FEAT_PACKET)
   VINETIC_CHANNEL *pCh = (VINETIC_CHANNEL*) pLLCh;
   IFX_int32_t     ret = IFX_SUCCESS;

   /* protect channel against concurrent tasks */
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
                             IFX_int32_t count, IFX_int32_t* ppos)
{
#if (VIN_CFG_FEATURES & VIN_FEAT_PACKET)
   VINETIC_CHANNEL *pCh = (VINETIC_CHANNEL*)pLLCh;
   IFX_int32_t     len;
#if 0
#if defined(EVALUATION) && defined(VIN_2CPE)
   /** Buffer size equals the maximum packet size possible including 64 bits of packet timestamp. */
   IFX_uint16_t pData[MAX_PACKET_WORD + sizeof(IFX_uint32_t) + 2] = {0};
#else
   IFX_uint16_t pData[MAX_PACKET_WORD] = {0};
#endif /* EVALUATION && VIN_2CPE */
#else
   IFX_uint16_t* pData = IFX_NULL;
#endif

   /* Protect channel against concurrent tasks. */
#if 1
   IFXOS_MutexLock(pCh->chAcc);
   Vinetic_IrqLockDevice(pCh->pParent);
   pData = (IFX_uint16_t *) bufferPoolGet(
                             (IFX_void_t *) TAPI_VoiceBufferPoolHandle_Get());
   memset(pData, 0, bufferPoolElementSize(TAPI_VoiceBufferPoolHandle_Get()));
   Vinetic_IrqUnlockDevice(pCh->pParent);
   IFXOS_MutexUnlock(pCh->chAcc);
#endif

   if (IFX_NULL == pData)
   {
      /* Error getting memory for packet data. */
      TRACE(VINETIC, DBG_LEVEL_HIGH, ("ERR: Reserving memory for packet "
            "data.\n\r"));
      return -EFAULT;
   }

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
   /* here the copy is only neccessary for linux. In VxWorks the user
      space could pass a buffer with two leading empty words */
   memcpy(&pData[2], (void *) buf, (IFX_uint32_t) count);

   /* protect channel against concurrent tasks */
   IFXOS_MutexLock (pCh->chAcc);
   /* count in words */
   len = pCh->if_write(pCh, (IFX_uint8_t *)pData, count);
   /* release lock */
   IFXOS_MutexUnlock (pCh->chAcc);

   /* Protect channel against concurrent tasks. */
#if 1
   IFXOS_MutexLock(pCh->chAcc);
   Vinetic_IrqLockDevice(pCh->pParent);
   bufferPoolPut(pData);
   Vinetic_IrqUnlockDevice(pCh->pParent);
   IFXOS_MutexUnlock(pCh->chAcc);
#endif

   return len;
#else
   return -EINVAL;
#endif /* (VIN_CFG_FEATURES & VIN_FEAT_PACKET) */
}


/****************************************************************************
Description:
    VINETIC device driver initialization.
    This is the device driver initialization function to call at the system
    startup prior any access to the VINETIC device driver.
    After the initialization the device driver is ready to be accessed by
    the application. The global structure "VIN_dev_ctx" contains all the data
    handling the interface (open, close, ioctl,...).

Arguments:
   None
Return Value:
   OK or ERROR
Remarks:
   None
****************************************************************************/
IFX_int32_t Vinetic_DeviceDriverInit(VOID)
{
   IFX_int32_t result = 0;


   result = VINETIC_DeviceDriverInit();

   /* very first call used for calibration  of the */
   /* self-calibrating hard delay routines library */
   /* "DelayLib.c"                 */
   /* For IDES3300 this calibration is done before, */
   /* so this leads only to an additonal affordable */
   /* wait of 1us */
   IFXOS_DELAYUS(1);

   return IFX_SUCCESS;
}



/****************************************************************************
Description:
    VINETIC device driver shutdown.
    This is the device driver shutdown function. This function is called by the
    system when the VINETIC device driver is no longer needed. Prior to shutdown
    the device driver all the device channels should be closed.
    This function releases all the resources granted by the device driver.

Arguments:
   None
Return Value:
   OK or ERROR
Remarks:
   None
****************************************************************************/
IFX_int32_t Vinetic_DeviceDriverStop(VOID)
{
   /*IFX_int32_t i = 0;*/
   IFX_TAPI_DRV_CTX_t DrvCtx;


   memset (&(DrvCtx), 0x00, sizeof (IFX_TAPI_DRV_CTX_t));

   DrvCtx.majorNumber = major;

   DrvCtx.minorBase = minorBase;

   DrvCtx.devNodeName = devName;
   DrvCtx.maxDevs = VINETIC_MAX_DEVICES;

   /*printk(KERN_INFO "vinetic_module_exit: Unregistering LL dev with major "
            "number %d and minor base %d\n",DrvCtx.majorNumber,
            DrvCtx.minorBase);*/

   /* notify HL Tapi when a LL dev unregisters */
   IFX_TAPI_Unregister_LL_Drv (major);

   TRACE(VINETIC, DBG_LEVEL_LOW, ("cleaning up %s module ...\n\r", DEV_NAME));

   return (OK);
}


/*******************************************************************************
Description:
   This function initializes the VINETIC base address.
   If necessary, OS specific mapping is done here.
Arguments:
    pDev    - handle of the VINETIC device structure.
   nBaseAddr - physical address for vinetic access.
Return:
   always IFX_SUCCESS
Remarks:
   none
*******************************************************************************/
IFX_int32_t OS_Init_Baseadress(VINETIC_DEVICE* pDev, IFX_uint32_t nBaseAddr)
{
   pDev->nBaseAddr = nBaseAddr;
   pDev->pBaseAddr = (IFX_uint16_t*) nBaseAddr;

   return IFX_SUCCESS;
}


/****************************************************************************
Description:
   VINETIC Interupt Service Routine entry-point.
Arguments:
   irq: number of the irq
   dev_id: pointer to the device structure
   pt_regs:
Remarks:
   This Function is called by the OS when a VINETIC interrupt occurs. This
   function calls the VINETIC OS-independent ISR routine which will serve
   interrupt events.
****************************************************************************/
IFX_LOCAL IFX_void_t OS_IRQHandler(vinetic_interrupt_t* pIrq)
{
   VINETIC_DEVICE* pDev = pIrq->pdev_head;

   while (pDev != IFX_NULL)
   {
      /* call the VINETIC Interrupt Service Routine */
      /* to serve the VINETIC interrupt events.     */
      VINETIC_interrupt_routine(pDev);
      pDev = pDev->pInt_NextDev;
   }
}

/*******************************************************************************
Description:
    Dummy Interrupt Service Routine in lieu of the VINETIC ISR.

Arguments:
   pDev  - handle of the device structure
Return:
   none
*******************************************************************************/
IFX_LOCAL IFX_void_t OS_IRQHandler_Dummy(int i)
{
   logMsg ("VINETIC_irq_handler_Dummy called for int %d !",i,0,0,0,0,0);
}


/*******************************************************************************
Description:
    Install the Interrupt Handler
Arguments:
   pDev  - pointer to device structure
   nIrq  - interrupt number
Return:
   none
*******************************************************************************/
IFX_void_t OS_Install_VineticIRQHandler(VINETIC_DEVICE* pDev, IFX_int32_t nIrq)
{
   vinetic_interrupt_t* pIrq;
   vinetic_interrupt_t* pIrqPrev = IFX_NULL;
   VINETIC_DEVICE* tmp_pDev;
   int intstatus;

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
      pIrq = malloc(sizeof(vinetic_interrupt_t));
      if (pIrq == IFX_NULL)
      {
         /** \todo error status !*/
         return;
      }
      memset(pIrq, 0, sizeof(vinetic_interrupt_t));
      IFXOS_MutexInit(pIrq->intAcc);
      /** \todo not working, because mutex must be given by same task */
      /*    IFXOS_MutexLock(pIrq->intAcc); */
      taskLock();
      /* register new dedicated or first of shared interrupt */
      pDev->pIrq = pIrq;
      pDev->pInt_NextDev = IFX_NULL;
      pIrq->pdev_head = pDev;
      pIrq->next_irq = IFX_NULL;
      pIrq->nIrq = nIrq;
      /* Install the Interrupt routine in the OS */
      if (VIN_SYS_REGISTER_INT_HANDLER(nIrq, OS_IRQHandler, pIrq) == ERROR)
      {
         TRACE (VINETIC, DBG_LEVEL_HIGH, ("ERROR: request interrupt failed\n\r"));
      }
      else
         pIrq->bRegistered = IFX_TRUE;

      /* add to list of interrupts */
      /* this list is not used by the interrupt handler itself,
         so no additional intLock necessary! */
      if (VIN_irq_head == IFX_NULL)
         VIN_irq_head = pIrq;
      else
      {
         /* pIrqPrev must be valid */
         IFXOS_ASSERT (pIrqPrev == IFX_NULL);
         pIrqPrev->next_irq = pIrq;
      }
      taskUnlock();
/*      pIrq->bIntEnabled = IFX_TRUE;
      VIN_ENABLE_IRQLINE (pIrq->nIrq);
*/
   }
   else
   {
      intstatus = intLock();
      /* add into the list of an existing shared interrupt */
      pDev->pIrq = pIrq;
      pDev->pInt_NextDev = IFX_NULL;
      tmp_pDev = pIrq->pdev_head;
      while (tmp_pDev->pInt_NextDev != IFX_NULL)
      {
         tmp_pDev = tmp_pDev->pInt_NextDev;
      }
      tmp_pDev->pInt_NextDev = pDev;
      intUnlock(intstatus);
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
IFX_void_t OS_UnInstall_VineticIRQHandler (VINETIC_DEVICE* pDev)
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
      /*IFXOS_MutexLock(pIrq->intAcc);*/

      if (pIrq->bIntEnabled == IFX_TRUE)
      {
         VIN_DISABLE_IRQLINE (pIrq->nIrq);
      }
      if (pIrq->bRegistered == IFX_TRUE)
      {
         /* unregister Interrupt routine */
         if (VIN_SYS_UNREGISTER_INT_HANDLER(pIrq->nIrq) == ERROR)
         {
            TRACE (VINETIC, DBG_LEVEL_HIGH, ("ERROR: request interrupt failed\n\r"));
         }
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
      free(pIrq);
   }
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
   (flag bIntEnabled is IFX_FALSE), the os disable function will
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
      routine (x);    <-----routineX unlocks interrupts. Be carefull!
      ... some more instructions;
      Vinetic_IrqUnlockDevice;
   }
****************************************************************************/
/*IFX_void_t Vinetic_IrqLockDevice (VINETIC_DEVICE *pDev)*/
IFX_void_t Vinetic_IrqLockDevice (IFX_TAPI_LL_DEV_t* pLLDev)
{
   VINETIC_DEVICE* pDev = (VINETIC_DEVICE *) pLLDev;
   vinetic_interrupt_t* pIrq = IFX_NULL;

   if (IFX_NULL == pDev)
   {
      TRACE(VINETIC, DBG_LEVEL_HIGH, ("Error retrieving vinetic device."));
      return;
   }

   pIrq = pDev->pIrq;

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
      if (intContext() != TRUE)
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
   Enable the global interrupt, if this was disabled by 'Vinetic_IrqLockDevice'
Arguments:
   pDev - vinetic device handle
Return Value:
   none
Remarks
   cf Remarks of Vinetic_IrqLockDevice () in this file.
****************************************************************************/
/*IFX_void_t Vinetic_IrqUnlockDevice(VINETIC_DEVICE *pDev)*/
IFX_void_t Vinetic_IrqUnlockDevice(IFX_TAPI_LL_DEV_t* pLLDev)
{
   VINETIC_DEVICE* pDev = (VINETIC_DEVICE *) pLLDev;
   vinetic_interrupt_t* pIrq = IFX_NULL;


   if (IFX_NULL == pDev)
   {
      TRACE(VINETIC, DBG_LEVEL_HIGH, ("Error retrieving vinetic device."));
      return;
   }

   pIrq = pDev->pIrq;

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

#if ((VIN_CFG_FEATURES & VIN_FEAT_GR909) && defined (VIN_V14_SUPPORT))
/****************************************************************************
Description:
   Gr909 task
Arguments:
   chanDev - handle to a channel device
Return Value:
   None.
****************************************************************************/
IFX_int32_t VINETIC_GR909_OSTask (IFX_uint32_t device)
{
   VINETIC_CHANNEL *pCh = (VINETIC_CHANNEL *)device;
   IFX_boolean_t bStop = IFX_FALSE;

   if (pCh->nChannel != 0)
   {
      for (;;)
      {
         IFXOS_WaitEvent (pCh->hostCh.Gr909.wqState);
         switch (pCh->hostCh.Gr909.nState)
         {
         case TEST_OK_STATE:
         case TEST_NOTOK_STATE:
         case INTERNAL_ERR_STATE:
            bStop = IFX_TRUE;
            break;
         default:
            break;
         }
         if (bStop == IFX_TRUE)
            break;
         if ((pCh->hostCh.Gr909.Usr.status == GR909_TEST_RUNNING) &&
             (pCh->hostCh.Gr909.Gr909Call != NULL))
         {
            pCh->hostCh.Gr909.Gr909Call (pCh);
         }
         if (pCh->hostCh.Gr909.Gr909Call == NULL)
            break;
      }
      pCh->hostCh.Gr909.Gr909Call (pCh);
      VINETIC_GR909_OSDeleteTask (pCh);
   }

   return IFX_SUCCESS;
}

/****************************************************************************
Description:
   Creates Gr909 task
Arguments:
   pCh - handle to VINETIC_CHANNEL structure
Return Value:
   IFX_SUCCESS / IFX_ERROR.
****************************************************************************/
IFX_int32_t VINETIC_GR909_OSCreateTask (VINETIC_CHANNEL *pCh)
{
   IFX_int32_t ret = IFX_SUCCESS;

   pCh->hostCh.Gr909.gr909tskId =
      taskSpawn("tskVin_gr909", 1, 0, 5000, (FUNCPTR)VINETIC_GR909_OSTask,
                 pCh, 0);

   if (pCh->hostCh.Gr909.gr909tskId == -1)
   {
      TRACE(VINETIC, DBG_LEVEL_HIGH, ("Gr909 task couldn't be started\n\r"));
      ret = IFX_ERROR;
   }
   else
   {
      /* set running state */
      pCh->hostCh.Gr909.Usr.status = GR909_TEST_RUNNING;
      /* activate the task */
      IFXOS_WakeUpEvent (pCh->hostCh.Gr909.wqState);
   }

   return ret;
}

/****************************************************************************
Description:
   Deletes gr909 task
Arguments:
   pCh - handle to VINETIC_CHANNEL structure
Return Value:
   IFX_SUCCESS / IFX_ERROR.
****************************************************************************/
IFX_int32_t VINETIC_GR909_OSDeleteTask (VINETIC_CHANNEL *pCh)
{
   IFX_int32_t ret;

   /* delete the task */
   ret = taskDelete(pCh->hostCh.Gr909.gr909tskId);
   if (ret == IFX_SUCCESS)
      pCh->hostCh.Gr909.gr909tskId = 0;

   return ret;
}

#endif /* ((VIN_CFG_FEATURES & VIN_FEAT_GR909) && (VIN_V14_SUPPORT)) */


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
IFX_TAPI_LL_DEV_t* IFX_TAPI_LL_Prepare_Dev(TAPI_DEV *pTapiDev,
                                           IFX_uint32_t devNum)
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
   /* nothing to be done for vxWorks */
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
   /* nothing to be done for vxWorks */
   return IFX_SUCCESS;
}


/****************************************************************************
Description:
   Get device pointer
Arguments:
   nr - number of device
   pDev - returned pointer
Return Value:
   IFX_ERROR if error, otherwise IFX_SUCCESS
Remarks:
   Used by board / main module to reset device states
****************************************************************************/
IFX_int32_t OS_GetDevice (IFX_int32_t nr, VINETIC_DEVICE** pDev)
{
   if (nr > VINETIC_MAX_DEVICES || sVinetic_Devices[nr] == NULL)
   {
      printf ("IFX_ERROR in OS_GetDevice: nr=%d, max=%d\n", nr, VINETIC_MAX_DEVICES);
      return IFX_ERROR;
   }

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
   Called form Board_DevOpen() to Used by board / main module to reset device states
****************************************************************************/
#if 0 /* Not used */
IFX_LOCAL IFX_int32_t OS_InitDevice (IFX_int32_t nDevn)
{
   /* get device pointer */
   VINETIC_DEVICE* pDev = (VINETIC_DEVICE*) &VDevices[nDevn];


   printf("OS_InitDevice\n");

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
#endif /* Not used */
