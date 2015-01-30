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
   Module      : drv_vinetic_alm.c
   Description : This file implements the ALM module
******************************************************************************/

/* ============================= */
/* Includes                      */
/* ============================= */
#include "ifx_types.h"
#include "drv_vinetic_alm_priv.h"
#include "drv_vinetic_alm.h"
#include "drv_vinetic_con.h"

/* ============================= */
/* Global variables              */
/* ============================= */
/* calculated table to set the Rx/Tx Gain in the ALM module.
   Converts the gain in 'dB' into the regiter values */
/* This table is shared with the PCM module */
const  IFX_uint8_t VINETIC_AlmPcmGain [] =
{
   /* Gain Byte Calculation Table in 'dB'
      See also VINETIC User's Manual Table 15 Rev 1.0 */
   /* -24   -23   -22   -21   -20   -19   -18   -17   -16   -15   */
      224,  228,  233,  238,  243,  249,  192,  196,  201,  206,
   /* -14   -13   -12   -11   -10   -9    -8    -7    -6    -5    */
      211,  217,  160,  164,  168,  173,  179,  185,  128,  132,
   /* -4    -3    -2    -1    0     1     2     3     4     5     */
      136,  141,  147,  153,  96,   100,  104,  109,  115,  121,
   /* 6     7     8     9     10    11    12    13    14    15    */
      64,   68,   72,   77,   83,   89,   32,   36,   40,   45,
   /* 16    17    18    19    20    21    22    23    24          */
      50,   57,   0,    4,    8,    13,   18,   25,   31
};

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
static IFX_int32_t vinetic_alm_Lec_Cfg (VINETIC_CHANNEL *pCh, 
                                        IFX_boolean_t bEnLec,
                                        IFX_boolean_t bEnNlp, 
                                        IFX_boolean_t bEnWLec);

/* ============================= */
/* Function definitions          */
/* ============================= */

#if (VIN_CFG_FEATURES & VIN_FEAT_VOICE)
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
   Configures the Near LEC in the analog module
   \param   pCh        - pointer to VINETIC channel structure
   \param   bEnLec     - IFX_FALSE : disable / IFX_TRUE : enable LEC
   \param   bEnNlp     - IFX_FALSE : disable / IFX_TRUE : enable NLP
   \param   bEnWLec    - IFX_FALSE : disable / IFX_TRUE : enable WLEC
                         implicitly enables LEC
   \return
      IFX_SUCCESS or IFX_ERROR
   \remarks
      This function does only handle one pool of LEC resources. So when WLEC
      is available it is assumed that all LEC are WLEC. When WLEC is not
      available all LEC will be treated as NLEC.
      The counter for NLEC must be set to the number of avilable WLEC when
      WLECs are available.
