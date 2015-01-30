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
  Module      : drv_vinetic_sig_mftd.c
  Description : This files implements the SIG - MFTD module
                Function definitions of Modem and Fax Tone Detection module
******************************************************************************/

/* ============================= */
/* Includes                      */
/* ============================= */

#include "ifx_types.h"
#include "drv_vinetic_sig_priv.h"
#include "drv_vinetic_api.h"

/* ============================= */
/* Local Macros  Definitions    */
/* ============================= */

static IFX_uint32_t pMftdMap[] =
{
   /* index 00 : no detection -> tone holding end */
   IFX_TAPI_EVENT_FAXMODEM_HOLDEND,
   /* index 01 : holding level detection - only when nothing else is detected */
   IFX_TAPI_EVENT_FAXMODEM_NONE,
   /* index 02 : V.21 DIS preamble detection */
   IFX_TAPI_EVENT_FAXMODEM_DIS,
   /* index 03 : Voice modem discriminator */
   IFX_TAPI_EVENT_FAXMODEM_VMD,
   /* index 04 : ANS detected */
   IFX_TAPI_EVENT_FAXMODEM_CED,
   /* index 05 : ANS with one phase reversal detected */
#if (IFX_TAPI_SIG_MFTD_PR_THRESHOLD == 1)
    IFX_TAPI_EVENT_FAXMODEM_PR,
#else
   IFX_TAPI_EVENT_FAXMODEM_NONE,
#endif
   /* index 06 : ANS with two phase reversals detected */
#if (IFX_TAPI_SIG_MFTD_PR_THRESHOLD == 2)
    IFX_TAPI_EVENT_FAXMODEM_PR,
#else
   IFX_TAPI_EVENT_FAXMODEM_NONE,
#endif
   /* index 07 : ANS with three phase reversals detected */
#if (IFX_TAPI_SIG_MFTD_PR_THRESHOLD == 3)
    IFX_TAPI_EVENT_FAXMODEM_PR,
#else
   IFX_TAPI_EVENT_FAXMODEM_NONE,
#endif
   /* index 08 : ANSam detected */
   IFX_TAPI_EVENT_FAXMODEM_AM,
   /* index 09 : ANSam with one phase reversal detected */
#if (IFX_TAPI_SIG_MFTD_PR_THRESHOLD == 1)
    IFX_TAPI_EVENT_FAXMODEM_PR,
#else
   IFX_TAPI_EVENT_FAXMODEM_NONE,
#endif
   /* index 10 : ANSam with two phase reversals detected */
#if (IFX_TAPI_SIG_MFTD_PR_THRESHOLD == 2)
    IFX_TAPI_EVENT_FAXMODEM_PR,
#else
   IFX_TAPI_EVENT_FAXMODEM_NONE,
#endif
   /* index 11 : ANSam with three phase reversals detected */
#if (IFX_TAPI_SIG_MFTD_PR_THRESHOLD == 3)
    IFX_TAPI_EVENT_FAXMODEM_PR,
#else
   IFX_TAPI_EVENT_FAXMODEM_NONE,
#endif
   /* index 12 :  980 Hz single tone detected (V.21L mark sequence) */
   IFX_TAPI_EVENT_FAXMODEM_V21L,
   /* index 13 : 1400 Hz single tone detected (V.18A mark sequence) */
   IFX_TAPI_EVENT_FAXMODEM_V18A,
   /* index 14 : 1800 Hz single tone detected (V.27, V.32 carrier signal) */
   IFX_TAPI_EVENT_FAXMODEM_V27,
   /* index 15 : 1300 Hz single tone detected (Modem calling tone) */
   IFX_TAPI_EVENT_FAXMODEM_CNGMOD,
   /* index 16 : 1100 Hz single tone detected (FAX calling tone) */
   IFX_TAPI_EVENT_FAXMODEM_CNGFAX,
   /* index 17 : 2225 Hz single tone detected (Bell answering tone) */
   IFX_TAPI_EVENT_FAXMODEM_BELL,
   /* index 18 : 2250 Hz single tone detected (V.22 unscrambled binary ones */
   IFX_TAPI_EVENT_FAXMODEM_V22,
   /* index 19 : 1650 Hz single tone (V.21H mark sequence) */
   IFX_TAPI_EVENT_FAXMODEM_V21H,
   /* index 20 : reserved */
   IFX_TAPI_EVENT_FAXMODEM_NONE,
   /* index 21 : 2225 Hz single tone or 2250 Hz single tone detected */
   IFX_TAPI_EVENT_FAXMODEM_V22ORBELL,
   /* index 22 : reserved */
   IFX_TAPI_EVENT_FAXMODEM_NONE,
   /* index 23 : reserved */
   IFX_TAPI_EVENT_FAXMODEM_NONE,
   /* index 24 : 600 Hz + 3000 Hz dual tone detected (V.32 AC signal) */
   IFX_TAPI_EVENT_FAXMODEM_V32AC,
   /* index 25 : 1375 Hz + 2002 Hz dual tone detected (V.8bis init segment) */
   IFX_TAPI_EVENT_FAXMODEM_V8BIS,
   /* index 26 : 2130 Hz + 2750 Hz dual tone (Bell Caller ID Type 2 Alert Tone)*/
   IFX_TAPI_EVENT_FAXMODEM_CAS_BELL,
   /* index 27 : reserved */
   IFX_TAPI_EVENT_FAXMODEM_NONE,
   /* index 28 : reserved */
   IFX_TAPI_EVENT_FAXMODEM_NONE,
   /* index 29 : reserved */
   IFX_TAPI_EVENT_FAXMODEM_NONE,
   /* index 30 : reserved */
   IFX_TAPI_EVENT_FAXMODEM_NONE,
   /* index 31 : reserved */
   IFX_TAPI_EVENT_FAXMODEM_NONE,
};

