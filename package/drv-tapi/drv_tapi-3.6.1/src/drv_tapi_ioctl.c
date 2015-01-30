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
   \file drv_tapi_ioctl.c
   Date: 2007-01-31

*/

/* ============================= */
/* Includes                      */
/* ============================= */

#include "drv_api.h"
#include "drv_tapi.h"
#include "drv_tapi_api.h"
#include "drv_tapi_ll_interface.h"
#include "drv_tapi_event.h"
#include "drv_tapi_stream.h"
#include "drv_tapi_polling.h"
#include "drv_tapi_fxo_ll_interface.h"
#include "drv_tapi_errno.h"
#include "drv_tapi_ioctl.h"

#ifdef TAPI_CID
#include "drv_tapi_cid.h"
#endif

/* ============================= */
/* Local Macros & Definitions    */
/* ============================= */

#define ON_IOCTL_LLFUNC(msg,func,ioarg)                    \
           case msg:                                        \
              if (func == IFX_NULL)                         \
                  ret = TAPI_statusLLNotSupp;            \
              else                                          \
                  ret = func(pChannel->pLLChannel, ioarg);  \
              break

/* 3 parameter, call like
   IFX_TAPI_Ring_SetConfig (pChannel, (IFX_TAPI_RING_CFG_t *) ioarg);
*/
#define ON_IOCTLW3(msg,func,arg1,arg2,type)          \
           case msg:                                   \
              ret = func(arg1,arg2,(type)ioarg);       \
              break

#define ON_IOCTLW2(msg,func,arg,type)          \
           case msg:                                  \
              ret = func(arg,(type)ioarg);              \
              break

#define ON_IOCTLR1(msg,func,arg,type)          \
           case msg:                                  \
              ret = func(arg,(type)ioarg);     \
              break

#define ON_IOCTLW1 (msg,func, arg)               \
           case msg:                           \
              ret = func(arg);                   \
              break

/* ============================= */
/* Global variable definition    */
/* ============================= */

/* ============================= */
/* Local function declaration    */
/* ============================= */

static IFX_int32_t  TAPI_IoctlCh (IFX_TAPI_DRV_CTX_t* pDrvCtx,
                                  IFX_TAPI_ioctlCtx_t* pCtx,
                                  IFX_uint32_t iocmd, IFX_uint32_t ioarg);
static IFX_int32_t TAPI_IoctlDev (IFX_TAPI_DRV_CTX_t* pDrvCtx,
                                  IFX_TAPI_ioctlCtx_t* pCtx,
                                  IFX_uint32_t cmd, IFX_uint32_t ioarg);
#ifdef TAPI_AUDIO_CHANNEL
static IFX_int32_t  TAPI_IoctlAudioCh (IFX_TAPI_DRV_CTX_t* pDrvCtx,
                                  IFX_TAPI_ioctlCtx_t* pCtx,
                                  IFX_uint32_t iocmd, IFX_uint32_t ioarg);
#endif /* TAPI_AUDIO_CHANNEL */

/* ============================= */
/* Local variable definition     */
/* ============================= */

/* ============================= */
/* Local function definition     */
/* ============================= */

/* ============================= */
/* Global function definition    */
/* ============================= */

void IFX_TAPI_ioctlContextGet (IFX_TAPI_DRV_CTX_t *pDrvCtx,
                               IFX_int32_t nMinor,
                               IFX_TAPI_ioctlCtx_t *pCtx)
{
   IFX_int32_t dev = 0, ch = 0;

   memset (pCtx, 0, sizeof(IFX_TAPI_ioctlCtx_t));
#ifdef TAPI_ONE_DEVNODE
   /* check if we are still in the old style with different
      fds for each channel and if the argument is really a structure
      and not only a number */
   if (nMinor == 0)
   {
      pCtx->bSingleFd = IFX_TRUE;
   }
   else
   {
      /* get device number, workaround to make it work till all applications
         support the new approach */
      dev = (nMinor / 10) - 1;
      ch  =  nMinor % 10;
   }
#else
   /* get device number */
   dev = (nMinor / pDrvCtx->minorBase) - 1;
   ch  =  nMinor % pDrvCtx->minorBase;
#endif /* TAPI_ONE_DEVNODE */
   if (pCtx->bSingleFd)
   {
      /* thats the first TapiDev */
      pCtx->bDrv = IFX_TRUE;
      pCtx->p.pTapiDev = pDrvCtx->pTapiDev;
   }
   else
   {
      if (ch == 0)
      {
         pCtx->p.pTapiDev = &(pDrvCtx->pTapiDev[dev]);
         pCtx->bDev = IFX_TRUE;
      }
      else
      {
         pCtx->p.pTapiCh = &(pDrvCtx->pTapiDev[dev].pTapiChanelArray[ch - 1]);
         pCtx->bCh = IFX_TRUE;
      }
   }
}


/**
   Device Specific ioctl handling

\param pDrvCtx  - handle to device driver context
\param pTapiCh  - handle to TAPI_CHANNEL structure (could be also TAPI_DEVICE)
\param nCmd     - ioctl command
\param ioarg     - ioctl argument
\param bCtrlDev - TRUE : Ioctl for Control device /  FALSE: Ioctl for channel
\return
   IFX_SUCCESS or IFX_ERROR
*/
int TAPI_Dev_Spec_Ioctl (IFX_TAPI_DRV_CTX_t* pDrvCtx,
                                 IFX_TAPI_ioctlCtx_t* pCtx, IFX_uint32_t nCmd,
                                 IFX_uint32_t ioarg)
{
   /* sanity check */
   if (pDrvCtx->Ioctl == IFX_NULL)
   {
      return IFX_ERROR;
   }
   if (pCtx->bSingleFd || pCtx->bDev)
      return pDrvCtx->Ioctl (pCtx->p.pTapiDev->pLLDev, nCmd, ioarg);
   /* Provide the correct low level device or channel context (casted to a
      low level channel) to the low level ioctl handler. The low level
      ioctl handler has to check the correct context again... */
   /* dispatch to low level ioctl handler */
   return pDrvCtx->Ioctl (pCtx->p.pTapiCh->pLLChannel, nCmd, ioarg);
}