*/
static IFX_int32_t vinetic_alm_Lec_Cfg(VINETIC_CHANNEL *pCh,
                                    IFX_boolean_t bEnLec,
                                    IFX_boolean_t bEnNlp,
                                    IFX_boolean_t bEnWLec)
{
   VINETIC_DEVICE *pDev = pCh->pParent;
   IFX_uint16_t    ch   = (pCh->nChannel - 1);
   IFX_uint16_t    pCmd1[3];
   IFX_uint16_t    pCmd2[11];
   IFX_uint8_t     lec_res;

   /* coefficients for NLEC (can be used for all firmware versions) */
   static const IFX_uint16_t alm_lec_nlec_coefs[5] =
   {
      ECMD_NELEC_COEFS, 0x1048, 0x1808, 0x6060, 0x0060
   };

   /* coefficients for WLEC (can only be used for firmware with WLEC ) */
   static const IFX_uint16_t alm_lec_wlec_coefs[7] =
   {
      ECMD_NELEC_COEFS, 0x1048, 0x1808, 0x6060, 0x0060, 0x0834, 0x0008
   };

   /* coefficients for Non Linear Processing */
   static const IFX_uint16_t alm_lec_nlp_coefs[10] =
   {
      ECMD_NELEC_NLP, 0x1502, 0x4540, 0x0C40, 0x41FE,
      0x1106, 0x5050, 0x0D10, 0x6480, 0x0810
   };

   /* ---------------------------------------------------- */

   /* if WLEC should be used, check for WLEC capability */
   if ( (bEnWLec == IFX_TRUE) && (pDev->caps.nWLEC == 0) )
   {
      /* WLEC is not available => error code VIN_ERR_FUNC_PARM */
      SET_ERROR (VIN_ERR_FUNC_PARM);
      return IFX_ERROR;
   }
   /* this should have been checked before but do it anyhow */
   if (ch >= pDev->caps.nALI)
   {
      /* this is not an analog channel */
      SET_ERROR (VIN_ERR_FUNC_PARM);
      return IFX_ERROR;
   }
   /* set channel */
   pCmd1 [0] = CMD1_EOP | ch;

   if ((bEnLec == IFX_FALSE) && (bEnWLec == IFX_FALSE))
   {
      /* disable LEC */
      if (pCh->pALM->ali_nelec & ALM_NLEC_EN)
      {
         /* release LEC resource back to the minimal resource management. */
         pDev->availLecRes |= (1 << (pCh->pALM->ali_nelec & ALM_LECNR));
         /* clear LEC enable bits in cached command and keep everything else */
         pCh->pALM->ali_nelec &= ~(ALM_NLEC_EN | ALM_NLP_EN);
         /* set and write ALI NELEC command */
         pCmd1 [1] = ECMD_ALM_NEARLEC;
         pCmd1 [2] = pCh->pALM->ali_nelec;
         return CmdWrite (pDev, pCmd1, 1);
      }
   }
   else
   {
      /* configure and enable LEC */

      if (!(pCh->pALM->ali_nelec & ALM_NLEC_EN))
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
         pCh->pALM->ali_nelec = 0;
         /* set channel in lec command */
         pCmd1 [2] = (ALM_NLEC_EN | lec_res);
         /* set channel in lec coefficient command */
         pCmd2 [0] = (CMD1_EOP | lec_res);
         /* set NLP coefficients only here where LEC is still unconfigured  */
         /* coefficients are set independently of the real activation later */
         memcpy(&pCmd2[1], alm_lec_nlp_coefs, sizeof(alm_lec_nlp_coefs));
         if (CmdWrite (pDev, pCmd2, 9) != IFX_SUCCESS)
            return IFX_ERROR;
      }
      else
      {
         /* No LEC resource is required, because LEC is already running */
         /* load command word from cache */
         pCmd1 [2] = pCh->pALM->ali_nelec;
         /* get the LEC resource which is used for this channel */
         lec_res = pCh->pALM->ali_nelec & ALM_LECNR;

         /* cross check with capabilities */
         if (lec_res >= pDev->caps.nNLEC)
            return IFX_ERROR;
         /* set channel in lec coefficient command */
         pCmd2 [0] = CMD1_EOP | lec_res;
      }

      /* Set the WLEC/NLEC coefficients */
      if ( bEnWLec == IFX_TRUE )
      {
         /* set WLEC coefficients defaults for ALM
         WLEC Configuration: Normal operation mode for the window based LEC.
         The window position as well as the filter coefficients are adapted by the LEC.
         OLDP = 0, OLDC = 0
         DCF = 1,
         MW = 1, The de-correlation filter is part of the new WLEC only
         and should be activated. */
         pCmd1 [2] &= ~(ALM_OLDC_EN | ALM_OLDP);
         pCmd1 [2] |=  (ALM_MW_EN | ALM_DCF_EN);
         /* set WLEC coefficients defaults for ALM */
         memcpy(&pCmd2[1], alm_lec_wlec_coefs, sizeof(alm_lec_wlec_coefs));
         /* Overwrite window size parameters */
         pCmd2[2] = ((IFX_uint16_t)(pCh->pALM->lec_window_coefs [1]) << 8) |
                    (pCmd2[2] & 0xFF);
         pCmd2[6] = ((IFX_uint16_t)(pCh->pALM->lec_window_coefs [2]) << 8) |
                    (pCmd2[6] & 0xFF);
         /* write LEC coefficients command to ALM */
         if (CmdWrite (pDev, pCmd2, 6) != IFX_SUCCESS)
            return IFX_ERROR;
      }
      else
      {
         /* set NLEC coefficients defaults for ALM */
         pCmd1 [2] &= ~(PCM_OLDC_EN | PCM_OLDP | PCM_MW_EN | PCM_DCF_EN);
         memcpy(&pCmd2[1], alm_lec_nlec_coefs, sizeof(alm_lec_nlec_coefs));
         /* Overwrite window size parameters */
         pCmd2[2] = ((IFX_uint16_t)(pCh->pALM->lec_window_coefs [0]) << 8) |
                    (pCmd2[2] & 0xFF);
         pCmd2[6] = ((IFX_uint16_t)(pCh->pALM->lec_window_coefs [0]) << 8) |
                    (pCmd2[6] & 0xFF);
         /* write LEC coefficients command to ALM */
         if (CmdWrite (pDev, pCmd2, 4) != IFX_SUCCESS)
            return IFX_ERROR;
      }

      /* Set the enable bit for NLP according to callers wishes */
      if (bEnNlp == IFX_FALSE)
         pCmd1 [2] &= ~ALM_NLP_EN;
      else
         pCmd1 [2] |= ALM_NLP_EN;

      /* only write to the device if the command word has changed */
      if (pCmd1 [2] != pCh->pALM->ali_nelec)
      {
         /* set ALI NELEC command */
         pCmd1 [1] = ECMD_ALM_NEARLEC;
         if (CmdWrite (pDev, pCmd1, 1) != IFX_SUCCESS)
            return IFX_ERROR;
         pCh->pALM->ali_nelec = pCmd1 [2];
      }
   }

   return IFX_SUCCESS;
}


