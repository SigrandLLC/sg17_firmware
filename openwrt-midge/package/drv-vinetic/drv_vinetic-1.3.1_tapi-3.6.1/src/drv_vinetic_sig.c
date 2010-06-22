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
   Module      : drv_vinetic_sig.c
   Description : This file contains the implementation of the functions
                 for the Signaling module
******************************************************************************/

/* ============================= */
/* Includes                      */
/* ============================= */
#include "ifx_types.h"
#include "drv_vinetic_sig_priv.h"
#include "drv_vinetic_api.h"
#include "drv_vinetic_sig_cid.h"
#include "drv_vinetic_con.h"

/* ============================= */
/* Local Macros & Definitions    */
/* ============================= */
/** Requested gap for tone holding to maximum 7000 ms sec (value / 10)*/
#define TONEHOLDING_RGAP (7000 / 10)
/** Requested gap for CED detection 120 ms sec (value / 10)*/
#define CED_RGAP         (120 / 10)
/** DIS detection Number of Requested Repetition of the Preamble Sequence */
#define DIS_REPET        0x8
/** DIS detection minimum signal level */
#define DIS_LEVEL         0xCF
/** CED detection minimum phase shift for the phase reversal  */
#define CED_PHASE        0x23
/** Toneholding detection minimum signal level */
#define TONEHOLDING_LEVEL 0x83
/** CED detection minimum signal level */
#define CED_LEVEL         0xCF


/* ============================= */
/*  Local function declarations  */
/* ============================= */

#if 0
/**
   Enables or disables the signaling interface modules :
   DTMF gen, DTMF rec, CID, Tone generators
*/
static IFX_int32_t Dsp_SwitchSignModules (VINETIC_CHANNEL *pCh, IFX_boolean_t bOn,
                                          SIG_MOD_STATE* pState);
#endif

/**
   Helper function to set CED and related settings
*/
static IFX_int32_t Atd_ConfCED (VINETIC_CHANNEL *pCh,
                                IFX_uint32_t nSignal, IFX_uint8_t* nAtd,
                                VIN_IntMask_t *pAtdIntMask);

/**
   Helper function to set signal level detection
*/
static IFX_int32_t Atd_ConfToneHolding (VINETIC_CHANNEL *pCh,
                                        IFX_uint32_t nSignal,
                                        IFX_uint8_t* nAtd,
                                        VIN_IntMask_t *pAtdIntMask);

static IFX_int32_t Atd_Activate        (VINETIC_CHANNEL *pCh,
                                        IFX_uint32_t     nSignal,
                                        IFX_uint8_t     *nAtd,
                                        IFX_uint16_t    *atdVal,
                                        VIN_IntMask_t   *pAtdIntMask);
#if 0
/**
   Enable/Disable ATD1, ATD2 and UTD1 according to bEn
*/
static IFX_int32_t Dsp_SetToneDetectors (VINETIC_CHANNEL *pCh,
                                         IFX_boolean_t bEn,
                                         SIG_MOD_STATE* pState);
#endif

static IFX_int32_t Atd_SetCoeff         (VINETIC_DEVICE *pDev,
                                         IFX_uint16_t nRgap,
                                         IFX_uint8_t nPhaseRep,
                                         IFX_uint16_t nLevel,
                                         IFX_uint8_t nAtd);



/* ============================= */
/* Global function definitions   */
/* ============================= */

void VINETIC_SIG_Func_Register (IFX_TAPI_DRV_CTX_SIG_t *pSig)
{
   /* Fill the function pointers of the signaling module ll interface */
   pSig->UTG_Start = IFX_TAPI_LL_SIG_UTG_Start;
   pSig->UTG_Stop = IFX_TAPI_LL_SIG_UTG_Stop;
   pSig->UTG_Level_Set = IFX_TAPI_LL_Tone_Set_Level;
   pSig->UTG_Count_Get = IFX_TAPI_LL_SIG_UTG_Count_Get;
   pSig->UTG_Event_Deactivated = IFX_TAPI_LL_UTG_Event_Deactivated;

   pSig->DTMFG_Cfg = IFX_TAPI_LL_SIG_DTMFG_Cfg;
   pSig->DTMFG_Start = IFX_TAPI_LL_SIG_DTMFG_Start;
   pSig->DTMFG_Stop = IFX_TAPI_LL_SIG_DTMFG_Stop;
   pSig->DTMFD_Start = IFX_NULL;  /* DTMFD is currently always running */
   pSig->DTMFD_OOB = IFX_TAPI_LL_SIG_DTMFD_OOB;
   pSig->DTMF_RxCoeff = IFX_TAPI_LL_SIG_DTMF_RX_CFG;

   pSig->CPTD_Start = IFX_TAPI_LL_SIG_CPTD_Start;
   pSig->CPTD_Stop = IFX_TAPI_LL_SIG_CPTD_Stop;

   pSig->MFTD_Enable = IFX_TAPI_LL_SIG_MFTD_Enable;
   pSig->MFTD_Disable = IFX_TAPI_LL_SIG_MFTD_Disable;
   pSig->MFTD_Signal_Get = IFX_TAPI_LL_SIG_MFTD_Signal_Get;
   pSig->MFTD_Signal_Set = IFX_TAPI_LL_SIG_MFTD_Signal_Set;
   pSig->MFTD_Signal_Ext_Get = IFX_TAPI_LL_SIG_MFTD_Signal_Ext_Get;
   pSig->MFTD_Signal_Ext_Set = IFX_TAPI_LL_SIG_MFTD_Signal_Ext_Set;
   pSig->MFTD_Signal_Enable = IFX_TAPI_LL_SIG_MFTD_Signal_Enable;
   pSig->MFTD_Signal_Disable = IFX_TAPI_LL_SIG_MFTD_Signal_Disable;

#ifdef TAPI_CID
   pSig->CID_TX_Start = IFX_TAPI_LL_SIG_CID_TX_Start;
   pSig->CID_TX_Stop = IFX_TAPI_LL_SIG_CID_TX_Stop;
   pSig->CID_RX_Start = IFX_TAPI_LL_SIG_CID_RX_Start;
   pSig->CID_RX_Stop = IFX_TAPI_LL_SIG_CID_RX_Stop;
#endif /* TAPI_CID */
}

/**
   Configures signal detection on answering tone detector

   \param pCh  - pointer to VINETIC channel structure
   \param nSignal - signal definition
   \return
      IFX_SUCCESS or IFX_ERROR
   \remarks
      no checks are done if configuration is valid
      CED must be set, if any related detection is required. CED end detection
      is separately set.
*/
/** \todo This function does not set the interrupts for ATDx_DT, ATDx_NPR
  and ATDx_AM individually because VINETIC_Host_Set_EdspIntMask does not
  support it. Instead even when ATD1 is activated ATD2 interrupts are
  also activated.
 */