/* ============================= */
/* Global variable definition    */
/* ============================= */

/* ============================= */
/* Global function declaration   */
/* ============================= */

/* ============================= */
/* Local function declaration    */
/* ============================= */

IFX_LOCAL IFX_int32_t Dsp_MFTD_SigSet(VINETIC_CHANNEL *pCh,
                                      IFX_uint32_t nSignal,
                                      IFX_uint32_t nSignalExt);

/* ============================= */
/* Local variable definition     */
/* ============================= */

/* ============================= */
/* Local function definition     */
/* ============================= */

/* ============================= */
/* Global function definition    */
/* ============================= */

/**
   Enables signal detection
\param pChannel Handle to TAPI_CHANNEL structure
\param pSig     Handle to IFX_TAPI_SIG_DETECTION_t structure
\return Return value according to IFX_return_t
   - IFX_ERROR if an error occured
   - IFX_SUCCESS if successful
\remarks
   none
*/
IFX_return_t IFX_TAPI_LL_SIG_MFTD_Enable (IFX_TAPI_LL_CH_t *pLLChannel,
                                   IFX_TAPI_SIG_DETECTION_t const *pSig)
{
#if (VIN_CFG_FEATURES & VIN_FEAT_VOICE)
   VINETIC_CHANNEL *pCh = (VINETIC_CHANNEL *) pLLChannel;
   VINETIC_DEVICE *pDev = (VINETIC_DEVICE*) (pCh->pParent);
   IFX_int32_t      err  = IFX_SUCCESS;

   if (pDev->nChipType == VINETIC_TYPE_S)
   {
      SET_ERROR (VIN_ERR_NOTSUPPORTED);
      return IFX_ERROR;
   }
   IFXOS_MutexLock   (pDev->memberAcc);
   if (pCh->pSIG == IFX_NULL)
   {
      SET_ERROR (VIN_ERR_NO_SIGCH);
      return IFX_ERROR;
   }

   err = Dsp_SigActStatus (pDev, pCh->nChannel - 1, IFX_TRUE, CmdWrite);

   if (err == IFX_SUCCESS)
   {
      if ( pDev->caps.nMFTD == 0 )
      {
         /* This section handles firmware which provides ATD/UTD */
         /* CED detection */
         if (pSig->sig &
             (IFX_TAPI_SIG_CEDMASK | IFX_TAPI_SIG_CEDENDMASK |
              IFX_TAPI_SIG_PHASEREVMASK | IFX_TAPI_SIG_AMMASK |
              IFX_TAPI_SIG_TONEHOLDING_ENDMASK | IFX_TAPI_SIG_DISMASK)
            )
         {
            /*  ATD1 configured for detection of CED */
            err = VINETIC_SIG_AtdSigEnable (pCh, pSig->sig);
         }
         if ((err == IFX_SUCCESS) &&
             (pSig->sig & (IFX_TAPI_SIG_CNGFAXMASK | IFX_TAPI_SIG_CNGMODMASK)))
         {
            /*  UTD1 configured for CNG (1100 Hz) detection */
            /*  UTD2 configured for CNG (1300 Hz) detection  */
            err = VINETIC_SIG_UtdSigEnable (pCh, pSig->sig);
         }
      }
      else
      {
         /* This section handles firmware which provides MFTD */
         err = VINETIC_SIG_MFTD_SigEnable (pCh, pSig->sig, pSig->sig_ext);
      }
   }

   if ((err == IFX_SUCCESS) && (pSig->sig & IFX_TAPI_SIG_CIDENDTX))
   {
      /* enable the Caller-ID transmission event */
      pCh->pSIG->sigMask |= IFX_TAPI_SIG_CIDENDTX;
   }
   if ((err == IFX_SUCCESS) &&
       (pSig->sig & (IFX_TAPI_SIG_DTMFTX | IFX_TAPI_SIG_DTMFRX)))
   {
      if (pSig->sig & IFX_TAPI_SIG_DTMFTX)
      {
         err = VINETIC_SIG_SetDtmfRec (pCh, IFX_TRUE, 1);
         if (err == IFX_SUCCESS)
         {
            /* enable the DTMF receiver */
            pCh->pSIG->sigMask |= IFX_TAPI_SIG_DTMFTX;
         }
      }
      else
      {
         err = VINETIC_SIG_SetDtmfRec (pCh, IFX_TRUE, 2);
         if (err == IFX_SUCCESS)
         {
            /* enable the DTMF receiver */
            pCh->pSIG->sigMask |= IFX_TAPI_SIG_DTMFRX;
         }
      }
   }
   IFXOS_MutexUnlock   (pDev->memberAcc);
   return err;
#else
   return IFX_ERROR;
#endif /* (VIN_CFG_FEATURES & VIN_FEAT_VOICE) */
}


