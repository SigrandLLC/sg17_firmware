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
   drv_vinetic_utd.c
   \remark
      This file contains the declaration of the functions
      for Universal Tone Detector
*/

#include "ifx_types.h"
#include "drv_vinetic_sig_priv.h"
#include "drv_vinetic_api.h"

/* Local function declarations */

/**
   Helper function to set UTD coefficients.
*/
IFX_LOCAL IFX_int32_t Utd_CheckSignals (VINETIC_CHANNEL *pCh,
                                        IFX_uint32_t nSignal, IFX_uint8_t* nUtd);

/**
   Helper function to set UTD coefficients
*/
IFX_LOCAL IFX_int32_t Utd_SetCoeff        (VINETIC_CHANNEL *pCh,
                                           IFX_uint32_t     freq,
                                           IFX_uint8_t      nUtd);
/* Global function definitions */

/**
   Configuration of the universal tone detector

  \param pCh  - pointer to VINETIC channel structure
  \param nSignal - signal to detect. combination of IFX_TAPI_SIG_CNGFAX,
                  IFX_TAPI_SIG_CNGFAXRX, IFX_TAPI_SIG_CNGFAXTX,
                  IFX_TAPI_SIG_CNGMOD, IFX_TAPI_SIG_CNGMODRX and
                  IFX_TAPI_SIG_CNGMODTX
   \return
      IFX_SUCCESS or IFX_ERROR
   \remarks
      coefficients are fixed to
       - Bandwidth 203.71 Hz
       - Levels -41.56 dB
       - detection time 400 ms
       - universal tone detection always
*/
IFX_int32_t VINETIC_SIG_UtdConf (VINETIC_CHANNEL *pCh, IFX_uint32_t nSignal)
{
   VINETIC_DEVICE *pDev = pCh->pParent;
   IFX_int32_t    err = IFX_SUCCESS;
   IFX_uint8_t    nUtd;
   IFX_uint8_t    ch = (pCh->nChannel - 1);
   IFX_uint16_t   nRising = 0;

   /* check the signals for both UTDs. The number of UTDs to configure
      is returned in nUtd  */
   err = Utd_CheckSignals (pCh, nSignal, &nUtd);
   /* program enable bits and interrupts. input signals are
      already programmed in the while loop */
   switch (nUtd)
   {
      case  0:
         /* Mask UTD interrupts */
         nRising = 0;
         /* no UTD resource used */
         pDev->pChannel[ch].pSIG->fw_sig_utd1.bit.en = 0;
         pDev->pChannel[ch].pSIG->fw_sig_utd2.bit.en = 0;
         break;
      case  1:
         /* unmask UTD1 interrupt 0 -> 1  */
         nRising |= VIN_UTD1_OK_MASK;
         /* first UTD used */
         pDev->pChannel[ch].pSIG->fw_sig_utd1.bit.en = 1;
         pDev->pChannel[ch].pSIG->fw_sig_utd2.bit.en = 0;
         break;
      default:
         /* all UTDs are used : unmask interrupt 0 -> 1 for all utds */
         nRising |= VIN_UTD1_OK_MASK;
         /*  */
         pDev->pChannel[ch].pSIG->fw_sig_utd1.bit.en = 1;
         if (pDev->caps.bUtd2supported)
         {
            pDev->pChannel[ch].pSIG->fw_sig_utd2.bit.en = 1;
            nRising |= VIN_UTD2_OK_MASK;
         }
         break;
   }
   if (err == IFX_SUCCESS)
      err = VINETIC_Host_Set_EdspIntMask (pCh, VIN_EDSP_UTD, IFX_FALSE,
                                          nRising, 0);
   if (err == IFX_SUCCESS)
      err = CmdWrite (pDev, pDev->pChannel[ch].pSIG->fw_sig_utd1.value, CMD_SIG_UTD_LEN);
   if (err == IFX_SUCCESS && (pDev->caps.bUtd2supported))
      err = CmdWrite (pDev, pDev->pChannel[ch].pSIG->fw_sig_utd2.value, CMD_SIG_UTD_LEN);

   return err;
}

