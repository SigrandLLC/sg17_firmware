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

*******************************************************************************
   Module      : drv_vinetic_init.c
   Description :
******************************************************************************/

#include "drv_vinetic_init.h"
#include "drv_vinetic_stream.h"
#include "drv_vinetic_main.h"
#include "drv_vinetic_basic.h"
#include "drv_vinetic_sig_cid.h"


/* ============================= */
/* Local Macros & Definitions    */
/* ============================= */


/* ============================= */
/* Global variable definition    */
/* ============================= */

/* Define trace output for vinetic
   OFF:    no output
   HIGH:   only important traces, as errors or some warnings
   NORMAL: including traces from high and general proceedings and
           possible problems
   LOW:    all traces and low level traces as basic chip access
           and interrupts, command data
   Traces can be completely switched off with the compiler switch
   ENABLE_TRACE to 0
*/
CREATE_TRACE_GROUP(VINETIC);
CREATE_LOG_GROUP(VINETIC);
#ifdef RUNTIME_TRACE
CREATE_TRACE_GROUP(VINETIC_RUNTIME_TRACE);
#endif /* RUNTIME_TRACE */

/** what string support, driver version string */
const IFX_char_t DRV_VINETIC_WHATVERSION[] = DRV_VINETIC_WHAT_STR;
#ifdef HAVE_CONFIG_H
/** which access mode is supported */
const IFX_char_t DRV_VINETIC_WHICHACCESS[] = VIN_ACCESS_MODE_STR;
/** which configure options were set */
const IFX_char_t DRV_VINETIC_WHICHCONFIG[] = VIN_CONFIGURE_STR;
#endif /* HAVE_CONFIG_H */


extern IFX_uint16_t major;
extern IFX_uint16_t minorBase;
extern IFX_char_t *devName;
extern VINETIC_DEVICE *sVinetic_Devices[VINETIC_MAX_DEVICES];
#ifdef VIN_SPI
#ifdef VIN_USE_SPI_CS_CALLBACK
extern IFX_int32_t v2cpe_spi_cs_handler (IFX_int32_t on, IFX_uint32_t cs_data);
#endif /* VIN_USE_SPI_CS_CALLBACK */
#endif /* VIN_SPI */

IFX_void_t Vinetic_IrqEnable(IFX_TAPI_LL_DEV_t *pLLDev);
IFX_void_t Vinetic_IrqDisable(IFX_TAPI_LL_DEV_t *pLLDev);
extern IFX_TAPI_LL_DEV_t *IFX_TAPI_LL_Prepare_Dev(TAPI_DEV *pTapiDev,
                                                  IFX_uint32_t devNum);
extern IFX_void_t *IFX_TAPI_LL_Prepare_Ch(TAPI_CHANNEL *pTapiCh,
                                          IFX_uint32_t devNum,
                                          IFX_uint32_t chNum);
extern IFX_int32_t IFX_TAPI_LL_Open(IFX_int32_t inode, IFX_void_t *file_p);
extern IFX_int32_t IFX_TAPI_LL_Close(IFX_int32_t inode,
                                     IFX_void_t *filp,
                                     IFX_TAPI_LL_DEV_t *pLLDev);
extern IFX_int32_t VINETIC_LL_Write(IFX_TAPI_LL_CH_t *pCh,
                                    const IFX_char_t *buf,
                                    IFX_int32_t count,
                                    IFX_int32_t *ppos);
extern IFX_int32_t VINETIC_LL_Read(IFX_TAPI_LL_CH_t *pCh,
                                   IFX_char_t *buf,
                                   IFX_int32_t count,
                                   IFX_int32_t *ppos);
IFX_void_t VINETIC_ALM_CPE_Func_Register(IFX_TAPI_DRV_CTX_ALM_t *pAlm);

#ifdef VXWORKS
IFX_void_t VINETIC_POLL_Func_Register(IFX_TAPI_DRV_CTX_POLL_t *pPoll);
#endif /* VXWORKS */

static IFX_int32_t VINETIC_Basic_VoIPConf (VINETIC_CHANNEL *pCh);
#if 0
static IFX_int32_t VINETIC_Basic_PcmConf  (VINETIC_CHANNEL *pCh, IFX_uint8_t nPcmType);
#endif


/* ============================= */
/* Local function declaration    */
/* ============================= */
/**
  Reset all driver state information
\param
   pDev   - handle to the device
\remarks
   Called upon reset of the device
*/
IFX_LOCAL IFX_void_t ResetDevState(VINETIC_DEVICE *pDev)
{
   IFX_int32_t      i;

   /* reset the states variable */
   pDev->nDevState = 0;
   /* reset error */
   pDev->err = VIN_ERR_OK;
#ifdef TAPI_CID
   /* reset FW caches */
   for (i = 0; i < pDev->caps.nSIG; ++i)
   {
      VINETIC_SIG_CID_Sender_MSG_Reset (&pDev->pChannel[i]);
   }
#endif
#if (VIN_CFG_FEATURES & (VIN_FEAT_VIN_C | VIN_FEAT_VIN_M | VIN_FEAT_VIN_2CPE))
   /* reset the version register to empty values */
   pDev->nEdspVers[0] = NOTVALID;
   for (i = 1; i < CMD_VERS_FEATURES_EXT_LEN; i++)
   {
      pDev->nEdspVers[i] = 0;
   }
#endif /* (VIN_CFG_FEATURES & (VIN_FEAT_VIN_C | VIN_FEAT_VIN_M)) */

   for (i = 0; i < VINETIC_MAX_ANA_CH_NR; i ++)
   {
      IFX_TAPI_ResetChState(pDev->pChannel[i].pTapiCh);
   }

   /* reset TS management flags */
   memset(pDev->PcmRxTs, 0, sizeof(pDev->PcmRxTs));
   memset(pDev->PcmTxTs, 0, sizeof(pDev->PcmTxTs));
#ifdef TESTING
   /* last command and short command */
   memset(pDev->lastCmd, 0xFF, sizeof(pDev->lastCmd));
#endif /* TESTING */

   VINETIC_Host_ResetDevMembers (pDev);
}


