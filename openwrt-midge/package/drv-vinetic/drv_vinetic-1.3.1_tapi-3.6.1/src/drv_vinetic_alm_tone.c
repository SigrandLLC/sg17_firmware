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
   Module      : drv_vinetic_alm_tone.c
 \file This file contains the implementation  of the functions
       related to tone operations for TAPI
   \remarks
   The tone implementation uses the TAPI as interface. The VINETIC supports
   tone generation via a firmware signaling module called UTG and additionally
   vi an analog tone generator (TG, actually two). Most of the services are
   using the UTG, because is reduces the host utilization and gains more
   features. The TG is only used in very rare situation, when the UTG is not
   available. This is the case, when the coder is already connected to another
   analog part. The TG then can generate a busy tone.
   The resources are handled by the low level part of the driver. Thus when TAPI
   requests a resource, that is not available, the low level returns an error.
   Timer and state handling is also implemented in the low level part.
   The TAPI provides a maximum set of resources, that may not be available.

******************************************************************************/

/* ============================= */
/* includes                      */
/* ============================= */

#include "ifx_types.h"
#include "drv_vinetic_alm_priv.h"
#include "drv_vinetic_api.h"
#ifndef VIN_2CPE
#include "drv_vinetic_cram.h"
#endif /* VIN_2CPE */


#ifndef VIN_2CPE
/**
   Function called from tone timer and switch the tone on/off.
\param
   Timer  - TimerID of timer that exipres
\param
   nArg   - Argument of timer including the TAPI_CHANNEL structure
           (as integer pointer)
\return
   none
\remarks
   Only for compability reasons
*/
IFX_void_t Tone_PredefOnTimer(Timer_ID Timer, IFX_int32_t nArg)
{
   VINETIC_CHANNEL *pCh = (VINETIC_CHANNEL *) nArg;
   VINETIC_DEVICE  *pDev     = (VINETIC_DEVICE *)pCh->pParent;
   TAPI_CHANNEL *pChannel = &pDev->pTapiDev->pTapiChanelArray[pCh->nChannel-1];
   IFX_uint32_t     nTime;
   IFX_uint16_t     nDscr;

   if (pChannel->TapiTgToneData.nToneState == TAPI_CT_IDLE)
   {
      /* error: tone already stopped */
      TRACE(VINETIC,DBG_LEVEL_HIGH,
         ("\n\rVINETIC IFX_ERROR: No Tone is played yet!\n\r"));
      return;
   }
   else if (pChannel->TapiTgToneData.nToneState == TAPI_CT_ACTIVE)
   {
      /* data to stop tone generator(s) (COR8_OFF) | (PTG_OFF) | (ALL_TG_OFF) */
      nDscr = 0;
      /* calculate off-time for timer */
      nTime = pChannel->TapiTgToneData.nOffTime / 4;
      if (nTime == 0)
      {
         nTime = pChannel->TapiTgToneData.nOnTime / 4;
         pChannel->TapiTgToneData.nToneState = TAPI_CT_ACTIVE;
      }
      else
      {
         pChannel->TapiTgToneData.nToneState = 2;
      }
   }
   else  /* pChannel->TapiTgToneData.nToneState == 2 */
   {
      /* data to start tone generator 1 */
      nDscr = (DSCR_COR8)|(DSCR_PTG)|(DSCR_TG1_EN);
      if (pChannel->TapiTgToneData.bUseSecondTG)
      {
         /* also start tone generator 2*/
         nDscr |= DSCR_TG2_EN;
      }
      /* calculate on-time for timer */
      nTime = pChannel->TapiTgToneData.nOnTime / 4;
      /* change tone state */
      pChannel->TapiTgToneData.nToneState = TAPI_CT_ACTIVE;
   }
   /* check time for smallest timer interval */
   if (nTime < 20)
   {
      nTime = 20;
   }
   if (pChannel->TapiTgToneData.nOffTime != 0)
   {
      /* call target function to set tone generator(s) */
      RegModify (pDev, CMD1_SOP|(pChannel->nChannel), SOP_OFFSET_DSCR,
                (DSCR_COR8)|(DSCR_PTG)|(DSCR_TG1_EN|DSCR_TG2_EN), nDscr);
   }
   /* set tone timer */
   TAPI_SetTime_Timer(Timer, nTime, IFX_FALSE, IFX_TRUE);
}

