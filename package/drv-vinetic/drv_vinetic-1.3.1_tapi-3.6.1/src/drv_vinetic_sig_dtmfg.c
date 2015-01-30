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
   \file drv_vinetic_sig_dtmfg.c
   \remark Implements the DTMF related functionality. */

/* ============================= */
/* Includes                      */
/* ============================= */
#include "ifx_types.h"
#include "drv_vinetic_sig_priv.h"
#include "drv_vinetic_api.h"

/* ============================= */
/* Local Macros & Definitions    */
/* ============================= */

/**
   Store status in channel structure and call according callback function if
   present
\params
   status - DTMF status
*/
#define DTMF_STATUS(status)\
do {\
   pDtmf->state = (status);\
   if (pDtmf->stateCb != IFX_NULL)\
      pDtmf->stateCb(pCh);\
} while (0)

/* max number of DTMF words per message */
#define DTMF_MAX_MSG_WORDS       10

/**
   Represents the minimal allowed for programming the DTMF Receiver Level
*/
#define VINETIC_DTMF_RX_LEVEL_MIN         -96

/**
   Represents the maximal allowed value for programming the DTMF Receiver Level
*/
#define VINETIC_DTMF_RX_LEVEL_MAX         -1

/**
   Represents the minimal value represented by table VINETIC_AlmPcmGain[]
   which can be used to program the DTMF Receiver Gain
*/
#define VINETIC_DTMF_RX_GAIN_MIN          IFX_TAPI_LINE_VOLUME_MIN_GAIN

/**
   Represents the maximum value represented by table VINETIC_AlmPcmGain[]
   which can be used to program the DTMF Receiver Gain
*/
#define VINETIC_DTMF_RX_GAIN_MAX          IFX_TAPI_LINE_VOLUME_MAX_GAIN

/**
   Represents the minimal value represented by table VINETIC_DtmfRxTwist[]
   which can be used to program the DTMF Receiver Twist
*/
#define VINETIC_DTMF_RX_TWIST_MIN         0

/**
   Represents the maximum value represented by table VINETIC_DtmfRxTwist[]
   which can be used to program the DTMF Receiver Gain
*/
#define VINETIC_DTMF_RX_TWIST_MAX         12

/**
   Table with calculated DTMF Receiver(detector) Twist coefficients used by
   the SIG module. It represents values between 0dB and 12dB, in steps of 1dB
   calculated with the formula below.

   TWIST = 256 * 10**(-(TWIST[dB] + 0.5)/10)
   { TWIST[dB] [-0.01, 12], TWIST [E4h, 0Eh] }
*/
const IFX_uint8_t VINETIC_DtmfRxTwist[] =
{
   0xe4, 0xb5, 0x8f, 0x72,       /*  0dB   1dB   2dB   3dB  */
   0x5a, 0x48, 0x39, 0x2d,       /*  4dB   5dB   6dB   7dB  */
   0x24, 0x1c, 0x16, 0x12,       /*  8dB   9dB  10dB  11dB  */
   0x0e                          /* 12dB                    */
};

/**
   If define is present the driver will not make use of the auto-deactivation
   feature of the DTMF/AT generator.
   When DTMF_NO_AUTODEACT is not defned, the AD bit will be set with the last
   portion of DTMF data and the generator will be deactivated by firmware after
   transmission.
   When DTMF_NO_AUTODEACT is defined, the AD bit is not set and the driver
   deactivates the generator on reception of the implirf BUF underrun interrupt.
\remarks
   DTMF_NO_AUTODEACT should be set to guarantee immediate deactivation of the
   DTMF/AT generator on hook-events.
*/
#define DTMF_NO_AUTODEACT

/* ============================= */
/* Local function declaration    */
/* ============================= */

/* ============================= */
/* Function definitions          */
/* ============================= */