/**
   Helper function to set UTD coefficients.

   \param pCh - pointer to the channel structure retrieved from pDev in
                the calling function
   \param nSignal - signal definition from \ref IFX_TAPI_SIG_t related to UTD
   \param nUtd - returns the number of UTD resource that were configured
   \return
      - IFX_ERROR if error
      - IFX_SUCCESS if successful
   \remarks
   This function is used in VINETIC_SIG_UtdConf for CED settings
*/
IFX_LOCAL IFX_int32_t Utd_CheckSignals (VINETIC_CHANNEL *pCh,
                                        IFX_uint32_t nSignal, IFX_uint8_t* nUtd)
{
   VINETIC_DEVICE *pDev = pCh->pParent;
   IFX_uint8_t     ch   = pCh->nChannel - 1,
                    /* signal input source to configure */
                   input;
   IFX_int32_t     err = IFX_SUCCESS;
   /* frequency to program */
   IFX_uint32_t    freq,
                   /* signal to configure for each loop */
                   tmpSig;
                   /* number of UTDs for a channel can only be 1 or 2 */
   IFX_uint8_t     nMaxUtd = VIN_NUM_UTD;

   /* When UTD2 is supported by FW set the number of UTDs to 2 */
   if (pDev->caps.bUtd2supported)
      nMaxUtd = 2;

   *nUtd = 0;
   /* check signal until no further signal to be detect, maybe zero.
      For each run the current configured signal is removed from nSignal */
   while ((nSignal != 0) && err == IFX_SUCCESS && (*nUtd) < nMaxUtd)
   {
      tmpSig = 0;
      freq = 1100;
      /* configure both paths per default */
      input = VIN_ECMD_SIGUTD_IS_SIGINBOTH;
      /* check first signal and mask it from the signal given.
         Note: nSignal could contain more signals (also ATDs) to detect.
         The current signal for the UTD is stored in tmpSig */
      if (nSignal & IFX_TAPI_SIG_CNGFAXMASK)
      {
         /* get the current signal */
         if (nSignal & IFX_TAPI_SIG_CNGFAX)
         {
            tmpSig = nSignal & IFX_TAPI_SIG_CNGFAX;
            /* clear the signal */
            nSignal &= ~IFX_TAPI_SIG_CNGFAX;
         }
         else
         {
            /* detect only one specific path. UTD1 becomes transmit (ALI) */
            if (nSignal & IFX_TAPI_SIG_CNGFAXTX)
            {
               /* the last possibility is TX */
               tmpSig = nSignal & IFX_TAPI_SIG_CNGFAXTX;
               /* clear the signal */
               nSignal &= ~IFX_TAPI_SIG_CNGFAXTX;
               input = VIN_ECMD_SIGUTD_IS_SIGINA;
            }
            else
            {
               tmpSig = nSignal & IFX_TAPI_SIG_CNGFAXRX;
               /* clear the signal */
               nSignal &= ~IFX_TAPI_SIG_CNGFAXRX;
               input = VIN_ECMD_SIGUTD_IS_SIGINB;
            }
         }
      }
      else if (nSignal & IFX_TAPI_SIG_CNGMODMASK)
      {
         /* modem CNG signal detection */
         freq = 1300;
         /* get the current signal */
         if (nSignal & IFX_TAPI_SIG_CNGMOD)
         {
            tmpSig = nSignal & IFX_TAPI_SIG_CNGMOD;
            /* clear the signal */
            nSignal &= ~IFX_TAPI_SIG_CNGMOD;
         }
         else
         {
            /* detect only one specific path */
            if (nSignal & IFX_TAPI_SIG_CNGMODTX)
            {
               /* the last possibility is TX */
               tmpSig = nSignal & IFX_TAPI_SIG_CNGMODTX;
               /* clear the signal */
               nSignal &= ~IFX_TAPI_SIG_CNGMODTX;
               input = VIN_ECMD_SIGUTD_IS_SIGINA;
            }
            else
            {
               tmpSig = nSignal & IFX_TAPI_SIG_CNGMODRX;
               /* clear the signal */
               nSignal &= ~IFX_TAPI_SIG_CNGMODRX;
               input = VIN_ECMD_SIGUTD_IS_SIGINB;
            }
         }
      }
      /* set the input value and program the coefficients */
      if (tmpSig != 0)
      {
         /* enable appropriate paths. And check whether UTD is already active.
            When programming coefficients the UTD must be disabled.
            It will be enabled later on by the caller of this function */
         if ((*nUtd) == 0)
         {
            if (pDev->pChannel[ch].pSIG->fw_sig_utd1.bit.en == 1)
            {
               pDev->pChannel[ch].pSIG->fw_sig_utd1.bit.en = 0;
               err = CmdWrite (pDev, pDev->pChannel[ch].pSIG->fw_sig_utd1.value, CMD_SIG_UTD_LEN);
            }
            pDev->pChannel[ch].pSIG->fw_sig_utd1.bit.is = input;
         }
         else
         {
            if (pDev->pChannel[ch].pSIG->fw_sig_utd2.bit.en == 1)
            {
               pDev->pChannel[ch].pSIG->fw_sig_utd2.bit.en = 0;
               err = CmdWrite (pDev, pDev->pChannel[ch].pSIG->fw_sig_utd2.value, CMD_SIG_UTD_LEN);
            }
            pDev->pChannel[ch].pSIG->fw_sig_utd2.bit.is = input;
         }
         /* set the signal type detection for the corresponding UTD */
         pCh->nUtdSig[*nUtd] = tmpSig;

         if ((err == IFX_SUCCESS) &&
             (Utd_SetCoeff (pCh, freq,
                   (IFX_uint8_t)(ch + pDev->caps.nSIG * (*nUtd))) != IFX_SUCCESS)
         )
         {
             break;
         }
      }
      (*nUtd)++;
   } /* of while */
   return err;
}

