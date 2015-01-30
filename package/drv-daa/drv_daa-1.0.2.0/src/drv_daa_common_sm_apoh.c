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

/*   Description : common apoh state machine */


#include "drv_daa.h"
#include "drv_daa_api.h"
#include "drv_daa_common_priv.h"
#include "drv_daa_common.h"
#include "drv_daa_boards.h"

static IFX_void_t daa_sh_apoh_Nopoh    (DAA_DEV_t *pDaa, DAA_APOH_EVT_t evt);
static IFX_void_t daa_sh_apoh_PreApoh  (DAA_DEV_t *pDaa, DAA_APOH_EVT_t evt);
static IFX_void_t daa_sh_apoh_Apoh     (DAA_DEV_t *pDaa, DAA_APOH_EVT_t evt);
static IFX_void_t daa_sh_apoh_PreNopoh (DAA_DEV_t *pDaa, DAA_APOH_EVT_t evt);


typedef IFX_void_t (*DAA_APOH_STATE_HANDLER) (DAA_DEV_t *pDaa, DAA_APOH_EVT_t evt);

DAA_APOH_STATE_HANDLER  gDaaApohStates[DAA_APOH_STATE_NO_STATES] =
                        {daa_sh_apoh_Nopoh, daa_sh_apoh_PreApoh,
                         daa_sh_apoh_Apoh, daa_sh_apoh_PreNopoh};


/** **************************************************************************
   daa apoh state machine timer callback
   \param timer_id   - timer context
   \param arg        - pointer to a daa device structure
   \return           - void
*/
IFX_void_t daa_tcb_apoh (IFX_void_t *timer_id, IFX_uint32_t arg)
{
   daa_sh_apoh ((DAA_DEV_t *) arg, DAA_APOH_EVT_TIMEOUT);
}


/** **************************************************************************
   daa apoh state machine handler for the state NOPOH
   \param pDaa       - pointer to a daa device structure
   \param evt        - event triggering the state machine
   \return void
*/
static IFX_void_t daa_sh_apoh_Nopoh (DAA_DEV_t *pDaa, DAA_APOH_EVT_t evt)
{
   IFX_boolean_t ret;

   switch (evt)
   {
      case DAA_APOH_EVT_APOH_IRQ:                  /* transition 1 */
         pDaa->state_apoh = DAA_APOH_STATE_PRE_APOH;
         /* start apoh timer */
         ret = TAPI_SetTime_Timer(pDaa->tid_apoh, DRV_DAA_DEBOUNCE_TIME,
                                  IFX_FALSE, IFX_FALSE);
         if (ret == IFX_FALSE)
         {
            TRACE(DAA_DRV, DBG_LEVEL_HIGH,
                 ("ERR drv_daa failed to start apoh timer\n\r"));
         }
         break;

      case DAA_APOH_EVT_NOPOH_IRQ:
      case DAA_APOH_EVT_TIMEOUT:
         /* keep state - do nothing */
         break;
   }
}

/** **************************************************************************
   daa apoh state machine handler for the state PreAPOH
   \param pDaa       - pointer to a daa device structure
   \param evt        - event triggering the state machine
   \return void
*/
static IFX_void_t daa_sh_apoh_PreApoh (DAA_DEV_t *pDaa, DAA_APOH_EVT_t evt)
{
   IFX_TAPI_EVENT_t tapi_evt;

   switch (evt)
   {
      case DAA_APOH_EVT_NOPOH_IRQ:                 /* transition 2 */
         TAPI_Stop_Timer(pDaa->tid_apoh);
         pDaa->state_apoh = DAA_APOH_STATE_NOPOH;
         break;
      case DAA_APOH_EVT_TIMEOUT:                   /* transition 3 */
         tapi_evt.id = IFX_TAPI_EVENT_FXO_APOH;
         IFX_TAPI_FXO_Event_Dispatch(pDaa->nDaa, &tapi_evt);
         pDaa->state_apoh = DAA_APOH_STATE_APOH;
         TRACE(DAA_DRV, DBG_LEVEL_LOW,("daa_sh_APOH\n\r"));
         break;

      case DAA_APOH_EVT_APOH_IRQ:
         /* keep state - do nothing */
         break;
   }
}

/** **************************************************************************
   daa apoh state machine handler for the state APOH
   \param pDaa       - pointer to a daa device structure
   \param evt        - event triggering the state machine
   \return void
*/
static IFX_void_t daa_sh_apoh_Apoh (DAA_DEV_t *pDaa, DAA_APOH_EVT_t evt)
{
   IFX_boolean_t ret;

   switch (evt)
   {
      case DAA_APOH_EVT_NOPOH_IRQ:                 /* transition 4 */
         pDaa->state_apoh = DAA_APOH_STATE_PRE_NOPOH;
         /* start apoh timer */
         ret = TAPI_SetTime_Timer(pDaa->tid_apoh, DRV_DAA_DEBOUNCE_TIME,
                                  IFX_FALSE, IFX_FALSE);
         if (ret == IFX_FALSE)
         {
            TRACE(DAA_DRV, DBG_LEVEL_HIGH,
                 ("ERR drv_daa failed to start apoh timer\n\r"));
         }
         break;

      case DAA_APOH_EVT_APOH_IRQ:
      case DAA_APOH_EVT_TIMEOUT:
         /* keep state - do nothing */
         break;
   }
}

/** **************************************************************************
   daa apoh state machine handler for the state PreNOPOH
   \param pDaa       - pointer to a daa device structure
   \param evt        - event triggering the state machine
   \return void
*/
static IFX_void_t daa_sh_apoh_PreNopoh (DAA_DEV_t *pDaa, DAA_APOH_EVT_t evt)
{
   IFX_TAPI_EVENT_t tapi_evt;

   switch (evt)
   {
      case DAA_APOH_EVT_APOH_IRQ:                  /* transition 5 */
         TAPI_Stop_Timer(pDaa->tid_apoh);
         pDaa->state_apoh = DAA_APOH_STATE_APOH;
         break;
      case DAA_APOH_EVT_TIMEOUT:                   /* transition 6 */
         tapi_evt.id = IFX_TAPI_EVENT_FXO_NOPOH;
         IFX_TAPI_FXO_Event_Dispatch(pDaa->nDaa, &tapi_evt);
         pDaa->state_apoh = DAA_APOH_STATE_NOPOH;
         TRACE(DAA_DRV, DBG_LEVEL_LOW,("daa_sh_NOPOH\n\r"));
         break;

      case DAA_APOH_EVT_NOPOH_IRQ:
         /* keep state - do nothing */
         break;
   }
}


/** **************************************************************************
   daa apoh state machine handler
   \param pDaa       - pointer to a daa device structure
   \param evt        - event triggering the state machine
   \return void
*/
IFX_void_t daa_sh_apoh (DAA_DEV_t *pDaa, DAA_APOH_EVT_t evt)
{
   IFXOS_LOCKINT(pDaa->lock);
   gDaaApohStates[pDaa->state_apoh](pDaa, evt);
   IFXOS_UNLOCKINT(pDaa->lock);
}