/**
   Disables signal detection
\param pChannel Handle to TAPI_CHANNEL structure
\param pSig     Handle to IFX_TAPI_SIG_DETECTION_t structure
\return Return value according to IFX_return_t
   - IFX_ERROR if an error occured
   - IFX_SUCCESS if successful
\remarks
   none
*/
IFX_return_t IFX_TAPI_LL_SIG_MFTD_Disable (IFX_TAPI_LL_CH_t *pLLChannel,
                                    IFX_TAPI_SIG_DETECTION_t const *pSig)
{
#if (VIN_CFG_FEATURES & VIN_FEAT_VOICE)
   VINETIC_CHANNEL *pCh  = (VINETIC_CHANNEL *) pLLChannel;
   VINETIC_DEVICE *pDev = (VINETIC_DEVICE*) (pCh->pParent);
   IFX_int32_t      err  = IFX_SUCCESS;

   IFXOS_MutexLock   (pDev->memberAcc);

   if ( pDev->caps.nMFTD == 0 )
   {
      /* This section handles firmware which provides ATD/UTD */
      pCh->pSIG->sigMask &= ~(pSig->sig);

      if (pSig->sig &
          (IFX_TAPI_SIG_CEDMASK | IFX_TAPI_SIG_CEDENDMASK |
           IFX_TAPI_SIG_PHASEREVMASK | IFX_TAPI_SIG_AMMASK |
           IFX_TAPI_SIG_TONEHOLDING_ENDMASK | IFX_TAPI_SIG_DISMASK)
         )
      {
         err = VINETIC_SIG_AtdConf (pCh, pCh->pSIG->sigMask);
      }

      if (pSig->sig & (IFX_TAPI_SIG_CNGFAXMASK | IFX_TAPI_SIG_CNGMODMASK))
      {
         /*  UTD1 configured for CNG (1100 Hz) detection */
         /*  UTD2 configured for CNG (1300 Hz) detection  */
         err = VINETIC_SIG_UtdConf (pCh, pCh->pSIG->sigMask &
                           (IFX_TAPI_SIG_CNGFAXMASK | IFX_TAPI_SIG_CNGMODMASK));
      }
   }
   else
   {
      /* This section handles firmware which provides MFTD */

      err = VINETIC_SIG_MFTD_SigDisable (pCh, pSig->sig, pSig->sig_ext);
   }
   if ((err == IFX_SUCCESS) && (pSig->sig & (IFX_TAPI_SIG_DTMFTX | IFX_TAPI_SIG_DTMFRX)))
   {
      err = VINETIC_SIG_SetDtmfRec (pCh, IFX_FALSE, 0);
      if (err == IFX_SUCCESS)
      {
         /* enable the DTMF receiver */
         pCh->pSIG->sigMask &= ~(IFX_TAPI_SIG_DTMFTX | IFX_TAPI_SIG_DTMFRX);
      }
   }
   if (err == IFX_SUCCESS)
      err = Dsp_SigActStatus (pDev, pCh->nChannel - 1, IFX_FALSE, CmdWrite);

   IFXOS_MutexUnlock   (pDev->memberAcc);

   return err;
#else
   return IFX_ERROR;
#endif /* (VIN_CFG_FEATURES & VIN_FEAT_VOICE) */
}