/**
   Controls the DTMF sending mode. The DTMF may be sent out-of-band (OOB)
   or in-band.
\param pChannel  Handle to TAPI_CHANNEL structure
\param nOobMode  Mode of the inband and out of band transmission of
                 RFC2883 event packets
          - 0:  IFX_TAPI_PKT_EV_OOB_ALL, DTMF is send in-band and out-ofband
          - 1: IFX_TAPI_PKT_EV_OOB_ONLY , DTMF is send only outof band
          - 2: IFX_TAPI_PKT_EV_OOB_NO, DTMF is send only inband
\return Return value according to IFX_return_t
   - IFX_ERROR if an error occured
   - IFX_SUCCESS if successful
\remarks
   This function sets the AS bit (Auto Suppression) in DTMF Receiver's
   configuration when the DTMF sending only out-of-band is requested. In the
   other case, AS bit is set to zero. In both cases ET bit (Event Transmission
   Support) is set. In-band DTMF sending mode is only valid for G.711 and G.726
   recording codecs. This function works only when the Coder, Signalling and
   ALI modules have been configured.
*/
IFX_int32_t IFX_TAPI_LL_SIG_DTMFD_OOB (IFX_TAPI_LL_CH_t *pLLChannel,
                                        IFX_TAPI_PKT_EV_OOB_t nOobMode)
{
#if (VIN_CFG_FEATURES & VIN_FEAT_VOICE)
   VINETIC_CHANNEL *pCh = (VINETIC_CHANNEL *) pLLChannel;
   VINETIC_DEVICE *pDev = (VINETIC_DEVICE*) (pCh->pParent);
   IFX_uint8_t     ch       = pCh->nChannel - 1;
   IFX_uint16_t    pCmd[3]  = { 0 };
   IFX_int32_t     ret      = IFX_SUCCESS;

   /* Read Signaling Module status */
   if (!(pDev->nDevState & DS_SIG_EN))
   {
      SET_ERROR(VIN_ERR_SIGMOD_NOTEN);
      return IFX_ERROR;
   }

   /* if oob event transmission should be enabled, the EventPT must be != 0 */
   if (nOobMode != IFX_TAPI_PKT_EV_OOB_NO && pCh->nEvtPT == 0)
   {
      SET_ERROR(VIN_ERR_WRONG_EVPT);
      return IFX_ERROR;
   }

   IFXOS_MutexLock (pDev->memberAcc);
   /* store the setting for later use */
   pCmd [2] = pCh->pSIG->fw_dtmf_rec.value;
   pCh->pSIG->et_stat.flag.dtmf_rec = IFX_TRUE;
   pCh->pSIG->fw_dtmf_rec.bit.as = IFX_FALSE;

   switch (nOobMode)
   {
      case IFX_TAPI_PKT_EV_OOB_ALL:
         /* enable transmission */
         break;
      case IFX_TAPI_PKT_EV_OOB_ONLY:
      case IFX_TAPI_PKT_EV_OOB_DEFAULT:
         /* enable transmission and suppress in band signals */
         pCh->pSIG->fw_dtmf_rec.bit.as = IFX_TRUE;
         break;
      case IFX_TAPI_PKT_EV_OOB_NO:
         /* disable transmission and enable in band signals */
         /* store the setting for later use */
         pCh->pSIG->et_stat.flag.dtmf_rec = IFX_FALSE;
         break;
      case IFX_TAPI_PKT_EV_OOB_BLOCK:
         /* don't tansmit the event - neither in- nor outofband */
         pCh->pSIG->et_stat.flag.dtmf_rec = 0;
         pCh->pSIG->fw_dtmf_rec.bit.as = IFX_TRUE;
         break;
      default:
         SET_ERROR(VIN_ERR_FUNC_PARM);
         return IFX_ERROR;
   }

   /* if coder running we apply it directely otherwise it is just stored */
   if (VINETIC_COD_ChStatusGet (pCh))
   {
      pCh->pSIG->fw_dtmf_rec.bit.et = pCh->pSIG->et_stat.flag.dtmf_rec;
   }

   if (pCmd[2] != pCh->pSIG->fw_dtmf_rec.value)
   {
      /* write only when AS or ET has changed */
      pCmd [0] = CMD1_EOP | ch;
      pCmd [1] = ECMD_DTMF_REC;
      pCmd [2] = pCh->pSIG->fw_dtmf_rec.value;
      ret = CmdWrite (pDev, pCmd, 1);
   }

   IFXOS_MutexUnlock (pDev->memberAcc);

   return ret;
#else
   return IFX_ERROR;
#endif /* (VIN_CFG_FEATURES & VIN_FEAT_VOICE) */
}


/**
   Firmware DTMF generator configuration

   \param
      pCh     - handle to VINETIC_CHANNEL structure
   \param
      nTIMT   - DTMF tone on time, TIM=TIM[ms]*2 with 0ms < TIM < 127ms
   \param
      nTIMP   - DTMF tone pause, TIM=TIM[ms]*2 with 0ms < TIM < 127ms
   \return
      IFX_SUCCESS or IFX_ERROR
   \remarks
      Disables DTMF generator during configuration and leave values set to 0xFF
      untouched. See description of DTMF/AT Generator, Coefficients message for
      more details on parameters.
*/
IFX_int32_t VINETIC_SIG_SetDtmfCoeff(VINETIC_CHANNEL *pCh, IFX_uint8_t nTIMT,
                                     IFX_uint8_t nTIMP)
{
   IFX_int32_t       err, enable;
   VINETIC_DEVICE    *pDev    = (VINETIC_DEVICE *)pCh->pParent;
   IFX_uint16_t      pCmd[2]  = {0}, ch = pCh->nChannel - 1;
   IFX_uint16_t      pData[5] = {0};

   /* Disable DTMF/AT generator */
   enable = pDev->pChannel[ch].pSIG->fw_sig_dtmfgen.bit.en;
   pDev->pChannel[ch].pSIG->fw_sig_dtmfgen.bit.en = 0;
   err = CmdWrite (pDev, pDev->pChannel[ch].pSIG->fw_sig_dtmfgen.value,
                   CMD_SIG_DTMFGEN_LEN);
   if (err == IFX_ERROR)
      return IFX_ERROR;

   /* Prepare and send DTMF/AT generator coefficients */
   pCmd [0] = (CMD1_EOP | ch);
   pCmd [1] = ECMD_DTMF_COEF;
   err = CmdRead(pDev, pCmd, pData, 3);

   if (err == IFX_SUCCESS)
   {
      /* Only modify values not set to 0xFF */
      if (nTIMT != 0xFF)
         pData[3] = (pData[3] & 0x00FF) | (nTIMT << 8);
      if (nTIMP != 0xFF)
         pData[3] = (pData[3] & 0xFF00) | nTIMP;
      err = CmdWrite(pDev, pData, 3);
   }
   if (enable)
   {
      pDev->pChannel[ch].pSIG->fw_sig_dtmfgen.bit.en = 1;
      err = CmdWrite (pDev, pDev->pChannel[ch].pSIG->fw_sig_dtmfgen.value,
                      CMD_SIG_DTMFGEN_LEN);
   }
   return err;
}