/**
   OS independent initializations, called on first device open
\param
   pCh       - pointer to VINETIC channel structure
\return
   IFX_SUCCESS if ok, otherwise IFX_ERROR
*/
IFX_int32_t initChMember(VINETIC_CHANNEL *pCh)
{
   IFX_int32_t ret;

#ifdef DEBUG
   pCh->magic = VCH_MAGIC;
#endif /* DEBUG */

   /* init channel lock */
   IFXOS_MutexInit (pCh->chAcc);
   /* select wait queue for reading data */
   /* IFXOS_Init_WakeList (pCh->SelRead);*/
   IFXOS_Init_WakeList (pCh->pTapiCh->wqRead);
   /* init Vinetic CID structure */
   memset (&pCh->cidSend, 0, sizeof (VINETIC_CID));
   /* init cid receiver temporary buffer */
   memset (&pCh->cidRxPacket, 0, sizeof (PACKET));
#if (VIN_CFG_FEATURES & VIN_FEAT_PACKET)
   {
#if 0
      /* Moved to high level tapi */
      /* init Channel Fifo */
      pCh->pFifo = NULL;
#endif
      /* init queues*/
      /* IFXOS_InitEvent (pCh->wqRead);*/
      IFXOS_InitEvent (pCh->pTapiCh->semReadBlock);
      /* IFXOS_Init_WakeList (pCh->wqWrite);*/
      IFXOS_Init_WakeList (pCh->pTapiCh->wqWrite);
      /* init read/write ptr */
      pCh->if_read  = VoIP_UpStream;
      pCh->if_write = VoIP_DownStream;
   }
#endif /* (VIN_CFG_FEATURES & VIN_FEAT_PACKET) */
#if (VIN_CFG_FEATURES & VIN_FEAT_VOICE)
   {
      pCh->bVoiceConnect = IFX_FALSE;
   }
#endif /* (VIN_CFG_FEATURES & VIN_FEAT_VOICE) */
#if (VIN_CFG_FEATURES & VIN_FEAT_FAX_T38)
   {
      /* Initialization time : No Fax data request. */
      pCh->pTapiCh->bFaxDataRequest = IFX_FALSE;

   }
#endif /* (VIN_CFG_FEATURES & VIN_FEAT_FAX_T38) */

   /* host channel member initialization */
   ret = VINETIC_Host_InitChMembers  (pCh);

   return ret;
}



/**
   Initalize the device capability structure
\param
   pDev   - handle to the device structure
\return
   none - function always succeeds
*/
void VINETIC_Set_DevCaps(VINETIC_DEVICE *pDev)
{
   IFX_int32_t ret;

   /* Try to read the firmware capabilities with a message */
   ret = VINETIC_Get_FwCap (pDev);
   if (ret != IFX_SUCCESS)
   {
      /* reading fw capabilities failed - set defaults and extend them with
         values determined by chip type and read from the version register. */
      TRACE (VINETIC, DBG_LEVEL_LOW,
             ("INFO: Filling capabilities from chip version register\n\r"));
      pDev->caps.nCOD      = 4;
      pDev->caps.nSIG      = 4;
      pDev->caps.nPCM      = 8;
      pDev->caps.nUTG      = 2;
      pDev->caps.nNLEC     = 4;
      pDev->caps.nWLEC     = 4;
      pDev->caps.nUtgPerCh = 1;
      pDev->caps.nCPTD     = 4;

      /* call device dependent init function, to overide above default values */
       VINETIC_Host_SetDevCaps(pDev);
   }

   /* Find out which protocol is supported by this firmware */
   pDev->caps.bProtocolRTP =
      ((pDev->nEdspVers[0] & ECMD_VERS_EDSP_PRT) == ECMD_VERS_EDSP_PRT_RTP) ? 1 : 0;
   pDev->caps.bProtocolAAL =
      ((pDev->nEdspVers[0] & ECMD_VERS_EDSP_PRT) == ECMD_VERS_EDSP_PRT_AAL) ? 1 : 0;

   TRACE (VINETIC, DBG_LEVEL_LOW,
          ("FWCAP: nPCM:%2u  nALM:%2u  nSIG:%2u  nCOD:%2u\n\r",
           pDev->caps.nPCM, pDev->caps.nALI,
           pDev->caps.nSIG, pDev->caps.nCOD));
   TRACE (VINETIC, DBG_LEVEL_LOW,
          ("FWCAP: nNLEC:%2u nWLEC:%2u nAGC:%2u  nFAX:%2u\n\r",
           pDev->caps.nNLEC, pDev->caps.nWLEC,
           pDev->caps.nAGC, pDev->caps.nFAX));
   TRACE (VINETIC, DBG_LEVEL_LOW,
          ("FWCAP: nUTG:%2u  UTG/CH:%2u nMFTD:%2u\n\r",
           pDev->caps.nUTG, pDev->caps.nUtgPerCh,
           pDev->caps.nMFTD));
}