/**
   Enables signal detection on the Modem Fax Tone Detector (MFTD)

   \param pCh - pointer to VINETIC channel structure
   \param nSignal - signal definition
   \param nSignalExt - extended signal definition
   \return
      IFX_SUCCESS or IFX_ERROR
*/
IFX_int32_t VINETIC_SIG_MFTD_SigEnable (VINETIC_CHANNEL *pCh,
                                IFX_uint32_t nSignal, IFX_uint32_t nSignalExt)
{
   IFX_int32_t err = IFX_SUCCESS;
   VIN_IntMask_t   intMask;

   /* disable interrupts to avoid unwanted interrupts */
   memset (&intMask, 0, sizeof (intMask));
   err = VINETIC_Host_Set_EdspIntMask (pCh, VIN_EDSP_MFTD, IFX_FALSE,
                                       intMask.rising, intMask.falling);

   /* enable the tone detectors given and leave the others running */
   if (err == IFX_SUCCESS)
      err = Dsp_MFTD_SigSet (pCh,
                             pCh->pSIG->sigMask | nSignal,
                             pCh->pSIG->sigMaskExt | nSignalExt);


   if (err == IFX_SUCCESS)
   {
      /* enable all interrupts of the MFTD */
      intMask.rising = intMask.falling = (VIN_MFTD1_MASK | VIN_MFTD2_MASK);
      err = VINETIC_Host_Set_EdspIntMask (pCh, VIN_EDSP_MFTD, IFX_FALSE,
                                          intMask.rising, intMask.falling);
   }

   return err;
}

/**
   Disables signal detection on the Modem Fax Tone Detector (MFTD)

   \param pCh - pointer to VINETIC channel structure
   \param nSignal - signal definition
   \param nSignalExt - extended signal definition
   \return
      IFX_SUCCESS or IFX_ERROR
*/
IFX_int32_t VINETIC_SIG_MFTD_SigDisable (VINETIC_CHANNEL *pCh,
                                 IFX_uint32_t nSignal, IFX_uint32_t nSignalExt)
{
   IFX_int32_t err = IFX_SUCCESS;
   VIN_IntMask_t   intMask;

   /* disable interrupts to avoid unwanted interrupts */
   memset (&intMask, 0, sizeof (intMask));
   err = VINETIC_Host_Set_EdspIntMask (pCh, VIN_EDSP_MFTD, IFX_FALSE,
                                       intMask.rising, intMask.falling);

   /* disable the tone detectors given and leave the others running */
   if (err == IFX_SUCCESS)
      err = Dsp_MFTD_SigSet (pCh,
                             pCh->pSIG->sigMask & ~nSignal,
                             pCh->pSIG->sigMaskExt & ~nSignalExt);

   if (err == IFX_SUCCESS)
   {
      /* enable all interrupts of the MFTD */
      intMask.rising = intMask.falling = (VIN_MFTD1_MASK | VIN_MFTD2_MASK);
      err = VINETIC_Host_Set_EdspIntMask (pCh, VIN_EDSP_MFTD, IFX_FALSE,
                                          intMask.rising, intMask.falling);
   }

   return err;
}