/**
   Firmware DTMF/AT generator control

   \param
      pCh   - pointer to the channel interface
   \param
      nMode - mode field according to DTMF/AT generator message
   \param
      nMask - only bits set to one will be modified
   \param
      nIsr  - access to VINETIC during normal execution (IFX_FALSE) or interrupt (IFX_TRUE)
   \return
      IFX_SUCCESS/IFX_ERROR
   \remarks
      See description of DTMF/AT Generator message for more details on parameter.
*/
IFX_int32_t VINETIC_SIG_DtmfGen (VINETIC_CHANNEL *pCh, IFX_uint16_t nMode,
                                 IFX_uint16_t nMask, IFX_boolean_t nIsr)
{
   IFX_int32_t    err   = IFX_SUCCESS;
   VINETIC_DEVICE *pDev = pCh->pParent;
   IFX_uint8_t     ch   = pCh->nChannel - 1;

   pDev->pChannel[ch].pSIG->fw_sig_dtmfgen.value[CMD_HEADER_CNT] =
       ((pDev->pChannel[ch].pSIG->fw_sig_dtmfgen.value[CMD_HEADER_CNT] & ~nMask) |
       (nMode & nMask));
   if (nIsr == 0)
   {
      err = CmdWrite (pDev, pDev->pChannel[ch].pSIG->fw_sig_dtmfgen.value,
                      CMD_SIG_DTMFGEN_LEN);
   }
   else
   {
      err = CmdWriteIsr (pDev, pDev->pChannel[ch].pSIG->fw_sig_dtmfgen.value,
                         CMD_SIG_DTMFGEN_LEN);
   }
   return err;
}


/**
   Start DTMF generator
\param
   pCh          - channel pointer
\param
   pDtmfData    - pointer to the DTMF data to send
\param
   nDtmfSize    - number of DTMF words to send
\param
   nFG          - frequency generation mode (0 = low, 1 = high)
\param
   cbDtmfStatus - callback on DTMF status change (set to IFX_NULL if unused)
\param
   bByteMode    - format of pDtmfData (0 = 16bit, 1 = 8bit)
\return
   IFX_SUCCESS/IFX_ERROR
\remarks
   After triggering the DTMF transmission by calling VINETIC_DtmfStart, the
   transmission will be handled automatically. The DTMF data will be sent on
   interrupt request and stopped on end of transmission, error or hook event.
   The callback cbDtmfStatus can be used to track the status of the DTMF
   transmission.
   If bByteMode is set, the driver will convert the IFX_char_t data to DTMF words.
   This Mode only supports restricted DTMF signs 0 to D (no alert tones or pause).
   Only supports DTMF generator high level timing mode.

Note : when ET = 1 : MOD = 0 , FG = 1 (ref FW Spec).
                If done another way, it leads to CERR in FW.
*/
IFX_int32_t VINETIC_SIG_DtmfStart(VINETIC_CHANNEL *pCh, IFX_uint16_t *pDtmfData,
                                  IFX_uint16_t nDtmfWords, IFX_uint32_t nFG,
                                  IFX_void_t (*cbDtmfStatus)(VINETIC_CHANNEL *pCh),
                                  IFX_boolean_t bByteMode)
{
   IFX_int32_t    err = IFX_SUCCESS;
   VINETIC_DEVICE *pDev  = pCh->pParent;
   VINETIC_DTMF   *pDtmf = &pCh->dtmfSend;
   IFX_uint8_t    ch    = pCh->nChannel - 1;
   IFX_uint16_t   dataLength;

   /* Prevent simultaneous usage of generator resource */
   if (++pDtmf->useCnt != 1)
   {
      pDtmf->useCnt--;
      return IFX_ERROR;
   }

   /* Store DTMF data in channel structure for non-blocking sending */
   pDtmf->nSent      = 0;
   pDtmf->nCurr      = 0;
   pDtmf->nWords     = nDtmfWords;
   pDtmf->stateCb    = cbDtmfStatus;
   pDtmf->bByteMode  = bByteMode;
   dataLength        = bByteMode ? nDtmfWords : nDtmfWords << 1;

   IFXOS_FREE(pDtmf->pData);
   if ((pDtmf->pData = IFXOS_MALLOC(dataLength)) != IFX_NULL)
   {
      memcpy (pDtmf->pData, pDtmfData, dataLength);
   }

   err = Dsp_SigActStatus (pDev, ch, IFX_TRUE, CmdWrite);
   /* Enable DTMF generator interrupts
      Note: Allow rising edge interrupts for DTMFG_BUF, DTMFG_REQ, DTMFG_ACK.
            Allow falling edge interrupts for DTMFG_ACK and mask others. */
   err = VINETIC_Host_Set_EdspIntMask (pCh, VIN_EDSP_DTMFG, IFX_FALSE,
                                       VIN_DTMFG_BUF_REQ_ACT_MASK,
                                       VIN_DTMFG_ACK_MASK);

   if (err == IFX_SUCCESS)
   {
      DTMF_STATUS(DTMF_START);
      /* Hardcoded settings, disable, ET = 0, MOD = 1, FG from user,
         A1 = A2 = 01 */
      err = VINETIC_SIG_DtmfGen(pCh, SIG_DTMGGEN_MOD
                        | (SIG_DTMGGEN_FG  & (nFG << SIG_DTMGGEN_FG_OF))
                        | (SIG_DTMGGEN_A1  & (0x01 << SIG_DTMGGEN_A1_OF))
                        | (SIG_DTMGGEN_A2  & (0x01 << SIG_DTMGGEN_A2_OF)),
                           0xFFF0, IFX_FALSE);
   }
   if (err == IFX_SUCCESS)
   {
      pDev->pChannel[ch].pSIG->fw_sig_dtmfgen.bit.en = 1;
      err = CmdWrite(pDev, pCh->pSIG->fw_sig_dtmfgen.value, CMD_SIG_DTMFGEN_LEN);
   }

   return err;
}