/**
   sets the LEC configuration on the ALM module.
\param pChannel        Handle to TAPI_CHANNEL structure
\param pLecConf        Handle to IFX_TAPI_LEC_CFG_t structure
\return Return value according to IFX_return_t
   - IFX_ERROR if an error occured
   - IFX_SUCCESS if successful
\remarks
   This function shouldn't be called when a disabling of ALM Nelec can have a
   fatal impact on the ongoing voice/fax connection. It will be more suitable to
   call the function at initialization / configuration time.
*/
IFX_int32_t IFX_TAPI_LL_ALM_Lec_Cfg (IFX_TAPI_LL_CH_t *pLLChannel,
                                     TAPI_LEC_DATA_t const *pLecConf)
{
   VINETIC_CHANNEL *pCh      = (VINETIC_CHANNEL *) pLLChannel;
   IFX_int32_t      ret      = IFX_SUCCESS;
   IFX_boolean_t    bEnLec;
   IFX_boolean_t    bEnNlp;
   IFX_boolean_t    bEnWLec;

   if ((pCh->nChannel-1) >= pCh->pParent->caps.nALI)
   {
      /* no ALM module in this channel */
      SET_ERROR (VIN_ERR_FUNC_PARM);
      return IFX_ERROR;
   }

   switch (pLecConf->nOpMode)
   {
      case IFX_TAPI_WLEC_TYPE_OFF:
         bEnLec  = IFX_FALSE;
         bEnWLec = IFX_FALSE;
         break;
      case IFX_TAPI_WLEC_TYPE_NE:
         bEnLec  = IFX_TRUE;
         bEnWLec = IFX_FALSE;
         pCh->pALM->lec_window_coefs [0] = pLecConf->nNBNEwindow;
         break;
      case IFX_TAPI_WLEC_TYPE_NFE:
         bEnLec  = IFX_TRUE;
         bEnWLec = IFX_TRUE;
         pCh->pALM->lec_window_coefs [1] = pLecConf->nNBNEwindow + 
                                           pLecConf->nNBFEwindow;
         pCh->pALM->lec_window_coefs [2] = pLecConf->nNBNEwindow;
         break;
      default:
         SET_ERROR (VIN_ERR_FUNC_PARM);
         return IFX_ERROR;
   }

   bEnNlp = (pLecConf->bNlp == IFX_TAPI_LEC_NLP_OFF) ? IFX_FALSE : IFX_TRUE;

   ret = vinetic_alm_Lec_Cfg(pCh, bEnLec, bEnNlp, bEnWLec);

   return (ret == IFX_SUCCESS) ? IFX_SUCCESS : IFX_ERROR;
}
#endif /* (VIN_CFG_FEATURES & VIN_FEAT_VOICE) */