/**
   Sets signal detection on the Modem Fax Tone Detector (MFTD)

   \param pCh - pointer to VINETIC channel structure
   \param nSignal - signal definition
   \param nSignalExt - extended signal definition
   \return
      IFX_SUCCESS or IFX_ERROR
   \remarks
      Sets the MFTD to detect exact the tones given in the signal and signalExt
      parameters.
*/
IFX_LOCAL IFX_int32_t Dsp_MFTD_SigSet (VINETIC_CHANNEL *pCh,
                                       IFX_uint32_t nSignal,
                                       IFX_uint32_t nSignalExt)
{
   IFX_uint8_t     ch   = pCh->nChannel - 1;
   VINETIC_DEVICE *pDev = pCh->pParent;
   IFX_int32_t err = IFX_SUCCESS;

   /* reprogram the MFTD only if something has changed */
   if ((nSignal    != pCh->pSIG->sigMask) ||
       (nSignalExt != pCh->pSIG->sigMaskExt))
   {
      /* changing anything but the MH bit requires to stop the MFTD first
         but stop only if the MFTD has previously been enabled */
      if ((pDev->pChannel[ch].pSIG->fw_sig_mftd.bit.en == 1) &&
          (((nSignal    ^ pCh->pSIG->sigMask) &
            ~IFX_TAPI_SIG_TONEHOLDING_ENDMASK )                ||
            (nSignalExt ^ pCh->pSIG->sigMaskExt)  ))
      {
         pDev->pChannel[ch].pSIG->fw_sig_mftd.bit.en = 0;
         err = CmdWrite (pDev, (IFX_uint16_t*)&pDev->pChannel[ch].pSIG->fw_sig_mftd,
                         CMD_SIG_MFTDCTRL_LEN);
         if (err != IFX_SUCCESS)
         {
            return err;
         }
      }

      /* clear the data fields of the cached command first */
      pDev->pChannel[ch].pSIG->fw_sig_mftd.value[CMD_HEADER_CNT+0] &= 0x000F;
      pDev->pChannel[ch].pSIG->fw_sig_mftd.value[CMD_HEADER_CNT+1]  = 0x0000;
      pDev->pChannel[ch].pSIG->fw_sig_mftd.value[CMD_HEADER_CNT+2]  = 0x0000;

      /* Modem holding characteristic is independent of direction */
      if (nSignal & IFX_TAPI_SIG_TONEHOLDING_ENDMASK)
         pDev->pChannel[ch].pSIG->fw_sig_mftd.bit.mh = 1;

      /* Next the single tone detectors are activated as required */

      /* V.21 mark sequence */
      if (nSignalExt & (IFX_TAPI_SIG_EXT_V21L | IFX_TAPI_SIG_EXT_V21LRX))
         pDev->pChannel[ch].pSIG->fw_sig_mftd.bit.single2 |=  SIG_MFTD_SINGLE_V21L;

      if (nSignalExt & (IFX_TAPI_SIG_EXT_V21L | IFX_TAPI_SIG_EXT_V21LTX))
         pDev->pChannel[ch].pSIG->fw_sig_mftd.bit.single1 |=  SIG_MFTD_SINGLE_V21L;

      /* V.18A mark sequence */
      if (nSignalExt & (IFX_TAPI_SIG_EXT_V18A | IFX_TAPI_SIG_EXT_V18ARX))
         pDev->pChannel[ch].pSIG->fw_sig_mftd.bit.single2 |=  SIG_MFTD_SINGLE_V18A;

      if (nSignalExt & (IFX_TAPI_SIG_EXT_V18A | IFX_TAPI_SIG_EXT_V18ATX))
         pDev->pChannel[ch].pSIG->fw_sig_mftd.bit.single1 |=  SIG_MFTD_SINGLE_V18A;

      /* V.27, V.32 carrier */
      if (nSignalExt & (IFX_TAPI_SIG_EXT_V27 | IFX_TAPI_SIG_EXT_V27RX))
         pDev->pChannel[ch].pSIG->fw_sig_mftd.bit.single2 |= SIG_MFTD_SINGLE_V27;

      if (nSignalExt & (IFX_TAPI_SIG_EXT_V27 | IFX_TAPI_SIG_EXT_V27TX))
         pDev->pChannel[ch].pSIG->fw_sig_mftd.bit.single1 |= SIG_MFTD_SINGLE_V27;

      /* CNG Modem Calling Tone */
      if (nSignal & (IFX_TAPI_SIG_CNGMOD | IFX_TAPI_SIG_CNGMODRX))
         pDev->pChannel[ch].pSIG->fw_sig_mftd.bit.single2 |= SIG_MFTD_SINGLE_CNGMOD;

      if (nSignal & (IFX_TAPI_SIG_CNGMOD | IFX_TAPI_SIG_CNGMODTX))
         pDev->pChannel[ch].pSIG->fw_sig_mftd.bit.single1 |= SIG_MFTD_SINGLE_CNGMOD;

      /* CNG Fax Calling Tone */
      if (nSignal & (IFX_TAPI_SIG_CNGFAX | IFX_TAPI_SIG_CNGFAXRX))
         pDev->pChannel[ch].pSIG->fw_sig_mftd.bit.single2 |= SIG_MFTD_SINGLE_CNGFAX;

      if (nSignal & (IFX_TAPI_SIG_CNGFAX | IFX_TAPI_SIG_CNGFAXTX))
         pDev->pChannel[ch].pSIG->fw_sig_mftd.bit.single1 |= SIG_MFTD_SINGLE_CNGFAX;

      /* Bell answering tone */
      if (nSignalExt & (IFX_TAPI_SIG_EXT_BELL | IFX_TAPI_SIG_EXT_BELLRX))
         pDev->pChannel[ch].pSIG->fw_sig_mftd.bit.single2 |= SIG_MFTD_SINGLE_BELL;

      if (nSignalExt & (IFX_TAPI_SIG_EXT_BELL | IFX_TAPI_SIG_EXT_BELLTX))
         pDev->pChannel[ch].pSIG->fw_sig_mftd.bit.single1 |= SIG_MFTD_SINGLE_BELL;

      /* V.22 unsrambled binary ones */
      if (nSignalExt & (IFX_TAPI_SIG_EXT_V22 | IFX_TAPI_SIG_EXT_V22RX))
         pDev->pChannel[ch].pSIG->fw_sig_mftd.bit.single2 |= SIG_MFTD_SINGLE_V22;

      if (nSignalExt & (IFX_TAPI_SIG_EXT_V22 | IFX_TAPI_SIG_EXT_V22TX))
         pDev->pChannel[ch].pSIG->fw_sig_mftd.bit.single1 |= SIG_MFTD_SINGLE_V22;

      /* V.21H mark sequence */
      if (nSignalExt & (IFX_TAPI_SIG_EXT_V21HRX))
         pDev->pChannel[ch].pSIG->fw_sig_mftd.bit.single2 |= SIG_MFTD_SINGLE_V21H;

      if (nSignalExt & (IFX_TAPI_SIG_EXT_V21HTX))
         pDev->pChannel[ch].pSIG->fw_sig_mftd.bit.single1 |= SIG_MFTD_SINGLE_V21H;

      /* Next the dual tone detectors are activated as required */

      /* V.32 AC signal */
      if (nSignalExt & (IFX_TAPI_SIG_EXT_V32AC | IFX_TAPI_SIG_EXT_V32ACRX))
         pDev->pChannel[ch].pSIG->fw_sig_mftd.bit.dual2 |= SIG_MFTD_DUAL_V32AC;

      if (nSignalExt & (IFX_TAPI_SIG_EXT_V32AC | IFX_TAPI_SIG_EXT_V32ACTX))
         pDev->pChannel[ch].pSIG->fw_sig_mftd.bit.dual1 |= SIG_MFTD_DUAL_V32AC;

      /* V8bis signal */
      if (nSignal & (IFX_TAPI_SIG_V8BISRX))
         pDev->pChannel[ch].pSIG->fw_sig_mftd.bit.dual2 |= SIG_MFTD_DUAL_V8bis;

      if (nSignal & (IFX_TAPI_SIG_V8BISTX))
         pDev->pChannel[ch].pSIG->fw_sig_mftd.bit.dual1 |= SIG_MFTD_DUAL_V8bis;

      /* BELL CAS (Caller ID Type 2 Alert Tone) */
      if (nSignalExt & (IFX_TAPI_SIG_EXT_CASBELLRX))
         pDev->pChannel[ch].pSIG->fw_sig_mftd.bit.dual2 |= SIG_MFTD_DUAL_CASBELL;

      if (nSignalExt & (IFX_TAPI_SIG_EXT_CASBELLTX))
         pDev->pChannel[ch].pSIG->fw_sig_mftd.bit.dual1 |= SIG_MFTD_DUAL_CASBELL;

      /* Next the answering tone detectors are activated as required */
      /* ATD can either be disabled, enabled to detect signal and phase
         reversals or enabled to detect signal with phase reversals and AM.
         So when enabled the phase reversal detection is always active.
         Enabeling detection of AM implicitly activates the detector. */

      if (nSignal & (IFX_TAPI_SIG_CED | IFX_TAPI_SIG_CEDRX |
                     IFX_TAPI_SIG_PHASEREV | IFX_TAPI_SIG_PHASEREVRX))
         pDev->pChannel[ch].pSIG->fw_sig_mftd.bit.atd2 = SIG_MFTD_ATD_EN;

      if (nSignal & (IFX_TAPI_SIG_CED | IFX_TAPI_SIG_CEDTX |
                     IFX_TAPI_SIG_PHASEREV | IFX_TAPI_SIG_PHASEREVTX))
         pDev->pChannel[ch].pSIG->fw_sig_mftd.bit.atd1 = SIG_MFTD_ATD_EN;

      if (nSignal & (IFX_TAPI_SIG_AM  | IFX_TAPI_SIG_AMRX))
         pDev->pChannel[ch].pSIG->fw_sig_mftd.bit.atd2 = SIG_MFTD_ATD_AM_EN;

      if (nSignal & (IFX_TAPI_SIG_AM  | IFX_TAPI_SIG_AMTX))
         pDev->pChannel[ch].pSIG->fw_sig_mftd.bit.atd1 = SIG_MFTD_ATD_AM_EN;

      /* Next the DIS tone detector is enabled if required  */
      if (nSignal & (IFX_TAPI_SIG_DIS | IFX_TAPI_SIG_DISRX))
         pDev->pChannel[ch].pSIG->fw_sig_mftd.bit.dis2 = 1;

      if (nSignal & (IFX_TAPI_SIG_DIS | IFX_TAPI_SIG_DISTX))
         pDev->pChannel[ch].pSIG->fw_sig_mftd.bit.dis1 = 1;

      /* Finally select which parts of the MFTD should be enabled  */

      /* Enable ATD event transmission if ATD is enabled */
      if ((pDev->pChannel[ch].pSIG->fw_sig_mftd.bit.atd1 != 0) ||
          (pDev->pChannel[ch].pSIG->fw_sig_mftd.bit.atd2 != 0)   )
      {
         pDev->pChannel[ch].pSIG->fw_sig_mftd.bit.eta     = 1;
      }

      /* Enable DIS event transmission if DIS tone detector is enabled */
      if ((pDev->pChannel[ch].pSIG->fw_sig_mftd.bit.dis1 != 0) ||
          (pDev->pChannel[ch].pSIG->fw_sig_mftd.bit.dis2 != 0)   )
      {
         pDev->pChannel[ch].pSIG->fw_sig_mftd.bit.etd  = 1;
      }

      /* Enable the CNG event transmission if CNG tone detector is enabled */
      if ((pDev->pChannel[ch].pSIG->fw_sig_mftd.bit.single1 & SIG_MFTD_SINGLE_CNGFAX) ||
          (pDev->pChannel[ch].pSIG->fw_sig_mftd.bit.single2 & SIG_MFTD_SINGLE_CNGFAX)   )
      {
         pDev->pChannel[ch].pSIG->fw_sig_mftd.bit.etc     = 1;
      }

      /* Finally enable the entire MFTD if any detectors are enabled */
      if ((pDev->pChannel[ch].pSIG->fw_sig_mftd.bit.mh != 0) ||
          (pDev->pChannel[ch].pSIG->fw_sig_mftd.value[CMD_HEADER_CNT+1] != 0) ||
          (pDev->pChannel[ch].pSIG->fw_sig_mftd.value[CMD_HEADER_CNT+2] != 0)   )
      {
         pDev->pChannel[ch].pSIG->fw_sig_mftd.bit.en   = 1;
      }

      err = CmdWrite (pDev, (IFX_uint16_t*)&pDev->pChannel[ch].pSIG->fw_sig_mftd,
                      CMD_SIG_MFTDCTRL_LEN);
   }

   if (err == IFX_SUCCESS)
   {
      pCh->pSIG->sigMask = nSignal;
      pCh->pSIG->sigMaskExt = nSignalExt;
   }
   return err;
}

