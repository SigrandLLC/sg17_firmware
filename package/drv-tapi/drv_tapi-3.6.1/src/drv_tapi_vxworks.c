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
/******************************************************************************
   Description : TAPI Driver, VxWorks part
******************************************************************************/

/* ============================= */
/* Includes                      */
/* ============================= */

#include "drv_api.h"
#include "drv_tapi_vxworks.h"

#include "drv_tapi.h"
#include "drv_tapi_api.h"
#include "drv_tapi_ll_interface.h"
#include "drv_tapi_event.h"
#include "drv_tapi_errno.h"
#include "drv_tapi_ioctl.h"

#ifdef TAPI_CID
#include "drv_tapi_cid.h"
#endif /* TAPI_CID */

#ifdef TAPI_POLL
#include "drv_tapi_polling.h"
#endif /* TAPI_POLL */

/* ============================= */
/* Defines                       */
/* ============================= */

/* VxWorks task priority of the task that executes the functions upon
   expiration of a timer. */
#define  TSK_PRIO_TIMER_HANDLER   20
/* VxWorks task priority of the task that processes all events */
#define  TSK_PRIO_EVENT_HANDLER   21

#ifdef VXWORKS

/*
    Global device driver structure.
    IOS Device driver Context.
*/
typedef struct
{
   /* Device Driver Number     */
   IFX_int32_t         nDrvNum;
   /* Major Driver Number     */
   IFX_int32_t         nMajorNum;
   /* Max device               */
   IFX_int32_t         nMaxDev;
   /* Max device channel       */
   IFX_int32_t         nMaxChn;
   /* Interface Semaphore      */
   IFXOS_mutex_t       oSemDrvIF;
   /* device header pointer    */
   Y_S_dev_header**     pDevHdr;
} Y_S_dev_ctx;

/* ============================= */
/* Local variable definition     */
/* ============================= */

/* The Global device driver structure */
Y_S_dev_ctx TAPI_devCtx;

/** install parameter debug_level: LOW (1), NORMAL (2), HIGH (3), OFF (4) */
#define DEBUG_TAPI

#ifdef DEBUG_TAPI
enum { debug_level = DBG_LEVEL_LOW };
#else /* DEBUG_TAPI */
enum { debug_level = DBG_LEVEL_NORMAL };
#endif /* DEBUG_TAPI */


/** Protection for array of timer structs. */
static IFXOS_mutex_t semTimerArrDataLock;


/* ============================= */
/* Global variable definition    */
/* ============================= */

/** Message queue ID, holding events to be handled. */
static MSG_Q_ID nMsgQueue_Events = 0;

/** Message queue ID, holding events to be handled. */
static IFX_int32_t nTaskForEvents_ID = -1;

/** Max. messages in queue. */
enum { MAX_MSG_CNT_EVENT_QUEUE = 32 };

/** Max. message size in queue. */
/*const IFX_int32_t MSG_SIZE_EVENT_QUEUE = sizeof(IFX_TAPI_EXT_EVENT_PARAM_t)
                                        + sizeof(IFX_TAPI_EVENT_t);*/

const IFX_int32_t MSG_SIZE_EVENT_QUEUE = sizeof(IFX_int32_t);


/* -------------------------------------------------------------------------- */

extern IFX_int32_t IFX_TAPI_Event_Dispatch_ProcessCtx(IFX_TAPI_EXT_EVENT_PARAM_t *pParam);

/* ============================= */
/* Local function declaration    */
/* ============================= */

/* Functions handling the select/unselect driver system io calls */
IFX_LOCAL IFX_int32_t OS_OnSelect(IFX_TAPI_ioctlCtx_t* pCtx,
                                  IFX_int32_t nArgument,
                                  IFX_int32_t nMaxCh);
IFX_LOCAL IFX_int32_t OS_OnUnselect(IFX_TAPI_ioctlCtx_t* pCtx,
                                    IFX_int32_t nArgument,
                                    IFX_int32_t nMaxCh);
IFX_LOCAL IFX_int32_t OS_DevNodeRegister (Y_S_dev_ctx* pDevCtx, int nMinor,
                                          IFX_char_t* buf);

IFX_LOCAL IFX_int32_t TAPI_DevCreate(Y_S_dev_ctx* pDevCtx,
                                     IFX_char_t* pzsDeviceName);
IFX_LOCAL IFX_int32_t TAPI_DevDelete(Y_S_dev_ctx* pDevCtx);

IFX_return_t IFX_TAPI_Is_Device_Registered(IFX_int32_t nMajor);

static IFX_return_t ifx_tapi_Event_StartMsgQueue(IFX_void_t);
static IFX_return_t ifx_tapi_Event_Send_Msg(IFX_void_t* pParam);

/* ============================= */
/* Local function definition     */
/* ============================= */

/* --------------------------------------------------------------------------
                          REGISTRATION mechanisms  -->  BEGIN
   --------------------------------------------------------------------------
*/

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
   /* The "TAPI_devCtx" global area contains all the         */
   /* data related to the VINETIC device driver installation */
   /* The area is used during all the life of the VINETIC    */
   /* device driver. The area is updated during the creation */
   /* and the access to the device driver.                   */
   /* Reset the memory area.                                 */
   memset(&TAPI_devCtx, 0x00, sizeof(Y_S_dev_ctx));

   /* Copy registration info from Low level driver. */
   TAPI_devCtx.nMaxDev = pLLDrvCtx->maxDevs;
   TAPI_devCtx.nMaxChn = pLLDrvCtx->maxChannels;
   TAPI_devCtx.nMajorNum = pLLDrvCtx->majorNumber;

   if (TAPI_DevCreate(&TAPI_devCtx, pLLDrvCtx->devNodeName) == IFX_ERROR)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("Err creating device. "
            "(File: %s, line: %d)\n", __FILE__, __LINE__));

      return IFX_ERROR;
   }

   IFXOS_MutexInit(TAPI_devCtx.oSemDrvIF);
   /* Initialize mutex for timer struct array access */
   IFXOS_MutexInit(semTimerArrDataLock);

   return IFX_SUCCESS;
} /* IFX_TAPI_Register_LL_Drv() */


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
   return IFX_SUCCESS;
}


/* --------------------------------------------------------------------------
                          REGISTRATION mechanisms  -->  END
   --------------------------------------------------------------------------
*/

/**
   Defer work to process context

   \param pFunc - pointer to function to be called (not needed in VxWorks)
   \param pParam - parameter passed to the function

   \return IFX_SUCCESS or IFX_ERROR in case of an error
*/
IFX_return_t TAPI_DeferWork(IFX_void_t* pFunc, IFX_void_t* pParam)
{
   IFX_return_t ret = IFX_SUCCESS;


   /* Notice: In VxWorks taskSpawn() is not working when we are in interrupt
      context, so message queue will be used. */

   if (pParam == IFX_NULL)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("Wrong input arguments."
            "(File: %s, line: %d)\n", __FILE__, __LINE__));
      return IFX_ERROR;
   }

   ret = ifx_tapi_Event_Send_Msg(pParam);

   return ret;
}


/* --------------------------------------------------------------------------
         DRIVER funcs (read/write/ioctl/release/open/poll)  -->  BEGIN
   --------------------------------------------------------------------------
*/


