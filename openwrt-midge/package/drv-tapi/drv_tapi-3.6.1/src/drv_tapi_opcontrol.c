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
   \file drv_tapi_opcontrol.c
   TAPI Operation Control Services.

   \remarks
   All operations done by functions in this module are phone
   related and assumes a phone channel file descriptor.
   Caller of anyone of the functions must make sure that a phone
   channel is used. In case data channel functions are invoked here,
   an instance of the data channel must be passed.
*/

/* ============================= */
/* Includes                      */
/* ============================= */

#include "drv_tapi.h"
#include "drv_tapi_ll_interface.h"
#include "drv_tapi_errno.h"

/* ============================= */
/* Global function definition    */
/* ============================= */

/**

   Reads the hook status from the device

   \param pChannel        - handle to TAPI_CHANNEL structure
   \param pState          - storage variable for hook state

   \return
   IFX_SUCCESS
*/
IFX_int32_t TAPI_Phone_Hookstate(TAPI_CHANNEL *pChannel, IFX_int32_t *pState)
{
   /* clear "Hook change" exception */
   pChannel->TapiMiscData.nException.Bits.hookstate = IFX_FALSE;

   if (pChannel->TapiOpControlData.bHookState)
   {
      *pState = IFX_TRUE;
   }
   else
   {
      *pState = IFX_FALSE;
   }

   return (IFX_SUCCESS);
}

/**
   Sets the linefeeding mode of the device

   \param pChannel      Pointer to TAPI_CHANNEL structure.
   \param nMode         Line mode.

   \return
   \ref IFX_SUCCESS or \ref IFX_ERROR

   \remarks
   Line mode is ALWAYS set, also if it was set before.
*/
IFX_int32_t TAPI_Phone_Set_Linefeed(TAPI_CHANNEL *pChannel,
                                    IFX_int32_t nMode)
{
   IFX_TAPI_DRV_CTX_t* pDrvCtx = pChannel->pTapiDevice->pDevDrvCtx;
   IFX_boolean_t       bBatSw  = IFX_FALSE;
   IFX_boolean_t       bPol    = IFX_FALSE;
   IFX_int32_t         ret     = IFX_SUCCESS;
   IFX_uint8_t         tmp_nLineMode = pChannel->TapiOpControlData.nLineMode;
   IFX_uint8_t         tmp_nBatterySw = pChannel->TapiOpControlData.nBatterySw;
   IFX_uint8_t         tmp_nPolarity = pChannel->TapiOpControlData.nPolarity;


   /* save current linemode in tapi structure */
   pChannel->TapiOpControlData.nLineMode = (IFX_uint8_t) nMode;

   /* check if auto battery switch have to enabled */
   if ((nMode == IFX_TAPI_LINE_FEED_NORMAL_AUTO    ||
        nMode == IFX_TAPI_LINE_FEED_REVERSED_AUTO) &&
       !pChannel->TapiOpControlData.nBatterySw)
   {
      bBatSw = IFX_TRUE;
      pChannel->TapiOpControlData.nBatterySw = 0x01;
   }

   /* check if auto battery switch have to disabled */
   if (!(nMode == IFX_TAPI_LINE_FEED_NORMAL_AUTO    ||
         nMode == IFX_TAPI_LINE_FEED_REVERSED_AUTO) &&
       pChannel->TapiOpControlData.nBatterySw)
   {
      bBatSw = IFX_TRUE;
      pChannel->TapiOpControlData.nBatterySw = 0x00;
   }

   /* check if polarity has to change */
   if ((nMode == IFX_TAPI_LINE_FEED_ACTIVE_REV        ||
        nMode == IFX_TAPI_LINE_FEED_REVERSED_AUTO     ||
        nMode == IFX_TAPI_LINE_FEED_REVERSED_LOW      ||
        nMode == IFX_TAPI_LINE_FEED_ACTIVE_RES_REVERSED) &&
       !pChannel->TapiOpControlData.nPolarity)
   {
      bPol = IFX_TRUE;
      pChannel->TapiOpControlData.nPolarity = 0x01;
   }
   if (!(nMode == IFX_TAPI_LINE_FEED_ACTIVE_REV       ||
         nMode == IFX_TAPI_LINE_FEED_REVERSED_AUTO    ||
         nMode == IFX_TAPI_LINE_FEED_REVERSED_LOW     ||
         nMode == IFX_TAPI_LINE_FEED_ACTIVE_RES_REVERSED) &&
       pChannel->TapiOpControlData.nPolarity)
   {
      bPol = IFX_TRUE;
      pChannel->TapiOpControlData.nPolarity = 0x00;
   }

   /* call low level function to change operation mode */
   if (ptr_chk(pDrvCtx->ALM.Line_Mode_Set, "pDrvCtx->ALM.Line_Mode_Set"))
   {
       ret = pDrvCtx->ALM.Line_Mode_Set(pChannel->pLLChannel,
                              nMode, tmp_nLineMode);
   }

   if (!TAPI_SUCCESS(ret))
   {
      /* restore the old value, because the configuration on the driver failed */
      pChannel->TapiOpControlData.nLineMode = tmp_nLineMode;
      RETURN_STATUS (TAPI_statusLineModeFail, ret);
   }

   /* switch polarity only when it is necessary */
   if (bPol && (ret == IFX_SUCCESS))
   {
      if (ptr_chk(pDrvCtx->ALM.Line_Polarity_Set,
                 "pDrvCtx->ALM.Line_Polarity_Set"))
         ret = pDrvCtx->ALM.Line_Polarity_Set(pChannel->pLLChannel);

      if (!TAPI_SUCCESS(ret))
      {
         /* restore the old value, because the configuration on the driver failed */
         pChannel->TapiOpControlData.nPolarity = tmp_nPolarity;
      }
   }

   /* enable / disable automatic battery switch */
   if (bBatSw && (ret == IFX_SUCCESS))
   {
      if (ptr_chk(pDrvCtx->ALM.AutoBatterySwitch,
                 "pDrvCtx->ALM.AutoBatterySwitch"))
         ret = pDrvCtx->ALM.AutoBatterySwitch(pChannel->pLLChannel);

      if (!TAPI_SUCCESS(ret))
      {
         /* restore the old value, because the configuration on the driver failed */
         pChannel->TapiOpControlData.nBatterySw = tmp_nBatterySw;
      }
   }

   return ret;
}