/**
   Set the phone volume

   \param   pChannel   Handle to TAPI_CHANNEL structure
   \param   pVol       Handle to IFX_TAPI_LINE_VOLUME_t structure
   \return  Return value according to IFX_return_t
            - IFX_ERROR if an error occured
            - IFX_SUCCESS if successful
   \remarks
   Gain Parameters are given in 'dB'. The range is -24dB ... 24dB.
*/
IFX_int32_t IFX_TAPI_LL_ALM_Volume_Set (IFX_TAPI_LL_CH_t *pLLChannel,
                                         IFX_TAPI_LINE_VOLUME_t const *pVol)
{
   VINETIC_CHANNEL *pCh = (VINETIC_CHANNEL *) pLLChannel;
   VINETIC_DEVICE *pDev = (VINETIC_DEVICE*) (pCh->pParent);
   IFX_int32_t    ret;

   /* range check, cos gain var is integer */
   if ((pVol->nGainTx < IFX_TAPI_LINE_VOLUME_MIN_GAIN) ||
       (pVol->nGainTx > IFX_TAPI_LINE_VOLUME_MAX_GAIN) ||
       (pVol->nGainRx < IFX_TAPI_LINE_VOLUME_MIN_GAIN) ||
       (pVol->nGainRx > IFX_TAPI_LINE_VOLUME_MAX_GAIN))
   {
      /* parameters are out of the supported range */
      TRACE(VINETIC, DBG_LEVEL_HIGH,
            ("\n\rDRV_ERROR: Volume Gain out of range for ALM, "
            "(Tx=%ddB, Rx=%ddB, allowed:%d..%ddB)!\n\r", pVol->nGainTx, pVol->nGainRx,
            IFX_TAPI_LINE_VOLUME_MIN_GAIN, IFX_TAPI_LINE_VOLUME_MAX_GAIN));
      return IFX_ERROR;
   }

   if ((pCh->nChannel-1) >= pCh->pParent->caps.nALI)
   {
      /* no ALM module in this channel */
      SET_ERROR (VIN_ERR_FUNC_PARM);
      return IFX_ERROR;
   }

   /* protect fw msg */
   IFXOS_MutexLock (pDev->memberAcc);

   /* get actual settings into local var */
   pCh->pALM->fw_ali_ch.bit.gain_x =
                     (IFX_uint32_t)VINETIC_AlmPcmGain [pVol->nGainTx + 24];
   pCh->pALM->fw_ali_ch.bit.gain_r =
                     (IFX_uint32_t)VINETIC_AlmPcmGain [pVol->nGainRx + 24];
   pCh->pALM->fw_ali_ch.bit.en = 1;

   /* write local configuration if */
   ret = CmdWrite (pDev, pCh->pALM->fw_ali_ch.value, CMD_ALM_CH_LEN);
   /* release lock */
   IFXOS_MutexUnlock (pDev->memberAcc);

   return ret;
}


/**
   Base ALM configuration

   \param   VINETIC_CHANNEL*   Pointer to the VINETIC channel structure
   \return  IFX_SUCCESS if no error, otherwise IFX_ERROR
*/
IFX_int32_t VINETIC_ALM_baseConf (VINETIC_CHANNEL *pCh)
{
   IFX_int32_t  ret;

   /* Set the inputs in the cached message and write it */
   ret = VINETIC_ALM_Set_Inputs(pCh);

   return ret;
}


/**
   Initalize the analog module and the cached firmware messages and variables

   \param   VINETIC_CHANNEL*   Pointer to the VINETIC channel structure
   \return  none
*/
IFX_void_t VINETIC_ALM_Init_Ch (VINETIC_CHANNEL *pCh)
{
   VINETIC_ALMCH_t *pALM = pCh->pALM;
   IFX_uint8_t     ch    = pCh->nChannel - 1;

   TRACE(VINETIC, DBG_LEVEL_LOW, ("INFO: VINETIC_ALM_Init_Ch called\n\r"));

   VINETIC_CON_Init_AlmCh (pCh);

   /* ALI CH message */
   memset (pALM->fw_ali_ch.value, 0, sizeof(FWM_ALM_CH));
   pALM->fw_ali_ch.value[0]   = CMD1_EOP | ch;
   pALM->fw_ali_ch.value[1]   = ECMD_ALM_CH;
   pALM->fw_ali_ch.bit.en     = 1;
   pALM->fw_ali_ch.bit.gain_r = ALM_GAIN_0DB;
   pALM->fw_ali_ch.bit.gain_x = ALM_GAIN_0DB;

   /* initially no LEC is assigned to this channel */
   pALM->ali_nelec = 0;

   /* defaults for LEC window sizes in different modes */
   pALM->lec_window_coefs[0] = 16; /* NB NLEC LEN (ms) and FIX_WIN_LEN (ms) */
   pALM->lec_window_coefs[1] = 16; /* NB WLEC LEN (ms) */
   pALM->lec_window_coefs[2] = 8;  /* NB WLEC FIX_WIN_LEN (ms) */
}


