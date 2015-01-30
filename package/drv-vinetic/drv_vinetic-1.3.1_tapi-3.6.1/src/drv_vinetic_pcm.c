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
   Module      : drv_vinetic_pcm.c
   Description : This file implements the PCM module
******************************************************************************/

/* ============================= */
/* Includes                      */
/* ============================= */
#include "drv_vinetic_pcm_priv.h"
#include "drv_vinetic_con.h"

/* ============================= */
/* Local Macros & Definitions    */
/* ============================= */

/* ============================= */
/* Local variables and types     */
/* ============================= */

/* ============================= */
/* Local function declaration    */
/* ============================= */

static IFX_uint8_t get_free_lec_res (VINETIC_DEVICE *pDev);
static IFX_int32_t vinetic_pcm_Lec_Cfg (VINETIC_CHANNEL *pCh,
                                        IFX_boolean_t bEnLec,
                                        IFX_boolean_t bEnNlp,
                                        IFX_boolean_t bEnWLec);
static IFX_return_t IFX_TAPI_LL_PCM_DEC_HP_Set (IFX_TAPI_LL_CH_t *pLLChannel,
                                                IFX_boolean_t bHp);

/* ============================= */
/* Function definitions          */
/* ============================= */

/******************************************************************************/
/**
   Get a free resource from the LEC resource management. It assumes that there
   is any resource available. The driver checks the availability of a
   resource before calling this function.
   The resource gets immediately allocated and reserved inside this function.

   \param
     pDev  -  Device handle
   \return
      Number of the LEC resource
   \remarks
      This function does not have any error handling!
*******************************************************************************/
static IFX_uint8_t get_free_lec_res (VINETIC_DEVICE *pDev)
{
   IFX_uint16_t   tmp = pDev->availLecRes;
   IFX_uint8_t    i;

   for (i = 0; i < pDev->caps.nNLEC; i++)
   {
      if (tmp & 1)
         break;

      tmp = tmp >> 1;
   }

   pDev->availLecRes &= ~((IFX_uint16_t)(1 << i));

   return i;
}