/**

   This function open a TAPI device previously registered to the OS during the
   device driver creation. For each Tapi channel, a device have been added in
   the device list with the device string "/dev/<device>DC":
      *<device> - vin, vmmc, ...
      - D: device number
      - C: channel number in the device

   The OS passes the Y_S_dev_header structure associated to the
   device "/dev/<device>DC":
      DevHdr: VxWorks specific header
      ddrvn : TAPI Device Driver Number (allocated by IOS)
      devn  : device number  (1 to TAPI_MAX_DEVICES)
      chnn  : channel number (0 to TAPI_MAX_CH_NR)
      pctx  : pointer to the channel context (will be allocated at the device open)

   The function will update the pctx field in the device header with the TAPI
   device channel (control/voice) selected structure.


   \param pDevHeader - device header pointer
   \param pAnnex - tail of device name / not used
   \param flags - open flags / not used

  \return pDevHeader - device header pointer
                <> IFX_NULL, the device is open without error
                == IFX_NULL, error
*/
IFX_int32_t ifx_tapi_open(Y_S_dev_header* pDevHeader,
                          INT8* pAnnex, IFX_int32_t flags)
{
   Y_S_dev_ctx* pDevCtx = IFX_NULL; /* device context pointer */
   TAPI_DEV* pTapiDev = IFX_NULL;
   IFX_TAPI_DRV_CTX_t* pDrvCtx = IFX_NULL;
   IFX_uint32_t devNum, chNum;


   if (pDevHeader == IFX_NULL)
   {
      /* The device driver is not installed */
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("pDevHeader is NULL. "
            "(File: %s, line: %d)\n", __FILE__, __LINE__));

      errno = ENODEV;
      return (IFX_int32_t) IFX_NULL;
   }

   /* Get the Global device driver context and check that the device driver
      is  already installed.  */
   pDevCtx = &TAPI_devCtx;

   /* Check device range. */
   if ((pDevHeader->nDevNum > pDevCtx->nMaxDev) || (0 > pDevHeader->nDevNum))
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("Max. device number exceed. "
            "(File: %s, line: %d)\n", __FILE__, __LINE__));
      goto OPEN_ERROR;
   }

   /* Get the index for the device driver context based on major number */
   pDrvCtx = IFX_TAPI_Get_Device_Driver_Context((int) pDevHeader->nMajorNum);
   if (pDrvCtx == IFX_NULL)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("Err getting pDrvCtx."
            "(File: %s, line: %d)\n", __FILE__, __LINE__));
      goto OPEN_ERROR;
   }

   /* protect against concurrent access */
   if (pDevCtx->oSemDrvIF != NULL)
   {
      IFXOS_MutexLock(pDevCtx->oSemDrvIF);
   }
   if (!(pDevHeader->nDevNum == 0 && pDevHeader->nChNum == 0))
   {
      /* channel or multi dev node */
      devNum = pDevHeader->nDevNum - 1;
      chNum  = pDevHeader->nChNum;
      pTapiDev = &(pDrvCtx->pTapiDev[pDevHeader->nDevNum - 1]);
   }
   else
   {
      pTapiDev = &(pDrvCtx->pTapiDev[0]);
   }

   if (pTapiDev == IFX_NULL)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_NORMAL, ("pTapiDev is NULL will save/create it.\n"));
      goto OPEN_ERROR;
   }

    /* Check if first open on this device. */
   if (pTapiDev->bInitialized == IFX_FALSE)
   {
      if (TAPI_Init_Dev (pDrvCtx, devNum) != TAPI_statusOk)
      {
         goto OPEN_ERROR;
      }
      /* Prepare message queue */
      ifx_tapi_Event_StartMsgQueue();
   }
   /* Increment the Usage counter */
   pTapiDev->nInUse++;
   pDevHeader->nInUse++;

   if (pDevHeader->nChNum == 0)
   {
      /* Save the device pointer */
      pDevHeader->pCtx = pTapiDev;
   }
   else if (pDevHeader->nChNum <= pDrvCtx->maxChannels)
   {
      /* Save the channel pointer */
      pDevHeader->pCtx = &(pTapiDev->pTapiChanelArray[pDevHeader->nChNum - 1]);
   }

   /* Call the Low level Device specific release routine */
   if (ptr_chk(pDrvCtx->Open, "pDrvCtx->Open"))
   {
      pDrvCtx->Open((IFX_int32_t)chNum, IFX_NULL);
   }

   /* increment module use counter */
#ifdef MODULE
   MOD_INC_USE_COUNT;
#endif

   /* release lock */
   if (pDevCtx->oSemDrvIF != NULL)
   {
      IFXOS_MutexUnlock(pDevCtx->oSemDrvIF);
   }

   return (IFX_int32_t) pDevHeader;

OPEN_ERROR:

   /* release lock */
   if (pDevCtx->oSemDrvIF != NULL)
   {
      IFXOS_MutexUnlock(pDevCtx->oSemDrvIF);
   }

   if (pDrvCtx != IFX_NULL && pDrvCtx->pTapiDev != IFX_NULL)
   {
      IFXOS_FREE(pDrvCtx->pTapiDev);
   }

   return -ENODEV;
} /* ifx_tapi_open() */


/**

   Configuration / Control for the device.

   \param pDevHeader - device header pointer
   \param nCmd - Configuration/Control command
   \param nArg - Configuration/Control arguments, optional

   \return IFX_SUCCESS    - no problems
           <> IFX_SUCCESS - Function is not implemented or other error

   \remark
   This function does the following functions:
      - If the ioctl command is device specific, low-level driver's ioctl function
      - If the ioctl command is TAPI specific, it is handled at this level
*/
STATUS ifx_tapi_ioctl(Y_S_dev_header* pDevHeader,
                      IFX_int32_t nCmd,
                      IFX_int32_t nArg)
{
   IFX_int32_t ret = IFX_SUCCESS;
   TAPI_DEV* pTapiDev = IFX_NULL;
   TAPI_CHANNEL* pDummyCh = IFX_NULL;
   IFX_TAPI_DRV_CTX_t* pDrvCtx = IFX_NULL;
   /*Y_S_dev_ctx* pDevCtx = NULL;*/ /* Global device context pointer */
   /*IFX_int32_t drvCtxIndex = -1;*/
   IFX_boolean_t bCtrlDev = IFX_TRUE;
   /* This tapi channel is valid and used for VxWorks onselect/onunselect*/
   TAPI_CHANNEL* pTapiCh = IFX_NULL;
   IFX_TAPI_ioctlCtx_t ctx;
   IFX_int32_t minor_num = 0;


   if (pDevHeader == IFX_NULL)
   {
      /* The device driver is not installed */
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("pDevHeader is NULL. "
            "(File: %s, line: %d)\n", __FILE__, __LINE__));
      errno = ENODEV;
      return IFX_ERROR;
   }

   pTapiDev = (TAPI_DEV *) pDevHeader->pCtx;

   /* initially we assume that we received a pTapiDev pointer -
      let's check if the initial assumption is not correct ... */
   if (pTapiDev->nChannel != IFX_TAPI_DEVICE_CH_NUMBER)
   {
      pDummyCh = (TAPI_CHANNEL *) pDevHeader->pCtx;
      pTapiDev = pDummyCh->pTapiDevice;
      if (pTapiDev == IFX_NULL)
      {
         printf("error IFX_NULL (2)\n\r");
         return IFX_ERROR;
      }
   }

   if (pDevHeader->pCtx == IFX_NULL)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("Device Driver not installed. "
            "(File: %s, line: %d)\n", __FILE__, __LINE__));
      /* The device driver is not installed */
      errno = ENODEV;
      return IFX_ERROR;
   }

   pDrvCtx = IFX_TAPI_Get_Device_Driver_Context(pDevHeader->nMajorNum);
   if (pDrvCtx == IFX_NULL)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("Err getting device driver."
            "(File: %s, line: %d)\n", __FILE__, __LINE__));
      errno = ENODEV;
      return IFX_ERROR;
   }

   minor_num = pDevHeader->nDevNum * 10 + pDevHeader->nChNum;
   IFX_TAPI_ioctlContextGet(pDrvCtx, minor_num, &ctx);

   /* In linux is like that, but actualy VxWorks does not have this command.
      Content of IOC_SIZE is used here */
   /*ctx.nParamSize = _IOC_SIZE(nCmd);*/
   ctx.nParamSize = (nCmd >> 16) & ((1 << 13) - 1);

   switch (IFXOX_IO_GETMAGIC (nCmd))
   {
      case VMMC_IOC_MAGIC:
      case DUS_IOC_MAGIC:
      case VINETIC_IOC_MAGIC:
      case SVIP_IOC_MAGIC:
         /* Device specific ioctl command */
         ret = TAPI_Dev_Spec_Ioctl(pDrvCtx, &ctx, nCmd, nArg);
         break;

      case IFX_TAPI_IOC_MAGIC:
         /* TAPI specific ioctl */
         ret = TAPI_Spec_Ioctl(pDrvCtx, &ctx, nCmd, nArg);
         if (ret != IFX_SUCCESS)
         {
            /* check if we are in device or channel context */
            if ((nCmd != IFX_TAPI_CAP_CHECK) &&
                (nCmd != IFX_TAPI_RING))
            {
               if (ctx.bDev == IFX_TRUE)
               {
                  ctx.p.pTapiDev->error.nCh = ctx.p.pTapiCh->nChannel;
                  ctx.p.pTapiDev->error.nCode = ret;
               }
               else
               {
                  /* we are in the channel context -> set the device context */
                  ctx.p.pTapiDev = ctx.p.pTapiCh->pTapiDevice;
                  ctx.p.pTapiDev->error.nCh = ctx.p.pTapiCh->nChannel;
                  ctx.p.pTapiDev->error.nCode = ret;
               }
               /* set errno = ret */
               ret = IFX_ERROR;
            }
         }
         break;
      default:
         switch (nCmd)
         {
         case FIOSELECT:
            ret = OS_OnSelect(&ctx, nArg, pDrvCtx->maxChannels);
            break;
         case FIOUNSELECT:
            ret = OS_OnUnselect(&ctx, nArg, pDrvCtx->maxChannels);
            break;
         default:
            pTapiDev = ctx.p.pTapiDev;
            RETURN_DEVSTATUS(TAPI_statusNoIoctl, 0);
         }
   } /* switch */

   if ((ret == -1) && (ctx.p.pTapiDev->error.nCode == 0))
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("Error code not set %d\n\r", nCmd));
   }
   return ret;
} /* ifx_tapi_ioctl() */