/**
   Set the signal inputs of the cached fw message for the given channel

   \param   VINETIC_CHANNEL*   Pointer to the VINETIC channel structure
   \return  IFX_SUCCESS or IFX_ERROR
*/
IFX_return_t VINETIC_ALM_Set_Inputs (VINETIC_CHANNEL *pCh)
{
   FWM_ALM_CH        *p_fw_ali_ch;
   IFX_return_t      ret = IFX_SUCCESS;

   /* update the signal inputs of this cached msg */
   p_fw_ali_ch = &pCh->pALM->fw_ali_ch;

   IFXOS_MutexLock (pCh->chAcc);
   p_fw_ali_ch->bit.i1 = VINETIC_CON_Get_ALM_SignalInput (pCh, 0);
   p_fw_ali_ch->bit.i2 = VINETIC_CON_Get_ALM_SignalInput (pCh, 1);
   p_fw_ali_ch->bit.i3 = VINETIC_CON_Get_ALM_SignalInput (pCh, 2);
   p_fw_ali_ch->bit.i4 = VINETIC_CON_Get_ALM_SignalInput (pCh, 3);
   p_fw_ali_ch->bit.i5 = VINETIC_CON_Get_ALM_SignalInput (pCh, 4);

   ret = CmdWrite (pCh->pParent, p_fw_ali_ch->value, CMD_ALM_CH_LEN);

   IFXOS_MutexUnlock (pCh->chAcc);

   return ret;
}


/**
  Allocate data structures of the ALM module in the given channel

  \param   VINETIC_CHANNEL*   Pointer to the VINETIC channel structure
  \return  IFX_SUCCESS or IFX_ERROR in case the stucture could not be created
  \remarks The channel parameter is not checked because the calling
           function assures correct values.
*/
IFX_return_t VINETIC_ALM_Allocate_Ch_Structures (VINETIC_CHANNEL *pCh)
{
   VINETIC_ALM_Free_Ch_Structures (pCh);

   pCh->pALM = IFXOS_MALLOC(sizeof(VINETIC_ALMCH_t));
   if (pCh->pALM == IFX_NULL)
   {
      return IFX_ERROR;
   }
   memset(pCh->pALM, 0, sizeof(VINETIC_ALMCH_t));

   return IFX_SUCCESS;
}


/**
   Free data structures of the ALM module in the given channel

   \param   VINETIC_CHANNEL*   Pointer to the VINETIC channel structure
   \return  none
*/
IFX_void_t VINETIC_ALM_Free_Ch_Structures (VINETIC_CHANNEL *pCh)
{
   if (pCh->pALM != IFX_NULL)
   {
      IFXOS_FREE(pCh->pALM);
   }
}


/**
   Function called by the init_module of device, fills up ALM module
   function pointers which are passed to HL TAPI during registration

   \param   pAlm   handle to the analog channel driver context structure
   \return  none
*/
void VINETIC_ALM_Func_Register (IFX_TAPI_DRV_CTX_ALM_t *pAlm)
{
   /* Fill the function pointers that are exported */
#if (VIN_CFG_FEATURES & VIN_FEAT_VOICE)
   pAlm->Lec_Cfg     = IFX_TAPI_LL_ALM_Lec_Cfg;
#endif /* (VIN_CFG_FEATURES & VIN_FEAT_VOICE) */
   pAlm->Volume_Set  = IFX_TAPI_LL_ALM_Volume_Set;
}