/**
   Configures the Near LEC in the PCM module
   \param   pCh        - pointer to VINETIC channel structure
   \param   bEnLec     - IFX_FALSE : disable / IFX_TRUE : enable LEC
   \param   bEnNlp     - IFX_FALSE : disable / IFX_TRUE : enable NLP
   \param   bEnWLec    - IFX_FALSE : disable / IFX_TRUE : enable WLEC
                         implicitly enables LEC

   \return
      IFX_SUCCESS or IFX_ERROR
*/
static IFX_int32_t vinetic_pcm_Lec_Cfg(VINETIC_CHANNEL *pCh,
                                       IFX_boolean_t bEnLec,
                                       IFX_boolean_t bEnNlp,
                                       IFX_boolean_t bEnWLec)
{
   VINETIC_DEVICE *pDev = pCh->pParent;
   IFX_uint16_t   ch = (pCh->nChannel - 1);
   IFX_uint16_t   pCmd1[3];
   IFX_uint16_t   pCmd2[11];
   IFX_uint8_t    lec_res;

   /* coefficients for NLEC (can be used for all firmware versions) */
   static const IFX_uint16_t pcm_lec_nlec_coefs[5] =
   {
      ECMD_NELEC_COEFS, 0x1048, 0x0808, 0x6060, 0x0060
   };

   /* coefficients for WLEC (can only be used for firmware with WLEC ) */
   static const IFX_uint16_t pcm_lec_wlec_coefs[7] =
   {
      ECMD_NELEC_COEFS, 0x1048, 0x0808, 0x6060, 0x0060, 0x0834, 0x0008
   };

   /* coefficients for Non Linear Processing */
   static const IFX_uint16_t pcm_lec_nlp_coefs[10] =
   {
      ECMD_NELEC_NLP, 0x1502, 0x4540, 0x0C40, 0x41FE,
      0x1106, 0x4647, 0x0D10, 0x6480, 0x0810
   };

   /* ---------------------------------------------------- */

   /* if WLEC should be used, check WLEC capability and parameters */
   if ( (bEnWLec == IFX_TRUE) && (pDev->caps.nWLEC == 0) )
   {
      /* WLEC is not available => error code VIN_ERR_FUNC_PARM */
      SET_ERROR (VIN_ERR_FUNC_PARM);
      return IFX_ERROR;
   }
   if (ch >= pDev->caps.nPCM)
   {
      /* this is not an pcm channel */
      SET_ERROR (VIN_ERR_FUNC_PARM);
      return IFX_ERROR;
   }
   /* set channel */
   pCmd1 [0] = CMD1_EOP | pCh->nPcmCh;

   if ((bEnLec == IFX_FALSE) && (bEnWLec == IFX_FALSE))
   {
      /* disable LEC */
      if (pDev->pChannel[ch].pPCM->pcm_nelec & PCM_NLEC_EN)
      {
         /* release LEC resource back to the minimal resource management. */
         pDev->availLecRes |= (1 << (pDev->pChannel[ch].pPCM->pcm_nelec & PCM_LECNR));
         /* clear LEC enable bits in cached command and keep everything else */
         pDev->pChannel[ch].pPCM->pcm_nelec &= ~(PCM_NLEC_EN | PCM_NLP_EN);
         /* set and write ALI NELEC command */
         pCmd1 [1] = ECMD_PCM_NEARLEC;
         pCmd1 [2] = pDev->pChannel[ch].pPCM->pcm_nelec;
         return CmdWrite (pDev, pCmd1, 1);
      }
   }
   else
   {
      /* configure and enable LEC */

      if (!(pDev->pChannel[ch].pPCM->pcm_nelec & PCM_NLEC_EN))
      {
         /* LEC resource required */
         if (pDev->availLecRes == 0)
         {
            SET_ERROR (VIN_ERR_NORESOURCE);
            return IFX_ERROR;
         }
         /* allocate a LEC resource */
         lec_res = get_free_lec_res(pDev);
         /* cross check with capabilities */
         if (lec_res >= pDev->caps.nNLEC)
            return IFX_ERROR;
         /* clear command word cache */
         pDev->pChannel[ch].pPCM->pcm_nelec = 0;
         /* set channel in lec command */
         pCmd1 [2] = (PCM_NLEC_EN | lec_res);
         /* set channel in lec coefficient command */
         pCmd2 [0] = (CMD1_EOP | lec_res);
         /* set NLP coefficients only here where LEC is still unconfigured  */
         /* coefficients are set independently of the real activation later */
         memcpy(&pCmd2[1], pcm_lec_nlp_coefs, sizeof(pcm_lec_nlp_coefs));
         if (CmdWrite (pDev, pCmd2, 9) != IFX_SUCCESS)
            return IFX_ERROR;
      }
      else
      {
         /* No LEC resource is required, because LEC is already running */
         /* load command word from cache */
         pCmd1 [2] = pDev->pChannel[ch].pPCM->pcm_nelec;
         /* get the LEC resource which is used for this channel */
         lec_res = pDev->pChannel[ch].pPCM->pcm_nelec & PCM_LECNR;
         /* cross check with capabilities */
         if (lec_res >= pDev->caps.nNLEC)
            return IFX_ERROR;
         /* set channel in lec coefficient command */
         pCmd2 [0] = CMD1_EOP | lec_res;
      }

      /* Set the WLEC/NLEC coefficients */
      if ( bEnWLec == IFX_TRUE )
      {
         /* set WLEC coefficients defaults for PCM
         WLEC Configuration: Normal operation mode for the window based LEC.
         The window position as well as the filter coefficients are adapted by the LEC.
         OLDP = 0, OLDC = 0
         DCF = 1,
         MW = 1, The de-correlation filter is part of the new WLEC only
         and should be activated. */
         pCmd1 [2] &= ~(PCM_OLDC_EN | PCM_OLDP);
         pCmd1 [2] |=  (PCM_MW_EN | PCM_DCF_EN);
         /* set NLEC coefficients defaults for PCM */
         memcpy(&pCmd2[1], pcm_lec_wlec_coefs, sizeof(pcm_lec_wlec_coefs));
         /* Overwrite window size parameters */
         pCmd2[2] = ((IFX_uint16_t)(pCh->pPCM->lec_window_coefs [1]) << 8) |
                    (pCmd2[2] & 0xFF);
         pCmd2[6] = ((IFX_uint16_t)(pCh->pPCM->lec_window_coefs [2]) << 8) |
                    (pCmd2[6] & 0xFF);
         /* write NLEC coefficients command to PCM */
         if (CmdWrite (pDev, pCmd2, 6) != IFX_SUCCESS)
            return IFX_ERROR;
      }
      else
      {
         /* set NLEC coefficients defaults for PCM */
         pCmd1 [2] &= ~(PCM_OLDC_EN | PCM_OLDP | PCM_MW_EN | PCM_DCF_EN);
         memcpy(&pCmd2[1], pcm_lec_nlec_coefs, sizeof(pcm_lec_nlec_coefs));
         /* Overwrite window size parameters */
         pCmd2[2] = ((IFX_uint16_t)(pCh->pPCM->lec_window_coefs [0]) << 8) |
                    (pCmd2[2] & 0xFF);
         pCmd2[6] = ((IFX_uint16_t)(pCh->pPCM->lec_window_coefs [0]) << 8) |
                    (pCmd2[6] & 0xFF);
         /* write NLEC coefficients command to PCM */
         if (CmdWrite (pDev, pCmd2, 4) != IFX_SUCCESS)
            return IFX_ERROR;
      }

      /* Set the enable bit for NLP according to callers wishes */
      if (bEnNlp == IFX_FALSE)
         pCmd1 [2] &= ~PCM_NLP_EN;
      else
         pCmd1 [2] |= PCM_NLP_EN;

      /* only write to the device if the command word has changed */
      if (pCmd1 [2] != pDev->pChannel[ch].pPCM->pcm_nelec)
      {
         /* set PCM NELEC command */
         pCmd1 [1] = ECMD_PCM_NEARLEC;
         if (CmdWrite (pDev, pCmd1, 1) != IFX_SUCCESS)
            return IFX_ERROR;
         pDev->pChannel[ch].pPCM->pcm_nelec = pCmd1 [2];
      }
   }
   return IFX_SUCCESS;
}