IFX_int32_t VINETIC_SIG_AtdConf (VINETIC_CHANNEL *pCh, IFX_uint32_t nSignal)
{
   /* FIXME: Santosh, not sure about support for ATD in INCA */
   IFX_int32_t     err  = IFX_SUCCESS;
   IFX_uint8_t     ch   = pCh->nChannel - 1;
   VINETIC_DEVICE *pDev = pCh->pParent;
   VIN_IntMask_t   atdMask;
   IFX_uint8_t     nAtd;


   memset (&atdMask, 0, sizeof (atdMask));
   /* disable interrupts to avoid unwanted interrupts */
   err = VINETIC_Host_Set_EdspIntMask (pCh, VIN_EDSP_ATD, IFX_FALSE,
                                       atdMask.rising, atdMask.falling);
   nAtd = 0;
   if (!(nSignal & (IFX_TAPI_SIG_CEDMASK | IFX_TAPI_SIG_CEDENDMASK |
                   IFX_TAPI_SIG_PHASEREVMASK | IFX_TAPI_SIG_AMMASK |
                   IFX_TAPI_SIG_TONEHOLDING_ENDMASK))
   )
   {
      /* no ATD1 use .
         Note : Mask registers bits are by default unset
                via previous memset */
      pDev->pChannel[ch].pSIG->fw_sig_atd1.bit.en = 0;
   }
   else
   {
      /* as a workaround of a CERR here we first disable the ATDs
         before enabling them again */
      pDev->pChannel[ch].pSIG->fw_sig_atd1.bit.en = 0;
      err = CmdWrite (pDev, (IFX_uint16_t*)&pDev->pChannel[ch].pSIG->fw_sig_atd1,
                      CMD_SIG_ATD_LEN); /* disable ATD1 */
      if (err == IFX_SUCCESS)
      {
         pDev->pChannel[ch].pSIG->fw_sig_atd2.bit.en = 0;
         err = CmdWrite (pDev, (IFX_uint16_t*)&pDev->pChannel[ch].pSIG->fw_sig_atd2,
                         CMD_SIG_ATD_LEN); /* disable ATD2 */
      }
   }
   if (((nSignal & IFX_TAPI_SIG_CEDMASK) != 0) ||
       ((nSignal & IFX_TAPI_SIG_CEDENDMASK) != 0))
   {
      /* modify ATDs and interrupt masks according to nSignal */
      err = Atd_ConfCED(pCh, nSignal, &nAtd, &atdMask);
   }
   /* handling for tone holding */
   if ((nSignal & IFX_TAPI_SIG_TONEHOLDING_ENDMASK) != 0)
   {
      if (nAtd == 2)
      {
         SET_ERROR (VIN_ERR_NORESOURCE);
         err = IFX_ERROR;
      }
      else
         err = Atd_ConfToneHolding (pCh,
                                    nSignal & IFX_TAPI_SIG_TONEHOLDING_ENDMASK,
                                    &nAtd, &atdMask);
   }
   /* if both used, set to enable option 3 (means both).
      Atd_ConfToneHolding and Atd_ConfCED return the number of used
      ATDs. Here a switching to bit wise use is done, to make enabling easier */
   if (nAtd == 2)
      nAtd = 3;
   /* currently DIS is only detected on ATD2 */
   if ((nSignal & IFX_TAPI_SIG_DISMASK) != 0)
   {
      if (nAtd == 3)
      {
         SET_ERROR (VIN_ERR_NORESOURCE);
         err = IFX_ERROR;
      }
      else
      {
         /* detect DIS on ATD2 */
         nAtd |= 0x2;
         pDev->pChannel[ch].pSIG->fw_sig_atd2.bit.md = VIN_ECMD_SIGATD_MD_DIS;

         if (nSignal & IFX_TAPI_SIG_DIS)
            pDev->pChannel[ch].pSIG->fw_sig_atd2.bit.is = VIN_ECMD_SIGATD_IS_SIGINBOTH;
         else
         {
            if (nSignal & IFX_TAPI_SIG_DISRX)
               /* for receive path input B is used */
               pDev->pChannel[ch].pSIG->fw_sig_atd2.bit.is = VIN_ECMD_SIGATD_IS_SIGINB;
            if (nSignal & IFX_TAPI_SIG_DISTX)
               /* for transmit path input A is used */
               pDev->pChannel[ch].pSIG->fw_sig_atd2.bit.is = VIN_ECMD_SIGATD_IS_SIGINA;
         }
         Atd_SetCoeff (pDev, CED_RGAP, DIS_REPET, DIS_LEVEL,
                       (IFX_uint8_t)(ch + pDev->caps.nSIG));
         /* unmask setting for interrupt 0 -> 1  */
         atdMask.rising |= VIN_ATD2_DT_MASK;
      }
   }
   else
   {
      /* no DIS and no other on ATD2 */
      pDev->pChannel[ch].pSIG->fw_sig_atd2.bit.en = 0;
      if (nAtd == 1)
      {
         /* disable 1 -> 0 interrupt */
         atdMask.rising &= ~VIN_ATD2_DT_MASK;
      }
   }
   if (nAtd & 0x1)
   {
      /* ATD1 is enabled */
      pDev->pChannel[ch].pSIG->fw_sig_atd1.bit.en = 1;
   }
   if (nAtd & 0x2)
   {
      pDev->pChannel[ch].pSIG->fw_sig_atd2.bit.en = 1;
   }
   /* write ATD configuration */
   if (err == IFX_SUCCESS)
      err = CmdWrite (pDev, pDev->pChannel[ch].pSIG->fw_sig_atd1.value, CMD_SIG_ATD_LEN);
   if (err == IFX_SUCCESS)
      err = CmdWrite (pDev, pDev->pChannel[ch].pSIG->fw_sig_atd2.value, CMD_SIG_ATD_LEN);
   /* write interrupt masks */
   if (err == IFX_SUCCESS)
      err = VINETIC_Host_Set_EdspIntMask (pCh, VIN_EDSP_ATD, IFX_FALSE,
                                          atdMask.rising, atdMask.falling);
   /* Saved SRE2 is cleared upon ATD submodule reconfiguration, i.e. execution
    * of either  IFX_TAPI_SIG_DETECT_ENABLE or IFX_TAPI_SIG_DETECT_DISABLE.
    * It is the responsibility of the developer using the signaling functionality
    * to reconfigure the ATD submodule in order to clear the cached SRE2 value.
    * This is part of a workaround needed, since bits ATD1_AM and ATD2_AM of
    * register SRE2/EdspX_Stat2 cannot be cleared through reading SRE2 register.
    */
#ifdef VIN_2CPE
   pCh->hostCh.regEdspX_Stat2 = 0;
#else
   pCh->hostCh.regSRE2 = 0;
#endif /* VIN_2CPE */

   return err;
}


#if 0
/**
   Enables or disables the signaling interface modules :
   DTMF gen, DTMF rec, CID, Tone generators

\param
   pCh    pointer to VINETIC channel structure
\param
   bOn    Switches on or off.
         - IFX_TRUE : restore the module state
         - IFX_FALSE : switch off and remember the state in pState
\param
   pState stores the current module state for restoring
\return
   IFX_SUCCESS or IFX_ERROR
\remark
   This function is called to disable the signaling modules without
   disabling the signaling channel itself and thus to avoid disturbance of an
   ongoing conference, as the input signals of signaling channel are connected
   in this process.
   If the modules are switched off the state is stored in pState. pState can
   be used to reactivate all modules, which were switched on before.
   If no matter what all modules should be activated pState->value must
   be 0xFFFF.
   The UTG and CID may be not, otherwise an error is generated */
IFX_LOCAL IFX_int32_t Dsp_SwitchSignModules (VINETIC_CHANNEL *pCh, IFX_boolean_t bOn,
                                   SIG_MOD_STATE* pState)
{
   VINETIC_DEVICE *pDev = pCh->pParent;
   IFX_int32_t ret, ch = pCh->nChannel - 1;
   IFX_uint16_t pCmd[3];

#if 1
   if (bOn == IFX_FALSE &&
       (pDev->pChannel[ch].pSIG->fw_sig_utg[0].bit.en == IFX_TRUE))
   {
      /* the UTG or CID sender may not be active while the module
         is switched off */
      return IFX_SUCCESS;
   }
#endif
   if (bOn == IFX_FALSE &&
       (pDev->pChannel[ch].pSIG->cid_sender[2] & SIG_CID_EN))
   {
      /* the UTG or CID sender may not be active while the module
         is switched off */
      SET_ERROR (VIN_ERR_INVALID_SIGSTATE);
      return IFX_ERROR;
   }
   ret = Dsp_SetToneDetectors (pCh, bOn, pState);
   if (ret == IFX_SUCCESS)
   {
      pCmd [0] = (CMD1_EOP | ch);
      pCmd [1] = ECMD_DTMF_REC;
      if (bOn == IFX_TRUE)
      {
         /* switch it on if state flags on */
         if (pDev->pChannel[ch].pSIG->fw_dtmf_rec.bit.en == 0 && pState->flag.dtmf_rec == 1)
         {
            pDev->pChannel[ch].pSIG->fw_dtmf_rec.bit.en = 1;
            pCmd [2] = pDev->pChannel[ch].pSIG->fw_dtmf_rec.value;
            ret = CmdWrite (pDev, pCmd, 1);
         }
      }
      else
      {
         /* store state and switch of */
         pState->flag.dtmf_rec = pDev->pChannel[ch].pSIG->fw_dtmf_rec.bit.en;
         if (pDev->pChannel[ch].pSIG->fw_dtmf_rec.bit.en == 1)
         {
            pDev->pChannel[ch].pSIG->fw_dtmf_rec.bit.en = 0;
            pCmd [2] = pDev->pChannel[ch].pSIG->fw_dtmf_rec.value;
            ret = CmdWrite (pDev, pCmd, 1);
         }
      }
   }
   if (ret == IFX_SUCCESS)
   {
      if (bOn == IFX_TRUE)
      {
         /* switch it on if state flags on */
         if (pDev->pChannel[ch].pSIG->fw_sig_dtmfgen.bit.en == 0 && pState->flag.dtmfgen == 1)
         {
            pDev->pChannel[ch].pSIG->fw_sig_dtmfgen.bit.en = 1;
            ret = CmdWrite (pDev, pDev->pChannel[ch].pSIG->fw_sig_dtmfgen.value,
                            CMD_SIG_DTMFGEN_LEN);
         }
      }
      else
      {
         /* store state and switch of */
         pState->flag.dtmfgen = pDev->pChannel[ch].pSIG->fw_sig_dtmfgen.bit.en;
         if (pDev->pChannel[ch].pSIG->fw_sig_dtmfgen.bit.en == 1)
         {
            pDev->pChannel[ch].pSIG->fw_sig_dtmfgen.bit.en = 0;
            ret = CmdWrite (pDev, pDev->pChannel[ch].pSIG->fw_sig_dtmfgen.value,
                            CMD_SIG_DTMFGEN_LEN);
         }
      }
   }

   return ret;
}
#endif