/**
   Handle DTMF generator activation/deactivation
\param
   pCh   - channel pointer
\return
   IFX_ERROR/IFX_SUCCESS
\remarks
   Currently only used for detection of succesful deactivation if
   auto-deactivation feature of DTMF/AT generator is used.
   This function does not deactivate the global signaling channel, because
   otherwise the switching function Dsp_SigActStatus must be protected
   against interrupts. Switching of signaling channel is not a must, but
   only a recommendation. In a typical scenario more sig resources would be
   used anyway, that the switching is not mandatory.
*/
IFX_int32_t irq_VINETIC_SIG_DtmfOnActivate(VINETIC_CHANNEL *pCh,
                                           IFX_boolean_t bAct)
{
   IFX_int32_t    err      = IFX_SUCCESS;
#ifndef DTMF_NO_AUTODEACT
   VINETIC_DEVICE *pDev  = pCh->pParent;
   VINETIC_DTMF   *pDtmf = &pCh->dtmfSend;
#endif

   if (bAct == IFX_TRUE)
   {
      /* DTMF/AT generator active */
   }
#ifndef DTMF_NO_AUTODEACT
   else
   {
      /* Detected auto-deactivation of DTMF/AT generator */
      DTMF_STATUS(DTMF_READY);
      IFXOS_FREE(pDtmf->pData);
      pDtmf->pData  = IFX_NULL;
      pDtmf->nWords = 0;
      pDtmf->nSent  = 0;
      pDtmf->nCurr  = 0;
      pDtmf->useCnt--;
      pDev->pChannel[ch].pSIG->fw_sig_dtmfgen.bit.en = 0;
      pDev->pChannel[ch].pSIG->fw_sig_dtmfgen.bit.ad = 0;
   }
#endif /* DTMF_NO_AUTODEACT */

   return err;
}


/**
   Handle DTMF generator underrun
\param
   pCh   - channel pointer
\return
   IFX_ERROR/IFX_SUCCESS
\remarks
   Stop the DTMF/AT generator with immediate effect on underrun. In case
   compiled with DTMF_NO_AUTODEACT a wanted underrun will occur on end of
   transmission.
*/
IFX_int32_t irq_VINETIC_SIG_DtmfOnUnderrun(VINETIC_CHANNEL *pCh)
{
   IFX_int32_t err;

   err = irq_VINETIC_SIG_DtmfStop(pCh, IFX_TRUE);
   return err;
}

/**
   Stop and disable DTMF generator
\remarks
   pCh   - channel pointer
   nIsr - if IFX_TRUE, this function has the interrupt context
\return
   IFX_ERROR/IFX_SUCCESS
\remarks
   Stops the DTMF/AT generator with immediate effect
   This function does not deactivate the global signaling channel, because
   otherwise the switching function Dsp_SigActStatus must be protected
   against interrupts. Switching of signaling channel is not a must, but
   only a recommendation. In a typical scenario more sig resources would be
   used anyway, that the switching is not mandatory.
*/
IFX_int32_t irq_VINETIC_SIG_DtmfStop(VINETIC_CHANNEL *pCh, IFX_int32_t nIsr)
{
   IFX_int32_t          err      = IFX_SUCCESS;
   VINETIC_DTMF_STATE   status   = DTMF_READY;
   IFX_uint8_t          ch       = (pCh->nChannel - 1);
   VINETIC_DEVICE       *pDev    = pCh->pParent;
   VINETIC_DTMF         *pDtmf   = &pCh->dtmfSend;

   /* it could be that a request wasn't generated after the last transfer.
      So sent must be updated again. */
   pDtmf->nSent += pDtmf->nCurr;

   if(pDtmf->nSent == pDtmf->nWords)
   {
      status = DTMF_READY;
   }
   else
   {
      status = DTMF_ABORT;
   }

   /* Free DTMF string */
   IFXOS_FREE(pDtmf->pData);
   pDtmf->pData  = IFX_NULL;
   pDtmf->nWords = 0;
   pDtmf->nSent  = 0;
   pDtmf->nCurr  = 0;
   if ( pDtmf->useCnt > 0 )
      pDtmf->useCnt--;

   /* Disable DTMF generator interrupts and generator itself */
   /* Stop the DTMF generator:
      - reactivate event transmission : ET = 1
      - Timing must be controlled by the events : MOD = 0
      - Frequency generation must be in high level mode : FG = 1
      - Adder A and B Configuration shouldn't be changed !
   */
   err = VINETIC_SIG_DtmfGen(pCh, (SIG_DTMGGEN_ET | SIG_DTMGGEN_FG),
                             0xFF00, IFX_TRUE);

   /* enable the dtmf generator for event transmission : EN = 1
      if the coder is active */
   if (err == IFX_SUCCESS && VINETIC_COD_ChStatusGet(pCh))
   {
      pDev->pChannel[ch].pSIG->fw_sig_dtmfgen.bit.en = 1;
      err = CmdWriteIsr (pDev, pCh->pSIG->fw_sig_dtmfgen.value,
                         CMD_SIG_DTMFGEN_LEN);
   }
   /* Disable generator interrupts
      Note: Disable rising edge interrupts for DTMFG_BUF, DTMFG_REQ, DTMFG_ACK.
            Disable falling edge interrupts for DTMFG_ACK and mask others. */
   if (err == IFX_SUCCESS)
      err = VINETIC_Host_Set_EdspIntMask (pCh, VIN_EDSP_DTMFG, nIsr, 0, 0);
   if (err == IFX_SUCCESS && !nIsr)
   {
      IFXOS_MutexLock (pDev->memberAcc);
      err = Dsp_SigActStatus (pDev, ch, IFX_FALSE, CmdWrite);
      IFXOS_MutexUnlock (pDev->memberAcc);
   }
   DTMF_STATUS(status);

   return err;
}