/**
   Sets the line type mode of the specific analog channel

   \param pDrvCtx       Pointer to low-level device driver context.
   \param pChannel      Pointer to TAPI_CHANNEL structure.
   \param pCfg          Pointer to struct IFX_TAPI_LINE_TYPE_CFG_t.

   \return IFX_SUCCESS or IFX_ERROR

   \remark Line mode is ALWAYS set, also if it was set before. By default
           FXS type is used.
*/
IFX_int32_t TAPI_Phone_Set_LineType(IFX_TAPI_DRV_CTX_t* pDrvCtx,
                                    TAPI_CHANNEL *pChannel,
                                    IFX_TAPI_LINE_TYPE_CFG_t *pCfg)
{
   IFX_int32_t ret = IFX_SUCCESS;

   /* call low level function to change operation mode */
   if (ptr_chk(pDrvCtx->ALM.Line_Type_Set, "pDrvCtx->ALM.Line_Type_Set"))
   {
       ret = pDrvCtx->ALM.Line_Type_Set(pChannel->pLLChannel, pCfg->lineType);
   }

   if (TAPI_SUCCESS(ret))
   {
      /* Save new type and DAA number */
      pChannel->TapiOpControlData.nLineType = pCfg->lineType;
      pChannel->TapiOpControlData.nDAA      = pCfg->nDaaCh;
      if (pCfg->lineType == IFX_TAPI_LINE_TYPE_FXO)
      {
         ret = TAPI_FXO_Register_DAA(pChannel->TapiOpControlData.nDAA, pChannel);
         /* init the DAA channel */
         if ((TAPI_SUCCESS(ret)) &&
             (! pChannel->TapiOpControlData.bDaaInitialized))
         {
            ret = TAPI_FXO_Init_DAA (pChannel);
            if (TAPI_SUCCESS(ret))
            {
               pChannel->TapiOpControlData.bDaaInitialized = IFX_TRUE;
            }
         }
      }
      else
      {
         ret = TAPI_FXO_Register_DAA (pChannel->TapiOpControlData.nDAA, IFX_NULL);
      }
   }
   return ret;
}


