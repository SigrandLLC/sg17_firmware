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

/*   Description : Common functions  */


/* ============================= */
/* Includes                      */
/* ============================= */
#include "drv_daa.h"
#include "drv_daa_api.h"
#include "drv_daa_common_priv.h"
#include "drv_daa_common.h"
#include "drv_daa_boards.h"

/* ============================= */
/* Local Macros & Definitions    */
/* ============================= */


/* ============================= */
/* Local function declaration    */
/* ============================= */

/* ============================= */
/* Global variable definition    */
/* ============================= */
CREATE_TRACE_GROUP(DAA_DRV);

/**
   global function pointer map, providing access to the board specific
   handling for common FXO functionality
*/
DAA_FUNCTION_MAP_t        gDaaFctMap[DRV_DAA_MAX_DAA_CHANNELS] = {};
/**
   global daa device data storage, used to daa device specific status
   inforamtion as well as timer ids, semaphores, etc.
*/
DAA_DEV_t                 gDaaDev   [DRV_DAA_MAX_DAA_CHANNELS] = {};

/**
   global daa flash hook time
*/
static IFX_uint32_t       gnFlashHookTime = DRV_DAA_DEFAULT_HOOK_FALSH_TIME;


/* ============================= */
/* Local variable definition     */
/* ============================= */

/* ============================= */
/* Local function definition     */
/* ============================= */

/* ============================= */
/* Global function definition    */
/* ============================= */


/** **************************************************************************
   common daa configuration for the flash hook time
   \param nFlashHookTime - flash hook time for all devices in [ms]
   \return - void
*/
void DAA_COM_FlashHookTimeSet (IFX_uint32_t nFlashHookTime)
{
   gnFlashHookTime = nFlashHookTime;
}

/** **************************************************************************
   timer callback for hookFlash
   \param tid  - timer id
   \param arg  - pointer to the correspoinding DAA context
   \return - void
*/
IFX_void_t daa_tcb_hookFlash (IFX_void_t *tid, IFX_uint32_t arg)
{
   DAA_DEV_t   *pDaa = (DAA_DEV_t *) arg;

   /* hook flash is a short onHook,
      when the timer expires, we go offHook again */
   gDaaFctMap[pDaa->nDaa].hookSet (IFX_ENABLE);
}

/** **************************************************************************
   common daa flashHook generation
   \param nDaa - daa resource number
   \return IFX_SUCCESS or IFX_ERROR in case of failure
*/
IFX_return_t DAA_COM_FlashHook (IFX_uint16_t nDaa)
{
   DAA_DEV_t *pDaa = &gDaaDev[nDaa];
   int ret;
   /* go onHook and start the timer... */
   ret = gDaaFctMap[nDaa].hookSet (IFX_DISABLE);
   if (ret == IFX_SUCCESS)
   {
      ret = TAPI_SetTime_Timer (pDaa->tid_flashHook, gnFlashHookTime,
                                IFX_FALSE, IFX_FALSE);
      if (ret == IFX_FALSE)
      {
         TRACE(DAA_DRV, DBG_LEVEL_HIGH,
              ("ERR drv_daa failed to start flashHook timer\n\r"));
      }
      /* ATTENTION: TAPI_SetTime_Timer currently returns IFX_TURE/FALSE
                    therefore the following conversion is required */
      ret = !ret;
   }
   return ret;
}

/** **************************************************************************
   common daa ringGet
   \param nDaa - daa resource number
   \param pEn  - IFX_ENABLE if line is ringing, IFX_DISABLE if not
   \return IFX_SUCCESS - this one cannot fail as it reads only a variable
*/
IFX_return_t DAA_COM_RingGet (IFX_uint16_t nDaa, IFX_enDis_t *pEn)
{
   IFXOS_LOCKINT(gDaaDev[nDaa].lock);
   switch (gDaaDev[nDaa].state_ring)
   {
      case DAA_RING_STATE_RINGING:
         *pEn = IFX_ENABLE;
         break;
      case DAA_RING_STATE_NOT_RINGING:
         *pEn = IFX_DISABLE;
         break;
   }
   IFXOS_UNLOCKINT(gDaaDev[nDaa].lock);
   return IFX_SUCCESS;
}

