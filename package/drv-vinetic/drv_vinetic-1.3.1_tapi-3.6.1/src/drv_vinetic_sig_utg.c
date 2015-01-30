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
   Module      : drv_vinetic_sig_utg.c
   Description : This file implements the SIG-UTG module
******************************************************************************/

/**
   \file   drv_vinetic_utg.c
   \remark This file contains the declaration of the functions
           for the Universal Tone Generator (UTG)
*/

#include "drv_api.h"
#include "drv_vinetic_sig_priv.h"
#include "drv_vinetic_api.h"
#include "drv_vinetic_basic.h"

/* ============================= */
/* Local function declaration    */
/* ============================= */

#if (VIN_CFG_FEATURES & VIN_FEAT_VOICE)
static IFX_int16_t vinetic_sig_UTG_CalcLevel(IFX_int32_t level);
static IFX_int32_t vinetic_sig_UTG_SetCoeff (IFX_TAPI_TONE_SIMPLE_t const *pTone, 
                                             IFX_uint16_t *pCmd);
#endif /* (VIN_CFG_FEATURES & VIN_FEAT_VOICE) */

static IFX_int32_t vinetic_sig_UTG_Start (IFX_TAPI_LL_CH_t *pLLChannel,
                                          IFX_TAPI_TONE_SIMPLE_t const *pSimpleTone,
                                          TAPI_TONE_DST dst,
                                          IFX_uint8_t nUtg);
static IFX_int32_t vinetic_sig_UTG_Stop  (VINETIC_CHANNEL *pCh,
                                          IFX_uint8_t nUtg);



/* ============================= */
/* Local function definition     */
/* ============================= */

#if (VIN_CFG_FEATURES & VIN_FEAT_VOICE)
/**
   function to calculate the utg level coefficient

   \param level - level in [0.1 dB]
   \return
      utg level coefficient
   \remarks
      none
*/
static IFX_int16_t vinetic_sig_UTG_CalcLevel (IFX_int32_t level)
{
   long lvl;
   IFX_uint32_t  exp, tenExp;
   IFX_uint32_t  shift;
   IFX_int32_t   i;

   if (level == 0)
      return 0xFF;
   /* calclulate the desired level in mdB and the exponent */
   lvl = level * 100;
   exp = (IFX_uint32_t)((-lvl) / 20);

   /* set the initial shift factor according to the level */
   if (lvl <= -20000)
      shift  = 1000;
   else if (lvl <= -10000)
      shift  = 10000;
   else
      shift  = 10000;

   /* the initial value of the result (tenExp) is the shift factor
      which will be compensated in the last step (calculating the
      coefficient)
      return ((xxxxx * shift) / tenExp); */
   tenExp = shift;

   /* go over allelements in the tens array, starting with the
      largest entry and ... */
   for (i=27;i>=0;i--)
   {
      /* ... loop while the current tens[i][0] is part of the remaining exp */
      while (exp >= tens[i][0])
      {

         /* calulate part of the result for tens[i][0], check which accuracy
            of the tens[i][1] can be used to multiply tenExp without overflow */
         if ((C2_31 / tenExp) > tens[i][1])
         {
            /* use the unscaled tens[i][1] value to calculate
               the tenExp share */
            tenExp *= tens[i][1];
            tenExp /= 100000;
         }
         else if ( (C2_31 / tenExp) > ROUND_DIV10(tens[i][1]))
         {
            /* scale the tens[i][1] value by 10
               to calculate the tenExp share */
            tenExp *= ROUND_DIV10(tens[i][1]);
            tenExp /= 10000;
         }
         else if ( (C2_31 / tenExp) > ROUND_DIV100(tens[i][1]))
         {
            /* scale the tens[i][1] value by 100
               to calculate the tenExp share */
            tenExp *= ROUND_DIV100(tens[i][1]);
            tenExp /= 1000;
         }
         else
         {
            /* scale the tens[i][1] value by 1000
               to calculate the tenExp share */
            tenExp *= ROUND_DIV1000(tens[i][1]);
            tenExp /= 100;
         }

         /* calculate the remaining exp, i.e. subtract that part of exponent
            that has been calculated in this step */
         exp -= tens[i][0];
      }
   }

   /* calculate the coefficient according to the specification... */
   return (IFX_int16_t) ROUND (((C2_8*shift)/(tenExp)));
}