/**
   Enables or disables the signalling interface modules :
   DTMF gen, DTMF rec, CID, Tone generators
\param  pDev    pointer to VINETIC device structure
\param  ch     channel number
\param  state  activation state of the modules
\return
   IFX_SUCCESS or IFX_ERROR
\remark
   This function is called to disable or enable the event transmission flags
   of the different firmware modules.
   It also programs it to the DSP.
   The DTMF receiver needs special handling cause one more bit is
   modified in TAPI_LL_Send_DTMF_OOB and in TAPI_LL_RTP_Conf.
   The calling function need at least to know, whether the command has been
   sent */
IFX_int32_t VINETIC_SIG_UpdateEventTrans (VINETIC_DEVICE *pDev, IFX_uint8_t ch,
                                          SIG_MOD_STATE *pState, SIG_MOD_STATE* apply)
{
   VINETIC_CHANNEL *pCh = &pDev->pChannel[ch];
   IFX_int32_t err = IFX_SUCCESS;

   /* if pState is a NULL pointer, the current data from the
      channel structure is taken.*/
   if (pState == IFX_NULL)
      pState = &pDev->pChannel[ch].pSIG->et_stat;

#if 0  /* \todo replace by MFTD */
#ifdef FW_ETU
   if (pState->flag.atd1 != pCh.pSIG->fw_sig_atd1.bit.et)
   {
      pCh->pSIG->fw_sig_atd1.bit.et = pState->flag.atd1;
      err = CmdWrite (pDev, pCh->pSIG->fw_sig_atd1.value, CMD_SIG_ATD_LEN);
   }
   if (err == IFX_SUCCESS &&
       pState->flag.atd2 != pCh->pSIG->fw_sig_atd2.bit.et)
   {
      pCh->pSIG->fw_sig_atd2.bit.et = pState->flag.atd2;
      err = CmdWrite (pDev, pCh->pSIG->fw_sig_atd2.value, CMD_SIG_ATD_LEN);
   }
   if (err == IFX_SUCCESS &&
       pState->flag.utd1 != pCh->pSIG->fw_sig_utd1.bit.et)
   {
      pCh->pSIG->fw_sig_utd1.bit.et = pState->flag.utd1;
      err = CmdWrite (pDev, pCh->pSIG->fw_sig_utd1.value, CMD_SIG_UTD_LEN);
   }
#ifdef INCLUDE_UTD2
   if (err == IFX_SUCCESS &&
       pState->flag.utd2 != pCh->pSIG->fw_sig_utd2.bit.et)
   {
      pCh->pSIG->fw_sig_utd2.bit.et = pState->flag.utd2;
      err = CmdWrite (pDev, pCh->pSIG->fw_sig_utd2.value, CMD_SIG_UTD_LEN);
   }
#endif /* INCLUDE_UTD2 */
#endif /* FW_ETU */
#endif /* 0 */

   /* DTMF Receiver / event transmission */
   if (err == IFX_SUCCESS &&
       pState->flag.dtmf_rec != pCh->pSIG->fw_dtmf_rec.bit.et)
   {
      IFX_uint16_t pCmd[3];
      if (apply != IFX_NULL)
         apply->flag.dtmf_rec = IFX_TRUE;

      pCh->pSIG->fw_dtmf_rec.bit.et = pState->flag.dtmf_rec;
      /* set DTMF Receiver read command */
      pCmd[0] = CMD1_EOP | ch;
      pCmd[1] = ECMD_DTMF_REC;
      pCmd[2] = pCh->pSIG->fw_dtmf_rec.value;
      err = CmdWrite (pDev, pCmd, 1);
   }

   return err;
}


/**
   Resets the signalling channel
\param  pDev    pointer to VINETIC device structure
\param  ch     channel number
\param  bOn Target operation status on or maybe off
         - IFX_TRUE it must be switched on. No check if needed is done
         - IFX_FALSE a resource has been deactivated, thus check if signaling
           channel is still needed
\return
   IFX_SUCCESS or IFX_ERROR
\remark
   This function is called to minimize the DSP usage after deactivating resources.
   If no more resource is used in the signaling channel the channel is shut down. */
IFX_int32_t Dsp_SigActStatus (VINETIC_DEVICE *pDev, IFX_uint8_t ch, IFX_boolean_t bOn,
            IFX_int32_t (*pCmdWrite) (VINETIC_DEVICE *pDev, IFX_uint16_t* pCmd, IFX_uint8_t count))
{
   IFX_int32_t ret = IFX_SUCCESS;

   /* FIXME: Santosh, no such data strucutre in INCA */
   if (bOn == IFX_FALSE)
   {
      if (pDev->pChannel[ch].pSIG->fw_dtmf_rec.bit.en == 0 &&
         pDev->pChannel[ch].pSIG->fw_sig_dtmfgen.bit.en == 0 &&
         pDev->pChannel[ch].pSIG->fw_sig_atd1.bit.en == 0 &&
         pDev->pChannel[ch].pSIG->fw_sig_atd2.bit.en == 0 &&
         pDev->pChannel[ch].pSIG->fw_sig_utd1.bit.en == 0 &&
         pDev->pChannel[ch].pSIG->fw_sig_utd2.bit.en == 0 &&
         pDev->pChannel[ch].pSIG->fw_sig_mftd.bit.en == 0 &&
         pDev->pChannel[ch].pSIG->fw_sig_utg[0].bit.en == 0 && pDev->pChannel[ch].pSIG->fw_sig_utg[0].bit.sm == 0 &&
         pDev->pChannel[ch].pSIG->fw_sig_utg[1].bit.en == 0 && pDev->pChannel[ch].pSIG->fw_sig_utg[1].bit.sm == 0 &&
         !(pDev->pChannel[ch].pSIG->cid_sender[2] & SIG_CID_EN) &&
         !(pDev->pChannel[ch].pSIG->nCpt & VIN_ECMD_SIGCPT_EN)
         )
      {
         /* check to be switched off */
         if (pDev->pChannel[ch].pSIG->fw_sig_ch.bit.en == 1)
         {
            /* switch the signaling channel off */
            pDev->pChannel[ch].pSIG->fw_sig_ch.bit.en = 0;
            ret = CmdWrite (pDev, pDev->pChannel[ch].pSIG->fw_sig_ch.value, CMD_SIG_CH_LEN);
         }
      }
      else
      {
         /* leave the state activated, but additional check is done */
         if (pDev->pChannel[ch].pSIG->fw_sig_ch.bit.en == 0)
         {
            SET_DEV_ERROR (VIN_ERR_INVALID_SIGSTATE);
            return IFX_ERROR;
         }
         return IFX_SUCCESS;
      }
   }
   else
   {
      /* switch the signaling channel on */
      if (pDev->pChannel[ch].pSIG->fw_sig_ch.bit.en == 0)
      {
         pDev->pChannel[ch].pSIG->fw_sig_ch.bit.en = 1;
         ret = CmdWrite (pDev, pDev->pChannel[ch].pSIG->fw_sig_ch.value, CMD_SIG_CH_LEN);
      }
   }
   return ret;
}