/**
   Tapi Specific ioctl handling

\param pDrvCtx - handle to device driver context
\param pCtx    - handle to ioctl context
\param iocmd   - ioctl command
\param ioarg   - ioctl argument

\return
   IFX_SUCCESS or IFX_ERROR

\remark
   In case the device ptr handle is needed, it will be accessed via
   the pParent of the first channel ptr pChannel[0].pParent. The valid ptr to
   that channel structure will then be passed by the overlying function.

   Only tapi ioctls with appropriate tapi magic number are handled within
   this function.
*/
IFX_int32_t  TAPI_Spec_Ioctl (IFX_TAPI_DRV_CTX_t* pDrvCtx,
                              IFX_TAPI_ioctlCtx_t* pCtx,
                              IFX_uint32_t iocmd,
                              IFX_uint32_t ioarg)
{
   TAPI_DEV* pTapiDev = pCtx->p.pTapiDev;
   IFX_int32_t ret = IFX_SUCCESS;

   /* if we are using single device nodes, this check cannot be done */
   switch (iocmd)
   {
      case IFX_TAPI_VERSION_GET:
      case IFX_TAPI_VERSION_CHECK:
      case IFX_TAPI_CAP_NR:
      case IFX_TAPI_CAP_LIST:
      case IFX_TAPI_CAP_CHECK:
      case IFX_TAPI_TONE_TABLE_CFG_SET:
      case IFX_TAPI_POLL_CONFIG:
      case IFX_TAPI_POLL_DEV_ADD:
      case IFX_TAPI_POLL_DEV_REM:
      case IFX_TAPI_POLL_WRITE:
      case IFX_TAPI_POLL_READ:
      case IFX_TAPI_POLL_EVENTS:
      case IFX_TAPI_POLL_TEST:
      case IFX_TAPI_LASTERR:
      case IFX_TAPI_DEBUG_REPORT_SET:
      case IFX_TAPI_PCM_IF_CFG_SET:
      case IFX_TAPI_EVENT_ENABLE:
      case IFX_TAPI_EVENT_DISABLE:
      case IFX_TAPI_EVENT_GET:
         if (pCtx->bDev == IFX_FALSE && pCtx->bDrv == IFX_FALSE)
         {
            if (!pCtx->bSingleFd)
            {
               /* be tolerant and transfer the context to device */
               switch (iocmd)
               {
                  case IFX_TAPI_LASTERR:
                     break;
                  default:
                     TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
                    ("Warning: dev ioctl issued on channel fd (ch%d), tolerated"
                     ", 0x%08X\n\r", pCtx->p.pTapiCh->nChannel, iocmd));
                     break;
               }
               pTapiDev         = pCtx->p.pTapiCh->pTapiDevice;
               pCtx->p.pTapiDev = pCtx->p.pTapiCh->pTapiDevice;
               pCtx->bCh  = IFX_FALSE;
               pCtx->bDev = IFX_TRUE;
            }
            else
            {
               RETURN_DEVSTATUS (TAPI_statusCtxErr, 0);
            }
         }

         /* device ioctls */
         IFXOS_MutexLock (pTapiDev->semTapiDevSingleIoctlAccess);
         ret = TAPI_IoctlDev (pDrvCtx, pCtx, iocmd, ioarg);
         IFXOS_MutexUnlock (pTapiDev->semTapiDevSingleIoctlAccess);
         break;

      /* FXO services */
      case IFX_TAPI_FXO_DIAL_CFG_SET:
      case IFX_TAPI_FXO_FLASH_CFG_SET:
      case IFX_TAPI_FXO_OSI_CFG_SET:
      case IFX_TAPI_FXO_DIAL_START:
      case IFX_TAPI_FXO_DIAL_STOP:
      case IFX_TAPI_FXO_HOOK_SET:
      case IFX_TAPI_FXO_FLASH_SET:
      case IFX_TAPI_FXO_BAT_GET:
      case IFX_TAPI_FXO_HOOK_GET:
      case IFX_TAPI_FXO_APOH_GET:
      case IFX_TAPI_FXO_RING_GET:
      case IFX_TAPI_FXO_POLARITY_GET:
         IFXOS_MutexLock (pCtx->p.pTapiCh->semTapiChSingleIoctlAccess);
         /**\todo not yet supporting single dev nodes */
         ret = TAPI_FXO_Ioctl (pDrvCtx, pCtx->p.pTapiCh, iocmd, ioarg);
         IFXOS_MutexUnlock (pCtx->p.pTapiCh->semTapiChSingleIoctlAccess);
         break;

#ifdef TAPI_AUDIO_CHANNEL
      case IFX_TAPI_AUDIO_VOLUME_SET:
      case IFX_TAPI_AUDIO_ROOM_TYPE_SET:
      case IFX_TAPI_AUDIO_MUTE_SET:
      case IFX_TAPI_AUDIO_MODE_SET:
      case IFX_TAPI_AUDIO_RING_START:
      case IFX_TAPI_AUDIO_RING_STOP:
      case IFX_TAPI_AUDIO_RING_VOLUME_SET:
      case IFX_TAPI_AUDIO_ICA_SET:
         if (pCtx->bDev == IFX_FALSE && pCtx->bDrv == IFX_FALSE)
         {
            RETURN_DEVSTATUS (TAPI_statusCtxErr, 0);
         }
         pCtx->p.pTapiCh = &pTapiDev->pTapiChanelArray[0];
         pCtx->bDev = IFX_FALSE;
         pCtx->bCh = IFX_TRUE;
         IFXOS_MutexLock (pCtx->p.pTapiCh->semTapiChSingleIoctlAccess);
         /* process with channel fd */
         ret = TAPI_IoctlAudioCh (pDrvCtx, pCtx, iocmd, ioarg);
         IFXOS_MutexUnlock (pCtx->p.pTapiCh->semTapiChSingleIoctlAccess);
         break;
#endif

      default:
         if (pCtx->bSingleFd && (ioarg >= 0 && ioarg <= 255))
         {
            /* case (5): return error if single dev node and
               no structure passed */
            RETURN_DEVSTATUS (TAPI_statusParam, 0);
         }
         if (pCtx->bDev == IFX_TRUE)
            RETURN_DEVSTATUS (TAPI_statusCtxErr, 0);

         /* process with channel fd */
         ret = TAPI_IoctlCh (pDrvCtx, pCtx, iocmd, ioarg);
         break;
      }

   return ret;
   }

/**
   Tapi Channel Specific ioctl handling

\param pDrvCtx handle to device driver context
\param pCtx    handle to the ioctl context structure
\param cmd     ioctl command
\param ioarg   ioctl argument

\return
   IFX_SUCCESS or IFX_ERROR
*/
static IFX_int32_t  TAPI_IoctlCh (IFX_TAPI_DRV_CTX_t* pDrvCtx,
                                  IFX_TAPI_ioctlCtx_t* pCtx,
                                  IFX_uint32_t iocmd, IFX_uint32_t ioarg)
{
   IFX_int32_t ret = IFX_SUCCESS,
               retLL = IFX_SUCCESS;
   /* param is used for simple int arguments in one direction */
   IFX_int32_t param;
   TAPI_CHANNEL* pChannel;
   IFX_TAPI_IOCTL_t tmpAdr;
   IFX_boolean_t bHandled;

   /* context verification */
   if (pCtx->bDev == IFX_TRUE)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
           ("ERROR: TAPI ch ioctl issued on dev fd (0x%08X), rejected\n\r",
           iocmd));
      pChannel = &pCtx->p.pTapiDev->pTapiChanelArray[0];
      RETURN_STATUS (TAPI_statusCtxErr, 0);
   }

   /* If we support the only one fd, find out the device and channel from the
      given structure. Distinguish between already ported structures,
      applications and ioctls that are global or device global.
      Following is supported:
      - (1) any ioctl using new structures with dev and ch on the only dev node
      - (2) any ioctl using new structurs with dev and ch on channel fds
      - (3) channel ioctls, when used on channel fds with new structures
      - (4) channel ioctls using simple integers, when used on channel fds
      - (5) return error if single dev node and no structure passed, handled in
            calling function
   */
#ifdef TAPI_ONE_DEVNODE
   if (pCtx->bSingleFd == IFX_FALSE)
   {
#endif /* TAPI_ONE_DEVNODE */
      /* handling case (2), (3) and (4) */
      pChannel = pCtx->p.pTapiCh;
#ifdef TAPI_ONE_DEVNODE
      if ((ioarg >= 0) && (ioarg <= 255))
         /* case (4) */
#endif
         param = ioarg;
#ifdef TAPI_ONE_DEVNODE
      else
      {
         /* case (2) and (3) using a new strucutre, get the context and pass
            again the one argument. For case (2) we ignore the dev and ch */
         IFXOS_CPY_USR2KERN ((char*)&tmpAdr, (IFX_uint8_t*)ioarg,
                              sizeof (IFX_TAPI_IOCTL_t));
         /* copy the one parameter */
         param = tmpAdr.param;
      }
   }
   else
   {
      /* case (1) using a new strucutre, get the context and pass again
         the one argument */
      IFXOS_CPY_USR2KERN ((char*)&tmpAdr, (IFX_uint8_t*)ioarg,
                          sizeof (IFX_TAPI_IOCTL_t));
      /* Dangerous: Its not allowed to use pointer to ints instead of a structure
         on single dev node */
      if (tmpAdr.dev >= pDrvCtx->maxDevs)
      {
         TAPI_DEV *pTapiDev = pDrvCtx->pTapiDev;
         RETURN_DEVSTATUS (TAPI_statusParam, 0);
      }
      if (tmpAdr.ch == IFX_TAPI_EVENT_ALL_CHANNELS)
      {
         pChannel = &((pDrvCtx->pTapiDev[tmpAdr.dev]).
                     pTapiChanelArray[0]);
      }
      else
      {
         if (tmpAdr.ch >= pDrvCtx->maxChannels)
         {
            TAPI_DEV *pTapiDev = pDrvCtx->pTapiDev;
            RETURN_DEVSTATUS (TAPI_statusParam, 0);
         }
         pChannel = &((pDrvCtx->pTapiDev[tmpAdr.dev]).
                       pTapiChanelArray[tmpAdr.ch]);
      }
      /* copy the one parameter */
      param = tmpAdr.param;
   }