/**
    Release the device. The in-use counter will be decreased

   \param pDevHeader - device header pointer

   \return OK - no error
           -ENODEV - MAX_SOC_CHANNELS is exceeded
           ERROR on error

   \remark
      This function gets called when a close is called on the device.
      It decrements the usage count, free the FIFOs
*/
STATUS ifx_tapi_release(Y_S_dev_header *pDevHeader)
{
   /* Current Tapi device structure */
   TAPI_DEV* pTapiDev = IFX_NULL;
   /* Current Tapi device channel structure */
   TAPI_CHANNEL* pTapiCh  = IFX_NULL;
   /* device context pointer */
   Y_S_dev_ctx* pDevCtx = IFX_NULL;
   IFX_TAPI_DRV_CTX_t* pDrvCtx = IFX_NULL;
   IFX_TAPI_LL_CH_t* pCh = IFX_NULL;

   if (IFX_NULL == pDevHeader)
   {
      /* The device driver is not installed */
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("pDevHeader is NULL. "
            "(File: %s, line: %d)\n", __FILE__, __LINE__));
      errno = ENODEV;
      return (ERROR);
   }

   /* Get the Global device driver context */
   /* and check that the device driver is  */
   /* already installed.                   */
   /*pDevCtx = &TAPI_devCtx;*/  /* Get the device context */
   if (IFX_NULL == pDevCtx)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("Device driver not installed. "
            "(File: %s, line: %d)\n", __FILE__, __LINE__));
       return -ENODEV;
   }

   /* pDevHeader: device header updated at the device opening      */
   /*     - pctx: points to the TAPI device channel structure   */
   if (pDevHeader->pCtx == IFX_NULL)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("Data context (ch, dev) missing. "
            "(File: %s, line: %d)\n", __FILE__, __LINE__));

       return -ENODEV;
   }

   IFXOS_MutexLock(pDevCtx->oSemDrvIF);
   /* We first try to cast the context pointer to a device structure, just
      to test if it's a channel (TAPI_CHANNEL *) or a device (TAPI_DEVICE *)
      since nChannel is available in both structures this will work
      if pDevHeader->pctx is a channel, pCh is used instead of pDev. */
   pTapiDev = (TAPI_DEV *) pDevHeader->pCtx;
   pDevHeader->nInUse--;

   if (pDevHeader->nInUse == 0)
   {
      /* We erase the reference to the Tapi structure in
         header. */
      pDevHeader->pCtx = IFX_NULL;
   }

   /* Test the channel number in order to determine the
      Tapi device structure. */
   if (pTapiDev->nChannel != IFX_TAPI_DEVICE_CH_NUMBER)
   {
      pTapiCh = (TAPI_CHANNEL *) ((IFX_void_t *) pTapiDev);
      pTapiDev = (TAPI_DEV *) pTapiCh->pTapiDevice;
      pDrvCtx = (IFX_TAPI_DRV_CTX_t *) pTapiCh->pTapiDevice->pDevDrvCtx;
      pTapiCh->nInUse--;
      pTapiCh->pTapiDevice->nInUse--;
      pCh = pTapiCh->pLLChannel;

#if (VMMC_CFG_FEATURES & VMMC_FEAT_PACKET)
      /* Free Fifo memory. */
      if (0 == pTapiCh->nInUse)
      {
         /* Call the Low level function to free the FIFO */
         /** \todo Do not free FIFO otherwise application could not start
             for the second time. Problem, in ifx_tapi_open only first time
             everything is initialized. */
      }
#endif /* (VMMC_CFG_FEATURES & VMMC_FEAT_PACKET) */
      TRACE(TAPI_DRV, DBG_LEVEL_LOW,
            ("Closing device channel %d (%d)\n\r",
             pTapiCh->nChannel, pTapiCh->nInUse));
   }
   else
   {
      pDrvCtx = (IFX_TAPI_DRV_CTX_t*) pTapiDev->pDevDrvCtx;
      pTapiDev->nInUse--;
      /* Memory will be released when module is removed. */
   }

   TRACE(TAPI_DRV, DBG_LEVEL_LOW,
        ("closing device %d, Minor %d...\n\r",
         pDevHeader->nDevNum, pDevHeader->nDevNum));
   IFXOS_MutexUnlock(pDevCtx->oSemDrvIF);

   /* Call the Low leve Device specific release routine */
   /* winder: LL_DEV_Release equal ifx_mps_close. */
   if ((IFX_NULL != pDrvCtx->Release) && (pTapiDev->nInUse == 0))
   {
      pDrvCtx->Release((IFX_int32_t) (pTapiDev->nChannel),
                      IFX_NULL,
                      pTapiDev->pLLDev);
   }

   return (OK);
} /* ifx_tapi_release() */


/**

   Read data from the Tapi.

   \param pDevHeader - device header pointer
   \param pDest - data destination pointer
   \param nLength - data length to read

   \return len - data length
*/
IFX_int32_t ifx_tapi_read(Y_S_dev_header* pDevHeader,
                          IFX_uint8_t* pDest,
                          IFX_int32_t nLength)
{
   TAPI_CHANNEL* pTapiCh = IFX_NULL;
   IFX_TAPI_DRV_CTX_t* pDrvCtx = IFX_NULL;
   IFX_TAPI_LL_CH_t* pCh = IFX_NULL;
   ssize_t size = 0;
   TAPI_DEV* pTapiDev = IFX_NULL;
   IFX_int32_t ch_idx = 0;


   if (IFX_NULL == pDevHeader)
   {
      /* The device driver is not installed */
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("pDevHeader is NULL. "
            "(File: %s, line: %d)\n", __FILE__, __LINE__));

      errno = ENODEV;
      return 0;
   }

   pTapiCh = (TAPI_CHANNEL *) pDevHeader->pCtx;
   pTapiDev = pTapiCh->pTapiDevice;
   ch_idx = pTapiCh->nChannel;
   if (0 > ch_idx)
   {
      ch_idx = 0;
   }
   pTapiCh = &(pTapiDev->pTapiChanelArray[ch_idx]);

   pDrvCtx = (IFX_TAPI_DRV_CTX_t *) pTapiCh->pTapiDevice->pDevDrvCtx;
   pCh = (IFX_TAPI_LL_CH_t *) pTapiCh->pLLChannel;

   if (IFX_NULL == pDrvCtx->Read)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("Could not read data,"
            " pDevHeader is NULL."
            "(File: %s, line: %d)\n", __FILE__, __LINE__));

      return 0;
   }

   /* Call the Low level driver's read function */
   size = pDrvCtx->Read (pCh, pDest, nLength, (IFX_int32_t *) IFX_NULL);

   return size;
}

/**
   Writes data to the device.

   \param pDevHeader - device header pointer
   \param pSrc - data source pointer
   \param nLength - data length to write

   \return nLength - 0 if failure else the length
*/
IFX_int32_t ifx_tapi_write(Y_S_dev_header* pDevHeader,
                           IFX_uint8_t* pSrc,
                           IFX_int32_t nLength)
{
   TAPI_CHANNEL* pTapiCh = IFX_NULL;
   IFX_TAPI_DRV_CTX_t* pDrvCtx = IFX_NULL;
   IFX_TAPI_LL_CH_t* pCh = IFX_NULL;
   ssize_t size = 0;
   TAPI_DEV* pTapiDev = IFX_NULL;
   IFX_int32_t ch_idx = 0;


   if (IFX_NULL == pDevHeader)
   {
      /* The device driver is not installed */
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("pDevHeader is NULL. "
            "(File: %s, line: %d)\n", __FILE__, __LINE__));

      errno = ENODEV;
      return 0;
   }

   pTapiCh = (TAPI_CHANNEL *) pDevHeader->pCtx;
   pTapiDev = pTapiCh->pTapiDevice;
   ch_idx = pTapiCh->nChannel;
   if (0 > ch_idx)
   {
      ch_idx = 0;
   }
   pTapiCh = &(pTapiDev->pTapiChanelArray[ch_idx]);

   pDrvCtx = (IFX_TAPI_DRV_CTX_t *) pTapiCh->pTapiDevice->pDevDrvCtx;
   pCh = (IFX_TAPI_LL_CH_t *) pTapiCh->pLLChannel;

   /* Call the Low level driver's write function */
   if (IFX_NULL == pDrvCtx->Write)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("Could not write data"
            ", pDrvCtx->Write is NULL. "
            "(File: %s, line: %d)\n", __FILE__, __LINE__));

      return 0;
   }

   /* Call the Low level driver's read function */
   size = pDrvCtx->Write(pCh, pSrc, nLength, (IFX_int32_t *) IFX_NULL);

   return size;
}


/* --------------------------------------------------------------------------
             DRIVER funcs (read/write/ioctl/release/open)  -->  END
   --------------------------------------------------------------------------
*/


/* --------------------------------------------------------------------------
                          CREATE/DELETE driver  -->  BEGIN
   --------------------------------------------------------------------------
*/