/**
   Helper function to set CED and related settings

   \param pCh         - pointer to VINETIC channel structure
   \param nSignal     - signal definition
   \param nAtd        - number of ATDs used
   \param pAtdIntMask - Atd mask settings.
                        Will be modified by this function.
   \return
      - IFX_ERROR if error
      - IFX_SUCCESS if successful
   \remarks
   This function is used in VINETIC_SIG_AtdConf for CED settings
*/
IFX_LOCAL IFX_int32_t Atd_ConfCED (VINETIC_CHANNEL *pCh,
                                   IFX_uint32_t nSignal, IFX_uint8_t* nAtd,
                                   VIN_IntMask_t *pAtdIntMask)
{
   /* FIXME: Santosh, not sure about support for ATD in INCA */
   /* signal to configure for each loop */
   IFX_uint32_t     tmpSig;
   IFX_uint16_t     atdVal;
   IFX_uint8_t      ch   = pCh->nChannel - 1;
   VINETIC_DEVICE  *pDev = pCh->pParent;

   /* check signal until no further signal to be detect, maybe zero.
      For each run the current configured signal is removed from nSignal */
   (*nAtd) = 0;
   while ((nSignal & (IFX_TAPI_SIG_CEDMASK | IFX_TAPI_SIG_CEDENDMASK)) != 0)
   {
      tmpSig = 0;
      if ((*nAtd) == 0)
         atdVal = pDev->pChannel[ch].pSIG->fw_sig_atd1.value[CMD_HEADER_CNT];
      else
         atdVal = pDev->pChannel[ch].pSIG->fw_sig_atd2.value[CMD_HEADER_CNT];

      if (nSignal & IFX_TAPI_SIG_CED || nSignal & IFX_TAPI_SIG_CEDEND)
      {
         /* enable both paths */
         VIN_ECMD_SIGATD_IS_SET (atdVal,
                                 VIN_ECMD_SIGATD_IS_SIGINBOTH);
         tmpSig = nSignal & (IFX_TAPI_SIG_CED | IFX_TAPI_SIG_CEDEND |
                             IFX_TAPI_SIG_PHASEREVMASK | IFX_TAPI_SIG_AMMASK);
         /* clear the signal */
         nSignal &= ~(IFX_TAPI_SIG_CED | IFX_TAPI_SIG_CEDEND);
      }
      else
      {
         /* enable only one path */
         if (nSignal & (IFX_TAPI_SIG_CEDENDTX | IFX_TAPI_SIG_CEDTX))
         {
            /* for transmit path input A is used. Selection of ATD needed */
            VIN_ECMD_SIGATD_IS_SET (atdVal, VIN_ECMD_SIGATD_IS_SIGINA);
            tmpSig = nSignal & (IFX_TAPI_SIG_CEDTX | IFX_TAPI_SIG_CEDENDTX |
                                IFX_TAPI_SIG_PHASEREVMASK | IFX_TAPI_SIG_AMMASK);
            /* clear the signal */
            nSignal &= ~(IFX_TAPI_SIG_CEDTX | IFX_TAPI_SIG_CEDENDTX);
         }
         else if (nSignal & (IFX_TAPI_SIG_CEDENDRX | IFX_TAPI_SIG_CEDRX))
         {
            /* for receive path input B is used. Only ATD1 is possible */
            VIN_ECMD_SIGATD_IS_SET (atdVal,
                                    VIN_ECMD_SIGATD_IS_SIGINB);
            tmpSig = nSignal & (IFX_TAPI_SIG_CEDRX | IFX_TAPI_SIG_CEDENDRX |
                             IFX_TAPI_SIG_PHASEREVMASK | IFX_TAPI_SIG_AMMASK);
            /* clear the signal */
            nSignal &= ~(IFX_TAPI_SIG_CEDRX | IFX_TAPI_SIG_CEDENDRX);
         }
      }
      Atd_Activate (pCh, tmpSig, nAtd, &atdVal, pAtdIntMask);

      if ((*nAtd) == 0)
         pDev->pChannel[ch].pSIG->fw_sig_atd1.value[CMD_HEADER_CNT] = atdVal;
      else
         pDev->pChannel[ch].pSIG->fw_sig_atd2.value[CMD_HEADER_CNT] = atdVal;
      /* detection time 210 ms */
      Atd_SetCoeff (pDev, CED_RGAP, CED_PHASE, CED_LEVEL,
                    (IFX_uint8_t)(ch + pDev->caps.nSIG * (*nAtd)));
      (*nAtd)++;
   } /* of while */
   return IFX_SUCCESS;
}

/**
   Helper function to set signal level detection

   \param pCh         - pointer to VINETIC channel structure
   \param nSignal     - signal definition
   \param nAtd        - returns the number of the ATD, that were configured
   \param pAtdIntMask - Atd mask settings.
                        Will be modified by this function.
   \return
      - IFX_ERROR if error
      - IFX_SUCCESS if successful
   \remarks
   This function is used in VINETIC_SIG_AtdConf for signal level, so called
   tone holding detection
*/
IFX_LOCAL IFX_int32_t Atd_ConfToneHolding (VINETIC_CHANNEL *pCh,
                                           IFX_uint32_t nSignal,
                                           IFX_uint8_t* nAtd,
                                           VIN_IntMask_t *pAtdIntMask)
{
   /* FIXME: Santosh, not sure about support for ATD in INCA */
   VINETIC_DEVICE *pDev = pCh->pParent;
   IFX_uint8_t     ch = pCh->nChannel - 1;
   IFX_uint16_t    atdVal;

   /* check signal until no further signal to be detect, maybe zero.
      For each run the current configured signal is removed from nSignal */
   (*nAtd) = 0;
   while (nSignal != 0)
   {
      if ((*nAtd) == 0)
         atdVal = pDev->pChannel[ch].pSIG->fw_sig_atd1.value[CMD_HEADER_CNT];
      else
         atdVal = pDev->pChannel[ch].pSIG->fw_sig_atd2.value[CMD_HEADER_CNT];

      VIN_ECMD_SIGATD_MD_SET (atdVal, VIN_ECMD_SIGATD_MD_TONEHOLDING);
      if ((nSignal & IFX_TAPI_SIG_TONEHOLDING_END) != 0)
      {
         VIN_ECMD_SIGATD_IS_SET (atdVal, VIN_ECMD_SIGATD_IS_SIGINBOTH);
         /* unmask setting for interrupt 1 -> 0 only for ATD1 */
         pAtdIntMask->rising  &= ~VIN_ATD1_DT_MASK;
         pAtdIntMask->falling |= VIN_ATD1_DT_MASK;
         /* clear the signal */
         nSignal &= ~(IFX_TAPI_SIG_TONEHOLDING_END);
      }
      else
      {
         /* disable 1 -> 0 interrupts */
         pAtdIntMask->falling &= ~(VIN_ATD1_DT_MASK | VIN_ATD2_DT_MASK);
         if (nSignal & IFX_TAPI_SIG_TONEHOLDING_ENDRX)
         {
            /* for receive path input B is used */
            VIN_ECMD_SIGATD_IS_SET (atdVal, VIN_ECMD_SIGATD_IS_SIGINB);
            /* enable 1 -> 0 interrupt */
            pAtdIntMask->falling |= VIN_ATD1_DT_MASK;
            /* clear the signal */
            nSignal &= ~(IFX_TAPI_SIG_TONEHOLDING_ENDRX);
         }
         else if (nSignal & IFX_TAPI_SIG_TONEHOLDING_ENDTX)
         {
            /* for transmit path input A is used */
            VIN_ECMD_SIGATD_IS_SET (atdVal, VIN_ECMD_SIGATD_IS_SIGINA);
            if (*nAtd == 0)
            {
               /* enable 1 -> 0 interrupt */
               pAtdIntMask->falling |= VIN_ATD1_DT_MASK;
            }
            else
            {
               /* enable 1 -> 0 interrupt */
               pAtdIntMask->falling |= VIN_ATD2_DT_MASK;
            }
            /* clear the signal */
            nSignal &= ~(IFX_TAPI_SIG_TONEHOLDING_ENDTX);
         }
      }
      if ((*nAtd) == 0)
         pDev->pChannel[ch].pSIG->fw_sig_atd1.value[CMD_HEADER_CNT] = atdVal;
      else
         pDev->pChannel[ch].pSIG->fw_sig_atd2.value[CMD_HEADER_CNT] = atdVal;
      /* detection time 1000 ms */
      Atd_SetCoeff (pDev, TONEHOLDING_RGAP, CED_PHASE, TONEHOLDING_LEVEL,
                    (IFX_uint8_t)(ch + pDev->caps.nSIG * (*nAtd)));

      (*nAtd)++;
   }
   return IFX_SUCCESS;
}

