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

*******************************************************************************/

/**
   \file drv_tapi_misc.c
   Contains the High-level TAPI miscellaneous functions.
   Initialisation functions.
*/

/* ============================= */
/* Includes                      */
/* ============================= */
#include "drv_tapi.h"
#include "drv_tapi_ll_interface.h"
#include "drv_tapi_event.h"
#include "drv_tapi_version.h"
#include "drv_tapi_api.h"
#ifdef TAPI_CID
#include "drv_tapi_cid.h"
#endif /* TAPI_CID */

/* ============================= */
/* Local variable definition     */
/* ============================= */

static COMPLEX_TONE toneCoefficients[TAPI_MAX_TONE_CODE];

/* ============================= */
/* Local function declaration    */
/* ============================= */

static IFX_int32_t ifx_tapi_InitCh(TAPI_CHANNEL *pChannel);
static IFX_int32_t ifx_tapi_InitChTimers (TAPI_CHANNEL *pTapiCh);

/* ============================= */
/* Global functions declaration  */
/* ============================= */

extern IFX_return_t TAPI_Phone_Tone_Predef_Config (COMPLEX_TONE* pToneTable);

/* ============================= */
/* Global function definition    */
/* ============================= */

/**
   Init the TAPI. This function is channel specific but the init is for the
   complete TAPI.

   \param pDrvCtx     - pointer to the low-level device driver context
   \param pChannel    - handle to TAPI_CHANNEL structure
   \param pInit       - handle to IFX_TAPI_CH_INIT_t structure

   \return
   error code: IFX_SUCCESS  -> init successful
               IFX_ERROR    -> init not successful
*/
IFX_int32_t TAPI_Phone_Init(IFX_TAPI_DRV_CTX_t* pDrvCtx, 
                            TAPI_CHANNEL *pChannel,
                            IFX_TAPI_CH_INIT_t const *pInit)
{
   TAPI_DEV            *pTapiDev;
   IFX_TAPI_CH_INIT_t   Init;
   IFX_int32_t          err = IFX_SUCCESS, i;

   pTapiDev = pChannel->pTapiDevice;

   /* number of tapi channel supported must be checked
      when first channel is opened. Check this. */
   if (pTapiDev->nMaxChannel == 0)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
          ("\n\rDRV_ERROR: TAPI maximum channels supported not set\n\r"));
      return IFX_ERROR;
   }
   if (!pChannel->bInitialized)
   {
      pTapiDev->pToneTbl = &toneCoefficients[0];
      /* Configure predefined tones, to be done only once because
         tones are valid for whole device */
      if (TAPI_Phone_Tone_Predef_Config (pTapiDev->pToneTbl) == IFX_ERROR)
            return IFX_ERROR;
   }

   if (pTapiDev->PwrSaveTimerID == 0)
   {
      /* initialize (create) timer for power saving feature */
      pTapiDev->PwrSaveTimerID =
         TAPI_Create_Timer((TIMER_ENTRY)TAPI_Power_Save_OnTimer,
                           (IFX_int32_t)pDrvCtx);
      if (pTapiDev->PwrSaveTimerID == 0)
      {
         TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
             ("TAPI_ERROR: Could not create timer for power saving feature\n\r"));
         return IFX_ERROR;
      }
   }

   /* we here initialize all channel member, if not already done.
      They could already be used for events, even if no channel specific
      initialization was done  */
   for (i = 0; i < pTapiDev->nMaxChannel; i++)
   {
      if (pTapiDev->pTapiChanelArray[i].bInitialized == IFX_FALSE)
      {
         err = ifx_tapi_InitCh(&pTapiDev->pTapiChanelArray[i]);
         if (err != IFX_SUCCESS)
            break;
      }
   }

   /* do low level initialization now */
   if (err == IFX_SUCCESS)
   {
      /* check if Init ptr is valid and set it if not. */
      if (pInit == NULL)
      {
         Init.nCountry = IFX_TAPI_INIT_COUNTRY_DEFAULT;
         Init.nMode    = IFX_TAPI_INIT_MODE_DEFAULT;
         Init.pProc    = IFX_NULL;
         pInit         = &Init;
      }

      /* this will also download the firmware */
      if (ptr_chk(pDrvCtx->Init_Dev, "pDrvCtx->Init_Dev"))
         err = pDrvCtx->Init_Dev (pChannel, pInit);

      /* initialize channel timers */
      ifx_tapi_InitChTimers (pChannel);
   }

   /* +++++ from here on the firmware is available +++++ */

   /* configure default PTs if LL device supports codecs */
   if (err == IFX_SUCCESS)
   {
      if (pDrvCtx->COD.RTP_PayloadTable_Cfg != IFX_NULL)
      {
         err = IFX_TAPI_PKT_RTP_PT_Defaults(pChannel);
      }
   }

   /* set timer to check if LL power save feature can be activated */
   TAPI_SetTime_Timer (pTapiDev->PwrSaveTimerID, 2000, IFX_TRUE, IFX_FALSE);