/**
   Prepare parameters and call the Target Configuration Function to activate/
   deactivate the pcm interface.
\param pChannel   Handle to TAPI_CHANNEL structure
\param nMode  Activation mode
   -1: timeslot activated
   -0: timeslot deactivated
\return Return value according to IFX_return_t
   - IFX_ERROR if an error occured
   - IFX_SUCCESS if successful
\remarks
   The configuration must be done previously with the low level function
   TAPI_LL_Phone_PCM_Activation.
   Resource availability check is done. The function returns with error when
   the resource or the timeslot is not available.
*/
IFX_return_t IFX_TAPI_LL_PCM_Enable (IFX_TAPI_LL_CH_t *pLLChannel,
                                     IFX_uint32_t nMode,
                                     IFX_TAPI_PCM_CFG_t *pPcmCfg)
{
   IFX_uint32_t      IdxRx, BitValueRx;
   IFX_uint32_t      IdxTx, BitValueTx;
   VINETIC_CHANNEL *pCh = (VINETIC_CHANNEL *) pLLChannel;
   VINETIC_DEVICE *pDev = (VINETIC_DEVICE*) (pCh->pParent);
   FWM_PCM_CH        *pPcmCh;
   IFX_int32_t       ret = IFX_SUCCESS;

   if (pCh->nChannel - 1  >= pDev->caps.nPCM)
   {
      ret = IFX_ERROR;
      SET_ERROR (VIN_ERR_NORESOURCE);
      goto end_fkt;
   }

   pPcmCh = &pCh->pPCM->fw_pcm_ch;

   /* Activate Timeslots and set highway */
   switch (nMode)
   {
   case  0:
      /*
      the device variables PcmRxTs, PcmTxTs are global for all
      channels and must be protected against concurent tasks access
      */
      IFXOS_MutexLock (pDev->memberAcc);
      switch (pPcmCh->bit.cod)
      {
      case 0: /* IFX_TAPI_PCM_RES_LINEAR_16BIT */
         /* get the index in the time slot array. Each bit reflects one time
         slot. Modulo 32 separate the index of the array and the bit inside
         the field. */
         /* for 16 Bit mode, the next higher time slot has to be reserved too */
         IdxRx = (pPcmCh->bit.rts + 1) >> 5;
         BitValueRx = 1 << ((pPcmCh->bit.rts + 1) & 0x1F);
         IdxTx = (pPcmCh->bit.xts + 1) >> 5;
         BitValueTx = 1 << ((pPcmCh->bit.xts + 1) & 0x1F);

         /* Receive Slot checking */
         if ((pDev->PcmRxTs[pPcmCh->bit.r_hw][IdxRx] & BitValueRx) == IFX_FALSE)
         {
            IFXOS_MutexUnlock (pDev->memberAcc);
            TRACE(VINETIC,DBG_LEVEL_HIGH,
               ("\n\rDRV_ERROR: Rx Slot is not available\n\r"));
            ret = IFX_ERROR;
            SET_ERROR (VIN_ERR_NORESOURCE);
            goto end_fkt;
         }

         if ((pDev->PcmTxTs[pPcmCh->bit.x_hw][IdxTx] & BitValueTx) == IFX_FALSE)
         {
            IFXOS_MutexUnlock (pDev->memberAcc);
            TRACE(VINETIC,DBG_LEVEL_HIGH,
               ("\n\rDRV_ERROR: Tx Slot is not available\n\r"));
            ret = IFX_ERROR;
            SET_ERROR (VIN_ERR_NORESOURCE);
            goto end_fkt;
         }

         /* reserve the time slots now, because the parameter are OK */
         pDev->PcmRxTs[pPcmCh->bit.r_hw][IdxRx]  &= ~BitValueRx;
         pDev->PcmTxTs[pPcmCh->bit.x_hw][IdxTx] &= ~BitValueTx;
         /* Fall through and do not 'break'!
         For 16-Bit PCM Mode, the next higher PCM time slot was reserved
         and now the used time slot will be reserved.
         That part is similar to the 8-Bit PCM Mode */
         /* break; */
         /*lint -fallthrough */
      case 2: /* IFX_TAPI_PCM_RES_ALAW_8BIT */
      case 3: /* IFX_TAPI_PCM_RES_ULAW_8BIT */
      default:
         /* get the index in the time slot array. Each bit reflects one time
         slot. Modulo 32 separate the index of the array and the bit inside
         the field. */
         IdxRx = pPcmCh->bit.rts >> 5;
         BitValueRx = 1 << (pPcmCh->bit.rts & 0x1F);
         IdxTx = pPcmCh->bit.xts >> 5;
         BitValueTx = 1 << (pPcmCh->bit.xts & 0x1F);

         /* Receive Slot checking */
         if ((pDev->PcmRxTs[pPcmCh->bit.r_hw][IdxRx] & BitValueRx) == IFX_FALSE)
         {
            IFXOS_MutexUnlock (pDev->memberAcc);
            TRACE(VINETIC,DBG_LEVEL_HIGH,
               ("\n\rDRV_ERROR: Rx Slot is not available\n\r"));
            ret = IFX_ERROR;
            SET_ERROR (VIN_ERR_NORESOURCE);
            goto end_fkt;
         }

         if ((pDev->PcmTxTs[pPcmCh->bit.x_hw][IdxTx] & BitValueTx) == IFX_FALSE)
         {
            IFXOS_MutexUnlock (pDev->memberAcc);
            TRACE(VINETIC,DBG_LEVEL_HIGH,
               ("\n\rDRV_ERROR: Tx Slot is not available\n\r"));
            ret = IFX_ERROR;
            SET_ERROR (VIN_ERR_NORESOURCE);
            goto end_fkt;
         }

         /* reserve the time slots now, because the parameter are OK */
         pDev->PcmRxTs[pPcmCh->bit.r_hw][IdxRx]  &= ~BitValueRx;
         pDev->PcmTxTs[pPcmCh->bit.x_hw][IdxTx] &= ~BitValueTx;
         /* Fall through and do not 'break'!
         For 16-Bit PCM Mode, the next higher PCM time slot was reserved
         and now the used time slot will be reserved.
         That part is similar to the 8-Bit PCM Mode */
         break;
      }
      /* disable channel: This will deactivate the timeslots */
      pPcmCh->bit.en = 0;
      /* write coefficients back */
      ret = CmdWrite (pDev, pPcmCh->value, CMD_PCM_CH_LEN);
      /* release share variables lock */
      IFXOS_MutexUnlock (pDev->memberAcc);
      break;
   case 1:
      /*
      the device variables PcmRxTs, PcmTxTs are global for all
      channels and must be protected against concurent tasks access
      */
      IFXOS_MutexLock (pDev->memberAcc);
#ifdef VIN_V14_SUPPORT
      /* The VINETIC-1.x has a limitation here. When 16-Bit mode, every second
         PCM channel is blocked. During driver initialization, every second
         PCM channel is assigned to the first half of the TAPI
         channel (0,2,4,6,...). The other PCM channel are assigned to the second
         half of the TAPI channel (1,3,5,7...). The first half are allowed to
         support 16-Bit mode if no PCM channel has been configured in 8-Bit
         before. In 16-Bit mode the upper half of TAPI channel can not be used.
      */
      if (pDev->nChipMajorRev == VINETIC_V1x)
      {
         /*VINETIC_CHANNEL *pCh = &pDev->pChannel[pChannel->nChannel];*/

         if (pDev->nPcmResolution == 0)
         {
            /* first time PCM channel gets enabled for this device . Set the
               running mode (8-Bit or 16-Bit). In case of 16-Bit mode block
               the upper half of TAPI channels to use for PCM. */
            if (pPcmCfg->nResolution == IFX_TAPI_PCM_RES_LINEAR_16BIT)
               pDev->nPcmResolution = 16;
            else
               pDev->nPcmResolution = 8;
         }

         if (((pDev->nPcmResolution == 16) &&
            (pPcmCfg->nResolution != IFX_TAPI_PCM_RES_LINEAR_16BIT)) ||
             ((pDev->nPcmResolution == 8) &&
             pPcmCfg->nResolution == IFX_TAPI_PCM_RES_LINEAR_16BIT))
         {
            /* this mode is not supported. A resolution that is configured once
               can not be changed anymore, because it could conflict wiht all
               previous configured PCM channels.
            */
            IFXOS_MutexUnlock (pDev->memberAcc);
            TRACE (VINETIC, DBG_LEVEL_HIGH,
                 ("\n\rDRV_ERROR: Previous PCM resolution conflict\n\r"));
            ret = IFX_ERROR;
            SET_ERROR (VIN_ERR_FUNC_PARM);
            goto end_fkt;
         }

         if ((pPcmCfg->nResolution == IFX_TAPI_PCM_RES_LINEAR_16BIT) &&
            (pCh->nPcmMaxResolution < 16))
         {
            /* This PCM channel does not support to run with 16-Bit resolution,
               because it might be one of the upper TAPI channels. These
               channels are supposed to only support 8-Bit mode. The lower half
               of the TAPI channel should support 16-Bit mode.
            */
            IFXOS_MutexUnlock (pDev->memberAcc);
            TRACE (VINETIC, DBG_LEVEL_HIGH,
                 ("\n\rDRV_ERROR: PCM 16-Bit resolution not supported\n\r"));
            ret = IFX_ERROR;
            SET_ERROR (VIN_ERR_NOTSUPPORTED);
            goto end_fkt;
         }
      }
#endif /* VIN_V14_SUPPORT */
      /* check timeslot allocation */
      switch (pPcmCfg->nResolution)
      {
      case IFX_TAPI_PCM_RES_LINEAR_16BIT:
         /* get the index in the time slot array. Each bit reflects one time
         slot. Modulo 32 separate the index of the array and the bit inside
         the field. */
         /* for 16 Bit mode, the next higher time slot has to be reserved too */
         IdxRx = (pPcmCfg->nTimeslotRX + 1) >> 5;
         BitValueRx = 1 << ((pPcmCfg->nTimeslotRX + 1) & 0x1F);
         IdxTx = (pPcmCfg->nTimeslotTX + 1) >> 5;
         BitValueTx = 1 << ((pPcmCfg->nTimeslotTX + 1) & 0x1F);

         /* Receive Slot checking */
         if (pDev->PcmRxTs[pPcmCfg->nHighway][IdxRx] & BitValueRx)
         {
            IFXOS_MutexUnlock (pDev->memberAcc);
            TRACE(VINETIC,DBG_LEVEL_HIGH,
               ("\n\rDRV_ERROR: Rx Slot is not available\n\r"));
            ret = IFX_ERROR;
            SET_ERROR (VIN_ERR_NORESOURCE);
            goto end_fkt;
         }

         if (pDev->PcmTxTs[pPcmCfg->nHighway][IdxTx] & BitValueTx)
         {
            IFXOS_MutexUnlock (pDev->memberAcc);
            TRACE(VINETIC,DBG_LEVEL_HIGH,
               ("\n\rDRV_ERROR: Tx Slot is not available\n\r"));
            ret = IFX_ERROR;
            SET_ERROR (VIN_ERR_NORESOURCE);
            goto end_fkt;
         }

         /* reserve the time slots now, because the parameter are OK */
         pDev->PcmRxTs[pPcmCfg->nHighway][IdxRx] |= BitValueRx;
         pDev->PcmTxTs[pPcmCfg->nHighway][IdxTx] |= BitValueTx;

         /* Fall through and do not 'break'!
         For 16-Bit PCM Mode, the next higher PCM time slot was reserved
         and now the used time slot will be reserved.
         That part is similar to the 8-Bit PCM Mode */
         /* break; */
         /*lint -fallthrough */
      case IFX_TAPI_PCM_RES_ALAW_8BIT:
      case IFX_TAPI_PCM_RES_ULAW_8BIT:
         /* get the index in the time slot array. Each bit reflects one time
         slot. Modulo 32 separate the index of the array and the bit inside
         the field. */
         IdxRx = pPcmCfg->nTimeslotRX >> 5;
         BitValueRx = 1 << (pPcmCfg->nTimeslotRX & 0x1F);
         IdxTx = pPcmCfg->nTimeslotTX >> 5;
         BitValueTx = 1 << (pPcmCfg->nTimeslotTX & 0x1F);

         /* Receive Slot checking */
         if (pDev->PcmRxTs[pPcmCfg->nHighway][IdxRx] & BitValueRx)
         {
            IFXOS_MutexUnlock (pDev->memberAcc);
            TRACE(VINETIC,DBG_LEVEL_HIGH,
               ("\n\rDRV_ERROR: Rx Slot is not available\n\r"));
            ret = IFX_ERROR;
            SET_ERROR (VIN_ERR_NORESOURCE);
            goto end_fkt;
         }

         if (pDev->PcmTxTs[pPcmCfg->nHighway][IdxTx] & BitValueTx)
         {
            IFXOS_MutexUnlock (pDev->memberAcc);
            TRACE(VINETIC,DBG_LEVEL_HIGH,
               ("\n\rDRV_ERROR: Tx Slot is not available\n\r"));
            ret = IFX_ERROR;
            SET_ERROR (VIN_ERR_NORESOURCE);
            goto end_fkt;
         }

         /* reserve the time slots now, because the parameter are OK */
         pDev->PcmRxTs[pPcmCfg->nHighway][IdxRx] |= BitValueRx;
         pDev->PcmTxTs[pPcmCfg->nHighway][IdxTx] |= BitValueTx;

         break;
      default:
         TRACE(VINETIC,DBG_LEVEL_HIGH,
            ("\n\rDRV_ERROR: This resolution is unknown\n\r"));
         IFXOS_MutexUnlock (pDev->memberAcc);
         ret = IFX_ERROR;
         SET_ERROR (VIN_ERR_NOTSUPPORTED);
         goto end_fkt;
      }
      /* activate Timeslots */
      switch (pPcmCfg->nResolution)
      {
      case  IFX_TAPI_PCM_RES_ALAW_8BIT:
         pPcmCh->bit.cod = 2;
         break;
      case IFX_TAPI_PCM_RES_ULAW_8BIT:
         pPcmCh->bit.cod = 3;
         break;
      case IFX_TAPI_PCM_RES_LINEAR_16BIT:
      default:
         pPcmCh->bit.cod = 0;
         break;
      }
      pPcmCh->bit.xts  = pPcmCfg->nTimeslotTX;
      pPcmCh->bit.rts  = pPcmCfg->nTimeslotRX;
      pPcmCh->bit.r_hw = pPcmCfg->nHighway;
      pPcmCh->bit.x_hw = pPcmCfg->nHighway;
      pPcmCh->bit.en = 1;

      ret = CmdWrite (pDev, pPcmCh->value, CMD_PCM_CH_LEN);
      IFXOS_MutexUnlock (pDev->memberAcc);
      break;
   default:
      ret = IFX_ERROR;
      goto end_fkt;
      /* break; */
   }

end_fkt:
   /* Check Error */
   if (ret == IFX_ERROR)
   {
      TRACE(VINETIC,DBG_LEVEL_HIGH,
          ("DRV_ERROR: Error in (de)activating PCM Module!\n\r"));
   }

   return ret;
}


