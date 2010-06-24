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

/*   Description : common ring state machine */


#include "drv_daa.h"
#include "drv_daa_api.h"
#include "drv_daa_common_priv.h"
#include "drv_daa_common.h"
#include "drv_daa_boards.h"

static IFX_void_t daa_sh_ring_notRinging (DAA_DEV_t *pDaa, DAA_RING_EVT_t evt);
static IFX_void_t daa_sh_ring_Ringing (DAA_DEV_t *pDaa, DAA_RING_EVT_t evt);

typedef IFX_void_t (*DAA_RING_STATE_HANDLER) (DAA_DEV_t *pDaa, DAA_RING_EVT_t evt);

DAA_RING_STATE_HANDLER  gDaaRingStates[DAA_RING_STATE_NO_STATES] =
                        {daa_sh_ring_notRinging, daa_sh_ring_Ringing};

/** **************************************************************************
   daa ring state machine timer callback
   \param timer_id   - timer context
   \param arg        - pointer to a daa device structure
   \return           - void
*/
IFX_void_t daa_tcb_ring (IFX_void_t *timer_id, IFX_uint32_t arg)
{
   daa_sh_ring ((DAA_DEV_t *) arg, DAA_RING_EVT_TIMEOUT);
}


/** **************************************************************************
   daa ring state machine handler for the state notRinging
   \param pDaa       - pointer to a daa device structure
   \param evt        - event triggering the state machine
   \return void
*/
static IFX_void_t daa_sh_ring_notRinging (DAA_DEV_t *pDaa, DAA_RING_EVT_t evt)
{
   IFX_TAPI_EVENT_t tapi_evt;
   IFX_boolean_t ret;

   switch (evt)
   {
      case DAA_RING_EVT_TIMEOUT:
         /* reset ring counter */
         pDaa->nRingCnt = 0;
         break;
      case DAA_RING_EVT_RING_IRQ:
         if (pDaa->nRingCnt == 0)
         {
            /* start ring timer */
            ret = TAPI_SetTime_Timer(pDaa->tid_ring, DRV_DAA_DEBOUNCE_TIME,
                                     IFX_FALSE, IFX_FALSE);
            if (ret == IFX_FALSE)
            {
               TRACE(DAA_DRV, DBG_LEVEL_HIGH,
                    ("ERR drv_daa failed to start ring timer\n\r"));
            }
         }
         else
         {
            /* restart ring timer */
            ret = TAPI_SetTime_Timer(pDaa->tid_ring, DRV_DAA_DEBOUNCE_TIME,
                                     IFX_FALSE, IFX_TRUE);
            if (ret == IFX_FALSE)
            {
               TRACE(DAA_DRV, DBG_LEVEL_HIGH,
                    ("ERR drv_daa failed to restart ring timer\n\r"));
            }
            /* filter wrong ring irqs */
            if (pDaa->nRingCnt >= 2)
            {
               /* report RING_START */
               tapi_evt.id = IFX_TAPI_EVENT_FXO_RING_START;
               IFX_TAPI_FXO_Event_Dispatch(pDaa->nDaa, &tapi_evt);
               pDaa->state_ring = DAA_RING_STATE_RINGING;
               TRACE(DAA_DRV, DBG_LEVEL_LOW,
                    ("DAA ring STAT notRinging EVT rIrqt ==> RingStart\n\r"));
            }
         }
         /* increase ring counter */
         pDaa->nRingCnt++;
         break;
   }
}


/** **************************************************************************
   daa ring state machine handler for the state Ringing
   \param pDaa       - pointer to a daa device structure
   \param evt        - event triggering the state machine
   \return void
*/
static IFX_void_t daa_sh_ring_Ringing (DAA_DEV_t *pDaa, DAA_RING_EVT_t evt)
{
   IFX_TAPI_EVENT_t tapi_evt;
   IFX_boolean_t ret;

   switch (evt)
   {
      case DAA_RING_EVT_TIMEOUT:
         /* report RING_STOP */
         tapi_evt.id = IFX_TAPI_EVENT_FXO_RING_STOP;
         IFX_TAPI_FXO_Event_Dispatch(pDaa->nDaa, &tapi_evt);
         pDaa->state_ring = DAA_RING_STATE_NOT_RINGING;
         pDaa->nRingCnt = 0;
         TRACE(DAA_DRV, DBG_LEVEL_LOW,
              ("DAA ring STAT Ringing EVT timeout ==> RingStop\n\r"));
         break;
      case DAA_RING_EVT_RING_IRQ:
         /* restart ring timer */
         ret = TAPI_SetTime_Timer(pDaa->tid_ring, DRV_DAA_DEBOUNCE_TIME,
                                  IFX_FALSE, IFX_TRUE);
         if (ret == IFX_FALSE)
         {
            TRACE(DAA_DRV, DBG_LEVEL_HIGH,
                 ("ERR drv_daa failed to restart ring timer\n\r"));
         }
         break;
   }
}


/** **************************************************************************
   daa ring state machine handler
   \param pDaa       - pointer to a daa device structure
   \param evt        - event triggering the state machine
   \return void
*/
IFX_void_t daa_sh_ring (DAA_DEV_t *pDaa, DAA_RING_EVT_t evt)
{
   IFXOS_LOCKINT(pDaa->lock);
   gDaaRingStates[pDaa->state_ring](pDaa, evt);
   IFXOS_UNLOCKINT(pDaa->lock);
}