/**
   Sets LEC configuration for ALM. (old)

   \param pChannel      Pointer to TAPI_CHANNEL structure.
   \param pLecConf      Pointer to IFX_TAPI_LEC_CFG_t structure.

   \return
   \ref IFX_SUCCESS or \ref IFX_ERROR
*/
IFX_int32_t TAPI_Phone_LecConf_Alm (TAPI_CHANNEL *pChannel,
                                    IFX_TAPI_LEC_CFG_t *pLecConf)
{
   IFX_TAPI_DRV_CTX_t *pDrvCtx = pChannel->pTapiDevice->pDevDrvCtx;
   TAPI_LEC_DATA_t  LecData;
   IFX_int32_t     ret;

   /* do not touch anything if LL does not support LEC */
   if (!ptr_chk(pDrvCtx->ALM.Lec_Cfg, "pDrvCtx->ALM.Lec_Cfg"))
   {
      TRACE (TAPI_DRV, DBG_LEVEL_HIGH,
             ("DRV_ERROR: LEC not supported by LL driver\n\r"));
      return IFX_ERROR;
   }

   /* do parameter checks */
   /* nLen is never evaluated so we no longer check it */
   if ((pLecConf->nGainIn  >  IFX_TAPI_LEC_GAIN_HIGH) ||
       (pLecConf->nGainOut >  IFX_TAPI_LEC_GAIN_HIGH) )
   {
      TRACE (TAPI_DRV, DBG_LEVEL_HIGH,
         ("DRV_ERROR: Gain In (%d) or Gain out (%d) or length out of range.\n\r",
          pLecConf->nGainIn, pLecConf->nGainOut));
      return IFX_ERROR;
   }

   /* make a copy of the current configuration */
   memcpy(&LecData, &pChannel->TapiLecAlmData, sizeof(TAPI_LEC_DATA_t));

   /* operating mode is off when one of the gains is set to off
      in case one of the gains is set off, set the other also off */
   if ((pLecConf->nGainIn  == IFX_TAPI_LEC_GAIN_OFF) ||
       (pLecConf->nGainOut == IFX_TAPI_LEC_GAIN_OFF))
   {
      pLecConf->nGainIn  = IFX_TAPI_LEC_GAIN_OFF;
      pLecConf->nGainOut = IFX_TAPI_LEC_GAIN_OFF;
      LecData.nOpMode    = IFX_TAPI_WLEC_TYPE_OFF;
   }
   else
   {
      LecData.nOpMode    = IFX_TAPI_WLEC_TYPE_NE;
   }

   /* assemble configuration data structure */
   LecData.bNlp     = pLecConf->bNlp;
   LecData.nGainIn  = pLecConf->nGainIn;
   LecData.nGainOut = pLecConf->nGainOut;
   LecData.nLen     = pLecConf->nLen;
   LecData.nNBNEwindow = 16;
   LecData.nNBFEwindow = 0;
   LecData.nWBNEwindow = 8;

   /* call low level for settings */
   ret = pDrvCtx->ALM.Lec_Cfg(pChannel->pLLChannel, &LecData);

   if (ret == IFX_SUCCESS)
   {
      /* begin of protected area */
      IFXOS_MutexLock(pChannel->semTapiChDataLock);
      /* save configuration */
      memcpy(&pChannel->TapiLecAlmData, &LecData, sizeof(TAPI_LEC_DATA_t));
      /* end of protected area */
      IFXOS_MutexUnlock(pChannel->semTapiChDataLock);
   }

   return ret;
}