/**
   Initalize the shadow device Firmware modules (PCM/SIGNALLING/ALM/CODER)
\param
   pDev   - handle to the device structure
\return
   IFX_SUCCESS or IFX_ERROR, if no memory is available
*/
IFX_int32_t VINETIC_InitDevFWData(VINETIC_DEVICE *pDev)
{
   IFX_uint8_t i;
   IFX_int32_t ret = IFX_SUCCESS;

   TRACE(VINETIC, DBG_LEVEL_LOW, ("INFO: VINETIC_InitDevFWData called\n\r"));

   /* Mark the available LEC resource in the firmware as free.
      Each bit set represents one available LEC resource */
   pDev->availLecRes = (1 << pDev->caps.nNLEC) - 1;

   /* CON module */
   for (i=0; i < 8; ++i)
   {
      if (ret == IFX_SUCCESS)
      {
         ret = VINETIC_CON_Allocate_Ch_Structures (&pDev->pChannel[i]);
      }
      if (ret != IFX_SUCCESS)
      {
         SET_DEV_ERROR (VIN_ERR_NO_MEM);
      }
   }

   /* ALM module */
   for (i = 0; (ret == IFX_SUCCESS) && (i < pDev->caps.nALI); ++i)
   {
      VINETIC_CHANNEL *pCh = &pDev->pChannel[i];

      ret = VINETIC_ALM_Allocate_Ch_Structures (pCh);
      if (ret != IFX_SUCCESS)
      {
         SET_DEV_ERROR (VIN_ERR_NO_MEM);
      }
      else
      {
         VINETIC_ALM_Init_Ch (pCh);
      }
   }

   /* COD module */
   for (i = 0; (ret == IFX_SUCCESS) && (i < pDev->caps.nCOD); ++i)
   {
      VINETIC_CHANNEL *pCh = &pDev->pChannel[i];

      ret = VINETIC_COD_Allocate_Ch_Structures (pCh);
      if (ret != IFX_SUCCESS)
      {
         SET_DEV_ERROR (VIN_ERR_NO_MEM);
      }
      else
      {
         VINETIC_COD_Init_Ch (pCh);
      }
   }

   /* Set the PCM channel numbering in the channels */
   switch (pDev->nChipMajorRev)
   {
#ifdef VIN_V14_SUPPORT
      case VINETIC_V1x:
         {
            IFX_uint8_t j;
            /* Define PCM channel resources:
               As the 16bit linear mode needs 2 channel resources on
               Vinetic V1.x.
               The first half of the PCM channels we use is every
               second one (0,2,4,6,...).
               The second half of the used PCM channels are the one
               in between (1,3,5,7,...).
               The first half can be used for 8-Bit mode and 16-Bit mode.
               The upper half can only be used if the whole device is
               running in 8-Bit mode.
               The capability keeping all available PCM resources.
            */
            for (i = 0, j = 0; i < (pDev->caps.nPCM / 2); i++, j += 2)
            {
               pDev->pChannel[i].nPcmCh = (IFX_uint8_t)j;
               pDev->pChannel[i].nPcmMaxResolution = (IFX_uint8_t)16;
            }
            for (j = 1; i < pDev->caps.nPCM; i++, j += 2)
            {
               pDev->pChannel[i].nPcmCh = (IFX_uint8_t)j;
               pDev->pChannel[i].nPcmMaxResolution = (IFX_uint8_t)8;
            }
         }
         break;
#endif /* VIN_V14_SUPPORT */
#if (defined VIN_V21_SUPPORT) || (defined VIN_2CPE)
      case VINETIC_V2x:
         for (i = 0; i < pDev->caps.nPCM; i++)
         {
            pDev->pChannel[i].nPcmCh = (IFX_uint8_t)i;
            pDev->pChannel[i].nPcmMaxResolution = (IFX_uint8_t)16;
         }
         break;
#endif /* VIN_V21_SUPPORT */
      default:
         SET_DEV_ERROR (VIN_ERR_NO_VERSION);
         return IFX_ERROR;
         break;
   }

   /* PCM module */
   for (i = 0; (ret == IFX_SUCCESS) && (i < pDev->caps.nPCM); ++i)
   {
      VINETIC_CHANNEL *pCh = &pDev->pChannel[i];

      ret = VINETIC_PCM_Allocate_Ch_Structures (pCh);
      if (ret != IFX_SUCCESS)
      {
         SET_DEV_ERROR (VIN_ERR_NO_MEM);
      }
      else
      {
         VINETIC_PCM_Init_Ch (pCh, pCh->nPcmCh);
      }
   }

   /* SIG module */
   for (i = 0; (ret == IFX_SUCCESS) && (i < pDev->caps.nSIG); ++i)
   {
      VINETIC_CHANNEL *pCh = &pDev->pChannel[i];

      ret = VINETIC_SIG_Allocate_Ch_Structures (pCh);
      if (ret != IFX_SUCCESS)
      {
         SET_DEV_ERROR (VIN_ERR_NO_MEM);
      }
      else
      {
         VINETIC_SIG_Init_Ch (pCh);
      }
   }

   return ret;
}

/**
   OS independent initializations, called only on first device open
\param
   pDev      - pointer to VINETIC device structure
\return
   IFX_SUCCESS if ok, otherwise IFX_ERROR
\remarks
   Called in open when the device memory was allocated. This happens only
   once, cause the device structure is cached.
   Another open would not lead to this function cause the memory is already
   available.
*/
IFX_int32_t VINETIC_InitDevMember(VINETIC_DEVICE *pDev)
{
   IFX_int32_t ret;

#ifdef DEBUG
   pDev->magic = VDEV_MAGIC;
#endif /* DEBUG */
   /* initialize mailbox protection semaphore */
   IFXOS_MutexInit (pDev->mbxAcc);
   /* initialize share variables protection semaphore */
   IFXOS_MutexInit(pDev->memberAcc);

   /* set polling mode */
   pDev->IrqPollMode = VIN_VOICE_POLL | VIN_EVENT_POLL;
   /* mark for reading */
   pDev->caps.nALI   = 0;
#ifdef ADDED_SUPPORT
   /* Allocate and Initialize the Device Interrupt
      Error Status FIFO */
   if (InitErrFifo (pDev) == IFX_ERROR)
        return IFX_ERROR;
#endif /* ADDED_SUPPORT */
#ifdef TESTING
   pDev->lastCmd[0] = 0;
   pDev->lastCmd[1] = 0;
#endif /* TESTING */
#ifdef EVALUATION
      if (Eval_Init(pDev) == IFX_ERROR)
         return IFX_ERROR;
#endif /* EVALUATION */
   ret = VINETIC_Host_InitDevMembers (pDev);

#ifdef VIN_MB_ACC_TRACE
   /* Initialize the mailbox access trace buffer inside the driver */
   /* As default use 'VINETIC_MB_ACC_DEFAULT_ENTRIES' mailbox accesses for the
      size of the trace buffer.*/
   {
      VINETIC_MB_ACC_TRACE *pTrace;
      IFX_uint32_t len;

      len = sizeof(VINETIC_MB_ACC_TRACE) * VINETIC_MB_ACC_DEFAULT_ENTRIES;
      pTrace = malloc(len);
      if (pTrace == NULL)
         return IFX_ERROR;

      /* reset the memory to zero */
      memset(pTrace, 0, len);

      pDev->pMbAccTrStart = pTrace;
      pDev->pMbAccTrWrite = pTrace;
      pDev->pMbAccTrEnd = pTrace + VINETIC_MB_ACC_DEFAULT_ENTRIES;
   }
#endif /* VIN_MB_ACC_TRACE */
   return ret;
}