/**
   Do low level UTG (Universal Tone Generator) configuration and activation
\param
   pChannel       - handle to TAPI_CHANNEL structure
\param
   pSimpleTone    - internal simple tone table entry
\param
   dst            - destination where to play the tone
\param
   UtgNum         - UTG submodule to play the tone on
\return
   IFX_SUCCESS/IFX_ERROR
\remarks
   The selected signalling channel has to be activated before this command can
   be sent.
   For the UTG, the coefficients have to be programmed with a separate command,
   before the UTG can be activated. The UTG must be inactive when programming
   the coefficients.
   The DTMF/AT Generator is overlaid with the tone generator, which means
   the same resources for the DTMF/AT Generator and the UTG must not be active
   simultaneously! DTMF/AT will be deactivated to avoid simultaneous activation.

   It is assumed that the UTG is not active before calling this function which
   programs the UTG coefficients.
   In case the UTG event interrupt is needed, the calling function should make
   sure that the DTMF/AT Generator is deactivated before.
   This function is completely protected from concurrent member access.
*/
static
IFX_int32_t vinetic_sig_UTG_Start (IFX_TAPI_LL_CH_t *pLLChannel,
                                   IFX_TAPI_TONE_SIMPLE_t const *pSimpleTone,
                                   TAPI_TONE_DST nDst,
                                   IFX_uint8_t nUtg)
{
   IFX_int32_t      ret = IFX_ERROR;
#if (VIN_CFG_FEATURES & VIN_FEAT_VOICE)
   VINETIC_CHANNEL *pCh  = (VINETIC_CHANNEL *)pLLChannel;
   VINETIC_DEVICE *pDev = (VINETIC_DEVICE*) (pCh->pParent);
   FWM_SIG_UTG     *pUtgCmd;
   IFX_uint16_t     pCmdCoef [27] = {0};
   IFX_uint8_t      utgRes = 0, 
                    ch = pCh->nChannel - 1;

   /* calling function ensures valid parameter */
   IFXOS_ASSERT(pSimpleTone != IFX_NULL);
   IFXOS_ASSERT(nUtg < pDev->caps.nUtgPerCh);

   /* choose utg resource according to the FW capabilities */
   if (nUtg == 0)
   {  /* resources #0..#3 for first utg */
      utgRes = (IFX_uint8_t)ch;
      /* Setup UTG1 interrupt masks.
      Note : Mask rising edge interrupts for VIN_UTG1_ACT_MASK.
             Allow falling edge interrupts for VIN_UTG1_ACT_MASK.
      Note: VIN_UTG2_ACT_MASK is overlaid with V2CPE_EDSP1_INT1_DTMFG_ACT
            For this reason the mask settings are set here within "Tone_UTG_Start"! */
      ret = VINETIC_Host_Set_EdspIntMask (pCh, VIN_EDSP_UTG1, IFX_FALSE, 0,
                                          VIN_UTG1_ACT_MASK);
      if (ret == IFX_ERROR)
      {
         return IFX_ERROR;
      }
   }
   else if (nUtg == 1)
   {
      /* second resource, UTG2, uses different resources as UTG1 */
      utgRes = (IFX_uint8_t)(ch + pDev->caps.nSIG);
      /* check whether CID Generator is active,
      because the status bits for CIS and UTG2 are overlaid
      => UTG cannot be used while CIS is active! */
      if (pDev->pChannel[ch].pSIG->cid_sender[2] & SIG_CID_EN)
      {
         SET_ERROR (VIN_ERR_CID_RUNNING);
         return (IFX_ERROR);
      }
      /* Setup UTG2 interrupt masks.
      Note : Mask rising edge interrupts for VIN_UTG2_ACT_MASK.
             Allow falling edge interrupts for VIN_UTG2_ACT_MASK.
      Note: VIN_UTG2_ACT_MASK is overlaid with V2CPE_EDSP1_INT1_CIS_ACT
            For this reason the mask settings are set here within "Tone_UTG_Start"! */
      ret = VINETIC_Host_Set_EdspIntMask (pCh, VIN_EDSP_UTG2, IFX_FALSE, 0,
                                          VIN_UTG2_ACT_MASK);
      if (ret == IFX_ERROR)
      {
         return IFX_ERROR;
      }
   }
   else
   {
      SET_ERROR (VIN_ERR_NORESOURCE);
      return IFX_ERROR;
   }

   /* prepare command for coefficients */
   memset (pCmdCoef, 0, sizeof(pCmdCoef));
   /* rewrite part of the command header and set the coefficients */
   pCmdCoef [0] = CMD1_EOP | (utgRes & CMD1_CH);
   pCmdCoef [1] = ECMD_UTG_COEF;
   vinetic_sig_UTG_SetCoeff (pSimpleTone, pCmdCoef);

   ret = CmdWrite (pDev, pCmdCoef, 25);

   if (ret != IFX_SUCCESS)
   {
      TRACE (VINETIC, DBG_LEVEL_HIGH,
            ("VINETIC: UTG_Start: failure writing UTG coefficients\n\r"));
      return IFX_ERROR;
   }

   /* pointer to cached UTG control message */
   pUtgCmd = &pCh->pSIG->fw_sig_utg[nUtg];
   /* overwrite utg resource number, because here the "overall number" is used */
   pUtgCmd->bit.utgnr = (utgRes & CMD1_CH);

   /* Tone signal injection and muting the voice signal
     into adder 1 and/or adder 2 */
   switch (nDst)
   {
      case  TAPI_TONE_DST_NET:
         /* put it to adder 1 to network */
         pUtgCmd->bit.a1 = SIG_UTG_INJECT;
         pUtgCmd->bit.a2 = SIG_UTG_NONE;
         break;
      case  TAPI_TONE_DST_NETLOCAL:
         /* put it to adder 1 to network and adder 2 for local */
         pUtgCmd->bit.a1 = SIG_UTG_INJECT;
         pUtgCmd->bit.a2 = SIG_UTG_INJECT;
         break;
      case  TAPI_TONE_DST_LOCAL:
      case  TAPI_TONE_DST_DEFAULT:
      default:
         /* play it locally: take adder 2 */
         pUtgCmd->bit.a1 = SIG_UTG_NONE;
         pUtgCmd->bit.a2 = SIG_UTG_INJECT;
         break;
   }

   /* activate the UTG with the tone action previously programmed */
   pUtgCmd->bit.sm = 0;
   pUtgCmd->bit.en = 1;

   ret = CmdWrite (pDev, pUtgCmd->value, 1);

   if (ret == IFX_SUCCESS)
   {
      if (pSimpleTone->loop > 0 || pSimpleTone->pause > 0)
      {
         /* auto stop after loop or after each generation step */
         pUtgCmd->bit.sm = 1;
         pUtgCmd->bit.en = 0;
         ret = CmdWrite (pDev, pUtgCmd->value, 1);
      }
   }
#endif /* (VIN_CFG_FEATURES & VIN_FEAT_VOICE) */

   return ret;
}