#endif /* TAPI_ONE_DEVNODE */

   IFXOS_MutexLock (pChannel->semTapiChSingleIoctlAccess);
   /* copy back one parameter if applicable. This is just temporarily till
      all applications and structures are ported. */
   bHandled = IFX_TRUE;
   /* simple calls with one or none parameter -> no copying from and to user
      space (linux) */
   switch (iocmd)
   {
      /* Operation Control Services */
      case IFX_TAPI_LINE_FEED_SET:
         ret = TAPI_Phone_Set_Linefeed(pChannel, param);
         break;
      /* Operation Control Services */
      case IFX_TAPI_LINE_HOOK_STATUS_GET:
#ifdef TAPI_ONE_DEVNODE
         if (pCtx->bSingleFd)
            ret = TAPI_Phone_Hookstate (pChannel,
                     (IFX_int32_t *) &tmpAdr.param);
         else
#endif /* TAPI_ONE_DEVNODE */
            ret = TAPI_Phone_Hookstate (pChannel, (IFX_int32_t *) ioarg);
         break;
      /* PCM Services */
      case IFX_TAPI_PCM_ACTIVATION_GET:
#ifdef TAPI_ONE_DEVNODE
         if (pCtx->bSingleFd)
            ret = TAPI_Phone_PCM_Get_Activation (pChannel,
                     (IFX_boolean_t*) &tmpAdr.param);
         else
#endif /* TAPI_ONE_DEVNODE */
            ret = TAPI_Phone_PCM_Get_Activation (pChannel,
                     (IFX_boolean_t*) ioarg);
         break;

      case IFX_TAPI_PCM_ACTIVATION_SET:
         ret = TAPI_Phone_PCM_Set_Activation(pChannel, param);
         break;

      case IFX_TAPI_PCM_DEC_HP_SET:
         if (ptr_chk(pDrvCtx->COD.DEC_HP_Set, "pDrvCtx->PCM.DEC_HP_Set"))
            retLL = pDrvCtx->PCM.DEC_HP_Set(pChannel->pLLChannel, (IFX_boolean_t)param);
         else
            ret = TAPI_statusLLNotSupp;
         break;

#ifdef TAPI_VOICE
      case IFX_TAPI_PKT_RTCP_STATISTICS_RESET:
         if (pDrvCtx->COD.RTCP_Reset == IFX_NULL)
            ret = TAPI_statusLLNotSupp;
         else
            retLL = pDrvCtx->COD.RTCP_Reset (pChannel->pLLChannel);
         break;

      case IFX_TAPI_PKT_FLUSH:
            ret = TAPI_ResetUpStreamFifo(pChannel);
         break;

      case IFX_TAPI_JB_STATISTICS_RESET:
         if (pDrvCtx->COD.JB_Stat_Reset == IFX_NULL)
            ret = TAPI_statusLLNotSupp;
         else
            retLL = pDrvCtx->COD.JB_Stat_Reset (pChannel->pLLChannel);
         break;

      /* Recording Services */
      case IFX_TAPI_ENC_START:
         if (pDrvCtx->COD.ENC_Start == IFX_NULL)
            ret = TAPI_statusLLNotSupp;
         else
            retLL = pDrvCtx->COD.ENC_Start (pChannel->pLLChannel);
         break;

      case IFX_TAPI_ENC_STOP:
         if (pDrvCtx->COD.ENC_Stop == IFX_NULL)
            ret = TAPI_statusLLNotSupp;
         else
            retLL = pDrvCtx->COD.ENC_Stop (pChannel->pLLChannel);
         break;

      case IFX_TAPI_ENC_HOLD:
         if (pDrvCtx->COD.ENC_Hold == IFX_NULL)
            ret = TAPI_statusLLNotSupp;
         else
            retLL = pDrvCtx->COD.ENC_Hold(pChannel->pLLChannel, param);
         break;


      case IFX_TAPI_ENC_TYPE_SET:
         if (pDrvCtx->COD.ENC_CoderType_Set == IFX_NULL)
            ret = TAPI_statusLLNotSupp;
         else
            retLL = pDrvCtx->COD.ENC_CoderType_Set (pChannel->pLLChannel,
                       param);
         break;

      case IFX_TAPI_ENC_FRAME_LEN_SET:
         if (pDrvCtx->COD.ENC_FrameLength_Set == IFX_NULL)
            ret = TAPI_statusLLNotSupp;
         else
            retLL = pDrvCtx->COD.ENC_FrameLength_Set (pChannel->pLLChannel,
                       param);
         break;

      case IFX_TAPI_ENC_VAD_CFG_SET:
         if (pDrvCtx->COD.VAD_Cfg == IFX_NULL)
            ret = TAPI_statusLLNotSupp;
         else
            retLL = pDrvCtx->COD.VAD_Cfg (pChannel->pLLChannel, param);
         break;

      case IFX_TAPI_DEC_START:
         if (pDrvCtx->COD.DEC_Start == IFX_NULL)
            ret = TAPI_statusLLNotSupp;
         else
            retLL = pDrvCtx->COD.DEC_Start (pChannel->pLLChannel);
         break;

      case IFX_TAPI_DEC_STOP:
         if (pDrvCtx->COD.DEC_Stop == IFX_NULL)
            ret = TAPI_statusLLNotSupp;
         else
            retLL = pDrvCtx->COD.DEC_Stop (pChannel->pLLChannel);
         break;

      case IFX_TAPI_COD_DEC_HP_SET:
         if (ptr_chk(pDrvCtx->COD.DEC_HP_Set, "pDrvCtx->COD.DEC_HP_Set"))
            retLL = pDrvCtx->COD.DEC_HP_Set(pChannel->pLLChannel,
                                            (IFX_boolean_t)param);
         else
            ret = TAPI_statusLLNotSupp;
         break;
#endif /* TAPI_VOICE */

      case IFX_TAPI_PHONE_ES_SET:
         if (ptr_chk(pDrvCtx->ALM.EchoSuppressor, "pDrvCtx->ALM.EchoSuppressor"))
            retLL = pDrvCtx->ALM.EchoSuppressor(pChannel->pLLChannel,
                                  (IFX_operation_t)param == IFX_ENABLE ? 1 : 0);
         else
            ret = TAPI_statusLLNotSupp;
         break;

      case IFX_TAPI_TEST_HOOKGEN:
         if (ptr_chk(pDrvCtx->ALM.TestHookGen, "ALM.TestHookGen"))
            retLL = pDrvCtx->ALM.TestHookGen (pChannel->pLLChannel,
                                              (IFX_boolean_t) param);
         else
            ret = TAPI_statusLLNotSupp;
         break;

      /* Ring services */
      case IFX_TAPI_RING_START:
         ret = IFX_TAPI_Ring_Start(pChannel);
         break;
      case IFX_TAPI_RING_STOP:
         ret = IFX_TAPI_Ring_Stop(pChannel);
         break;
      case IFX_TAPI_RING_CADENCE_SET:
         ret = IFX_TAPI_Ring_SetCadence(pChannel, param);
         break;
      case IFX_TAPI_RING_MAX_SET:
         ret = IFX_TAPI_Ring_SetMaxRings(pChannel, (IFX_uint32_t)param);
         break;
      case IFX_TAPI_RING:
         ret = IFX_TAPI_Ring_DoBlocking(pChannel);
         break;


      case IFX_TAPI_LINE_LEVEL_SET:
         if (pDrvCtx->ALM.Volume_High_Level == IFX_NULL)
            ret = TAPI_statusLLNotSupp;
         else
            retLL = pDrvCtx->ALM.Volume_High_Level (pChannel->pLLChannel,
                       param);
         break;
       /* Tone Services */
      case IFX_TAPI_TONE_STATUS_GET:
         if (pCtx->bSingleFd)
            ret = TAPI_Phone_Tone_Get_State (pChannel,
                     &tmpAdr.param);
         else
            ret = TAPI_Phone_Tone_Get_State (pChannel,
                     (IFX_uint32_t *)ioarg);
         break;
      case IFX_TAPI_TONE_DIALTONE_PLAY:
         ret = TAPI_Phone_Tone_Dial(pDrvCtx, pChannel);
         break;

      case IFX_TAPI_TONE_RINGBACK_PLAY:
         ret = TAPI_Phone_Tone_Ringback(pDrvCtx, pChannel);
         break;

      case IFX_TAPI_TONE_BUSY_PLAY:
         ret = TAPI_Phone_Tone_Busy(pDrvCtx, pChannel);
         break;

      case IFX_TAPI_TONE_LOCAL_PLAY:
         ret = TAPI_Phone_Tone_Play (pDrvCtx, pChannel,
                                     param, TAPI_TONE_DST_LOCAL);
         break;

      case IFX_TAPI_TONE_NET_PLAY:
         ret = TAPI_Phone_Tone_Play (pDrvCtx, pChannel,
                                     param, TAPI_TONE_DST_NET);
         break;

      case IFX_TAPI_TONE_LOCAL_STOP:
         ret = TAPI_Phone_Tone_Stop (pDrvCtx, pChannel,
                                     param, TAPI_TONE_DST_LOCAL);
         break;

      case IFX_TAPI_TONE_NET_STOP:
         ret = TAPI_Phone_Tone_Stop (pDrvCtx, pChannel,
                                     param, TAPI_TONE_DST_NET);
         break;

#ifdef DECT_SUPPORT
      case IFX_TAPI_TONE_DECT_PLAY:
         ret = TAPI_Phone_Tone_Play (pDrvCtx, pChannel,
                                     param | IFX_TAPI_TONE_SRC_DECT,
                                     TAPI_TONE_DST_LOCAL);
         break;

      case IFX_TAPI_TONE_DECT_STOP:
         ret = TAPI_DECT_Tone_Stop (pDrvCtx, pChannel);
         break;
#endif /* DECT_SUPPORT */

#ifdef TAPI_FAX_T38
      case IFX_TAPI_T38_STOP:
         if (pDrvCtx->COD.T38_Datapump_Disable == IFX_NULL)
            ret = TAPI_statusLLNotSupp;
         else
            retLL = pDrvCtx->COD.T38_Datapump_Disable (pChannel->pLLChannel);
         break;
#endif /* TAPI_FAX_T38 */

      case IFX_TAPI_PKT_EV_OOB_SET:
#ifdef TAPI_EXT_KEYPAD
         if(pChannel->nChannel > 0 )
         {
            ret = TAPI_statusInvalidCh;
         }
         else
         {
           /* Overwrite lower 7 bits of status byte with passed argument
              and ensure that local play flag remains unchanged. */
            pChannel->nDtmfInfo = (pChannel->nDtmfInfo & DTMF_EV_LOCAL_PLAY) |
                                   ((IFX_uint8_t)ioarg & ~DTMF_EV_LOCAL_PLAY );
         }
#else
         if (pDrvCtx->SIG.DTMFD_OOB == IFX_NULL)
            ret = TAPI_statusLLNotSupp;
         else
            retLL = pDrvCtx->SIG.DTMFD_OOB (pChannel->pLLChannel, param);
#endif
         break;

#ifdef ENABLE_OBSOLETE_BITFIELD_EVENT_INTERFACE
      case IFX_TAPI_EXCEPTION_GET:
         ret = TAPI_Phone_Exception(pChannel);
         break;
#endif /* ENABLE_OBSOLETE_BITFIELD_EVENT_INTERFACE */

      case IFX_TAPI_EXCEPTION_MASK:
         ret = TAPI_Phone_Mask_Exception(pChannel, param);
         break;

      case IFX_TAPI_METER_START:
         ret = TAPI_Phone_Meter_Start(pChannel);
         break;

      case IFX_TAPI_METER_STOP:
         ret = TAPI_Phone_Meter_Stop(pChannel);
         break;

#ifdef TAPI_CID
      /* Caller ID Reception service */
      case IFX_TAPI_CID_RX_START:
         ret = TAPI_Phone_CidRx_Start (pChannel,
                  (IFX_TAPI_CID_HOOK_MODE_t) param);
         break;

      case IFX_TAPI_CID_RX_STOP:
         ret = TAPI_Phone_CidRx_Stop (pChannel);
         break;

      case IFX_TAPI_CID_TX_INFO_STOP:
         if (ptr_chk(pDrvCtx->SIG.CID_TX_Stop, "pDrvCtx->SIG.CID_TX_Stop"))
            ret = TAPI_Phone_CID_Stop_Tx(pChannel);
         else
            ret = TAPI_statusLLNotSupp;
         break;
#endif /* TAPI_CID */

      case IFX_TAPI_TONE_CPTD_STOP:
         ret = TAPI_Phone_DetectToneStop(pDrvCtx, pChannel);
         break;

      case IFX_TAPI_ENC_AGC_ENABLE:
         if (pDrvCtx->COD.AGC_Enable == IFX_NULL)
            ret = TAPI_statusLLNotSupp;
         else
            retLL = pDrvCtx->COD.AGC_Enable(pChannel->pLLChannel, param);
         break;

      case IFX_TAPI_ENC_ROOM_NOISE_DETECT_STOP:
         if (ptr_chk(pDrvCtx->COD.ENC_RoomNoise, "pDrvCtx->COD.ENC_RoomNoise"))
            retLL = pDrvCtx->COD.ENC_RoomNoise(pChannel->pLLChannel,
                                               IFX_FALSE, 0, 0, 0);
         else
            ret = TAPI_statusLLNotSupp;
         break;

      /* Tone Services */
      case IFX_TAPI_TONE_ON_TIME_GET:
      case IFX_TAPI_TONE_OFF_TIME_GET:
      case IFX_TAPI_TONE_ON_TIME_SET:
      case IFX_TAPI_TONE_OFF_TIME_SET:
      /* Dial Services */
#if 0  /* ENABLE_OBSOLETE_BITFIELD_EVENT_INTERFACE */
      case IFX_TAPI_EXCEPTION_GET:
#endif /* ENABLE_OBSOLETE_BITFIELD_EVENT_INTERFACE */
         ret = TAPI_statusNotSupported;
      default:
         bHandled = IFX_FALSE;
         break;
   }
   if (GET_HL_ERROR(ret))
   {
      /* The HL error is already reported in the correct position. */
      IFXOS_MutexUnlock (pChannel->semTapiChSingleIoctlAccess);
      return ret;
   }
   if (!TAPI_SUCCESS(ret))
   {
      /* The errorcode needs to be built from HL and LL errorcode. */
      IFXOS_MutexUnlock (pChannel->semTapiChSingleIoctlAccess);
      RETURN_STATUS(ret, retLL);
   }
   if (!TAPI_SUCCESS(retLL))
   {
      /* Just the LL reported an error so add an generic HL errorcode. */
      IFXOS_MutexUnlock (pChannel->semTapiChSingleIoctlAccess);
      /* errmsg: LL driver returned an error */
      RETURN_STATUS (TAPI_statusLLFailed, retLL);
   }
   if (bHandled)
   {
      IFXOS_MutexUnlock (pChannel->semTapiChSingleIoctlAccess);
#ifdef TAPI_ONE_DEVNODE
      if (pCtx->bSingleFd)
      {
         /* using a new strucutre, get the context and pass again
            the one argument */
         IFXOS_CPY_KERN2USR ((char*)ioarg, (IFX_uint8_t*)&tmpAdr,
                              sizeof (IFX_TAPI_IOCTL_t));
      }
#endif /* TAPI_ONE_DEVNODE */
      return ret;
   }
   if (iocmd == IFX_TAPI_CID_RX_DATA_GET
#ifdef ENABLE_OBSOLETE_BITFIELD_EVENT_INTERFACE
      || iocmd == IFX_TAPI_CH_STATUS_GET
#endif /* ENABLE_OBSOLETE_BITFIELD_EVENT_INTERFACE */
      )
{
      /* this io commands are reading from fifo and access must
      be therefore protected against interrupts */
      if (pDrvCtx->IRQ.LockDevice != IFX_NULL)
         pDrvCtx->IRQ.LockDevice (pChannel->pTapiDevice->pLLDev);
   }