/**

   TAPI device driver initialization.
   This is the device driver initialization function to call at the system
   startup prior any access to the TAPI device driver.
   After the initialization the device driver is ready to be accessed by
   the application. The global structure "TAPI_devCtx" contains all the data
   handling the interface (open, close, ioctl,...).

   \arguments None

   \return OK or ERROR
*/
IFX_int32_t Tapi_DeviceDriverInit(IFX_void_t)
{
   IFX_int32_t i = 0;

   printf("%s, (c) 2007 Infineon Technologies AG\n\r", &TAPI_WHATVERSION[4]);

   SetTraceLevel(TAPI_DRV, debug_level);
   SetLogLevel(TAPI_DRV, debug_level);


   /* Alloc the global driver context array. */
   for (i = 0; i < TAPI_MAX_LL_DEVICES; i++)
   {
      gHLDrvCtx[i].pDrvCtx = IFX_NULL;
   }

   IFX_TAPI_On_Driver_Load();

   /* Very first call used for calibration  of the self-calibrating hard
      delay routines library "DelayLib.c" For IDES3300 this calibration
      is done before, so this leads only to an additonal affordable
      wait of 1us. */
   IFXOS_DELAYUS(1);

   return IFX_SUCCESS;
} /* Tapi_DeviceDriverInit() */


/**

   TAPI device driver shutdown.
   This is the device driver shutdown function. This function is called by the
   system when the TAPI device driver is no longer needed. Prior to shutdown
   the device driver all the device channels should be closed.
   This function releases all the resources granted by the device driver.

   \param None

   \return OK or ERROR

   \remarks
*/
IFX_int32_t Tapi_DeviceDriverStop(VOID)
{
   IFX_int32_t i = 0;
   IFX_TAPI_DRV_CTX_t* pDrvCtx = IFX_NULL;
   TAPI_CHANNEL* pChannel = IFX_NULL;
   IFX_int32_t nCh = 0;


   printf("Removing Highlevel TAPI module\n");

   if (TAPI_DevDelete(&TAPI_devCtx) == IFX_ERROR)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("Err deleting device. "
            "(File: %s, line: %d)\n", __FILE__, __LINE__));

      return IFX_ERROR;
   }

   IFXOS_MutexDelete(TAPI_devCtx.oSemDrvIF);

   /* Delete mutex for timer struct array */
   IFXOS_MutexDelete(semTimerArrDataLock);

   /* Free the device data block */
   for (i = 0; i < TAPI_MAX_LL_DEVICES; i++)
   {
      pDrvCtx = gHLDrvCtx[i].pDrvCtx;

      if (pDrvCtx != IFX_NULL)
      {
         for (nCh = 0; nCh < pDrvCtx->maxChannels; nCh++)
         {
            pChannel = &(pDrvCtx->pTapiDev->pTapiChanelArray[nCh]);

            if (IFX_NULL != pChannel)
            {
               IFX_TAPI_EventDispatcher_Exit(pChannel);
            }
         }
         IFXOS_FREE(gHLDrvCtx[i].pDrvCtx);
         gHLDrvCtx[i].pDrvCtx = IFX_NULL;
      }
   }

   IFX_TAPI_On_Driver_Unload();

   TRACE(TAPI_DRV,DBG_LEVEL_NORMAL,("TAPI_DRV: cleanup successful\n\r"));

   return IFX_SUCCESS;
} /* Tapi_DeviceDriverStop() */


/**

   TAPI device driver creation.
   This function is called at the system startup (Tapi_DeviceDriverInit)
   in order to create and install the TAPI device driver in the target OS (VxWorks).

   Device driver Creation: the device driver is declared to the IOS system with
   "iosDrvInstall" OS function. We pass to the IOS the device driver interface
   functions (open, close, ioctl, ...), the IOS returns a Device Driver Number which
   will be used for adding the devices to the device driver.

   Device addition: Once the Device driver have been declared to the IOS system,
   we gone add the devices to the device driver list. Each device is identified by
   a unique string which will be used by the application for opening a device
   (the application will get in return a unique file descriptor (fd) allocated by
   the system and will be used for further access to the selected device).

   Device string selection: "/dev/vinDC" with,

        - "/dev/vin", this is a VINETIC device
        - "D",        designate the device number
                        "1",    VINETIC device 1
                        "2",    VINETIC device 2
                        ...
                        "VINETIC_MAX_DEVICES" VINETIC device VINETIC_MAX_DEVICES
        - "C",        designate a channel (C) in the designated device (D)
                        "0",    Control channel (device wise)
                        "1",    voice channel 1 (channel wise)
                        ...,
                        "VINETIC_MAX_CH_NR", voice channel VINETIC_MAX_CH_NR (channel wise)


   VINETIC_MAX_DEVICES, number maximum of VINETIC device
   VINETIC_MAX_CH_NR,   number maximum of voice channels

   The TAPI device driver is a multi-device driver, for each device, a device is
   accessed through channels. We distinghish two types of channels:
    - Control channel, used for control and configuration purpose. It's device wise and
    is always the channel number ZERO (0).
    - Voice channel, used to access a device voice channel. It's channel wise and the
    channel is <> to ZERO (0).

    Each device channel (Control/Voice) is represented by a device in the IOS system.
    The device is added to the device list for the VINETIC device driver, the number
    of devices per VINETIC device is:

          VINETIC_MAX_CH_NR (number of Voice channels) + 1 (Control Channel).


    The "Y_S_dev_header" structure is passed to the IOS sytem:
        DevHdr: VxWorks specific header
        ddrvn : VINETIC Device Driver Number (allocated by IOS)
        devn  : device number  (1 to VINETIC_MAX_DEVICES)
        chnn  : channel number (0 to VINETIC_MAX_CH_NR)
        pctx  : pointer to the channel context (will be allocated at the device open)


   \param pDevCtx - The Global device driver structure address

   \return OK or ERROR
*/
IFX_LOCAL IFX_int32_t TAPI_DevCreate(Y_S_dev_ctx* pDevCtx,
                                     IFX_char_t* pzsDeviceName)
{
   IFX_uint8_t    nDev, nCh, nMinor;
   IFX_char_t     buf[64];

   if ((IFX_NULL == pDevCtx) || (IFX_NULL == pzsDeviceName))
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("Wrong input arguments. "
            "(File: %s, line: %d)\n", __FILE__, __LINE__));
      return IFX_ERROR;
   }
   /* IOS device driver creation. We add the TAPI device driver to the
      IOS device list providing the device driver function interface.
      The IOS returns the Device Driver Number which will be used later on. */
   pDevCtx->nDrvNum = iosDrvInstall(IFX_NULL, IFX_NULL,
                                   (FUNCPTR) ifx_tapi_open,
                                   (FUNCPTR) ifx_tapi_release,
                                   (FUNCPTR) ifx_tapi_read,
                                   (FUNCPTR) ifx_tapi_write,
                                   (FUNCPTR) ifx_tapi_ioctl);

   if (ERROR == pDevCtx->nDrvNum)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("Unable to install the driver. "
            "(File: %s, line: %d)\n", __FILE__, __LINE__));
      return IFX_ERROR;
   }

   /* The device driver is declared to the IOS: Now we add to the device
      driver IOS list the devices for each TAPI device channel. */
   nMinor = 0;
#ifdef TAPI_ONE_DEVNODE
   sprintf(buf, "/dev/%s", pzsDeviceName);
   if (OS_DevNodeRegister (pDevCtx, nMinor, buf) == IFX_ERROR)
      return IFX_ERROR;
   pDevCtx->pDevHdr[nMinor]->nDevNum = nDev;
   pDevCtx->pDevHdr[nMinor]->nChNum = 0;
#endif /* TAPI_ONE_DEVNODE */
   for (nDev = 1; nDev < pDevCtx->nMaxDev + 1; nDev++)
   {
      /* Allocate memory for channels and control device. */
      for (nCh = 0; nCh < pDevCtx->nMaxChn + 1; nCh++)
      {
         sprintf(buf, "/dev/%s%d%d", pzsDeviceName, nDev, nCh);
         if (OS_DevNodeRegister (pDevCtx, nMinor, buf) == IFX_ERROR)
         {
            return IFX_ERROR;
         }
         pDevCtx->pDevHdr[nMinor]->nDevNum = nDev;
         pDevCtx->pDevHdr[nMinor]->nChNum = nCh;
         nMinor++;
      }
   }

   return IFX_SUCCESS;
} /* TAPI_DevCreate() */


IFX_LOCAL IFX_int32_t OS_DevNodeRegister (Y_S_dev_ctx* pDevCtx, int nMinor,
                                          IFX_char_t* buf)
{
   Y_S_dev_header* pDevHeader = IFX_NULL;


   /* Allocate memory for the device header. Reset the memory.
      Build the device string "/dev/vinij" and add the device
      in the IOS device list. Calculate the device channel minor (k)
      update the device context. */
   pDevHeader = (Y_S_dev_header*) IFXOS_MALLOC(sizeof(Y_S_dev_header));
   if (IFX_NULL == pDevHeader)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("Err get mem pDevHeader. "
            "(File: %s, line: %d)\n", __FILE__, __LINE__));
      return IFX_ERROR;
   }
   memset(pDevHeader, 0x00, sizeof(Y_S_dev_header));

   /* Add the device to the device list    */
   if (iosDevAdd(&pDevHeader->DevHdr, buf, pDevCtx->nDrvNum) == ERROR)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("Unable to add the device %s "
            "(File: %s, line: %d)\n", buf, __FILE__, __LINE__));
      IFXOS_FREE(pDevHeader);
      return (IFX_ERROR);
   }

   pDevHeader->nDrvNum = pDevCtx->nDrvNum;
   pDevHeader->nMajorNum = pDevCtx->nMajorNum;
   pDevHeader->nInUse = 0;
   pDevHeader->pCtx = IFX_NULL;
   pDevCtx->pDevHdr[nMinor] = pDevHeader;

   return IFX_SUCCESS;
}