/** **************************************************************************
   common daa batGet
   \param nDaa - daa resource number
   \param pBatFeeded   - IFX_ENABLE if line is feeded, IFX_DISABLE if not
   \return IFX_SUCCESS - this one cannot fail as it reads only a variable
*/
IFX_return_t DAA_COM_BatGet (IFX_uint16_t nDaa, IFX_enDis_t *pBatFeeded)
{
   IFXOS_LOCKINT(gDaaDev[nDaa].lock);
   switch (gDaaDev[nDaa].state_bat)
   {
      case DAA_BAT_STATE_FEEDED:
      case DAA_BAT_STATE_PRE_OSI:
      case DAA_BAT_STATE_OSI:
         *pBatFeeded = IFX_ENABLE;
         break;
      case DAA_BAT_STATE_NOT_FEEDED:
      case DAA_BAT_STATE_PRE_FEEDED:
         *pBatFeeded = IFX_DISABLE;
         break;
   }
   IFXOS_UNLOCKINT(gDaaDev[nDaa].lock);
   return IFX_SUCCESS;
}

/** **************************************************************************
   common daa apohGet
   \param nDaa - daa resource number
   \param pApoh - IFX_ENABLE another phone is offhook, IFX_DISABLE if not
   \return IFX_SUCCESS - this one cannot fail as it reads only a variable
*/
IFX_return_t DAA_COM_ApohGet (IFX_uint16_t nDaa, IFX_enDis_t *pApoh)
{
   IFXOS_LOCKINT(gDaaDev[nDaa].lock);
   switch (gDaaDev[nDaa].state_apoh)
   {
      case DAA_APOH_STATE_APOH:
      case DAA_APOH_STATE_PRE_NOPOH:
         *pApoh = IFX_ENABLE;
         break;
      case DAA_APOH_STATE_NOPOH:
      case DAA_APOH_STATE_PRE_APOH:
         *pApoh = IFX_DISABLE;
         break;
   }
   IFXOS_UNLOCKINT(gDaaDev[nDaa].lock);
   return IFX_SUCCESS;
}

/** **************************************************************************
   common daa polGet
   \param nDaa - daa resource number
   \param pPol - IFX_ENABLE if normal pol, IFX_DISABLE if not
   \return IFX_SUCCESS - this one cannot fail as it reads only a variable
*/
IFX_return_t DAA_COM_PolGet (IFX_uint16_t nDaa, IFX_enDis_t *pPol)
{
   IFXOS_LOCKINT(gDaaDev[nDaa].lock);
   switch (gDaaDev[nDaa].state_pol)
   {
      case DAA_POL_STATE_NORMAL:
      case DAA_POL_STATE_PRE_REVERSE:
         *pPol = IFX_ENABLE;
         break;
      case DAA_POL_STATE_REVERSE:
      case DAA_POL_STATE_PRE_NORMAL:
         *pPol = IFX_DISABLE;
         break;
   }
   IFXOS_UNLOCKINT(gDaaDev[nDaa].lock);
   return IFX_SUCCESS;
}

/** **************************************************************************
   common insmod handler / basic initialisation
   steps, which can be done already without accessing any device
   Note that the DAA itself is initialized after the TAPI ioctl \ref
   IFX_TAPI_LINE_TYPE_SET, which also provides the mapping of TAPI ch
   to DAA resource number.
   \return IFX_SUCCESS or IFX_ERROR
*/
IFX_return_t DAA_COM_OnInsmod (void)
{
   return daa_board_OnInsmod ();
}

/** **************************************************************************
   common rmmod handler / deinitialisation
   board specific unregister from GPIOs etc.
   \return IFX_SUCCESS or IFX_ERROR
*/
IFX_return_t DAA_COM_OnRmmod (void)
{
   return daa_board_OnRmmod ();
}