#if 0
/**
   Enable/Disable ATD1, ATD2 and UTD1 according to bEn

  \param pCh  - pointer to VINETIC channel structure
  \param bEn  - IFX_TRUE : enable / IFX_FALSE : disable Tone Detectors
   \return
   IFX_SUCCESS or IFX_ERROR
   \remarks
   If bEn is IFX_FALSE
   -  The function calling this function have to check if nFaxConnect is READY
   -  This function must be called after either an SRE1 [UTD1_OK] or
      SRE2 [ATD1_DT] or SRE2 [ATD2_DT] interrupt occurs.
   If bEn is IFX_TRUE
   -  The function is called by TAPI_LL_FaxT38_DisableDataPump to make sure
      that a next fax connection can take place after disabling the datapump
*/
IFX_LOCAL IFX_int32_t Dsp_SetToneDetectors (VINETIC_CHANNEL *pCh,
                                            IFX_boolean_t bEn,
                                            SIG_MOD_STATE* pState)
{
   /* FIXME: Santosh, not sure about support for ATD in INCA */
   IFX_int32_t   ret = IFX_SUCCESS;
   IFX_uint8_t   ch = pCh->nChannel - 1;
   VINETIC_DEVICE *pDev    = pCh->pParent;

   if (bEn == IFX_FALSE)
   {
      /* remember module state */
      pState->flag.atd1 = pDev->pChannel[ch].pSIG->fw_sig_atd1.bit.en;
      pState->flag.atd2 = pDev->pChannel[ch].pSIG->fw_sig_atd2.bit.en;
      pState->flag.utd1 = pDev->pChannel[ch].pSIG->fw_sig_utd1.bit.en;
      pState->flag.utd2 = pDev->pChannel[ch].pSIG->fw_sig_utd2.bit.en;
      if (pDev->pChannel[ch].pSIG->fw_sig_atd1.bit.en == 1)
      {
         pDev->pChannel[ch].pSIG->fw_sig_atd1.bit.en = 0;
         ret = CmdWrite (pDev, pDev->pChannel[ch].pSIG->fw_sig_atd1.value, CMD_SIG_ATD_LEN);
      }
      if (ret == IFX_SUCCESS && pDev->pChannel[ch].pSIG->fw_sig_atd2.bit.en == 1)
      {
         pDev->pChannel[ch].pSIG->fw_sig_atd2.bit.en = 0;
         ret = CmdWrite (pDev, pDev->pChannel[ch].pSIG->fw_sig_atd2.value, CMD_SIG_ATD_LEN);
      }
      if (ret == IFX_SUCCESS && pDev->pChannel[ch].pSIG->fw_sig_utd1.bit.en == 1)
      {
         pDev->pChannel[ch].pSIG->fw_sig_utd1.bit.en = 0;
         ret = CmdWrite (pDev, pDev->pChannel[ch].pSIG->fw_sig_utd1.value, CMD_SIG_UTD_LEN);
      }
      if (ret == IFX_SUCCESS && pDev->pChannel[ch].pSIG->fw_sig_utd2.bit.en == 1 &&
          (pDev->caps.bUtd2supported) )
      {
         pDev->pChannel[ch].pSIG->fw_sig_utd2.bit.en = 0;
         ret = CmdWrite (pDev, pDev->pChannel[ch].pSIG->fw_sig_utd2.value, CMD_SIG_UTD_LEN);
      }
   }
   else
   {
      if (pState->flag.atd1 == 1 && pDev->pChannel[ch].pSIG->fw_sig_atd1.bit.en == 0)
      {
         pDev->pChannel[ch].pSIG->fw_sig_atd1.bit.en = 1;
         ret = CmdWrite (pDev, pDev->pChannel[ch].pSIG->fw_sig_atd1.value, CMD_SIG_ATD_LEN);
      }
      if (ret == IFX_SUCCESS && pState->flag.atd1 == 1 &&
          pDev->pChannel[ch].pSIG->fw_sig_atd2.bit.en == 0)
      {
         pDev->pChannel[ch].pSIG->fw_sig_atd2.bit.en = 1;
         ret = CmdWrite (pDev, pDev->pChannel[ch].pSIG->fw_sig_atd2.value, CMD_SIG_ATD_LEN);
      }
      if (ret == IFX_SUCCESS && pState->flag.atd1 == 1 &&
          pDev->pChannel[ch].pSIG->fw_sig_utd1.bit.en == 0)
      {
         pDev->pChannel[ch].pSIG->fw_sig_utd1.bit.en = 1;
         ret = CmdWrite (pDev, pDev->pChannel[ch].pSIG->fw_sig_utd1.value, CMD_SIG_UTD_LEN);
      }
      if (ret == IFX_SUCCESS && pState->flag.atd1 == 1 &&
          pDev->pChannel[ch].pSIG->fw_sig_utd2.bit.en == 0 &&
          (pDev->caps.bUtd2supported))
      {
         pDev->pChannel[ch].pSIG->fw_sig_utd2.bit.en = 1;
         ret = CmdWrite (pDev, pDev->pChannel[ch].pSIG->fw_sig_utd2.value, CMD_SIG_UTD_LEN);
      }
   }
   return ret;
}
#endif

/**
   Helper function to set ATD coefficients

   \param pDev - pointer to the device structure
   \param nRgap - value for RGAP
   \param nPhaseRep value for
           - CED detection minimum phase shift for the phase reversal
           - DIS detection Number of Requested Repetition of the Preamble
             Sequence
   \param nLevel - value for minimum signal level for CED, DIS and TONEHOLDING
   \param nAtd - number of ATD resource: 0 .. 7 to configure
   \return
      - IFX_ERROR if error
      - IFX_SUCCESS if successful
   \remarks
   This function is used in VINETIC_SIG_AtdConf for CED, DIS and TONEHOLDING settings
*/
IFX_LOCAL IFX_int32_t Atd_SetCoeff (VINETIC_DEVICE *pDev,
                                    IFX_uint16_t nRgap, IFX_uint8_t nPhaseRep,
                                    IFX_uint16_t nLevel, IFX_uint8_t nAtd)
{
   /* FIXME: Santosh, not sure about support for ATD in INCA */
   IFX_uint16_t pCmd[CMD_SIG_ATDCOEFF_LEN + CMD_HEADER_CNT];

   pCmd [0] = CMD1_EOP | nAtd;
   pCmd [1] = ECMD_ATD_COEF;

   /* programming default */
   pCmd [2] = 0x0123;
   pCmd [3] = 0x2E00 | nPhaseRep;
   pCmd [4] = 0x4026;
   pCmd [5] = nLevel;
   pCmd [6] = 0x010A;
   pCmd [7] = 0x0100 | nRgap;

   return CmdWrite (pDev, pCmd, CMD_SIG_ATDCOEFF_LEN);
}

/**
   Helper function to activate ATDs

   \param pCh         - pointer to VINETIC channel structure
   \param nSignal     - signal definition
   \param nAtd        - returns the number of ATDs used
   \param atdVal      - reference value of the ATD configuration from the
                        firmware structure given by the calling function.
                        Will be modified by this function.
   \param pAtdIntMask - Atd mask settings.
                        Will be modified by this function.
   \return
      - IFX_ERROR if error
      - IFX_SUCCESS if successful
   \remarks
   This function is used in VINETIC_SIG_AtdConf for CED settings
*/
IFX_LOCAL IFX_int32_t Atd_Activate (VINETIC_CHANNEL *pCh,
                                    IFX_uint32_t nSignal, IFX_uint8_t* nAtd,
                                    IFX_uint16_t *atdVal,
                                    VIN_IntMask_t *pAtdIntMask)
{

   if (nSignal & IFX_TAPI_SIG_CEDMASK)
   {
      if (nSignal & IFX_TAPI_SIG_CEDENDMASK)
      {
         /* unmask setting for interrupt both edges */
         if ((*nAtd) == 0)
         {
               pAtdIntMask->rising  |= VIN_ATD1_DT_MASK;
               pAtdIntMask->falling |= VIN_ATD1_DT_MASK;
         }
         else
         {
            pAtdIntMask->rising  |= VIN_ATD2_DT_MASK;
            pAtdIntMask->falling |= VIN_ATD2_DT_MASK;
         }
      }
      else
      {
         /* mask setting for interrupt 1 -> 0 cause no end should
            be reported */
         if ((*nAtd) == 0)
         {
            pAtdIntMask->rising  |= VIN_ATD1_DT_MASK;
            pAtdIntMask->falling &= ~VIN_ATD1_DT_MASK;
         }
         else
         {
            pAtdIntMask->rising  |= VIN_ATD2_DT_MASK;
            pAtdIntMask->falling &= ~VIN_ATD2_DT_MASK;
         }
      }

      /* detection of phase reversals is always done
         For AM and phase reversal: no specific path can be set */
      if (nSignal & IFX_TAPI_SIG_AM)
      {
         /* enable amplitude modulation */
         VIN_ECMD_SIGATD_MD_SET ((*atdVal), VIN_ECMD_SIGATD_MD_ANS_PHREV_AM);
         if ((*nAtd) == 0)
         {
            pAtdIntMask->rising  |= VIN_ATD1_AM_MASK;
         }
         else
         {
            pAtdIntMask->rising  |= VIN_ATD2_AM_MASK;
         }
      }
      else
      {
         VIN_ECMD_SIGATD_MD_SET ((*atdVal), VIN_ECMD_SIGATD_MD_ANS_PHREV);
         if ((*nAtd) == 0)
         {
            pAtdIntMask->rising  &= ~VIN_ATD1_AM_MASK;
         }
         else
         {
            pAtdIntMask->rising  &= ~VIN_ATD2_AM_MASK;
         }
      }
      if (nSignal & IFX_TAPI_SIG_PHASEREV)
      {
         /* enable phase reversal interrupt */
         /* Interrupt will be generated on the first and third occurring phase reversal */
         if ((*nAtd) == 0)
         {
            pAtdIntMask->rising  |= VIN_ATD1_NPR_1_REV_MASK;
         }
         else
         {
            pAtdIntMask->rising  |= VIN_ATD2_NPR_1_REV_MASK;
         }
      }
      else
      {
         /* disable phase reversal interrupt */
         if ((*nAtd) == 0)
         {
            pAtdIntMask->rising  &= ~VIN_ATD1_NPR_MASK;
         }
         else
         {
            pAtdIntMask->rising  &= ~VIN_ATD2_NPR_MASK;
         }
      }
   }
   else
   {
      /* enable CED end only detection */
      VIN_ECMD_SIGATD_MD_SET ((*atdVal), VIN_ECMD_SIGATD_MD_ANS_PHREV);
      /* unmask setting for interrupt 1 -> 0  */
      if ((*nAtd) == 0)
      {
         pAtdIntMask->rising  &= ~VIN_ATD1_DT_MASK;
         pAtdIntMask->falling |= VIN_ATD1_DT_MASK;
      }
      else
      {
         pAtdIntMask->rising  &= ~VIN_ATD2_DT_MASK;
         pAtdIntMask->falling |= VIN_ATD2_DT_MASK;
      }
   }

   return IFX_SUCCESS;
}