/**

   TAPI device driver deletion.
   This function is called at the device driver shutdown
   (Tapi_DeviceDriverStop) in order to release and uninstall the OS-target
   resources (VxWorks) granted by the TAPI device driver.

   \param pDevCtx - The Global device driver structure address

   \return
   OK or ERROR
*/
IFX_LOCAL IFX_int32_t TAPI_DevDelete(Y_S_dev_ctx* pDevCtx)
{
   IFX_uint8_t nDev = 0;


   /* Check that we have a valid Global structure address      */
   /* and that a device is installed (OS-target point of view) */

   if ((pDevCtx == IFX_NULL) || (pDevCtx->nDrvNum == ERROR))
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("Wrong input arguments. "
            "(File: %s, line: %d)\n", __FILE__, __LINE__));

      return (ERROR);
   }

   for (nDev = 0; nDev < pDevCtx->nMaxDev; nDev++)
   {

      /* Delete the device from the IOS link list */
      /* free the device header                   */

      if (pDevCtx->pDevHdr[nDev])
      {
         iosDevDelete((DEV_HDR *)pDevCtx->pDevHdr[nDev]);
         IFXOS_FREE(pDevCtx->pDevHdr[nDev]);
      }
   }

   /* We remove the TAPI device driver from the IOS. */
   /* We free the Tapi device structures.            */

   iosDrvRemove(pDevCtx->nDrvNum, IFX_TRUE);

   /* Reset to 0 the Global device driver structure */
   /* in order to reset all the pointers to NULL    */
   /* marking that the device driver is no longer   */
   /* installed.                                    */

   memset(pDevCtx, 0x00, sizeof(Y_S_dev_ctx));
   return (OK);
} /* TAPI_DevDelete() */


/* --------------------------------------------------------------------------
                          CREATE/DELETE driver  -->  END
   --------------------------------------------------------------------------
*/


/* --------------------------------------------------------------------------
                             SELECT mechanism  -->  BEGIN
   --------------------------------------------------------------------------
*/

/**
   Executes the select for the channel fd.

   \param pTapiCh  - handle to channel control structure
   \param node - node list
   \param opt - optional argument, which contains needed information for
                IFXOS_SleepQueue

   \return System event qualifier. Either 0 or IFXOS_SYSREAD.
           IFX_ERROR fo error.

   \remarks
   This function needs operating system services, that are hidden by
   IFXOS macros.
*/
IFX_int32_t TAPI_SelectCh(TAPI_CHANNEL* pTapiCh,
                          IFX_int32_t node,
                          IFX_int32_t opt)
{
   IFX_TAPI_DRV_CTX_t* pDrvCtx = IFX_NULL;
   IFX_TAPI_LL_DEV_t* pLLDev = IFX_NULL;
   IFX_uint32_t  flags = 0;
   IFX_int32_t   ret = 0;
   IFX_TAPI_T38_STATUS_t TapiFaxStatus;


   if (IFX_NULL == pTapiCh)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("Wrong input arguments. "
            "(File: %s, line: %d)\n", __FILE__, __LINE__));
      return IFX_ERROR;
   }

   pDrvCtx = (IFX_TAPI_DRV_CTX_t*) pTapiCh->pTapiDevice->pDevDrvCtx;
   pLLDev = pTapiCh->pTapiDevice->pLLDev;

   /* Get the Status from the low level driver. */
   if (ptr_chk(pDrvCtx->COD.T38_Status_Get,
              "pDrvCtx->COD.T38_Status_Get"))
   {
      pDrvCtx->COD.T38_Status_Get(pTapiCh->pLLChannel, &TapiFaxStatus);
   }

   IFXOS_SleepQueue(pTapiCh->wqRead, node, opt);
#ifdef TAPI_FAX_T38
   IFXOS_SleepQueue(pTapiCh->wqWrite, node, opt);
   if ((TapiFaxStatus.nStatus & IFX_TAPI_FAX_T38_TX_ON)
      && (IFX_TRUE == pTapiCh->bFaxDataRequest))
   {
      /* Task should write a new packet now. */
      ret |= IFXOS_SYSWRITE;
   }
#endif /* TAPI_FAX_T38 */
   /* Select on a voice channel -- only implemented for TAPI. */
   flags |= CF_NEED_WAKEUP;
   /* Clear flags first, then apply new flags. */

   if ((IFX_NULL == pDrvCtx->IRQ.LockDevice)
       || (IFX_NULL == pDrvCtx->IRQ.UnlockDevice))
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("IRQ missing for locking/unlocking"
            " the device. (File: %s, line: %d)\n", __FILE__, __LINE__));
      return IFX_ERROR;
   }

   pDrvCtx->IRQ.LockDevice (pLLDev);

#ifdef TAPI_PACKET
   if (pDrvCtx->Read != IFX_NULL && !fifoEmpty(pTapiCh->pUpStreamFifo))
   {
      flags |= CF_WAKEUPSRC_STREAM;
      flags &= ~CF_NEED_WAKEUP;
      ret |= IFXOS_SYSREAD;
   }
#endif /* TAPI_PACKET */

   pTapiCh->nFlags &= ~(CF_WAKEUPSRC_GR909
                        | CF_WAKEUPSRC_STREAM
                        | CF_WAKEUPSRC_TAPI
                        | CF_NEED_WAKEUP);
   pTapiCh->nFlags |= flags;

   pDrvCtx->IRQ.UnlockDevice(pLLDev);

   return ret;
} /* TAPI_SelectCh() */


/**

   Handles the FIOSELECT system call.

   \param pTapiCh - handle of the channel structure
   \param nArgument - Ioctl argument
   \param bCtrlDev - TRUE: Control device / FALSE

   \return IFX_SUCCESS / IFX_ERROR

   \remark
   The set of wake up queues are different for the VINETIC configuration/control
   channel and for the voice channel.
*/
IFX_LOCAL IFX_int32_t OS_OnSelect(IFX_TAPI_ioctlCtx_t* pCtx,
                                  IFX_int32_t nArgument,
                                  IFX_int32_t nMaxCh)
{
   IFX_int32_t ret = 0;
   TAPI_DEV* pTapiDev = IFX_NULL;
   IFX_uint8_t  i, needWakeup = 0;
#ifdef EVALUATION
   IFX_int32_t ch = 0;
   VINETIC_EVAL* pEval = IFX_NULL;
#endif /* EVALUATION */


   if (pCtx == IFX_NULL)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("Wrong input arguments. "
            "(File: %s, line: %d)\n", __FILE__, __LINE__));
      return IFX_ERROR;
   }

   if (pCtx->bDev == IFX_TRUE)
   {
      /* handle device file descriptor */
      pTapiDev = pCtx->p.pTapiDev;

      /* add node to the wakup list */
      selNodeAdd(&(pTapiDev->wqEvent), (SEL_WAKEUP_NODE *) nArgument);
      pTapiDev->bNeedWakeup = IFX_TRUE;

      /* check if any data channels have events available */
      for (i=0; (i < pTapiDev->nMaxChannel) && (needWakeup == 0); i++)
      {
         needWakeup += !IFX_TAPI_EventFifoEmpty(pTapiDev->pTapiChanelArray + i);
      }

      if (needWakeup != 0)
      {
         /* events are pending - wake up the task */
         selWakeup((SEL_WAKEUP_NODE *) nArgument);
         pTapiDev->bNeedWakeup = IFX_FALSE;
      }

#ifdef EVALUATION
      {
         pEval = pDev->pEval;

         /* No LEC tests are supported. */
         /* Select on a configuration/control channel. */
         if (IFX_TRUE == pEval->bPollRead [POLLDEV_INTINFO])
         {
            pEval->bNeedWakeup = IFX_TRUE;
            selNodeAdd(&(pEval->PollQueue[POLLDEV_INTINFO]),
                       (SEL_WAKEUP_NODE *) nArgument);
            if (pEval->bPollEvt[POLLDEV_INTINFO] == IFX_TRUE)
            {
               pEval->bNeedWakeup = IFX_FALSE;
               selWakeup((SEL_WAKEUP_NODE *) nArgument);
            }
            /* Reset for futher use. */
            pEval->bPollEvt[POLLDEV_INTINFO] = IFX_FALSE;
         }

         for (ch = 0; ch < nMaxCh; ch ++)
         {
            if (IFX_TRUE ==
                ((VINCH_EVAL *) pTapiDev->pChannel[ch].pEval)->bDialDetection)
            {
               pEval->bNeedWakeup = IFX_TRUE;
               selNodeAdd(&(pEval->PollQueue[POLLDEV_INTINFO]),
                          (SEL_WAKEUP_NODE *) nArgument);
               if (IFX_TRUE ==
                   ((VINCH_EVAL *) pTapiDev->pChannel[ch].pEval)->bDialDetected)
               {
                  pEval->bNeedWakeup = IFX_FALSE;
                  selWakeup((SEL_WAKEUP_NODE *) nArgument);
               }
            }
         }
      }
#endif /* EVALUATION */
   }
   else
   {
      /* handle data file descriptors */

      ret = TAPI_SelectCh(pCtx->p.pTapiCh, nArgument, 0);

      if ((ret & IFXOS_SYSREAD) &&
          (selWakeupType((SEL_WAKEUP_NODE*) nArgument) == SELREAD))
      {
         selWakeup((SEL_WAKEUP_NODE*) nArgument);
      }

      if ((ret & IFXOS_SYSWRITE) &&
          (selWakeupType((SEL_WAKEUP_NODE*) nArgument) == SELWRITE))
      {
         selWakeup((SEL_WAKEUP_NODE*) nArgument);
      }

      /* Select() requires a return value of zero (OK, IFX_SUCCESS). */
   }

   return IFX_SUCCESS;
} /* OS_OnSelect() */