/**
   Basic initializations of the device.
\param
   pDev              - pointer to the device structure
\param
   pBasicDeviceInit  - pointer to the device init parameters
\return
   IFX_SUCCESS if ok, otherwise IFX_ERROR
\remarks
   Must be the first action after installing the driver and
   before any access to the hardware (corresponding vinetic chip or channel).
*/
IFX_int32_t VINETIC_BasicDeviceInit(VINETIC_DEVICE *pDev,
                                    VINETIC_BasicDeviceInit_t *pBasicDeviceInit)
{
   IFX_int32_t err = IFX_SUCCESS;

   /* reset this device */
   ResetDevState(pDev);
   /* initialize base address in case the access isn't via SPI/SCI */
   if ((pBasicDeviceInit->AccessMode != VIN_ACCESS_SCI) &&
       (pBasicDeviceInit->AccessMode != VIN_ACCESS_SPI))
   {
      err = OS_Init_Baseadress(pDev, pBasicDeviceInit->nBaseAddress);
   }
#ifdef VIN_2CPE
   else
   {
#ifdef VIN_SPI
      if (pBasicDeviceInit->AccessMode == VIN_ACCESS_SPI)
      {
         pDev->hostDev.nSPIDevAddr = (pBasicDeviceInit->nBaseAddress &
                                      V2CPE_SPI_DEVADDR_MASK);
      }
      else
#endif /* VIN_SPI */
      {
         SET_DEV_ERROR(VIN_ERR_FUNC_PARM);
         err = IFX_ERROR;
      }
   }
#endif /* VIN_2CPE */
   /* set access methods (functions, pointers)*/
   if (err == IFX_SUCCESS)
   {
      err = VINETIC_Host_SetupAccess (pDev, pBasicDeviceInit->AccessMode);
   }
   if (err==IFX_SUCCESS)
   {
      if (pBasicDeviceInit->nIrqNum < 0)
      {
         pDev->IrqPollMode = VIN_VOICE_POLL | VIN_EVENT_POLL;
      }
      else
      {
         OS_Install_VineticIRQHandler(pDev, pBasicDeviceInit->nIrqNum);

         if (pDev->pIrq == IFX_NULL)
            pDev->IrqPollMode = VIN_VOICE_POLL | VIN_EVENT_POLL;
         else
            pDev->IrqPollMode = VIN_TAPI_WAKEUP;
      }
   }

#ifdef SPI_INIT
      /* macro defined in drv_config_user.h to configure / initialize GPIO
         pins handled by the BSP, this macro must be executed before the
         first chip access is done */
      SPI_INIT(pDev);
#endif

   if (err == IFX_SUCCESS)
   {
#ifdef VIN_2CPE
      IFX_uint16_t nRev = 0;
#endif /* VIN_2CPE */

      pDev->nDevState |= DS_BASIC_INIT;
#ifdef VIN_2CPE

      REG_READ_PROT (pDev, V2CPE_DUPO_REVISION, &nRev);
      if (pDev->err == VIN_ERR_OK)
      {
         pDev->nChipRev = V2CPE_DUPO_REVISION_REV_GET (nRev);
         if ( (pDev->nChipRev == VINETIC_2CPE_V21) ||
              (pDev->nChipRev == VINETIC_2CPE_V22) ||
              (pDev->nChipRev == VINETIC_2CPE_AMR) )
         {
            pDev->caps.nALI     = V2CPE_DUPO_REVISION_CHAN_GET (nRev);
            pDev->nChipType     = V2CPE_DUPO_REVISION_TYPE_GET (nRev);
            pDev->nChipMajorRev = VINETIC_V2x;
         }
      }
      VINETIC_AddCaps (pDev);
#endif /* VIN_2CPE */
   }

   return err;
}



/**
   Low Level initialization function. This function is called from the
   TAPI init function.
\param pChannel Handle to TAPI_CHANNEL structure
\param pInit    Initializations flags
\return Return value according to IFX_return_t
      - IFX_ERROR if an error occured
      - IFX_SUCCESS if successful
\remarks
   Performs all neccesary actions to provide a fully operating system including
   firmware download and initialization. Althoug this function is called for
   each channel the device initialization must also be made sure within this
   function.
   Depending on the underlying device the default mode is mapped to the best
   possible initialization covering all features of the device.
   For VINETIC-S the default mode is IFX_TAPI_INIT_MODE_PCM_PHONE,
   for VINETIC-C the default mode is IFX_TAPI_INIT_MODE_PCM_DSP,
   for VINETIC-VIP and M the default mode is IFX_TAPI_INIT_MODE_VOICE_CODER. */