/**
   Interrupt handler for MFTD events
\return
   IFX_ERROR on error, otherwise IFX_SUCCESS
\remarks
*/
void irq_VINETIC_SIG_MFTD_Event (VINETIC_CHANNEL *pCh,
                                 IFX_uint8_t nVal,
                                 IFX_boolean_t bRx)
{
   IFX_TAPI_EVENT_t tapiEvent;

   /* Fill event structure. */
   memset(&tapiEvent, 0, sizeof(IFX_TAPI_EVENT_t));
   tapiEvent.data.fax_sig.signal = nVal;

   if (bRx)
   {
      /* MFTD2 listens on the downstream direction - receive path */
      TRACE (VINETIC, DBG_LEVEL_LOW, ("MFTD Rx Event %X\n\r", nVal));
      tapiEvent.data.fax_sig.network = 1;
      /* handle CED end signalling */
      if (((pCh->pSIG->lastMftd2ToneIdx >= 4) &&
           (pCh->pSIG->lastMftd2ToneIdx <= 11)) &&
          ((nVal < 4) || (nVal > 11)))
      {
         tapiEvent.id = IFX_TAPI_EVENT_FAXMODEM_CEDEND;
         IFX_TAPI_Event_Dispatch(pCh->pTapiCh, &tapiEvent);
      }
      /* decode the tone index into signals using a lookup table and
         dispatch signals */
      tapiEvent.id = pMftdMap[nVal];
      /* change "holding end" event into "none" event when holding tone detector
         is not enabled. the MH working independent of the direction so take
         also care of desired direction for reporting */
      if ((tapiEvent.id == IFX_TAPI_EVENT_FAXMODEM_HOLDEND) &&
          !(pCh->pSIG->sigMask & (IFX_TAPI_SIG_TONEHOLDING_END |
                                  IFX_TAPI_SIG_TONEHOLDING_ENDRX)))
      {
         tapiEvent.id = IFX_TAPI_EVENT_FAXMODEM_NONE;
      }
      /* remember the tone index for the next time "CED end" handling */
      pCh->pSIG->lastMftd2ToneIdx = nVal;
   }
   else
   {
      /* MFTD1 listens on the upstream direction - transmit path */
      TRACE (VINETIC, DBG_LEVEL_LOW, ("MFTD Tx Event %X\n\r", nVal));
      tapiEvent.data.fax_sig.local   = 1;
      /* handle CED end signalling */
      if (((pCh->pSIG->lastMftd1ToneIdx >= 4) &&
           (pCh->pSIG->lastMftd1ToneIdx <= 11)) &&
          ((nVal < 4) || (nVal > 11)))
      {
         tapiEvent.id = IFX_TAPI_EVENT_FAXMODEM_CEDEND;
         IFX_TAPI_Event_Dispatch(pCh->pTapiCh, &tapiEvent);
      }
      /* decode the tone index into signals using a lookup table and
         dispatch signals */
      tapiEvent.id = pMftdMap[nVal];
      /* change "holding end" event into "none" event when holding tone detector
         is disabled */
      /* change "holding end" event into "none" event when holding tone detector
         is not enabled. the MH working independent of the direction so take
         also care of desired direction for reporting */
      if ((tapiEvent.id == IFX_TAPI_EVENT_FAXMODEM_HOLDEND) &&
          !(pCh->pSIG->sigMask & (IFX_TAPI_SIG_TONEHOLDING_END |
                                  IFX_TAPI_SIG_TONEHOLDING_ENDTX)))
      {
         tapiEvent.id = IFX_TAPI_EVENT_FAXMODEM_NONE;
      }
      /* remember the tone index for the next time "CED end" handling */
      pCh->pSIG->lastMftd1ToneIdx = nVal;
   }
   IFX_TAPI_Event_Dispatch(pCh->pTapiCh, &tapiEvent);
}