/**
   Send data to DTMF generator
\param
   pCh   - channel pointer
\return
   IFX_ERROR/IFX_SUCCESS
\remarks
   DTMF data is taken from channel specific structure. Octets are expected to
   contain either DTC of frequency values acc. to DTMF generator mode. See
   description of DTMF/AT Generator, Data message for more details.
   Function to be called from interrupt mode (DTMF/AT generator request) only.
*/
IFX_int32_t irq_VINETIC_SIG_DtmfOnRequest(VINETIC_CHANNEL *pCh)
{
   IFX_uint16_t    i, nSend;
   IFX_int32_t     err       = IFX_SUCCESS;
   IFX_uint16_t    pCmd[12] = {0};
   IFX_uint8_t     ch       = pCh->nChannel - 1;
   VINETIC_DEVICE *pDev     = pCh->pParent;
   VINETIC_DTMF   *pDtmf    = &pCh->dtmfSend;
#ifndef DTMF_NO_AUTODEACT
   IFX_boolean_t   deact    = IFX_FALSE;
#endif /* DTMF_NO_AUTODEACT */

#ifdef DTMF_NO_AUTODEACT
   if ( pDtmf->useCnt < 1 )
   {
      /* We can get one request too much without auto-deactivation... */
      return err;
   }
#endif /* DTMF_NO_AUTODEACT */

   if (pDtmf->state != DTMF_TRANSMIT)
   {
      DTMF_STATUS(DTMF_TRANSMIT);
   }

   /* Update the number of DTMF signs already sent */
   pDtmf->nSent += pDtmf->nCurr;
   pDtmf->nCurr = 0;

   /* exit here if we have no more data to send */
   if (pDtmf->nSent >= pDtmf->nWords)
   {
      return err;
   }

   /* Determine max. number of signs to transmit depending on generator mode */
   if (pCh->pSIG->fw_sig_dtmfgen.value[CMD_HEADER_CNT] & SIG_DTMGGEN_MOD )
   {
      /* High level timing mode, up to 5 signs in low level frequency mode or
         up to 10 in high level frequency mode */
      if ((pDtmf->nWords - pDtmf->nSent) > DTMF_MAX_MSG_WORDS)
      {
         nSend = DTMF_MAX_MSG_WORDS;
      }
      else
      {
         /* Last data portion... */
         nSend = (pDtmf->nWords - pDtmf->nSent);
#ifndef DTMF_NO_AUTODEACT
         deact = IFX_TRUE;
#endif /* DTMF_NO_AUTODEACT */
      }
   }
   else
   {
      /* Low level timing mode, only supports one sign per command */
      nSend = 1;
   }

   /* DTMF/AT generator, data message */
   pCmd [0] = CMD1_EOP | ch;
   pCmd [1] = ECMD_DTMF_DATA | nSend;
   if (pDtmf->bByteMode == IFX_TRUE)
   {
      for (i = 0; i < nSend; i++)
      {
         pCmd[2+i] = (IFX_uint16_t)
                     *((IFX_char_t*)pDtmf->pData + pDtmf->nSent + i);
      }
   }
   else
   {
      memcpy(&pCmd [2], (pDtmf->pData + pDtmf->nSent), nSend*2);
   }

   err = CmdWriteIsr(pDev, pCmd, nSend);

#ifndef DTMF_NO_AUTODEACT
   if ((err == IFX_SUCCESS) && (deact == IFX_TRUE))
   {
      /* Enable DTMF/AT generator auto-deactivation feature */
      err = VINETIC_SIG_DtmfGen(pCh, SIG_DTMGGEN_AD ,
                        SIG_DTMGGEN_EN |SIG_DTMGGEN_AD , IFX_TRUE);
   }
#endif /* DTMF_NO_AUTODEACT */
   if (err == IFX_SUCCESS)
   {
      pDtmf->nCurr = nSend;
   }
   else
   {
      irq_VINETIC_SIG_DtmfStop(pCh, IFX_TRUE);
   }

   return err;
}

/**
   This function translates the given FW specific DTMF code to the corresponding
   TAPI DTMF coding.
\param fwDtmfCode       Digit to encode in FW specific encoding
\return Returns the translated DTMF digit in TAPI specific encoding
*/
IFX_uint8_t irq_VINETIC_SIG_DTMF_encode_fw2tapi (IFX_uint8_t fwDtmfCode)
{
   if (fwDtmfCode <= 0x0F)
   {
      switch (fwDtmfCode)
      {
      case 0x00:
         return 0x0B;
      case 0x0B:
         return 0x0C;
      default:
         if (fwDtmfCode <= 0x0A)
            return fwDtmfCode;
         else if ((fwDtmfCode >= 0x0C) && (fwDtmfCode <= 0x0F))
            return fwDtmfCode + 0x10;
      }
   }
   return 0x00;
}

/**
   This function converts the given FW specific DTMF code to the corresponding
   ASCII character coding.
\param fwDtmfCode       Digit to encode in FW specific encoding
\return Returns the translated DTMF digit in ASCII encoding
\remarks
   For the vinetic firmware, following caracters are coded as follows:
       '*' = 0x2A (ASCII) = 0x0A (VINETIC)
       '#' = 0x23 (ASCII) = 0x0B (VINETIC)
       'A' = 0x41 (ASCII) = 0x0C (VINETIC)
       'B' = 0x42 (ASCII) = 0x0D (VINETIC)
       'C' = 0x43 (ASCII) = 0x0E (VINETIC)
       'D' = 0x44 (ASCII) = 0x0F (VINETIC)
       '0' - '9'          = 0x00 - 0x09
*/
IFX_char_t irq_VINETIC_SIG_DTMF_encode_fw2ascii (IFX_uint8_t fwDtmfCode)
{
   if (fwDtmfCode <= 0x0F)
   {
      switch (fwDtmfCode)
      {
      case 0x0A:
         return '*';
      case 0x0B:
         return '#';
      default:
         if (fwDtmfCode <= 0x09)
            return fwDtmfCode + '0';
         else if ((fwDtmfCode >= 0x0C) && (fwDtmfCode <= 0x0F))
            return (fwDtmfCode - 0x0C) + 'A';
      }
   }
   return '0';
}