/**
   Sets the PCM interface volume.
\param pChannel Handle to TAPI_CHANNEL structure
\param pVol     Handle to IFX_TAPI_LINE_VOLUME_t structure
\return Return value according to IFX_return_t
   - IFX_ERROR if an error occured
   - IFX_SUCCESS if successful
\remarks
   Gain Parameter are given in 'dB'. The range is -24dB ... 24dB.
*/
IFX_return_t IFX_TAPI_LL_PCM_Volume_Set (IFX_TAPI_LL_CH_t *pLLChannel,
                                         IFX_TAPI_LINE_VOLUME_t const *pVol)
{
   IFX_int32_t       ret;
   VINETIC_CHANNEL *pCh = (VINETIC_CHANNEL *) pLLChannel;
   VINETIC_DEVICE *pDev = (VINETIC_DEVICE*) (pCh->pParent);

   /* range check, because gain var is integer */
   if ((pVol->nGainTx < IFX_TAPI_LINE_VOLUME_MIN_GAIN) ||
       (pVol->nGainTx > IFX_TAPI_LINE_VOLUME_MAX_GAIN) ||
       (pVol->nGainRx < IFX_TAPI_LINE_VOLUME_MIN_GAIN) ||
       (pVol->nGainRx > IFX_TAPI_LINE_VOLUME_MAX_GAIN))
   {
      /* parameters are out of the supported range */
      TRACE(VINETIC, DBG_LEVEL_HIGH,
            ("\n\rDRV_ERROR: Volume Gain out of range for PCM, "
            "(Tx=%ddB, Rx=%ddB, allowed:%d..%ddB)!\n\r", pVol->nGainTx, pVol->nGainRx,
            IFX_TAPI_LINE_VOLUME_MIN_GAIN, IFX_TAPI_LINE_VOLUME_MAX_GAIN));
      return IFX_ERROR;
   }

   if ((pCh->nChannel-1) >= pCh->pParent->caps.nPCM)
   {
      /* no PCM module in this channel */
      SET_ERROR (VIN_ERR_FUNC_PARM);
      return IFX_ERROR;
   }

   /* protect fw msg */
   IFXOS_MutexLock (pDev->memberAcc);

   /* get actual settings into local var */
   pCh->pPCM->fw_pcm_ch.bit.gain_1 = (IFX_uint32_t)VINETIC_AlmPcmGain [pVol->nGainTx + 24];
   pCh->pPCM->fw_pcm_ch.bit.gain_2 = (IFX_uint32_t)VINETIC_AlmPcmGain [pVol->nGainRx + 24];
   pCh->pPCM->fw_pcm_ch.bit.en = 1;

   /* write local configuration if */
   ret = CmdWrite (pDev, pCh->pPCM->fw_pcm_ch.value, CMD_PCM_CH_LEN);
   /* release lock */
   IFXOS_MutexUnlock (pDev->memberAcc);

   return ret;
}