/**
   Helper function to set UTD coefficients

   \param pDev - pointer to the device structure
   \param freq - frequency: 1100 or 1300 Hz
   \param nUtd - number of UTD resource: 0 .. 7
   \return
      - IFX_ERROR if error
      - IFX_SUCCESS if successful
   \remarks
   This function is used in VINETIC_SIG_AtdConf for CED settings
*/
IFX_LOCAL IFX_int32_t Utd_SetCoeff (VINETIC_CHANNEL *pCh,
                                    IFX_uint32_t freq, IFX_uint8_t nUtd)
{
   IFX_uint16_t pCmd[CMD_SIG_UTDCOEFF_LEN + CMD_HEADER_CNT];

   pCmd [0] = CMD1_EOP | nUtd;
   pCmd [1] = ECMD_UTD_COEF;
   /* LEVELS = 0x0112 (-41.56 dB) */
   pCmd [2] = 0x0112;

   /* program CF according to freq */
   switch (freq)
   {
      case  1100:
         pCmd [3] = 0x5321;
      break;
      case  1300:
         pCmd [3] = 0x42E1;
      break;
      default:
         {
            /* or calculate freq: f = 32768 * cos(360 * freq / 8000) */
            SET_ERROR(VIN_ERR_FUNC_PARM);
            return IFX_ERROR;
         }
   }
   /* BW = 203.71 Hz */
   pCmd [4] = 0x1300;
   /* DEL and NLEV  */
   pCmd [5] = 0x4104;
   /* LEVELH  = -96 dB */
   pCmd [6] = 0x0000;
   /* AGAP = 0x1 4 ms */
   pCmd [7] = (0x1 << 8) & 0xFF00;
   /* RTIME = 400 ms */
   pCmd [7] |= 0x19;
   /* ABREAK = 0x4 (4 ms) */
   pCmd [8] = (0x4 << 8) & 0xFF00;
   /* RGAP = 0x32 (200 ms) */
   pCmd [8] |= 0x32;

   return CmdWrite (pCh->pParent, pCmd, CMD_SIG_UTDCOEFF_LEN);
}

/**
   Enables signal detection on universal tone detector

   \param pCh  - pointer to VINETIC channel structure
   \param nSignal - signal definition from \ref IFX_TAPI_SIG_t
   \return
      IFX_SUCCESS or IFX_ERROR
   \remarks
      Checks are done if configuration is valid
*/
IFX_int32_t VINETIC_SIG_UtdSigEnable (VINETIC_CHANNEL *pCh, IFX_uint32_t nSignal)
{
   IFX_int32_t  err = IFX_SUCCESS;
   IFX_uint32_t sigMask;

   sigMask = (nSignal | pCh->pSIG->sigMask) &
             (IFX_TAPI_SIG_CNGFAXMASK | IFX_TAPI_SIG_CNGMODMASK);

   err = VINETIC_SIG_UtdConf(pCh, sigMask);
   if (err == IFX_SUCCESS)
   {
      pCh->pSIG->sigMask |= nSignal;
   }

   return err;
}