IFX_return_t TAPI_LL_Phone_Init(TAPI_CHANNEL *pChannel, IFX_TAPI_CH_INIT_t const *pInit)
{
   VINETIC_DEVICE *pDev = (VINETIC_DEVICE*) (pChannel->pTapiDevice->pLLDev);
   IFX_uint8_t nCh = pChannel->nChannel;

   VINETIC_CHANNEL *pCh = &pDev->pChannel[nCh];
   IFX_int32_t ret = IFX_SUCCESS;
   IFX_uint8_t mode = IFX_TAPI_INIT_MODE_NONE;
#if (VIN_CFG_FEATURES & VIN_FEAT_VOICE)
   IFX_TAPI_PKT_RTP_CFG_t rtpConf;
   IFX_TAPI_JB_CFG_t jbConf;
#endif /* (VIN_CFG_FEATURES & VIN_FEAT_VOICE) */
   VINETIC_IO_INIT IoInit;

   /* check if device initialization was done already   */
   if (!(pDev->nDevState & DS_DEV_INIT))
   {
      /* initialize chip one time */
      if (pInit->pProc == NULL)
      {
         /* reset all init pointers and init flag */
         memset (&IoInit, 0, sizeof (VINETIC_IO_INIT));
         /* use country info here if needed */
         /* set additional default flags */
         IoInit.nFlags = FW_AUTODWLD;
      }
      /* do initialization as specified in init.
         Country isn't taken in count then. */
      else
      {
         IFXOS_CPY_USR2KERN(&IoInit, pInit->pProc, sizeof (VINETIC_IO_INIT));
      }

      /* Initialize Vinetic */
      ret = VINETIC_Host_InitChip (pDev, &IoInit);
   }

   if (!(pDev->nDevState & DS_DEV_INIT))
   {
      return IFX_ERROR;
   }

   /* check if tapi was previously initialized */
   else if (!(pDev->nDevState & DS_TAPI_INIT))
   {
      /* set Tapi max Channel */
      ret = VINETIC_AddCaps(pDev);
   }
   if (ret == IFX_SUCCESS)
   {
      /* set tapi init flag to prevent non tapi driver functions */
      pDev->nDevState |= DS_TAPI_INIT;
   }
   if (pInit->nMode != IFX_TAPI_INIT_MODE_NONE)
   {
      switch (pDev->nChipType)
      {
         case VINETIC_TYPE_C:
            /* error checking for C. No coder available and default must be PCM */
            switch (pInit->nMode)
            {
               case IFX_TAPI_INIT_MODE_VOICE_CODER:
                  SET_ERROR (VIN_ERR_FUNC_PARM);
                  break;
               case IFX_TAPI_INIT_MODE_DEFAULT:
                  mode = IFX_TAPI_INIT_MODE_PCM_DSP;
                  break;
               default:
                  mode = pInit->nMode;
                  break;
            }
            break;
         case VINETIC_TYPE_S:
            switch (pInit->nMode)
            {
               /* error checking for S. No DSP and coder available, only one
                  default possible */
               case IFX_TAPI_INIT_MODE_PCM_DSP:
               case IFX_TAPI_INIT_MODE_VOICE_CODER:
                  SET_ERROR (VIN_ERR_FUNC_PARM);
                  break;
               default:
                  mode = IFX_TAPI_INIT_MODE_PCM_PHONE;
                  break;
            }
            break;
         default:
            mode = pInit->nMode;
            break;
      }
   }


   /* Do appropriate initialization for channel */
   if (ret == IFX_SUCCESS)
   {
      switch (mode)
      {
      case IFX_TAPI_INIT_MODE_DEFAULT:
#if (VIN_CFG_FEATURES & VIN_FEAT_VOICE)
      case IFX_TAPI_INIT_MODE_VOICE_CODER:
         ret = VINETIC_Basic_VoIPConf (pCh);

         if (ret == IFX_SUCCESS && pDev->nProtocolType == IFX_TAPI_PRT_TYPE_RTP)
         {
            memset (&rtpConf, 0, sizeof (IFX_TAPI_PKT_RTP_CFG_t));
            rtpConf.nEventPT = pCh->nEvtPT;
            rtpConf.nEvents = IFX_TAPI_PKT_EV_OOB_ONLY;
            ret = IFX_TAPI_LL_COD_RTP_Cfg (pChannel->pLLChannel, &rtpConf);
         }

         if (ret == IFX_SUCCESS)
         {
            memset (&jbConf, 0, sizeof (IFX_TAPI_JB_CFG_t));
            jbConf.nInitialSize = 0x0050;
            jbConf.nMinSize = 0x0050;
            jbConf.nMaxSize = 0x05A0;
            jbConf.nPckAdpt = IFX_TAPI_JB_PKT_ADAPT_VOICE;
            jbConf.nJbType = IFX_TAPI_JB_TYPE_ADAPTIVE;
            jbConf.nScaling = 0x16;
            ret = IFX_TAPI_LL_COD_JB_Cfg (pChannel->pLLChannel, &jbConf);
         }

#ifdef QOS_SUPPORT
         if ((ret == IFX_SUCCESS) && !(pDev->nDevState & DS_QOS_INIT))
         {
            ret = Qos_LL_Init((IFX_uint32_t) pDev);
         }
#endif /* QOS_SUPPORT */
#endif /* (VIN_CFG_FEATURES & VIN_FEAT_VOICE) */
         if (ret == IFX_SUCCESS)
            pCh->pTapiCh->nFlags |= CF_TAPI_VOICEMODE;
         break;
      case IFX_TAPI_INIT_MODE_PCM_DSP:
         /* this preconfiguration is currently not supported */
         SET_ERROR (VIN_ERR_NOTSUPPORTED);
         ret = IFX_ERROR;
#if 0
         ret = VINETIC_Basic_PcmConf (pCh, mode);
         if (ret == IFX_SUCCESS)
            pCh->pTapiCh->nFlags |= CF_TAPI_PCMMODE;
#endif /* if 0 */
         break;
      case IFX_TAPI_INIT_MODE_PCM_PHONE :
         /* this preconfiguration is currently not supported */
         SET_ERROR (VIN_ERR_NOTSUPPORTED);
         ret = IFX_ERROR;
#if 0
         ret = VINETIC_Basic_PcmConf (pCh, mode);
         if (ret == IFX_SUCCESS)
            pCh->pTapiCh->nFlags |= CF_TAPI_S_PCMMODE;
#endif /* if 0 */
         break;
      default:
         /* no specific initialization */
         break;
      }
   }
   return ret;
}


/**
  Configure VINETIC Chip according to Tapi settings
\param
   pDev  - pointer to VINETIC device structure
\return
   IFX_SUCCESS or IFX_ERROR
\remarks
   This configuration will be done if tapi configuration mode is
   IFX_TAPI_INIT_MODE_VOICE_CODER

   Access to cached firmware messages is protected against concurrent tasks by
   the share variable mutex
*/
static IFX_int32_t VINETIC_Basic_VoIPConf (VINETIC_CHANNEL *pCh)
{
   VINETIC_DEVICE *pDev = pCh->pParent;
   IFX_int32_t err = IFX_SUCCESS;
   IFX_uint16_t pCmd [3] = {0};
   IFX_uint8_t ch = pCh->nChannel - 1;

#if (VIN_CFG_FEATURES & VIN_FEAT_VIN_S)
   if ((pDev->nChipType == VINETIC_TYPE_S) &&
       (pDev->nChipMajorRev == VINETIC_V2x))
   {
      /* not supported for S type */
      SET_ERROR (VIN_ERR_NOTSUPPORTED);
      return IFX_ERROR;
   }
#endif /* (VIN_CFG_FEATURES & VIN_FEAT_VIN_S) */


   /* create the data channels by connecting COD to SIG on the same channel */
   if ((ch < pDev->caps.nSIG) && (ch < pDev->caps.nCOD))
   {
      VINETIC_CON_ConnectPrepare (pDev, ch, VINDSP_MT_COD, VINDSP_MT_SIG,
                                  REMOTE_SIG_OUT);
      VINETIC_CON_ConnectPrepare (pDev, ch, VINDSP_MT_SIG, VINDSP_MT_COD,
                                  REMOTE_SIG_OUT);
   }

#ifdef ENABLE_OBSOLETE_PREMAPPING
   /* preconnect ALM to data channels on the same channels */
   if ((ch < pDev->caps.nSIG) && (ch < pDev->caps.nALM))
   {
      VINETIC_CON_ConnectPrepare (pDev, ch, VINDSP_MT_SIG, VINDSP_MT_ALM,
                                  LOCAL_SIG_OUT);
      VINETIC_CON_ConnectPrepare (pDev, ch, VINDSP_MT_ALM, VINDSP_MT_SIG,
                                  LOCAL_SIG_OUT);
   }
#endif /* ENABLE_OBSOLETE_PREMAPPING */


   /* EOP Cmd */
   pCmd [0] = CMD1_EOP;
   /* EN = 1 */
   pCmd [2] = 0x8000;

   if (!(pDev->nDevState & DS_COD_EN))
   {
      /* configure Coder Module */
      pCmd [1] = ECMD_COD_CTL;
      err = CmdWrite (pDev, pCmd, 1);
      if (err == IFX_SUCCESS)
         /* set coder module to enabled */
         pDev->nDevState |= DS_COD_EN;
   }

   if (!(pDev->nDevState & DS_ALM_EN))
   {
      /* configure analog line module */
      pCmd [1] = ECMD_ALM_CTL;
      err = CmdWrite (pDev, pCmd, 1);
      if (err == IFX_SUCCESS)
         /* set coder module to enabled */
         pDev->nDevState |= DS_ALM_EN;
   }

   if (!(pDev->nDevState & DS_SIG_EN))
   {
      /* configure signaling module */
      pCmd [1] = ECMD_SIG_CTL;
      err = CmdWrite (pDev, pCmd, 1);
      if (err == IFX_SUCCESS)
         /* set coder module to enabled */
         pDev->nDevState |= DS_SIG_EN;
   }

#if 1
   if (!(pDev->nDevState & DS_PCM_EN))
   {
      /* configure PCM interface control */
      pCmd [1] = ECMD_PCM_CTL;
      pCmd [2] = PCM_CTL_EN;
      err = CmdWrite (pDev, pCmd, 1);
      if (err == IFX_SUCCESS)
         /* set coder module to enabled */
         pDev->nDevState |= DS_PCM_EN;
   }
   if (err != IFX_SUCCESS)
      return err;
#endif /** \todo quick fix: activate PCM module for possible connections  */


   /* protect fwmsgs against concurrent tasks */
   IFXOS_MutexLock (pDev->memberAcc);
   /* configure ALI for VoIP */
   if (ch < pDev->caps.nALI)
      err = VINETIC_ALM_baseConf (pCh);
   /* configure Coder  */
   if (err == IFX_SUCCESS && (ch < pDev->caps.nCOD))
      err = VINETIC_COD_baseConf (pCh);
   /* Configure Signalling module */
   if (err == IFX_SUCCESS && (ch < pDev->caps.nSIG))
      err = VINETIC_SIG_baseConf (pCh);
   /* unlock */
   IFXOS_MutexUnlock (pDev->memberAcc);

   if (err == IFX_SUCCESS)
      VINETIC_CON_ConnectConfigure (pDev);

   return err;
}