/** **************************************************************************
   common daa initialization
   initialization of data structures, timers, etc.
   \param pDaa    - reference to Daa context
   \return IFX_SUCCESS or IFX_ERROR in case of failure
*/
IFX_return_t DAA_COM_init (IFX_uint16_t nDaa)
{
   DAA_DEV_t *pDaa = &gDaaDev[nDaa];
   int ret = IFX_SUCCESS;

   pDaa->nDaa           = nDaa;
   pDaa->tid_flashHook  = TAPI_Create_Timer(daa_tcb_hookFlash,
                                           (IFX_int32_t)pDaa);
   if (pDaa->tid_flashHook == IFX_NULL)
   {
      TRACE(DAA_DRV, DBG_LEVEL_HIGH,
           ("ERR drv_daa failed to create flashHook timer\n\r"));
      ret = IFX_ERROR;
   }

   /* ring state machine init */
   pDaa->tid_ring       = TAPI_Create_Timer(daa_tcb_ring, (IFX_int32_t)pDaa);
   if (pDaa->tid_ring == IFX_NULL)
   {
      TRACE(DAA_DRV, DBG_LEVEL_HIGH,
           ("ERR drv_daa failed to create ring timer\n\r"));
      ret = IFX_ERROR;
   }
   pDaa->state_ring     = DAA_RING_STATE_NOT_RINGING;
   pDaa->nRingCnt       = 0;

   /* battery state machine init */
   pDaa->tid_bat        = TAPI_Create_Timer(daa_tcb_bat, (IFX_int32_t)pDaa);
   if (pDaa->tid_bat == IFX_NULL)
   {
      TRACE(DAA_DRV, DBG_LEVEL_HIGH,
           ("ERR drv_daa failed to create battery timer\n\r"));
      ret = IFX_ERROR;
   }

   /* apoh state machine init */
   pDaa->tid_apoh       = TAPI_Create_Timer(daa_tcb_apoh, (IFX_int32_t)pDaa);
   if (pDaa->tid_apoh == IFX_NULL)
   {
      TRACE(DAA_DRV, DBG_LEVEL_HIGH,
           ("ERR drv_daa failed to create apoh timer\n\r"));
      ret = IFX_ERROR;
   }

   /* pol state machine init */
   pDaa->tid_pol        = TAPI_Create_Timer(daa_tcb_pol, (IFX_int32_t)pDaa);
   if (pDaa->tid_pol == IFX_NULL)
   {
      TRACE(DAA_DRV, DBG_LEVEL_HIGH,
           ("ERR drv_daa failed to create polarity timer\n\r"));
      ret = IFX_ERROR;
   }

   return ret;
}