IFX_return_t IFX_TAPI_LL_SIG_MFTD_Signal_Set (IFX_TAPI_LL_CH_t *pLLChannel,
                                    IFX_uint32_t signal)
{
   VINETIC_CHANNEL *pCh = (VINETIC_CHANNEL *)pLLChannel;

   /* fixme: sanity check that is only needed as long as the IOCTL is not
      blocked while the channel is not initialised. */
   if (pCh == NULL || pCh->pSIG == NULL)
   {
      return IFX_SUCCESS;
   }

   if (signal == 0)
   {
      pCh->pSIG->signal = 0;
      return IFX_SUCCESS;
   }

   /* remove unwanted signals */
   signal &= (pCh->pSIG->sigMask);

   if (signal != 0)
   {
      pCh->pSIG->signal |= signal;
      return IFX_SUCCESS;
   }

   return IFX_ERROR;
}


IFX_return_t IFX_TAPI_LL_SIG_MFTD_Signal_Get (IFX_TAPI_LL_CH_t *pLLChannel,
                                    IFX_uint32_t *signal)
{
   VINETIC_CHANNEL *pCh = (VINETIC_CHANNEL *)pLLChannel;

   /* fixme: sanity check that is only needed as long as the IOCTL is not
      blocked while the channel is not initialised. */
   if (pCh == NULL || pCh->pSIG == NULL)
   {
      *signal = 0;
      return IFX_SUCCESS;
   }

   /* remove unwanted signals */
   *signal = (pCh->pSIG->signal);

   return IFX_SUCCESS;
}

