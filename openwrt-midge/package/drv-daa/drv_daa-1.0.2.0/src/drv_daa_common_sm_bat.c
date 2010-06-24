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

/*   Description : common battery/OSI state machine */


#include "drv_daa.h"
#include "drv_daa_api.h"
#include "drv_daa_common_priv.h"
#include "drv_daa_common.h"
#include "drv_daa_boards.h"

static IFX_void_t daa_sh_bat_notFeeded (DAA_DEV_t *pDaa, DAA_BAT_EVT_t evt);
static IFX_void_t daa_sh_bat_preFeeded (DAA_DEV_t *pDaa, DAA_BAT_EVT_t evt);
static IFX_void_t daa_sh_bat_Feeded    (DAA_DEV_t *pDaa, DAA_BAT_EVT_t evt);
static IFX_void_t daa_sh_bat_preOSI    (DAA_DEV_t *pDaa, DAA_BAT_EVT_t evt);
static IFX_void_t daa_sh_bat_OSI       (DAA_DEV_t *pDaa, DAA_BAT_EVT_t evt);

/**
   global daa max OSI time
*/
static IFX_uint32_t  gnMaxOsiTime    = DRV_DAA_DEFAULT_MAX_OSI_TIME;


typedef IFX_void_t (*DAA_BAT_STATE_HANDLER) (DAA_DEV_t *pDaa, DAA_BAT_EVT_t evt);

DAA_BAT_STATE_HANDLER
            gDaaBatStates[DAA_BAT_STATE_NO_STATES] = { daa_sh_bat_notFeeded,
                                                       daa_sh_bat_preFeeded,
                                                       daa_sh_bat_Feeded,
                                                       daa_sh_bat_preOSI,
                                                       daa_sh_bat_OSI
                                                      };


/** **************************************************************************
   daa battery state machine timer callback
   \param timer_id   - timer context
   \param arg        - pointer to a daa device structure
   \return           - void
*/
IFX_void_t daa_tcb_bat (IFX_void_t *timer_id, IFX_uint32_t arg)
{
   daa_sh_bat ((DAA_DEV_t *) arg, DAA_BAT_EVT_TIMEOUT);
}


/** **************************************************************************
   daa battery state machine handler for the state notFeeded
   \param pDaa       - pointer to a daa device structure
   \param evt        - event triggering the state machine
   \return void
*/
static IFX_void_t daa_sh_bat_notFeeded (DAA_DEV_t *pDaa, DAA_BAT_EVT_t evt)
{
   IFX_boolean_t ret;

   switch (evt)
   {
      case DAA_BAT_EVT_BATTERY_IRQ:       /* transition 1 */
         pDaa->state_bat = DAA_BAT_STATE_PRE_FEEDED;
         /* start battery timer */
         ret = TAPI_SetTime_Timer(pDaa->tid_bat, DRV_DAA_DEBOUNCE_TIME,
                                  IFX_FALSE, IFX_FALSE);
         if (ret == IFX_FALSE)
         {
            TRACE(DAA_DRV, DBG_LEVEL_HIGH,
                 ("ERR drv_daa failed to start battery timer\n\r"));
         }
         break;

      case DAA_BAT_EVT_TIMEOUT:
      case DAA_BAT_EVT_BATTERY_DROP_IRQ:
         /* do nothing, keep state */
         break;
   }
}

/** **************************************************************************
   daa battery state machine handler for the state preFeeded
   \param pDaa       - pointer to a daa device structure
   \param evt        - event triggering the state machine
   \return void
*/
static IFX_void_t daa_sh_bat_preFeeded (DAA_DEV_t *pDaa, DAA_BAT_EVT_t evt)
{
   IFX_TAPI_EVENT_t tapi_evt;

   switch (evt)
   {
      case DAA_BAT_EVT_BATTERY_DROP_IRQ:  /* transition 2 */
         TAPI_Stop_Timer(pDaa->tid_bat);
         pDaa->state_bat = DAA_BAT_STATE_NOT_FEEDED;
         break;

      case DAA_BAT_EVT_TIMEOUT:           /* transition 3 */
         /* report feeded */
         tapi_evt.id = IFX_TAPI_EVENT_FXO_BAT_FEEDED;
         IFX_TAPI_FXO_Event_Dispatch(pDaa->nDaa, &tapi_evt);
         TRACE(DAA_DRV, DBG_LEVEL_LOW,("daa_sh_BAT feeded\n\r"));
         pDaa->state_bat = DAA_BAT_STATE_FEEDED;
         break;

      case DAA_BAT_EVT_BATTERY_IRQ:
         /* do nothing */
         break;
   }
}

