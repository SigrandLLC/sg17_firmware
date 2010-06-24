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
   \file drv_tapi_event.c
   Contains TAPI Event Handling.
*/

/* ============================= */
/* Includes                      */
/* ============================= */

#include "drv_api.h"
#include "drv_tapi.h"
#include "drv_tapi_errno.h"
#include "drv_tapi_ll_interface.h"
#include "drv_tapi_event.h"
#ifdef TAPI_CID
#include "drv_tapi_cid.h"
#endif /* TAPI_CID */


/* ============================= */
/* Local Macros  Definitions    */
/* ============================= */

/* ============================= */
/* Global variable definition    */
/* ============================= */

/* ============================= */
/* Global functions declaration  */
/* ============================= */

extern IFX_void_t TAPI_Tone_Step_Completed(TAPI_CHANNEL* pChannel,
                                           IFX_uint8_t utgNum);
extern IFX_return_t TAPI_DeferWork (IFX_void_t* pFunc, IFX_void_t* pParam);

extern IFX_int32_t IFX_TAPI_Event_Dispatch_ProcessCtx(IFX_TAPI_EXT_EVENT_PARAM_t *pParam);

#ifdef TAPI_EXT_KEYPAD
IFX_return_t TAPI_EXT_EVENT_Key_Handler (TAPI_CHANNEL * pChannel,
                                         IFX_TAPI_EVENT_t * pTapiEvent);
extern TAPI_CHANNEL *TAPI_Get_Channel_Ctx ();
#endif /* TAPI_EXT_KEYPAD */

/* ============================= */
/* Local variable definition     */
/* ============================= */

/** Memory pool for event dispatcher (global for all channels) */
static BUFFERPOOL *pIFX_TAPI_BP_Event = IFX_NULL;
static BUFFERPOOL *pIFX_TAPI_BP_Deferred_Event = IFX_NULL;
/* Buffer pool access protection */
static IFXOS_mutex_t semBufferPoolAcc;


/* ============================= */
/* Local function declaration    */
/* ============================= */

#ifdef ENABLE_OBSOLETE_BITFIELD_EVENT_INTERFACE
static IFX_return_t Signal_CompEvent (TAPI_CHANNEL *pChannel,
                                      IFX_TAPI_EVENT_t *pEvent);
static IFX_int32_t GetChStatus (TAPI_CHANNEL * pChannel,
                                IFX_TAPI_CH_STATUS_t * status);
static IFX_int32_t TAPI_Signal_Event_Update (TAPI_CHANNEL *pChannel,
                                             IFX_uint32_t signal,
                                             IFX_uint32_t signal_ext);
#endif /* ENABLE_OBSOLETE_BITFIELD_EVENT_INTERFACE */
static IFX_boolean_t Check_FaxModem_Status (TAPI_CHANNEL *pChannel,
                                               IFX_TAPI_EVENT_t *pEvent);

static IFX_void_t TAPI_WakeUp (TAPI_DEV *pTapiDev);
static void stripPathCpy (char* dst, const char* src);

/* ============================= */
/* Local function definition     */
/* ============================= */
/* ============================= */
/* Global function definition    */
/* ============================= */


#ifdef ENABLE_OBSOLETE_BITFIELD_EVENT_INTERFACE
/**
   Returns the current exception status.

   \param pChannel - handle to TAPI_CHANNEL structure

   \return
    nException
*/
IFX_uint32_t TAPI_Phone_Exception (TAPI_CHANNEL *pChannel)
{
   IFX_uint32_t   nException = 0;
   IFX_TAPI_EXCEPTION_t *ex;

   ex = &pChannel->TapiMiscData.nException;
   /* set fault bit */
   if (ex->Bits.ground_key          ||
       ex->Bits.ground_key_polarity ||
       ex->Bits.ground_key_high     ||
       ex->Bits.otemp)
   {
      ex->Bits.fault = IFX_TRUE;
   }
   /* get exception */
   nException = (~pChannel->TapiMiscData.nExceptionMask.Status)
                 & pChannel->TapiMiscData.nException.Status;
   /* delete hook bit */
   ex->Bits.hookstate = IFX_FALSE;
   /* delete cid rx status update bit */
   ex->Bits.cidrx_supdate = IFX_FALSE;
   /* delete flash hook bit */
   ex->Bits.flash_hook = IFX_FALSE;
   /* delete ground key bit */
   ex->Bits.ground_key = IFX_FALSE;
   /* delete ground key polarity bit */
   ex->Bits.ground_key_polarity = IFX_FALSE;
   /* delete ground key high bit */
   ex->Bits.ground_key_high = IFX_FALSE;
   ex->Bits.otemp = IFX_FALSE;
   /* delete fault bit */
   ex->Bits.fault = IFX_FALSE;
   /* clear FAX exceptions */
   ex->Bits.fax_ced     = IFX_FALSE;
   ex->Bits.fax_cng     = IFX_FALSE;
   ex->Bits.fax_supdate = IFX_FALSE;
   ex->Bits.fax_dis     = IFX_FALSE;
   ex->Bits.fax_ced_net = IFX_FALSE;
   ex->Bits.fax_cng_net = IFX_FALSE;
   /* clear gr909 exception bit */
   ex->Bits.gr909result = IFX_FALSE;
   ex->Bits.eventDetect = IFX_FALSE;
   /* clear runtime error bit */
   ex->Bits.runtime_error = IFX_FALSE;

   return nException;
}
#endif /* ENABLE_OBSOLETE_BITFIELD_EVENT_INTERFACE */

/**
   Masks the reporting of the exceptions.                      DEPRECIATED CODE
                                                     use IFX_TAPI_EVENT_DISABLE
   \param pChannel    - handle to TAPI_CHANNEL structure
   \param nException  - exception mask

   \return
   IFX_SUCCESS
*/
IFX_int32_t TAPI_Phone_Mask_Exception (TAPI_CHANNEL *pChannel,
                                       IFX_uint32_t nException)
{
   IFX_TAPI_EVENT_DISABLE_t *pDisable;

   pDisable = &(pChannel->eventHandler.eventDisable);

   pChannel->TapiMiscData.nExceptionMask.Status = nException;
   if(pChannel->TapiMiscData.nExceptionMask.Bits.dtmf_ready)
       pDisable->dtmf.bits.digit_local = IFX_EVENT_DISABLE;
   if (pChannel->TapiMiscData.nExceptionMask.Bits.hookstate)
   {
      pDisable->fxs.bits.offhook = IFX_EVENT_DISABLE;
      pDisable->fxs.bits.onhook = IFX_EVENT_DISABLE;
   }
   if (pChannel->TapiMiscData.nExceptionMask.Bits.otemp)
      pDisable->fault_line.bits.overtemp = IFX_EVENT_DISABLE;
   if (pChannel->TapiMiscData.nExceptionMask.Bits.pulse_digit_ready)
      pDisable->pulse.bits.digit = IFX_EVENT_DISABLE;
   if (pChannel->TapiMiscData.nExceptionMask.Bits.flash_hook)
      pDisable->fxs.bits.flash = IFX_EVENT_DISABLE;
   if (pChannel->TapiMiscData.nExceptionMask.Bits.cidrx_supdate)
   {
      pDisable->cid.bits.rx_end = IFX_EVENT_DISABLE;
   }
   if (pChannel->TapiMiscData.nExceptionMask.Bits.eventDetect)
      pDisable->rfc2833.bits.event = IFX_EVENT_DISABLE;
   if (pChannel->TapiMiscData.nExceptionMask.Bits.ring_finished)
   {
      pDisable->fxs.bits.ringing_end = IFX_EVENT_DISABLE;
   }

   if (pChannel->TapiMiscData.nExceptionMask.Bits.ground_key)
      pDisable->fault_line.bits.gk_low = IFX_EVENT_DISABLE;
   if (pChannel->TapiMiscData.nExceptionMask.Bits.ground_key_high)
      pDisable->fault_line.bits.gk_high = IFX_EVENT_DISABLE;
   if (pChannel->TapiMiscData.nExceptionMask.Bits.ground_key_polarity)
      pDisable->fault_line.bits.gk_pos = IFX_EVENT_DISABLE;
   if (pChannel->TapiMiscData.nExceptionMask.Bits.gr909result)
      pDisable->lt.bits.gr909_rdy = IFX_EVENT_DISABLE;
   if (pChannel->TapiMiscData.nExceptionMask.Bits.fax_supdate)
   {
      pDisable->t38.bits.error_data = IFX_EVENT_DISABLE;
      pDisable->t38.bits.error_ovld = IFX_EVENT_DISABLE;
      pDisable->t38.bits.error_gen = IFX_EVENT_DISABLE;
      pDisable->t38.bits.error_read = IFX_EVENT_DISABLE;
      pDisable->t38.bits.error_write = IFX_EVENT_DISABLE;
      pDisable->t38.bits.error_setup = IFX_EVENT_DISABLE;
   }
   if (pChannel->TapiMiscData.nExceptionMask.Bits.signal)
   {
      pDisable->cid.bits.tx_info_end = IFX_EVENT_DISABLE;
      pDisable->fax_sig.bits.am_local = IFX_EVENT_DISABLE;
      pDisable->fax_sig.bits.am_network= IFX_EVENT_DISABLE;
      pDisable->fax_sig.bits.ced_local = IFX_EVENT_DISABLE;
      pDisable->fax_sig.bits.ced_network= IFX_EVENT_DISABLE;
      pDisable->fax_sig.bits.pr_local = IFX_EVENT_DISABLE;
      pDisable->fax_sig.bits.pr_network= IFX_EVENT_DISABLE;
      pDisable->fax_sig.bits.dis_local = IFX_EVENT_DISABLE;
      pDisable->fax_sig.bits.dis_network= IFX_EVENT_DISABLE;
      pDisable->tone_det.bits.cpt = IFX_EVENT_DISABLE;
      pDisable->tone_det.bits.receive = IFX_EVENT_DISABLE;
      pDisable->tone_det.bits.transmit = IFX_EVENT_DISABLE;
   }

   return IFX_SUCCESS;
}