#ifdef LINUX
   ret = TAPI_OS_IoctlCh (pDrvCtx, pCtx, pChannel, iocmd, ioarg);
#else

   switch(iocmd)
   {
   /* Ringing Services */
      ON_IOCTLR1 (IFX_TAPI_RING_CFG_GET, IFX_TAPI_Ring_GetConfig, pChannel, IFX_TAPI_RING_CFG_t*);
/*
   case IFX_TAPI_RING_CFG_GET:
      return IFX_TAPI_Ring_GetConfig(pChannel, (IFX_TAPI_RING_CFG_t*)ioarg);
*/
      ON_IOCTLW2 (IFX_TAPI_RING_CFG_SET, IFX_TAPI_Ring_SetConfig, pChannel, IFX_TAPI_RING_CFG_t *);
/*
   case IFX_TAPI_RING_CFG_SET:
      return IFX_TAPI_Ring_SetConfig(pChannel, (IFX_TAPI_RING_CFG_t *) ioarg);
*/
      /* simple parameter without result */
      ON_IOCTLW2 (IFX_TAPI_RING_CADENCE_SET, IFX_TAPI_Ring_SetCadence, pChannel, IFX_uint32_t);
/*
   case IFX_TAPI_RING_CADENCE_SET:
      return (IFX_TAPI_Ring_SetCadence (pChannel, param));
*/
   case IFX_TAPI_RING_CADENCE_HR_SET:
      {
         IFX_TAPI_RING_CADENCE_t *pCadence = (IFX_TAPI_RING_CADENCE_t*) ioarg;
         ret = (IFX_TAPI_Ring_SetCadenceHighRes (pChannel, pCadence));
      }
      break;
   case IFX_TAPI_RING_START:
      /* no parameter without result */
      ret = IFX_TAPI_Ring_Start (pChannel);
      break;

   /* Operation Control Services */
   case IFX_TAPI_LINE_FEED_SET:
      /* simple parameter without result */
      ret = TAPI_Phone_Set_Linefeed(pChannel, param);
      break;

   case IFX_TAPI_LINE_TYPE_SET:
      ret = TAPI_Phone_Set_LineType(pDrvCtx, pChannel, (IFX_TAPI_LINE_TYPE_CFG_t *)ioarg);
      break;

   /* PCM Services */
   case IFX_TAPI_PCM_IF_CFG_SET:
      ret = TAPI_Phone_PCM_IF_Set_Config(pChannel->pTapiDevice,
                                        (IFX_TAPI_PCM_IF_CFG_t *) ioarg);
      break;

   case IFX_TAPI_PCM_CFG_SET:
      {
         IFX_TAPI_PCM_CFG_t *pPCMConfig = (IFX_TAPI_PCM_CFG_t*) ioarg;
         ret = (TAPI_Phone_PCM_Set_Config(pChannel, pPCMConfig));
      }
      break;
   case IFX_TAPI_PCM_CFG_GET:
      {
         IFX_TAPI_PCM_CFG_t *pPCMConfig = (IFX_TAPI_PCM_CFG_t*) ioarg;
         TAPI_Phone_PCM_Get_Config (pChannel, pPCMConfig);
         ret = (IFX_SUCCESS);
      }
      break;

   case IFX_TAPI_PCM_ACTIVATION_SET:
      TAPI_Phone_PCM_Set_Activation(pChannel, ioarg);
      ret = (IFX_SUCCESS);
      break;

   case IFX_TAPI_TONE_LEVEL_SET:
      {
         IFX_TAPI_PREDEF_TONE_LEVEL_t *pToneLevel = (IFX_TAPI_PREDEF_TONE_LEVEL_t *) ioarg;
         ret = TAPI_Phone_Tone_Set_Level(pDrvCtx, pChannel, pToneLevel);
      }
      break;

   case IFX_TAPI_TONE_DIALTONE_PLAY:
      ret = TAPI_Phone_Tone_Dial(pDrvCtx, pChannel);
      break;

   case IFX_TAPI_TONE_RINGBACK_PLAY:
      ret = TAPI_Phone_Tone_Ringback(pDrvCtx, pChannel);
      break;

   case IFX_TAPI_TONE_BUSY_PLAY:
      ret = TAPI_Phone_Tone_Busy(pDrvCtx, pChannel);
      break;

   case IFX_TAPI_TONE_STOP:
      ret = TAPI_Phone_Tone_Stop (pDrvCtx, pChannel,
               (IFX_int32_t) ioarg, TAPI_TONE_DST_DEFAULT);
      break;

   case IFX_TAPI_TONE_TABLE_CFG_SET:
      ret = TAPI_Phone_Tone_TableConf(((TAPI_DEV*)pChannel)->pToneTbl,
               (IFX_TAPI_TONE_t *)ioarg);
      break;

   /* Dial Services */
   case IFX_TAPI_LINE_HOOK_VT_SET:
      ret = (TAPI_Phone_Validation_Time(pChannel,
         (IFX_TAPI_LINE_HOOK_VT_t *)ioarg));
      break;

   case IFX_TAPI_PKT_EV_OOB_SET:
      ret = pDrvCtx->SIG.DTMFD_OOB (pChannel->pLLChannel, ioarg);
      break;

   /* Miscellaneous Services */
#ifdef ENABLE_OBSOLETE_BITFIELD_EVENT_INTERFACE
   case IFX_TAPI_EXCEPTION_GET:
      ret = (TAPI_Phone_Exception(pChannel));
      break;
#endif /* ENABLE_OBSOLETE_BITFIELD_EVENT_INTERFACE */

   case IFX_TAPI_EXCEPTION_MASK:
      TAPI_Phone_Mask_Exception(pChannel, ioarg);
      ret = IFX_SUCCESS;
      break;

   ON_IOCTLW3 (IFX_TAPI_CH_INIT, TAPI_Phone_Init, pDrvCtx, pChannel, IFX_TAPI_CH_INIT_t *);
/*
   case IFX_TAPI_CH_INIT:
      ret = TAPI_Phone_Init(pDrvCtx, pChannel, (IFX_TAPI_CH_INIT_t *)ioarg));
      break;
*/
   case IFX_TAPI_ENC_VAD_CFG_SET:
      /** \todo check if this is ok.
      ret = TAPI_LL_Phone_VAD(pDrvCtx, pChannel, ioarg);*/
      ret = pDrvCtx->COD.VAD_Cfg (pChannel->pLLChannel, ioarg);
      break;

#ifdef TAPI_VOICE
   case IFX_TAPI_ENC_FRAME_LEN_SET:
      ret = pDrvCtx->COD.ENC_FrameLength_Set (pChannel->pLLChannel, (IFX_int32_t) ioarg);
         /*(TAPI_LL_Phone_Set_Frame_Length(pDrvCtx, pChannel, ioarg));*/
      break;

   case IFX_TAPI_ENC_FRAME_LEN_GET:
      ret = pDrvCtx->COD.ENC_FrameLength_Get (pChannel->pLLChannel, (IFX_uint32_t*)ioarg);
      break;
#endif /* TAPI_VOICE */

   /* Metering Services */
   case IFX_TAPI_METER_CFG_SET:
   {
      IFX_TAPI_METER_CFG_t *pMeterConfig = (IFX_TAPI_METER_CFG_t *) ioarg;
      TAPI_Phone_Meter_Config(pChannel, pMeterConfig);
      ret = (IFX_SUCCESS);
      break;
   }

   /* Lec Configuration */
   case IFX_TAPI_LEC_PHONE_CFG_SET:
      ret = TAPI_Phone_LecConf_Alm (pChannel, (IFX_TAPI_LEC_CFG_t *)ioarg);
      break;
   case IFX_TAPI_LEC_PHONE_CFG_GET:
      ret = TAPI_Phone_GetLecConf_Alm (pChannel, (IFX_TAPI_LEC_CFG_t *)ioarg);
      break;
   case IFX_TAPI_LEC_PCM_CFG_SET:
      ret = TAPI_Phone_LecConf_Pcm (pChannel, (IFX_TAPI_LEC_CFG_t *)ioarg);
      break;
   case IFX_TAPI_LEC_PCM_CFG_GET:
      ret = TAPI_Phone_GetLecConf_Pcm (pChannel, (IFX_TAPI_LEC_CFG_t *)ioarg);
      break;
   case IFX_TAPI_WLEC_PHONE_CFG_SET:
      ret = TAPI_Phone_LecMode_Alm_Set (pChannel, (IFX_TAPI_WLEC_CFG_t *)ioarg);
      break;
   case IFX_TAPI_WLEC_PHONE_CFG_GET:
      ret = TAPI_Phone_LecMode_Alm_Get (pChannel, (IFX_TAPI_WLEC_CFG_t *)ioarg);
      break;
   case IFX_TAPI_WLEC_PCM_CFG_SET:
      ret = TAPI_Phone_LecMode_Pcm_Set (pDrvCtx, pChannel, (IFX_TAPI_WLEC_CFG_t *)ioarg);
      break;
   case IFX_TAPI_WLEC_PCM_CFG_GET:
      ret = TAPI_Phone_LecMode_Pcm_Get (pChannel, (IFX_TAPI_WLEC_CFG_t *)ioarg);
      break;
#ifdef TAPI_CID
   /* Caller ID Transmission service */
   case IFX_TAPI_CID_CFG_SET:
      ret = TAPI_Phone_CID_SetConfig (pChannel, (IFX_TAPI_CID_CFG_t *)ioarg);
      break;
   case IFX_TAPI_CID_TX_INFO_START:
      ret = TAPI_Phone_CID_Info_Tx (pChannel, (IFX_TAPI_CID_MSG_t *)ioarg);
      break;
   case IFX_TAPI_CID_TX_SEQ_START:
      ret = TAPI_Phone_CID_Seq_Tx (pChannel, (IFX_TAPI_CID_MSG_t *)ioarg);
      break;

   case IFX_TAPI_CID_TX_INFO_STOP:
      if (ptr_chk(pDrvCtx->SIG.CID_TX_Stop, "pDrvCtx->SIG.CID_TX_Stop"))
      {
         ret = TAPI_Phone_CID_Stop_Tx(pChannel);
      }
      else
      {
         TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
               ("TAPI: Abort CID sending not supported by LL driver\n\r"));
         ret = IFX_ERROR;
      }
      break;

   /* Caller ID Reception service */
   case IFX_TAPI_CID_RX_START:
      ret = (TAPI_Phone_CidRx_Start (pChannel, (IFX_TAPI_CID_HOOK_MODE_t) ioarg));
      break;
   case IFX_TAPI_CID_RX_STOP:
      ret = ( TAPI_Phone_CidRx_Stop (pChannel));
      break;
   case IFX_TAPI_CID_RX_STATUS_GET:
      ret = (TAPI_Phone_CidRx_Status (pChannel, (IFX_TAPI_CID_RX_STATUS_t *) ioarg));
      break;
   case IFX_TAPI_CID_RX_DATA_GET:
      ret = (TAPI_Phone_Get_CidRxData (pChannel, (IFX_TAPI_CID_RX_DATA_t *) ioarg));
      break;