/**
   sets the LEC configuration on the PCM.
\param pChannel        Handle to TAPI_CHANNEL structure
\param pLecConf        Handle to IFX_TAPI_LEC_CFG_t structure
\return Return value according to IFX_return_t
   - IFX_ERROR if an error occured
   - IFX_SUCCESS if successful
\remarks
   This function shouldn't be called when a disabling of PCM Nelec can have a
   fatal impact on the ongoing voice/fax connection. It will be more suitable to
   call the function at initialization / configuration time.
*/
IFX_return_t IFX_TAPI_LL_PCM_Lec_Cfg (IFX_TAPI_LL_CH_t *pLLChannel,
                                      TAPI_LEC_DATA_t const *pLecConf)
{
#if (VIN_CFG_FEATURES & VIN_FEAT_VOICE)
   VINETIC_CHANNEL   *pCh     = (VINETIC_CHANNEL *) pLLChannel;
   IFX_int32_t       ret      = IFX_SUCCESS;
   IFX_boolean_t     bEnLec;
   IFX_boolean_t     bEnNlp;
   IFX_boolean_t     bEnWLec;

   /* Make sure that a PCM module exists on the given channel */
   if ((pCh->nChannel-1) >= pCh->pParent->caps.nPCM)
   {
      /* no PCM module in this channel */
      SET_ERROR (VIN_ERR_FUNC_PARM);
      return IFX_ERROR;
   }
   /* Make sure that the PCM channel is activated */
   if (pCh->pPCM->fw_pcm_ch.bit.en != 1)
   {
      /* PCM channel not activated */
      return IFX_ERROR;
   }

   switch (pLecConf->nOpMode)
   {
      case  IFX_TAPI_WLEC_TYPE_OFF:
         bEnLec  = IFX_FALSE;
         bEnWLec = IFX_FALSE;
         break;
      case  IFX_TAPI_WLEC_TYPE_NE:
         bEnLec  = IFX_TRUE;
         bEnWLec = IFX_FALSE;
         pCh->pPCM->lec_window_coefs [0] = pLecConf->nNBNEwindow;
         break;
      case  IFX_TAPI_WLEC_TYPE_NFE:
         bEnLec  = IFX_TRUE;
         bEnWLec = IFX_TRUE;
         pCh->pPCM->lec_window_coefs [1] = pLecConf->nNBNEwindow + 
                                           pLecConf->nNBFEwindow;
         pCh->pPCM->lec_window_coefs [2] = pLecConf->nNBNEwindow;
         break;
      default:
         SET_ERROR (VIN_ERR_FUNC_PARM);
         return IFX_ERROR;
   }

   bEnNlp = (pLecConf->bNlp == IFX_TAPI_LEC_NLP_OFF) ? IFX_FALSE : IFX_TRUE;

   ret = vinetic_pcm_Lec_Cfg(pCh, bEnLec, bEnNlp, bEnWLec);

   return (ret == IFX_SUCCESS) ? IFX_SUCCESS : IFX_ERROR;
#else
   return IFX_ERROR;
#endif /* (VIN_CFG_FEATURES & VIN_FEAT_VOICE) */
}