#if 0
/**
  Configure VINETIC Chip for Voice Coder purposes
\param
   pDev       - pointer to VINETIC device structure
\param
   nPcmType   - PCM Advanced or for application. Not yet in use.
\return
   IFX_SUCCESS or IFX_ERROR
\remarks
   This function configures the chip for voice communication via PCM.
   The actual configuration is for PCM-S and the parameter nPcmType isn't
   evaluated . This will be done when the PCM-Standard config will be done.

   Access to cached firmware messages is protected against concurrent tasks by
   the share variable mutex
*/
static IFX_int32_t VINETIC_Basic_PcmConf (VINETIC_CHANNEL *pCh, IFX_uint8_t nPcmType)
{
   VINETIC_DEVICE *pDev = pCh->pParent;
   IFX_uint16_t pCmd [3] = {0};
   IFX_int32_t err = IFX_SUCCESS;
   IFX_uint8_t ch = pCh->nChannel - 1;

#if (VIN_CFG_FEATURES & (VIN_FEAT_VIN_C | VIN_FEAT_VIN_M))
   if ((pDev->nChipType == VINETIC_TYPE_S) &&
       (nPcmType != IFX_TAPI_INIT_MODE_PCM_PHONE))
   {
      /* not supported for S type */
      SET_ERROR (VIN_ERR_NOTSUPPORTED);
      return IFX_ERROR;
   }
#endif /* (VIN_CFG_FEATURES & (VIN_FEAT_VIN_C | VIN_FEAT_VIN_M)) */

   /* EOP Cmd */
   pCmd [0] = CMD1_EOP;

   if (!(pDev->nDevState & DS_PCM_EN))
   {
      /* configure PCM interface control */
      pCmd [1] = ECMD_PCM_CTL;
      pCmd [2] = PCM_CTL_EN;
      err = CmdWrite (pDev, pCmd, 1);
   }
   if (err != IFX_SUCCESS)
      return err;
   /* set PCM module to enabled */
   pDev->nDevState |= DS_PCM_EN;

   if (!(pDev->nDevState & DS_ALM_EN))
   {
      /* configure analog line module */
      pCmd [1] = ECMD_ALM_CTL;
      pCmd [2] = ALM_CTL_EN;
      err = CmdWrite (pDev, pCmd, 1);
   }
   if (err != IFX_SUCCESS)
      return err;
   /* set analog line module to enabled */
   pDev->nDevState |= DS_ALM_EN;

#if (VIN_CFG_FEATURES & (VIN_FEAT_VIN_C | VIN_FEAT_VIN_M | VIN_FEAT_VIN_2CPE))
   if (nPcmType == IFX_TAPI_INIT_MODE_PCM_DSP)
   {
      if (!(pDev->nDevState & DS_SIG_EN))
      {
         /* configure signaling module */
         pCmd [1] = ECMD_SIG_CTL;
         pCmd [2] = SIG_CTL_EN;
         err = CmdWrite (pDev, pCmd, 1);
      }
      if (err != IFX_SUCCESS)
         return err;

      /* set signalling module to enabled */
      pDev->nDevState |= DS_SIG_EN;
   }
#endif /* (VIN_FEAT_VIN_C | VIN_FEAT_VIN_M | VIN_FEAT_VIN_2CPE) */

#ifdef ENABLE_OBSOLETE_PREMAPPING
   switch (nPcmType)
   {
   case IFX_TAPI_INIT_MODE_PCM_DSP:
      if (ch < pDev->caps.nSIG)
      {
         err = VINETIC_CON_ConnectPrepare (pDev, ch,
                                           VINDSP_MT_SIG, VINDSP_MT_PCM,
                                           REMOTE_SIG_OUT);
         ret = VINETIC_CON_ConnectPrepare (pDev, ch,
                                           VINDSP_MT_SIG, VINDSP_MT_ALM,
                                           LOCAL_SIG_OUT);
      }
      break;
   case IFX_TAPI_INIT_MODE_PCM_PHONE:
      ret = VINETIC_CON_ConnectPrepare (pDev, ch,
                                        VINDSP_MT_PCM, VINDSP_MT_ALM,
                                        REMOTE_SIG_OUT);
      err = VINETIC_CON_ConnectPrepare (pDev, ch,
                                        VINDSP_MT_ALM, VINDSP_MT_PCM,
                                        REMOTE_SIG_OUT);
      break;
   default:
      SET_ERROR(VIN_ERR_NOTSUPPORTED);
      return IFX_ERROR;
   }
#endif /* ENABLE_OBSOLETE_PREMAPPING */

   /* protect fwmsgs against concurrent tasks */
   IFXOS_MutexLock (pDev->memberAcc);

   /* configure ALI for PCM */
   if (ch < pDev->caps.nALI)
      err = VINETIC_ALM_baseConf (pCh, nPcmType);
   if (err != IFX_SUCCESS)
      goto error;

   /* configure PCM  */
   if (ch < pDev->caps.nPCM)
      err = VINETIC_PCM_baseConf (pCh, nPcmType);
#if (VIN_CFG_FEATURES & (VIN_FEAT_VIN_C | VIN_FEAT_VIN_M | VIN_FEAT_VIN_2CPE))
   if (err != IFX_SUCCESS)
      goto error;

   /* configure SIG */
   if ((nPcmType == IFX_TAPI_INIT_MODE_PCM_DSP) &&
       (ch < pDev->caps.nSIG))
      err = VINETIC_SIG_Base_Conf (pCh);
#endif /* (VIN_FEAT_VIN_C | VIN_FEAT_VIN_M | VIN_FEAT_VIN_2CPE) */

error:
   IFXOS_MutexUnlock (pDev->memberAcc);
   if (err == IFX_SUCCESS)
      err = VINETIC_CON_ConnectConfigure (pDev);

   return err;
}
#endif