/**
   Stops the UTG
   \param   pCh    - pointer to VINETIC channel structure

   \return
      IFX_SUCCESS or IFX_ERROR
*/
static
IFX_int32_t vinetic_sig_UTG_Stop (VINETIC_CHANNEL *pCh, IFX_uint8_t nUtg)
{
   IFX_int32_t     ret = IFX_SUCCESS;
   VINETIC_DEVICE *pDev = pCh->pParent;
   FWM_SIG_UTG    *pUtgCmd = &pCh->pSIG->fw_sig_utg[nUtg];
   IFX_uint8_t     utgRes = 0;

   /* calling function ensures valid parameter */
   IFXOS_ASSERT(nUtg < pDev->caps.nUtgPerCh);

   /* choose utg resource according to the FW capabilities */
   if (nUtg == 0)
   {  /* resources #0..#3 for first utg */
      utgRes = (IFX_uint8_t)(pCh->nChannel - 1);
   }
   else if (nUtg == 1)
   {  /* resources #4..#7 for second utg */
      utgRes = (IFX_uint8_t)(pCh->nChannel - 1) + pDev->caps.nSIG; 
   }
   else
   {
      SET_ERROR (VIN_ERR_NORESOURCE);
      return IFX_ERROR;
   }

   /* Mask the interrupt before actually turning off the UTG. The interrupt
      is only needed for restarting the UTG. So we do not need it here. */

   if (nUtg == 0)
   {  /* first utg */
      ret = VINETIC_Host_Set_EdspIntMask (pCh, VIN_EDSP_UTG1, IFX_FALSE, 0, 0);
   }
   else if (nUtg == 1)
   {  /* second utg */
      ret = VINETIC_Host_Set_EdspIntMask (pCh, VIN_EDSP_UTG2, IFX_FALSE, 0, 0);
   }

   /* overwrite utg resource number, because here the "overall number" is used */
   pUtgCmd->bit.utgnr = (utgRes & CMD1_CH);

   /* INFO: 2CPE requires to enable the UTG before disabling it with SM bit 
            set to zero. */

   /* Stop tone playing for tones which is using the UTG Force the UTG to
      stop immediately if already disabled */
   if (pUtgCmd->bit.sm == 1 && pUtgCmd->bit.en == 0)
   {
      /* enable the UTG first because it is already disabled and auto stop
         is activated. So first enabled it and set auto stop to false */
      pUtgCmd->bit.en = 1;
      pUtgCmd->bit.sm = 0;
      ret = CmdWrite (pDev, pUtgCmd->value, 1);
   }
   if (pUtgCmd->bit.en == 1)
   {
      pUtgCmd->bit.en = 0;
      pUtgCmd->bit.sm = 0;
      ret = CmdWrite (pDev, pUtgCmd->value, 1);
   }

#ifndef FW_ETU
   /* reenable the DTMF module */
   if (ret == IFX_SUCCESS && nUtg == 0)
   {
      pDev->pChannel[pCh->nChannel - 1].pSIG->fw_sig_dtmfgen.bit.en = 1;
      ret = CmdWrite (pDev,
                      pDev->pChannel[pCh->nChannel - 1].pSIG->fw_sig_dtmfgen.value,
                      CMD_SIG_DTMFGEN_LEN);
   }
#endif /* FW_ETU */

   return ret;
}


