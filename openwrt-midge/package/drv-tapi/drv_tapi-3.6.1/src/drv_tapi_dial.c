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
   \file drv_tapi_dial.c
   Contains TAPI Dial Services.
*/

/* ============================= */
/* Includes                      */
/* ============================= */

#include "drv_tapi.h"

/* ============================= */
/* Global function definition    */
/* ============================= */

/**
   Event handling function. Entry to the hook-state-machine.

   \param pChannel    - handle to TAPI_CHANNEL structure
   \param bHookState  - IFX_TRUE : off hook
                        IFX_FALSE: on hook
   \return
     None
*/
IFX_void_t TAPI_Phone_Event_HookState (TAPI_CHANNEL * pChannel,
                                       IFX_uint8_t bHookState)
{
   if (bHookState == IFX_TRUE)
   {
      /* phone has gone offhook */
      switch (pChannel->TapiDialData.nHookState)
      {
      case TAPI_HOOK_STATE_ONHOOK:
         /* Set timer to verify this offhook isn't just some line noise */
         TAPI_SetTime_Timer (pChannel->TapiDialData.DialTimerID,
                             pChannel->TapiHookOffTime.nMinTime, IFX_FALSE,
                             IFX_FALSE);
         pChannel->TapiDialData.nHookState = TAPI_HOOK_STATE_OFFHOOK_VAL;
         break;

      case TAPI_HOOK_STATE_PULSE_L_CONFIRM:
         /* Offhook may indicate completion of low pulse: set timer to validate
          */
         TAPI_SetTime_Timer (pChannel->TapiDialData.DialTimerID,
                             pChannel->TapiDigitHighTime.nMinTime, IFX_FALSE,
                             IFX_TRUE);
         pChannel->TapiDialData.nHookState = TAPI_HOOK_STATE_PULSE_H_VAL;
         break;
      case TAPI_HOOK_STATE_PULSE_L_FLASH_CONFIRM:
         /* overlap of hook flash min and pulse max time. Confirmation needed:
            - interdigit time and hook flash min time ended: report digit 1 and
            flash hook - next pulse occurs: report only digit */
         TAPI_SetTime_Timer (pChannel->TapiDialData.DialTimerID,
                             pChannel->TapiDigitHighTime.nMinTime, IFX_FALSE,
                             IFX_TRUE);
         pChannel->TapiDialData.nHookState = TAPI_HOOK_STATE_PULSE_H_VAL;
         pChannel->TapiDialData.state.bProbablyFlash = IFX_TRUE;
         break;
      case TAPI_HOOK_STATE_FLASH_WAIT:
         /* Offhook arrives too soon for flash: go to offhook (no event) */
         TAPI_Stop_Timer (pChannel->TapiDialData.DialTimerID);
         pChannel->TapiDialData.nHookState = TAPI_HOOK_STATE_OFFHOOK;
         break;

      case TAPI_HOOK_STATE_FLASH_CONFIRM:
         if (pChannel->TapiHookFlashMakeTime.nMinTime == 0)
         {
            IFX_TAPI_EVENT_t tapiEvent;
            IFX_return_t ret;
            /* Offhook indicates completion of flash */
            memset (&tapiEvent, 0, sizeof (IFX_TAPI_EVENT_t));
            tapiEvent.id = IFX_TAPI_EVENT_FXS_FLASH;
            ret = IFX_TAPI_Event_Dispatch (pChannel, &tapiEvent);
            if (ret != IFX_SUCCESS)
            {
               /* \todo if dispatcher error?? */
            }
         }
         else
         {
            /* validation time specified so validate */
            TAPI_SetTime_Timer (pChannel->TapiDialData.DialTimerID,
                                pChannel->TapiHookFlashMakeTime.nMinTime,
                                IFX_FALSE, IFX_TRUE);
            pChannel->TapiDialData.nHookState = TAPI_HOOK_STATE_FLASH_VAL;
         }
         break;

      case TAPI_HOOK_STATE_ONHOOK_CONFIRM:
         /* Offhook arrives too soon for final onhook: go to offhook (no event)
          */
         TAPI_Stop_Timer (pChannel->TapiDialData.DialTimerID);
         pChannel->TapiDialData.nHookState = TAPI_HOOK_STATE_OFFHOOK;
         break;

      case TAPI_HOOK_STATE_ONHOOK_VAL:
         /* Offhook while validating onhook: stop validation timer and return
            to OFFHOOK */
         /* If collecting digits, abort */
         pChannel->TapiDialData.nHookChanges = 0;
         TAPI_Stop_Timer (pChannel->TapiDialData.DialTimerID);
         pChannel->TapiDialData.nHookState = TAPI_HOOK_STATE_OFFHOOK;
         break;

      case TAPI_HOOK_STATE_DIAL_L_VAL:
         TAPI_SetTime_Timer (pChannel->TapiDialData.DialTimerID,
                             pChannel->TapiDigitHighTime.nMinTime, IFX_FALSE,
                             IFX_TRUE);
         pChannel->TapiDialData.nHookState = TAPI_HOOK_STATE_PULSE_H_VAL;
         break;

      default:
         /* all other states represent an error condition */
         break;
      }
   }
   else
   {
      /* phone has gone onhook */
      switch (pChannel->TapiDialData.nHookState)
      {
      case TAPI_HOOK_STATE_FLASH_VAL:
      case TAPI_HOOK_STATE_OFFHOOK:

         /* Set timer to confirm whether this onhook is not just result of line
            noise */
         /* Restart=IFX_TRUE because interdigit timer could be running */
         TAPI_SetTime_Timer (pChannel->TapiDialData.DialTimerID,
                             pChannel->TapiDigitLowTime.nMinTime, IFX_FALSE,
                             IFX_TRUE);
         pChannel->TapiDialData.nHookState = TAPI_HOOK_STATE_ONHOOK_VAL;
         break;

      case TAPI_HOOK_STATE_PULSE_H_CONFIRM:
         /* Set timer to confirm whether this onhook is not just result of line
            noise */
         /* Restart=IFX_TRUE because DigitHigh.nMaxTime timer is running */
         TAPI_SetTime_Timer (pChannel->TapiDialData.DialTimerID,
                             pChannel->TapiDigitLowTime.nMinTime, IFX_FALSE,
                             IFX_TRUE);
         pChannel->TapiDialData.nHookState = TAPI_HOOK_STATE_DIAL_L_VAL;
         break;

      case TAPI_HOOK_STATE_PULSE_H_VAL:
         /* Pulse duration too short */
         TAPI_SetTime_Timer (pChannel->TapiDialData.DialTimerID,
                             pChannel->TapiDigitLowTime.nMinTime, IFX_FALSE,
                             IFX_TRUE);
         pChannel->TapiDialData.nHookState = TAPI_HOOK_STATE_DIAL_L_VAL;
         break;

      case TAPI_HOOK_STATE_OFFHOOK_VAL:
         TAPI_Stop_Timer (pChannel->TapiDialData.DialTimerID);
         pChannel->TapiDialData.nHookState = TAPI_HOOK_STATE_ONHOOK;
         break;

      default:
         /* all other states represent an error condition */
         break;
      }
   }

   return;
}