static void stripPathCpy (char* dst, const char* src)
{
   IFX_uint32_t nMax = strlen (src);
   IFX_uint32_t i = nMax;

   /* only name of file without path */
   while (i > 0 && src[i] != 0x5C && src[i] != '/')
   {
      i--;
   }
   if (i > 0)
   {
      nMax = nMax - i;
      /* ignore / */
      i++;
   }
   if (nMax > (IFX_TAPI_MAX_FILENAME - 1))
   {
      nMax = IFX_TAPI_MAX_FILENAME - 2;
      dst[IFX_TAPI_MAX_FILENAME - 1] = 0;
   }
   memcpy (dst, &src[i], nMax+1);
}
#ifdef ENABLE_OBSOLETE_BITFIELD_EVENT_INTERFACE
/**
   Compability Signal update to map new event interface to the old signal
   interface.

   \param pChannel - handle to tapi channel structure
   \param pEvent   - handle to event

   \return
   IFX_ERROR on error, otherwise IFX_SUCCESS

   \remarks
*/
static IFX_return_t Signal_CompEvent (TAPI_CHANNEL *pChannel,
                                         IFX_TAPI_EVENT_t *pEvent)
{
   IFX_boolean_t bRx = (IFX_boolean_t)pEvent->data.fax_sig.network;
   IFX_boolean_t bTx = (IFX_boolean_t)pEvent->data.fax_sig.local;

   switch (pEvent->id)
   {
      case  IFX_TAPI_EVENT_FAXMODEM_AM:
         if (bRx)
            TAPI_Signal_Event_Update (pChannel, IFX_TAPI_SIG_AMRX,
                                                IFX_TAPI_SIG_EXT_NONE);
         else if (bTx)
            TAPI_Signal_Event_Update (pChannel, IFX_TAPI_SIG_AMTX,
                                                IFX_TAPI_SIG_EXT_NONE);
         else
            TAPI_Signal_Event_Update (pChannel, IFX_TAPI_SIG_AM,
                                                IFX_TAPI_SIG_EXT_NONE);
      break;
      case  IFX_TAPI_EVENT_FAXMODEM_CED:
         if (bRx)
            TAPI_Signal_Event_Update (pChannel, IFX_TAPI_SIG_CEDRX,
                                                IFX_TAPI_SIG_EXT_NONE);
         else if (bTx)
            TAPI_Signal_Event_Update (pChannel, IFX_TAPI_SIG_CEDTX,
                                                IFX_TAPI_SIG_EXT_NONE);
         else
            TAPI_Signal_Event_Update (pChannel, IFX_TAPI_SIG_CED,
                                                IFX_TAPI_SIG_EXT_NONE);
      break;
      case  IFX_TAPI_EVENT_FAXMODEM_CEDEND:
         if (bRx)
            TAPI_Signal_Event_Update (pChannel, IFX_TAPI_SIG_CEDENDRX,
                                                IFX_TAPI_SIG_EXT_NONE);
         else if (bTx)
            TAPI_Signal_Event_Update (pChannel, IFX_TAPI_SIG_CEDENDTX,
                                                IFX_TAPI_SIG_EXT_NONE);
         else
            TAPI_Signal_Event_Update (pChannel, IFX_TAPI_SIG_CEDEND,
                                                IFX_TAPI_SIG_EXT_NONE);
      break;
      case  IFX_TAPI_EVENT_FAXMODEM_DIS:
         if (bRx)
            TAPI_Signal_Event_Update (pChannel, IFX_TAPI_SIG_DISRX,
                                                IFX_TAPI_SIG_EXT_NONE);
         else if (bTx)
            TAPI_Signal_Event_Update (pChannel, IFX_TAPI_SIG_DISTX,
                                                IFX_TAPI_SIG_EXT_NONE);
         else
            TAPI_Signal_Event_Update (pChannel, IFX_TAPI_SIG_DIS,
                                                IFX_TAPI_SIG_EXT_NONE);
      break;
      case  IFX_TAPI_EVENT_FAXMODEM_PR:
         if (bRx)
            TAPI_Signal_Event_Update (pChannel, IFX_TAPI_SIG_PHASEREVRX,
                                                IFX_TAPI_SIG_EXT_NONE);
         else if (bTx)
            TAPI_Signal_Event_Update (pChannel, IFX_TAPI_SIG_PHASEREVTX,
                                                IFX_TAPI_SIG_EXT_NONE);
         else
            TAPI_Signal_Event_Update (pChannel, IFX_TAPI_SIG_PHASEREV,
                                                IFX_TAPI_SIG_EXT_NONE);
      break;
      case  IFX_TAPI_EVENT_FAXMODEM_CNGFAX:
         if (bRx)
            TAPI_Signal_Event_Update (pChannel, IFX_TAPI_SIG_CNGFAXRX,
                                                IFX_TAPI_SIG_EXT_NONE);
         else if (bTx)
            TAPI_Signal_Event_Update (pChannel, IFX_TAPI_SIG_CNGFAXTX,
                                                IFX_TAPI_SIG_EXT_NONE);
         else
            TAPI_Signal_Event_Update (pChannel, IFX_TAPI_SIG_CNGFAX,
                                                IFX_TAPI_SIG_EXT_NONE);
      break;
      case  IFX_TAPI_EVENT_FAXMODEM_CNGMOD:
         if (bRx)
            TAPI_Signal_Event_Update (pChannel, IFX_TAPI_SIG_CNGMODRX,
                                                IFX_TAPI_SIG_EXT_NONE);
         else if (bTx)
            TAPI_Signal_Event_Update (pChannel, IFX_TAPI_SIG_CNGMODTX,
                                                IFX_TAPI_SIG_EXT_NONE);
         else
            TAPI_Signal_Event_Update (pChannel, IFX_TAPI_SIG_CNGMOD,
                                                IFX_TAPI_SIG_EXT_NONE);
      break;
      case  IFX_TAPI_EVENT_FAXMODEM_V21L:
         if (bRx)
            TAPI_Signal_Event_Update (pChannel, IFX_TAPI_SIG_NONE,
                                                IFX_TAPI_SIG_EXT_V21LRX);
         else if (bTx)
            TAPI_Signal_Event_Update (pChannel, IFX_TAPI_SIG_NONE,
                                                IFX_TAPI_SIG_EXT_V21LTX);
         else
            TAPI_Signal_Event_Update (pChannel, IFX_TAPI_SIG_NONE,
                                                IFX_TAPI_SIG_EXT_V21L);
      break;
      case  IFX_TAPI_EVENT_FAXMODEM_V18A:
         if (bRx)
            TAPI_Signal_Event_Update (pChannel, IFX_TAPI_SIG_NONE,
                                                IFX_TAPI_SIG_EXT_V18ARX);
         else if (bTx)
            TAPI_Signal_Event_Update (pChannel, IFX_TAPI_SIG_NONE,
                                                IFX_TAPI_SIG_EXT_V18ATX);
         else
            TAPI_Signal_Event_Update (pChannel, IFX_TAPI_SIG_NONE,
                                                IFX_TAPI_SIG_EXT_V18A);
      break;
      case  IFX_TAPI_EVENT_FAXMODEM_V27:
         if (bRx)
            TAPI_Signal_Event_Update (pChannel, IFX_TAPI_SIG_NONE,
                                                IFX_TAPI_SIG_EXT_V27RX);
         else if (bTx)
            TAPI_Signal_Event_Update (pChannel, IFX_TAPI_SIG_NONE,
                                                IFX_TAPI_SIG_EXT_V27TX);
         else
            TAPI_Signal_Event_Update (pChannel, IFX_TAPI_SIG_NONE,
                                                IFX_TAPI_SIG_EXT_V27);
      break;
      case  IFX_TAPI_EVENT_FAXMODEM_BELL:
         if (bRx)
            TAPI_Signal_Event_Update (pChannel, IFX_TAPI_SIG_NONE,
                                                IFX_TAPI_SIG_EXT_BELLRX);
         else if (bTx)
            TAPI_Signal_Event_Update (pChannel, IFX_TAPI_SIG_NONE,
                                                IFX_TAPI_SIG_EXT_BELLTX);
         else
            TAPI_Signal_Event_Update (pChannel, IFX_TAPI_SIG_NONE,
                                                IFX_TAPI_SIG_EXT_BELL);
      break;
      case  IFX_TAPI_EVENT_FAXMODEM_V22:
         if (bRx)
            TAPI_Signal_Event_Update (pChannel, IFX_TAPI_SIG_NONE,
                                                IFX_TAPI_SIG_EXT_V22RX);
         else if (bTx)
            TAPI_Signal_Event_Update (pChannel, IFX_TAPI_SIG_NONE,
                                                IFX_TAPI_SIG_EXT_V22TX);
         else
            TAPI_Signal_Event_Update (pChannel, IFX_TAPI_SIG_NONE,
                                                IFX_TAPI_SIG_EXT_V22);
      break;
      case  IFX_TAPI_EVENT_FAXMODEM_V22ORBELL:
         if (bRx)
            TAPI_Signal_Event_Update (pChannel, IFX_TAPI_SIG_NONE,
                                                IFX_TAPI_SIG_EXT_V22ORBELLRX);
         else if (bTx)
            TAPI_Signal_Event_Update (pChannel, IFX_TAPI_SIG_NONE,
                                                IFX_TAPI_SIG_EXT_V22ORBELLTX);
         else
            TAPI_Signal_Event_Update (pChannel, IFX_TAPI_SIG_NONE,
                                                IFX_TAPI_SIG_EXT_V22ORBELL);
      break;
      case  IFX_TAPI_EVENT_FAXMODEM_V32AC:
         if (bRx)
            TAPI_Signal_Event_Update (pChannel, IFX_TAPI_SIG_NONE,
                                                IFX_TAPI_SIG_EXT_V32ACRX);
         else if (bTx)
            TAPI_Signal_Event_Update (pChannel, IFX_TAPI_SIG_NONE,
                                                IFX_TAPI_SIG_EXT_V32ACTX);
         else
            TAPI_Signal_Event_Update (pChannel, IFX_TAPI_SIG_NONE,
                                                IFX_TAPI_SIG_EXT_V32AC);
      break;
      case  IFX_TAPI_EVENT_FAXMODEM_V8BIS:
         if (bRx)
            TAPI_Signal_Event_Update (pChannel, IFX_TAPI_SIG_V8BISRX,
                                                IFX_TAPI_SIG_EXT_NONE);
         else if (bTx)
            TAPI_Signal_Event_Update (pChannel, IFX_TAPI_SIG_V8BISTX,
                                                IFX_TAPI_SIG_EXT_NONE);
      break;
      case IFX_TAPI_EVENT_FAXMODEM_CAS_BELL:
         if (bRx)
            TAPI_Signal_Event_Update (pChannel, IFX_TAPI_SIG_NONE,
                                                IFX_TAPI_SIG_EXT_CASBELLRX);
         else if (bTx)
            TAPI_Signal_Event_Update (pChannel, IFX_TAPI_SIG_NONE,
                                                IFX_TAPI_SIG_EXT_CASBELLTX);
         else
            TAPI_Signal_Event_Update (pChannel, IFX_TAPI_SIG_NONE,
                                                IFX_TAPI_SIG_EXT_CASBELL);
         break;
      case IFX_TAPI_EVENT_FAXMODEM_V21H:
         if (bRx)
            TAPI_Signal_Event_Update (pChannel, IFX_TAPI_SIG_NONE,
                                                IFX_TAPI_SIG_EXT_V21HRX);
         else if (bTx)
            TAPI_Signal_Event_Update (pChannel, IFX_TAPI_SIG_NONE,
                                                IFX_TAPI_SIG_EXT_V21HTX);
         else
            TAPI_Signal_Event_Update (pChannel, IFX_TAPI_SIG_NONE,
                                                IFX_TAPI_SIG_EXT_V21H);
         break;
      case  IFX_TAPI_EVENT_FAXMODEM_HOLDEND:
         TAPI_Signal_Event_Update (pChannel, IFX_TAPI_SIG_TONEHOLDING_END,
                                             IFX_TAPI_SIG_EXT_NONE);
         break;
      case  IFX_TAPI_EVENT_FAXMODEM_VMD:
         TAPI_Signal_Event_Update (pChannel, IFX_TAPI_SIG_NONE,
                                             IFX_TAPI_SIG_EXT_VMD);
         break;
      default:
          TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
                ("Signal_CompEvent: Unhandled event %d\n", pEvent->id));
      break;
   }
   return IFX_SUCCESS;
}
#endif /* ENABLE_OBSOLETE_BITFIELD_EVENT_INTERFACE */

#ifdef ENABLE_OBSOLETE_BITFIELD_EVENT_INTERFACE
/**
   Reports the status information for a channel

   \param pChannel - handle to TAPI_CHANNEL structure, in case more than one
                     channel's status shall be retrieved, still a pointer
                     to a channel structure must be provided.
   \param status   - pointer to a list of IFX_TAPI_CH_STATUS_t structures, maybe only one

   \return
   IFX_SUCCESS
*/
IFX_int32_t TAPI_Phone_GetStatus (TAPI_CHANNEL *pChannel,
                                  IFX_TAPI_CH_STATUS_t *status)
{
   IFX_uint8_t channels = status->channels;

   if (channels == 0)
   {
      /* return status for this channel */
      memset (status, 0, sizeof(IFX_TAPI_CH_STATUS_t));
      GetChStatus(pChannel, status);
   }
   else
   {
      /* more channels */
      TAPI_DEV *pTapiDev = pChannel->pTapiDevice;
      IFX_int32_t i;

      memset (status, 0, sizeof (IFX_TAPI_CH_STATUS_t) * channels);

      /* reduce the amount of copied data to the maximum supported channels */
      if (channels > pTapiDev->nMaxChannel)
         channels = pTapiDev->nMaxChannel;

      for (i = 0; i < channels ; i++)
      {
         GetChStatus((pTapiDev->pTapiChanelArray + i), status++);
      }
   }
   return IFX_SUCCESS;
}
#endif /* ENABLE_OBSOLETE_BITFIELD_EVENT_INTERFACE */

#ifdef ENABLE_OBSOLETE_BITFIELD_EVENT_INTERFACE
/**
   Set status information for one channel

   \param pChannel - handle to TAPI_CHANNEL structure
   \param status   - pointer to a IFX_TAPI_CH_STATUS_t structure

   \return
   IFX_SUCCESS
*/
static IFX_int32_t GetChStatus (TAPI_CHANNEL* pChannel,
                                IFX_TAPI_CH_STATUS_t *status)
{
   /* Note: not a event pointer. */
   IFX_TAPI_EVENT_t tapiEvent;
   IFX_int32_t moreEvents;
   IFX_TAPI_DRV_CTX_t *pDrvCtx = (IFX_TAPI_DRV_CTX_t*) pChannel->pTapiDevice->pDevDrvCtx;
   IFX_uint32_t signal, signalExt;

   status->event = (unsigned long)IFX_TAPI_PKT_EV_NUM_NO_EVENT;

   do
   {
      moreEvents = IFX_TAPI_EventFifoGet(pChannel, &tapiEvent);

      switch (tapiEvent.id)
      {
      /* dial events. */
      case IFX_TAPI_EVENT_DTMF_DIGIT:
         if (tapiEvent.data.dtmf.local)
         {
            /* Get DTMF digit. (char) */
            status->digit = (unsigned char) (tapiEvent.data.dtmf.digit & 0xff);
            status->dialing = IFX_TAPI_DIALING_STATUS_DTMF;
         }
         break;
      case IFX_TAPI_EVENT_PULSE_DIGIT:
         /* Get pulse dial digit. (char) */
         status->digit = (unsigned char) (tapiEvent.data.pulse.digit & 0xff);
         status->dialing = IFX_TAPI_DIALING_STATUS_PULSE;
         break;
      /* hook events. */
      case IFX_TAPI_EVENT_FXS_ONHOOK:
      case IFX_TAPI_EVENT_FXS_OFFHOOK:
         if (pChannel->TapiOpControlData.bHookState)
            status->hook = IFX_TAPI_LINE_HOOK_STATUS_OFFHOOK |
                           IFX_TAPI_LINE_HOOK_STATUS_HOOK;
         else
            status->hook = IFX_TAPI_LINE_HOOK_STATUS_HOOK;
         break;
      /* flash hook event. */
      case IFX_TAPI_EVENT_FXS_FLASH:
         status->hook |= IFX_TAPI_LINE_HOOK_STATUS_FLASH;
         break;
      /* overtemp event, fault event. */
      case IFX_TAPI_EVENT_FAULT_LINE_OVERTEMP:
         status->line |= IFX_TAPI_LINE_STATUS_OTEMP;
         break;
      /* Not implemented gr909res */
      case IFX_TAPI_EVENT_LT_GR909_RDY:
         status->line |= IFX_TAPI_LINE_STATUS_GR909RES;
         break;
      /* cid events */
      case IFX_TAPI_EVENT_CID_RX_END:
      case IFX_TAPI_EVENT_CID_RX_ERROR_READ:
         status->line |= IFX_TAPI_LINE_STATUS_CIDRX;
         break;
      case IFX_TAPI_EVENT_CID_TX_INFO_END:
         status->signal |= IFX_TAPI_SIG_CIDENDTX;
         status->line |= IFX_TAPI_LINE_STATUS_FAX;
         break;
      /* RFC2833 event */
      case IFX_TAPI_EVENT_RFC2833_EVENT:
         status->event = tapiEvent.data.rfc2833.event;
         break;
      /* line status events */
      case IFX_TAPI_EVENT_FXS_RINGING_END:
         status->line |= IFX_TAPI_LINE_STATUS_RINGFINISHED;
         break;
      case IFX_TAPI_EVENT_FAULT_LINE_GK_LOW:
         status->line |= IFX_TAPI_LINE_STATUS_GNDKEY;
         break;
      case IFX_TAPI_EVENT_FAULT_LINE_GK_HIGH:
         status->line |= IFX_TAPI_LINE_STATUS_GNDKEYHIGH;
         break;
      case IFX_TAPI_EVENT_FAULT_LINE_GK_NEG:
         /* \todo */
         break;
      case IFX_TAPI_EVENT_FAULT_LINE_GK_POS:
         status->line |= IFX_TAPI_LINE_STATUS_GNDKEYPOL;
         break;
      /* T38 events */
      case IFX_TAPI_EVENT_T38_ERROR_OVLD:
      case IFX_TAPI_EVENT_T38_ERROR_READ:
      case IFX_TAPI_EVENT_T38_ERROR_WRITE:
      case IFX_TAPI_EVENT_T38_ERROR_DATA:
      case IFX_TAPI_EVENT_T38_ERROR_SETUP:
         status->line |= IFX_TAPI_LINE_STATUS_FAX;
         break;
      /* set signal detection status */
      case IFX_TAPI_EVENT_TONE_DET_CPT:
      case IFX_TAPI_EVENT_TONE_DET_RECEIVE:
      case IFX_TAPI_EVENT_TONE_DET_TRANSMIT:
         status->line |= IFX_TAPI_LINE_STATUS_FAX;
         break;


      /* No events detected - fifos are empty */
      case IFX_TAPI_EVENT_NONE:
         /* do nothing - moreEvents is 0 and we will exit below */
         break;
      default:
         /* all events not listed above have not representation in the
            channel status information and are silently discarded */
         break;
      }
   } while (moreEvents);


   /* set signal detection status */
   if ((tapiEvent.id == IFX_TAPI_EVENT_CID_TX_INFO_END) ||
       (tapiEvent.id == IFX_TAPI_EVENT_FAXMODEM_AM) ||
       (tapiEvent.id == IFX_TAPI_EVENT_FAXMODEM_CED) ||
       (tapiEvent.id == IFX_TAPI_EVENT_FAXMODEM_PR) ||
       (tapiEvent.id == IFX_TAPI_EVENT_FAXMODEM_DIS) ||
       (tapiEvent.id == IFX_TAPI_EVENT_TONE_DET_CPT) ||
       (tapiEvent.id == IFX_TAPI_EVENT_TONE_DET_RECEIVE) ||
       (tapiEvent.id == IFX_TAPI_EVENT_TONE_DET_TRANSMIT))
      status->line |= IFX_TAPI_LINE_STATUS_FAX;

   if (ptr_chk(pDrvCtx->SIG.MFTD_Signal_Get,
              "pDrvCtx->SIG.MFTD_Signal_Get"))
   {
      pDrvCtx->SIG.MFTD_Signal_Get (pChannel->pLLChannel, &signal);
      status->signal = signal;
   }
   else
      status->signal = pChannel->TapiMiscData.signal;
   if (ptr_chk(pDrvCtx->SIG.MFTD_Signal_Set, "pDrvCtx->SIG.MFTD_Signal_Set"))
      pDrvCtx->SIG.MFTD_Signal_Set (pChannel->pLLChannel, 0);
   else
      pChannel->TapiMiscData.signal = 0;

   if (ptr_chk(pDrvCtx->SIG.MFTD_Signal_Ext_Get,
              "pDrvCtx->SIG.MFTD_Signal_Ext_Get"))
   {
      pDrvCtx->SIG.MFTD_Signal_Ext_Get (pChannel->pLLChannel, &signalExt);
      status->signal_ext = signalExt;
   }
   else
      status->signal_ext = pChannel->TapiMiscData.signalExt;
   if (ptr_chk(pDrvCtx->SIG.MFTD_Signal_Ext_Set,
              "pDrvCtx->SIG.MFTD_Signal_Ext_Set"))
      pDrvCtx->SIG.MFTD_Signal_Ext_Set (pChannel->pLLChannel, 0);
   else
      pChannel->TapiMiscData.signalExt = 0;

   /* set device status */
   status->device = pChannel->TapiMiscData.device;
   pChannel->TapiMiscData.device = IFX_FALSE;
   /* set runtime errors */
   status->error = pChannel->TapiMiscData.error;
   pChannel->TapiMiscData.error = 0;

   return IFX_SUCCESS;
}
#endif /* ENABLE_OBSOLETE_BITFIELD_EVENT_INTERFACE */

