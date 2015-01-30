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

/*   Description : common polarity state machine */


#include "drv_daa.h"
#include "drv_daa_api.h"
#include "drv_daa_common_priv.h"
#include "drv_daa_common.h"
#include "drv_daa_boards.h"

static IFX_void_t daa_sh_pol_Normal      (DAA_DEV_t *pDaa, DAA_POL_EVT_t evt);
static IFX_void_t daa_sh_pol_PreReverse  (DAA_DEV_t *pDaa, DAA_POL_EVT_t evt);
static IFX_void_t daa_sh_pol_Reverse     (DAA_DEV_t *pDaa, DAA_POL_EVT_t evt);
static IFX_void_t daa_sh_pol_PreNormal   (DAA_DEV_t *pDaa, DAA_POL_EVT_t evt);

typedef IFX_void_t (*DAA_POL_STATE_HANDLER) (DAA_DEV_t *pDaa, DAA_POL_EVT_t evt);

DAA_POL_STATE_HANDLER  gDaaPolStates[DAA_POL_STATE_NO_STATES] =
                        {daa_sh_pol_Normal, daa_sh_pol_PreReverse,
                         daa_sh_pol_Reverse, daa_sh_pol_PreNormal};



/** **************************************************************************
   daa pol state machine timer callback
   \param timer_id   - timer context
   \param arg        - pointer to a daa device structure
   \return           - void
*/
IFX_void_t daa_tcb_pol (IFX_void_t *timer_id, IFX_uint32_t arg)
{
   daa_sh_pol ((DAA_DEV_t *) arg, DAA_POL_EVT_TIMEOUT);
}


/** **************************************************************************
   daa pol state machine handler for the state NORMAL
   \param pDaa       - pointer to a daa device structure
   \param evt        - event triggering the state machine
   \return void
*/
static IFX_void_t daa_sh_pol_Normal (DAA_DEV_t *pDaa, DAA_POL_EVT_t evt)
{
   IFX_boolean_t  ret;
   switch (evt)
   {
      case DAA_POL_EVT_REVERSE_IRQ:                /* transition 1 */
         pDaa->state_pol = DAA_POL_STATE_PRE_REVERSE;
         /* start pol timer */
         ret = TAPI_SetTime_Timer(pDaa->tid_pol, DRV_DAA_DEBOUNCE_TIME,
                                  IFX_FALSE, IFX_FALSE);
         if (ret == IFX_FALSE)
         {
            TRACE(DAA_DRV, DBG_LEVEL_HIGH,
                 ("ERR drv_daa failed to start polarity timer\n\r"));
         }
         break;

      case DAA_POL_EVT_NORMAL_IRQ:
      case DAA_POL_EVT_TIMEOUT:
         /* keep state - do nothing */
         break;
   }
}

/** **************************************************************************
   daa pol state machine handler for the state PreREVERSE
   \param pDaa       - pointer to a daa device structure
   \param evt        - event triggering the state machine
   \return void
*/
static IFX_void_t daa_sh_pol_PreReverse (DAA_DEV_t *pDaa, DAA_POL_EVT_t evt)
{
   IFX_TAPI_EVENT_t tapi_evt;

   switch (evt)
   {
      case DAA_POL_EVT_NORMAL_IRQ:                 /* transition 2 */
         TAPI_Stop_Timer(pDaa->tid_pol);
         pDaa->state_pol = DAA_POL_STATE_NORMAL;
         break;
      case DAA_POL_EVT_TIMEOUT:                    /* transition 3 */
         tapi_evt.id = IFX_TAPI_EVENT_FXO_POLARITY;
         tapi_evt.data.value = 0; /* reverse */
         IFX_TAPI_FXO_Event_Dispatch(pDaa->nDaa, &tapi_evt);
         pDaa->state_pol = DAA_POL_STATE_REVERSE;
         TRACE(DAA_DRV, DBG_LEVEL_LOW,("daa_sh_POL reverse\n\r"));
         break;

      case DAA_POL_EVT_REVERSE_IRQ:
         /* keep state - do nothing */
         break;
   }
}

/** **************************************************************************
   daa pol state machine handler for the state REVERSE
   \param pDaa       - pointer to a daa device structure
   \param evt        - event triggering the state machine
   \return void
*/
static IFX_void_t daa_sh_pol_Reverse (DAA_DEV_t *pDaa, DAA_POL_EVT_t evt)
{
   IFX_boolean_t ret;

   switch (evt)
   {
      case DAA_POL_EVT_NORMAL_IRQ:                 /* transition 4 */
         pDaa->state_pol = DAA_POL_STATE_PRE_NORMAL;
         /* start pol timer */
         ret = TAPI_SetTime_Timer(pDaa->tid_pol, DRV_DAA_DEBOUNCE_TIME,
                                  IFX_FALSE, IFX_FALSE);
         if (ret == IFX_FALSE)
         {
            TRACE(DAA_DRV, DBG_LEVEL_HIGH,
                 ("ERR drv_daa failed to start polarity timer\n\r"));
         }
         break;

      case DAA_POL_EVT_REVERSE_IRQ:
      case DAA_POL_EVT_TIMEOUT:
         /* keep state - do nothing */
         break;
   }
}

/** **************************************************************************
   daa pol state machine handler for the state PreNORMAL
   \param pDaa       - pointer to a daa device structure
   \param evt        - event triggering the state machine
   \return void
*/
static IFX_void_t daa_sh_pol_PreNormal (DAA_DEV_t *pDaa, DAA_POL_EVT_t evt)
{
   IFX_TAPI_EVENT_t tapi_evt;

   switch (evt)
   {
      case DAA_POL_EVT_REVERSE_IRQ:                /* transition 5 */
         TAPI_Stop_Timer(pDaa->tid_pol);
         pDaa->state_pol = DAA_POL_STATE_REVERSE;
         break;
      case DAA_POL_EVT_TIMEOUT:                    /* transition 6 */
         tapi_evt.id = IFX_TAPI_EVENT_FXO_POLARITY;
         tapi_evt.data.value = 1; /* normal */
         IFX_TAPI_FXO_Event_Dispatch(pDaa->nDaa, &tapi_evt);
         pDaa->state_pol = DAA_POL_STATE_NORMAL;
         TRACE(DAA_DRV, DBG_LEVEL_LOW,("daa_sh_POL normal\n\r"));
         break;

      case DAA_POL_EVT_NORMAL_IRQ:
         /* keep state - do nothing */
         break;
   }
}


/** **************************************************************************
   daa pol state machine handler
   \param pDaa       - pointer to a daa device structure
   \param evt        - event triggering the state machine
   \return void
*/
IFX_void_t daa_sh_pol (DAA_DEV_t *pDaa, DAA_POL_EVT_t evt)
{
   IFXOS_LOCKINT(pDaa->lock);
   gDaaPolStates[pDaa->state_pol](pDaa, evt);
   IFXOS_UNLOCKINT(pDaa->lock);
}

