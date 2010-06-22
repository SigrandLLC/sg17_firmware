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

*******************************************************************************/

/**
   \file drv_tapi_fxo.c
   TAPI FXO implementation
   based on a system specific DAA abstraction (GPIO handling) as in drv_daa
*/

#include "drv_tapi.h"
#include "drv_tapi_api.h"
#include "drv_tapi_fxo_ll_interface.h"


/** maximum number of DAA channels TAPI can handle,
    (size of the DAA to TAPI_CH lookuptable) */
#define IFX_TAPI_FXO_MAX_DAA_CH             16


static IFX_TAPI_DRV_CTX_DAA_t *gpDaaCtx = IFX_NULL;
static TAPI_CHANNEL* daaChLookUpTable[IFX_TAPI_FXO_MAX_DAA_CH] = {0};

static TAPI_CHANNEL* lookup_pTAPICh (int nDAA);
static void *tapi_fxo_MallocAndCopy(void *p_user, size_t size);


/**
   lookup the TAPI_CHANNEL context belonging to a DAA number
   (required for DAA events)
   \param nDAA - DAA number which raised the event, starting at 0
   \return     - pointer to a TAPI_CHANNEL context,
                 IFX_NULL in case of unknown
*/
static TAPI_CHANNEL* lookup_pTAPICh (int nDAA)
{
   /* sanity check */
   if (nDAA >= IFX_TAPI_FXO_MAX_DAA_CH)
      return (TAPI_CHANNEL*) IFX_NULL;
   return daaChLookUpTable[nDAA];
}


/**
   register a DAA channel in the lookup table
   \param nDAA     - DAA number (starting at 0)
   \param pChannel - pointer to a TAPI_CHANNEL context
   \return IFX_SUCCESS or IFX_ERROR in case of failure
*/
IFX_return_t TAPI_FXO_Register_DAA (int nDAA, TAPI_CHANNEL *pChannel)
{
   /* protection could be added as the lookup table is accessed from
      the DAA's interrupt context - which can be any interreupt context,
      currently we expect that the the linetype configuration is finished
      before the first event is reported, i.e. the interrupts of the
      DAA are unmasked(!) */
   /* sanity check */
   if (nDAA >= IFX_TAPI_FXO_MAX_DAA_CH)
      return(IFX_ERROR);
   /* store channel context */
   daaChLookUpTable[nDAA] = pChannel;
   return IFX_SUCCESS;
}


/**
   initialize a daa channel after registration, i.e. configure the GPIOs
   and internal data structures of the daa driver
   \param pChannel  - pointer to a TAPI_CHANNEL context
   \return IFX_SUCCESS or IFX_ERROR in case of failure
*/
IFX_return_t TAPI_FXO_Init_DAA (TAPI_CHANNEL *pChannel)
{
   int err;

   if (gpDaaCtx == IFX_NULL)
   {
      return IFX_ERROR;
   }

   err = gpDaaCtx->Init(pChannel->TapiOpControlData.nDAA);
   if (err == IFX_SUCCESS)
   {
      err = gpDaaCtx->hookSet(pChannel->TapiOpControlData.nDAA,
                              IFX_TAPI_FXO_HOOK_ONHOOK);
   }
   return err;
}


/**
   register a daa abstraction driver to drv_tapi
   \param pDaaCtx - reference to a daa driver context
   \return        - IFX_SUCCESS or IFX_ERROR in case of failure
   \remarks
   function is exported to be called by the DAA abstraction driver
*/
IFX_return_t IFX_TAPI_Register_DAA_Drv (IFX_TAPI_DRV_CTX_DAA_t *pDaaCtx)
{
   /* add version check before assigning the pointer */
   gpDaaCtx = pDaaCtx;
   return IFX_SUCCESS;
}


/**
   FXO event dispatcher wrapper function
   (including the mapping from DAA numbers to TAPI channels)
   \param nDAA   - DAA number which raised the event, starting at 0
   \param pEvent - TAPI event structure containing the event details
   \return       - IFX_SUCCESS or IFX_ERROR in case of failure
   \remarks
      called from daa driver's interrupt context
*/
IFX_return_t IFX_TAPI_FXO_Event_Dispatch (int nDAA, IFX_TAPI_EVENT_t *pEvent)
{
   TAPI_CHANNEL *pTAPICh = lookup_pTAPICh(nDAA);

   /* sanity check */
   if (pTAPICh == IFX_NULL)
      return IFX_ERROR;
   /* fill-in missing fields */
   pEvent->ch = pTAPICh->nChannel;
   /* forward event to TAPI Event Dispatcher */
   return IFX_TAPI_Event_Dispatch (pTAPICh, pEvent);
}