#ifdef ENABLE_OBSOLETE_BITFIELD_EVENT_INTERFACE
/**
   Modem Fax Tone Discriminator event handling.

\param pChannel    - handle to TAPI_CHANNEL structure

 \return Return value according to IFX_return_t
    - IFX_ERROR if an error occured
    - IFX_SUCCESS if successful
                   */
static IFX_int32_t TAPI_Signal_Event_Update (TAPI_CHANNEL *pChannel,
                                             IFX_uint32_t signal,
                                             IFX_uint32_t signal_ext)
{
   IFX_TAPI_DRV_CTX_t *pDrvCtx =
      (IFX_TAPI_DRV_CTX_t *) pChannel->pTapiDevice->pDevDrvCtx;
   IFX_int32_t ret = IFX_SUCCESS;

   if (ptr_chk (pDrvCtx->SIG.MFTD_Signal_Set, "pDrvCtx->SIG.MFTD_Signal_Set"))
      ret = pDrvCtx->SIG.MFTD_Signal_Set (pChannel->pLLChannel, signal);
   else
   {
      pChannel->TapiMiscData.signal |= signal;
   }
   if (ptr_chk (pDrvCtx->SIG.MFTD_Signal_Ext_Set, "pDrvCtx->SIG.MFTD_Signal_Ext_Set"))
      ret = pDrvCtx->SIG.MFTD_Signal_Ext_Set (pChannel->pLLChannel, signal_ext);
   else
   {
      pChannel->TapiMiscData.signalExt |= signal_ext;
   }
   return ret;
}
#endif /* ENABLE_OBSOLETE_BITFIELD_EVENT_INTERFACE */

/**
   Disables the event of a channel

   \param pChannel    - handle to TAPI_CHANNEL structure
   \param pEvent   - The event to be disabled
   \param value Either IFX_EVENT_ENABLE or IFX_EVENT_DISABLE

   \return
   IFX_SUCCESS or IFX_ERROR
*/
/**\todo Map the signal detection events to the low level signal
enable */
/**\todo group FAXMODE and make use of a common union member:
         if (pDisEvnet->data.fax_sig.local)
            pDisable->fax_sig.bits.dis_local = value;
         if (pDisEvnet->data.fax_sig.network)
            pDisable->fax_sig.bits.dis_network = value;
*/
IFX_return_t TAPI_Phone_Set_Event_Disable (TAPI_CHANNEL *pChannel,
                                                  IFX_TAPI_EVENT_t *pEvent,
                                                  IFX_uint32_t value)
{
   IFX_return_t ret = IFX_SUCCESS;
   IFX_TAPI_EVENT_DISABLE_t *pDisable;

   pDisable = &(pChannel->eventHandler.eventDisable);

   switch(pEvent->id)
   {
      case IFX_TAPI_EVENT_FXS_RING:
         pDisable->fxs.bits.ring = value;
         break;
      case IFX_TAPI_EVENT_FXS_RINGBURST_END:
         pDisable->fxs.bits.ringburst_end = value;
         break;
      case IFX_TAPI_EVENT_FXS_RINGING_END:
         pDisable->fxs.bits.ringing_end = value;
         break;
      case IFX_TAPI_EVENT_FXS_ONHOOK:
         pDisable->fxs.bits.onhook = value;
         break;
      case IFX_TAPI_EVENT_FXS_OFFHOOK:
         pDisable->fxs.bits.offhook = value;
         break;
      case IFX_TAPI_EVENT_FXS_FLASH:
         pDisable->fxs.bits.flash = value;
         break;
      case IFX_TAPI_EVENT_FXO_RING_START:
         pDisable->fxo.bits.ring_start = value;
         break;
      case IFX_TAPI_EVENT_FXO_RING_STOP:
         pDisable->fxo.bits.ring_stop = value;
         break;
      case IFX_TAPI_EVENT_FXO_POLARITY:
         pDisable->fxo.bits.polarity = value;
         break;
      case IFX_TAPI_EVENT_FXO_BAT_FEEDED:
         pDisable->fxo.bits.bat_feeded = value;
         break;
      case IFX_TAPI_EVENT_FXO_BAT_DROPPED:
         pDisable->fxo.bits.bat_dropped = value;
         break;
      case IFX_TAPI_EVENT_LT_GR909_RDY:
         pDisable->lt.bits.gr909_rdy = value;
         break;
      case IFX_TAPI_EVENT_PULSE_DIGIT:
         pDisable->pulse.bits.digit = value;
         break;
      case IFX_TAPI_EVENT_DTMF_DIGIT:
         if (pEvent->data.dtmf.local)
            pDisable->dtmf.bits.digit_local = value;
         if (pEvent->data.dtmf.network)
            pDisable->dtmf.bits.digit_network = value;
         break;
      case IFX_TAPI_EVENT_CID_TX_SEQ_START:
         pDisable->cid.bits.tx_seq_start = value;
         break;
      case IFX_TAPI_EVENT_CID_TX_SEQ_END:
         pDisable->cid.bits.tx_seq_end = value;
         break;
      case IFX_TAPI_EVENT_CID_TX_INFO_START:
         pDisable->cid.bits.tx_info_start = value;
         break;
      case IFX_TAPI_EVENT_CID_TX_INFO_END:
         pDisable->cid.bits.tx_info_end = value;
         break;
      case IFX_TAPI_EVENT_CID_RX_END:
         pDisable->cid.bits.rx_end = value;
         break;
      case IFX_TAPI_EVENT_CID_RX_CD:
         pDisable->cid.bits.rx_cd = value;
         break;
      case IFX_TAPI_EVENT_TONE_GEN_BUSY:
         pDisable->tone_gen.bits.busy = value;
         break;
      case IFX_TAPI_EVENT_TONE_GEN_END:
         if (pEvent->data.tone_gen.local)
            pDisable->tone_gen.bits.end_local = value;
         if (pEvent->data.tone_gen.network)
            pDisable->tone_gen.bits.end_network = value;
         break;
      case IFX_TAPI_EVENT_TONE_DET_RECEIVE:
         pDisable->tone_det.bits.receive = value;
         break;
      case IFX_TAPI_EVENT_TONE_DET_TRANSMIT:
         pDisable->tone_det.bits.transmit = value;
         break;
      case IFX_TAPI_EVENT_TONE_DET_CPT:
         pDisable->tone_det.bits.cpt = value;
         break;
      case IFX_TAPI_EVENT_FAXMODEM_DIS:
         if (pEvent->data.fax_sig.local)
            pDisable->fax_sig.bits.dis_local = value;
         if (pEvent->data.fax_sig.network)
            pDisable->fax_sig.bits.dis_network = value;
         break;
      case IFX_TAPI_EVENT_FAXMODEM_CED:
         if (pEvent->data.fax_sig.local)
            pDisable->fax_sig.bits.ced_local = value;
         if (pEvent->data.fax_sig.network)
            pDisable->fax_sig.bits.ced_network = value;
         break;
      case IFX_TAPI_EVENT_FAXMODEM_PR:
         if (pEvent->data.fax_sig.local)
            pDisable->fax_sig.bits.pr_local = value;
         if (pEvent->data.fax_sig.network)
            pDisable->fax_sig.bits.pr_network = value;
         break;
      case IFX_TAPI_EVENT_FAXMODEM_AM:
         if (pEvent->data.fax_sig.local)
            pDisable->fax_sig.bits.am_local = value;
         if (pEvent->data.fax_sig.network)
            pDisable->fax_sig.bits.am_network = value;
         break;
      case IFX_TAPI_EVENT_FAXMODEM_CNGFAX:
         if (pEvent->data.fax_sig.local)
            pDisable->fax_sig.bits.cngfax_local = value;
         if (pEvent->data.fax_sig.network)
            pDisable->fax_sig.bits.cngfax_network = value;
         break;
      case IFX_TAPI_EVENT_FAXMODEM_CNGMOD:
         if (pEvent->data.fax_sig.local)
            pDisable->fax_sig.bits.cngmod_local = value;
         if (pEvent->data.fax_sig.network)
            pDisable->fax_sig.bits.cngmod_network = value;
         break;
      case IFX_TAPI_EVENT_FAXMODEM_V21L:
         if (pEvent->data.fax_sig.local)
            pDisable->fax_sig.bits.v21l_local = value;
         if (pEvent->data.fax_sig.network)
            pDisable->fax_sig.bits.v21l_network = value;
         break;
      case IFX_TAPI_EVENT_FAXMODEM_V18A:
         if (pEvent->data.fax_sig.local)
            pDisable->fax_sig.bits.v18a_local = value;
         if (pEvent->data.fax_sig.network)
            pDisable->fax_sig.bits.v18a_network = value;
         break;
      case IFX_TAPI_EVENT_FAXMODEM_V27:
         if (pEvent->data.fax_sig.local)
            pDisable->fax_sig.bits.v27_local = value;
         if (pEvent->data.fax_sig.network)
            pDisable->fax_sig.bits.v27_network = value;
         break;
      case IFX_TAPI_EVENT_FAXMODEM_BELL:
         if (pEvent->data.fax_sig.local)
            pDisable->fax_sig.bits.bell_local = value;
         if (pEvent->data.fax_sig.network)
            pDisable->fax_sig.bits.bell_network = value;
         break;
      case IFX_TAPI_EVENT_FAXMODEM_V22:
         if (pEvent->data.fax_sig.local)
            pDisable->fax_sig.bits.v22_local = value;
         if (pEvent->data.fax_sig.network)
            pDisable->fax_sig.bits.v22_network = value;
         break;
      case IFX_TAPI_EVENT_FAXMODEM_V22ORBELL:
         if (pEvent->data.fax_sig.local)
            pDisable->fax_sig.bits.v22orbell_local = value;
         if (pEvent->data.fax_sig.network)
            pDisable->fax_sig.bits.v22orbell_network = value;
         break;
      case IFX_TAPI_EVENT_FAXMODEM_V32AC:
         if (pEvent->data.fax_sig.local)
            pDisable->fax_sig.bits.v32ac_local = value;
         if (pEvent->data.fax_sig.network)
            pDisable->fax_sig.bits.v32ac_network = value;
         break;
      case IFX_TAPI_EVENT_FAXMODEM_CAS_BELL:
         if (pEvent->data.fax_sig.local)
            pDisable->fax_sig.bits.cas_local = value;
         if (pEvent->data.fax_sig.network)
            pDisable->fax_sig.bits.cas_network = value;
         break;
      case IFX_TAPI_EVENT_FAXMODEM_V21H:
         if (pEvent->data.fax_sig.local)
            pDisable->fax_sig.bits.v21h_local = value;
         if (pEvent->data.fax_sig.network)
            pDisable->fax_sig.bits.v21h_network = value;
         break;
      case IFX_TAPI_EVENT_FAXMODEM_V8BIS:
         if (pEvent->data.fax_sig.local)
            pDisable->fax_sig.bits.v8bis_local = value;
         if (pEvent->data.fax_sig.network)
            pDisable->fax_sig.bits.v8bis_network = value;
         break;
      case IFX_TAPI_EVENT_FAXMODEM_HOLDEND:
         if (pEvent->data.fax_sig.local)
            pDisable->fax_sig.bits.hold_local = value;
         if (pEvent->data.fax_sig.network)
            pDisable->fax_sig.bits.hold_network = value;
         break;
      case IFX_TAPI_EVENT_RFC2833_EVENT:
         pDisable->rfc2833.bits.event = value;
         break;
      case IFX_TAPI_EVENT_LL_DRIVER_WD_FAIL:
         pDisable->ll_driver.bits.alive = value;
         break;
      case IFX_TAPI_EVENT_FAULT_LINE_GK_POS:
         pDisable->fault_line.bits.gk_pos = value;
         break;
      case IFX_TAPI_EVENT_FAULT_LINE_GK_NEG:
         pDisable->fault_line.bits.gk_neg = value;
         break;
      case IFX_TAPI_EVENT_FAULT_LINE_GK_LOW:
         pDisable->fault_line.bits.gk_low = value;
         break;
      case IFX_TAPI_EVENT_FAULT_LINE_GK_HIGH:
         pDisable->fault_line.bits.gk_high = value;
         break;
      case IFX_TAPI_EVENT_FAULT_LINE_OVERTEMP:
         pDisable->fault_line.bits.overtemp = value;
         break;
      case IFX_TAPI_EVENT_FAULT_HW_CLOCK_FAIL:
         pDisable->fault_hw.bits.clock_fail = value;
         break;
      case IFX_TAPI_EVENT_FAULT_HW_CLOCK_FAIL_END:
         pDisable->fault_hw.bits.clock_fail_end = value;
         break;
      case IFX_TAPI_EVENT_FAULT_HW_FAULT:
         pDisable->fault_hw.bits.hw_fault = value;
         break;
      case IFX_TAPI_EVENT_COD_DEC_CHG:
         pDisable->coder.bits.dec_chg = value;
         break;
      case IFX_TAPI_EVENT_COD_ROOM_NOISE:
         pDisable->coder.bits.room_noise = value;
         break;
      case IFX_TAPI_EVENT_COD_ROOM_SILENCE:
         pDisable->coder.bits.room_silence = value;
         break;
      default :
         ret = IFX_ERROR;
         break;
   }
   return ret;
}