#endif /* VIN_2CPE */




#ifndef VIN_2CPE /* No tone generator on analog side for 2CPE */
/**
   Tone step of tone generator.
\param
   pCh   - handle to a VINETIC channel structure
\param
   pTone   - internal simple tone table entry
\return
   none
\remarks

*/

TAPI_CMPLX_TONE_STATE_t TG_ToneStep(VINETIC_CHANNEL* pCh,
                                    IFX_TAPI_TONE_SIMPLE_t const *pTone,
                                    IFX_uint8_t res,
                                            IFX_uint8_t *nToneStep)
{
   if (pCh->nToneStep > IFX_TAPI_TONE_STEPS_MAX ||
       pTone->cadence[pCh->nToneStep] == 0)
   {
      pCh->nToneStep = 0;
      if (pTone->loop != 0)
      {
         pCh->nTone_Cnt--;
         if (pCh->nTone_Cnt == 0)
         {
            DSCR.value &= ~(DSCR_COR8 | DSCR_PTG | DSCR_TG1_EN | DSCR_TG2_EN);
            wrReg (pCh, DSCR_REG);

            return TAPI_CT_DEACTIVATED;
         }
      }
      if (pTone->loop == 0 || pTone->loop > 1)
      {
         if (pTone->pause > 0)
         {
            return TAPI_CT_ACTIVE_PAUSE;
         }
      }
   }
   DSCR.value &= ~(DSCR_COR8 | DSCR_PTG | DSCR_TG1_EN | DSCR_TG2_EN);
   if (pTone->frequencies[pCh->nToneStep] & IFX_TAPI_TONE_FREQA)
      /* data to start tone generator 1 */
      DSCR.value |= (DSCR_COR8 | DSCR_PTG | DSCR_TG1_EN);
   if (pTone->frequencies[pCh->nToneStep] & IFX_TAPI_TONE_FREQB)
      DSCR.value |= DSCR_TG2_EN;
   /* call target function to set tone generator(s) */
   wrReg (pCh, DSCR_REG);

   pCh->nToneStep++;
   /* to decide which timer as to be started, useful for HL */
   *nToneStep = pCh->nToneStep;
   return TAPI_CT_ACTIVE;
}


#endif /* VIN_2CPE */

/* ============================= */
/* Global function definition    */
/* ============================= */