/**
   Prepare parameters and call the target configuration function to configure
   the PCM interface.
\param pChannel Handle to TAPI_CHANNEL structure
\param pPCMConfig Contains the new configuration for PCM interface
\return Return value according to IFX_return_t
   - IFX_ERROR if an error occured
   - IFX_SUCCESS if successful
\remarks
   Performs error checking according to the underlying device capability.
*/
IFX_return_t IFX_TAPI_LL_PCM_Cfg (IFX_TAPI_LL_CH_t *pLLChannel,
                                      IFX_TAPI_PCM_CFG_t const *pPCMConfig)
{
   IFX_int32_t       ret = IFX_SUCCESS;
   VINETIC_CHANNEL *pCh = (VINETIC_CHANNEL *) pLLChannel;
   VINETIC_DEVICE *pDev = (VINETIC_DEVICE*) (pCh->pParent);

   if ((pDev->nDevState & DS_PCM_EN) == IFX_FALSE)
   {
      TRACE(VINETIC,DBG_LEVEL_HIGH,
           ("DRV_ERROR: PCM Module not enabled during Initialization!\n\r"));
      ret = IFX_ERROR;
   }

   if (ret == IFX_SUCCESS)
   {
      /* check PCM resolution for supported values */
      switch (pPCMConfig->nResolution)
      {
      case IFX_TAPI_PCM_RES_ALAW_8BIT:
      case IFX_TAPI_PCM_RES_ULAW_8BIT:
      case IFX_TAPI_PCM_RES_LINEAR_16BIT:
         break;
      default:
         ret = IFX_ERROR;
         break;
      }
   }

   if ((pPCMConfig->nTimeslotRX > PCM_MAX_TS) ||
      (pPCMConfig->nTimeslotTX > PCM_MAX_TS))
   {
      TRACE (VINETIC,DBG_LEVEL_HIGH,
         ("\n\rDRV_ERROR: Number of PCM timeslot out of range\n\r"));
      ret = IFX_ERROR;
   }

   if (pPCMConfig->nHighway >= PCM_HIGHWAY)
   {
      TRACE (VINETIC,DBG_LEVEL_HIGH,
         ("\n\rDRV_ERROR: Number of PCM Highway %d out of range\n\r",
         (int)pPCMConfig->nHighway));
      ret = IFX_ERROR;
   }

   return ret;
}