#endif /* TAPI_CID */
#ifdef TAPI_VOICE
   /* Connection Services*/
   case IFX_TAPI_MAP_DATA_ADD:
      ret = TAPI_Data_Channel_Add (pChannel, (IFX_TAPI_MAP_DATA_t *)ioarg);
      break;
   case IFX_TAPI_MAP_DATA_REMOVE:
      ret = TAPI_Data_Channel_Remove (pChannel, (IFX_TAPI_MAP_DATA_t *)ioarg);
      break;
   case IFX_TAPI_MAP_PHONE_ADD:
      ret = TAPI_Phone_Channel_Add (pChannel, (IFX_TAPI_MAP_PHONE_t *)ioarg);
      break;
   case IFX_TAPI_MAP_PHONE_REMOVE:
      ret = TAPI_Phone_Channel_Remove (pChannel, (IFX_TAPI_MAP_PHONE_t *)ioarg);
      break;
   case IFX_TAPI_MAP_PCM_ADD:
      ret = pDrvCtx->CON.PCM_Channel_Add (pChannel->pLLChannel, (IFX_TAPI_MAP_PCM_t *)ioarg);
      break;
   case IFX_TAPI_MAP_PCM_REMOVE:
      ret = pDrvCtx->CON.PCM_Channel_Remove (pChannel->pLLChannel, (IFX_TAPI_MAP_PCM_t *)ioarg);
      break;
   case IFX_TAPI_PKT_RTP_CFG_SET:
      ret = pDrvCtx->COD.RTP_Cfg (pChannel->pLLChannel, (IFX_TAPI_PKT_RTP_CFG_t*)ioarg);
      break;
   case IFX_TAPI_PKT_RTP_PT_CFG_SET:
      ret = pDrvCtx->COD.RTP_PayloadTable_Cfg (pChannel->pLLChannel, (IFX_TAPI_PKT_RTP_PT_CFG_t *)ioarg);
      break;
   case IFX_TAPI_PKT_RTCP_STATISTICS_GET:
      ret = pDrvCtx->COD.RTCP_Get (pChannel->pLLChannel, (IFX_TAPI_PKT_RTCP_STATISTICS_t *)ioarg);
      break;
   case IFX_TAPI_PKT_RTCP_STATISTICS_RESET:
      ret = pDrvCtx->COD.RTCP_Reset (pChannel->pLLChannel);
      break;
   case IFX_TAPI_JB_CFG_SET:
      ret = pDrvCtx->COD.JB_Cfg (pChannel->pLLChannel, (IFX_TAPI_JB_CFG_t *)ioarg);
      break;
   case IFX_TAPI_JB_STATISTICS_GET:
      ret = pDrvCtx->COD.JB_Stat_Get (pChannel->pLLChannel, (IFX_TAPI_JB_STATISTICS_t *)ioarg);
      break;
   case IFX_TAPI_JB_STATISTICS_RESET:
      ret = pDrvCtx->COD.JB_Stat_Reset (pChannel->pLLChannel);
      break;