/**
   Set the coefficients for the UTG
\param
   pTone - internal simple tone table entry
\param
   pCmd  - command to be filled, it is not cleared in this function.
\return
   IFX_SUCCESS
\remark
   A memset must be done on pCmd and the command words have to be set
   by the calling function
*/
static IFX_int32_t vinetic_sig_UTG_SetCoeff (IFX_TAPI_TONE_SIMPLE_t const *pTone, 
                                             IFX_uint16_t *pCmd)
{
   IFX_uint8_t step = 12, i;

   /* set frequency A for F_1 */
   pCmd [7] = (IFX_uint16_t)((8192 * pTone->freqA) / 1000);
   /* set frequency B  for F_2 */
   pCmd [8] = (IFX_uint16_t)((8192 * pTone->freqB) / 1000);
   /* set frequency C for F_3 */
   pCmd [9] = (IFX_uint16_t)((8192 * pTone->freqC) / 1000);
   /* set frequency C for F_3 */
   pCmd [10] = (IFX_uint16_t)((8192 * pTone->freqD) / 1000);

   /* set power level for LEV_1, LEV_2 and LEV_3 */
   pCmd [11] = ((IFX_uint8_t)vinetic_sig_UTG_CalcLevel(pTone->levelA) << 8) |
                (IFX_uint8_t)vinetic_sig_UTG_CalcLevel(pTone->levelB);
   pCmd [12] = ((IFX_uint8_t)vinetic_sig_UTG_CalcLevel(pTone->levelC) << 8) |
                (IFX_uint8_t)vinetic_sig_UTG_CalcLevel(pTone->levelD);

   /* set step times:  T_x */
   for (i = 0; i < IFX_TAPI_TONE_STEPS_MAX; ++i)
   {
      /* check for last cadence step */
      if (pTone->cadence[i] == 0)
      {
         /* special case: continous tone for the one and only cadence step.
            In that case a value FFFF must be programmed to the firmware */
         if (pTone->pause == 0 && pTone->loop == 0 && i == 1)
         {
            pCmd [step - 1] = 0xFFFF;
         }
         break;
      }
      /* increase step to timing. Step is initialized with 12 (one before
         the first timing setting */
      step++;
      /* the firmware uses a timebase of 0.5 ms */
      pCmd [step] = (IFX_uint16_t)(2 * pTone->cadence[i]);
      /* move step to step setting */
      step++;
      /* set mask for MSK_i - use frequency A. Initialize the field */
      if (pTone->frequencies[i] & IFX_TAPI_TONE_FREQA)
         pCmd [step] = ECMD_UTG_COEF_MSK_F1;
      else
         pCmd [step] = 0;
      /* set mask for MSK_i - use frequency B  */
      if (pTone->frequencies[i] & IFX_TAPI_TONE_FREQB)
         pCmd [step] |= ECMD_UTG_COEF_MSK_F2;
      /* set mask for MSK_i - use frequency C  */
      if (pTone->frequencies[i] & IFX_TAPI_TONE_FREQC)
         pCmd [step] |= ECMD_UTG_COEF_MSK_F3;
      /* set mask for MSK_i - use frequency D  */
      if (pTone->frequencies[i] & IFX_TAPI_TONE_FREQD)
         pCmd [step] |= ECMD_UTG_COEF_MSK_F4;
      if (pTone->modulation[i] & IFX_TAPI_TONE_MODULATION_ON)
         pCmd [step] |= ECMD_UTG_COEF_MSK_M12;
      /* set the next step */
      pCmd[step] |= (IFX_uint16_t)((i+1) << 12);
   }

   /* clear the next step to start at 0 */
   pCmd [step] &= ~ECMD_UTG_COEF_MSK_NXT;
   if (pTone->pause == 0 && pTone->loop)
   {
      /* The deactivation of the tone
         generator is delayed until the current tone generation step
         has been completely executed inclusive of the requested
         repetitions */
      pCmd [step] |= (IFX_uint16_t)((pTone->loop - 1) << 9) | ECMD_UTG_COEF_SA_01;
   }
   else
   {
      /* The deactivation of the tone
         generator is delayed until the current tone generation step
         has been completely executed. The deactivation is made
         immediately after the execution of the current tone
         generation step. */
      pCmd [step] |= ECMD_UTG_COEF_SA_10;
   }
   /* Always set the GO_1 and GO_2 coefficients to 0 dB (0x8000), i.e.
      tone level is defined only via the level settings above.
      If multiple tones are played out simultaneously the application has
      to adapt the levels to prevent clipping. */
   pCmd [25] = 0x8000;
   pCmd [26] = 0x8000;
   return IFX_SUCCESS;
}
#endif /* (VIN_CFG_FEATURES & VIN_FEAT_VOICE) */