/**
   Function called from the dial timer.
   The timer is used to verify the duration of various on- and offhook events:
     - Offhook long enough to be confirmed (not just line noise)
     - Offhook too long to be high pulse
     - Offhook long enough to indicate pulse digit completion (interdigit timer)
     - Onhook long enough to be confirmed (not just line noise)
     - Onhook too long to be a digit pulse (it may be flash or final onhook)
     - Onhook long enough to be a flash
     - Onhook too long to be a flash (it may be final onhook)
     - Onhook long enough to be a final onhook

   \param Timer - TimerID of timer that exipres
   \param nArg  - Argument of timer including the TAPI_CHANNEL structure
           (as integer pointer)
   \return:
*/
IFX_void_t TAPI_Phone_Dial_OnTimer(Timer_ID Timer, IFX_int32_t nArg)
{
   TAPI_CHANNEL *pChannel = (TAPI_CHANNEL *) nArg;
   IFX_uint8_t     nPulseDigit;
  /* Event information. */
   IFX_TAPI_EVENT_t tapiEvent;
   IFX_return_t ret;

   switch (pChannel->TapiDialData.nHookState)
   {
      case TAPI_HOOK_STATE_OFFHOOK:
         /* Interdigit timer expiry indicates a complete pulse digit has
            been collected */
         nPulseDigit = pChannel->TapiDialData.nHookChanges;
         if (nPulseDigit == 10)
            nPulseDigit = 0xB;  /* digit 0 is represented by 0xB */
         if ((nPulseDigit == 1)&&
             (pChannel->TapiDialData.state.bProbablyFlash == IFX_TRUE))
         {
            /* FLASH event. Put pulse event into even fifo.*/
            memset(&tapiEvent, 0, sizeof(IFX_TAPI_EVENT_t));
            tapiEvent.id = IFX_TAPI_EVENT_FXS_FLASH;
            IFX_TAPI_Event_Dispatch(pChannel,&tapiEvent);
         }

         /* PULSE event. Put pulse event into even fifo. */
         memset(&tapiEvent, 0, sizeof(IFX_TAPI_EVENT_t));
         tapiEvent.id = IFX_TAPI_EVENT_PULSE_DIGIT;
         tapiEvent.data.pulse.digit = (nPulseDigit & 0xff);
         ret = IFX_TAPI_Event_Dispatch(pChannel,&tapiEvent);
         if(ret != IFX_SUCCESS)
         {
            /* \todo if dispatcher error?? */
         }
         /* NB: the FSM remains in the same state, OFFHOOK */
                  /* NB: the FSM remains in the same state, OFFHOOK */
         break;

      case TAPI_HOOK_STATE_PULSE_H_CONFIRM:
         /* digit_h_max expiry indicates offhook has lasted too long to be
            high pulse */
         TAPI_SetTime_Timer(pChannel->TapiDialData.DialTimerID,
                            pChannel->TapiInterDigitTime.nMinTime -
                            pChannel->TapiDigitHighTime.nMaxTime,
                            IFX_FALSE,
                            IFX_FALSE);
         pChannel->TapiDialData.nHookState = TAPI_HOOK_STATE_OFFHOOK;
         break;

      case TAPI_HOOK_STATE_OFFHOOK_VAL:
         /* Timer indicates offhook has lasted long enough to be validated */
         /* IFX_TRUE => hook event, put into fifo */
         memset(&tapiEvent, 0, sizeof(IFX_TAPI_EVENT_t));
         tapiEvent.id = IFX_TAPI_EVENT_FXS_OFFHOOK;
         ret = IFX_TAPI_Event_Dispatch(pChannel,&tapiEvent);
         if(ret != IFX_SUCCESS)
         {
            /* \todo if dispatcher error?? */
         }
         break;

      case TAPI_HOOK_STATE_PULSE_H_VAL:
         /* digit_h_min timer indicates offhook has lasted long enough to
            be validated */
         TAPI_SetTime_Timer(pChannel->TapiDialData.DialTimerID,
                            pChannel->TapiDigitHighTime.nMaxTime -
                            pChannel->TapiDigitHighTime.nMinTime,
                            IFX_FALSE,
                            IFX_FALSE);
         /* A pulse has been detected (not yet a complete pulse digit) */
         pChannel->TapiDialData.nHookChanges++;
         pChannel->TapiDialData.nHookState = TAPI_HOOK_STATE_PULSE_H_CONFIRM;
         break;

      case TAPI_HOOK_STATE_PULSE_L_CONFIRM:
         /* Timer digit_l_max expires: onhook has lasted too long to be
            a pulse reset the pulse counter: pulse digit collection, if in
            progress, has been aborted */
         if (pChannel->TapiHookFlashTime.nMinTime >
             pChannel->TapiDigitLowTime.nMaxTime)
         {
            TAPI_SetTime_Timer(pChannel->TapiDialData.DialTimerID,
                               pChannel->TapiHookFlashTime.nMinTime -
                               pChannel->TapiDigitLowTime.nMaxTime,
                               IFX_FALSE, IFX_FALSE);
            pChannel->TapiDialData.nHookState = TAPI_HOOK_STATE_FLASH_WAIT;
         }
         else
         {
            if (pChannel->TapiHookFlashTime.nMinTime ==
                pChannel->TapiDigitLowTime.nMaxTime)
            {
               /* special case no time to wait in state L_FLASH_CONFIRM */
               TAPI_SetTime_Timer(pChannel->TapiDialData.DialTimerID,
                                     pChannel->TapiHookFlashTime.nMaxTime -
                                     pChannel->TapiDigitLowTime.nMaxTime,
                                     IFX_FALSE, IFX_FALSE);
               pChannel->TapiDialData.nHookState = TAPI_HOOK_STATE_FLASH_CONFIRM;
            }
            else
            {
               /* If TapiHookFlashTime.nMinTime <= TapiDigitLowTime.nMaxTime,
                * skip over state TAPI_HOOK_STATE_FLASH_WAIT.           */
               TAPI_SetTime_Timer(pChannel->TapiDialData.DialTimerID,
                                  pChannel->TapiDigitLowTime.nMaxTime -
                                  pChannel->TapiHookFlashTime.nMinTime,
                                  IFX_FALSE, IFX_FALSE);
               pChannel->TapiDialData.nHookState = TAPI_HOOK_STATE_PULSE_L_FLASH_CONFIRM;

            }
         }
         break;

      case TAPI_HOOK_STATE_FLASH_WAIT:
         /* Timer flash_min expires: the onhook has lasted long enough
            to be a flash */
         TAPI_SetTime_Timer(pChannel->TapiDialData.DialTimerID,
                            pChannel->TapiHookFlashTime.nMaxTime -
                            pChannel->TapiHookFlashTime.nMinTime,
                            IFX_FALSE, IFX_FALSE);
         pChannel->TapiDialData.nHookState = TAPI_HOOK_STATE_FLASH_CONFIRM;
         break;

      case TAPI_HOOK_STATE_FLASH_CONFIRM:
         /* Timer flash_max expires: onhook has lasted too long to be a flash */
         if (pChannel->TapiHookOnTime.nMinTime > pChannel->TapiHookFlashTime.nMaxTime)
         {
            TAPI_SetTime_Timer(pChannel->TapiDialData.DialTimerID,
                               pChannel->TapiHookOnTime.nMinTime -
                               pChannel->TapiHookFlashTime.nMaxTime,
                               IFX_FALSE, IFX_FALSE);
            pChannel->TapiDialData.nHookState = TAPI_HOOK_STATE_ONHOOK_CONFIRM;
         }
         else
         {
            /* If TapiHookOnTime <= TapiHookFlashTime.nMaxTime,
             * skip over state TAPI_HOOK_STATE_ONHOOK_CONFIRM: offhook is
             already confirmed.  */
            /* IFX_TRUE => hook event */
            memset(&tapiEvent, 0, sizeof(IFX_TAPI_EVENT_t));
            tapiEvent.id = IFX_TAPI_EVENT_FXS_ONHOOK;
            ret = IFX_TAPI_Event_Dispatch(pChannel, &tapiEvent);
            if(ret != IFX_SUCCESS)
            {
               /* \todo if dispatcher error?? */
            }
         }
         break;
      case TAPI_HOOK_STATE_FLASH_VAL:
         /* Still Offhook indicates completion of flash */
         memset(&tapiEvent, 0, sizeof(IFX_TAPI_EVENT_t));
         tapiEvent.id = IFX_TAPI_EVENT_FXS_FLASH;
         ret = IFX_TAPI_Event_Dispatch(pChannel,&tapiEvent);
         if(ret != IFX_SUCCESS)
         {
            /* \todo if dispatcher error?? */
         }
         break;
      case TAPI_HOOK_STATE_ONHOOK_CONFIRM:
         /* Timer onhook expires: onhook has lasted long enough to be final */
         /* IFX_TRUE => hook event */
         memset(&tapiEvent, 0, sizeof(IFX_TAPI_EVENT_t));
         tapiEvent.id = IFX_TAPI_EVENT_FXS_ONHOOK;
         ret = IFX_TAPI_Event_Dispatch(pChannel, &tapiEvent);
         if(ret != IFX_SUCCESS)
         {
            /* \todo if dispatcher error?? */
         }
         break;

      case TAPI_HOOK_STATE_ONHOOK_VAL:
         /* digit_l_min expires: onhook has lasted long enough to be a
            low pulse (not noise). Now reset the pulse counter,
            ready for next digit */
         pChannel->TapiDialData.nHookChanges = 0;
         /* NOTE: the "break" statement has been intentionally omitted */
         /*lint -fallthrough */
      case TAPI_HOOK_STATE_DIAL_L_VAL:
         /* digit_l_min expires: onhook has lasted long enough to be a
            certain low pulse (not noise). The next state is the overlap with
            flash hook */
         if (pChannel->TapiDigitLowTime.nMaxTime <
             pChannel->TapiHookFlashTime.nMinTime)
         {
            /* no overlap of hook flash min and pulse max time */
            TAPI_SetTime_Timer(pChannel->TapiDialData.DialTimerID,
                               pChannel->TapiDigitLowTime.nMaxTime -
                               pChannel->TapiDigitLowTime.nMinTime,
                               IFX_FALSE, IFX_FALSE);
         }
         else
         {
            TAPI_SetTime_Timer(pChannel->TapiDialData.DialTimerID,
                               pChannel->TapiHookFlashTime.nMinTime -
                               pChannel->TapiDigitLowTime.nMinTime,
                               IFX_FALSE, IFX_FALSE);
         }
         pChannel->TapiDialData.nHookState = TAPI_HOOK_STATE_PULSE_L_CONFIRM;
         break;
      case TAPI_HOOK_STATE_PULSE_L_FLASH_CONFIRM:
         /* Overlap time of flash hook min and digit low max time ended */
         TAPI_SetTime_Timer(pChannel->TapiDialData.DialTimerID,
                            pChannel->TapiHookFlashTime.nMaxTime -
                            pChannel->TapiDigitLowTime.nMaxTime,
                            IFX_FALSE, IFX_FALSE);
         pChannel->TapiDialData.nHookState = TAPI_HOOK_STATE_FLASH_CONFIRM;

         break;

      default:
         /* all other states represent error cases */
         break;
   }

   return;
}