/**
   Read current LEC configuration for ALM. (old)

   \param pChannel      Pointer to TAPI_CHANNEL structure.
   \param pLecConf      Pointer to IFX_TAPI_LEC_CFG_t structure.

   \return
   IFX_SUCCESS or IFX_ERROR
*/
IFX_int32_t TAPI_Phone_GetLecConf_Alm (TAPI_CHANNEL *pChannel,
                                       IFX_TAPI_LEC_CFG_t *pLecConf)
{
   TAPI_LEC_DATA_t *pLecData = &pChannel->TapiLecAlmData;

   /* begin of protected area */
   IFXOS_MutexLock(pChannel->semTapiChDataLock);

   /* read config from channel structure */
   pLecConf->bNlp     = pLecData->bNlp;
   pLecConf->nGainIn  = pLecData->nGainIn;
   pLecConf->nGainOut = pLecData->nGainOut;
   pLecConf->nLen     = pLecData->nLen;

   /* end of protected area */
   IFXOS_MutexUnlock(pChannel->semTapiChDataLock);

   return IFX_SUCCESS;
}


/**
   Sets LEC configuration for PCM. (old)

   \param pChannel      Pointer to TAPI_CHANNEL structure.
   \param pLecConf      Pointer to IFX_TAPI_LEC_CFG_t structure.

   \return
   \ref IFX_SUCCESS or \ref IFX_ERROR
*/
IFX_int32_t TAPI_Phone_LecConf_Pcm (TAPI_CHANNEL *pChannel,
                                    IFX_TAPI_LEC_CFG_t *pLecConf)
{
   IFX_TAPI_DRV_CTX_t *pDrvCtx = pChannel->pTapiDevice->pDevDrvCtx;
   TAPI_LEC_DATA_t LecData;
   IFX_int32_t     ret;

   /* do not touch anything if LL does not support LEC */
   if (!ptr_chk(pDrvCtx->PCM.Lec_Cfg, "pDrvCtx->PCM.Lec_Cfg"))
   {
      TRACE (TAPI_DRV, DBG_LEVEL_HIGH,
             ("DRV_ERROR: LEC not supported by LL driver\n\r"));
      return IFX_ERROR;
   }

   /* do parameter checks */
   /* nLen is never evaluated so we no longer check it */
   if ((pLecConf->nGainIn  >  IFX_TAPI_LEC_GAIN_HIGH) ||
       (pLecConf->nGainOut >  IFX_TAPI_LEC_GAIN_HIGH) )
   {
      TRACE (TAPI_DRV, DBG_LEVEL_HIGH,
         ("DRV_ERROR: Gain In (%d) or Gain out (%d) or length out of range.\n\r",
          pLecConf->nGainIn, pLecConf->nGainOut));
      return IFX_ERROR;
   }

   /* make a copy of the current configuration */
   memcpy(&LecData, &pChannel->TapiLecPcmData, sizeof(TAPI_LEC_DATA_t));

   /* operating mode is off when one of the gains is set to off
      in case one of the gains is set off, set the other also off */
   if ((pLecConf->nGainIn  == IFX_TAPI_LEC_GAIN_OFF) ||
       (pLecConf->nGainOut == IFX_TAPI_LEC_GAIN_OFF))
   {
      pLecConf->nGainIn  = IFX_TAPI_LEC_GAIN_OFF;
      pLecConf->nGainOut = IFX_TAPI_LEC_GAIN_OFF;
      LecData.nOpMode    = IFX_TAPI_WLEC_TYPE_OFF;
   }
   else
   {
      LecData.nOpMode    = IFX_TAPI_WLEC_TYPE_NE;
   }

   /* assemble configuration data structure */
   LecData.bNlp     = pLecConf->bNlp;
   LecData.nGainIn  = pLecConf->nGainIn;
   LecData.nGainOut = pLecConf->nGainOut;
   LecData.nLen     = pLecConf->nLen;
   LecData.nNBNEwindow = 16;
   LecData.nNBFEwindow = 0;
   LecData.nWBNEwindow = 8;

   /* call low level for settings */
   ret = pDrvCtx->PCM.Lec_Cfg (pChannel->pLLChannel, &LecData);
   if (ret == IFX_SUCCESS)
   {
      /* begin of protected area */
      IFXOS_MutexLock(pChannel->semTapiChDataLock);
      /* save configuration */
      memcpy(&pChannel->TapiLecPcmData, &LecData, sizeof(TAPI_LEC_DATA_t));
      /* end of protected area */
      IFXOS_MutexUnlock(pChannel->semTapiChDataLock);
   }

   return ret;
}