#ifndef VIN_2CPE /* No tone generator on analog side for 2CPE */
/**
   Play a local tone on one or both tone generators.
\param
   pCh   - handle to a VINETIC channel structure
\param
   vers - chip revision
\param
   freqA - frequency A in mHz
\param
   freqB - frequency B in mHz, or zero if not applicable
\return
   IFX_SUCCESS or IFX_ERROR
*/
IFX_int32_t Tone_TG_SetCoeff (VINETIC_CHANNEL* pCh, IFX_uint8_t vers,
                         IFX_int32_t freqA, IFX_int32_t freqB)
{
   IFX_int32_t ret;
#ifdef VIN_V14_SUPPORT
   COEFF_TAB_ENTRY  tab_entry;
   IFX_uint16_t     pData [4] = {0};
#endif /* VIN_V14_SUPPORT */

   switch (vers)
   {
#ifdef VIN_V14_SUPPORT
      case VINETIC_V1x:
         tab_entry.quantVal = freqA;
         /* now get the coefficient from global frequency / coefficient table.
            result is returned in pEntry */
         ret = Cram_getUnitCoef (IOSET, &tab_entry, (COEFF_TAB_ENTRY *)
                                 VINETIC_CRAM_TGFreqQuantVal_Table);
         if (ret != IFX_SUCCESS)
            return IFX_ERROR;
         pData[0] = LOWWORD ((IFX_uint32_t)tab_entry.CRAMcoeff);
         pData[1] = HIGHWORD ((IFX_uint32_t)tab_entry.CRAMcoeff);
         /* c1 = tab_entry.CRAMcoeff; */
         if (freqB)
         {
            tab_entry.quantVal = (IFX_int32_t)freqB;
            ret = Cram_getUnitCoef (IOSET, &tab_entry,
                     (COEFF_TAB_ENTRY *) VINETIC_CRAM_TGFreqQuantVal_Table);
            if (ret != IFX_SUCCESS)
               return IFX_ERROR;
            pData[2] = LOWWORD  ((IFX_uint32_t)tab_entry.CRAMcoeff);
            pData[3] = HIGHWORD ((IFX_uint32_t)tab_entry.CRAMcoeff);
            /* c2 = tab_entry.CRAMcoeff; */
            ret = RegWrite (pCh->pParent, CMD1_COP | (pCh->nChannel - 1),
                            pData, 4, CRAM_PTG1);
         }
         else
         {
            ret = RegWrite (pCh->pParent, CMD1_COP | (pCh->nChannel - 1),
                            pData, 2, CRAM_PTG1);
         }
         break;
#endif /* VIN_V14_SUPPORT */
#ifdef VIN_V21_SUPPORT
      case VINETIC_V2x:
         /* set the frequency for the first TG */
         ret = setRegVal (pCh, TG1F_REG, freqA);
         if (ret == IFX_SUCCESS)
            ret = wrReg (pCh, TG1F_REG);

         /* set the frequency for the second TG */
         if ((ret == IFX_SUCCESS) && (freqB != 0))
         {
            ret = setRegVal (pCh, TG2F_REG, freqB);
            if (ret == IFX_SUCCESS)
               ret = wrReg (pCh, TG2F_REG);
         }
         break;
#endif /* VIN_V21_SUPPORT */
      default:
         return IFX_ERROR;
   }
   if (ret == IFX_SUCCESS)
   {
      DSCR.value &= ~(DSCR_TG2_EN);
      /* data to start tone generator 1 */
      DSCR.value |= (DSCR_COR8 | DSCR_PTG | DSCR_TG1_EN);
      if (freqB)
         DSCR.value |= DSCR_TG2_EN;
      /* call target function to set tone generator(s) */
      ret = wrReg (pCh, DSCR_REG);
   }
   return ret;
}

/**
   Play a local tone on one or both tone generators.
\param
   pCh   - handle to a VINETIC channel structure
\return
   IFX_SUCCESS or IFX_ERROR
*/
IFX_int32_t Tone_TG_Play (VINETIC_CHANNEL* pCh)
{
   TAPI_DEV* pTapiDev = pCh->pParent->pTapiDev;
   VINETIC_DEVICE    *pDev     = (VINETIC_DEVICE *)pCh->pParent;
   TAPI_CHANNEL   *pChannel = &pDev->pTapiDev->pTapiChanelArray[pCh->nChannel-1];
   IFX_uint32_t toneCode;
   IFX_int32_t freqA, freqB;
   IFX_int32_t ret;
   IFX_TAPI_TONE_SIMPLE_t *pTone;
   TAPI_TONE_DATA_t *pData;

   /* in this function resource number 1 (TG) is used */
   pData = &pChannel->TapiComplexToneData[1];

   if (pData->nType == TAPI_TONE_TYPE_SIMPLE)
   {
      toneCode = pData->nSimpleToneCode;
      /* Retrieve simple tone configuration from internal tone coefficients table */
      pTone = &(pTapiDev->pToneTbl[toneCode].tone.simple);
   }
   else
   {
      SET_ERROR (VIN_ERR_INVALID_TONERES);
      return IFX_ERROR;
   }
   pCh->nToneStep = 0;
   pCh->nTone_Cnt = pTone->loop;
   freqA = (IFX_int32_t)pTone->freqA * 1000;
   if (pTone->frequencies[0] & IFX_TAPI_TONE_FREQB)
   {
      freqB = (IFX_int32_t)pTone->freqB * 1000;
      /* use the second tone generator */
      pChannel->TapiTgToneData.bUseSecondTG = IFX_TRUE;
   }
   else
   {
      freqB = 0;
      /* do not use the second tone generator */
      pChannel->TapiTgToneData.bUseSecondTG = IFX_FALSE;
   }
   /* program the coefficients */
   ret = Tone_TG_SetCoeff (pCh, pDev->nChipMajorRev, freqA, freqB);
   /* tone is playing now, so set the state */
   pData->nToneState = TAPI_CT_ACTIVE;

   return ret;
}

#endif /* VIN_2CPE */


