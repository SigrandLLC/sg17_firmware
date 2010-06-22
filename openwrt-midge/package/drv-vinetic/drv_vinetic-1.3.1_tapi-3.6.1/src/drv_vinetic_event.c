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
   Module      : drv_vinetic_event.c
   Description : Handling of driver, firmware and device specific events.
******************************************************************************/

/* ============================= */
/* Includes                      */
/* ============================= */
#ifdef HAVE_CONFIG_H
#include "drv_config.h"
#endif /* HAVE_CONFIG_H */
#include "drv_vinetic_api.h"

/* ============================= */
/* Global function definition    */
/* ============================= */

/**
   Report a device error to TAPI
\param pDev - Device context
\param err - error number
*/
IFX_void_t VINETIC_DevErrorEvent (VINETIC_DEVICE* pDev, IFX_uint16_t err)
{
   IFX_TAPI_EVENT_t tapiEvent;
   /* Fill event structure. */
   memset(&tapiEvent, 0, sizeof(IFX_TAPI_EVENT_t));
   tapiEvent.id = IFX_TAPI_EVENT_TYPE_FAULT_GENERAL;
   tapiEvent.data.value = IFX_TAPI_ERRSRC_LL_DEV | err;

   IFX_TAPI_Event_Dispatch(pDev->pChannel[0].pTapiCh, &tapiEvent);
}

/**
   Report a channel error to TAPI
\param pCh - Channel context
\param err - error number
*/
IFX_void_t VINETIC_ChErrorEvent (VINETIC_CHANNEL* pCh, IFX_uint16_t err)
{
   IFX_TAPI_EVENT_t tapiEvent;
   /* Get tapi channel context. */
   TAPI_CHANNEL* pChannel = (TAPI_CHANNEL *) pCh->pTapiCh;

   /* Fill event structure. */
   memset(&tapiEvent, 0, sizeof(IFX_TAPI_EVENT_t));
   tapiEvent.id = IFX_TAPI_EVENT_TYPE_FAULT_GENERAL;
   tapiEvent.data.value = IFX_TAPI_ERRSRC_LL_CH | err;

   IFX_TAPI_Event_Dispatch(pChannel, &tapiEvent);
}