/**
   Reset of the device.
\param
   pDev              - pointer to the device structure
\return
   IFX_SUCCESS if ok, otherwise IFX_ERROR
\remarks
   Can be used to reset device internal state after a hard reset from
   application in case the whole device initialization done before is
   still valid.
*/
IFX_int32_t  VINETIC_DeviceReset(VINETIC_DEVICE *pDev)
{
   IFX_uint32_t nDevState = pDev->nDevState;

   /* reset this device */
   ResetDevState(pDev);

   /* reset all bits except the basic init bit, which should be
      kept unchanged */
   pDev->nDevState = (nDevState & DS_BASIC_INIT);

   return IFX_SUCCESS;
}


/**
   Updates specific channel members on channel open.
\param
   pCh       - pointer to VINETIC channel structure
\return
   IFX_SUCCESS if ok, otherwise IFX_ERROR
\remarks
   none
*/
IFX_int32_t VINETIC_UpdateChMember(IFX_TAPI_LL_CH_t *pLLCh)
{
   /* Moved to TAPI high level. */
   IFX_int32_t err = IFX_SUCCESS;

   return err;
}


/**
  Frees all memory previously allocated in VINETIC_InitDevMember
\param
   pDev   - handle to the device
\remarks
   Called when device is released. Nobody uses this device
*/
IFX_void_t VINETIC_ExitDev(VINETIC_DEVICE *pDev)
{
    int i;

   /* delete device mutexes */
   IFXOS_MutexDelete (pDev->mbxAcc);
   IFXOS_MutexDelete (pDev->memberAcc);

   /* delete channel mutexes */
   for (i = 0; i < VINETIC_MAX_CH_NR; i++)
   {
      IFXOS_MutexDelete (pDev->pChannel[i].chAcc);
   }

   /* free fw module related structures */
   for (i = 0; i < pDev->caps.nCOD; ++i)
   {
      VINETIC_COD_Free_Ch_Structures (&pDev->pChannel[i]);
   }

   for (i = 0; i < pDev->caps.nSIG; ++i)
   {
      VINETIC_SIG_Free_Ch_Structures (&pDev->pChannel[i]);
   }

   for (i = 0; i < pDev->caps.nALI; ++i)
   {
      VINETIC_ALM_Free_Ch_Structures (&pDev->pChannel[i]);
   }

   for (i = 0; i < pDev->caps.nPCM; ++i)
   {
      VINETIC_PCM_Free_Ch_Structures (&pDev->pChannel[i]);
   }

   /* free capability structure */
   IFXOS_FREE (pDev->CapList);

#ifdef EVALUATION
#ifndef VIN_2CPE
   IFXOS_FREE (((VINETIC_EVAL*)pDev->pEval)->irqFifo.pStart);
   for (i = 0; i < VINETIC_MAX_CH_NR; i++)
   {
      IFXOS_FREE (pDev->pChannel[i].pEval);
   }
#endif /* VIN_2CPE */
   IFXOS_FREE (pDev->pEval);
#endif /* EVALUATION */

#ifdef ADDED_SUPPORT
   /* Deallocate the Device Interrupt */
   /* Error Status FIFO               */
    FreeErrFifo (pDev);
#endif /* ADDED_SUPPORT */
}


/**
   Initialize the low level device pointer.

   \param pTapiDev - handle to TAPI_DEV
   \param pDev   - handle to VINETIC_DEVICE
   \param devNum - number of device

   \return  IFX_NULL on error, otherwise low level device

   \remark
      Called by the High Level TAPI module (tapi_open).
*/
IFX_TAPI_LL_DEV_t *VINETIC_Prepare_Dev(TAPI_DEV *pTapiDev,
                                       VINETIC_DEVICE *pDev,
                                       IFX_int32_t devNum)
{
#ifdef DEBUG
   SetTraceLevel(VINETIC, DBG_LEVEL_NORMAL);
#else
   SetTraceLevel(VINETIC, DBG_LEVEL_HIGH);
#endif /* DEBUG */

   if (pTapiDev == IFX_NULL)
   {
      TRACE(VINETIC, DBG_LEVEL_HIGH,
            ("VINETIC_Prepare_Dev: pTapiDev is NULL\n\r"));
      return IFX_NULL;
   }

   /* Initialization */
   memset(pDev, 0, sizeof(VINETIC_DEVICE));
   pDev->nDevNr = devNum;
   sVinetic_Devices[devNum] = pDev;

   /* Store the corresponding HL pointer */
   pDev->pTapiDev = pTapiDev;

   /* Os and board independend initializations, resets all values */
   if (VINETIC_InitDevMember(pDev) == IFX_ERROR)
   {
       TRACE(VINETIC, DBG_LEVEL_HIGH,
            ("ERROR: Initialization of Low level device structure failed\n\r"));
       return IFX_NULL;
   }

   /* Should return the pDev which should be stored by the HL */
   return pDev;
}