/**
   Switches on/off the HP filter in the decoder path of the PCM module

\param pLLChannel Handle to TAPI low level channel structure
\param bHp        IFX_FALSE to switch HP filter off
                  IFX_TRUE  to switch HP filter on
\return
   IFX_ERROR on success, IFX_ERROR on error
*/
IFX_return_t IFX_TAPI_LL_PCM_DEC_HP_Set (IFX_TAPI_LL_CH_t *pLLChannel,
                                         IFX_boolean_t bHp)
{
   VINETIC_CHANNEL *pCh = (VINETIC_CHANNEL *)pLLChannel;
   VINETIC_DEVICE *pDev = (VINETIC_DEVICE*) (pCh->pParent);
   FWM_PCM_CH *p_fw_pcm_ch = &pCh->pPCM->fw_pcm_ch;
   IFX_int32_t ret = IFX_SUCCESS;
   unsigned newHP = (bHp == IFX_TRUE) ? 1 : 0;

   /* protect fwmsg access against concurrent tasks */
   IFXOS_MutexLock (pDev->memberAcc);

   if (p_fw_pcm_ch->bit.hp != newHP)
   {
      /* store new HP setting */
      p_fw_pcm_ch->bit.hp = newHP;

      /* if decoder is currently running write new setting to fw */
      if (p_fw_pcm_ch->bit.en != 0)
      {
         ret = CmdWrite (pDev, (IFX_uint16_t *) p_fw_pcm_ch, 2);
      }
   }

   /* unlock protection */
   IFXOS_MutexUnlock (pDev->memberAcc);

   return ret;
}


/**
   base PCM Module configuration

   \param   VINETIC_CHANNEL*   Pointer to the VINETIC channel structure
   \return  IFX_SUCCESS if no error, otherwise IFX_ERROR
*/
IFX_int32_t VINETIC_PCM_baseConf (VINETIC_CHANNEL *pCh)
{
   IFX_int32_t  ret;

   /* Set the inputs in the cached message and write it */
   ret = VINETIC_PCM_Set_Inputs(pCh);

   return ret;
}