/**
   Reports the event information for a channel

   \param  pTapiDev     Pointer to TAPI device structure.
   \param  pEvent       Pointer to a event structure.

   \return
   IFX_SUCCESS
*/
IFX_int32_t TAPI_Phone_GetEvent (TAPI_DEV *pTapiDev,
                                 IFX_TAPI_EVENT_t *pEvent)
{
   static IFX_uint8_t last_ch_nr = 0;  /* remember channel between calls */
   IFX_int16_t found_ch_nr;
   IFX_uint8_t i;
   IFX_int32_t moreEvents = 0,
               needWakeup = 0;

   if (pEvent->ch == IFX_TAPI_EVENT_ALL_CHANNELS)
   {
      /* Scan every channel to see if any event inside.
         If one event gotten, then report to application.
         So, application get one event at one time. */

      /* We start scanning with the next channel after the last channel
         processed during the last call. By adding one we are called at
         least one time. Even if there is only one channel the loop will
         be aborted because of the wrap-around correction. */
      for (i = last_ch_nr+1, found_ch_nr = -1 ;
           i = (i >= pTapiDev->nMaxChannel) ? 0 : i, 1 ;
           i++)
      {
         /* As long as we have not found any event we check all the channels
            for events. As soon as we got an event we still check if there
            are any more events in any of the remaining channels. */
         if (found_ch_nr < 0)
         {
            /* check this channel for events */
            moreEvents = IFX_TAPI_EventFifoGet(pTapiDev->pTapiChanelArray + i,
                                               pEvent);
            if (pEvent->id != IFX_TAPI_EVENT_NONE)
            {
               found_ch_nr = i;
            }
         }
         else
         {
            /* check only if events present but do not get them */
            moreEvents += !IFX_TAPI_EventFifoEmpty(pTapiDev->pTapiChanelArray + i);
         }

         /* abort loop if we either wrapped around or found that there are
            more than just one event which implies that we already got one */
         if ((i == last_ch_nr) || (moreEvents != 0))
            break;
      }

      /* remember the last channel number that was serviced */
      last_ch_nr = found_ch_nr < 0 ? i : found_ch_nr;
   }
   else /* Scanning event by channel index. */
   {
      if (pEvent->ch >= pTapiDev->nMaxChannel)
         return IFX_ERROR;
      /* Still get one event at one time. */
      moreEvents = IFX_TAPI_EventFifoGet(pTapiDev->pTapiChanelArray + pEvent->ch,
                                         pEvent);

      /* if this channel does not hold more events check all the other
         channel if maybe a wakeup is needed because of events there */
      if (moreEvents == 0)
      {
         for (i=0; (i < pTapiDev->nMaxChannel) && (needWakeup == 0); i++)
         {
            needWakeup += !IFX_TAPI_EventFifoEmpty(pTapiDev->pTapiChanelArray + i);
         }
      }
   }

   /* Set the more events flag inside the event message. The flag indicates
      if there are any more events in the channel that was given as a parameter.
      So for specific channels it returns if there are more events in the
      specific channel. For IFX_TAPI_EVENT_ALL_CHANNELS it returns if there
      are any more events in any of the channels. */
   if (pEvent != IFX_NULL)
   {
      pEvent->more = moreEvents;
   }

/* marked for deletion in TAPI 3.7 - should not hurt and but has no effect as
   we aren't sleeping in select while this ioctl is handled... */
   /* Wake up the filedescriptor again while there is any event in any channel
      left that needs service. In case the event doing polling and fetches the
      event before going to sleep next time the wakeup we do not cancle the
      wakup. So when the application tries to sleep it will be woken because
      of an event which it already got. It will then not find any event and
      run an empty cycle before going to sleep again. */
   if ((moreEvents != 0) || (needWakeup != 0))
   {
      /* Wake up the device file descriptor as long as there are events in
         any of the channel queues. Channel parameter is just needed for
         access to the device so any TAPI channel will do. */
      TAPI_WakeUp(pTapiDev);
   }
/* marked for deletion in TAPI 3.7 - should not hurt and but has no effect */

   return IFX_SUCCESS;
}

/**

   Event handling function. Is called from interrupt routine.

   \param pChannel    - handle to TAPI_CHANNEL structure

   \return
   none
*/
IFX_void_t TAPI_Phone_Event_GNKH (TAPI_CHANNEL * pChannel)
{
   /* set the TAPI line mode to power down. The TAPI will not signal hook
      changes to the client application anymore till the mode is changed again */

   switch (pChannel->TapiMiscData.GndkhState)
   {
   case GNDKH_STATE_READY:
      /* start of state machine */
      TAPI_SetTime_Timer (pChannel->TapiMiscData.GndkhTimerID,
                          GNDKH_RECHECKTIME1, IFX_FALSE, IFX_TRUE);
      pChannel->TapiMiscData.GndkhState = GNDKH_STATE_POWDN;
      break;
   case GNDKH_STATE_POWDN:
      /* ground key in state power down should not be possible */
      break;
   case GNDKH_STATE_RECHECK:
      /* ground key occured second time -> raise error */
      pChannel->TapiMiscData.GndkhState = GNDKH_STATE_READY;
      pChannel->TapiMiscData.nException.Bits.ground_key_high = IFX_TRUE;
      pChannel->TapiOpControlData.bFaulCond = IFX_TRUE;
      break;
   default:
      break;
   }

   return;
}

/**

   Ground key high recheck timer entry.

   \param Timer - timer ID
   \param nArg    - handle to TAPI_CHANNEL structure

   \return
     None

   \remark
   To ignore invalid ground key high interrupts the line is switched off on
   the first GNDKH and this timer is started. After the first expiry the
   line is switched on again and checked again for GNDKH. If the GNDKH
   is signaled again it is handled as an fault.
*/
IFX_void_t TAPI_Gndkh_OnTimer (Timer_ID Timer, IFX_int32_t nArg)
{
   TAPI_CHANNEL *pChannel = (TAPI_CHANNEL *) nArg;
   IFX_TAPI_DRV_CTX_t *pDrvCtx =
      (IFX_TAPI_DRV_CTX_t *) pChannel->pTapiDevice->pDevDrvCtx;

   switch (pChannel->TapiMiscData.GndkhState)
   {
   case GNDKH_STATE_READY:
      /* cannot occur */
      break;
   case GNDKH_STATE_POWDN:
      /* recheck the line with origin state */
      if (ptr_chk
          (pDrvCtx->ALM.FaultLine_Restore, "pDrvCtx->ALM.FaultLine_Restore"))
         pDrvCtx->ALM.FaultLine_Restore (pChannel->pLLChannel);

      pChannel->TapiMiscData.GndkhState = GNDKH_STATE_RECHECK;
      TAPI_SetTime_Timer (pChannel->TapiMiscData.GndkhTimerID,
                          GNDKH_RECHECKTIME2, IFX_FALSE, IFX_TRUE);
      break;
   case GNDKH_STATE_RECHECK:
      /* no further ground key occured within time thus no error */
      pChannel->TapiMiscData.GndkhState = GNDKH_STATE_READY;
      pChannel->TapiOpControlData.bFaulCond = IFX_FALSE;
      TAPI_Stop_Timer (pChannel->TapiMiscData.GndkhTimerID);
      break;
   default:
      break;
   }
}

/**
   Returns the enable status of the fax modem event.

   \param pChannel - handle to tapi channel structure
   \param pEvent   - handle to event

   \return
      IFX_TRUE if the event is enabled, otherwise IFX_FALSE

   \remarks
*/
static IFX_boolean_t Check_FaxModem_Status (TAPI_CHANNEL *pChannel,
                                               IFX_TAPI_EVENT_t *pEvent)
{
   IFX_boolean_t eventDisabled = IFX_FALSE;

   if (pEvent->id == IFX_TAPI_EVENT_FAXMODEM_AM)
   {
      if ((pChannel->eventHandler.eventDisable.fax_sig.bits.am_local) &&
          (pEvent->data.fax_sig.local))
         eventDisabled = IFX_TRUE;
      else
         if ((pChannel->eventHandler.eventDisable.fax_sig.bits.am_network)
             && (pEvent->data.fax_sig.network))
         eventDisabled = IFX_TRUE;
      else
         eventDisabled = IFX_FALSE;
   }
   if (pEvent->id == IFX_TAPI_EVENT_FAXMODEM_CED)
   {
      if ((pChannel->eventHandler.eventDisable.fax_sig.bits.ced_local) &&
          (pEvent->data.fax_sig.local))
         eventDisabled = IFX_TRUE;
      else
         if ((pChannel->eventHandler.eventDisable.fax_sig.bits.
              ced_network) && (pEvent->data.fax_sig.network))
         eventDisabled = IFX_TRUE;
      else
         eventDisabled = IFX_FALSE;
   }
   if (pEvent->id == IFX_TAPI_EVENT_FAXMODEM_PR)
   {
      if ((pChannel->eventHandler.eventDisable.fax_sig.bits.pr_local) &&
          (pEvent->data.fax_sig.local))
         eventDisabled = IFX_TRUE;
      else
         if ((pChannel->eventHandler.eventDisable.fax_sig.bits.pr_network)
             && (pEvent->data.fax_sig.network))
         eventDisabled = IFX_TRUE;
      else
         eventDisabled = IFX_FALSE;
   }
   if (pEvent->id == IFX_TAPI_EVENT_FAXMODEM_DIS)
   {
      if ((pChannel->eventHandler.eventDisable.fax_sig.bits.dis_local) &&
          (pEvent->data.fax_sig.local))
         eventDisabled = IFX_TRUE;
      else
         if ((pChannel->eventHandler.eventDisable.fax_sig.bits.
              dis_network) && (pEvent->data.fax_sig.network))
         eventDisabled = IFX_TRUE;
      else
         eventDisabled = IFX_FALSE;
   }
   if (pEvent->id == IFX_TAPI_EVENT_FAXMODEM_CNGFAX)
   {
      if ((pChannel->eventHandler.eventDisable.fax_sig.bits.cngfax_local) &&
          (pEvent->data.fax_sig.local))
         eventDisabled = IFX_TRUE;
      else
         if ((pChannel->eventHandler.eventDisable.fax_sig.bits.
              cngfax_network) && (pEvent->data.fax_sig.network))
         eventDisabled = IFX_TRUE;
      else
         eventDisabled = IFX_FALSE;
   }
   if (pEvent->id == IFX_TAPI_EVENT_FAXMODEM_CNGMOD)
   {
      if ((pChannel->eventHandler.eventDisable.fax_sig.bits.cngmod_local) &&
          (pEvent->data.fax_sig.local))
         eventDisabled = IFX_TRUE;
      else
         if ((pChannel->eventHandler.eventDisable.fax_sig.bits.
              cngmod_network) && (pEvent->data.fax_sig.network))
         eventDisabled = IFX_TRUE;
      else
         eventDisabled = IFX_FALSE;
   }
   if (pEvent->id == IFX_TAPI_EVENT_FAXMODEM_V21L)
   {
      if ((pChannel->eventHandler.eventDisable.fax_sig.bits.cngmod_local) &&
          (pEvent->data.fax_sig.local))
         eventDisabled = IFX_TRUE;
      else
         if ((pChannel->eventHandler.eventDisable.fax_sig.bits.
              cngmod_network) && (pEvent->data.fax_sig.network))
         eventDisabled = IFX_TRUE;
      else
         eventDisabled = IFX_FALSE;
   }
   if (pEvent->id == IFX_TAPI_EVENT_FAXMODEM_V18A)
   {
      if ((pChannel->eventHandler.eventDisable.fax_sig.bits.v18a_local) &&
          (pEvent->data.fax_sig.local))
         eventDisabled = IFX_TRUE;
      else
         if ((pChannel->eventHandler.eventDisable.fax_sig.bits.
              v18a_network) && (pEvent->data.fax_sig.network))
         eventDisabled = IFX_TRUE;
      else
         eventDisabled = IFX_FALSE;
   }
   if (pEvent->id == IFX_TAPI_EVENT_FAXMODEM_V27)
   {
      if ((pChannel->eventHandler.eventDisable.fax_sig.bits.v27_local) &&
          (pEvent->data.fax_sig.local))
         eventDisabled = IFX_TRUE;
      else
         if ((pChannel->eventHandler.eventDisable.fax_sig.bits.
              v27_network) && (pEvent->data.fax_sig.network))
         eventDisabled = IFX_TRUE;
      else
         eventDisabled = IFX_FALSE;
   }
   if (pEvent->id == IFX_TAPI_EVENT_FAXMODEM_BELL)
   {
      if ((pChannel->eventHandler.eventDisable.fax_sig.bits.bell_local) &&
          (pEvent->data.fax_sig.local))
         eventDisabled = IFX_TRUE;
      else
         if ((pChannel->eventHandler.eventDisable.fax_sig.bits.
              bell_network) && (pEvent->data.fax_sig.network))
         eventDisabled = IFX_TRUE;
      else
         eventDisabled = IFX_FALSE;
   }
   if (pEvent->id == IFX_TAPI_EVENT_FAXMODEM_V22)
   {
      if ((pChannel->eventHandler.eventDisable.fax_sig.bits.v22_local) &&
          (pEvent->data.fax_sig.local))
         eventDisabled = IFX_TRUE;
      else
         if ((pChannel->eventHandler.eventDisable.fax_sig.bits.
              v22_network) && (pEvent->data.fax_sig.network))
         eventDisabled = IFX_TRUE;
      else
         eventDisabled = IFX_FALSE;
   }
   if (pEvent->id == IFX_TAPI_EVENT_FAXMODEM_V22ORBELL)
   {
      if ((pChannel->eventHandler.eventDisable.fax_sig.bits.v22orbell_local) &&
          (pEvent->data.fax_sig.local))
         eventDisabled = IFX_TRUE;
      else
         if ((pChannel->eventHandler.eventDisable.fax_sig.bits.
              v22orbell_network) && (pEvent->data.fax_sig.network))
         eventDisabled = IFX_TRUE;
      else
         eventDisabled = IFX_FALSE;
   }
   if (pEvent->id == IFX_TAPI_EVENT_FAXMODEM_V32AC)
   {
      if ((pChannel->eventHandler.eventDisable.fax_sig.bits.v32ac_local) &&
          (pEvent->data.fax_sig.local))
         eventDisabled = IFX_TRUE;
      else
         if ((pChannel->eventHandler.eventDisable.fax_sig.bits.
              v32ac_network) && (pEvent->data.fax_sig.network))
         eventDisabled = IFX_TRUE;
      else
         eventDisabled = IFX_FALSE;
   }
   if (pEvent->id == IFX_TAPI_EVENT_FAXMODEM_CAS_BELL)
   {
      if ((pChannel->eventHandler.eventDisable.fax_sig.bits.cas_local) &&
          (pEvent->data.fax_sig.local))
         eventDisabled = IFX_TRUE;
      else
         if ((pChannel->eventHandler.eventDisable.fax_sig.bits.
              cas_network) && (pEvent->data.fax_sig.network))
         eventDisabled = IFX_TRUE;
      else
         eventDisabled = IFX_FALSE;
   }
   if (pEvent->id == IFX_TAPI_EVENT_FAXMODEM_V21H)
   {
      if ((pChannel->eventHandler.eventDisable.fax_sig.bits.v21h_local) &&
          (pEvent->data.fax_sig.local))
         eventDisabled = IFX_TRUE;
      else
         if ((pChannel->eventHandler.eventDisable.fax_sig.bits.
              v21h_network) && (pEvent->data.fax_sig.network))
         eventDisabled = IFX_TRUE;
      else
         eventDisabled = IFX_FALSE;
   }
   if (pEvent->id == IFX_TAPI_EVENT_FAXMODEM_V8BIS)
   {
      if ((pChannel->eventHandler.eventDisable.fax_sig.bits.v8bis_local) &&
          (pEvent->data.fax_sig.local))
         eventDisabled = IFX_TRUE;
      else
         if ((pChannel->eventHandler.eventDisable.fax_sig.bits.
              v8bis_network) && (pEvent->data.fax_sig.network))
         eventDisabled = IFX_TRUE;
      else
         eventDisabled = IFX_FALSE;
   }
   if (pEvent->id == IFX_TAPI_EVENT_FAXMODEM_HOLDEND)
   {
      /* holding will usually send from local and network as a sum */
      if ((pChannel->eventHandler.eventDisable.fax_sig.bits.hold_local) &&
          (pEvent->data.fax_sig.local))
         eventDisabled = IFX_TRUE;
      else
         if ((pChannel->eventHandler.eventDisable.fax_sig.bits.
              hold_network) && (pEvent->data.fax_sig.network))
         eventDisabled = IFX_TRUE;
      else
         eventDisabled = IFX_FALSE;
   }

   return eventDisabled;
}

