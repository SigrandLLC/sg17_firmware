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
   \file drv_tapi_meter.c
   Contains TAPI Metering Services.
*/

/* ============================= */
/* Includes                      */
/* ============================= */

#include "drv_tapi.h"
#include "drv_tapi_ll_interface.h"

/* ============================= */
/* Local function declaration    */
/* ============================= */

static IFX_int32_t TAPI_Phone_Meter_StartTimer(TAPI_CHANNEL *pChannel);

/* ============================= */
/* Global function definition    */
/* ============================= */

/**
   Function called from Timer and switches the meter status.

   \param Timer         TimerID of timer that exipres.
   \param nArg          Argument of timer including the TAPI_CHANNEL structure
                        (cast to an integer)
*/
IFX_void_t TAPI_Phone_Meter_OnTimer(Timer_ID Timer, IFX_int32_t nArg)
{
   TAPI_CHANNEL *pChannel = (TAPI_CHANNEL *) nArg;

   TAPI_METER_DATA_t *pMeter   = &pChannel->TapiMeterData;

   if (pMeter->bMeterBurstMode)
   {
      /* Metering off */
      if (pMeter->MeterConfig.burst_dist > 0)
      {
         /* periodically sending bursts */
         pMeter->bMeterBurstMode = IFX_FALSE;
         TAPI_SetTime_Timer(pMeter->MeterTimerID,
                            pMeter->MeterConfig.burst_dist*1000,
                            IFX_FALSE, IFX_FALSE);
         TAPI_Phone_Set_Linefeed(pChannel, pMeter->nLastLineMode);
      }
      else
      {
         /* special case: if burst_cnt is 1, burst_dist may be zero */
         TAPI_Phone_Meter_Stop (pChannel);
      }
   }
   else
   {
      /* Metering burst */
      if (pMeter->MeterConfig.burst_cnt)
      {
         pMeter->nElapsedCnt --;
         if (pMeter->nElapsedCnt == 0)
         {
            TAPI_Phone_Meter_Stop (pChannel);
         }
         else
         {
            pMeter->bMeterBurstMode = IFX_TRUE;
            TAPI_SetTime_Timer(pMeter->MeterTimerID,
                               pMeter->MeterConfig.burst_len,
                               IFX_FALSE, IFX_FALSE);
            TAPI_Phone_Set_Linefeed(pChannel, IFX_TAPI_LINE_FEED_METER);
         }
      }
      else
      {
         pMeter->bMeterBurstMode = IFX_TRUE;
         TAPI_SetTime_Timer(pMeter->MeterTimerID,
                            pMeter->MeterConfig.burst_len,
                            IFX_FALSE, IFX_FALSE);
         TAPI_Phone_Set_Linefeed(pChannel, IFX_TAPI_LINE_FEED_METER);
      }
   }

   return;
}


/**
   Starts metering and check if all conditions are IFX_TRUE

   \param pChannel      Pointer to TAPI_CHANNEL structure.

   \return
   IFX_SUCCESS or IFX_ERROR
*/
IFX_int32_t TAPI_Phone_Meter_Start(TAPI_CHANNEL *pChannel)
{
   IFX_int32_t ret = IFX_SUCCESS;

   /* check if meter characteristic ist set */
   if ((pChannel->TapiMeterData.MeterConfig.burst_len == 0) ||
      (pChannel->TapiMeterData.MeterConfig.burst_cnt > 1 &&
       pChannel->TapiMeterData.MeterConfig.burst_dist == 0))
   {
      TRACE(TAPI_DRV,DBG_LEVEL_HIGH, ("\n\rDRV_ERROR: no characteristic for metering set\n\r"));
      return IFX_ERROR;
   }

   /* check if telephone isn't ringing */
   if (pChannel->TapiRingData.bRingingMode)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("\n\rDRV_ERROR: channel is just ringing\n\r"));
      return IFX_ERROR;
   }

   /* check if line is in active state */
   if (!(pChannel->TapiOpControlData.nLineMode == IFX_TAPI_LINE_FEED_ACTIVE ||
       pChannel->TapiOpControlData.nLineMode == IFX_TAPI_LINE_FEED_ACTIVE_REV ||
       pChannel->TapiOpControlData.nLineMode == IFX_TAPI_LINE_FEED_NORMAL_AUTO ||
       pChannel->TapiOpControlData.nLineMode == IFX_TAPI_LINE_FEED_REVERSED_AUTO))
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("\n\rDRV_ERROR: line is not in active mode\n\r"));
      return IFX_ERROR;
   }

   /* check if metering is always activated */
   if (pChannel->TapiMeterData.bMeterActive)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("\n\rDRV_ERROR: metering is just active\n\r"));
      return IFX_ERROR;
   }

   /* save line mode */
   pChannel->TapiMeterData.nLastLineMode =
               pChannel->TapiOpControlData.nLineMode;

   /* start the timer for metering */
   ret = TAPI_Phone_Meter_StartTimer(pChannel);

   return ret;
}