/**
   Enables signal detection on answering tone detector

   \param pCh  - pointer to VINETIC channel structure
   \param nSignal - signal definition
   \return
      IFX_SUCCESS or IFX_ERROR
   \remarks
      Checks are done if configuration is valid
*/
IFX_int32_t VINETIC_SIG_AtdSigEnable (VINETIC_CHANNEL *pCh, IFX_uint32_t nSignal)
{
   IFX_int32_t err = IFX_SUCCESS;
   IFX_uint32_t sigMask;

   sigMask = pCh->pSIG->sigMask;

   /* set temporary mask for sanity checks */
   sigMask |= nSignal;
   /* check if CED receive and other transmit should be set */
   if ( (sigMask & (IFX_TAPI_SIG_CEDRX | IFX_TAPI_SIG_AMRX |
                    IFX_TAPI_SIG_PHASEREVRX | IFX_TAPI_SIG_CEDENDRX)) &&
        (sigMask & (IFX_TAPI_SIG_CEDTX | IFX_TAPI_SIG_AMTX |
                    IFX_TAPI_SIG_PHASEREVTX | IFX_TAPI_SIG_CEDENDTX)) )
   {
      SET_ERROR (VIN_ERR_NOTSUPPORTED);
      return IFX_ERROR;
   }

   /* check if tone holding or CED is to be enabled */
   if ((sigMask & IFX_TAPI_SIG_TONEHOLDING_ENDMASK) &&
       (sigMask & IFX_TAPI_SIG_CEDMASK))
   {
      /* resource is in use */
      SET_ERROR (VIN_ERR_NORESOURCE);
      return IFX_ERROR;
   }
   /* check ATD2 configuration **********************************************/

   err = VINETIC_SIG_AtdConf(pCh, sigMask);
   if (err == IFX_SUCCESS)
   {
      pCh->pSIG->sigMask |= nSignal;
   }
   return err;
}


#if (VIN_CFG_FEATURES & (VIN_FEAT_VIN_C | VIN_FEAT_VIN_M | VIN_FEAT_VIN_2CPE))

IFX_uint16_t VINETIC_SIG_Event_Tx_Status_Get (VINETIC_DEVICE *pDev,
                                              IFX_uint8_t ch)
{
   return pDev->pChannel[ch].pSIG->et_stat.value;
}


/**
   configure rt(c)p for a new connection
\param pChannel        Handle to TAPI_CONNECTION structure
\param pRtpConf        Handle to IFX_TAPI_PKT_RTP_CFG_t structure
\return Return value according to IFX_return_t
   - IFX_ERROR if an error occured
   - IFX_SUCCESS if successful
\remarks
*/
IFX_return_t VINETIC_SIG_RTP_OOB_Cfg (struct _VINETIC_CHANNEL *pCh,
                                      IFX_TAPI_PKT_RTP_CFG_t const *pRtpConf)
{
   VINETIC_DEVICE *pDev = (VINETIC_DEVICE*) (pCh->pParent);
   FWM_SIG_CH_CONF *pSigCfg = &pCh->pSIG->fw_sig_ch_cfg;
   IFX_uint16_t pCmd [5] = {0};
   IFX_int32_t ret = IFX_SUCCESS;
   IFX_uint8_t ch = pCh->nChannel - 1, bApplyRec = IFX_FALSE;

   if (pRtpConf->nEventPT == 0)
   {
      SET_ERROR(VIN_ERR_WRONG_EVPT);
      return IFX_ERROR;
   }
   pSigCfg->bit.eventPtUp = pRtpConf->nEventPT & 0x7F;
   pSigCfg->bit.eventPtDn = pRtpConf->nEventPT & 0x7F;
   pSigCfg->bit.ssrch     = HIGHWORD (pRtpConf->nSsrc);
   pSigCfg->bit.ssrcl     = LOWWORD  (pRtpConf->nSsrc);

   /* do event transmission settings for DTMF receiver */
   if (ret == IFX_SUCCESS)
   {
      pCmd[2] = pCh->pSIG->fw_dtmf_rec.value;
      IFXOS_MutexLock (pDev->memberAcc);
      pCh->pSIG->fw_dtmf_rec.bit.as = IFX_FALSE;
      switch (pRtpConf->nEvents)
      {
         case IFX_TAPI_PKT_EV_OOB_NO:
            /* No Event transmission support */
            /* store the setting for later use */
            pCh->pSIG->et_stat.value = 0;
            break;
         case IFX_TAPI_PKT_EV_OOB_DEFAULT:
         case IFX_TAPI_PKT_EV_OOB_ONLY:
            pCh->pSIG->fw_dtmf_rec.bit.as = IFX_TRUE;
            /*lint-fallthrough*/
         case IFX_TAPI_PKT_EV_OOB_ALL:
            /* event transmission support */
            /* store the setting for later use */
            pCh->pSIG->et_stat.value = 0xFFFF;
            break;
         case IFX_TAPI_PKT_EV_OOB_BLOCK:
            /* don't tansmit the event - neither in- nor outofband */
            pCh->pSIG->fw_dtmf_rec.bit.as = IFX_TRUE;
            pCh->pSIG->et_stat.value = 0;
            break;
         default:
           ret = IFX_ERROR;
           break;
      }
      if (pCmd[2] != pCh->pSIG->fw_dtmf_rec.value)
      {
        pCmd[2] = pCh->pSIG->fw_dtmf_rec.value;
        bApplyRec = IFX_TRUE;
      }
      IFXOS_MutexUnlock (pDev->memberAcc);
   }

   /* set event pt, ssrc, sequence nr for signaling channel : This has to be
      done with inactive signaling channel */
#ifdef FW_ETU
   {
      pSigCfg->bit.coderCh = ch;
      switch (pRtpConf->nEvents)
      {
         case IFX_TAPI_PKT_EV_OOB_NO:
            /* No Event transmission support */
            break;
         case IFX_TAPI_PKT_EV_OOB_DEFAULT:
         case IFX_TAPI_PKT_EV_OOB_ONLY:
            /* muting the voice in sig channel rtp */
            sigConf.bit.a1 = 2;
            sigConf.bit.a2 = 2;
            break;
         case IFX_TAPI_PKT_EV_OOB_ALL:
            /* event transmission support no muting */
            pSigCfg->bit.a1 = 1;
            pSigCfg->bit.a2 = 1;
         break;
         default:
            ret = IFX_ERROR;
            break;
      }
      switch (pRtpConf->nPlayEvents)
      {
         case IFX_TAPI_PKT_EV_OOBPLAY_DEFAULT:
         case IFX_TAPI_PKT_EV_OOBPLAY_PLAY:
               /* all playout always enabled */
               pSigCfg->bit.evMaskTOG = 0x7;
               break;
         case IFX_TAPI_PKT_EV_OOBPLAY_MUTE:
            /* all playout always enabled */
            pSigCfg->bit.evMaskTOG = 0x0;
            break;
         default:
            ret = IFX_ERROR;
            break;
      }
      /* enable the trigger if mask is set */
      if (pChannel->TapiMiscData.nExceptionMask.Bits.eventDetect == 0)
      {
         pSigCfg->bit.evMaskTrig = 0x7F;
      }
      else
      {
         pSigCfg->bit.evMaskTrig = 0;
      }
      ret = CmdWrite (pDev, pSigCfg->value, 6);
      if (ret == IFX_SUCCESS)
      {
         SIG_MOD_STATE state;

         state.value = 0;
         IFXOS_MutexLock (pDev->memberAcc);
         if (pDev->pCodCh[ch].cod_ch.bit.en == IFX_TRUE)
         {
            /* if coder running we apply it directely otherwise
               it is just stored */
            ret = VINETIC_SIG_UpdateEventTrans(pCh, pCh->pSIG->et_stat, &state);
         }
         IFXOS_MutexUnlock (pDev->memberAcc);
         /* if DTMF receiver has been written we do not need it afterwards */
         if (state.flag.fw_dtmf_rec == IFX_TRUE)
            bApplyRec = IFX_FALSE;

      }
   }
#else
   ret = CmdWrite (pDev, pSigCfg->value, 5);
#endif /* FW_ETU */

   if (bApplyRec)
   {
      /* value changed, so write it */
      pCmd[0] = CMD1_EOP | ch;
      pCmd[1] = ECMD_DTMF_REC;
      pCmd[2] = pCh->pSIG->fw_dtmf_rec.value;
      ret = CmdWrite (pDev, pCmd, 1);
   }

   /* store the information for downstream service. */
   if (ret == IFX_SUCCESS)
   {
      /* error when coder is active */
      if (pRtpConf->nEventPT)
      {
         /* Configure transmission through events and use payload type value
            from nEventPT */
         pCh->nEvtPT = pRtpConf->nEventPT;
      }
   }
   return ret;
}