#ifdef TAPI_FAX_T38
/**

   Fax Status update Event handling function. Is called from interrupt routine
   or elsewhere a fax related error occurs.

   \param pChannel    - handle to TAPI_CHANNEL structure
   \param status      - Fax status
   \param error       - Fax error

   \return
      None
*/
IFX_void_t TAPI_FaxT38_Event_Update (TAPI_CHANNEL * pChannel,
                                     IFX_uint8_t status, IFX_uint8_t error)
{
   IFX_TAPI_DRV_CTX_t *pDrvCtx =
      (IFX_TAPI_DRV_CTX_t *) pChannel->pTapiDevice->pDevDrvCtx;

   if (ptr_chk
       (pDrvCtx->COD.T38_Status_Set, "pDrvCtx->COD.T38_Status_Set"))
      pDrvCtx->COD.T38_Status_Set (pChannel->pLLChannel, status);

   if (ptr_chk (pDrvCtx->COD.T38_Error_Set, "pDrvCtx->COD.T38_Error_Set"))
      pDrvCtx->COD.T38_Error_Set (pChannel->pLLChannel, error);

   pChannel->TapiMiscData.nException.Bits.fax_supdate = IFX_TRUE;
}
#endif /* TAPI_FAX_T38 */

/**
   WakeUp the TAPI channel task.

   \param  pTapiDev     Pointer to TAPI device structure.
*/
static IFX_void_t TAPI_WakeUp (TAPI_DEV *pTapiDev)
{
   /* In polled mode, there are no sleeping tasks (blocked on select), so no
      wakeup needed */
   /* Polling Not supported
    if ((pTapiDev->bNeedWakeup == IFX_TRUE) && (pDev->IrqPollMode &
    VIN_TAPI_WAKEUP))*/

   if (pTapiDev->bNeedWakeup)
   {
      IFXOS_WakeUp (pTapiDev->wqEvent, IFXOS_READQ);
   }
}

/**
  Check whether event fifo is empty or not (protected).
  \param pChannel - High level TAPI Channel

  \return
    If empty, return 1,else 0.
*/
IFX_uint8_t IFX_TAPI_EventFifoEmpty (TAPI_CHANNEL * pChannel)
{
   IFX_uint8_t ret = 0;

   /* Lock the fifo access protection. */
   IFXOS_MutexLock (pChannel->eventHandler.fifoAcc);

   if (fifoEmpty (pChannel->eventHandler.pTapiEventFifoHi) &&
       fifoEmpty (pChannel->eventHandler.pTapiEventFifoLo))
   {
      ret = 1;
   }
   else
   {
      ret = 0;
   }

   /* Unlock fifo access protection. */
   IFXOS_MutexUnlock (pChannel->eventHandler.fifoAcc);

   return ret;
}

/**
   Get event stored in the event fifo.

   \param pChannel -Channel context for HL TAPI.
   \param pTapiEvent - Event.

   \return
   If there are more events stored in hi/lo fifo, return 1, otherwise return 0.

   \remarks
   High Level (ioctl) event fifo access function.
   Because this function was called by ioctl, it needs semphore protection
   and ceases interrupt routine.
   It will free the buffer after it returns a event.
*/
IFX_int32_t IFX_TAPI_EventFifoGet (TAPI_CHANNEL *pChannel,
                                   IFX_TAPI_EVENT_t *pTapiEvent)
{
   IFX_TAPI_EVENT_t *pTempEvt = NULL;
   IFX_int32_t moreEvent = 0;
   IFX_int32_t err;
   IFXOS_INTSTAT lock;

   /* Lock the fifo access protection. */
   IFXOS_MutexLock (pChannel->eventHandler.fifoAcc);

   /* Get event stored in high prio fifo */
   pTempEvt = fifoGet (pChannel->eventHandler.pTapiEventFifoHi, NULL);

   if (pTempEvt)
   {
      /* copy event data into struct provided by the caller */
      memcpy (pTapiEvent, pTempEvt, sizeof (IFX_TAPI_EVENT_t));
      /* release the buffer back to the pool */
      IFXOS_MutexLock (semBufferPoolAcc);
      IFXOS_LOCKINT(lock);
      err = bufferPoolPut ((void *) pTempEvt);
      IFXOS_UNLOCKINT(lock);
      IFXOS_MutexUnlock (semBufferPoolAcc);
      if (err == BUFFERPOOL_ERROR)
      {
         /* \todo if bufferpool handling fail? */
      }

      /* Check whether there are any more events stored in hi or lo fifo. */
      if (fifoEmpty (pChannel->eventHandler.pTapiEventFifoHi) &&
          fifoEmpty (pChannel->eventHandler.pTapiEventFifoLo))
      {
         /* no events stored in hi or lo fifo */
         moreEvent = 0;
      }
      else
      {
         moreEvent = 1;
      }

      /* Unlock fifo access protection. */
      IFXOS_MutexUnlock (pChannel->eventHandler.fifoAcc);

      return moreEvent;
   }

   /* If hi prio fifo is empty, get event stored in low prio fifo. */
   pTempEvt = fifoGet (pChannel->eventHandler.pTapiEventFifoLo, NULL);
   if (pTempEvt)
   {
      /* copy event data into struct provided by the caller */
      memcpy (pTapiEvent, pTempEvt, sizeof (IFX_TAPI_EVENT_t));
      /* release the buffer back to the pool */
      IFXOS_MutexLock (semBufferPoolAcc);
      IFXOS_LOCKINT(lock);
      err = bufferPoolPut ((void *) pTempEvt);
      IFXOS_UNLOCKINT(lock);
      IFXOS_MutexUnlock (semBufferPoolAcc);

      if (err == BUFFERPOOL_ERROR)
      {
         /* \todo if bufferpool handling fail? */
      }

      /* Check whether there are any more events stored in lo fifo. */
      if (fifoEmpty (pChannel->eventHandler.pTapiEventFifoLo))
      {
         /* no events stored in lo fifo */
         moreEvent = 0;
      }
      else
      {
         moreEvent = 1;
      }

      /* Unlock fifo access protection. */
      IFXOS_MutexUnlock (pChannel->eventHandler.fifoAcc);

      return moreEvent;
   }

   /* Unlock fifo access protection. */
   IFXOS_MutexUnlock (pChannel->eventHandler.fifoAcc);

   /* If high and low prio fifo is empty, set moreEvent to 0 and event id
      to IFX_TAPI_EVENT_NONE. Do not change the channel parameter. */
   pTapiEvent->id = IFX_TAPI_EVENT_NONE;
   moreEvent = 0;

   return moreEvent;
}


/********************NOTE *****************************************************

 1.IFX_TAPI_Event_Dispatch : Call this function to dispatch an event. The
 event is stored in an allocated buffer and a second buffer with additional
 information such as the channel is created. This is then passed on to task
 context with the TAPI_DeferWork() function.

 2.IFX_TAPI_Event_Dispatch_ProcessCtx : This function is working in task
 context. It receives the events dispatched by IFX_TAPI_Event_Dispatch
 processes and filters them and finally put them into the queue towards the
 application.

*******************************************************************************/

/**
   High Level Event Dispatcher function
   (May be called from task or interrupt context.)

   \param pChannel     - handle to tapi channel structure
   \param pTapiEvent   - handle to event

   \return IFX_SUCCESS on ok, otherwise IFX_ERROR
 */
IFX_return_t IFX_TAPI_Event_Dispatch (TAPI_CHANNEL *pChannel,
                                      IFX_TAPI_EVENT_t *pTapiEvent)
{
   IFX_return_t ret = IFX_SUCCESS;
   IFX_TAPI_EVENT_t *pEvt;
   IFX_TAPI_EXT_EVENT_PARAM_t *pParam;
   IFXOS_INTSTAT lock;

   /* filter error message and just copy the error stack */
   if (pTapiEvent->id == IFX_TAPI_EVENT_FAULT_GENERAL_CHINFO ||
       pTapiEvent->id == IFX_TAPI_EVENT_FAULT_GENERAL_DEVINFO)
   {
      if (pChannel->pTapiDevice->error.nCnt < IFX_TAPI_MAX_ERROR_ENTRIES - 1)
      {
         memcpy (&pChannel->pTapiDevice->error.stack[pChannel->pTapiDevice->error.nCnt++],
                 (IFX_TAPI_ErrorLine_t*)pTapiEvent->data.error,
                 sizeof (IFX_TAPI_ErrorLine_t));
         pChannel->pTapiDevice->error.nCh = pChannel->nChannel;
         pChannel->pTapiDevice->error.nCode = pTapiEvent->data.error->nLlCode;
      }
      return IFX_SUCCESS;
   }

   /* for testing purpose start */
   if (pIFX_TAPI_BP_Event == IFX_NULL)
   {
      /* event dispatch not inited so init it */
      IFX_TAPI_EventDispatcher_Init (IFX_NULL);
   }
   /* for testing purpose end */

   /* global irq lock - multiple drivers may be loaded and all share this
      event dispatch function. Lock access to the shared buffer pool. */
   if (!IFXOS_IN_INTERRUPT())
      IFXOS_MutexLock (semBufferPoolAcc);
   IFXOS_LOCKINT(lock);

   pEvt = (IFX_TAPI_EVENT_t *) bufferPoolGet (pIFX_TAPI_BP_Event);
   if (pEvt == IFX_NULL)
   {
      IFXOS_UNLOCKINT(lock);
      if (!IFXOS_IN_INTERRUPT())
         IFXOS_MutexUnlock (semBufferPoolAcc);
      return IFX_ERROR;
   }
   pParam = (IFX_TAPI_EXT_EVENT_PARAM_t *)
                               bufferPoolGet (pIFX_TAPI_BP_Deferred_Event);
   if (pParam == IFX_NULL)
   {
      /* if no deferred_event buffer is free - discard the event as well :-/ */
      bufferPoolPut (pEvt);
      IFXOS_UNLOCKINT(lock);
      if (!IFXOS_IN_INTERRUPT())
         IFXOS_MutexUnlock (semBufferPoolAcc);
      return IFX_ERROR;
   }

   IFXOS_UNLOCKINT(lock);
   if (!IFXOS_IN_INTERRUPT())
      IFXOS_MutexUnlock (semBufferPoolAcc);

   memcpy (pEvt, pTapiEvent, sizeof (IFX_TAPI_EVENT_t));
   pParam->pChannel = pChannel;
   pParam->pTapiEvent = pEvt;

#ifdef TAPI_POLL
   if (TAPI_POLLING_MODE_EVENTS ==
       (pChannel->pTapiDevice->fWorkingMode & TAPI_POLLING_MODE_EVENTS))
   {
      ret = IFX_TAPI_Event_Dispatch_ProcessCtx(pParam);
   }
   else
#endif /* TAPI_POLL */
   {
      ret = TAPI_DeferWork((IFX_void_t *) IFX_TAPI_Event_Dispatch_ProcessCtx,
                           (IFX_void_t *) pParam);

      if (ret != IFX_SUCCESS)
      {
         /* deferring failed - return the event and deferred_event data
            structures to the bufferpool */
         if (!IFXOS_IN_INTERRUPT())
            IFXOS_MutexLock (semBufferPoolAcc);
         IFXOS_LOCKINT(lock);
         bufferPoolPut(pParam->pTapiEvent);
         bufferPoolPut(pParam);
         IFXOS_UNLOCKINT(lock);
         if (!IFXOS_IN_INTERRUPT())
            IFXOS_MutexUnlock (semBufferPoolAcc);
      }
   }

   return ret;
}


/**
   High Level Event Dispatcher task.

   \param pParam - parameters with task structure and detected event.

   \return IFX_SUCCESS on ok, otherwise IFX_ERROR

   \remark
   This function is spawned as a task to dispatch all events from process
   context. It is the single resource of event processing. The buffers it
   uses are allocated in IFX_TAPI_Event_Dispatch above which is called
   from interrupt or task context. So locking needs to be done during
   all buffer operations. Multiple drivers may be loaded which all can
   generate events. Instead of locking each driver separately we lock all
   interrupts globally. Additionally semaphores are needed to lock the
   access to the buffers and fifos.
 */