#ifdef ENABLE_OBSOLETE_LEC_ACTIVATION
   {
      /* preconfigure LEC in NE mode on + NLP on for this ALM */
      IFX_TAPI_LEC_CFG_t LecConf;
      LecConf.nGainIn  = IFX_TAPI_LEC_GAIN_MEDIUM;
      LecConf.nGainOut = IFX_TAPI_LEC_GAIN_MEDIUM;
      LecConf.nLen     = IFX_TAPI_LEC_LEN_MAX;
      LecConf.bNlp     = IFX_TAPI_LEC_NLP_ON;
      TAPI_Phone_LecConf_Alm(pChannel, &LecConf);
   }
#endif /* ENABLE_OBSOLETE_LEC_ACTIVATION */

   return (err);
}

/**
   Init one TAPI channel.

   \param pChannel      Pointer to TAPI_CHANNEL structure.

   \return:
    error code: IFX_SUCCESS  -> init successful
                IFX_ERROR    -> init not successful
*/
static IFX_int32_t ifx_tapi_InitCh(TAPI_CHANNEL *pChannel)
{
   IFX_int32_t i;
   IFX_TAPI_EXCEPTION_t ex;

   IFXOS_MutexInit (pChannel->semTapiChDataLock);
   IFXOS_InitEvent (pChannel->TapiRingEvent);

   if (IFX_TAPI_Ring_Initialise(pChannel) != IFX_SUCCESS)
      goto error;

   /* clear exception mask -> enable exceptions */
   ex.Status = 0;
   ex.Bits.feedLowBatt = 1;
   TAPI_Phone_Mask_Exception (pChannel, ex.Status);
#ifdef TAPI_CID
   if (TAPI_Phone_CID_SetDefaultConfig(pChannel) == IFX_ERROR)
      goto error;
#endif /* TAPI_CID */

   if (pChannel->TapiDialData.DialTimerID == 0)
   {
      /* initialize (create) dial timer */
      pChannel->TapiDialData.DialTimerID =
         TAPI_Create_Timer((TIMER_ENTRY)TAPI_Phone_Dial_OnTimer, (IFX_int32_t)pChannel);
      if (pChannel->TapiDialData.DialTimerID == 0)
         goto error;
   }
   if (pChannel->TapiMiscData.GndkhTimerID == 0)
   {
      /* initialize (create) dial timer */
      pChannel->TapiMiscData.GndkhTimerID =
         TAPI_Create_Timer((TIMER_ENTRY)TAPI_Gndkh_OnTimer, (IFX_int32_t)pChannel);
      if (pChannel->TapiMiscData.GndkhTimerID == 0)
         goto error;
   }
   if (pChannel->TapiMeterData.MeterTimerID == 0)
   {
      /* initialize (create) metering timer */
      pChannel->TapiMeterData.MeterTimerID =
         TAPI_Create_Timer((TIMER_ENTRY)TAPI_Phone_Meter_OnTimer, (IFX_int32_t)pChannel);
      if(pChannel->TapiMeterData.MeterTimerID == 0)
         goto error;
   }
#ifdef TAPI_CID
   if (pChannel->TapiCidTx.CidTimerID == 0)
   {
      /* initialize (create) cid timer */
      pChannel->TapiCidTx.CidTimerID =
         TAPI_Create_Timer((TIMER_ENTRY)TAPI_Phone_CID_OnTimer, (IFX_int32_t)pChannel);
      if(pChannel->TapiCidTx.CidTimerID == 0)
         goto error;
   }
   /* by default, assume phone and data channel are the same. In case of other
      mappings, this will be changed at the runtime */
   pChannel->TapiCidTx.pPhoneCh = pChannel;
#endif /* TAPI_CID */

   /* Set defaults for validation timers */
   pChannel->TapiDigitLowTime.nMinTime   = TAPI_MIN_DIGIT_LOW;
   pChannel->TapiDigitLowTime.nMaxTime   = TAPI_MAX_DIGIT_LOW;
   pChannel->TapiDigitHighTime.nMinTime  = TAPI_MIN_DIGIT_HIGH;
   pChannel->TapiDigitHighTime.nMaxTime  = TAPI_MAX_DIGIT_HIGH;
   pChannel->TapiHookFlashTime.nMinTime  = TAPI_MIN_FLASH;
   pChannel->TapiHookFlashTime.nMaxTime  = TAPI_MAX_FLASH;
   pChannel->TapiHookFlashMakeTime.nMinTime  = TAPI_MIN_FLASH_MAKE;
   pChannel->TapiInterDigitTime.nMinTime = TAPI_MIN_INTERDIGIT;
   pChannel->TapiHookOffTime.nMinTime    = TAPI_MIN_OFF_HOOK;
   pChannel->TapiHookOnTime.nMinTime     = TAPI_MIN_ON_HOOK;
   /* Start hookstate FSM in onhook state */
   pChannel->TapiDialData.nHookState = TAPI_HOOK_STATE_ONHOOK;

   for (i = 0; i < TAPI_TONE_MAXRES; ++i)
   {
      /* Start complex tone generation in initialized state */
      pChannel->TapiComplexToneData[i].nToneState = TAPI_CT_IDLE;
   }

   pChannel->bInitialized = IFX_TRUE;

   return IFX_SUCCESS;

error:
   return IFX_ERROR;
}

