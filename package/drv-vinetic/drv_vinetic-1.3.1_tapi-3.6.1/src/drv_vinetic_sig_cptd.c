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

/*
   \file
   drv_vinetic_sig_cptd.c
   \remark
      This file contains the definitions of the functions
      for Call Progress Tone Detection module
*/

#include "ifx_types.h"
#include "drv_vinetic_sig_priv.h"
#include "drv_vinetic_api.h"
#include "drv_vinetic_basic.h"


/* ============================= */
/* Local Macros & Definitions    */
/* ============================= */
/* default detection level tolerance, -20dB */
#define CPTD_DETECTION_LEVEL_TOLERANCE  200
/* allow -42dB total power in a pause step */
#define CPTCOEFF_POW_PAUSE       0x0002
/* allow -10dB maximum frequency power to total power ratio for valid tone
   detection */
#define CPTCOEFF_FP_TP_R_DEFAULT 0x075D
/** minimum time for verification of continuos tone in ms */
#define CPTCOEFF_CNT_MINTIME     1000

/** hamming constant for cpt level calculation */
#define CONST_CPT_HAMMING        (  610)
/** blackman constant for cpt level calculation */
#define CONST_CPT_BLACKMAN       (-1580)
/** define which constant is taken for cpt level calculation */
#define CONST_CPT                CONST_CPT_BLACKMAN



/* ============================= */
/* Local function declaration    */
/* ============================= */

IFX_LOCAL IFX_int32_t Dsp_Cptd_SetCoeff (VINETIC_CHANNEL *pCh,
                               IFX_TAPI_TONE_SIMPLE_t const *pTone,
                               IFX_uint16_t *pCmd,
                               DSP_CPTD_FL_t *pFrameLength);
IFX_LOCAL IFX_int32_t Dsp_Cptd_Conf (VINETIC_CHANNEL *pCh,
            IFX_TAPI_TONE_SIMPLE_t const *pTone,
            IFX_int32_t signal, IFX_uint16_t *pCmd,
            DSP_CPTD_FL_t frameLength);

IFX_LOCAL IFX_int16_t cpt_calc_freq       (IFX_uint32_t    f);
IFX_LOCAL IFX_int16_t cpt_calc_level      (IFX_int32_t     level);

/* ============================= */
/* Local function definitions    */
/* ============================= */

/**
   function to calculate the cpt frequency coefficients

   \param f - frequency in [Hz]
   \return
      cpt frequency coefficient
   \remarks
      none
*/
IFX_LOCAL IFX_int16_t cpt_calc_freq (IFX_uint32_t f)
{
   IFX_int16_t    coef = 0;

   /*
      The formula to calculate the cpt frequency is
         f_coef = 2 * cos ( 2*pi*f[Hz] / 8000) * 2^14
                = 2^15 * cos (pi*f[Hz] / 4000)

      Symmetrie:
         cos (phi) = sin (pi/2 - phi)
         --> we will use the taylor approximation for sin in the
             interval [pi/4 .. 3/4pi]

      with
              X = pi*f/4000

      Approximation with Taylor for cos in [-pi/4 .. +pi/4]
                = 2^15 * ( 1 - (X^2 /2) + (X^4 /24) )
                = 2^15 - (2^15 * X^2 / 2) + (2^15 * X^4 /24)
                = 2^15 - (2^15 * X^2 / 2) + A

                to ensure that f^4 does not overflow, use f/10
                A = 2^15 * X^4 /24
                  = 2^15 * (pi*f /4000)^4 /24
                  = 2^15 * (pi/4000)^4 * f^4 /24
                  = 2^15 * (pi/400)^4 *(f/10)^4 /24

      Approximation with Taylor for sin in [-pi/4 .. +pi/4]
                = 2^15 * ( X - (X^3 /6) + (X^5 /120) )
                = (2^15 * X) - (2^15 * X^3 /6) + (2^15 * X^5 /120)
                = (2^15 * X) - (2^15 * X^3 /6) + B

                to ensure that f^5 does not overflow, use f/20
                B = 2^15 * X^5 /120
                  = 2^15 * (pi*f /4000)^5 / 120
                  = 2^15 * (pi/4000)^5 * f^5 / 120
                  = 2^15 * (pi*20/4000)^5 * (f/20)^5 /120
                  = 2^15 * (pi/200)^5 * (f/20)^5 /120
   */

   if (f <= 1000)
   {
      /* cos approximation using the taylor algorithm */
      /* this formula covers the cos from 0 to pi/4 */
      coef =   (IFX_int16_t)(  (C2_15
                             - ((f*f*1011)/100000))
                             + ((((f/10)*(f/10)*(f/10)*(f/10)) / 192487)
                             - 1)
                             );
   }
   else if (f <= 2000)
   {
      /* sin approximation using the taylor algorithm */
      /* this formula covers the cos from pi/4 to pi/2 */
      f = 2000 - f;
      coef = (IFX_int16_t)( ((25736 * f)/1000
                           - (f*f*f/377948))
                           + ((f/20)*(f/20)*(f/20)*(f/20)*(f/20)/3829411)
                          );
   }
   else if (f <= 3000)
   {
      /* sin approximation using the taylor algorithm */
      /* this formula covers the cos from pi/2 to 3/4pi */
      f = f - 2000;
      coef =
         -(IFX_int16_t) (((((25736 * f) / 1000) -
                           (f * f * f / 377948)) +
                          ((f / 20) * (f / 20) * (f / 20) * (f / 20) *
                           (f / 20) / 3829411)));
   }
   else if (f <= 4000)
   {
      /* cos approximation using the taylor algorithm */
      /* this formula covers the cos from 3/4 pi to pi */
      f = 4000 - f;
      coef = -(IFX_int16_t)(( (C2_15
                             - ((f*f*1011)/100000))
                             + ((f/10)*(f/10)*(f/10)*(f/10) / 192487)
                           ));
   }
   else
   {
      IFXOS_ASSERT(IFX_FALSE);
   }

   return coef;
}