/**
   base SIGNALING Module configuration
\param
   pDev  - pointer to the device interface
\param
   nPath  - Voice path selected. Either PCM or CODEC
\return
    IFX_SUCCESS if no error, otherwise IFX_ERROR
Remark:
   Use this function where needed to set the base configuration
   of Signaling Module. This function isn't an IOCTL function
   This function configures:
      - SIGNALLING Module
      - all SIGNALING Channels
      - CID, DTMF gen & recv
      - ATDs, UTDs or MFTDs

   In case of PCM voice path, the signaling channle isn't yet connected to any
   PCM Output. This is to be changed later if.

   Note: the following configuration is suitable for network tone detection :
         pCmd [2] = (0x8000 | (((0x18 + i) << 8) & 0x3F00)) | (0x11 + i);
         The switching should be done when this feature is activated by ioctl.
         this configuration is still in examination.
*/
IFX_int32_t  VINETIC_SIG_baseConf (VINETIC_CHANNEL *pCh)

{
   VINETIC_DEVICE *pDev  = pCh->pParent;
   IFX_uint16_t pCmd [9] = {0};
   IFX_uint8_t  ch       = (pCh->nChannel - 1);
   IFX_int32_t  err      = IFX_SUCCESS;

   /* set channel */
   pCmd [0] = CMD1_EOP | ch;

   /* Configure Signaling Channels ***************************************** */
   pCh->pSIG->fw_sig_ch.bit.en  = 1;

   /* Set the inputs in the cached message and write it */
   err = VINETIC_SIG_Set_Inputs(pCh);


#ifdef TAPI_CID
   /* set CID Defaults ****************************************************** */
   if (err == IFX_SUCCESS)
   {
      /* EN = 0 , AD = 1, HLEV = 1, V23 = 1, A1 = 1, A2 = 1, CISNR = ch */
      pCh->pSIG->cid_sender[CMD_HEADER_CNT] = SIG_CID_AD | SIG_CID_HLEV |
                     SIG_CID_V23 | SIG_CID_A1_VOICE | SIG_CID_A2_VOICE | ch;
      err = CmdWrite (pDev, pCh->pSIG->cid_sender, 1);
   }
   /* Set Cid Receiver Defaults ******************************************* */
   /* P.S: Only the ressource number is programmed.
           The default coefficients are used for the cid receiver.  */
   if (err == IFX_SUCCESS)
   {
      /* set length & cmd 2 */
      pCmd [1] = ECMD_CIDRX;
      /* CIDRNR = ch . Other parameters as is. */
      pCmd [2] = ch;
      err = CmdWrite (pDev, pCmd, 1);
   }
#endif /* TAPI_CID */
   /* Set DTMF Generator Defaults ******************************************* */
   if (err == IFX_SUCCESS)
   {
      /* EN = 1, AD = 1, MOD = 0, FGMOD = 1 */
      /* Note : when ET = 1 : MOD = 0 , FG = 1 (ref FW Spec). If done another
         way, it leads to CERR in FW. */
      pCh->pSIG->fw_sig_dtmfgen.value[CMD_HEADER_CNT] = (0x4A00 | ch);
      pCh->pSIG->fw_sig_dtmfgen.bit.addb = 1;
#ifdef FW_ETU
      pCh->pSIG->fw_sig_dtmfgen.bit.et = 0;
#else
      pCh->pSIG->fw_sig_dtmfgen.bit.et = 1;
#endif /* FW_ETU */
      err = CmdWrite (pDev, pCh->pSIG->fw_sig_dtmfgen.value, CMD_SIG_DTMFGEN_LEN);
   }

   /* Set DTMF Receiver Coefficients **************************************** */
   if (err == IFX_SUCCESS)
   {
      /* set the default level of the DTMF receiver to -30dB. The firmware
         default is set to -56dB. It is recommended not to use a value smaller
         than -30dB. The "TWIST" value stays unchanged */
      /* Level= -30dB, Twist= 9.1dB */
      pCh->pSIG->fw_dtmfr_coeff.value[2] = 0xE21C;
      /* Gain adjustment 0dB */
      pCh->pSIG->fw_dtmfr_coeff.value[3] = 0x0060;
      err = CmdWrite (pDev, pCh->pSIG->fw_dtmfr_coeff.value, 2);
   }

   /* Set DTMF Receiver Defaults ******************************************** */
   if (err == IFX_SUCCESS)
   {
      /* set length & cmd 2 */
      pCmd[1] = ECMD_DTMF_REC;
      /* EN = 1, ET = 0, AS = 1 */
      pCh->pSIG->fw_dtmf_rec.value = 0x8010 | ch;
      pCmd[2] = pCh->pSIG->fw_dtmf_rec.value;
      err = CmdWrite (pDev, pCmd, 1);
   }
   if (err == IFX_SUCCESS)
   {
      /* Set ATD1 Defaults for Answering Tone Detection (CED) Set ATD2 for CED
         network tone detection Default ATD coefficients can be used in this
         case */
      /* set resource information -> will never be changed */
      pCh->pSIG->fw_sig_atd1.value[CMD_HEADER_CNT] = 0;
      pCh->pSIG->fw_sig_atd1.bit.resnr = ch;
      pCh->pSIG->fw_sig_atd2.value[CMD_HEADER_CNT] = 0;
      pCh->pSIG->fw_sig_atd2.bit.resnr = ch + pDev->caps.nSIG;

      /* set UTD1/2 coefficients for Calling Tone Detection, 1100 Hz **********/
      /* UTD1 : Ressources 0 - 3 for Tone detection from SigInA
                (here ALI for Detection of Tones from ALM)
         UTD1 is used for detection of tones comming from ALI */
      pCh->pSIG->fw_sig_utd1.value[CMD_HEADER_CNT] = 0;
      pCh->pSIG->fw_sig_utd1.bit.md = VIN_ECMD_SIGUTD_MD_UNV;
      pCh->pSIG->fw_sig_utd1.bit.resnr = ch;
      /* UTD2 : Ressources 4 - 7 for Tone detection from SigInB
                (here Coder for Network Tone Detection)
         UTD2 is used to detect tones comming for Coder (Network) */
      pCh->pSIG->fw_sig_utd2.value[CMD_HEADER_CNT] = 0;
      pCh->pSIG->fw_sig_utd2.bit.md = VIN_ECMD_SIGUTD_MD_UNV;
      pCh->pSIG->fw_sig_utd2.bit.resnr = ch + pDev->caps.nSIG;
      /* set resource information equal to channel for mftd */
      pCh->pSIG->fw_sig_mftd.value[CMD_HEADER_CNT+0] = 0;
      pCh->pSIG->fw_sig_mftd.bit.resnr = ch;
   }

   /* set UTG1 without enabling it */
   if (err == IFX_SUCCESS)
   {
      /* set length & cmd 2 */
      pCmd [1] = ECMD_UTG1;
      /* set channel */
      pCmd [0] = CMD1_EOP | ch;
      /* EN = 0, A1 = 0b10, A2 = 0b10, UTGNR = ch */
      pCmd [2] = 0x00A0 | ch;
      err = CmdWrite (pDev, pCmd, 1);
   }

   if (pDev->caps.nMFTD > 0 && ch == 0)
   {
      /* hardcode MFTD gap holdtime for transp. fax transmissions to 7 sec,
         all other parameters are FW defaults, only once for ch = 0 (broadcast)
       */
      if (err == IFX_SUCCESS)
      {
         pCmd[0] = 0x2600;
         pCmd[1] = 0xCD07;
         pCmd[2] = 0x0123;    /* LEVELS=-35dB */
         pCmd[3] = 0x2E6A;    /* AM-MOD=0.18, PHASE=150DEG */
         pCmd[4] = 0x287A;    /* SNR=5dB */
         pCmd[5] = 0x0A15;    /* AGAP=100ms, RTIME=210ms */
         pCmd[6] = 0x0A0C;    /* ABREAK=100ms, RGAP=120ms */
         pCmd[7] = 0x0246;    /* LEVELHOLD=-35dB */
         pCmd[8] = TONEHOLDING_RGAP;   /* RGAPHOLD=7000ms */
         err = CmdWrite (pDev, pCmd, 7);
      }
   }

   if (err != IFX_SUCCESS)
   {
      TRACE (VINETIC, DBG_LEVEL_HIGH,
             ("SIGNALING configuration failed." " Dev Err = %d\n\r",
              pDev->err));
   }

   return (err == IFX_SUCCESS) ? IFX_SUCCESS : IFX_ERROR;
}