IFX_int32_t IFX_TAPI_Event_Dispatch_ProcessCtx(IFX_TAPI_EXT_EVENT_PARAM_t*
                                                 pParam)
{
   IFX_int32_t ret = IFX_SUCCESS;
   IFX_uint32_t eventDisabled = IFX_TRUE;
   TAPI_CHANNEL* pChannel = pParam->pChannel;
   IFX_TAPI_EVENT_t* pEvent = pParam->pTapiEvent;
   IFX_TAPI_DRV_CTX_t *pDrvCtx = IFX_NULL;
   IFX_TAPI_EVENT_DISABLE_t *pEventMask;
   IFXOS_INTSTAT lock;

   /* global irq lock for all accesses to the bufferpool */
   IFXOS_MutexLock (semBufferPoolAcc);
   IFXOS_LOCKINT(lock);
   /* data was copied so free the buffer for the wrapper struct */
   ret = bufferPoolPut (pParam);
   IFXOS_UNLOCKINT(lock);
   IFXOS_MutexUnlock (semBufferPoolAcc);

#ifdef TAPI_EXT_KEYPAD
   /* in case of inca2 it will be NULL coz no chnl context when an external
      keypad reports an event. Channel is mapped to 0th channel */
   if (pChannel == IFX_NULL)
   {
      /* STBD : Find channel context */
      pChannel = TAPI_Get_Channel_Ctx ();
   }
#endif /* TAPI_EXT_KEYPAD */
   if (pChannel != IFX_NULL)
   {
      pDrvCtx = (IFX_TAPI_DRV_CTX_t *) pChannel->pTapiDevice->pDevDrvCtx;
      pEvent->ch = (IFX_uint16_t) pChannel->nChannel;
   }
   else
   {
      /* this should not happen - discard event and exit now */
      IFXOS_MutexLock (semBufferPoolAcc);
      IFXOS_LOCKINT(lock);
      ret = bufferPoolPut ((void *) pEvent);
      IFXOS_UNLOCKINT(lock);
      IFXOS_MutexUnlock (semBufferPoolAcc);
      return ret;
   }
   /* at this point pChannel is now valid */

   pEventMask = &(pChannel->eventHandler.eventDisable);
   /**\todo log event with new log type:
    LOG_STR (pEvent->ch, "Evt ID 0x%X DATA 0x%X", pEvent->id, pEvent->data.value);
    */

   /* Errors are handled first and put into the high-priority fifo.
      Normal events are handled afterwards and go to the low-priority fifo. */
   if ((pEvent->id & IFX_TAPI_EVENT_TYPE_FAULT_MASK) == 0xF0000000)
   {
      /*
       * If any fault event occured, invoke the error state machine.
       *  Then, put this event into the high-priority FIFO.
       */
      switch (pEvent->id & IFX_TAPI_EVENT_TYPE_MASK)
      {
      case IFX_TAPI_EVENT_TYPE_FAULT_GENERAL:
         {
            int src = pEvent->data.value & IFX_TAPI_ERRSRC_MASK;
            /* clear error clasification */
            pEvent->data.value &= ~(IFX_TAPI_ERRSRC_MASK);
            switch (src)
            {
               case  IFX_TAPI_ERRSRC_LL_DEV:
                  /**\todo put in device fifo */
                  pEvent->ch = IFX_TAPI_DEVICE_CH_NUMBER;
                  /* fall through */
               case  IFX_TAPI_ERRSRC_LL_CH:
                  pEvent->data.value |= IFX_TAPI_ERRSRC_LL;
                  break;
               case  IFX_TAPI_ERRSRC_TAPI_CH:
               case  IFX_TAPI_ERRSRC_TAPI_DEV:
               default:
               /* do nothing */
                  break;
            }
            eventDisabled = IFX_FALSE;
         }
         break;
      case IFX_TAPI_EVENT_TYPE_FAULT_LINE:
         switch (pEvent->id)
         {
         case IFX_TAPI_EVENT_FAULT_LINE_OVERTEMP:
            pChannel->TapiOpControlData.nLineMode = IFX_TAPI_LINE_FEED_DISABLED;
            eventDisabled = pEventMask->fault_line.bits.overtemp ?
                            IFX_TRUE : IFX_FALSE;
            if ( !eventDisabled )
               pChannel->TapiMiscData.nException.Bits.otemp = IFX_TRUE;
            break;
         case IFX_TAPI_EVENT_FAULT_LINE_GK_HIGH:
            TAPI_Phone_Event_GNKH (pChannel);
            eventDisabled = pEventMask->fault_line.bits.gk_high ?
                            IFX_TRUE : IFX_FALSE;
            break;
         case IFX_TAPI_EVENT_FAULT_LINE_GK_LOW:
            /* set the TAPI line mode to power down. The TAPI will not signal
               hook changes to the client application anymore till the mode is
               changed again */
            pChannel->TapiOpControlData.bFaulCond = IFX_TRUE;
            eventDisabled = pEventMask->fault_line.bits.gk_low ?
                            IFX_TRUE : IFX_FALSE;
            break;
         case IFX_TAPI_EVENT_FAULT_LINE_GK_POS:
            eventDisabled = pEventMask->fault_line.bits.gk_pos ?
                            IFX_TRUE : IFX_FALSE;
            if ( !eventDisabled )
               pChannel->TapiMiscData.nException.Bits.ground_key_polarity = IFX_TRUE;
            break;
         case IFX_TAPI_EVENT_FAULT_LINE_GK_NEG:
            eventDisabled = pEventMask->fault_line.bits.gk_neg ?
                            IFX_TRUE : IFX_FALSE;
            /* \todo */
            break;
         default:
         /* do nothing */
            break;
         }
         break;
      case IFX_TAPI_EVENT_TYPE_FAULT_HW:
         switch (pEvent->id)
         {
         case IFX_TAPI_EVENT_FAULT_HW_CLOCK_FAIL:
            eventDisabled = pEventMask->fault_hw.bits.clock_fail ?
                            IFX_TRUE : IFX_FALSE;
            break;
         case IFX_TAPI_EVENT_FAULT_HW_CLOCK_FAIL_END:
            eventDisabled = pEventMask->fault_hw.bits.clock_fail_end ?
                            IFX_TRUE : IFX_FALSE;
            break;
         case IFX_TAPI_EVENT_FAULT_HW_FAULT:
            eventDisabled = pEventMask->fault_hw.bits.hw_fault ?
                            IFX_TRUE : IFX_FALSE;
            break;
         default:
         /* do nothing */
            break;
         }
         break;
      case IFX_TAPI_EVENT_TYPE_FAULT_FW:
         break;
      case IFX_TAPI_EVENT_TYPE_FAULT_SW:
         break;
      default:
         break;
      }

      if (eventDisabled == IFX_FALSE)
      {
         /* Lock the fifo access protection */
         IFXOS_MutexLock(pChannel->eventHandler.fifoAcc);
         /* The event is not masked. So put it into the high priority fifo
            and wake up the application waiting on the filedescriptor */
         ret = fifoPut (pChannel->eventHandler.pTapiEventFifoHi,
                        (void *) pEvent, sizeof (IFX_TAPI_EVENT_t));
         TAPI_WakeUp (pChannel->pTapiDevice);
         /* Unlock the fifo access protection */
         IFXOS_MutexUnlock(pChannel->eventHandler.fifoAcc);
      }
      else
      {
         /* The event is masked. Just release the buffer and forget it. */
         IFXOS_MutexLock (semBufferPoolAcc);
         IFXOS_LOCKINT(lock);
         ret = bufferPoolPut ((void *) pEvent);
         IFXOS_UNLOCKINT(lock);
         IFXOS_MutexUnlock (semBufferPoolAcc);
      }
   }
   else
   {
      /*
       * Normal events, put these into low-priority FIFO
       */
      switch (pEvent->id)
      {
      /* EVENT TYPE: -- IFX_TAPI_EVENT_TYPE_NONE -- */

      /* EVENT TYPE: -- IFX_TAPI_EVENT_TYPE_IO_GENERAL -- */

      /* EVENT TYPE: -- IFX_TAPI_EVENT_TYPE_INTERRUPT -- */

      /* EVENT TYPE: -- IFX_TAPI_EVENT_TYPE_FXS -- */

      case IFX_TAPI_EVENT_FXS_ONHOOK_INT:
      case IFX_TAPI_EVENT_FXS_OFFHOOK_INT:
         {
#ifdef TAPI_CID
            IFX_boolean_t   bSendHookEvent = IFX_TRUE;
            TAPI_CHANNEL *pTapiDataCh;
            IFX_uint8_t data_ch;

            /* get tapi data channel for operation on data channel */
            TAPI_Phone_Get_Data_Channel (pChannel, &data_ch);
            pTapiDataCh = &pChannel->pTapiDevice->pTapiChanelArray[data_ch];

            /* During running NTT transmissions on hook/off hook is used as a
               special signal. In this case set a flag for CID and suppress
               the hook event.*/
            if ((pTapiDataCh->TapiCidTx.bActive == IFX_TRUE) &&
                (pTapiDataCh->TapiCidConf.nStandard == IFX_TAPI_CID_STD_NTT))
            {
               /* off hook ("primary answer signal") is flagged in bAck */
               if ((pEvent->id == IFX_TAPI_EVENT_FXS_OFFHOOK_INT) &&
                   (pTapiDataCh->TapiCidTx.bAck == IFX_FALSE))
               {
                  pTapiDataCh->TapiCidTx.bAck = IFX_TRUE;
                  bSendHookEvent = IFX_FALSE;
               }
               /* on hook ("incoming successful signal") is flagged in bAck2 */
               if ((pEvent->id == IFX_TAPI_EVENT_FXS_ONHOOK_INT) &&
                   (pTapiDataCh->TapiCidTx.bAck2 == IFX_FALSE))
               {
                  pTapiDataCh->TapiCidTx.bAck2 = IFX_TRUE;
                  bSendHookEvent = IFX_FALSE;
               }
            }
            if (bSendHookEvent == IFX_TRUE)
#endif /* TAPI_CID */
            {
               /* stop ringing (including an optional CID transmission) */
               IFX_TAPI_Ring_Stop(pChannel);
               /* raise TAPI exception */
               TAPI_Phone_Event_HookState (pChannel,
                                           ((pEvent->id ==
                                             IFX_TAPI_EVENT_FXS_OFFHOOK_INT) ?
                                            IFX_TRUE : IFX_FALSE));
            }
            /* We drop the internal hook events. The hook state machine will
               generate new events from this. Depending on the timing we get
               onhook, offhook, flashhook or pulsedialing digits */
            eventDisabled = IFX_TRUE;
         }
         break;

      case IFX_TAPI_EVENT_FXS_ONHOOK:
         /* IFX_FALSE => nature of hook event is ONhook */
         pChannel->TapiOpControlData.bHookState = IFX_FALSE;
         pChannel->TapiDialData.nHookState = TAPI_HOOK_STATE_ONHOOK;
         eventDisabled = pEventMask->fxs.bits.onhook ? IFX_TRUE : IFX_FALSE;
         break;

      case IFX_TAPI_EVENT_FXS_OFFHOOK:
         /* IFX_TRUE => nature of hook event is OFFhook */
         pChannel->TapiOpControlData.bHookState = IFX_TRUE;
         pChannel->TapiDialData.nHookState = TAPI_HOOK_STATE_OFFHOOK;
         /* stop blocking ring */
         IFXOS_WakeUpEvent (pChannel->TapiRingEvent);
         eventDisabled = pEventMask->fxs.bits.offhook ? IFX_TRUE : IFX_FALSE;
         break;

      case IFX_TAPI_EVENT_FXS_FLASH:
         TAPI_Stop_Timer (pChannel->TapiDialData.DialTimerID);
         pChannel->TapiDialData.nHookState = TAPI_HOOK_STATE_OFFHOOK;
         eventDisabled = pEventMask->fxs.bits.flash ? IFX_TRUE : IFX_FALSE;
         break;

      case IFX_TAPI_EVENT_FXS_RINGING_END:
         eventDisabled = pEventMask->fxs.bits.ringing_end ?
                         IFX_TRUE : IFX_FALSE;
         break;

      /* EVENT TYPE: -- IFX_TAPI_EVENT_TYPE_FXO -- */
      case IFX_TAPI_EVENT_FXO_RING_START:
         eventDisabled = IFX_FALSE;
         break;
      case IFX_TAPI_EVENT_FXO_RING_STOP:
         eventDisabled = IFX_FALSE;
         break;

      case IFX_TAPI_EVENT_FXO_BAT_FEEDED:
         eventDisabled = IFX_FALSE;
         break;

      case IFX_TAPI_EVENT_FXO_BAT_DROPPED:
         eventDisabled = IFX_FALSE;
         break;

      case IFX_TAPI_EVENT_FXO_OSI:
         eventDisabled = IFX_FALSE;
         break;

      case IFX_TAPI_EVENT_FXO_POLARITY:
         eventDisabled = IFX_FALSE;
         break;

      case IFX_TAPI_EVENT_FXO_APOH:
         eventDisabled = IFX_FALSE;
         break;
      case IFX_TAPI_EVENT_FXO_NOPOH:
         eventDisabled = IFX_FALSE;
         break;


      /* EVENT TYPE: -- IFX_TAPI_EVENT_TYPE_LT -- */

#ifdef TAPI_GR909
      case IFX_TAPI_EVENT_LT_GR909_RDY:
         pChannel->TapiOpControlData.nLineMode = IFX_TAPI_LINE_FEED_DISABLED;
         eventDisabled = pEventMask->lt.bits.gr909_rdy ? IFX_TRUE : IFX_FALSE;
         break;
#endif

      /* EVENT TYPE: -- IFX_TAPI_EVENT_TYPE_PULSE -- */

      case IFX_TAPI_EVENT_PULSE_DIGIT:
         pChannel->TapiDialData.state.bProbablyFlash = IFX_FALSE;
         /* reset the pulse counter, ready for next digit */
         pChannel->TapiDialData.nHookChanges = 0;
         eventDisabled = pEventMask->pulse.bits.digit ? IFX_TRUE : IFX_FALSE;
         break;

      /* EVENT TYPE: -- IFX_TAPI_EVENT_TYPE_DTMF -- */

      case IFX_TAPI_EVENT_DTMF_DIGIT:
         {
#ifdef TAPI_CID
            if ((pChannel->TapiCidTx.bActive == IFX_TRUE) &&
                (pChannel->TapiCidTx.ackToneCode == pEvent->data.dtmf.ascii))
            {
               pChannel->TapiCidTx.bAck = IFX_TRUE;
               eventDisabled = IFX_TRUE;
            }
            else
#endif /* TAPI_CID */
            {
#ifdef TAPI_DTMF
               if (pEvent->data.dtmf.local)
               {
                  eventDisabled = pEventMask->dtmf.bits.digit_local ?
                                  IFX_TRUE : IFX_FALSE;
               }
               else if (pEvent->data.dtmf.network)
               {
                  eventDisabled = pEventMask->dtmf.bits.digit_network ?
                                  IFX_TRUE : IFX_FALSE;
               }
#endif /* TAPI_DTMF */
            }
         }
         break;

      /* EVENT TYPE: -- IFX_TAPI_EVENT_TYPE_CID -- */

#ifdef TAPI_CID
      case IFX_TAPI_EVENT_CID_RX_END:
         pChannel->TapiCidRx.stat.nStatus = IFX_TAPI_CID_RX_STATE_DATA_READY;
         eventDisabled = pEventMask->cid.bits.rx_end ? IFX_TRUE : IFX_FALSE;
         if ( !eventDisabled )
            pChannel->TapiMiscData.nException.Bits.cidrx_supdate = IFX_TRUE;
         break;

      case IFX_TAPI_EVENT_CID_RX_ERROR_READ:
         eventDisabled = pEventMask->cid.bits.rx_err_read ?
                         IFX_TRUE : IFX_FALSE;
         if ( !eventDisabled )
            pChannel->TapiMiscData.nException.Bits.cidrx_supdate = IFX_TRUE;
         break;

      case IFX_TAPI_EVENT_CID_TX_INFO_END:
         /* no event while in a CID sequence with the statemachine */
         if (pChannel->TapiCidTx.bUseSequence == IFX_FALSE)
         {
         eventDisabled = pEventMask->cid.bits.tx_info_end ?
                         IFX_TRUE : IFX_FALSE;
#ifdef ENABLE_OBSOLETE_BITFIELD_EVENT_INTERFACE
         if ( !eventDisabled )
            TAPI_Signal_Event_Update (pChannel, IFX_TAPI_SIG_CIDENDTX,
                                                IFX_TAPI_SIG_EXT_NONE);
#endif /* ENABLE_OBSOLETE_BITFIELD_EVENT_INTERFACE */
         }
         break;

      case IFX_TAPI_EVENT_CID_TX_SEQ_END:
         eventDisabled = pEventMask->cid.bits.tx_seq_end ?
                         IFX_TRUE : IFX_FALSE;
#ifdef ENABLE_OBSOLETE_BITFIELD_EVENT_INTERFACE
         if ( !eventDisabled )
            TAPI_Signal_Event_Update (pChannel, IFX_TAPI_SIG_CIDENDTX,
                                                IFX_TAPI_SIG_EXT_NONE);
#endif /* ENABLE_OBSOLETE_BITFIELD_EVENT_INTERFACE */
         break;

      case IFX_TAPI_EVENT_CID_TX_RINGCAD_ERR:
         eventDisabled = pEventMask->cid.bits.tx_ringcad_err ?
                         IFX_TRUE : IFX_FALSE;
         if ( !eventDisabled )
         {
            pChannel->TapiMiscData.error |= IFX_TAPI_RT_ERROR_RINGCADENCE_CIDTX;
            pChannel->TapiMiscData.nException.Bits.runtime_error = 1;
         }
         break;

      case IFX_TAPI_EVENT_CID_TX_NOACK_ERR:
         eventDisabled = pEventMask->cid.bits.tx_noack_err ?
                         IFX_TRUE : IFX_FALSE;
         if ( !eventDisabled )
         {
            pChannel->TapiMiscData.error |= IFX_TAPI_RT_ERROR_CIDTX_NOACK;
            pChannel->TapiMiscData.nException.Bits.runtime_error = 1;
         }
         break;

      case IFX_TAPI_EVENT_CID_TX_NOACK2_ERR:
         eventDisabled = pEventMask->cid.bits.tx_noack_err ?
                         IFX_TRUE : IFX_FALSE;
         if ( !eventDisabled )
         {
            pChannel->TapiMiscData.error |= IFX_TAPI_RT_ERROR_CIDTX_NOACK2;
            pChannel->TapiMiscData.nException.Bits.runtime_error = 1;
         }
         break;
#endif /* TAPI_CID */

      /* EVENT TYPE: -- IFX_TAPI_EVENT_TYPE_TONE_GEN -- */

      case IFX_TAPI_EVENT_TONE_GEN_END:
         {
            eventDisabled = IFX_TRUE;
            /* check event masks */
            if ((pEvent->data.tone_gen.local) &&
               (!pEventMask->tone_gen.bits.end_local))
               eventDisabled = IFX_FALSE;
            if ((pEvent->data.tone_gen.network) &&
               (!pEventMask->tone_gen.bits.end_network))
               eventDisabled = IFX_FALSE;
         }
         break;

      /* EVENT TYPE: -- IFX_TAPI_EVENT_TYPE_TONE_GEN_RAW -- */
      /*                not available to the application    */
      case IFX_TAPI_EVENT_TONE_GEN_END_RAW:
         {
            TAPI_Tone_Step_Completed (pChannel, pEvent->data.value);
            eventDisabled = IFX_TRUE;
         }
         break;

      /* EVENT TYPE: -- IFX_TAPI_EVENT_TYPE_TONE_DET -- */

      case IFX_TAPI_EVENT_TONE_DET_CPT:
         eventDisabled = pEventMask->tone_det.bits.cpt ? IFX_TRUE : IFX_FALSE;
#ifdef ENABLE_OBSOLETE_BITFIELD_EVENT_INTERFACE
         if ( !eventDisabled )
            TAPI_Signal_Event_Update (pChannel, IFX_TAPI_SIG_CPTD,
                                                IFX_TAPI_SIG_EXT_NONE);
#endif /* ENABLE_OBSOLETE_BITFIELD_EVENT_INTERFACE */
         break;

      case IFX_TAPI_EVENT_TONE_DET_RECEIVE:
         eventDisabled = pEventMask->tone_det.bits.receive ?
                         IFX_TRUE : IFX_FALSE;
#ifdef ENABLE_OBSOLETE_BITFIELD_EVENT_INTERFACE
         if ( !eventDisabled )
            TAPI_Signal_Event_Update (pChannel, pEvent->data.value,
                                                IFX_TAPI_SIG_EXT_NONE);
#endif /* ENABLE_OBSOLETE_BITFIELD_EVENT_INTERFACE */
         break;

      case IFX_TAPI_EVENT_TONE_DET_TRANSMIT:
         eventDisabled = pEventMask->tone_det.bits.transmit ?
                         IFX_TRUE : IFX_FALSE;
#ifdef ENABLE_OBSOLETE_BITFIELD_EVENT_INTERFACE
         if ( !eventDisabled )
            TAPI_Signal_Event_Update (pChannel, pEvent->data.value,
                                                IFX_TAPI_SIG_EXT_NONE);
#endif /* ENABLE_OBSOLETE_BITFIELD_EVENT_INTERFACE */
         break;

      /* EVENT TYPE: -- IFX_TAPI_EVENT_TYPE_FAXMODEM_SIGNAL -- */

      case  IFX_TAPI_EVENT_FAXMODEM_AM:
      case  IFX_TAPI_EVENT_FAXMODEM_CED:
      case  IFX_TAPI_EVENT_FAXMODEM_CEDEND:
      case  IFX_TAPI_EVENT_FAXMODEM_DIS:
      case  IFX_TAPI_EVENT_FAXMODEM_PR:
      case  IFX_TAPI_EVENT_FAXMODEM_CNGFAX:
      case  IFX_TAPI_EVENT_FAXMODEM_CNGMOD:
      case  IFX_TAPI_EVENT_FAXMODEM_V21L:
      case  IFX_TAPI_EVENT_FAXMODEM_V18A:
      case  IFX_TAPI_EVENT_FAXMODEM_V27:
      case  IFX_TAPI_EVENT_FAXMODEM_BELL:
      case  IFX_TAPI_EVENT_FAXMODEM_V22:
      case  IFX_TAPI_EVENT_FAXMODEM_V22ORBELL:
      case  IFX_TAPI_EVENT_FAXMODEM_V32AC:
      case  IFX_TAPI_EVENT_FAXMODEM_V8BIS:
      case  IFX_TAPI_EVENT_FAXMODEM_HOLDEND:
      case IFX_TAPI_EVENT_FAXMODEM_CAS_BELL:
      case IFX_TAPI_EVENT_FAXMODEM_V21H:
      case IFX_TAPI_EVENT_FAXMODEM_VMD:
         eventDisabled = Check_FaxModem_Status (pChannel, pEvent);
#ifdef ENABLE_OBSOLETE_BITFIELD_EVENT_INTERFACE
         if ( !eventDisabled )
            Signal_CompEvent (pChannel, pEvent);
#endif /* ENABLE_OBSOLETE_BITFIELD_EVENT_INTERFACE */
         break;

      /* EVENT TYPE: -- IFX_TAPI_EVENT_TYPE_CODER -- */

      case IFX_TAPI_EVENT_COD_DEC_CHG:
         {
            IFX_TAPI_DEC_DETAILS_t dec_details;
            memset (&dec_details, 0, sizeof(IFX_TAPI_DEC_DETAILS_t));
            if (ptr_chk(pDrvCtx->COD.DEC_Chg_Evt_Detail_Req,
                       "pDrvCtx->COD.DEC_Chg_Evt_Detail_Req"))
            {
               ret = pDrvCtx->COD.DEC_Chg_Evt_Detail_Req(pChannel->pLLChannel, &dec_details);
               if (ret == IFX_SUCCESS)
               {
                  pEvent->data.dec_chg.dec_type        = dec_details.dec_type;
                  pEvent->data.dec_chg.dec_framelength = dec_details.dec_framelength;
               }
            }
            eventDisabled = pEventMask->coder.bits.dec_chg ? IFX_TRUE : IFX_FALSE;
         }
         break;

      case IFX_TAPI_EVENT_COD_ROOM_NOISE:
         eventDisabled = pEventMask->coder.bits.room_noise ?
                         IFX_TRUE : IFX_FALSE;
         break;

      case IFX_TAPI_EVENT_COD_ROOM_SILENCE:
         eventDisabled = pEventMask->coder.bits.room_silence ?
                         IFX_TRUE : IFX_FALSE;
         break;

      /* EVENT TYPE: -- IFX_TAPI_EVENT_TYPE_RTP -- */

      /* EVENT TYPE: -- IFX_TAPI_EVENT_TYPE_AAL -- */

      /* EVENT TYPE: -- IFX_TAPI_EVENT_TYPE_RFC2883 -- */

      case IFX_TAPI_EVENT_RFC2833_EVENT:
         eventDisabled = pEventMask->rfc2833.bits.event ? IFX_TRUE : IFX_FALSE;
         if ( !eventDisabled )
         {
            pChannel->TapiMiscData.nException.Bits.eventDetect = 1;
            pChannel->TapiMiscData.event = pEvent->data.rfc2833.event;
            eventDisabled = IFX_FALSE;
         }
         break;

      /* EVENT TYPE: -- IFX_TAPI_EVENT_TYPE_T38 -- */

#ifdef TAPI_FAX_T38
      case IFX_TAPI_EVENT_T38_ERROR_OVLD:
         eventDisabled = pEventMask->t38.bits.error_ovld ? IFX_TRUE : IFX_FALSE;
         if ( !eventDisabled)
            TAPI_FaxT38_Event_Update (pChannel, IFX_TAPI_FAX_T38_TX_OFF,
                                      IFX_TAPI_FAX_T38_ERROR_MIPS_OVLD);
         break;

      case IFX_TAPI_EVENT_T38_ERROR_DATA:
         eventDisabled = pEventMask->t38.bits.error_data ? IFX_TRUE : IFX_FALSE;
         if ( !eventDisabled )
         {
            if (pEvent->data.value)
               TAPI_FaxT38_Event_Update (pChannel, IFX_TAPI_FAX_T38_TX_OFF,
                                         IFX_TAPI_FAX_T38_ERROR_DATAPUMP);
            else
               TAPI_FaxT38_Event_Update (pChannel, 0,
                                         IFX_TAPI_FAX_T38_ERROR_DATAPUMP);
         }
         break;

      case IFX_TAPI_EVENT_T38_ERROR_WRITE:
         eventDisabled = pEventMask->t38.bits.error_write ?
                         IFX_TRUE : IFX_FALSE;
         if ( !eventDisabled )
            TAPI_FaxT38_Event_Update (pChannel, 0, IFX_TAPI_FAX_T38_ERROR_WRITE);
         break;

      case IFX_TAPI_EVENT_T38_ERROR_READ:
         eventDisabled = pEventMask->t38.bits.error_read ? IFX_TRUE : IFX_FALSE;
         if ( !eventDisabled )
            TAPI_FaxT38_Event_Update (pChannel, 0, IFX_TAPI_FAX_T38_ERROR_READ);
         break;

      case IFX_TAPI_EVENT_T38_ERROR_SETUP:
         eventDisabled = pEventMask->t38.bits.error_setup ? IFX_TRUE : IFX_FALSE;
         if ( !eventDisabled )
            TAPI_FaxT38_Event_Update (pChannel, 0, IFX_TAPI_FAX_T38_ERROR_SETUP);
         break;

      case IFX_TAPI_EVENT_T38_NONE:
         TAPI_FaxT38_Event_Update (pChannel, IFX_TAPI_FAX_T38_TX_OFF,
                                   IFX_TAPI_FAX_T38_ERROR_NONE);
         eventDisabled = IFX_TRUE;
         break;

      case IFX_TAPI_EVENT_T38_FDP_REQ:
         /* FAX T38 FDP REQ; used for polling mode access only -
            currently not maskable */
         eventDisabled = IFX_FALSE;
         break;
#endif /* TAPI_FAX_T38 */

      /* EVENT TYPE: -- IFX_TAPI_EVENT_TYPE_JB -- */

      /* EVENT TYPE: -- IFX_TAPI_EVENT_TYPE_DOWNLOAD -- */

      /* EVENT TYPE: -- IFX_TAPI_EVENT_TYPE_INFORMATION -- */

      /* EVENT TYPE: -- IFX_TAPI_EVENT_TYPE_DEBUG -- */

      case IFX_TAPI_EVENT_DEBUG_CERR:
         if (ptr_chk (pDrvCtx->Dbg_CErr_Handler,
                     "pDrvCtx->Dbg_CErr_Handler"))
         {
            IFX_TAPI_DBG_CERR_t data;
            memset(&data, 0, sizeof(IFX_TAPI_DBG_CERR_t));
            if (IFX_SUCCESS ==
                pDrvCtx->Dbg_CErr_Handler(pChannel->pTapiDevice->pLLDev, &data))
            {
               pEvent->data.cerr.fw_id = 0x0001;
               pEvent->data.cerr.reason = data.cause;
               pEvent->data.cerr.command = data.cmd;
            }
         }
         /* print data if any data is available */
         if (pEvent->data.cerr.fw_id != 0x0000)
         {
            TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
                  ("\n\r!!! cErrHdlr cause 0x%04X, cHdr 0x%08X\n\r",
                   pEvent->data.cerr.reason, pEvent->data.cerr.command));
         }
         /* report this event to the application. */
         eventDisabled = IFX_FALSE;
         break;

      /* EVENT TYPE: -- IFX_TAPI_EVENT_TYPE_LL_DRIVER -- */

      case IFX_TAPI_EVENT_LL_DRIVER_WD_FAIL:
         eventDisabled =
         pEventMask->ll_driver.bits.alive ? IFX_TRUE : IFX_FALSE;
         break;

      /* EVENT TYPE: -- IFX_TAPI_EVENT_TYPE_EXT -- */

#ifdef TAPI_EXT_KEYPAD
      case IFX_TAPI_EVENT_EXT_KEY_UP:
      case IFX_TAPI_EVENT_EXT_KEY_DOWN:
         /* In case of inca, key event is given to the application by HAPI
            so TAPI does not need to report this */
         ret = TAPI_EXT_EVENT_Key_Handler (pChannel, pEvent);
         eventDisabled = IFX_TRUE;
         break;
#endif /* TAPI_EXT_KEYPAD */

      default:
         /* discard all unhandled events */
         eventDisabled = IFX_TRUE;
      }

      if (eventDisabled == IFX_FALSE)
      {
         /* Lock the fifo access protection */
         IFXOS_MutexLock(pChannel->eventHandler.fifoAcc);
         /* The event is not masked. So put it into the low priority fifo
            and wake up the application waiting on the filedescriptor */
         ret = fifoPut (pChannel->eventHandler.pTapiEventFifoLo,
                        (void *) pEvent, sizeof (IFX_TAPI_EVENT_t));
         TAPI_WakeUp (pChannel->pTapiDevice);
         /* Unlock the fifo access protection */
         IFXOS_MutexUnlock(pChannel->eventHandler.fifoAcc);
      }
      else
      {
         /* The event is masked. Just release the buffer and forget it. */
         IFXOS_MutexLock (semBufferPoolAcc);
         IFXOS_LOCKINT(lock);
         ret = bufferPoolPut ((void *) pEvent);
         IFXOS_UNLOCKINT(lock);
         IFXOS_MutexUnlock (semBufferPoolAcc);
      }
   }

   return ret;
}