/**

   Handles the FIOUNSELECT system call.

   \pararm pTapiCh - handle of the channel structure
   \pararm nArgument - Ioctl argument
   \pararm bCtrlDev - TRUE: Control device / FALSE

   \return IFX_SUCCESS / IFX_ERROR

   \remarks
   The set of wake up queues are different for the VINETIC configuration/control
   channel and for the voice channel.
*/
IFX_LOCAL IFX_int32_t OS_OnUnselect(IFX_TAPI_ioctlCtx_t* pCtx,
                                    IFX_int32_t nArgument,
                                    IFX_int32_t nMaxCh)
{
   TAPI_DEV* pTapiDev = IFX_NULL;


#ifdef EVALUATION
   IFX_int32_t ch = 0;
   VINETIC_EVAL* pEval = IFX_NULL;
#endif /* EVALUATION */


   if (pCtx == IFX_NULL)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("Wrong input arguments. "
            "(File: %s, line: %d)\n", __FILE__, __LINE__));
      return IFX_ERROR;
   }

   if (pCtx->bDev == IFX_TRUE)
   {
      pTapiDev = pCtx->p.pTapiDev;

      selNodeDelete(&(pTapiDev->wqEvent), (SEL_WAKEUP_NODE *) nArgument);
#ifdef EVALUATION
      {
         pEval = pDev->pEval;

         /* unselect on a configuration/control  channel */
         if (IFX_TRUE == ((VINETIC_EVAL *) pTapiDev->pEval)->bPollRead[POLLDEV_INTINFO])
         {
            selNodeDelete(&(pEval->PollQueue[POLLDEV_INTINFO]),
                          (SEL_WAKEUP_NODE *) nArgument);
         }
         for (ch = 0; ch < nMaxCh; ch++)
         {
            if (IFX_TRUE == ((VINCH_EVAL *) pTapiDev->pChannel[ch].pEval)->bDialDetection)
            {
               selNodeDelete(&(pEval->PollQueue[POLLDEV_INTINFO]),
                             (SEL_WAKEUP_NODE *) nArgument);
            }
         }
      }
#endif /* #else #ifdef EVALUATION */
   }
   else
   {
      /* Voice Streaming */
      selNodeDelete(&(pCtx->p.pTapiCh->wqRead), (SEL_WAKEUP_NODE *) nArgument);
#ifdef EXCHFD_SUPPORT
      selNodeDelete(&(pCtx->p.pTapiCh->WakeupList), (SEL_WAKEUP_NODE *) nArgument);
#endif /* EXCHFD_SUPPORT */
#ifdef TAPI_FAX_T38
      selNodeDelete(&(pCtx->p.pTapiCh->wqWrite), (SEL_WAKEUP_NODE *) nArgument);
#endif /* (TAPI_FAX_T38) */
      /* clear */
      pCtx->p.pTapiCh->nFlags &= ~CF_NEED_WAKEUP;
   }

   return IFX_SUCCESS;
}


/* --------------------------------------------------------------------------
                             SELECT mechanism  -->  END
   --------------------------------------------------------------------------
*/


/* --------------------------------------------------------------------------
                             TIMER mechanisms  -->  BEGIN
   --------------------------------------------------------------------------
*/


/* New usage of timers. We have high priority task which waits for message
   pointer to timer struct or timer struct. When he gets it it starts func with
   parameters (both arguments located in timer struct).
   Timer are used as before (create it, start it, stop it, ..) BUT when timer
   elapses message with usefull data is send.
   When timer is created also array of timer struct is created.
 */


/** Message queue ID, holding funcs with arguments to be handled
    after timer has elapsed. */
static MSG_Q_ID nTimerMsgQueue = 0;

/** Task ID for high priority task handling timer messages */
static IFX_int32_t nTimerMsgHandler_ID = -1;

/** Max. messages in queue. */
enum { MSG_CNT_TIMER_QUEUE = 32 };

/** Message size in queue. */
const IFX_int32_t MSG_SIZE_TIMER_QUEUE = sizeof(IFX_int32_t);

/** Initial size of array in element count when created. */
static IFX_int32_t START_ELEM_CNT = 10;

/** Increase number of elements for array if full. */
enum { INCREASE_ELEM_CNT = 5 };

/** Holding timer information. When timer will elapse function
    with following prototype will be called : func(timer, arguments) */
typedef struct _TIMER_STRUCT_t
{
   /** Timer ID, needed when calling function */
   Timer_ID Timer;
   /** Arguments to func when timer will elapse. */
   IFX_int32_t nArg;
   /** Func called when timer will elapse. */
   TIMER_ENTRY pFunc;
   /** Flag if used by timer, IFX_TRUE used, IFX_FALSE not used */
/*   IFX_boolean_t fUsed;*/
} TIMER_STRUCT_t;

/** Array of timer struct. */
static TIMER_STRUCT_t* rgTimers = IFX_NULL;

/** Number of timer struct in array. */
static IFX_int32_t nTimersCnt = 0;

/** Number of used timer struct in array. */
static IFX_int32_t nUsedTimers = 0;



/**
   When timer elapses this function is called and will send message
   to message queue.

   \param Timer - Timer ID (timer structure)
   \param nArg  - array index of timers
 */
IFX_void_t TAPI_Timer_SendMsg(Timer_ID Timer, IFX_int32_t nArg)
{
   IFX_return_t ret = IFX_SUCCESS;


   /* Check if message queue exists */
   if (nTimerMsgQueue == IFX_NULL)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("Message queue is missing.\n"));
      return;
   }

   /* Just make copy of ptr and send it. */
   ret = msgQSend(nTimerMsgQueue, /* Message queue ID */
                  (IFX_char_t *) &nArg, /* Message, just pointer to it */
                   MSG_SIZE_TIMER_QUEUE, /* Message len */
                   NO_WAIT,
                   MSG_PRI_NORMAL);

   if (ret != IFX_SUCCESS)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
            ("TAPI_EVENT: Error sending messsage, errno %d.\n", errno));
      return;
   }
}


/**
   This function will read message queue and start func.

   \return
      On success IFX_SUCCESS or else IFX_ERROR

   \remarks
    Aftert timer has elapsed message was send with msgQSend()
    this handler is waiting for it and handle it.
 */
IFX_LOCAL IFX_void_t TAPI_HandleTimerMsg(IFX_void_t)
{
   IFX_int32_t ret = IFX_SUCCESS;
   TIMER_STRUCT_t* pParam = IFX_NULL;
   IFX_int32_t timer_idx = 0;


   /* Check if message queue exists */
   if (nTimerMsgQueue == IFX_NULL)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("Message queue is missing.\n"));
      return;
   }

   for(;;)
   {
      /* Wait for message, got number of bytes read or error (-1). */
      ret = msgQReceive(nTimerMsgQueue,
                        (IFX_char_t *) &timer_idx,
                        MSG_SIZE_TIMER_QUEUE,
                        WAIT_FOREVER);

      if ((ret == ERROR) || (ret < MSG_SIZE_TIMER_QUEUE))
      {
         TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("Error receiving message %d.\n", ret));
      }
      else
      {
         if ((0 > timer_idx) || (nTimersCnt < timer_idx))
         {
            TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("Timer message with wrong value received %d\n", timer_idx));
            return;
         }

         /* Call function to handle this event. This one will also
            free buffer, put them back to bufferpool. */
         pParam = &rgTimers[timer_idx - 1];

         pParam->pFunc(pParam->Timer, (IFX_int32_t) pParam->nArg);
      }
   }
}