/**
   Initalize the signalling module and the cached firmware messages

   \param   VINETIC_CHANNEL*   Pointer to the VINETIC channel structure
   \return  none
*/
IFX_void_t VINETIC_SIG_Init_Ch (VINETIC_CHANNEL *pCh)
{
   VINETIC_SIGCH_t *pSIG = pCh->pSIG;
   IFX_uint8_t     ch    = pCh->nChannel - 1;

   TRACE(VINETIC, DBG_LEVEL_LOW, ("INFO: VINETIC_SIG_Init_Ch called\n\r"));

   VINETIC_CON_Init_SigCh (pCh);

   /* SIG CH */
   memset (pSIG->fw_sig_ch.value, 0, sizeof(pSIG->fw_sig_ch));
   pSIG->fw_sig_ch.value[0] = CMD1_EOP | ch;
   pSIG->fw_sig_ch.value[1] = ECMD_SIG_CH;

   /* SIG CH CFG */
   memset (pSIG->fw_sig_ch_cfg.value, 0, sizeof(pSIG->fw_sig_ch_cfg));
   pSIG->fw_sig_ch_cfg.value[0]      = CMD1_EOP | ch;
   pSIG->fw_sig_ch_cfg.value[1]      = ECMD_SIG_CH_RTP;
   pSIG->fw_sig_ch_cfg.bit.enDnEPt   = 1;
   pSIG->fw_sig_ch_cfg.bit.a2        = 1;
   pSIG->fw_sig_ch_cfg.bit.evMaskTOG = 0x7;
   pSIG->fw_sig_ch_cfg.bit.coderCh   = ch;

   /* SIG UTG */
   memset (pSIG->fw_sig_utg[0].value, 0, sizeof(pSIG->fw_sig_utg[0]));
   memset (pSIG->fw_sig_utg[1].value, 0, sizeof(pSIG->fw_sig_utg[1]));
   pSIG->fw_sig_utg[0].value[0]    = CMD1_EOP | ch;
   pSIG->fw_sig_utg[0].value[1]    = ECMD_UTG1;
   pSIG->fw_sig_utg[0].bit.utgnr   = ch;
   pSIG->fw_sig_utg[1].value[0]    = CMD1_EOP | ch;
   pSIG->fw_sig_utg[1].value[1]    = ECMD_UTG2;
   pSIG->fw_sig_utg[1].bit.utgnr   = ch + pCh->pParent->caps.nSIG;

   /* tone detectors */
   memset (pSIG->fw_sig_atd1.value, 0, sizeof(pSIG->fw_sig_atd1));
   memset (pSIG->fw_sig_atd2.value, 0, sizeof(pSIG->fw_sig_atd2));
   pSIG->fw_sig_atd1.value[0] = CMD1_EOP | ch;
   pSIG->fw_sig_atd1.value[1] = ECMD_SIG_ATD1;
   pSIG->fw_sig_atd2.value[0] = CMD1_EOP | ch;
   pSIG->fw_sig_atd2.value[1] = ECMD_SIG_ATD2;
   memset (pSIG->fw_sig_utd1.value, 0, sizeof(pSIG->fw_sig_utd1));
   memset (pSIG->fw_sig_utd2.value, 0, sizeof(pSIG->fw_sig_utd2));
   pSIG->fw_sig_utd1.value[0] = CMD1_EOP | ch;
   pSIG->fw_sig_utd1.value[1] = ECMD_SIG_UTD1;
   pSIG->fw_sig_utd2.value[0] = CMD1_EOP | ch;
   pSIG->fw_sig_utd2.value[1] = ECMD_SIG_UTD2;
   memset (pSIG->fw_sig_mftd.value, 0, sizeof(pSIG->fw_sig_mftd));
   pSIG->fw_sig_mftd.value[0] = CMD1_EOP | ch;
   pSIG->fw_sig_mftd.value[1] = ECMD_SIG_MFTD;

   /* FSK Sender (CID) */
   memset (pSIG->cid_sender, 0, sizeof(pSIG->cid_sender));
   pSIG->cid_sender [0] = CMD1_EOP | ch;
   pSIG->cid_sender [1] = ECMD_CID_SEND;

   /* DTMF generator */
   memset (pSIG->fw_sig_dtmfgen.value, 0, sizeof(pSIG->fw_sig_dtmfgen));
   pSIG->fw_sig_dtmfgen.value[0] = CMD1_EOP | ch;
   pSIG->fw_sig_dtmfgen.value[1] = ECMD_DTMF_GEN | CMD_SIG_DTMFGEN_LEN;

   /* DTMF receiver coefficients */
   memset (pSIG->fw_dtmfr_coeff.value, 0, sizeof(pSIG->fw_dtmfr_coeff));
   pSIG->fw_dtmfr_coeff.value[0] = CMD1_EOP | ch;
   pSIG->fw_dtmfr_coeff.value[1] = ECMD_DTMF_REC_COEFF;

   /* DTMF generator default timing configuration (times in ms) */
   pSIG->nDtmfInterDigitTime = 100;
   pSIG->nDtmfDigitPlayTime  = 100;
}


/**
   Set the signal inputs of the cached fw message for the given channel

   \param   VINETIC_CHANNEL*   Pointer to the VINETIC channel structure
   \return  IFX_SUCCESS or IFX_ERROR
*/
IFX_return_t VINETIC_SIG_Set_Inputs (VINETIC_CHANNEL *pCh)
{
   FWM_SIG_CH        *p_fw_sig_ch;
   IFX_return_t      ret = IFX_SUCCESS;

   /* update the signal inputs of this cached msg */
   p_fw_sig_ch = &pCh->pSIG->fw_sig_ch;

   IFXOS_MutexLock (pCh->chAcc);
   p_fw_sig_ch->bit.i1 = VINETIC_CON_Get_SIG_SignalInput (pCh, 0);
   p_fw_sig_ch->bit.i2 = VINETIC_CON_Get_SIG_SignalInput (pCh, 1);

   ret = CmdWrite (pCh->pParent, p_fw_sig_ch->value, CMD_SIG_CH_LEN);

   IFXOS_MutexUnlock (pCh->chAcc);

   return ret;
}


/**
  Allocate data structures of the SIG module in the given channel

  \param   VINETIC_CHANNEL*   Pointer to the VINETIC channel structure
  \return  IFX_SUCCESS or IFX_ERROR in case the stucture could not be created
  \remarks The channel parameter is not checked because the calling
           function assures correct values.
*/
IFX_return_t VINETIC_SIG_Allocate_Ch_Structures (VINETIC_CHANNEL *pCh)
{
   VINETIC_SIG_Free_Ch_Structures (pCh);

   pCh->pSIG = IFXOS_MALLOC(sizeof(VINETIC_SIGCH_t));
   if (pCh->pSIG == IFX_NULL)
   {
      return IFX_ERROR;
   }
   memset(pCh->pSIG, 0, sizeof(VINETIC_SIGCH_t));

   return IFX_SUCCESS;
}


/**
   Free data structures of the SIG module in the given channel

   \param   VINETIC_CHANNEL*   Pointer to the VINETIC channel structure
   \return  none
*/
IFX_void_t VINETIC_SIG_Free_Ch_Structures (VINETIC_CHANNEL *pCh)
{
   if (pCh->pSIG != IFX_NULL)
   {
      IFXOS_FREE(pCh->pSIG);
   }
}

#endif /* (VIN_FEAT_VIN_C | VIN_FEAT_VIN_M | VIN_FEAT_VIN_2CPE) */