/**
   Resource allocation and initialisation upon loading the driver.

   This function allocates memory for the buffer pool of events that the 
   event dispatcher transports. There are two pools: One for the events
   and one for the wrapper structures that transport the events.
   These pools are shared between all devices of this driver.

   \return
   Returns IFX_ERROR in case of an error, otherwise returns IFX_SUCCESS.
*/
IFX_int32_t IFX_TAPI_Event_On_Driver_Load(IFX_void_t)
{
   /* allocate memory pool for the events */
   if (pIFX_TAPI_BP_Event == IFX_NULL)
   {
      pIFX_TAPI_BP_Event =
         bufferPoolInit (sizeof (IFX_TAPI_EVENT_t),
                         IFX_TAPI_EVENT_POOL_INITIAL_SIZE,
                         IFX_TAPI_EVENT_POOL_GROW_SIZE);
   }

   /* allocate memory pool for the wrapper structures */
   if (pIFX_TAPI_BP_Deferred_Event == IFX_NULL)
   {
      pIFX_TAPI_BP_Deferred_Event =
         bufferPoolInit (sizeof (IFX_TAPI_EXT_EVENT_PARAM_t),
                         IFX_TAPI_EVENT_POOL_INITIAL_SIZE,
                         IFX_TAPI_EVENT_POOL_GROW_SIZE);

   }

   /* initialize buffer pool access protection semaphore */
   if ((pIFX_TAPI_BP_Event != IFX_NULL) &&
       (pIFX_TAPI_BP_Deferred_Event != IFX_NULL))
   {
      IFXOS_MutexInit (semBufferPoolAcc);
   }

   return (pIFX_TAPI_BP_Event != IFX_NULL) && 
          (pIFX_TAPI_BP_Deferred_Event != IFX_NULL) ? IFX_SUCCESS : IFX_ERROR;
}

/**
   Free resources upon unloading the driver.

   This function frees the memory for the buffer pool of events that the 
   event dispatcher transports. There are two pools: One for the events
   and one for the wrapper structures that transport the events.
*/
IFX_void_t IFX_TAPI_Event_On_Driver_Unload(IFX_void_t)
{
   IFXOS_INTSTAT lock;

   IFXOS_LOCKINT(lock);

   /* free memory pool for the events */
   if (pIFX_TAPI_BP_Event != IFX_NULL)
   {
      if (bufferPoolFree(pIFX_TAPI_BP_Event) != IFX_SUCCESS)
      {
         TRACE (TAPI_DRV, DBG_LEVEL_HIGH,
                ("WARN: Free event buffer error.\n\r"));
      }
      pIFX_TAPI_BP_Event = IFX_NULL;
   }

   /* free memory pool for the wrapper structures */
   if (pIFX_TAPI_BP_Deferred_Event != IFX_NULL)
   {
      if (bufferPoolFree (pIFX_TAPI_BP_Deferred_Event) != IFX_SUCCESS)
      {
         TRACE (TAPI_DRV, DBG_LEVEL_HIGH,
                ("WARN: Free deferred-event buffer error.\n\r"));
      }
      pIFX_TAPI_BP_Deferred_Event = IFX_NULL;
   }

   /* delete the buffer pool access protection semaphore */
   IFXOS_MutexDelete(semBufferPoolAcc);

   IFXOS_UNLOCKINT(lock);
}