/**
   Read current LEC configuration for PCM. (old)

   \param pChannel      Pointer to TAPI_CHANNEL structure.
   \param pLecConf      Poiner to IFX_TAPI_LEC_CFG_t structure.

   \return
   IFX_SUCCESS or IFX_ERROR
*/
IFX_int32_t TAPI_Phone_GetLecConf_Pcm (TAPI_CHANNEL *pChannel,
                                       IFX_TAPI_LEC_CFG_t *pLecConf)
{
   TAPI_LEC_DATA_t *pLecData = &pChannel->TapiLecPcmData;

   /* begin of protected area */
   IFXOS_MutexLock(pChannel->semTapiChDataLock);

   /* read config from channel structure */
   pLecConf->bNlp     = pLecData->bNlp;
   pLecConf->nGainIn  = pLecData->nGainIn;
   pLecConf->nGainOut = pLecData->nGainOut;
   pLecConf->nLen     = pLecData->nLen;

   /* end of protected area */
   IFXOS_MutexUnlock(pChannel->semTapiChDataLock);

   return IFX_SUCCESS;
}


/**
   Sets the LEC operating mode

   \param pChannel      Pointer to TAPI_CHANNEL structure.
   \param pLecConf      Pointer to IFX_TAPI_WLEC_CFG_t structure.

   \return
   IFX_SUCCESS or IFX_ERROR
*/
IFX_int32_t TAPI_Phone_LecMode_Alm_Set (TAPI_CHANNEL *pChannel,
                                        IFX_TAPI_WLEC_CFG_t *pLecConf)
{
   IFX_TAPI_DRV_CTX_t *pDrvCtx = pChannel->pTapiDevice->pDevDrvCtx;
   TAPI_LEC_DATA_t oLecData;
   IFX_int32_t     ret;

   /* do not touch anything if LL does not support LEC */
   if (!ptr_chk(pDrvCtx->ALM.Lec_Cfg, "pDrvCtx->ALM.Lec_Cfg"))
   {
      TRACE (TAPI_DRV, DBG_LEVEL_HIGH,
             ("DRV_ERROR: LEC not supported by LL driver\n\r"));
      return IFX_ERROR;
   }

   /* do parameter checks */
   if (pLecConf->nType == IFX_TAPI_WLEC_TYPE_NE)
   {
      /* default (0) is substituted with 16 ms */
      if (pLecConf->nNBNEwindow == 0)   pLecConf->nNBNEwindow = 16;
   }
   if (pLecConf->nType == IFX_TAPI_WLEC_TYPE_NFE)
   {
      /* default (0) is substituted with 8 ms */
      if (pLecConf->nNBNEwindow == 0)   pLecConf->nNBNEwindow = 8;
      if (pLecConf->nNBFEwindow == 0)   pLecConf->nNBFEwindow = 8;
   }
   if (pLecConf->nWBNEwindow == 0)  pLecConf->nWBNEwindow = 8;
   /* set values below the lower limit to the lower limit of 4 ms */
   if (pLecConf->nNBNEwindow < 4)    pLecConf->nNBNEwindow = 4;
   if (pLecConf->nNBFEwindow < 4)    pLecConf->nNBFEwindow = 4;
   if (pLecConf->nWBNEwindow < 4)    pLecConf->nWBNEwindow = 4;
   /* set values above the upper limit to the upper limit of 16 ms */
   if (pLecConf->nNBNEwindow > 16)   pLecConf->nNBNEwindow = 16;
   if (pLecConf->nNBFEwindow > 16)   pLecConf->nNBFEwindow = 16;
   if (pLecConf->nWBNEwindow > 16)   pLecConf->nWBNEwindow = 16;
   /* if the combined window size exceeds the limit of 16 ms abort */
   if ((pLecConf->nType == IFX_TAPI_WLEC_TYPE_NFE) &&
       (pLecConf->nNBNEwindow + pLecConf->nNBFEwindow > 16))
   {
      TRACE (TAPI_DRV, DBG_LEVEL_HIGH, ("DRV_ERROR: combined window sizes "
             "for narrwoband mode exceed the 16 ms limit.\n\r"));
      return IFX_ERROR;
   }

   /* make a copy of the current configuration */
   memcpy(&oLecData, &pChannel->TapiLecAlmData, sizeof(TAPI_LEC_DATA_t));

   /* assemble configuration data structure */
   oLecData.nOpMode     = pLecConf->nType;
   oLecData.bNlp        = pLecConf->bNlp;
   oLecData.nNBNEwindow = pLecConf->nNBNEwindow;
   oLecData.nNBFEwindow = pLecConf->nNBFEwindow;
   oLecData.nWBNEwindow = pLecConf->nWBNEwindow;

   /* call low level for settings */
   ret = pDrvCtx->ALM.Lec_Cfg (pChannel->pLLChannel, &oLecData);
   if (ret == IFX_SUCCESS)
   {
      /* begin of protected area */
      IFXOS_MutexLock(pChannel->semTapiChDataLock);
      /* save configuration */
      memcpy(&pChannel->TapiLecAlmData, &oLecData, sizeof(TAPI_LEC_DATA_t));
      /* end of protected area */
      IFXOS_MutexUnlock(pChannel->semTapiChDataLock);
   }

   return ret;
}