#if (VIN_CFG_FEATURES & VIN_FEAT_PACKET_AAL)
   case IFX_TAPI_PKT_AAL_CFG_SET:
      ret = TAPI_LL_Phone_Aal_Set (pDrvCtx, pChannel, (IFX_TAPI_PCK_AAL_CFG_t *)ioarg);
      break;
   case IFX_TAPI_PKT_AAL_PROFILE_SET:
      ret = TAPI_LL_Phone_AalProfile (pDrvCtx, pChannel, (IFX_TAPI_PCK_AAL_PROFILE_t *)ioarg);
      break;
#endif /* (VIN_CFG_FEATURES & VIN_FEAT_PACKET_AAL) */
   /* Recording services */
   case IFX_TAPI_ENC_START:
      ret = pDrvCtx->COD.ENC_Start (pChannel->pLLChannel);
      break;
   case IFX_TAPI_ENC_STOP:
      ret = pDrvCtx->COD.ENC_Stop (pChannel->pLLChannel);
      break;
   case IFX_TAPI_ENC_HOLD:
   {
      if (pDrvCtx->COD.ENC_Hold == IFX_NULL)
      {
         TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
               ("TAPI:  pDrvCtx->COD.ENC_Hold is NULL !!!\n\r"));
         ret = IFX_ERROR;
      }
      else
      {
         ret = pDrvCtx->COD.ENC_Hold(pChannel->pLLChannel,
                                     (IFX_int32_t) ioarg);
      }
      break;
   }
   case IFX_TAPI_ENC_TYPE_SET:
      ret = pDrvCtx->COD.ENC_CoderType_Set (pChannel->pLLChannel, ioarg);
      break;
   /* Play Services*/
   /* not used */
   /*
   case IFX_TAPI_DEC_TYPE_SET:
      ret =(TAPI_Phone_Set_Play_Codec(pDrvCtx, pChannel, ioarg));
      break;
   */
   case IFX_TAPI_DEC_START:
      ret = pDrvCtx->COD.DEC_Start (pChannel->pLLChannel);
      break;
   case IFX_TAPI_DEC_STOP:
      ret = pDrvCtx->COD.DEC_Stop (pChannel->pLLChannel);
      break;
#endif /* TAPI_VOICE */
#ifdef TAPI_FAX_T38
   /* Fax Services */
   case IFX_TAPI_T38_MOD_START:
      ret = TAPI_LL_FaxT38_SetModulator (pDrvCtx, pChannel, (IFX_TAPI_T38_MOD_DATA_t *)ioarg);
      break;
   case IFX_TAPI_T38_DEMOD_START:
      ret = TAPI_LL_FaxT38_SetDemodulator (pDrvCtx, pChannel, (IFX_TAPI_T38_DEMOD_DATA_t *)ioarg);
      break;
   case IFX_TAPI_T38_STOP:
      ret = TAPI_LL_FaxT38_DisableDataPump (pChannel);
      break;
   case IFX_TAPI_T38_STATUS_GET:
      ret = TAPI_LL_FaxT38_GetStatus (pDrvCtx, pChannel, (IFX_TAPI_T38_STATUS_t *)ioarg);
      break;
#endif /* TAPI_FAX_T38 */
   /* Report Level */
#ifdef ENABLE_OBSOLETE_BITFIELD_EVENT_INTERFACE
   case IFX_TAPI_CH_STATUS_GET:
      ret = TAPI_Phone_GetStatus (pChannel, (IFX_TAPI_CH_STATUS_t *)ioarg);
      break;
#endif /* ENABLE_OBSOLETE_BITFIELD_EVENT_INTERFACE */


   case IFX_TAPI_PHONE_VOLUME_SET:
      ret = pDrvCtx->ALM.Volume_Set (pChannel->pLLChannel, (IFX_TAPI_LINE_VOLUME_t *)ioarg);
      break;

   case IFX_TAPI_PCM_VOLUME_SET:
      ret = pDrvCtx->PCM.Volume_Set (pChannel->pLLChannel, (IFX_TAPI_LINE_VOLUME_t *)ioarg);
      break;

   case IFX_TAPI_SIG_DETECT_ENABLE:
      if (pDrvCtx->SIG.MFTD_Enable != IFX_NULL)
         ret = pDrvCtx->SIG.MFTD_Enable (pChannel->pLLChannel,
                                         (IFX_TAPI_SIG_DETECTION_t *)ioarg);
      else
         ret = IFX_ERROR;
      break;
   case IFX_TAPI_SIG_DETECT_DISABLE:
      if (pDrvCtx->SIG.MFTD_Disable != IFX_NULL)
         ret = pDrvCtx->SIG.MFTD_Disable (pChannel->pLLChannel,
                                          (IFX_TAPI_SIG_DETECTION_t *)ioarg);
      else
         ret = IFX_ERROR;
      break;
   case IFX_TAPI_TONE_CPTD_START:
      ret = TAPI_Phone_DetectToneStart(pDrvCtx, pChannel, (IFX_TAPI_TONE_CPTD_t*) ioarg);
   break;
   case IFX_TAPI_TONE_CPTD_STOP:
      ret = TAPI_Phone_DetectToneStop(pDrvCtx, pChannel);
      break;

   ON_IOCTL_LLFUNC (IFX_TAPI_ENC_AGC_CFG, pDrvCtx->COD.AGC_Cfg,
                    ((IFX_TAPI_ENC_AGC_CFG_t *) ioarg));

/*
   case IFX_TAPI_ENC_AGC_CFG:
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
         ret = pDrvCtx->COD.AGC_Cfg(pChannel->pLLChannel,
                                    (IFX_TAPI_ENC_AGC_CFG_t *) ioarg);
      }

      break;
   }
*/
   case IFX_TAPI_ENC_AGC_ENABLE:
   {
      if (pDrvCtx->COD.AGC_Enable == IFX_NULL)
      {
         TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
               ("TAPI: pDrvCtx->COD.AGC_Enable is NULL. "
               "(File: %s, line: %d)\n", __FILE__, __LINE__));

         ret = IFX_ERROR;
      }
      else
      {
         ret = pDrvCtx->COD.AGC_Enable(pChannel->pLLChannel,
                                       (IFX_TAPI_ENC_AGC_MODE_t) ioarg);
      }
      break;
   }

   case IFX_TAPI_ENC_ROOM_NOISE_DETECT_START:
      if (ptr_chk(pDrvCtx->COD.ENC_RoomNoise, "pDrvCtx->COD.ENC_RoomNoise"))
      {
         ret = pDrvCtx->COD.ENC_RoomNoise(pChannel->pLLChannel, IFX_TRUE,
                  ((IFX_TAPI_ENC_ROOM_NOISE_DETECT_t *)ioarg)->nThreshold,
                  ((IFX_TAPI_ENC_ROOM_NOISE_DETECT_t *)ioarg)->nVoicePktCnt,
                  ((IFX_TAPI_ENC_ROOM_NOISE_DETECT_t *)ioarg)->nSilencePktCnt);
      }
      else
      {
         TRACE(TAPI_DRV,DBG_LEVEL_HIGH,
               ("TAPI: Room-Noise feature not supported by LL driver\n\r"));
         ret = IFX_ERROR;
      }
      break;

   case IFX_TAPI_ENC_ROOM_NOISE_DETECT_STOP:
      if (ptr_chk(pDrvCtx->COD.ENC_RoomNoise, "pDrvCtx->COD.ENC_RoomNoise"))
      {
         ret = pDrvCtx->COD.ENC_RoomNoise(pChannel->pLLChannel,
                                          IFX_FALSE, 0, 0, 0);
      }
      else
      {
         TRACE(TAPI_DRV,DBG_LEVEL_HIGH,
               ("TAPI: Room-Noise feature not supported by LL driver\n\r"));
         ret = IFX_ERROR;
      }
      break;
   /* TEST services */
   ON_IOCTL_LLFUNC (IFX_TAPI_TEST_LOOP, pDrvCtx->ALM.TestLoop,
                    ((IFX_TAPI_TEST_LOOP_t *) ioarg));

   default:
      TRACE(TAPI_DRV,DBG_LEVEL_HIGH,("TAPI: unknown ioctl 0x%x\n",iocmd));
      ret = TAPI_statusInvalidIoctl;
   }
#endif /* LINUX */

   if (iocmd == IFX_TAPI_CID_RX_DATA_GET
#ifdef ENABLE_OBSOLETE_BITFIELD_EVENT_INTERFACE
      || iocmd == IFX_TAPI_CH_STATUS_GET
#endif /* ENABLE_OBSOLETE_BITFIELD_EVENT_INTERFACE */
      )
   {
      /* this io commands are reading from fifo and access must
      be therefore protected against interrupts */
      if (pDrvCtx->IRQ.UnlockDevice != IFX_NULL)
         pDrvCtx->IRQ.UnlockDevice (pChannel->pTapiDevice->pLLDev);
   }
   IFXOS_MutexUnlock (pChannel->semTapiChSingleIoctlAccess);

   return ret;
}