/**
   This function converts the given TAPI DTMF code to the device DTMF code
\param nChar       Digit to encode
\param pDtmfCode   Pointer to return DTMF code
\return Return value according to IFX_return_t
   - IFX_ERROR if an error occured
   - IFX_SUCCESS if successful
\remarks
   This function is used by the DTMF CID implementation to store the device
   DTMF codes in the playout buffer.
   For the vinetic firmware, following caracters are coded as follows:
       '*' = 0x2A (ASCII) = 0x0A (VINETIC)
       '#' = 0x23 (ASCII) = 0x0B (VINETIC)
       'A' = 0x41 (ASCII) = 0x0C (VINETIC)
       'B' = 0x42 (ASCII) = 0x0D (VINETIC)
       'C' = 0x43 (ASCII) = 0x0E (VINETIC)
       'D' = 0x44 (ASCII) = 0x0F (VINETIC)
       '0' ...  '9' as is.
*/
IFX_return_t VINETIC_SIG_DTMF_encode_ascii2fw (IFX_char_t nChar, IFX_uint8_t *pDtmfCode)
{
   IFX_int32_t ret;

   if (pDtmfCode == IFX_NULL)
      return IFX_ERROR;

   ret = IFX_SUCCESS;

   switch (nChar)
   {
   case '*':
      *pDtmfCode = 0x0A;
      break;
   case '#':
      *pDtmfCode = 0x0B;
      break;
   case 'A':
      *pDtmfCode = 0x0C;
      break;
   case 'B':
      *pDtmfCode = 0x0D;
      break;
   case 'C':
      *pDtmfCode = 0x0E;
      break;
   case 'D':
      *pDtmfCode = 0x0F;
      break;
   default:
      if ((nChar >= '0') && (nChar <= '9'))
         *pDtmfCode = (IFX_uint8_t)nChar - '0';
      else
         ret = IFX_ERROR;
      break;
   }

   return ret;
}

/**
   Disables or Enables Dtmf receiver according to bEn

   \param pCh  - pointer to VINETIC channel structure
   \param bEn  - IFX_TRUE : enable / IFX_FALSE : disable

   \param dir  - VIN_SIG_TX: local tx
               - VIN_SIG_RX: remote rx

   \return
      IFX_SUCCESS or IFX_ERROR
*/
IFX_int32_t VINETIC_SIG_SetDtmfRec (VINETIC_CHANNEL const *pCh, IFX_boolean_t bEn,
                            IFX_uint8_t dir)
{
   IFX_int32_t ret = IFX_SUCCESS, ch = pCh->nChannel - 1;
   IFX_uint16_t pCmd [3] = {0};
   VINETIC_DEVICE *pDev = pCh->pParent;

   /* EOP Cmd */
   pCmd [0] = (CMD1_EOP | ch);
   /* DTMF REC */
   pCmd [1] = ECMD_DTMF_REC;
   pCmd [2] = pDev->pChannel[ch].pSIG->fw_dtmf_rec.value;

   switch (dir)
   {
   case VIN_SIG_TX:
      pDev->pChannel[ch].pSIG->fw_dtmf_rec.bit.is = 0;
      break;
   case VIN_SIG_RX:
      pDev->pChannel[ch].pSIG->fw_dtmf_rec.bit.is = 1;
      break;
   default:
      break;
   }

   if ((bEn == IFX_TRUE) && !(pDev->pChannel[ch].pSIG->fw_dtmf_rec.bit.en))
   {
      pDev->pChannel[ch].pSIG->fw_dtmf_rec.bit.en = 1;
   }
   else if ((bEn == IFX_FALSE) && (pDev->pChannel[ch].pSIG->fw_dtmf_rec.bit.en))
   {
      pDev->pChannel[ch].pSIG->fw_dtmf_rec.bit.en = 0;
   }
   if (pCmd[2] != pDev->pChannel[ch].pSIG->fw_dtmf_rec.value)
   {
      pCmd [2] = pDev->pChannel[ch].pSIG->fw_dtmf_rec.value;
      ret = CmdWrite (pDev, pCmd, 1);
   }
   return ret;
}

/**
   Get staus information of DTMF generator
   \param pDev  - pointer to VINETIC device structure
   \param ch    - channel number

*/
IFX_int32_t VINETIC_SIG_DTMFG_Get_Status (VINETIC_DEVICE *pDev, IFX_uint8_t ch)
{
   return pDev->pChannel[ch].pSIG->fw_sig_dtmfgen.bit.en;
}

/**
   Enable DTMF generator
   \param pDev  - pointer to VINETIC device structure
   \param ch    - channel number

*/
IFX_int32_t VINETIC_SIG_DTMFG_Enable (VINETIC_DEVICE *pDev, IFX_uint8_t ch)
{
   IFX_int32_t ret;

   pDev->pChannel[ch].pSIG->fw_sig_dtmfgen.bit.en = 1;
   ret = CmdWrite (pDev, pDev->pChannel[ch].pSIG->fw_sig_dtmfgen.value,
                         CMD_SIG_DTMFGEN_LEN);
   return ret;
}