/**
   Get the current LEC operating mode

   \param pChannel      Pointer to TAPI_CHANNEL structure.
   \param pWLecConf     Pointer to IFX_TAPI_WLEC_CFG_t structure.

   \return
   IFX_SUCCESS or IFX_ERROR
*/
IFX_int32_t TAPI_Phone_LecMode_Alm_Get (TAPI_CHANNEL *pChannel,
                                        IFX_TAPI_WLEC_CFG_t *pWLecConf)
{
   TAPI_LEC_DATA_t *pLecData = &pChannel->TapiLecAlmData;

   /* begin of protected area */
   IFXOS_MutexLock(pChannel->semTapiChDataLock);

   /* read config from channel structure */
   pWLecConf->nType = pLecData->nOpMode;
   pWLecConf->bNlp  = (IFX_TAPI_WLEC_NLP_t)pLecData->bNlp;
   pWLecConf->nNBNEwindow = pLecData->nNBNEwindow;
   pWLecConf->nNBFEwindow = pLecData->nNBFEwindow;
   pWLecConf->nWBNEwindow = pLecData->nWBNEwindow;

   /* end of protected area */
   IFXOS_MutexUnlock(pChannel->semTapiChDataLock);

   return IFX_SUCCESS;
}


/**
   Sets the LEC operating mode

   \param pChannel      Pointer to TAPI_CHANNEL structure.
   \param pLecConf      Pointer to IFX_TAPI_WLEC_CFG_t structure.

   \return
   IFX_SUCCESS or IFX_ERROR
*/
IFX_int32_t TAPI_Phone_LecMode_Pcm_Set (TAPI_CHANNEL *pChannel,
                                        IFX_TAPI_WLEC_CFG_t *pLecConf)
{
   IFX_TAPI_DRV_CTX_t *pDrvCtx = pChannel->pTapiDevice->pDevDrvCtx;
   TAPI_LEC_DATA_t oLecData;
   IFX_int32_t     ret;

   /* do not touch anything if LL does not support LEC */
   if (!ptr_chk(pDrvCtx->PCM.Lec_Cfg, "pDrvCtx->PCM.Lec_Cfg"))
   {
      TRACE (TAPI_DRV, DBG_LEVEL_HIGH,
             ("DRV_ERROR: LEC not supported by LL driver\n\r"));
      return IFX_ERROR;
   }

   /* do parameter checks */
   if (pLecConf->nType == IFX_TAPI_WLEC_TYPE_NE)
   {
      /* default (0) is substituted with 16 ms */
      if (pLecConf->nNBNEwindow == 0)   pLecConf->nNBNEwindow = 16;
   }
   if (pLecConf->nType == IFX_TAPI_WLEC_TYPE_NFE)
   {
      /* default (0) is substituted with 8 ms */
      if (pLecConf->nNBNEwindow == 0)   pLecConf->nNBNEwindow = 8;
      if (pLecConf->nNBFEwindow == 0)   pLecConf->nNBFEwindow = 8;
   }
   if (pLecConf->nWBNEwindow == 0)  pLecConf->nWBNEwindow = 8;
   /* set values below the lower limit to the lower limit of 4 ms */
   if (pLecConf->nNBNEwindow < 4)    pLecConf->nNBNEwindow = 4;
   if (pLecConf->nNBFEwindow < 4)    pLecConf->nNBFEwindow = 4;
   if (pLecConf->nWBNEwindow < 4)    pLecConf->nWBNEwindow = 4;
   /* set values above the upper limit to the upper limit of 16 ms */
   if (pLecConf->nNBNEwindow > 16)   pLecConf->nNBNEwindow = 16;
   if (pLecConf->nNBFEwindow > 16)   pLecConf->nNBFEwindow = 16;
   if (pLecConf->nWBNEwindow > 16)   pLecConf->nWBNEwindow = 16;
   /* if the combined window size exceeds the limit of 16 ms abort */
   if ((pLecConf->nType == IFX_TAPI_WLEC_TYPE_NFE) &&
       (pLecConf->nNBNEwindow + pLecConf->nNBFEwindow > 16))
   {
      TRACE (TAPI_DRV, DBG_LEVEL_HIGH, ("DRV_ERROR: combined window sizes "
             "for narrwoband mode exceed the 16 ms limit.\n\r"));
      return IFX_ERROR;
   }

   /* make a copy of the current configuration */
   memcpy(&oLecData, &pChannel->TapiLecPcmData, sizeof(TAPI_LEC_DATA_t));

   /* assemble configuration data structure */
   oLecData.nOpMode     = pLecConf->nType;
   oLecData.bNlp        = pLecConf->bNlp;
   oLecData.nNBNEwindow = pLecConf->nNBNEwindow;
   oLecData.nNBFEwindow = pLecConf->nNBFEwindow;
   oLecData.nWBNEwindow = pLecConf->nWBNEwindow;

   /* call low level for settings */
   ret = pDrvCtx->PCM.Lec_Cfg (pChannel->pLLChannel, &oLecData);
   if (ret == IFX_SUCCESS)
   {
      /* begin of protected area */
      IFXOS_MutexLock(pChannel->semTapiChDataLock);
      /* save configuration */
      memcpy(&pChannel->TapiLecPcmData, &oLecData, sizeof(TAPI_LEC_DATA_t));
      /* end of protected area */
      IFXOS_MutexUnlock(pChannel->semTapiChDataLock);
   }

   return ret;
}