#ifdef TAPI_AUDIO_CHANNEL
/**
   Tapi Channel audio Specific ioctl handling

\param pDrvCtx handle to device driver context
\param pCtx    handle to the ioctl context structure
\param cmd     ioctl command
\param ioarg   ioctl argument

\return
   IFX_SUCCESS or IFX_ERROR
*/
static IFX_int32_t TAPI_IoctlAudioCh (IFX_TAPI_DRV_CTX_t*  pDrvCtx,
                                      IFX_TAPI_ioctlCtx_t* pCtx,
                                      IFX_uint32_t iocmd,
                                      IFX_uint32_t ioarg)
{
   TAPI_CHANNEL* pTapiCh  = pCtx->p.pTapiCh;
   TAPI_DEV*     pTapiDev = pTapiCh->pTapiDevice;
   IFX_int32_t    ret     = IFX_SUCCESS;

   /* Audio chnl Services */
   switch (iocmd)
   {
      case IFX_TAPI_AUDIO_AFE_CFG_SET :
      {
         if (pDrvCtx->AUDIO.AFE_Cfg_Set == IFX_NULL)
            RETURN_DEVSTATUS (TAPI_statusLLNotSupp, 0);
         ret = pDrvCtx->AUDIO.AFE_Cfg_Set (pTapiCh->pLLChannel,
                           (IFX_TAPI_AUDIO_AFE_CFG_SET_t*) ioarg);
         break;
      }
      case IFX_TAPI_AUDIO_VOLUME_SET :
      {
         if (pDrvCtx->AUDIO.Volume_Set == IFX_NULL)
            RETURN_DEVSTATUS (TAPI_statusLLNotSupp, 0);
         ret = pDrvCtx->AUDIO.Volume_Set(pTapiCh->pLLChannel, ioarg);
         break;
      }
      case IFX_TAPI_AUDIO_ROOM_TYPE_SET :
      {
         if (pDrvCtx->AUDIO.Room_Set == IFX_NULL)
            RETURN_DEVSTATUS (TAPI_statusLLNotSupp, 0);
         ret = pDrvCtx->AUDIO.Room_Set (pTapiCh->pLLChannel, ioarg);
         break;
      }
      case IFX_TAPI_AUDIO_MUTE_SET :
      {
         if (pDrvCtx->AUDIO.Mute_Set == IFX_NULL)
            RETURN_DEVSTATUS (TAPI_statusLLNotSupp, 0);
         ret = pDrvCtx->AUDIO.Mute_Set (pTapiCh->pLLChannel, ioarg);
         break;
      }
      case IFX_TAPI_AUDIO_MODE_SET :
      {
         if (pDrvCtx->AUDIO.Mode_Set == IFX_NULL)
            RETURN_DEVSTATUS (TAPI_statusLLNotSupp, 0);
         ret = pDrvCtx->AUDIO.Mode_Set (pTapiCh->pLLChannel, ioarg);
         break;
      }
      case IFX_TAPI_AUDIO_RING_START :
      {
         break;
      }
      case IFX_TAPI_AUDIO_RING_STOP :
      {
         if (pDrvCtx->AUDIO.Ring_Stop == IFX_NULL)
            RETURN_DEVSTATUS (TAPI_statusLLNotSupp, 0);
         ret = pDrvCtx->AUDIO.Ring_Stop (pTapiCh->pLLChannel, ioarg);
         break;
      }
      case IFX_TAPI_AUDIO_RING_VOLUME_SET :
      {
         if (pDrvCtx->AUDIO.Ring_Volume_Set == IFX_NULL)
            RETURN_DEVSTATUS (TAPI_statusLLNotSupp, 0);
         ret = pDrvCtx->AUDIO.Ring_Volume_Set (pTapiCh->pLLChannel, ioarg);
         break;
      }
      case IFX_TAPI_AUDIO_ICA_SET :
      {
         if (pDrvCtx->AUDIO.Incall_Anouncement == IFX_NULL)
            RETURN_DEVSTATUS (TAPI_statusLLNotSupp, 0);
         ret = pDrvCtx->AUDIO.Incall_Anouncement (pTapiCh->pLLChannel, ioarg);
         break;
      }
      case IFX_TAPI_AUDIO_TEST_SET:
      {
         if (pDrvCtx->AUDIO.Test_Mode_Set == IFX_NULL)
           RETURN_DEVSTATUS (TAPI_statusLLNotSupp, 0);
         ret = pDrvCtx->AUDIO.Test_Mode_Set (pTapiCh->pLLChannel, ioarg);
         break;
      }
   }
}
#endif /* TAPI_AUDIO_CHANNEL */