/**
   Initialize the low level channel pointer.

   \param  pTapiCh - handle to TAPI_CHANNEL
   \param  pDev    - handle to VINETIC_DEVICE
   \pararm chNum   - channel number

   \return
              IFX_SUCCESS / IFX_ERROR
   \remark
      Called by the High Level TAPI module (tapi_open).
*/
IFX_void_t *VINETIC_Prepare_Ch(TAPI_CHANNEL *pTapiCh,
                               VINETIC_DEVICE *pDev,
                               IFX_uint32_t chNum)
{
   VINETIC_CHANNEL *pCh = &(pDev->pChannel[chNum]);


   if (pTapiCh == IFX_NULL)
   {
      TRACE(VINETIC, DBG_LEVEL_HIGH,
            ("VINETIC_Prepare_Ch: pTapiCh is NULL\n\r"));
      return IFX_NULL;
   }

   pCh->pParent  = pDev;
   pCh->nChannel = (IFX_uint8_t)chNum + 1;

   /* Defaut Payload type */
   pCh->nEvtPT   = DEFAULT_EVTPT;

   /* Store the corresponding HL TAPI pointer */
   pCh->pTapiCh = pTapiCh;

   if (initChMember(pCh) == IFX_ERROR)
   {
      return IFX_NULL;
   }
   pDev->nChipRev = VINETIC_2CPE_V22;
   /* add basic caps */
   VINETIC_AddCaps(pDev);

   /* Should return the pCh which should be stored by the HL */
   return pCh;
}


/**
   VINETIC device driver initialization.
   This is the device driver initialization function to call at the system
   startup prior any access to the VINETIC device driver.
   After the initialization the device driver is ready to be accessed by
   the application. The global structure "VIN_dev_ctx" contains all the data
   handling the interface (open, close, ioctl,...).

   \param none

   \return OK or ERROR
*/
IFX_int32_t VINETIC_DeviceDriverInit(IFX_void_t)
{
   IFX_int32_t result = 0;
   IFX_TAPI_DRV_CTX_t *pDrvCtx = IFX_NULL;

   /* Set the Default Traces */
#ifdef DEBUG
   SetTraceLevel(VINETIC, DBG_LEVEL_NORMAL);
#else
   SetTraceLevel(VINETIC, DBG_LEVEL_HIGH);
#endif

   pDrvCtx = (IFX_TAPI_DRV_CTX_t*) IFXOS_MALLOC (sizeof(IFX_TAPI_DRV_CTX_t));

   if (pDrvCtx == IFX_NULL)
   {
      return IFX_ERROR;
   }

   memset(pDrvCtx, 0, sizeof(IFX_TAPI_DRV_CTX_t));

   /* Initialize the function pointers structure and register
      with the High Level TAPI */
   pDrvCtx->majorNumber = major;
   pDrvCtx->minorBase = minorBase;

   pDrvCtx->devNodeName = devName;
   pDrvCtx->maxDevs = VINETIC_MAX_DEVICES;
   pDrvCtx->maxChannels = VINETIC_MAX_CH_NR;

   /* procfs info */
   pDrvCtx->drvName = DEV_NAME;
   pDrvCtx->drvVersion = DRV_VINETIC_VER_STR;
   pDrvCtx->hlLLInterfaceVersion = DRV_LL_INTERFACE_VER_STR;

   /* Generic functions  */
   pDrvCtx->Prepare_Dev = IFX_TAPI_LL_Prepare_Dev;
   pDrvCtx->Prepare_Ch = IFX_TAPI_LL_Prepare_Ch;
   pDrvCtx->Ioctl = VINETIC_Dev_Spec_Ioctl;

   /** \todo Change function input arguments. */
   pDrvCtx->Read = VINETIC_LL_Read;
   pDrvCtx->Write = VINETIC_LL_Write;

   pDrvCtx->Open = IFX_TAPI_LL_Open;
   pDrvCtx->Release = IFX_TAPI_LL_Close;

   pDrvCtx->GetCmdMbxSize = VINETIC_Host_GetCmdMbxSize;

   pDrvCtx->CAP_Number_Get = TAPI_LL_Phone_Get_Capabilities;
   pDrvCtx->CAP_List_Get = TAPI_LL_Phone_Get_Capability_List;
   pDrvCtx->CAP_Check = TAPI_LL_Phone_Check_Capability;

   pDrvCtx->Init_Dev = TAPI_LL_Phone_Init;

   /* FIFO related functions */
   pDrvCtx->UpdateChMember = VINETIC_UpdateChMember;

   /* IRQ information */
   pDrvCtx->IRQ.LockDevice = Vinetic_IrqLockDevice;
   pDrvCtx->IRQ.UnlockDevice = Vinetic_IrqUnlockDevice;
   pDrvCtx->IRQ.IrqEnable = Vinetic_IrqEnable;
   pDrvCtx->IRQ.IrqDisable = Vinetic_IrqDisable;

   /* CODer Module */
   VINETIC_COD_Func_Register (&pDrvCtx->COD);

#ifdef TAPI_VOICE
   /* CONnection module */
   VINETIC_CON_Func_Register (&pDrvCtx->CON);
#endif

   /* PCM related */
   VINETIC_PCM_Func_Register (&pDrvCtx->PCM);

   /* SIGnalling related */
   VINETIC_SIG_Func_Register (&pDrvCtx->SIG);

   /* ALM specific */
   VINETIC_ALM_Func_Register (&pDrvCtx->ALM);
   VINETIC_ALM_CPE_Func_Register (&pDrvCtx->ALM);

#ifdef TAPI_POLL
   VINETIC_POLL_Func_Register (&pDrvCtx->POLL);
#endif

#ifndef VIN_2CPE
/*   pDrvCtx->ALM.TG_ToneStep = TG_ToneStep;*/
#endif

#ifdef QOS_SUPPORT
   /** Register Qos Ingress functions. */
   pDrvCtx->Ingress = Qos_LL_PktIngress;
   pDrvCtx->Egress = Qos_LL_PktEgress;
#endif /* QOS_SUPPORT */

   result = IFX_TAPI_Register_LL_Drv (pDrvCtx);
   if (result == IFX_ERROR)
   {
#ifndef LINUX
      printf("vinetic_module_init: registration failed \n");
#else /* VXWORKS */
      printk(KERN_INFO "vinetic_module_init: registration failed \n");
#endif /* VXWORKS */
      IFXOS_FREE(pDrvCtx);
      return IFX_ERROR;
   }

#ifdef VXWORKS
   printf("%s, (c) 2007 Infineon Technologies AG\n\r", &DRV_VINETIC_WHATVERSION [4]);
#else /* VXWORKS */
   printk("%s, (c) 2007 Infineon Technologies AG\n\r", &DRV_VINETIC_WHATVERSION [4]);
#endif /* VXWORKS */

   return IFX_SUCCESS;
}