/* ============================= */
/* Global function definitions   */
/* ============================= */


/**
   Do low level UTG (Universal Tone Generator) configuration and activation

   This function handles all necessary steps to play out a full simple tone
   on the UTG. It returns immediately.
   Per device there may be multiple resources to play a tone. If the resource
   number exceeds the amount of available resources this function returns an
   error code VMMC_statusNoRes.

   The selected signalling channel will be automatically activated when this
   function is executed.
   First the coefficients are programmed, before the UTG is be activated.
   The UTG must be inactive when programming the coefficients.
   Also here is no check done.
   It is assumed that the UTG is not active before calling this function which
   programs the UTG coefficients.

   The tone signal is injection and muting the voice signal into adder 1 for
   network and/or adder 2 for local tones .

\param pLLChannel  Handle to TAPI low level channel structure
\param pToneCoeff  Handle to the tone definition to play. May not be NULL.
\param res         Resource number which is used for playing the tone. The
                   available resources are device dependend.
\param dst         Destination where to play the tone: local or network
\return Return value according to IFX_return_t
   - IFX_ERROR if an error occured
   - IFX_SUCCESS if successful

   This function handles all necessary steps to play out the complete
   tone sequence. It returns immediately.
*/
IFX_int32_t IFX_TAPI_LL_SIG_UTG_Start (IFX_TAPI_LL_CH_t *pLLChannel,
                                       IFX_TAPI_TONE_SIMPLE_t const *pSimpleTone,
                                       TAPI_TONE_DST dst,
                                       IFX_uint8_t res)
{
   VINETIC_CHANNEL *pCh = (VINETIC_CHANNEL *)pLLChannel;
   VINETIC_DEVICE *pDev = (VINETIC_DEVICE*) (pCh->pParent);
   IFX_int32_t ret = IFX_SUCCESS;
#ifndef FW_ETU
   IFX_uint8_t ch = pCh->nChannel - 1;
#endif

   /* check validity of the parameters */
   if (res >= pDev->caps.nUtgPerCh)
   {
      TRACE (VINETIC, DBG_LEVEL_HIGH, 
             ("VMMC: Invalid resource parameter to UTG_Start\n"));
      SET_ERROR (VIN_ERR_NORESOURCE);
      return IFX_ERROR;
   }
   if (pSimpleTone == IFX_NULL)
   {
      TRACE (VINETIC, DBG_LEVEL_HIGH, 
             ("VMMC: Tone parameter to UTG_Start is NULL\n"));
      SET_ERROR (VIN_ERR_FUNC_PARM);
      return IFX_ERROR;
   }

   TAPI_Tone_Set_Source (pCh->pTapiCh, res, IFX_TAPI_TONE_SRC_DSP);

   IFXOS_MutexLock (pDev->memberAcc);
   /* If not already running activate the signalling module in this channel */
   ret = Dsp_SigActStatus (pDev, pCh->nChannel - 1, IFX_TRUE, CmdWrite);

#ifndef FW_ETU
   /* disable DTMF / AT generator because it is overlaid with the UTG */
   if ( ret == IFX_SUCCESS &&
        pDev->pChannel[ch].pSIG->fw_sig_dtmfgen.bit.en != 0)
   {
      pDev->pChannel[ch].pSIG->fw_sig_dtmfgen.bit.en = 0;
      ret = CmdWrite (pDev,
                      pDev->pChannel[ch].pSIG->fw_sig_dtmfgen.value,
                      CMD_SIG_DTMFGEN_LEN);
   }
#endif
   /* Activate the universal tone generator disable the DTMF generator
      and program the simple tone sequence */
   if (ret == IFX_SUCCESS)
   {
      ret = vinetic_sig_UTG_Start(pCh, pSimpleTone, dst, res);
   }
   IFXOS_MutexUnlock (pDev->memberAcc);

   return ret;
}


