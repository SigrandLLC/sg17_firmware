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
   Module      : drv_tapi_common.c
   Date        : 2005/09/05
   Description : Common functions for the TAPI Driver

   \remark
   1. This driver is loaded before all others low level drivers are loaded.
   2. The low level driver using the TAPI interface must registers itself with
   TAPI_DrvRegister. This creates a context structure which is passed back
   to the low level driver (LLDrv).
   3. The LLDrv sets all function pointers to the low level functions and
      calls TAPI_DrvInit. This sets the state to initialized.

******************************************************************************/

/* ============================= */
/* Includes                      */
/* ============================= */

#include "drv_api.h"
#include "drv_tapi.h"
#include "drv_tapi_api.h"

/* ============================= */
/* Local Macros & Definitions    */
/* ============================= */

/* ============================= */
/* Global variable definition    */
/* ============================= */

/** what string support, driver version string */
const char TAPI_WHATVERSION[] = DRV_TAPI_WHAT_STR;

/** pointer to device structures. */
TAPI_DEV* TAPI_Devices[MAX_TAPI_INSTANCES] = {};

/** trace group implementation */
CREATE_TRACE_GROUP(TAPI_DRV);
/** log group implementation */
CREATE_LOG_GROUP(TAPI_DRV);

/* ============================= */
/* Local function declaration    */
/* ============================= */

/* ============================= */
/* Local variable definition     */
/* ============================= */

/* ============================= */
/* Local function definition     */
/* ============================= */

/* ============================= */
/* Global function definition    */
/* ============================= */

/**
   Initializes the corresponding driver instance

   \param pDev Pointer to high-level TAPI device

   \return
   IFX_TRUE     Success
   IFX_FALSE    in case of error
*/
IFX_uint32_t TAPI_Drv_Init(TAPI_DEV *pDev)
{
    /* configure chip selects */
    TRACE(TAPI_DRV, DBG_LEVEL_NORMAL, ("TAPI_DRV: init device\r\n\r"));

    return IFX_TRUE;
}


#ifdef TAPI_SUPPORT_RESET
/**
   Reset TAPI Device

   \param pDev private device data
   \param sel  optional parameter

   \return
   IFX_TRUE    Success
   IFX_FALSE   in case of error
*/
IFX_uint32_t TAPI_Reset_TAPIDevice(TAPI_DEV *pDev, IFX_uint32_t sel)
{
    TRACE(TAPI_DRV, DBG_LEVEL_NORMAL,("TAPI_DRV: Reset TAPI Device\r\n"));

    return IFX_FALSE;
}
#endif /* TAPI_SUPPORT_RESET */