/**
   function to calculate the cpt level coefficient

   \param level - level in [0.1 dB]
   \return
      cpt level coefficient
   \remarks
      CONST_CPT has to be set by define to
      CONST_CPT_HAMMING or CONST_CPT_BLACKMAN
*/
IFX_LOCAL IFX_int16_t cpt_calc_level (IFX_int32_t level)
{
   long lvl;
   IFX_uint32_t tenExp;
   IFX_uint32_t shift;
   IFX_int32_t  exp, i;

   /* calculate the desired level in mdB and the exponent */
   lvl = level * 100;
   exp = (IFX_int32_t)(-lvl-CONST_CPT) / 10;

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

   /* go over all elements in the tens array, starting with the
      largest entry and ... */
   for (i=27;i>=0;i--)
   {
      /* ... loop while the current tens[i][0] is part of the remaining exp */
      while (exp >= ((IFX_int32_t)tens[i][0]))
      {
         /* calculate part of the result for tens[i][0], check which accuracy
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
         exp -= ((IFX_int32_t)tens[i][0]);
      }
   }

   /* calculate the coefficient according to the specification... */
   return (IFX_int16_t) ROUND (((C2_14*shift)/(tenExp)));
}

/**
   Starts CPTD
\param pChannel Handle to TAPI_CHANNEL structure
\param pTone    Handle to the simple tone structure
\param signal   the type of signal and direction to detect
\return Return value according to IFX_return_t
   - IFX_ERROR if an error occured
   - IFX_SUCCESS if successful
\remarks
   none
*/
IFX_return_t IFX_TAPI_LL_SIG_CPTD_Start (IFX_TAPI_LL_CH_t *pLLChannel,
                                IFX_TAPI_TONE_SIMPLE_t const *pTone, IFX_int32_t signal)
{
#if (VIN_CFG_FEATURES & VIN_FEAT_VOICE)
   /*VINETIC_DEVICE  *pDev = (VINETIC_DEVICE *)pChannel->pDevice;*/
   VINETIC_CHANNEL *pCh  = (VINETIC_CHANNEL *) pLLChannel;
   VINETIC_DEVICE *pDev = (VINETIC_DEVICE*) (pCh->pParent);
   IFX_int32_t      err = IFX_SUCCESS;
   IFX_uint8_t      ch = pCh->nChannel - 1;
   IFX_uint16_t     pCmd[CMD_SIG_CPTCOEFF_LEN + CMD_HEADER_CNT];
   DSP_CPTD_FL_t    frameLength;

   if (pDev->nChipType == VINETIC_TYPE_S)
   {
      SET_ERROR (VIN_ERR_NOTSUPPORTED);
      return IFX_ERROR;
   }
   err = Dsp_SigActStatus (pDev, pCh->nChannel - 1, IFX_TRUE, CmdWrite);

   /* set CPTD coefficients ************************************************* */
   if (err == IFX_SUCCESS)
   {
      memset (pCmd, 0, (CMD_SIG_CPTCOEFF_LEN  + CMD_HEADER_CNT) * 2);
      pCmd[0] = CMD1_EOP | ch;
      pCmd[1] = ECMD_CPT_COEF;
      /* nFrameSize is depending on the tone cadence and will be
         decided inside Dsp_Cptd_SetCoeff */
      Dsp_Cptd_SetCoeff (pCh, pTone, pCmd, &frameLength);
      /* write CPTD coefficients */
      err = CmdWrite (pDev, pCmd, CMD_SIG_CPTCOEFF_LEN);
   }

   /* activate CPTD ********************************************************* */
   pCmd[1] = ECMD_CPT;
   if (err == IFX_SUCCESS)
   {
      /* on activation of the CPTD also nFrameSize is configured as
         determined above... */
      err = Dsp_Cptd_Conf (pCh, pTone, signal, pCmd, frameLength);
   }
   if (err == IFX_SUCCESS)
   {
      err = CmdWrite (pDev, pCmd, CMD_SIG_CPT_LEN);
      /* write interrupt masks */
      if (err == IFX_SUCCESS)
      {
         /* enable cpt 0 -> 1 interrupt */
         err = VINETIC_Host_Set_EdspIntMask (pCh, VIN_EDSP_CPT, IFX_FALSE,
                                             VIN_CPT_MASK, 0);
      }
   }

   return err;
#else
   return IFX_ERROR;
#endif /* (VIN_CFG_FEATURES & VIN_FEAT_VOICE) */
}

/**
   Stops CPTD
\param pChannel Handle to TAPI_CHANNEL structure
\return Return value according to IFX_return_t
   - IFX_ERROR if an error occured
   - IFX_SUCCESS if successful
\remarks
   none
*/
IFX_return_t IFX_TAPI_LL_SIG_CPTD_Stop (IFX_TAPI_LL_CH_t *pLLChannel)
{
   /* VINETIC_DEVICE  *pDev = (VINETIC_DEVICE *)pChannel->pDevice;*/
   /* VINETIC_CHANNEL *pCh  = &pDev->pChannel[pChannel->nChannel];*/

   VINETIC_CHANNEL *pCh = (VINETIC_CHANNEL *) pLLChannel;
   VINETIC_DEVICE *pDev = (VINETIC_DEVICE*) (pCh->pParent);
   IFX_int32_t      err  = IFX_SUCCESS;
   IFX_uint8_t      ch = pCh->nChannel - 1;
   IFX_uint16_t     pCmd[3];

   /* disable cpt 0 -> 1 interrupt which was enabled during _CTTD_Start.
      falling edge interrupts are dont care. */
   err = VINETIC_Host_Set_EdspIntMask (pCh, VIN_EDSP_CPT, IFX_FALSE, 0, 0);
   /* write CPTD configuration */
   pCmd[0] = CMD1_EOP | ch;
   pCmd[1] = ECMD_CPT;
   pCmd[2] = 0;
   /* set resource */
   VIN_ECMD_SIGCPT_CPTNR_SET (pCmd[2], ch);
   if (err == IFX_SUCCESS)
      err = CmdWrite (pDev, pCmd, CMD_SIG_CPT_LEN);
   if (err == IFX_SUCCESS)
      err = Dsp_SigActStatus (pDev, pCh->nChannel - 1, IFX_FALSE, CmdWrite);


   return err;
}

/**
   Configures the call progress tone detector
   \param pCh    - pointer to VINETIC channel structure
   \param pTone  - pointer to the complex tone to detect
   \param signal - signal as specified in IFX_TAPI_TONE_CPTD_DIRECTION_t
   \param pCmd   - pointer to the command structure to fill
   \return
      IFX_SUCCESS or IFX_ERROR
*/
IFX_LOCAL IFX_int32_t Dsp_Cptd_Conf (VINETIC_CHANNEL *pCh,
            IFX_TAPI_TONE_SIMPLE_t const *pTone,
            IFX_int32_t signal, IFX_uint16_t *pCmd,
            DSP_CPTD_FL_t frameLength)
{
   VINETIC_DEVICE *pDev = (VINETIC_DEVICE *)pCh->pParent;
   IFX_uint8_t    ch    = (pCh->nChannel - 1);

   /* clean cmd data word */
   pCmd[2] = 0x0000;

   /* set frame length */
   VIN_ECMD_SIGCPT_FL_SET (pCmd[2], frameLength);
   /* set total power */
   VIN_ECMD_SIGCPT_TP_SET (pCmd[2], DSP_CPTD_TP_250_3400_HZ);
   /* select window type */
   VIN_ECMD_SIGCPT_WS_SET (pCmd[2], VIN_ECMD_SIGCPT_WS_HAMMING);
   /* set resource number */
   VIN_ECMD_SIGCPT_CPTNR_SET (pCmd[2], ch);
   /* enable CPTD */
   VIN_ECMD_SIGCPT_EN_SET (pCmd[2], 1);

   /* check for continuos tone */
   if (pTone->cadence[1] == 0)
   {
      VIN_ECMD_SIGCPT_CNT_SET (pCmd[2], 1);
   }
   switch (signal)
   {
      case  IFX_TAPI_TONE_CPTD_DIRECTION_RX:
         /* for receive path input B is used */
         VIN_ECMD_SIGCPT_IS_SET (pCmd[2], VIN_ECMD_SIGCPT_IS_SIGINB);
      break;
      case  IFX_TAPI_TONE_CPTD_DIRECTION_TX:
         VIN_ECMD_SIGCPT_IS_SET (pCmd[2], VIN_ECMD_SIGCPT_IS_SIGINA);
      break;
      default:
         SET_ERROR(VIN_ERR_FUNC_PARM);
         return IFX_ERROR;
   }
   /* remember CPT settings and enable status for resource management */
   pDev->pChannel[ch].pSIG->nCpt = pCmd[2];
   return IFX_SUCCESS;
}

/**
   Configures call progress tone detection coefficients
   \param pCh   - pointer to VINETIC channel structure
   \param pTone - tone definition
   \param pCmd - command to fill
   \return
      IFX_SUCCESS
   \remarks
*/
IFX_LOCAL IFX_int32_t Dsp_Cptd_SetCoeff (VINETIC_CHANNEL *pCh,
                               IFX_TAPI_TONE_SIMPLE_t const *pTone,
                               IFX_uint16_t *pCmd,
                               DSP_CPTD_FL_t *pFrameLength)
{
   IFX_uint8_t nr = 0;
   IFX_uint8_t step = 10, i;
   int         nTenPercentTimeTolerance,
               nMaxTenPercentTimeTolerance = 0,
               nAbsoluteTimeTolerance;

   /* set frame length, in case no step is <= 200ms we use a frame lenth of
      32ms, otherwise 16ms */
   *pFrameLength = DSP_CPTD_FL_32;
   for (i = 0; i < IFX_TAPI_TONE_STEPS_MAX; ++i)
   {
      /* check for last cadence step */
      if (pTone->cadence[i] == 0)
         break;
      /* if one of the steps is shorter or eaqual to 200ms we reduce the
         frame length to 16 ms */
      if (pTone->cadence[i] <= 200)
         *pFrameLength = DSP_CPTD_FL_16;
   }

   /* set nAbsoluteTimeTolerance to roughly 1.2 times the frame length */
   switch (*pFrameLength)
   {
      case DSP_CPTD_FL_16:
         nAbsoluteTimeTolerance =  40;
         break;
      case DSP_CPTD_FL_64:
         nAbsoluteTimeTolerance = 160;
         break;
      case DSP_CPTD_FL_32:
      default:
         nAbsoluteTimeTolerance =  80;
         break;
   }

   /* default settings */
   /* program allowed twist to 6 dB */
   pCmd[10] = 0x2020;
   pCmd[20] = CPTCOEFF_POW_PAUSE;
   pCmd[21] = CPTCOEFF_FP_TP_R_DEFAULT;

   /* set frequency and level A for F_1. Freq A is always set. Tone API
      assures it, the detection level is always below the defined tone. */
   pCmd [2] = (IFX_uint16_t)cpt_calc_freq   (pTone->freqA);
   pCmd [6] = (IFX_uint16_t)cpt_calc_level  (pTone->levelA -
                                             CPTD_DETECTION_LEVEL_TOLERANCE);
   /* set frequency and level B  for F_2 */
   if (pTone->freqB)
   {
      pCmd [3] = (IFX_uint16_t)cpt_calc_freq   (pTone->freqB);
      pCmd [7] = (IFX_uint16_t)cpt_calc_level  (pTone->levelB -
                                                CPTD_DETECTION_LEVEL_TOLERANCE);
   }
   else
   {
      pCmd [3] = VIN_ECMD_SIGCPTCOEFF_FX_0HZ;
      pCmd [7] = VIN_ECMD_SIGCPTCOEFF_LEVEL_0DB;
   }
   /* set frequency and level C for F_3 */
   if (pTone->freqC)
   {
      pCmd [4] = (IFX_uint16_t)cpt_calc_freq   (pTone->freqC);
      pCmd [8] = (IFX_uint16_t)cpt_calc_level  (pTone->levelC -
                                                CPTD_DETECTION_LEVEL_TOLERANCE);
   }
   else
   {
      pCmd [4] = VIN_ECMD_SIGCPTCOEFF_FX_0HZ;
      pCmd [8] = VIN_ECMD_SIGCPTCOEFF_LEVEL_0DB;
   }
   /* set frequency and level D for F_4 */
   if (pTone->freqD)
   {
      pCmd [5] = (IFX_uint16_t)cpt_calc_freq   (pTone->freqD);
      pCmd [9] = (IFX_uint16_t)cpt_calc_level  (pTone->levelD -
                                                CPTD_DETECTION_LEVEL_TOLERANCE);
   }
   else
   {
      pCmd [5] = VIN_ECMD_SIGCPTCOEFF_FX_0HZ;
      pCmd [9] = VIN_ECMD_SIGCPTCOEFF_LEVEL_0DB;
   }
   /* set step times:  T_x */
   for (i = 0; i < IFX_TAPI_TONE_STEPS_MAX; ++i)
   {
      /* check for last cadence step */
      if (pTone->cadence[i] == 0)
         break;
      /* increase step to timing. Step is initialized with 12 (one before
         the first timing setting */
      step++;

      /* to allow +/- 10% deviation in the time, we'll reduce each time
         by 10% and program +TIM_TOL to 2 * MAX ( cadence [i] / 10 ).
         In addition we check if the tolerance for each step as well as
         +TIM_TOL is smaller than nAbsoluteTimeTolerance, if so we use the
         latter one.
       */
      nTenPercentTimeTolerance = pTone->cadence[i] / 10;
      /* limit the time tolerance to 8 bit / 2 */
      if (nTenPercentTimeTolerance > 127)
         nTenPercentTimeTolerance = 127;
      if (nTenPercentTimeTolerance > nMaxTenPercentTimeTolerance)
         nMaxTenPercentTimeTolerance = nTenPercentTimeTolerance;

      pCmd [step] = (IFX_uint16_t)(pTone->cadence[i] -
                        MAX(nAbsoluteTimeTolerance, nTenPercentTimeTolerance) );

      /* move step to step setting */
      step++;
      if ((pTone->frequencies[i] & IFX_TAPI_TONE_FREQALL) == 0)
      {
         pCmd [step] = VIN_ECMD_SIGCPTCOEFF_P;
      }
      else
      {
         /* set mask for MSK_i - use frequency A. Initialize the field */
         if (pTone->frequencies[i] & IFX_TAPI_TONE_FREQA)
            pCmd [step] = VIN_ECMD_SIGCPTCOEFF_F1;
         else
            pCmd [step] = 0;
         /* set mask for MSK_i - use frequency B  */
         if (pTone->frequencies[i] & IFX_TAPI_TONE_FREQB)
            pCmd [step] |= VIN_ECMD_SIGCPTCOEFF_F2;
         /* set mask for MSK_i - use frequency C  */
         if (pTone->frequencies[i] & IFX_TAPI_TONE_FREQC)
            pCmd [step] |= VIN_ECMD_SIGCPTCOEFF_F3;
         /* set mask for MSK_i - use frequency D  */
         if (pTone->frequencies[i] & IFX_TAPI_TONE_FREQD)
            pCmd [step] |= VIN_ECMD_SIGCPTCOEFF_F4;
      }
      nr++;
   }
   if (pTone->cadence[1] == 0 && pTone->pause == 0)
   {
      /* continuos tone, program time for verification */
      pCmd [step-1] = CPTCOEFF_CNT_MINTIME;
      /* set tolerance +TIM_TOL */
      pCmd [19] = (2 * MAX(nAbsoluteTimeTolerance, nMaxTenPercentTimeTolerance)) << 8;
      /* return because nothing more to do */
      return IFX_SUCCESS;
   }
   /* set end of steps */
   pCmd[step] |= VIN_ECMD_SIGCPTCOEFF_E;

   /* set tolerance +TIM_TOL and NR of successfully fulfilled timing
      requirement steps required to pass */
   pCmd[19] = (2 * MAX(nAbsoluteTimeTolerance, nMaxTenPercentTimeTolerance)) << 8 | nr;

   return IFX_SUCCESS;
}

IFX_uint8_t VINETIC_SIG_CPTD_Get_Status (VINETIC_DEVICE *pDev, IFX_uint8_t
                ch)
{
        return VIN_ECMD_SIGCPT_EN_GET (pDev->pChannel[ch].pSIG->nCpt);
}