/**

   Sets the validation timers for hook, pulse digit and hook flash during
   system startup.

   \param pChannel        - handle to TAPI_CHANNEL structure
   \param pTime           - type of validation setting, min and max time in milliseconds

   \return
   IFX_SUCCESS or IFX_ERROR

   \remark
   For timers that do not distinguish between a min and max value, both
   min and max values are set equal.
*/
IFX_int32_t TAPI_Phone_Validation_Time(TAPI_CHANNEL *pChannel,
                                       IFX_TAPI_LINE_HOOK_VT_t const *pTime)
{
   IFX_int32_t ret = IFX_SUCCESS;

   /* zero timer values are not allowed */
   if (pTime->nMinTime == 0 || pTime->nMaxTime == 0)
   {
      TRACE(TAPI_DRV,DBG_LEVEL_HIGH,
            ("DRV_ERROR: zero validation timer values not allowed for channel %d\n",
                                     pChannel->nChannel));
      return IFX_ERROR;
   }

   /* check whether min timer value > max timer value */
   if (pTime->nMinTime > pTime->nMaxTime)
   {
      TRACE(TAPI_DRV,DBG_LEVEL_HIGH,
           ("DRV_ERROR: min value %ld > max value %ld for channel %d\n",
             pTime->nMinTime, pTime->nMaxTime, pChannel->nChannel));
      return IFX_ERROR;
   }

   /* configure the validation timers at system startup */
   switch (pTime->nType)
   {
      case IFX_TAPI_LINE_HOOK_VT_HOOKOFF_TIME:
         pChannel->TapiHookOffTime.nType    = pTime->nType;
         pChannel->TapiHookOffTime.nMinTime = pTime->nMinTime;
         pChannel->TapiHookOffTime.nMaxTime = pTime->nMaxTime;
         break;

      case IFX_TAPI_LINE_HOOK_VT_HOOKON_TIME:
         pChannel->TapiHookOnTime.nType    = pTime->nType;
         pChannel->TapiHookOnTime.nMinTime = pTime->nMinTime;
         pChannel->TapiHookOnTime.nMaxTime = pTime->nMaxTime;
         break;

      case IFX_TAPI_LINE_HOOK_VT_HOOKFLASH_TIME:
         pChannel->TapiHookFlashTime.nType    = pTime->nType;
         pChannel->TapiHookFlashTime.nMinTime = pTime->nMinTime;
         pChannel->TapiHookFlashTime.nMaxTime = pTime->nMaxTime;
         break;

      case IFX_TAPI_LINE_HOOK_VT_DIGITLOW_TIME:
         pChannel->TapiDigitLowTime.nType    = pTime->nType;
         pChannel->TapiDigitLowTime.nMinTime = pTime->nMinTime;
         pChannel->TapiDigitLowTime.nMaxTime = pTime->nMaxTime;
         break;

      case IFX_TAPI_LINE_HOOK_VT_DIGITHIGH_TIME:
         pChannel->TapiDigitHighTime.nType    = pTime->nType;
         pChannel->TapiDigitHighTime.nMinTime = pTime->nMinTime;
         pChannel->TapiDigitHighTime.nMaxTime = pTime->nMaxTime;
         break;

      case IFX_TAPI_LINE_HOOK_VT_INTERDIGIT_TIME:
         pChannel->TapiInterDigitTime.nType    = pTime->nType;
         pChannel->TapiInterDigitTime.nMinTime = pTime->nMinTime;
         pChannel->TapiInterDigitTime.nMaxTime = pTime->nMaxTime;
         break;

      default:
         TRACE(TAPI_DRV,DBG_LEVEL_HIGH,
               ("DRV_ERROR: unknown validation type 0x%x\n",pTime->nType));
         return IFX_ERROR;

   }
   return ret;
}