/**
   Disable DTMF generator
   \param pDev  - pointer to VINETIC device structure
   \param ch    - channel number

*/
IFX_int32_t VINETIC_SIG_DTMFG_Disable (VINETIC_DEVICE *pDev, IFX_uint8_t
                ch)
{
   IFX_int32_t ret;

   pDev->pChannel[ch].pSIG->fw_sig_dtmfgen.bit.en = 0;
   ret = CmdWrite (pDev, pDev->pChannel[ch].pSIG->fw_sig_dtmfgen.value,
                         CMD_SIG_DTMFGEN_LEN);
   return ret;
}

/**
   Configure the DTMF tone generator

   \param pLLChannel      - Handle to VINETIC_CHANNEL structure
   \param nInterDigitTime - Inter-digit-time in ms
   \param nDigitPlayTime  - Active digit-play-time in ms

   \return
      IFX_SUCCESS or IFX_ERROR
*/
IFX_int32_t IFX_TAPI_LL_SIG_DTMFG_Cfg (IFX_TAPI_LL_CH_t *pLLChannel,
                                       IFX_uint16_t nInterDigitTime,
                                       IFX_uint16_t nDigitPlayTime)
{
   VINETIC_CHANNEL *pCh  = (VINETIC_CHANNEL *) pLLChannel;

   if (nInterDigitTime > 127 || nDigitPlayTime > 127 )
   {
      TRACE (VINETIC, DBG_LEVEL_HIGH,
             ("DTMF generator timing coefficients too big (max 127ms allowed)\n\r"));
      return IFX_ERROR;
   }

   pCh->pSIG->nDtmfInterDigitTime = nInterDigitTime;
   pCh->pSIG->nDtmfDigitPlayTime  = nDigitPlayTime;

   return IFX_SUCCESS;
}

/**
   Start the DTMF tone generator

   \param pLLChannel      - Handle to VINETIC_CHANNEL structure
   \param nDigits  - Number of digits in the data string to be sent
   \param *data    - String with the digits (ascii 0-9 A-D) to be sent

   \return
      IFX_SUCCESS or IFX_ERROR
*/
IFX_int32_t IFX_TAPI_LL_SIG_DTMFG_Start (IFX_TAPI_LL_CH_t *pLLChannel,
                                         IFX_uint8_t nDigits,
                                         IFX_char_t  *data)
{
   VINETIC_CHANNEL *pCh  = (VINETIC_CHANNEL *) pLLChannel;
   IFX_int32_t ret;
   int i;

   /* prevent starting the generator while it is already running */
   if (pCh->dtmfSend.useCnt != 0)
      return IFX_ERROR;

   /* arguments to this function are in half ms steps so multiply values by 2 */
   ret = VINETIC_SIG_SetDtmfCoeff(pCh, pCh->pSIG->nDtmfDigitPlayTime << 1,
                                       pCh->pSIG->nDtmfInterDigitTime << 1);

   /* transcode Characters A-D, # and * to FW specific setting */
   /* errors may occur if input string contains invalid characters */
   for (i=0; ret == IFX_SUCCESS && i<nDigits; i++)
      ret = VINETIC_SIG_DTMF_encode_ascii2fw(data[i], data+i);

   /* arguments to the DTMF generator is a byte string with ASCII encoded
      digits and special characters (high level frequency generation) */
   if ( ret == IFX_SUCCESS )
      ret = VINETIC_SIG_DtmfStart(pCh, (IFX_uint16_t *)data,
                                       (IFX_uint16_t)nDigits, 1, NULL, IFX_TRUE);

   return ret;
}

/**
   Stop the DTMF tone generator

   \param pLLChannel      - Handle to VINETIC_CHANNEL structure

   \return
      IFX_SUCCESS or IFX_ERROR
*/
IFX_int32_t IFX_TAPI_LL_SIG_DTMFG_Stop (IFX_TAPI_LL_CH_t *pLLChannel)
{
   VINETIC_CHANNEL *pCh  = (VINETIC_CHANNEL *) pLLChannel;

   return irq_VINETIC_SIG_DtmfStop (pCh, IFX_FALSE);
}