IFX_return_t IFX_TAPI_LL_SIG_MFTD_Signal_Ext_Set (IFX_TAPI_LL_CH_t *pLLChannel,
                                    IFX_uint32_t signalExt)
{
   VINETIC_CHANNEL *pCh = (VINETIC_CHANNEL *)pLLChannel;

   /* fixme: sanity check that is only needed as long as the IOCTL is not
      blocked while the channel is not initialised. */
   if (pCh == NULL || pCh->pSIG == NULL)
   {
      return IFX_SUCCESS;
   }

   if (signalExt == 0)
   {
      pCh->pSIG->signalExt = 0;
      return IFX_SUCCESS;
   }

   /* remove unwanted signals */
   signalExt &= (pCh->pSIG->sigMaskExt);

   if (signalExt != 0)
   {
      pCh->pSIG->signalExt |= signalExt;
      return IFX_SUCCESS;
   }

   return IFX_ERROR;
}


IFX_return_t IFX_TAPI_LL_SIG_MFTD_Signal_Ext_Get (IFX_TAPI_LL_CH_t *pLLChannel,
                                    IFX_uint32_t *signalExt)
{
   VINETIC_CHANNEL *pCh = (VINETIC_CHANNEL *)pLLChannel;

   /* fixme: sanity check that is only needed as long as the IOCTL is not
      blocked while the channel is not initialised. */
   if (pCh == NULL || pCh->pSIG == NULL)
   {
      *signalExt = 0;
      return IFX_SUCCESS;
   }

   /* remove unwanted signals */
   *signalExt = (pCh->pSIG->signalExt);

   return IFX_SUCCESS;
}

IFX_return_t IFX_TAPI_LL_SIG_MFTD_Signal_Enable (IFX_TAPI_LL_CH_t *pLLChannel,
                IFX_TAPI_SIG_t signal)
{
   VINETIC_CHANNEL *pCh = (VINETIC_CHANNEL *)pLLChannel;

   pCh->pSIG->sigMask |= signal;

   return IFX_SUCCESS;
}

IFX_return_t IFX_TAPI_LL_SIG_MFTD_Signal_Disable (IFX_TAPI_LL_CH_t *pLLChannel,
                IFX_TAPI_SIG_t signal)
{
   VINETIC_CHANNEL *pCh = (VINETIC_CHANNEL *)pLLChannel;

   pCh->pSIG->sigMask  &= ~signal;

   return IFX_SUCCESS;
}