/**
   Get the current LEC operating mode

   \param pChannel      Pointer to TAPI_CHANNEL structure.
   \param pLecConf      Pointer to IFX_TAPI_WLEC_CFG_t structure.

   \return
   IFX_SUCCESS or IFX_ERROR
*/
IFX_int32_t TAPI_Phone_LecMode_Pcm_Get (TAPI_CHANNEL *pChannel,
                                        IFX_TAPI_WLEC_CFG_t *pLecConf)
{
   TAPI_LEC_DATA_t *pLecData = &pChannel->TapiLecPcmData;

   /* begin of protected area */
   IFXOS_MutexLock(pChannel->semTapiChDataLock);

   /* read config from channel structure */
   pLecConf->bNlp  = (IFX_TAPI_WLEC_NLP_t)pLecData->bNlp;
   pLecConf->nType = pLecData->nOpMode;
   pLecConf->nNBNEwindow = pLecData->nNBNEwindow;
   pLecConf->nNBFEwindow = pLecData->nNBFEwindow;
   pLecConf->nWBNEwindow = pLecData->nWBNEwindow;

   /* end of protected area */
   IFXOS_MutexUnlock(pChannel->semTapiChDataLock);

   return IFX_SUCCESS;
}

/**
   Sets/Gets the DTMF Receiver Coefficients

   \param pDrvCtx         - pointer to low-level device driver context
   \param pChannel        - handle to TAPI_CHANNEL structure
   \param bRW
   \param pCoeff          - handle to IFX_TAPI_DTMF_RX_CFG_t structure

   \return
   IFX_SUCCESS or IFX_ERROR
*/
IFX_int32_t TAPI_Phone_Dtmf_RxCoeff_Cfg (IFX_TAPI_DRV_CTX_t *pDrvCtx,
                                         TAPI_CHANNEL *pChannel,
                                         IFX_boolean_t bRW,
                                         IFX_TAPI_DTMF_RX_CFG_t *pCoeff)
{
   IFX_int32_t ret;

   /* do not touch anything if LL device DTMF Rx coefficients
      not configurable */
   if (!ptr_chk(pDrvCtx->SIG.DTMF_RxCoeff, "pDrvCtx->SIG.DTMF_RxCoeff"))
   {
      TRACE (TAPI_DRV, DBG_LEVEL_HIGH,
            ("DRV_ERROR: DTMF Rx coefficients not configurable on LL device\n\r"));
      return IFX_ERROR;
   }

   /* call low level routine to read/write the settings */
   ret = pDrvCtx->SIG.DTMF_RxCoeff (pChannel->pLLChannel, bRW, pCoeff);

   TRACE (TAPI_DRV, DBG_LEVEL_LOW,
         ("Ch%d: DTMF Rx coefficients %s: "
          "Level=%d [dB], Twist=%d [dB], Gain=%d [dB]\n\r",
          pChannel->nChannel, bRW == IFX_FALSE? "setting" : "getting",
          pCoeff->nLevel, pCoeff->nTwist, pCoeff->nGain));

   return ret;
}

/**
   Power save recheck timer entry.

   \param Timer   - timer ID

   \param nArg    - handle to IFX_TAPI_DRV_CTX_t structure

   \return
   None.

   \remark
   Every two seconds the low level device is polled to activate a power
   save mode if this mode is available for the device.
*/
IFX_void_t  TAPI_Power_Save_OnTimer (Timer_ID Timer, IFX_int32_t nArg)
{
   IFX_int32_t          err = IFX_SUCCESS;
   IFX_TAPI_DRV_CTX_t   *pDrvCtx = (IFX_TAPI_DRV_CTX_t *)nArg;
   IFX_TAPI_LL_DEV_t    *pLLDev  = (IFX_TAPI_LL_DEV_t *)pDrvCtx->pTapiDev->pLLDev;

   /* check if ll device supports a power save feature */
   if (ptr_chk(pDrvCtx->Pwr_Save_Dev, "pDrvCtx->Pwr_Save_Dev"))
      err = pDrvCtx->Pwr_Save_Dev (pLLDev);

   if (err != IFX_SUCCESS)
      TRACE (TAPI_DRV, DBG_LEVEL_LOW,
            ("DRV_ERROR: Low level device failed to set power save mode.\n\r"));
}