/** **************************************************************************
   common daa initialization, retrieve the initial states of the statemachines
   \param pDaa    - reference to Daa context
   \return IFX_SUCCESS or IFX_ERROR in case of failure
*/
IFX_return_t DAA_COM_InitStates (IFX_uint16_t nDaa)
{
   DAA_DEV_t *pDaa = &gDaaDev[nDaa];
   IFX_enDis_t bat, apoh, pol;
   int ret = IFX_SUCCESS;

   /* battery state machine init */
   if (gDaaFctMap[nDaa].batGet != IFX_NULL)
   {
      ret = gDaaFctMap[nDaa].batGet(&bat);
      pDaa->state_bat   = (bat == IFX_ENABLE) ?
                           DAA_BAT_STATE_FEEDED : DAA_BAT_STATE_NOT_FEEDED;
      TRACE(DAA_DRV, DBG_LEVEL_LOW,
           ("initial bat state %s feeded, ret %d\n\r",
           (pDaa->state_bat == DAA_BAT_STATE_FEEDED) ? "" : "not", ret));
   }
   else
   {
      ret = IFX_ERROR;
      pDaa->state_bat   = DAA_BAT_STATE_NOT_FEEDED;
   }

   /* apoh state machine init */
   if (gDaaFctMap[nDaa].apohGet != IFX_NULL)
   {
      if (ret == IFX_SUCCESS)
      {
         ret = gDaaFctMap[nDaa].apohGet(&apoh);
         pDaa->state_apoh  = (apoh == IFX_ENABLE) ?
                              DAA_APOH_STATE_APOH : DAA_APOH_STATE_NOPOH;
         TRACE(DAA_DRV, DBG_LEVEL_LOW,
              ("initial apoh state %sPOH, ret %d\n\r",
              (pDaa->state_apoh == DAA_APOH_STATE_APOH) ? "A" : "NO", ret));
      }
      else
      {
         ret = IFX_ERROR;
         pDaa->state_apoh  = DAA_APOH_STATE_NOPOH;
      }
   }

   /* pol state machine init */
   if ((ret == IFX_SUCCESS) && (gDaaFctMap[nDaa].polGet != IFX_NULL))
   {
      ret = gDaaFctMap[nDaa].polGet(&pol);
      pDaa->state_pol   = (pol == IFX_ENABLE) ?
                           DAA_POL_STATE_NORMAL : DAA_POL_STATE_REVERSE;
      TRACE(DAA_DRV, DBG_LEVEL_LOW,
           ("initial pol state %s, ret %d\n\r",
           (pDaa->state_pol == DAA_POL_STATE_NORMAL) ?
            "normal" : "reverse", ret));
   }
   else
   {
      ret = IFX_ERROR;
      pDaa->state_pol = DAA_POL_STATE_NORMAL;
   }
   return ret;
}


/** **************************************************************************
   common daa interrupt callback for ring interrupts
   this function is called by the board specific callbacks (drv_board_xx.c)
   and triggers the ring statemachine in drv_daa_common_sm_ring.c
   \param nDaa    - daa resource number
   \return        - void
*/
void irq_DAA_COM_ring_cb (IFX_uint16_t nDaa)
{
   /* this would be the place to distinguish different daa devices and
      specific state machines */
   daa_sh_ring (&gDaaDev[nDaa], DAA_RING_EVT_RING_IRQ);
}


/** **************************************************************************
   common daa interrupt callback for polarity interrupts
   this function is called by the board specific callbacks (drv_board_xx.c)
   and triggers the polarity statemachine in drv_daa_common_sm_pol.c
   \param nDaa    - daa resource number
   \param evt     - pol status
   \return        - void
*/
void irq_DAA_COM_pol_cb (IFX_uint16_t nDaa, DAA_POL_EVT_t evt)
{
   /* this would be the place to distinguish different daa devices and
      specific state machines */
   daa_sh_pol (&gDaaDev[nDaa], evt);
}

/** **************************************************************************
   common daa interrupt callback for battery interrupts
   this function is called by the board specific callbacks (drv_board_xx.c)
   and triggers the battery statemachine in drv_daa_common_sm_bat.c
   \param nDaa    - daa resource number
   \param evt     - bat status
   \return        - void
*/
void irq_DAA_COM_bat_cb (IFX_uint16_t nDaa, DAA_BAT_EVT_t evt)
{
   /* this would be the place to distinguish different daa devices and
      specific state machines */
   daa_sh_bat (&gDaaDev[nDaa], evt);
}

/** **************************************************************************
   common daa interrupt callback for apoh interrupts
   this function is called by the board specific callbacks (drv_board_xx.c)
   and triggers the apoh statemachine in drv_daa_common_sm_apoh.c
   \param nDaa    - daa resource number
   \param evt     - apoh status
   \return        - void
*/
void irq_DAA_COM_apoh_cb (IFX_uint16_t nDaa, DAA_APOH_EVT_t evt)
{
   /* this would be the place to distinguish different daa devices and
      specific state machines */
   daa_sh_apoh (&gDaaDev[nDaa], evt);
}