/**
   Initalize the PCM module and the cached firmware messages and variables

   \param   VINETIC_CHANNEL*   Pointer to the VINETIC channel structure
   \param   pcmCh              The PCM channel resource to use
   \return  none
*/
IFX_void_t VINETIC_PCM_Init_Ch (VINETIC_CHANNEL *pCh, IFX_uint8_t pcmCh)
{
   VINETIC_PCMCH_t *pPCM = pCh->pPCM;
   IFX_uint8_t     ch    = pCh->nChannel - 1;

   TRACE(VINETIC, DBG_LEVEL_LOW, ("INFO: VINETIC_PCM_Init_Ch called\n\r"));

   VINETIC_CON_Init_PcmCh (pCh, pcmCh);

   /* PCM CH message */
   memset (pPCM->fw_pcm_ch.value, 0, sizeof(FWM_PCM_CH));
   pPCM->fw_pcm_ch.value[0]   = CMD1_EOP | pcmCh;
   pPCM->fw_pcm_ch.value[1]   = ECMD_PCM_CH;
   pPCM->fw_pcm_ch.bit.en     = 0;
   pPCM->fw_pcm_ch.bit.gain_1 = PCM_GAIN_0DB;
   pPCM->fw_pcm_ch.bit.gain_2 = PCM_GAIN_0DB;
   pPCM->fw_pcm_ch.bit.cod    = 2; /* G711- ALaw */
   pPCM->fw_pcm_ch.bit.codnr  = ch;
   pPCM->fw_pcm_ch.bit.hp     = 1;

   /* initially no LEC is assigned to this channel */
   pPCM->pcm_nelec = 0;

   /* defaults for LEC window sizes in different modes */
   pPCM->lec_window_coefs[0] = 16; /* NB NLEC LEN (ms) and FIX_WIN_LEN (ms) */
   pPCM->lec_window_coefs[1] = 16; /* NB WLEC LEN (ms) */
   pPCM->lec_window_coefs[2] = 8;  /* NB WLEC FIX_WIN_LEN (ms) */
}


/**
   Set the signal inputs of the cached fw message for the given channel

   \param   VINETIC_CHANNEL*   Pointer to the VINETIC channel structure
   \return  IFX_SUCCESS or IFX_ERROR
*/
IFX_return_t VINETIC_PCM_Set_Inputs (VINETIC_CHANNEL *pCh)
{
   FWM_PCM_CH        *p_fw_pcm_ch;
   IFX_return_t      ret = IFX_SUCCESS;

   /* update the signal inputs of this cached msg */
   p_fw_pcm_ch = &pCh->pPCM->fw_pcm_ch;

   IFXOS_MutexLock (pCh->chAcc);
   p_fw_pcm_ch->bit.i1 = VINETIC_CON_Get_PCM_SignalInput (pCh, 0);
   p_fw_pcm_ch->bit.i2 = VINETIC_CON_Get_PCM_SignalInput (pCh, 1);
   p_fw_pcm_ch->bit.i3 = VINETIC_CON_Get_PCM_SignalInput (pCh, 2);
   p_fw_pcm_ch->bit.i4 = VINETIC_CON_Get_PCM_SignalInput (pCh, 3);
   p_fw_pcm_ch->bit.i5 = VINETIC_CON_Get_PCM_SignalInput (pCh, 4);

   ret = CmdWrite (pCh->pParent, p_fw_pcm_ch->value, CMD_PCM_CH_LEN);

   IFXOS_MutexUnlock (pCh->chAcc);

   return ret;
}


/**
  Allocate data structures of the PCM module in the given channel

  \param   VINETIC_CHANNEL*   Pointer to the VINETIC channel structure
  \return  IFX_SUCCESS or IFX_ERROR in case the stucture could not be created
  \remarks The channel parameter is not checked because the calling
           function assures correct values.
*/
IFX_return_t VINETIC_PCM_Allocate_Ch_Structures (VINETIC_CHANNEL *pCh)
{
   VINETIC_PCM_Free_Ch_Structures (pCh);

   pCh->pPCM = IFXOS_MALLOC(sizeof(VINETIC_PCMCH_t));
   if (pCh->pPCM == NULL)
   {
      return IFX_ERROR;
   }
   memset(pCh->pPCM, 0, sizeof(VINETIC_PCMCH_t));

   return IFX_SUCCESS;
}


/**
   Free data structures of the PCM module in the given channel

   \param   VINETIC_CHANNEL*   Pointer to the VINETIC channel structure
   \return  none
*/
IFX_void_t VINETIC_PCM_Free_Ch_Structures (VINETIC_CHANNEL *pCh)
{
   if (pCh->pPCM != IFX_NULL)
   {
      IFXOS_FREE(pCh->pPCM);
   }
}


/**
   Function called by the init_module of device, fills up PCM module
   function pointers which are passed to HL TAPI during registration

   \param   pPCM   handle to the pcm channel driver context structure
   \return  none
*/
IFX_void_t VINETIC_PCM_Func_Register (IFX_TAPI_DRV_CTX_PCM_t *pPCM)
{
   pPCM->Enable         = IFX_TAPI_LL_PCM_Enable;
   pPCM->Cfg            = IFX_TAPI_LL_PCM_Cfg;
   pPCM->Lec_Cfg        = IFX_TAPI_LL_PCM_Lec_Cfg;
   pPCM->Volume_Set     = IFX_TAPI_LL_PCM_Volume_Set;
   pPCM->DEC_HP_Set     = IFX_TAPI_LL_PCM_DEC_HP_Set;
}
