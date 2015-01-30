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
   Module      : drv_vinetic_cod_aal.c
   Description : This file contains the implementation of the functions
                 for AAL2 operations
******************************************************************************/

/* ============================= */
/* Includes                      */
/* ============================= */
#include "drv_vinetic_cod_priv.h"
#include "drv_vinetic_api.h"
#include "drv_vinetic_cod_aal.h"

/* ============================= */
/* Local Macros & Definitions    */
/* ============================= */


/* ============================= */
/* Global variable definition    */
/* ============================= */

/* ============================= */
/* Global function declaration   */
/* ============================= */

/* ============================= */
/* Local function declaration    */
/* ============================= */


/* ============================= */
/* Local variable definition     */
/* ============================= */

/* ============================= */
/* Local function definition     */
/* ============================= */
#if (VIN_CFG_FEATURES & VIN_FEAT_PACKET_AAL)
/**
   Programs an AAL profile
\param pChannel        Handle to TAPI_CHANNEL structure
\param pProfile        Handle to IFX_TAPI_PCK_AAL_PROFILE_t structure
\return Return value according to IFX_return_t
   - IFX_ERROR if an error occured
   - IFX_SUCCESS if successful
\remarks
   none
*/
IFX_return_t IFX_TAPI_LL_COD_AAL_Profile_Cfg (IFX_TAPI_LL_CH_t *pLLChannel,
                                      IFX_TAPI_PCK_AAL_PROFILE_t const *pProfile)
{
   IFX_int32_t      ret  = IFX_SUCCESS, i, nEnc, nMax = 0;
   VINETIC_CHANNEL *pCh = (VINETIC_CHANNEL *)pLLChannel;
   VINETIC_DEVICE *pDev = (VINETIC_DEVICE*) (pCh->pParent);
   IFX_uint8_t ch = pCh->nChannel - 1;
   IFX_uint16_t     pCmd [2], pData[14], nUUI;

   if (pProfile->rows < 1 || pProfile->rows > 10)
      return IFX_ERROR;

   memset (pData, 0, sizeof(IFX_uint16_t) * 14);
   pCmd [0] = (CMD1_EOP | ch);
   pCmd [1] = ECMD_COD_CHAAL | 2;
   /* read first both entries */
   ret = CmdRead (pDev, pCmd, pData, 2);
   for (i = 0; i < pProfile->rows; i++)
   {
#if (VIN_CFG_FEATURES & VIN_FEAT_VOICE)
      nEnc = VINETIC_COD_trans_cod_tapi2fw (pProfile->codec[i]);
#endif
      if (nEnc == IFX_ERROR)
         return IFX_ERROR;
      switch (pProfile->nUUI[i])
      {
         case IFX_TAPI_PKT_AAL_PROFILE_RANGE_12_15:
         case IFX_TAPI_PKT_AAL_PROFILE_RANGE_8_11:
         case IFX_TAPI_PKT_AAL_PROFILE_RANGE_4_7:
         case IFX_TAPI_PKT_AAL_PROFILE_RANGE_0_3:
            nMax = 4;
         break;
         case IFX_TAPI_PKT_AAL_PROFILE_RANGE_0_7:
         case IFX_TAPI_PKT_AAL_PROFILE_RANGE_8_15:
            nMax = 1;
         break;
         default:
            nMax = 0;
            nUUI = 0;
         break;
      }

      switch (pProfile->nUUI[i])
      {
         case IFX_TAPI_PKT_AAL_PROFILE_RANGE_12_15:
            nUUI = 3 * 4;
         break;
         case IFX_TAPI_PKT_AAL_PROFILE_RANGE_8_11:
            nUUI = 2 * 4;
         break;
         case IFX_TAPI_PKT_AAL_PROFILE_RANGE_4_7:
            nUUI = 4;
         break;
         case IFX_TAPI_PKT_AAL_PROFILE_RANGE_8_15:
            nUUI = 8;
         break;
         default:
            nUUI = 0;
         break;
      }
      pData[i + 4] = (IFX_uint16_t)((IFX_uint8_t)pProfile->len[i] << 10) |
                     (IFX_uint16_t)(nUUI << 6) |
                     (IFX_uint16_t)((IFX_uint8_t)nEnc);
   }
   /* clear the UUIS field and retain SQNR-INTV */
   pData[3] &= ~COD_CHAAL_UUIS;
   /* set the UUI subrange parameter according to the maximum ranges */
   switch (nMax)
   {
      case  0:
         pData[3] |= COD_CHAAL_UUI_1RANGE;
      break;
      case  1:
         pData[3] |= COD_CHAAL_UUI_2RANGES;
      break;
      case  4:
         pData[3] |= COD_CHAAL_UUI_4RANGES;
      break;
      case  8:
         pData[3] |= COD_CHAAL_UUI_8RANGES;
      break;
      case  16:
         pData[3] |= COD_CHAAL_UUI_16RANGES;
      break;
      default:
         SET_ERROR (VIN_ERR_INVALID);
         return IFX_ERROR;
   }
   ret = CmdWrite (pDev, pData, 12);
   return ret;
}

/**
   configures AAL fields for a new connection
\param pChannel          Handle to TAPI_CHANNEL structure
\param pAalConf          Handle to IFX_TAPI_PCK_AAL_CFG_t structure
\return Return value according to IFX_return_t
   - IFX_ERROR if an error occured
   - IFX_SUCCESS if successful
\remarks
   The coder channel has to be disabled before the AAL channel configuration
   can be done. The user task should make sure it activate the coder channel
   after having set this configuration.
*/
IFX_return_t IFX_TAPI_LL_COD_AAL_Cfg (IFX_TAPI_LL_CH_t  *pLLChannel,
                                   IFX_TAPI_PCK_AAL_CFG_t const *pAalConf)
{

   VINETIC_CHANNEL *pCh = (VINETIC_CHANNEL *)pLLChannel;
   VINETIC_DEVICE *pDev = (VINETIC_DEVICE*) (pCh->pParent);
   IFX_uint8_t ch = pCh->nChannel - 1;
   IFX_uint16_t    pCmd [3] = {0}, pData [3] = {0};
   IFX_int32_t     ret      = IFX_SUCCESS;

   /* set cmd 1 */
   pCmd [0] = (CMD1_EOP | ch);

   /* write aal start timestamp */
   pCmd [1] = ECMD_COD_AALCONF;
   pCmd [2] = pAalConf->nTimestamp;
   ret = CmdWrite (pDev, pCmd, 1);
   /* read actual aal coder channel configuration */
   if (ret == IFX_SUCCESS)
   {
      pCmd [1] = ECMD_COD_CHAAL;
      ret = CmdRead (pDev, pCmd, pData, 1);
   }
   /* disable coder channel */
   if (ret == IFX_SUCCESS)
      ret = VINETIC_COD_Voice_Enable (pCh, 0);
   /* set connection Id : This has to be done with disabled coder channel */
   if (ret == IFX_SUCCESS)
   {
      pData [2] &= ~COD_CHAAL_CID;
      pData [2] |= (pAalConf->nCid << 8) & COD_CHAAL_CID;
      ret = CmdWrite (pDev, pData, 1);
   }
   /* \todo what about signalling channel */

   return ret;
}
#endif /* (VIN_CFG_FEATURES & VIN_FEAT_PACKET_AAL) */