/**
   Stops the metering

   \param pChannel      Pointer to TAPI_CHANNEL structure.

   \return
   IFX_SUCCESS or IFX_ERROR
*/
IFX_int32_t TAPI_Phone_Meter_Stop(TAPI_CHANNEL *pChannel)
{
   IFX_int32_t ret = IFX_SUCCESS;

   if (pChannel->TapiMeterData.bMeterActive == IFX_TRUE)
   {
      TAPI_Stop_Timer(pChannel->TapiMeterData.MeterTimerID);
      /* reset metering burst mode */
      pChannel->TapiMeterData.bMeterBurstMode = IFX_FALSE;
      /* recover the last line mode */
      ret = TAPI_Phone_Set_Linefeed(pChannel, pChannel->TapiMeterData.nLastLineMode);
      /* Metering is deactivated */
      pChannel->TapiMeterData.bMeterActive = IFX_FALSE;
   }

   return ret;
}

/**
   Sets the characteristic for the metering service

   \param pChannel      Pointer to TAPI_CHANNEL structure.
   \param pMeterConfig  Contains the metering settings (IFX_TAPI_METER_CFG_t)

   \return
   IFX_SUCCESS or IFX_ERROR
*/
IFX_int32_t TAPI_Phone_Meter_Config(TAPI_CHANNEL *pChannel,
                                    IFX_TAPI_METER_CFG_t const *pMeterConfig)
{
   IFX_TAPI_DRV_CTX_t* pDrvCtx = pChannel->pTapiDevice->pDevDrvCtx;
   IFX_int32_t ret = IFX_SUCCESS;

   pChannel->TapiMeterData.MeterConfig.mode       = pMeterConfig->mode;
   pChannel->TapiMeterData.MeterConfig.freq       = pMeterConfig->freq;
   pChannel->TapiMeterData.MeterConfig.burst_len  = pMeterConfig->burst_len;
   pChannel->TapiMeterData.MeterConfig.burst_dist = pMeterConfig->burst_dist;
   pChannel->TapiMeterData.MeterConfig.burst_cnt  = pMeterConfig->burst_cnt;

   if (ptr_chk(pDrvCtx->ALM.Metering_Cfg, "pDrvCtx->ALM.Metering_Cfg"))
      ret = pDrvCtx->ALM.Metering_Cfg (pChannel->pLLChannel, pMeterConfig->mode,
                                       pMeterConfig->freq);
   return ret;
}


/* ============================= */
/* Local function declaration    */
/* ============================= */


/**
   Starts the timer for metering

   \param pChannel        - handle to TAPI_CHANNEL structure

   \return
   IFX_SUCCESS or IFX_ERROR
*/
static IFX_int32_t TAPI_Phone_Meter_StartTimer(TAPI_CHANNEL *pChannel)
{
   TAPI_METER_DATA_t *pMeter   = &pChannel->TapiMeterData;

   if (pMeter->MeterTimerID == 0)
      return IFX_ERROR;

   if (pMeter->MeterConfig.burst_cnt == 1 &&
       pMeter->MeterConfig.burst_dist == 0)
   {
      pMeter->bMeterBurstMode = IFX_TRUE;
      TAPI_SetTime_Timer(pMeter->MeterTimerID,
                         pMeter->MeterConfig.burst_len,
                         IFX_FALSE, IFX_FALSE);
      TAPI_Phone_Set_Linefeed(pChannel, IFX_TAPI_LINE_FEED_METER);
   }
   else
   {
      /* set counter variable if burst count is set */
      if (pMeter->MeterConfig.burst_cnt)
         pMeter->nElapsedCnt = pMeter->MeterConfig.burst_cnt + 1;
      /* Set and start metering timer */
      TAPI_SetTime_Timer(pMeter->MeterTimerID,
                         pMeter->MeterConfig.burst_dist * 1000,
                         IFX_FALSE, IFX_FALSE);
   }
   /* Metering is active now */
   pMeter->bMeterActive = IFX_TRUE;

   return IFX_SUCCESS;
}
