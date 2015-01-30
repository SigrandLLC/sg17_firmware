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


/* ============================= */
/* Includes                      */
/* ============================= */
#include "drv_daa.h"
#include "drv_daa_api.h"
#include "drv_daa_common.h"

/* ============================= */
/* Local Macros & Definitions    */
/* ============================= */


/* ============================= */
/* Local function declaration    */
/* ============================= */

/* ============================= */
/* Global variable definition    */
/* ============================= */
/** what string support, driver version string */
const char                       DAA_WHATVERSION[] = {DRV_DAA_WHAT_STR};
static IFX_TAPI_DRV_CTX_DAA_t    gDaaCtx;
extern DAA_FUNCTION_MAP_t        gDaaFctMap[];


/* ============================= */
/* Local variable definition     */
/* ============================= */

/* ============================= */
/* Local function definition     */
/* ============================= */

static IFX_return_t DAA_Init         (IFX_uint16_t nDAA);
static IFX_return_t DAA_FlashHookCfg (IFX_uint32_t nFlashHookTime);
static IFX_return_t DAA_OsiCfg       (IFX_uint32_t nMaxOsiTime);

/* ============================= */
/* Global function definition    */
/* ============================= */

IFX_TAPI_DRV_CTX_DAA_t* DAA_CtxGet(void)
{
   gDaaCtx.hookSet = DAA_HookSet;
   gDaaCtx.hookGet = DAA_HookGet;
   gDaaCtx.Init    = DAA_Init;
   gDaaCtx.osiCfg  = DAA_OsiCfg;
   gDaaCtx.fhCfg   = DAA_FlashHookCfg;
   gDaaCtx.fhSet   = DAA_COM_FlashHook;
   gDaaCtx.ringGet = DAA_COM_RingGet;
   gDaaCtx.batGet  = DAA_COM_BatGet;
   gDaaCtx.apohGet = DAA_COM_ApohGet;
   gDaaCtx.polGet  = DAA_COM_PolGet;
   return (&gDaaCtx);
}


/** **************************************************************************
   daa channel initialisation
   \param nDaa    - daa resourcc number
   \return IFX_SUCCESS or IFX_ERROR in case of failure
*/
static IFX_return_t DAA_Init (IFX_uint16_t nDaa)
{
   int err = IFX_SUCCESS;

   if (nDaa >= DRV_DAA_MAX_DAA_CHANNELS)
   {
      TRACE(DAA_DRV, DBG_LEVEL_HIGH,
           ("DAA ERR daa resource %d not known\n\r", nDaa));
      return IFX_ERROR;
   }
   if (gDaaFctMap[nDaa].init == IFX_NULL)
   {
      TRACE(DAA_DRV, DBG_LEVEL_HIGH,
           ("DAA ERR daa resource %d didn't initialize .init fct ptr\n\r",
            nDaa));
      return IFX_ERROR;
   }

   /* initialize internal data structures, timers, etc */
   err = DAA_COM_init(nDaa);
   if (err != IFX_SUCCESS)
      TRACE(DAA_DRV, DBG_LEVEL_HIGH,
           ("DAA ERR DAA_COM_init failed for daa resource %d\n\r", nDaa));

   /* initialize board specifics */
   err = gDaaFctMap[nDaa].init();
   if (err != IFX_SUCCESS)
      TRACE(DAA_DRV, DBG_LEVEL_HIGH,
           ("DAA ERR DAA_board_init failed for daa resource %d\n\r", nDaa));

   /* initialize internal statemachines with their initial state
      (requires a) initialized timers and b) board initialisation */
   err = DAA_COM_InitStates(nDaa);
   if (err != IFX_SUCCESS)
      TRACE(DAA_DRV, DBG_LEVEL_HIGH,
           ("DAA ERR DAA_COM_InitStates for daa resource %d\n\r", nDaa));

   return err;
}

/** **************************************************************************
   daa global flash hook time configuration
   \param nFlashHookTime  - flash hook time in [ms]
   \return IFX_SUCCESS, no failure possible
*/
static IFX_return_t DAA_FlashHookCfg (IFX_uint32_t nFlashHookTime)
{
   DAA_COM_FlashHookTimeSet (nFlashHookTime);
   return IFX_SUCCESS;
}

/**  **************************************************************************
   daa global max. OSI time configuration
   \param nMaxOsiTime   - max Osi time in [ms]
   \return IFX_SUCCESS, no failure possible
*/
static IFX_return_t DAA_OsiCfg (IFX_uint32_t nMaxOsiTime)
{
   DAA_COM_MaxOsiTimeSet (nMaxOsiTime);
   return IFX_SUCCESS;
}

/** **************************************************************************
   daa hook status set
   set the hook state for a daa channel
   \param nDaa - daa resource number
   \param offHook - hook state
   \return IFX_SUCCES or IFX_ERROR in case of failure
*/
IFX_return_t DAA_HookSet (IFX_uint16_t nDaa, IFX_TAPI_FXO_HOOK_t offHook)
{
   /* sanity check */
   if (gDaaFctMap[nDaa].hookSet == IFX_NULL)
      return IFX_ERROR;

   return gDaaFctMap[nDaa].hookSet(offHook);
}

/** **************************************************************************
   daa hook status get
   read back the current hook state from a daa channel
   \param nDaa - daa resource number
   \param pOffHook - returns the current hook state
   \return IFX_SUCCESS or IFX_ERROR in case of failure
*/
IFX_return_t DAA_HookGet (IFX_uint16_t nDaa, IFX_TAPI_FXO_HOOK_t *pOffHook)
{
   /* sanity check */
   if (gDaaFctMap[nDaa].hookGet == IFX_NULL)
      return IFX_ERROR;

   return gDaaFctMap[nDaa].hookGet(pOffHook);
}