/** **************************************************************************
   daa battery state machine handler for the state Feeded
   \param pDaa       - pointer to a daa device structure
   \param evt        - event triggering the state machine
   \return void
*/
static IFX_void_t daa_sh_bat_Feeded (DAA_DEV_t *pDaa, DAA_BAT_EVT_t evt)
{
   IFX_boolean_t ret;

   switch (evt)
   {
      case DAA_BAT_EVT_BATTERY_DROP_IRQ:    /* transition 4 */
         pDaa->state_bat = DAA_BAT_STATE_PRE_OSI;
         /* start battery timer */
         ret = TAPI_SetTime_Timer(pDaa->tid_bat, DRV_DAA_DEBOUNCE_TIME,
                                  IFX_FALSE, IFX_FALSE);
         if (ret == IFX_FALSE)
         {
            TRACE(DAA_DRV, DBG_LEVEL_HIGH,
                 ("ERR drv_daa failed to start battery timer\n\r"));
         }
         break;

      case DAA_BAT_EVT_TIMEOUT:
      case DAA_BAT_EVT_BATTERY_IRQ:
         /* do nothing, keep state */
         break;
   }
}

/** **************************************************************************
   daa battery state machine handler for the state preOSI
   \param pDaa       - pointer to a daa device structure
   \param evt        - event triggering the state machine
   \return void
*/
static IFX_void_t daa_sh_bat_preOSI (DAA_DEV_t *pDaa, DAA_BAT_EVT_t evt)
{
   IFX_boolean_t ret;

   switch (evt)
   {
      case DAA_BAT_EVT_TIMEOUT:              /* transition 6 */
        pDaa->state_bat = DAA_BAT_STATE_OSI;
         /* start battery timer */
         ret = TAPI_SetTime_Timer(pDaa->tid_bat, gnMaxOsiTime - DRV_DAA_DEBOUNCE_TIME,
                                  IFX_FALSE, IFX_FALSE);
         if (ret == IFX_FALSE)
         {
            TRACE(DAA_DRV, DBG_LEVEL_HIGH,
                 ("ERR drv_daa failed to start battery timer\n\r"));
         }
         break;

      case DAA_BAT_EVT_BATTERY_IRQ:          /* transition 5 */
         TAPI_Stop_Timer (pDaa->tid_bat);
         pDaa->state_bat = DAA_BAT_STATE_FEEDED;
         break;

      case DAA_BAT_EVT_BATTERY_DROP_IRQ:
         /* do nothing, keep state */
         break;
   }
}

/** **************************************************************************
   daa battery state machine handler for the state OSI
   \param pDaa       - pointer to a daa device structure
   \param evt        - event triggering the state machine
   \return void
*/
static IFX_void_t daa_sh_bat_OSI (DAA_DEV_t *pDaa, DAA_BAT_EVT_t evt)
{
   IFX_TAPI_EVENT_t tapi_evt;

   switch (evt)
   {
      case DAA_BAT_EVT_TIMEOUT:                 /* transition 8 */
         pDaa->state_bat = DAA_BAT_STATE_NOT_FEEDED;
         /* report battery dropped */
         tapi_evt.id = IFX_TAPI_EVENT_FXO_BAT_DROPPED;
         IFX_TAPI_FXO_Event_Dispatch(pDaa->nDaa, &tapi_evt);
         TRACE(DAA_DRV, DBG_LEVEL_LOW,("daa_sh_BAT dropped\n\r"));
         break;

      case DAA_BAT_EVT_BATTERY_IRQ:             /* transition 7 */
         TAPI_Stop_Timer (pDaa->tid_bat);
         /* report OSI event */
         tapi_evt.id = IFX_TAPI_EVENT_FXO_OSI;
         IFX_TAPI_FXO_Event_Dispatch(pDaa->nDaa, &tapi_evt);
         pDaa->state_bat = DAA_BAT_STATE_FEEDED;
         TRACE(DAA_DRV, DBG_LEVEL_LOW,("daa_sh_OSI\n\r"));
         break;

      case DAA_BAT_EVT_BATTERY_DROP_IRQ:
         /* do nothing, keep state */
         break;
   }
}


/** **************************************************************************
   daa battery state machine handler
   \param pDaa       - pointer to a daa device structure
   \param evt        - event triggering the state machine
   \return void
*/
IFX_void_t daa_sh_bat (DAA_DEV_t *pDaa, DAA_BAT_EVT_t evt)
{
   IFXOS_LOCKINT(pDaa->lock);
   gDaaBatStates[pDaa->state_bat](pDaa, evt);
   IFXOS_UNLOCKINT(pDaa->lock);
}


/** **************************************************************************
   common daa configuration for the max OSI time
   \param nMaxOsiTime - maximum time for a voltage drop to be
                        recognized as an OSI in [ms]
   \return - void
*/
IFX_void_t DAA_COM_MaxOsiTimeSet (IFX_uint32_t nMaxOsiTime)
{
   gnMaxOsiTime = nMaxOsiTime;
}