/**
   High Level Event Dispatcher function init.

   \param pChannel      Pointer to tapi channel structure.

   \return IFX_SUCCESS on ok, otherwise IFX_ERROR
*/
IFX_return_t IFX_TAPI_EventDispatcher_Init (TAPI_CHANNEL * pChannel)
{
   IFX_return_t ret = IFX_SUCCESS;

   if (pChannel != IFX_NULL)
   {
      /* Initialize high and low priority FIFO struct */
      pChannel->eventHandler.pTapiEventFifoHi =
         fifoInit (IFX_TAPI_EVENT_FIFO_SIZE);
      pChannel->eventHandler.pTapiEventFifoLo =
         fifoInit (IFX_TAPI_EVENT_FIFO_SIZE);

      if (pChannel->eventHandler.pTapiEventFifoHi == NULL ||
          pChannel->eventHandler.pTapiEventFifoLo == NULL)
      {
         ret = IFX_ERROR;
      }

      /* Clear the mask to disable events. */
      memset (&pChannel->eventHandler.eventDisable, 0,
              sizeof (IFX_TAPI_EVENT_DISABLE_t));

      /* initialize fifo access protection semaphore */
      IFXOS_MutexInit (pChannel->eventHandler.fifoAcc);
   }

   return ret;
}

/**
   High Level Event Dispatcher function exit.

   \param pChannel      Pointer to tapi channel structure.

   \return IFX_SUCCESS on ok, otherwise IFX_ERROR
*/
IFX_int32_t IFX_TAPI_EventDispatcher_Exit (TAPI_CHANNEL * pChannel)
{
   IFX_int32_t ret = IFX_SUCCESS;
   void *pEvt;

   /* Lock the fifo access protection. */
   IFXOS_MutexLock (pChannel->eventHandler.fifoAcc);

   /* silently drop all events stored in high-prio fifo */
   while ((pEvt = fifoGet(pChannel->eventHandler.pTapiEventFifoHi, NULL)) != NULL)
   {
      bufferPoolPut(pEvt);
   }
   /* silently drop all events stored in low-prio fifo */
   while ((pEvt = fifoGet(pChannel->eventHandler.pTapiEventFifoLo, NULL)) != NULL)
   {
      bufferPoolPut(pEvt);
   }

   /* free the low prio fifo itself */
   if (fifoFree (pChannel->eventHandler.pTapiEventFifoHi) != IFX_SUCCESS)
   {
      TRACE (TAPI_DRV, DBG_LEVEL_HIGH,
             ("INFO: Free event hi fifo error. ch(%d)\n\r", pChannel->nChannel));
      ret = IFX_ERROR;
   }
   /* free the high prio fifo itself */
   if (fifoFree (pChannel->eventHandler.pTapiEventFifoLo) != IFX_SUCCESS)
   {
      TRACE (TAPI_DRV, DBG_LEVEL_HIGH,
             ("INFO: Free event lo fifo error. ch(%d)\n\r", pChannel->nChannel));
      ret = IFX_ERROR;
   }

   /* Unlock the protection and delete the semaphore */
   IFXOS_MutexUnlock (pChannel->eventHandler.fifoAcc);
   IFXOS_MutexDelete(pChannel->eventHandler.fifoAcc);

   return ret;
}

/**

   Generate events either as tone events played locally or as RFC2833 events

   \param pChannel -Channel context for HL TAPI.
   \param pPacketEvent - Event description.

   \return
   On success IFX_SUCCESS or else IFX_ERROR

   \remarks
   This functions is generic. This is called when user calls through ioctl to
   handle DTMF and also when other module like hapi reports the events to TAPI
*/
IFX_return_t TAPI_EVENT_PKT_EV_Generate (TAPI_CHANNEL * pChannel,
                                         IFX_TAPI_PKT_EV_GENERATE_t *pPacketEvent)
{
#ifdef TAPI_EXT_KEYPAD
/*   IFX_uint8_t   tmp = 1;  */
   IFX_uint8_t   ch = 0;
   IFX_int32_t dtmfmode;
   TAPI_CHANNEL *pChnl;
   IFX_int32_t   tone_dst = -1;            /* tone destination flag */
   IFX_boolean_t gen_rfc2833 = IFX_FALSE;  /* packet generation flag */
   IFX_int32_t   play_event;
#endif /* TAPI_EXT_KEYPAD */
   IFX_TAPI_DRV_CTX_t *pDrvCtx;

   if ((pChannel->bInitialized == IFX_FALSE) || (pPacketEvent == IFX_NULL))
   {
      return IFX_ERROR;
   }
#ifdef TAPI_EXT_KEYPAD
   if ((pChannel->nChannel > 0) || (pChannel->nAudioDataChannels == 0))
   {
      TRACE (TAPI_DRV, DBG_LEVEL_HIGH,
             ("TAPI:non supported chnl or no data chnl added to audio \n\r"));
      return IFX_ERROR;
   }
#endif /* TAPI_EXT_KEYPAD */
   pDrvCtx = (IFX_TAPI_DRV_CTX_t *) pChannel->pTapiDevice->pDevDrvCtx;

   /* Check if Event packet generation function is available */
   if (!ptr_chk (pDrvCtx->COD.RTP_EV_Generate, "pDrvCtx->COD.RTP_EV_Generate"))
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
         ("%s - COD.RTP_EV_Generate not valid\n", __FUNCTION__));
      return IFX_ERROR;
   }


#ifdef TAPI_EXT_KEYPAD

#if 0
   tmp = pChannel->nAudioDataChannels;

   /* to be clarified how event transmission for conferencing should look like */
   while (tmp)
   {
      if (tmp & 1)
      {
         break;
      }
      tmp = tmp >> 1;
      ch++;
   }
#endif
   /* ch is nothing but the channel number where tone should be played so get
      chnl context */
   pChnl = (pChannel->pTapiDevice->pTapiChanelArray) + ch;
   dtmfmode = (pChannel->nDtmfInfo & ~DTMF_EV_LOCAL_PLAY);


   /*
    * Depending on current event generation mode and local play setting
    * generate tone events to be inserted in data stream (in band) and/or
    * generate RFC2833 events (out of band).
    * If local play flag is set the event is additionally
    * played as tone on the local speaker.
    */
   switch(dtmfmode)
   {
      case IFX_TAPI_PKT_EV_OOB_DEFAULT:
      case IFX_TAPI_PKT_EV_OOB_ALL:
         /* inband and OOB packet generation requested */
         gen_rfc2833 = IFX_TRUE;
         if( pChannel->nDtmfInfo & DTMF_EV_LOCAL_PLAY )
            tone_dst = (IFX_int32_t) TAPI_TONE_DST_NETLOCAL;
         else
            tone_dst = (IFX_int32_t) TAPI_TONE_DST_NET;
         break;

      case IFX_TAPI_PKT_EV_OOB_ONLY:
         /* only OOB packets, no inband tone packets */
         gen_rfc2833 = IFX_TRUE;
         if( pChannel->nDtmfInfo & DTMF_EV_LOCAL_PLAY )
            tone_dst = (IFX_int32_t) TAPI_TONE_DST_LOCAL;
         break;

      case IFX_TAPI_PKT_EV_OOB_NO:
         /* no OOB packets, only inband tone packets */
         gen_rfc2833 = IFX_FALSE;
         if( pChannel->nDtmfInfo & DTMF_EV_LOCAL_PLAY )
            tone_dst = (IFX_int32_t) TAPI_TONE_DST_NETLOCAL;
         else
            tone_dst = (IFX_int32_t) TAPI_TONE_DST_NET;
         break;

      case IFX_TAPI_PKT_EV_OOB_BLOCK:
         /* neither OOB packets nor inband tone packets */
         gen_rfc2833 = IFX_FALSE;
         if( pChannel->nDtmfInfo & DTMF_EV_LOCAL_PLAY )
            tone_dst = (IFX_int32_t) TAPI_TONE_DST_LOCAL;
         break;
      default:
         TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
            ("%s - Invalid event mode %d\n", __FUNCTION__, dtmfmode));
         return IFX_ERROR;
   }

   if (pPacketEvent->action == IFX_TAPI_EV_GEN_START)
   {
      if( tone_dst != -1 )
      {
         /* since we are using Tone table to generate dtmfs,
            if dtmf 0 then we have use the index 11.
            It has to be removed if we are not using Tones for dtmf */
         if(pPacketEvent->event == 11)/* for # dtmfcode=11 but tone index is 12 */
         {
            play_event =12;
         }
         else if(pPacketEvent->event == 0)/*dtmf 0 means index in tone table is 11*/
         {
            play_event = 11;
         }
         else
            play_event = pPacketEvent->event;

         TAPI_Phone_Tone_Play(pDrvCtx, pChnl, play_event, tone_dst);
      }
   }
   else   /* pPacketEvent->action == IFX_TAPI_EV_GEN_STOP  */
   {
      TAPI_Phone_Tone_Stop(pDrvCtx, pChnl, 0, TAPI_TONE_DST_DEFAULT);
   }

   if( gen_rfc2833 )
   {
      /* start/stop generation of rfc 2833 pkt */
      pDrvCtx->COD.RTP_EV_Generate(pChannel->pLLChannel,
                                   pPacketEvent->event,
                                   pPacketEvent->action,
                                   pPacketEvent->duration);
   }

#if 0
   /* Incase of conferencing,Check which are the data channel added to audio
      channel,and do for all */
   /* if Local play bit is set then first coder is already played dtmf
      so move to next coderor else repeat from coder 0*/
   if (pChannel->nDtmfInfo & DTMF_EV_LOCAL_PLAY)
   {
      tmp = tmp >> 1;
      ch++;
   }

   while (tmp)
   {
      if (tmp & 1)
      {
         /* ch is nothing but the channel number where tone should be played so
            get chnl context */
         pChnl = (pChannel->pTapiDevice->pTapiChanelArray) + ch;
         /* Vmask the last bit to get the value */
         switch (dtmfmode)
         {
         case IFX_TAPI_PKT_EV_OOB_DEFAULT:
         case IFX_TAPI_PKT_EV_OOB_ALL:
            {
               /* (Transmit in band and out of band). */
               if (pPacketEvent->action == IFX_TAPI_EV_GEN_START)
               {
                  TAPI_Phone_Tone_Play (pDrvCtx, pChnl,
                                       (IFX_int32_t) pPacketEvent->event,
                                        TAPI_TONE_DST_NET);
                  /* generate rfc 2833 pkt */
                  /* TAPI_Phone_DTMF_OOB(pDrvCtx,pChnl,
                     (IFX_int32_t)pPacketEvent->event,IFX_TAPI_EV_GEN_START); */
               }
               else
               {
                  TAPI_Phone_Tone_Stop (pDrvCtx, pChnl, 0, TAPI_TONE_DST_DEFAULT);
                  /* generate rfc 2833 pkt */
                  /* TAPI_Phone_DTMF_OOB(pDrvCtx,pChnl,
                     (IFX_int32_t)pPacketEvent->event,IFX_TAPI_EV_GEN_STOP); */
               }
               break;
            }
         case IFX_TAPI_PKT_EV_OOB_ONLY:
            {
               /* Transmit only out of band. */
               if (pPacketEvent->action == IFX_TAPI_EV_GEN_START)
               {
                  /* generate rfc 2833 pkt */
                  /* TAPI_Phone_DTMF_OOB(pDrvCtx,pChnl,
                     (IFX_int32_t)pPacketEvent->event,IFX_TAPI_EV_GEN_START); */
               }
               else
               {
                  /* generate rfc 2833 pkt */
                  /* TAPI_Phone_DTMF_OOB(pDrvCtx,pChnl,
                     (IFX_int32_t)pPacketEvent->event,IFX_TAPI_EV_GEN_STOP); */
               }
               break;
            }
         case IFX_TAPI_PKT_EV_OOB_NO:
            {
               /* Transmit only in band. */
               if (pPacketEvent->action == IFX_TAPI_EV_GEN_START)
               {
                  TAPI_Phone_Tone_Play (pDrvCtx, pChnl,
                                       (IFX_int32_t) pPacketEvent->event,
                                        TAPI_TONE_DST_NET);
               }
               else
               {
                  TAPI_Phone_Tone_Stop (pDrvCtx, pChnl, 0, TAPI_TONE_DST_DEFAULT);
               }

               break;
            }
         case IFX_TAPI_PKT_EV_OOB_BLOCK:
            {
               /* neither in-band nor out-of-band */
               /* Do nothing */
               break;
            }
         }
      }                         /* end of if */
      tmp = tmp >> 1;
      ch++;
   }                            /* end of while */
#endif   /* #if 0*/
#endif /* TAPI_EXT_KEYPAD */

   return IFX_SUCCESS;
}

#ifdef TAPI_EXT_KEYPAD
/**

   This function is called by EventDispatchProcesContext.
   This will handle the external keypad DTMF events reported by other
   module like HAPI.

   \param pChannel -Channel context for HL TAPI.
   \param pTapiEvent - TAPIEvent.

   \return
   On success IFX_SUCCESS or else IFX_ERROR

   \remarks
   This functions maps the TAPI event structure to DTMF event structure
   so that a common function is used for both the events reported from
   user or from other kernel module.
*/
IFX_return_t TAPI_EXT_EVENT_Key_Handler (TAPI_CHANNEL * pChannel,
                                         IFX_TAPI_EVENT_t * pTapiEvent)
{
   IFX_return_t ret = IFX_SUCCESS;
   IFX_TAPI_PKT_EV_GENERATE_t dtmf;
   /* IFX_TAPI_EventGen_t event; */
   IFX_TAPI_DRV_CTX_t *pDrvCtx;
   IFX_int32_t state = pTapiEvent->id;
   pDrvCtx = (IFX_TAPI_DRV_CTX_t *) pChannel->pTapiDevice->pDevDrvCtx;
   memset (&dtmf, 0, sizeof (dtmf));
   dtmf.event = pTapiEvent->data.keyinfo.key;
   dtmf.duration = pTapiEvent->data.keyinfo.duration;
   TRACE (TAPI_DRV, DBG_LEVEL_LOW, ("digit = %d\n\r", dtmf.event));
   if (state == IFX_TAPI_EVENT_EXT_KEY_UP)
   {
      TRACE (TAPI_DRV, DBG_LEVEL_LOW, ("Key is released\n\r"));
      dtmf.action = IFX_TAPI_EV_GEN_STOP;
   }
   else
   {
      TRACE (TAPI_DRV, DBG_LEVEL_LOW, ("Key is pressed\n\r"));
      dtmf.action = IFX_TAPI_EV_GEN_START;
   }
   ret = TAPI_EVENT_PKT_EV_Generate (pChannel, &dtmf);
   return ret;
}

#endif /* TAPI_EXT_KEYPAD */

void TAPI_ErrorStatus (TAPI_CHANNEL * pChannel, TAPI_Status_t nHlCode,
                       IFX_int32_t nLlCode,
                       IFX_uint32_t nLine, const IFX_char_t* sFile)
{
   IFX_TAPI_ErrorLine_t* errorLine;


   if (pChannel == IFX_NULL ||
       pChannel->pTapiDevice->error.nCnt >= IFX_TAPI_MAX_ERROR_ENTRIES)
   {
      /* stack full */
      return;
   }
   errorLine = &pChannel->pTapiDevice->error.stack[
                        pChannel->pTapiDevice->error.nCnt];
   stripPathCpy (errorLine->sFile, sFile);
   if (nHlCode == IFX_ERROR)
      nHlCode = TAPI_statusClassErr | TAPI_statusClassCh;
   errorLine->nHlCode = nHlCode;
   errorLine->nLlCode = nLlCode;
   errorLine->nLine = nLine;
   pChannel->pTapiDevice->error.nCnt++;
   pChannel->pTapiDevice->error.nCode = (nHlCode << 16) | nLlCode;
}