/**
   FXO ioctl dispatcher
   \param pDrvCtx     - device driver context / handle to ll driver
   \param pChannel    - TAPI channel context
   \param cmd         - ioctl cmd id
   \param arg         - pointer to parameters / integer paramter itself
   \return            - IFX_SUCCESS or IFX_ERROR in case of failure
*/
IFX_int32_t TAPI_FXO_Ioctl (IFX_TAPI_DRV_CTX_t *pDrvCtx,
                            TAPI_CHANNEL *pChannel,
                            IFX_uint32_t cmd,
                            IFX_uint32_t arg)
{
   IFX_int32_t   ret      = IFX_ERROR;

   switch (cmd)
   {
      case IFX_TAPI_FXO_DIAL_CFG_SET:
         if (ptr_chk(pDrvCtx->SIG.DTMFG_Cfg, "pDrvCtx->SIG.DTMFG_Cfg"))
         {
            IFX_TAPI_FXO_DIAL_CFG_t *p_DialCfg =
               tapi_fxo_MallocAndCopy((IFX_void_t *)arg,
                                      sizeof(IFX_TAPI_FXO_DIAL_CFG_t));
            if (p_DialCfg != IFX_NULL)
            {
               ret = pDrvCtx->SIG.DTMFG_Cfg(pChannel->pLLChannel,
                                            p_DialCfg->nInterDigitTime,
                                            p_DialCfg->nDigitPlayTime);
               IFXOS_FREE(p_DialCfg);
            }
         }
         break;

      case IFX_TAPI_FXO_FLASH_CFG_SET:
         /* configuration is allowed also on non FXO channels, sanity check */
         if ((gpDaaCtx != IFX_NULL) && (gpDaaCtx->fhCfg != IFX_NULL))
         {
            IFX_TAPI_FXO_FLASH_CFG_t *p_fhCfg =
               tapi_fxo_MallocAndCopy((IFX_void_t *)arg,
                                      sizeof(IFX_TAPI_FXO_FLASH_CFG_t));
            ret = gpDaaCtx->fhCfg(p_fhCfg->nFlashTime);
            IFXOS_FREE(p_fhCfg);
         }
         break;

      case IFX_TAPI_FXO_OSI_CFG_SET:
         /* configuration is allowed also on non FXO channels */
         if ((gpDaaCtx != IFX_NULL) && (gpDaaCtx->osiCfg != IFX_NULL))
         {
            IFX_TAPI_FXO_OSI_CFG_t *p_osiCfg =
               tapi_fxo_MallocAndCopy((IFX_void_t *)arg,
                                      sizeof(IFX_TAPI_FXO_OSI_CFG_t));
            ret = gpDaaCtx->osiCfg(p_osiCfg->nOSIMax);
            IFXOS_FREE(p_osiCfg);
         }
         break;

      case IFX_TAPI_FXO_DIAL_START:
         if (ptr_chk(pDrvCtx->SIG.DTMFG_Start, "pDrvCtx->SIG.DTMFG_Start"))
         {
            IFX_TAPI_FXO_DIAL_t *p_DialData =
               tapi_fxo_MallocAndCopy((IFX_void_t *)arg,
                                      sizeof(IFX_TAPI_FXO_DIAL_t));
            if (p_DialData != IFX_NULL)
            {
               if ( p_DialData->nDigits > IFX_TAPI_FXO_DIAL_DIGITS )
               {
                  TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
                       ("TAPI: FXO dial-length value exceeds buffer size\n\r"));
                  ret = IFX_ERROR;
               }
               else
               {
                  ret = pDrvCtx->SIG.DTMFG_Start(pChannel->pLLChannel,
                                                 p_DialData->nDigits,
                                                 p_DialData->data);
               }
               IFXOS_FREE(p_DialData);
            }
         }
         break;

      case IFX_TAPI_FXO_DIAL_STOP:
         if (ptr_chk(pDrvCtx->SIG.DTMFG_Stop, "pDrvCtx->SIG.DTMFG_Stop"))
         {
            ret = pDrvCtx->SIG.DTMFG_Stop(pChannel->pLLChannel);
         }
         break;

      case IFX_TAPI_FXO_HOOK_SET:
         /* sanitiy check - ioctl can only be applied to FXO channels */
         if (pChannel->TapiOpControlData.nLineType != IFX_TAPI_LINE_TYPE_FXO)
            return IFX_ERROR;
         if (! pChannel->TapiOpControlData.bDaaInitialized)
            return IFX_ERROR;
         if ((gpDaaCtx != IFX_NULL) && (gpDaaCtx->hookSet != IFX_NULL))
         {
            ret = gpDaaCtx->hookSet(pChannel->TapiOpControlData.nDAA,
                                    (IFX_TAPI_FXO_HOOK_t) arg);
         }
         break;

      case IFX_TAPI_FXO_FLASH_SET:
         /* sanitiy check - ioctl can only be applied to FXO channels */
         if (pChannel->TapiOpControlData.nLineType != IFX_TAPI_LINE_TYPE_FXO)
            return IFX_ERROR;
         if (! pChannel->TapiOpControlData.bDaaInitialized)
            return IFX_ERROR;
         if ((gpDaaCtx != IFX_NULL) && (gpDaaCtx->fhSet != IFX_NULL))
         {
            ret = gpDaaCtx->fhSet(pChannel->TapiOpControlData.nDAA);
         }
         break;

      case IFX_TAPI_FXO_BAT_GET:
         /* sanitiy check - ioctl can only be applied to FXO channels */
         if (pChannel->TapiOpControlData.nLineType != IFX_TAPI_LINE_TYPE_FXO)
            return IFX_ERROR;
         if (! pChannel->TapiOpControlData.bDaaInitialized)
            return IFX_ERROR;
         if ((gpDaaCtx != IFX_NULL) && (gpDaaCtx->batGet != IFX_NULL))
         {
            ret = gpDaaCtx->batGet(pChannel->TapiOpControlData.nDAA,
                                   (IFX_enDis_t*) arg);
         }
         break;

      case IFX_TAPI_FXO_HOOK_GET:
         /* sanitiy check - ioctl can only be applied to FXO channels */
         if (pChannel->TapiOpControlData.nLineType != IFX_TAPI_LINE_TYPE_FXO)
            return IFX_ERROR;
         if (! pChannel->TapiOpControlData.bDaaInitialized)
            return IFX_ERROR;
         if ((gpDaaCtx != IFX_NULL) && (gpDaaCtx->hookGet != IFX_NULL))
         {
            ret = gpDaaCtx->hookGet(pChannel->TapiOpControlData.nDAA,
                                   (IFX_TAPI_FXO_HOOK_t*) arg);
         }
         break;

      case IFX_TAPI_FXO_APOH_GET:
         /* sanitiy check - ioctl can only be applied to FXO channels */
         if (pChannel->TapiOpControlData.nLineType != IFX_TAPI_LINE_TYPE_FXO)
            return IFX_ERROR;
         if (! pChannel->TapiOpControlData.bDaaInitialized)
            return IFX_ERROR;
         if ((gpDaaCtx != IFX_NULL) && (gpDaaCtx->apohGet != IFX_NULL))
         {
            ret = gpDaaCtx->apohGet(pChannel->TapiOpControlData.nDAA,
                                   (IFX_enDis_t*) arg);
         }
         break;

      case IFX_TAPI_FXO_POLARITY_GET:
         /* sanitiy check - ioctl can only be applied to FXO channels */
         if (pChannel->TapiOpControlData.nLineType != IFX_TAPI_LINE_TYPE_FXO)
            return IFX_ERROR;
         if (! pChannel->TapiOpControlData.bDaaInitialized)
            return IFX_ERROR;
         if ((gpDaaCtx != IFX_NULL) && (gpDaaCtx->polGet != IFX_NULL))
         {
            ret = gpDaaCtx->polGet(pChannel->TapiOpControlData.nDAA,
                                  (IFX_enDis_t*) arg);
         }
         break;

      case IFX_TAPI_FXO_RING_GET:
         /* sanitiy check - ioctl can only be applied to FXO channels */
         if (pChannel->TapiOpControlData.nLineType != IFX_TAPI_LINE_TYPE_FXO)
            return IFX_ERROR;
         if (! pChannel->TapiOpControlData.bDaaInitialized)
            return IFX_ERROR;
         if ((gpDaaCtx != IFX_NULL) && (gpDaaCtx->ringGet != IFX_NULL))
         {
            ret = gpDaaCtx->ringGet(pChannel->TapiOpControlData.nDAA,
                                   (IFX_enDis_t*) arg);
         }
         break;

   } /* switch */

   return ret;
}


/**
   allocate kernel memory and copy user data to it
   \param p_user - pointer to user memory
   \param size   - number of bytes to copy from user memory
   \return     - pointer to allocated kernel memory
                 IFX_NULL in case of no memory or copy error
*/
static void *tapi_fxo_MallocAndCopy(void *p_user, size_t size)
{
   /* allocate kernel memory */
   void *p_kern = IFXOS_MALLOC(size);
   if (p_kern == IFX_NULL)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
           ("TAPI: allocate kernel memory failed\n\r"));
   }
   else
   {
      /* return value is IFX_NULL on error */
      if (IFXOS_CPY_USR2KERN(p_kern, p_user, size) == IFX_NULL)
      {
         TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
              ("TAPI: cannot copy from user space\n\r"));
         IFXOS_FREE(p_kern);
         p_kern = IFX_NULL;
      }
   }

   return p_kern;
}