/**
   Initialize mesage queue and starts message queue handler.

   \param none

   \return IFX_SUCESS on ok, otherwise IFX_ERROR.
*/
IFX_return_t TAPI_Timer_StartMsgQueue(IFX_void_t)
{
   IFX_return_t ret = IFX_SUCCESS;


   TRACE(TAPI_DRV, DBG_LEVEL_NORMAL, ("Create message queue.\n\r"));

   /* Create message queue for dispatching events */
   nTimerMsgQueue = msgQCreate(MSG_CNT_TIMER_QUEUE,
                               MSG_SIZE_TIMER_QUEUE,
                               0 /* MSG_Q_FIFO */);

   if (nTimerMsgQueue == IFX_NULL)
   {
      /* Error creating mesage queue. */
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("Error creating message queue.\n\r"));

      ret = IFX_ERROR;
   }

   if ((ret != IFX_ERROR) && (nTimerMsgHandler_ID == -1))
   {
      TRACE(TAPI_DRV, DBG_LEVEL_NORMAL,
            ("Start task which will handle timer messages.\n"));

      /* Create task which will read events from message queue and
         call func to handle them */
      nTimerMsgHandler_ID = taskSpawn("tTimerMsgHandler",
                                      TSK_PRIO_TIMER_HANDLER,
                                      0, /* Options */
                                      8192, /* Stack size */
                                      (FUNCPTR) TAPI_HandleTimerMsg,
                                      /* 10 arguments */
                                      0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

      if (nTimerMsgHandler_ID == IFX_ERROR)
      {
         TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("Error creating task.\n"));

         ret = IFX_ERROR;
      }
   }

   return ret;
}


/**
   Search for free timer struct in array.

   \param nTimerIdx - array index of free element in array

   \return pointer to timer structure, otherwise IFX_NULL.
 */
TIMER_STRUCT_t* TAPI_Timer_GetFreeElem(IFX_int32_t* nTimerIdx)
{
   TIMER_STRUCT_t* free_struct = IFX_NULL;
   IFX_int32_t i = 0;
   IFX_void_t* increased_part = IFX_NULL;


   if (nTimerIdx == IFX_NULL)
   {
      /* Wrong input arguments */
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
           ("Invalid input argument(s). (File: %s, line: %d)\n",
            __FILE__, __LINE__));
      return IFX_NULL;
   }

   if (rgTimers == IFX_NULL)
   {
      nTimersCnt = START_ELEM_CNT;

      TRACE(TAPI_DRV, DBG_LEVEL_NORMAL,
            ("Timer array is created with num %d of timers\n",
            (int) nTimersCnt));

      /* Allocate memory for timers. */
      /*          dev * ch * max_timers elements, SIZE - size of struct */
      rgTimers = calloc(nTimersCnt, sizeof(TIMER_STRUCT_t));
      if (rgTimers == IFX_NULL)
      {
         TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
               ("Error creating array of timer structs."
                "(File: %s, line: %d)\n", __FILE__, __LINE__));
         return IFX_NULL;
      }
   }

   free_struct = IFX_NULL;

   /* Search for free struct */
   for (i = 0; i < nTimersCnt; i++)
   {
      if (rgTimers[i].pFunc == IFX_NULL)
      {
         /* Got free struct */
         free_struct = &rgTimers[i];
         *nTimerIdx = i + 1;
         break;
      }
   }

   if (free_struct == IFX_NULL)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_NORMAL,
            ("Array is full, increase its size for %d timers\n",
             INCREASE_ELEM_CNT));
      /* Array of timer struct is full, try to increase it */
      increased_part = realloc(rgTimers, (nTimersCnt + INCREASE_ELEM_CNT)
                                          * sizeof(TIMER_STRUCT_t));
      if (increased_part == IFX_NULL)
      {
         TRACE(TAPI_DRV, DBG_LEVEL_NORMAL,
               ("Error increasing array of timer structs."
                " (File: %s, line: %d)\n", __FILE__, __LINE__));
         return IFX_NULL;
      }
      else
      {
         /* \todo should we free old structure */
         /*if (IFX_NULL != rgTimers)
         {
            free(rgTimers);
            rgTimers = IFX_NULL;
         }*/
         rgTimers = increased_part;
      }

      /* Set to zero increased memory. */
      memset(&rgTimers[nTimersCnt], 0,
             INCREASE_ELEM_CNT * sizeof(TIMER_STRUCT_t));

      /* Set free_struct to first timer in mem which was increased */
      free_struct = &rgTimers[nTimersCnt];
      *nTimerIdx = nTimersCnt + 1;

      /* Increase count of timers */
      nTimersCnt += INCREASE_ELEM_CNT;
   }

   /* Also increase number of used timer structs. */
   nUsedTimers++;

   return free_struct;
}


/**
   Function create a timer.

   \param pTimerEntry - Function pointer to the call back function
   \pararm nArgument - Argument including the Dev structure
                       (as integer pointer)
   \return Timer - Timer ID handle for vxWorks
*/
Timer_ID TAPI_Create_Timer(TIMER_ENTRY pTimerEntry, IFX_int32_t nArgument)
{
   /* Use common timer functions if sys_timerlib_vxworks.c is included
      (done in BSPProj.c) */
   TIMER_STRUCT_t* free_struct = IFX_NULL;
   IFX_int32_t timer_idx = 0;


   IFXOS_MutexLock(semTimerArrDataLock);

   if (nTimerMsgQueue == 0)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_NORMAL, ("Create timer msg queue and start handler\n"));
      TAPI_Timer_StartMsgQueue();
   }

   free_struct = TAPI_Timer_GetFreeElem(&timer_idx);
   if (free_struct == IFX_NULL)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("Err getting free timer struct elem"
            " from array. (File: %s, line: %d)\n", __FILE__, __LINE__));

      IFXOS_MutexUnlock(semTimerArrDataLock);

      return (0);
   }

   IFXOS_MutexUnlock(semTimerArrDataLock);

   free_struct->nArg = nArgument;
   free_struct->pFunc = pTimerEntry;

#ifndef SYS_TIMERLIB_VXWORKS_H

   /* Derive timer from CLK realtimer, do not use signal handler. */
   if (timer_create(CLOCK_REALTIME, NULL, &free_struct->Timer) == IFX_ERROR)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("Err creating timer. "
            "(File: %s, line: %d)\n", __FILE__, __LINE__));

      /* Set also that this timer struct is free, because could not create timer. */
      free_struct->pFunc = IFX_NULL;

      return (0);
   }

   /* Connect timer to function, which will send message with arguments. */
   if (timer_connect(free_struct->Timer, TAPI_Timer_SendMsg,
                     (IFX_int32_t) timer_idx) == IFX_ERROR)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("Err connecting to timer. "
            "(File: %s, line: %d)\n", __FILE__, __LINE__));

      /* Set also that this timer struct is free, because could not create timer. */
      free_struct->pFunc = IFX_NULL;

      return (0);
   }


   return (free_struct->Timer);
#else
   return (InstallTimer(TAPI_Timer_SendMsg, 0, timer_idx, IFX_FALSE));
#endif
}


/**

   Function set and starts a timer with a specific time. It can be choose if the
   timer starts periodically.

   \param Timer - Timer ID handle for vxWorks
   \pararm nTime - Time in ms
   \param bPeriodically - Starts the timer periodically or not
   \param bRestart - Restart the timer or normal start

   \return Returns an error code: IFX_TRUE / IFX_FALSE
*/
IFX_boolean_t TAPI_SetTime_Timer(Timer_ID Timer,
                                 IFX_uint32_t nTime,
                                 IFX_boolean_t bPeriodically,
                                 IFX_boolean_t bRestart)
{
   /* Use common timer functions if sys_timerlib_vxworks.c is included (done in BSPProj.c). */
#ifndef SYS_TIMERLIB_VXWORKS_H
   struct itimerspec   timeToSet;        /* time to be set */
   struct timespec     timeValue;        /* timer expiration value */
   struct timespec     timeInterval;     /* timer period */


   /* Stop timer. */
   if (bRestart == IFX_TRUE)
   {
      if (timer_cancel(Timer) == IFX_ERROR)
      {
         TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("Err cancelling timer. "
               "(File: %s, line: %d)\n", __FILE__, __LINE__));

         return (IFX_FALSE);
      }
   }

   /* Initialize timer expiration value. */
   timeValue.tv_sec = (nTime / 1000);
   timeValue.tv_nsec = (nTime % 1000) * 1000 * 1000;

   /* Initialize timer period */
   if (bPeriodically == IFX_TRUE)
   {
      timeInterval.tv_sec = (nTime / 1000);
      timeInterval.tv_nsec = (nTime % 1000) * 1000 * 1000;
   }
   else
   {
      timeInterval.tv_sec = 0;
      timeInterval.tv_nsec = 0;
   }

   /* Reset timer structure. */
   memset((IFX_char_t *) &timeToSet, 0, sizeof (struct itimerspec));

   /* Set the time to be set value. */
   /* NOTICE: Copy all parameter in simple steps. This is a workaround to avoid
      crashes on the CheckBoard, because of incompatiple compiler versions. */
   timeToSet.it_value.tv_sec = timeValue.tv_sec;
   timeToSet.it_value.tv_nsec = timeValue.tv_nsec;
   timeToSet.it_interval.tv_sec = timeInterval.tv_sec;
   timeToSet.it_interval.tv_nsec = timeInterval.tv_nsec;

   /* Pass timer value & reload value. */
   /* NOTICE: Do not use 'TIMER_ABSTIME' flag because timer will then expire
      immediatly. */
   if (timer_settime(Timer, 0, &timeToSet, IFX_NULL) == IFX_ERROR)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("Err setting time for timer. "
            "(File: %s, line: %d)\n", __FILE__, __LINE__));

      return (IFX_FALSE);
   }

   return (IFX_TRUE);