/**
   Tapi Device Specific ioctl handling

\param pDrvCtx handle to device driver context
\param pCtx    handle to the ioctl context structure
\param cmd     ioctl command
\param ioarg   ioctl argument

\return
   IFX_SUCCESS or IFX_ERROR
*/
static IFX_int32_t TAPI_IoctlDev (IFX_TAPI_DRV_CTX_t* pDrvCtx,
                                  IFX_TAPI_ioctlCtx_t* pCtx,
                                  IFX_uint32_t cmd, IFX_uint32_t ioarg)
{
   IFX_int32_t ret = IFX_SUCCESS;
   TAPI_DEV* pTapiDev = pCtx->p.pTapiDev;

   switch(cmd)
   {
      case IFX_TAPI_EVENT_GET:
#ifdef LINUX
         {
            IFX_TAPI_EVENT_t *p_tmp = IFX_NULL;

            if (ioarg == 0)
            {
               ret =IFX_ERROR;
               break;
            }
            p_tmp = IFXOS_MALLOC(sizeof(IFX_TAPI_EVENT_t));
            if (p_tmp == IFX_NULL)
            {
               ret = IFX_ERROR;
               break;
            }
            if (copy_from_user(p_tmp, (IFX_void_t *) ioarg, sizeof(IFX_TAPI_EVENT_t)) > 0)
            {
               TRACE(TAPI_DRV,DBG_LEVEL_HIGH,("TAPI: ioctl parameter error\n\r"));
               ret = IFX_ERROR;
            }
            if (ret == IFX_SUCCESS)
            {
               ret = TAPI_Phone_GetEvent(pTapiDev, p_tmp);
               if (copy_to_user ((IFX_void_t *)ioarg, (IFX_void_t *) p_tmp, sizeof(IFX_TAPI_EVENT_t))> 0)
               {
                  TRACE(TAPI_DRV,DBG_LEVEL_HIGH,
                       ("TAPI: cannot copy to user space\n\r"));
                  ret = IFX_ERROR;
               }
            }
            IFXOS_FREE (p_tmp);
         }
#else
         ret = TAPI_Phone_GetEvent(pTapiDev, (IFX_TAPI_EVENT_t*)ioarg);
#endif /* LINUX */
         break;

      /* Miscellaneous Services */
      case IFX_TAPI_VERSION_GET:
#ifdef LINUX
         {
            IFX_char_t *p_tmp = (IFX_char_t *)IFXOS_MALLOC(80);

            ret = TAPI_Phone_Get_Version(p_tmp);
            /* check for ret (string length) > 0 */
            if( ret > 0 )
            {
               if( (ret=copy_to_user((IFX_char_t *)ioarg, p_tmp, 80)) != 0)
               {
                  TRACE(TAPI_DRV,DBG_LEVEL_HIGH,
                        ("TAPI: cannot copy to user space\n\r"));
                  ret = IFX_ERROR;
               }
               else
               {
                  ret = IFX_SUCCESS;
               }
            }
            else
            {
               ret = IFX_ERROR;
            }
            IFXOS_FREE (p_tmp);
         }
#else
         ret = TAPI_Phone_Get_Version((IFX_char_t*)ioarg);
#endif /* LINUX */
         break;

      case IFX_TAPI_VERSION_CHECK:
         ret = TAPI_Phone_Check_Version ((IFX_TAPI_VERSION_t *)ioarg);
         break;

      case IFX_TAPI_CAP_NR:
         if (pDrvCtx->CAP_Number_Get == IFX_NULL)
         {
            RETURN_DEVSTATUS (TAPI_statusLLNotSupp, 0);
         }
         *((int*)ioarg) = pDrvCtx->CAP_Number_Get (pTapiDev->pLLDev);
         break;

      case IFX_TAPI_CAP_LIST:
         if (pDrvCtx->CAP_List_Get == IFX_NULL)
         {
            RETURN_DEVSTATUS (TAPI_statusLLNotSupp, 0);
         }
#ifdef LINUX
         {
            IFX_TAPI_CAP_t *p_tmp = IFX_NULL;
            IFX_uint32_t   tmpcapsize;

            if (pDrvCtx->CAP_Number_Get == IFX_NULL)
            {
               RETURN_DEVSTATUS (TAPI_statusLLNotSupp, 0);
            }
            else
            {
               tmpcapsize = pDrvCtx->CAP_Number_Get (pTapiDev->pLLDev);
               p_tmp = IFXOS_MALLOC(tmpcapsize * sizeof(IFX_TAPI_CAP_t));
               if (p_tmp == IFX_NULL)
               {
                  return IFX_ERROR;
               }

               ret = pDrvCtx->CAP_List_Get (pTapiDev->pLLDev, p_tmp);
               if ((ret == IFX_SUCCESS) &&
                   (copy_to_user((void*)ioarg, p_tmp,
                                 sizeof(IFX_TAPI_CAP_t) * tmpcapsize) > 0))
               {
                  ret = TAPI_statusErrKernCpy;
               }
               IFXOS_FREE(p_tmp);
            }
         }
#else
         ret = pDrvCtx->CAP_List_Get (pTapiDev->pLLDev, (IFX_TAPI_CAP_t *)ioarg);
#endif /* LINUX */
         break;

      case IFX_TAPI_CAP_CHECK:
         if (pDrvCtx->CAP_Check == IFX_NULL)
         {
            RETURN_DEVSTATUS (TAPI_statusLLNotSupp, 0);
         }
#ifdef LINUX
         {
            IFX_TAPI_CAP_t *p_tmp;
            p_tmp = (IFX_TAPI_CAP_t *) IFXOS_MALLOC (sizeof(IFX_TAPI_CAP_t));

            if (copy_from_user (p_tmp, (void *)ioarg,
                                sizeof(IFX_TAPI_CAP_t)) > 0 )
            {
               ret = TAPI_statusErrKernCpy;
            }
            else
            {
               ret = pDrvCtx->CAP_Check (pTapiDev->pLLDev, p_tmp);

               if (ret == IFX_SUCCESS || ret == 1)
               {
                  copy_to_user ((void*)ioarg, p_tmp, sizeof(IFX_TAPI_CAP_t));
               }
            }
            IFXOS_FREE (p_tmp);
         }
#else
         ret = pDrvCtx->CAP_Check (pTapiDev->pLLDev, (IFX_TAPI_CAP_t*)ioarg);
#endif /* LINUX */
         break;

      case IFX_TAPI_TONE_TABLE_CFG_SET:
         {
            IFX_TAPI_TONE_t *p_tmp;
#ifdef LINUX
            IFX_TAPI_TONE_t tone;

            p_tmp = &tone;
            if (copy_from_user (p_tmp, (IFX_TAPI_TONE_t*)ioarg,
                                sizeof(IFX_TAPI_TONE_t)) > 0 )
            {
               return IFX_ERROR;
            }
            else
#else
            {
               p_tmp = (IFX_TAPI_TONE_t *) ioarg;
            }
#endif /* LINUX */
            {
               ret = TAPI_Phone_Tone_TableConf (pTapiDev->pToneTbl, p_tmp);
            }
         }
         break;

      case IFX_TAPI_LASTERR:
         IFXOS_CPY_KERN2USR ((void*)ioarg, &(pTapiDev->error),
                             sizeof(IFX_TAPI_Error_t));
         pCtx->p.pTapiDev->error.nCnt = 0;
         pCtx->p.pTapiDev->error.nCode = 0;
         break;

      case IFX_TAPI_DEBUG_REPORT_SET:
         SetTraceLevel(TAPI_DRV, ioarg);
         break;


      /* PCM Services */
      case IFX_TAPI_PCM_IF_CFG_SET:
         {
            IFX_TAPI_PCM_IF_CFG_t *p_tmp;
#ifdef LINUX
            p_tmp = (IFX_TAPI_PCM_IF_CFG_t *)
               IFXOS_MALLOC(sizeof(IFX_TAPI_PCM_IF_CFG_t));

            if (IFXOS_CPY_USR2KERN (p_tmp, ioarg,
                                    sizeof(IFX_TAPI_PCM_IF_CFG_t)) == 0)
            {
               TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
                    ("TAPI: cannot copy from user space\n\r"));
               ret = IFX_ERROR;
            }
            else
#else
            {
               p_tmp = (IFX_TAPI_PCM_IF_CFG_t *)ioarg;
            }
#endif /* LINUX */
            {
               ret = TAPI_Phone_PCM_IF_Set_Config(pTapiDev, p_tmp);
            }
         }
         break;


      case IFX_TAPI_EVENT_ENABLE:
      case IFX_TAPI_EVENT_DISABLE:
         {
            /* Enables or disables a event on a given channel. The parameter
               specifies which channel and also the details of the event.
               The command can only be sent on a device file descriptor to make
               clear that the channel used is the one given in the parameter. */
            IFX_TAPI_EVENT_t *pEvent;

#ifdef LINUX
            pEvent = (IFX_TAPI_EVENT_t *) IFXOS_MALLOC(sizeof(IFX_TAPI_EVENT_t));

            if (IFXOS_CPY_USR2KERN (pEvent, ioarg, sizeof(IFX_TAPI_EVENT_t)) == 0)
            {
               TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
                    ("TAPI: cannot copy from user space\n\r"));
               ret = IFX_ERROR;
               break;
            }
            else
#else
            {
               pEvent = (IFX_TAPI_EVENT_t *)ioarg;
            }
#endif /* LINUX */

            /* Make sure the given channel parameter is within limits.
               The channel is passed in the struct of the parameter and needs to
               be verified to prevent out of range access of the channel array. */
            if ((pEvent->ch > pTapiDev->nMaxChannel) &&
                (pEvent->ch != IFX_TAPI_EVENT_ALL_CHANNELS))
            {
               TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
                    ("TAPI: IFX_TAPI_EVENT_EN-/DISABLE called with channel "
                     "parameter out of range\n\r"));
               ret = IFX_ERROR;
               break;
            }
            /* call function to set event reporting mask according to the ioctl
               on either one or all channels of a device */
            if (pEvent->ch != IFX_TAPI_EVENT_ALL_CHANNELS)
            {
               /* only one specific channel is addressed */
               ret = TAPI_Phone_Set_Event_Disable(
                        (pTapiDev->pTapiChanelArray + pEvent->ch),
                        pEvent,
                        (cmd == IFX_TAPI_EVENT_ENABLE) ?
                                          IFX_EVENT_ENABLE : IFX_EVENT_DISABLE);
            }
            else
            {
               /* all channels of the device are addressed */
               IFX_uint8_t i;

               for (i = 0; (ret == IFX_SUCCESS) &&
                           (i < pTapiDev->nMaxChannel); i++)
               {
                  ret = TAPI_Phone_Set_Event_Disable(
                           (pTapiDev->pTapiChanelArray + i), pEvent,
                           (cmd == IFX_TAPI_EVENT_ENABLE) ?
                                          IFX_EVENT_ENABLE : IFX_EVENT_DISABLE);
               }
            }
#ifdef LINUX
            IFXOS_FREE(pEvent);
#endif /* LINUX */
         }
         break;

#ifdef TAPI_POLL
         case IFX_TAPI_POLL_CONFIG:
         {
            IFX_TAPI_POLL_CONFIG_t* pTapiPollCfg = IFX_NULL;
#ifdef LINUX
             ret = IFX_ERROR;
#else
            {
                pTapiPollCfg = (IFX_TAPI_POLL_CONFIG_t *)ioarg;
            }
#endif /* LINUX */
            if (pTapiPollCfg != IFX_NULL)
            {
               ret = TAPI_IrqPollConf(pTapiPollCfg);
            }
            else
            {
               ret = IFX_ERROR;
            }
            break;
         }

         case IFX_TAPI_POLL_DEV_ADD:
         {
             ret = TAPI_AddDev_PollPkts(pDrvCtx, pTapiDev);
             if (ret != IFX_ERROR)
             {
                ret = TAPI_AddDev_PollEvts(pDrvCtx, pTapiDev);
             }
             break;
         }

         case IFX_TAPI_POLL_DEV_REM:
         {
            ret = TAPI_RemDev_PollPkts(pDrvCtx, pTapiDev);
            if (ret != IFX_ERROR)
            {
               ret = TAPI_RemDev_PollEvts(pDrvCtx, pTapiDev);
            }
            break;
         }

         /* Test TAPI DEVICE list handling with simulation. */
         case IFX_TAPI_POLL_TEST:
         {
            ret = TAPI_Poll_Test();
            break;
         }

#if 0
         case IFX_TAPI_POLL_WRITE:
         {
            IFX_void_t* pPktsArray[IFX_TAPI_POLL_QUEUE];
            IFX_void_t** ppPkts;
            IFX_int32_t pPktsNum = pPollDown->pPktsNum;
            IFX_TAPI_POLL_DATA_t *pPollDown = (IFX_TAPI_POLL_DATA_t *) parg;
            IFX_int32_t i = 0;


            ppPkts = pPollDown->ppPkts;
            for (i = 0; i < pPktsNum; i++)
            {
               /* skip any NULL pointers in the array */
               while (*ppPkts == IFX_NULL)
               {
                  *ppPkts++;

                  pPkt = *ppPkts++;

                  if (copy_from_user (pPollDown, parg, sizeof(IFX_TAPI_POLL_DATA_t)) > 0 )
                  {
                     ret = IFX_ERROR;
                  }
               }
               ret = TAPI_Poll_Down (pPollDown->ppPkts, &pPollDown->pPktsNum);
               break;
            }
         }

         case IFX_TAPI_POLL_READ:
            /* to be completed... */
            ret = TAPI_Poll_Up (pPollDown->ppPkts, &pPollDown->pPktsNum);
         break;
#endif

#endif /* TAPI_POLL */

   default:
      RETURN_DEVSTATUS (TAPI_statusNoIoctl, 0);
   }

   return ret;
}
