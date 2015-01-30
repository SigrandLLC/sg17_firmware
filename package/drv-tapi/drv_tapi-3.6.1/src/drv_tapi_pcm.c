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
   \file drv_tapi_pcm.c
   Desription  : Contains PCM Services.
*/

/* ============================= */
/* Includes                      */
/* ============================= */

#include "drv_tapi.h"
#include "drv_tapi_ll_interface.h"

/* ============================= */
/* Global function definition    */
/* ============================= */


/**
   Sets the configuration of the PCM channel interface

   \param pTAPIDev      Pointer to TAPI_DEV structure.
   \param pPCMif        Contains the configuration for the pcm interface.

   \return
   Returns an error code:
      - \ref IFX_SUCCESS if configuration is set
      - \ref IFX_ERROR   if configuration is NOT set.
*/
IFX_int32_t TAPI_Phone_PCM_IF_Set_Config(TAPI_DEV *pTAPIDev,
                                         IFX_TAPI_PCM_IF_CFG_t const *pPCMif)
{
   IFX_TAPI_DRV_CTX_t *pDrvCtx = pTAPIDev->pDevDrvCtx;
   IFX_int32_t ret = IFX_SUCCESS;

   /* chip specific function */
   if (ptr_chk(pDrvCtx->PCM.ifCfg, "pDrvCtx->PCM.ifCfg"))
      ret = pDrvCtx->PCM.ifCfg (pTAPIDev->pLLDev, pPCMif);

   return ret;
}

/**
   Sets the configuration of one PCM channel

   \param pChannel      Pointer to TAPI_CHANNEL structure.
   \param pPCMConfig    Contains the configuration for the pcm channel.

   \return
   Returns an error code:
      - \ref IFX_SUCCESS if configuration is set
      - \ref IFX_ERROR   if configuration is NOT set.
*/
IFX_int32_t TAPI_Phone_PCM_Set_Config(TAPI_CHANNEL *pChannel,
                                      IFX_TAPI_PCM_CFG_t const *pPCMConfig)
{
   IFX_TAPI_DRV_CTX_t *pDrvCtx = pChannel->pTapiDevice->pDevDrvCtx;
   IFX_int32_t ret = IFX_SUCCESS;

   /* chip specific function */
   if (ptr_chk(pDrvCtx->PCM.Cfg, "pDrvCtx->PCM.Cfg"))
      ret = pDrvCtx->PCM.Cfg (pChannel->pLLChannel, pPCMConfig);

   /* save configuration */
   if (ret == IFX_SUCCESS)
      pChannel->TapiPCMData.PCMConfig = *pPCMConfig;

   return ret;
}


/**
   Gets the configuration of one PCM channel

   \param pChannel      Pointer to TAPI_CHANNEL structure.
   \param pPCMConfig    Contains the configuration for pcm channel.

   \return
   Returns an error code:
      - \ref IFX_SUCCESS if configuration is set
      - \ref IFX_ERROR   if configuration is NOT set.
*/
IFX_int32_t TAPI_Phone_PCM_Get_Config(TAPI_CHANNEL *pChannel,
                                      IFX_TAPI_PCM_CFG_t *pPCMConfig)
{
   /* get configuration */
   *pPCMConfig = pChannel->TapiPCMData.PCMConfig;

   return IFX_SUCCESS;
}


/**
   Activate or deactivate the pcm timeslots configured for this channel

   \param pChannel      Pointer to TAPI_CHANNEL structure.
   \param nMode         - 1: timeslot activated
                        - 0: timeslot deactivated
   \return
   \ref IFX_SUCCESS or \ref IFX_ERROR
*/
IFX_int32_t TAPI_Phone_PCM_Set_Activation(TAPI_CHANNEL *pChannel,
                                          IFX_uint32_t nMode)
{
   IFX_TAPI_DRV_CTX_t *pDrvCtx = pChannel->pTapiDevice->pDevDrvCtx;
   IFX_int32_t ret = IFX_SUCCESS;

   if (nMode > 1)
   {
      TRACE(TAPI_DRV,DBG_LEVEL_HIGH,
         ("\n\rDRV_ERROR: unknown mode %u (Phone_PCM_Activation)\n\r", nMode));
      return IFX_ERROR;
   }

   /* chip specific function */
   if (ptr_chk(pDrvCtx->PCM.Enable, "pDrvCtx->PCM.Enable"))
   {
      ret = pDrvCtx->PCM.Enable (pChannel->pLLChannel, nMode, &(pChannel->TapiPCMData.PCMConfig));
   }

   /* save activation level */
   if (ret == IFX_SUCCESS)
   {
      pChannel->TapiPCMData.bTimeSlotActive = (IFX_boolean_t)(nMode == 1);
   }

   return ret;
}

/**
   Get the activation status from the pcm interface

   \param pChannel      Pointer to TAPI_CHANNEL structure.
   \param pbAct         Pointer to a boolean, returning the activation state.

   \return IFX_SUCCESS
*/
IFX_int32_t TAPI_Phone_PCM_Get_Activation (TAPI_CHANNEL *pChannel,
                                          IFX_boolean_t *pbAct)
{
   *pbAct = pChannel->TapiPCMData.bTimeSlotActive;
   return IFX_SUCCESS;
}