#else
   return ( SetTimeTimer(Timer, nTime, bPeriodically, bRestart) );
#endif
}


/**

   Function stop a timer.

   \param Timer - Timer ID handle for vxWorks

   \return Returns an error code: IFX_TRUE / IFX_FALSE
*/
IFX_boolean_t TAPI_Stop_Timer(Timer_ID Timer)
{
#ifndef SYS_TIMERLIB_VXWORKS_H
   /* Stop timer. */
   if (timer_cancel(Timer) == IFX_ERROR)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("Err cancelling a timer. "
            "(File: %s, line: %d)\n", __FILE__, __LINE__));

      return (IFX_FALSE);
   }

   return (IFX_TRUE);
#else
   return ( StopTimer(Timer) );
#endif
}


/**
   Search for used struct and free it (its not used by this timer anymore).

   \param Timer - Pointer to timer struct
*/
IFX_void_t TAPI_Timer_RemoveStruct(Timer_ID* Timer)
{
   IFX_int32_t i = 0;


   if (Timer == IFX_NULL)
   {
      /* Wrong input arguments */
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
           ("Invalid input argument(s). (File: %s, line: %d)\n",
            __FILE__, __LINE__));
      return;
   }

   IFXOS_MutexLock(semTimerArrDataLock);

   /* Search for free struct */
   for (i = 0; i < nTimersCnt; i++)
   {
      if (Timer == &rgTimers[i].Timer)
      {
         /* Set to unused */
         memset(&rgTimers[i], 0, sizeof(TIMER_STRUCT_t));

         /* Decrease number of used timer structs. */
         nUsedTimers--;

         if (nUsedTimers == 0)
         {
            TRACE(TAPI_DRV, DBG_LEVEL_NORMAL, ("Remove array with timer structs\n"));
            /* Release array of timer structs, because its empty. */
            free(rgTimers);
            rgTimers = IFX_NULL;
         }

         break;
      }
   }

   IFXOS_MutexUnlock(semTimerArrDataLock);
}


/**

   Function delete a timer.

   \param Timer - Timer ID handle for vxWorks

   \return Returns an error code: IFX_TRUE / IFX_FALSE
*/
IFX_boolean_t TAPI_Delete_Timer(Timer_ID Timer)
{
#ifndef SYS_TIMERLIB_VXWORKS_H
   /* Stop timer. */
   if (timer_cancel(Timer) == IFX_ERROR)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("Err cancelling a timer. "
            "(File: %s, line: %d)\n", __FILE__, __LINE__));
      return (IFX_FALSE);
   }

   /* Delete timer. */
   if (timer_delete(Timer) == IFX_ERROR)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("Err deleting timer. "
            "(File: %s, line: %d)\n", __FILE__, __LINE__));
      return (IFX_FALSE);
   }

   TAPI_Timer_RemoveStruct(&Timer);

   return (IFX_TRUE);
#else
   TAPI_Timer_RemoveStruct(&Timer);

   return ( DeleteTimer(Timer) );
#endif
}


/* --------------------------------------------------------------------------
                             TIMER mechanisms  -->  END
   --------------------------------------------------------------------------
*/


/* ============================= */
/* Global function definition    */
/* ============================= */


/* --------------------------------------------------------------------------
                             EVENT handling  -->  BEGIN
   --------------------------------------------------------------------------
*/


/**
   This function will read message queue for events.

   \remarks
   When message is send with msgQSend() this handler is waiting for it
   and dispatch it to event handler.
*/
static IFX_void_t IFX_TAPI_Event_Dispatch_Queue(IFX_void_t)
{
   IFX_return_t ret = IFX_SUCCESS;
   IFX_TAPI_EXT_EVENT_PARAM_t* pParam = IFX_NULL;


   TRACE(TAPI_DRV, DBG_LEVEL_NORMAL, ("IFX_TAPI_Event_Dispatch_Queue().\n"));

   /* Check if message queue exists */
   if (nMsgQueue_Events == IFX_NULL)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("Message queue is missing.\n"));
      return;
   }

   for (;;)
   {
      /* Wait for message */
      ret = msgQReceive(nMsgQueue_Events,
                        (IFX_char_t *) &pParam,
                        MSG_SIZE_EVENT_QUEUE,
                        WAIT_FOREVER);

      if ((ret == IFX_ERROR) || (ret < MSG_SIZE_EVENT_QUEUE))
      {
         TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
               ("Error receiving message %d.\n", ret));
      }
      else
      {
         if (IFX_TAPI_Event_Dispatch_ProcessCtx(pParam) != IFX_SUCCESS)
         {
            TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("Error when processing event.\n"));
         }
      }
   }
}


/**
   Initialize mesage queue and starts message queue handler.

   \param none

   \return IFX_SUCESS on ok, otherwise IFX_ERROR.
*/
IFX_return_t ifx_tapi_Event_StartMsgQueue(IFX_void_t)
{
   IFX_return_t ret = IFX_SUCCESS;


   TRACE(TAPI_DRV, DBG_LEVEL_NORMAL, ("Create message queue.\n\r"));

   /* Create message queue for dispatching events */
   nMsgQueue_Events = msgQCreate(MAX_MSG_CNT_EVENT_QUEUE,
                                 MSG_SIZE_EVENT_QUEUE,
                                 0 /* MSG_Q_FIFO */);

   if (IFX_NULL == nMsgQueue_Events)
   {
      /* Error creating mesage queue. */
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("Error creating message queue.\n\r"));
      ret = IFX_ERROR;
   }

   if ((ret != IFX_ERROR) && (nTaskForEvents_ID == -1))
   {
      TRACE(TAPI_DRV, DBG_LEVEL_NORMAL,
            ("Start task for handling msg queue with events.\n"));

      /* Create task which will read events from message queue and
         calls functions to handle them before putting it into the
         fifo towards the application */
      nTaskForEvents_ID = taskSpawn("tEventHandler",
                                    TSK_PRIO_EVENT_HANDLER,
                                    0, /* Options */
                                    8192, /* Stack size */
                                    (FUNCPTR) IFX_TAPI_Event_Dispatch_Queue,
                                    /* 10 arguments */
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

      if (nTaskForEvents_ID == IFX_ERROR)
      {
         TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("Error creating task.\n"));
         ret = IFX_ERROR;
      }
   }

   return ret;
}


#if 0 /* For future use */
/**
   Deletes mesage queue and stops message queue handler.

   \pararm none

   \return IFX_SUCCESS on ok, otherwise IFX_ERROR.
*/
IFX_int32_t IFX_TAPI_Event_StopMsgQueue(IFX_void_t)
{
   IFX_int32_t ret = IFX_SUCCESS;

   /* Delete task */
   if (taskDelete(nTaskForEvents_ID) != IFX_SUCCESS)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("Error deleting task.\n"));
      ret = IFX_ERROR;
   }
   /* set to inital state even if kill failed */
   nTaskForEvents_ID = -1;

   /* Delete message queue (try even if task delete failed) */
   if (msgQDelete(nMsgQueue_Events) != IFX_SUCCESS)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("Error deleting message queue.\n"));
      ret = IFX_ERROR;
   }
   /* set to inital state even if delete failed */
   nMsgQueue_Events = 0;

   return ret;
}
#endif /* if 0 */


/**
   This function sends message to task which will handle it.

   \param pParam - holding detected event and some additional info.

   \return IFX_SUCCESS if Message is send and everything is ok, otherwise
           IFX_ERROR.
*/
IFX_return_t ifx_tapi_Event_Send_Msg(IFX_void_t* pParam)
{
   IFX_return_t ret = IFX_SUCCESS;
   IFX_TAPI_EXT_EVENT_PARAM_t* tmp_param = IFX_NULL;


   tmp_param = (IFX_TAPI_EXT_EVENT_PARAM_t *) pParam;

   TRACE(TAPI_DRV, DBG_LEVEL_NORMAL, ("ifx_tapi_Event_Send_Msg()\n"));

   if (nMsgQueue_Events == IFX_NULL)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
            ("No message queue, so won't send message.\n"));

      return IFX_ERROR;
   }

   /* Will also make a copy so afterthat release back to bufferpool. */
   /* Just make copy of ptr and send it. */
   ret = msgQSend(nMsgQueue_Events, /* Message queue ID */
                  (IFX_char_t *) &tmp_param, /* Message */
                  MSG_SIZE_EVENT_QUEUE, /* Message len */
                  NO_WAIT,
                  MSG_PRI_NORMAL);

   if (ret == IFX_ERROR)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
            ("TAPI_EVENT: Error sending messsage, errno %d.\n", errno));
   }

   return ret;
}

#endif /* VXWORKS */