/**
   Sets/Gets DTMF Receiver Coefficients

\param pLLChannel    Handle to TAPI low level channel structure
\param bRW           IFX_FALSE to read, IFX_TRUE to write coefficients
\param pDtmfRxCoeff  Pointer to DTMF Rx coefficients data structure,
                     to read from or write to
\return
   IFX_SUCCESS on successful read/write coefficients
   IFX_ERROR on error
\remarks
   Setting of the DTMF coefficients is only allowed while the DTMF receiver
   is disabled. As a result, if setting of the coefficients is attempted while
   the DTMF receiver is enabled, it will be disabled temporarily in order to
   write the coefficients, and reenabled again.
*/
IFX_return_t IFX_TAPI_LL_SIG_DTMF_RX_CFG (IFX_TAPI_LL_CH_t *pLLChannel,
                                         IFX_boolean_t bRW,
                                         IFX_TAPI_DTMF_RX_CFG_t *pCoeff)
{
   IFX_int32_t ret          = IFX_SUCCESS;
   VINETIC_CHANNEL *pCh     = (VINETIC_CHANNEL *) pLLChannel;
   VINETIC_DEVICE  *pDev    = pCh->pParent;
   IFX_uint16_t    pCmd [3] = {0};
   FWM_RES_DTMFREC_COEFF *pDtmfRxCoeff = &pCh->pSIG->fw_dtmfr_coeff;

   if (bRW == IFX_FALSE)
   {
      /* Write DTMF receiver coefficients */

      /* Check the Coefficients first */

      /* Level parameter */
      if (pCoeff->nLevel < VINETIC_DTMF_RX_LEVEL_MIN ||
          pCoeff->nLevel > VINETIC_DTMF_RX_LEVEL_MAX)
      {
         /* parameter is out of supported range */
         TRACE (VINETIC, DBG_LEVEL_HIGH,
                ("\n\rDRV_ERROR: DTMF Receiver Level out of range (%d),"
                 "               (allowed range %d..%d dB)\n\r", pCoeff->nLevel,
                 VINETIC_DTMF_RX_LEVEL_MIN, VINETIC_DTMF_RX_LEVEL_MAX));
         ret = IFX_ERROR;
      }

      /* Twist parameter */
      if (pCoeff->nTwist < VINETIC_DTMF_RX_TWIST_MIN ||
          pCoeff->nTwist > VINETIC_DTMF_RX_TWIST_MAX)
      {
         /* parameter is out of supported range */
         TRACE (VINETIC, DBG_LEVEL_HIGH,
                ("\n\rDRV_ERROR: DTMF Receiver Twist out of range (%d),"
                 "               (allowed range %d..%d dB)\n\r", pCoeff->nTwist,
                 VINETIC_DTMF_RX_TWIST_MIN, VINETIC_DTMF_RX_TWIST_MAX));
         ret = IFX_ERROR;
      }

      /* Gain parameter */
      if (pCoeff->nGain < VINETIC_DTMF_RX_GAIN_MIN ||
          pCoeff->nGain > VINETIC_DTMF_RX_GAIN_MAX)
      {
         /* parameter is out of supported range */
         TRACE (VINETIC, DBG_LEVEL_HIGH,
                ("\n\rDRV_ERROR: DTMF Receiver Gain out of range (%d),"
                 "               (allowed range %d..%d dB)\n\r", pCoeff->nGain,
                 VINETIC_DTMF_RX_GAIN_MIN, VINETIC_DTMF_RX_GAIN_MAX));
         ret = IFX_ERROR;
      }

      /* Check first if DTMF Receiver is enabled */
      if ((ret == IFX_SUCCESS) && (pCh->pSIG->fw_dtmf_rec.bit.en))
      {
         /* DTMF Receiver is enabled, it must be disabled before
            writing the DTMF Receiver Coefficients */
         pCh->pSIG->fw_dtmf_rec.bit.en = 0;

         /* DTMF RECeiver EOP Cmd */
         pCmd [0] = (CMD1_EOP | (pCh->nChannel - 1));
         pCmd [1] = ECMD_DTMF_REC;
         pCmd [2] = pCh->pSIG->fw_dtmf_rec.value;
         ret = CmdWrite (pDev, pCmd, 1);

         /* Set the enable bit again and prepare the command for writing.
            We will write it later. */
         pCh->pSIG->fw_dtmf_rec.bit.en = 1;
         pCmd [2] = pCh->pSIG->fw_dtmf_rec.value;
      }

      if (ret == IFX_SUCCESS)
      {
         /* Convert LEVEL value to 2's complement */
         pDtmfRxCoeff->bit.level = (IFX_uint8_t)pCoeff->nLevel;
         /* Lookup TWIST coefficient value */
         pDtmfRxCoeff->bit.twist =
            VINETIC_DtmfRxTwist[pCoeff->nTwist + abs(VINETIC_DTMF_RX_TWIST_MIN)];
         /* Lookup Gain coefficient value */
         pDtmfRxCoeff->bit.gain  =
            VINETIC_AlmPcmGain[pCoeff->nGain + abs(VINETIC_DTMF_RX_GAIN_MIN)];

         /* Write the DTMF Receiver Coefficients */
         ret = CmdWrite (pDev, (IFX_uint16_t *)pDtmfRxCoeff, 2);
      }

      /* Reenable the DTMF Receiver, if it was initially enabled */
      if ((ret == IFX_SUCCESS) && (pCh->pSIG->fw_dtmf_rec.bit.en))
      {
         ret = CmdWrite (pDev, pCmd, 1);
      }
   }
   else
   {
      /* Read DTMF receiver coefficients */

      IFX_uint32_t i, nTableLen;

      /* convert LEVEL to signed number */
      pCoeff->nLevel = (IFX_int8_t)pDtmfRxCoeff->bit.level;

      /*
         Lookup TWIST dB value
      */
      nTableLen = sizeof(VINETIC_DtmfRxTwist) / sizeof(IFX_uint8_t);
      for (i = 0; i < nTableLen; i++)
      {
         if ((i == 0) &&
             (pDtmfRxCoeff->bit.twist > VINETIC_DtmfRxTwist[0]))
            break;
         if ((pDtmfRxCoeff->bit.twist == VINETIC_DtmfRxTwist[i]) ||
             ((pDtmfRxCoeff->bit.twist < VINETIC_DtmfRxTwist[i]) &&
             (i < nTableLen - 1) &&
             (pDtmfRxCoeff->bit.twist > VINETIC_DtmfRxTwist[i+1])))
            break;
      }
      pCoeff->nTwist = VINETIC_DTMF_RX_TWIST_MIN + i;

      /*
         Lookup GAIN dB value
      */
      nTableLen = sizeof(VINETIC_AlmPcmGain) / sizeof(IFX_uint8_t);
      for (i = 0; i < nTableLen; i++)
      {
         if (pDtmfRxCoeff->bit.gain == VINETIC_AlmPcmGain[i])
         {
            break;
         }
      }
      if ( i >= nTableLen)
      {
         /* we did not find the exact value - return error and max value */
         i = nTableLen;
         ret = IFX_ERROR;
      }
      pCoeff->nGain = VINETIC_DTMF_RX_GAIN_MIN + i;
   }

   return ret;
}