/**
   Initalizes channel timers)

   \param pTapiCh       Pointer to TAPI_CHANNEL structure.

   \return
   IFX_SUCCESS or IFX_ERROR
*/
static IFX_int32_t ifx_tapi_InitChTimers (TAPI_CHANNEL *pTapiCh)
{
   IFX_uint32_t i;

   /* create a timer for each of the tone generator resources */
   for (i = 0; i < TAPI_TONE_MAXRES; ++i)
   {
      if (pTapiCh->pToneRes[i].Tone_Timer == 0)
      {
         /* initialize (create) voice path teardown timer */
         pTapiCh->pToneRes[i].Tone_Timer =
           TAPI_Create_Timer((TIMER_ENTRY)Tone_OnTimer,
                             (IFX_int32_t)&pTapiCh->pToneRes[i]);
         pTapiCh->pToneRes[i].pTapiCh = pTapiCh;
         pTapiCh->pToneRes[i].nRes = i;
         if (pTapiCh->pToneRes[i].Tone_Timer == 0)
         {
           return IFX_ERROR;
         }
      }
   }

   return IFX_SUCCESS;
}

/**
   Returns the version string.

   \param version_string - pointer to buffer for version string

   \return
   length of version string
*/
IFX_int32_t TAPI_Phone_Get_Version(IFX_char_t *version_string)
{
   IFX_int32_t len = (IFX_int32_t) strlen(TAPI_WHATVERSION) - 4;
   strncpy(version_string, &TAPI_WHATVERSION [4], 80);
   return len;
}

/**
   Returns error if the requested version is not supported

   \param vers     - pointer to version structure

   \return
   IFX_SUCCESS if version is supported or IFX_ERROR

   \remark
   Since an application is always build against one specific TAPI interface
   version it should check if it is supported. If not the application should
   abort. This interface checks if the current TAPI version supports a
   particular version. For example the TAPI versions 2.1 will support TAPI 2.0.
   But version 3.0 might not support 2.0.
*/
IFX_int32_t TAPI_Phone_Check_Version (IFX_TAPI_VERSION_t const *vers)
{
   switch (vers->majorNumber)
   {
      /* we also support version 2.3 */
      case 2:
         switch (vers->minorNumber)
         {
            case 3:
               return IFX_SUCCESS;
               /* break; */
            default:
               break;
         }
         break;
      /* supports this version */
      case  DRV_TAPI_VER_MAJOR:
         switch (vers->minorNumber)
         {
            case 0:
            case 1:
            case DRV_TAPI_VER_MINOR:
               return IFX_SUCCESS;
               /* break; */
            default:
               break;
         }
         break;
      default:
         break;
   }
   return IFX_ERROR;
}

/**
   Check for a valid function pointer and issue a trace with the name of the
   function pointer, in case it doesn't exist.

   \param  ptr          Function pointer to be checked.
   \param  pPtrName     Identifier used to trace in case the ptr is NULL.

   \return
      - IFX_SUCCESS or 
      - IFX_ERROR in case the ptr is NULL
*/
IFX_boolean_t ptr_chk(IFX_void_t *ptr, const IFX_char_t *pPtrName)
{
   if (ptr == IFX_NULL)
   {
      /*
      TRACE (TAPI_DRV, DBG_LEVEL_LOW,
            ("INFO, function pointer not registered for %s\n\r", pPtrName));
      */
      return IFX_FALSE;
   }
   /*
   TRACE (TAPI_DRV, DBG_LEVEL_LOW,
         ("INFO, function pointer called %s\n\r", pPtrName));
   */
   return IFX_TRUE;
}