/**
   Stop playing a tone of given definition immediately
\param pLLChannel  Handle to TAPI low level channel structure
\param res Resource number which is used for playing the tone. The
           available resources is device dependend. Usually res is set to zero.
\return Return value according to IFX_return_t
   - IFX_ERROR if an error occured
   - IFX_SUCCESS if successful
*/
IFX_int32_t  IFX_TAPI_LL_SIG_UTG_Stop (IFX_TAPI_LL_CH_t *pLLChannel,
                                       IFX_uint8_t res)
{
   VINETIC_CHANNEL *pCh = (VINETIC_CHANNEL *) pLLChannel;
   VINETIC_DEVICE *pDev = (VINETIC_DEVICE *) pCh->pParent;
   IFX_int32_t ret = IFX_SUCCESS;

   /* check for valid resource number */
   if (res >= pDev->caps.nUtgPerCh)
   {
      TRACE (VINETIC, DBG_LEVEL_HIGH, 
             ("VINETICC: UTG stop called on incorrect ressource number\n"));
      SET_ERROR (VIN_ERR_NORESOURCE);
      return IFX_ERROR;
   }

   IFXOS_MutexLock (pDev->memberAcc);
   ret = vinetic_sig_UTG_Stop (pCh, res);
   if (ret == IFX_SUCCESS)
      ret = Dsp_SigActStatus (pDev, pCh->nChannel - 1, IFX_FALSE, CmdWrite);
   IFXOS_MutexUnlock (pDev->memberAcc);

   return ret;
}


/**
   Returns the total number of UTGs per channel
\param pLLChannel  Handle to TAPI low level channel structure
\return Returns the total number of UTGs per channel - function always succeeds
*/
IFX_uint8_t IFX_TAPI_LL_SIG_UTG_Count_Get (IFX_TAPI_LL_CH_t *pLLChannel)
{
   VINETIC_CHANNEL *pCh = (VINETIC_CHANNEL *) pLLChannel;
   VINETIC_DEVICE *pDev = (VINETIC_DEVICE *) pCh->pParent;
   return pDev->caps.nUtgPerCh;
}


/**
 Deactivate UTG. Called upon event to update the state information.
\param  pLLChannel  Handle to TAPI low level channel structure
\param  utgNum      utg resource number
\return none
*/
IFX_void_t IFX_TAPI_LL_UTG_Event_Deactivated (IFX_TAPI_LL_CH_t *pLLChannel,
                                              IFX_uint8_t utgNum)
{
   VINETIC_CHANNEL *pCh  = (VINETIC_CHANNEL *)pLLChannel;
   FWM_SIG_UTG     *pUtgCmd;

   /* we are called upon event with already verified parameters */
   IFXOS_ASSERT(utgNum < ((VINETIC_DEVICE *)pCh->pParent)->caps.nUtgPerCh);
   /* after asserting valid resource number get the cached message */
   pUtgCmd = &pCh->pSIG->fw_sig_utg[utgNum];

   /* reset the cache state */
   pUtgCmd->bit.sm = 0;
   pUtgCmd->bit.en = 0;
}
