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

******************************************************************************
   Module      : drv_vinetic_dspconf.c
   Description : This file contains the implementations of vinetic dsp
                 related configurations
   Remarks:
      the functions implemented in this module must be protected from calling
      functions against concurrent access or interrupts susceptible to modify
      the contain of cached fw messages. The share variable mutex should be used
      for locking.
*******************************************************************************/

/** \file
   This file contains DSP specific functions like signal detection,
   tone generation.    */

/* ============================= */
/* includes                      */
/* ============================= */
#include "drv_vinetic_api.h"

#if (VIN_CFG_FEATURES & (VIN_FEAT_VIN_C | VIN_FEAT_VIN_M | VIN_FEAT_VIN_2CPE))

#include "drv_vinetic_dspconf.h"
#include "drv_vinetic_basic.h"



/* ============================= */
/*             Enums             */
/* ============================= */

/* ============================= */
/* Local structures              */
/* ============================= */



/* ============================= */
/* Local variable definition     */
/* ============================= */







/* ============================= */
/* Global function definitions   */
/* ============================= */

/**
   Disables or Enables DSP Idle mode

   \param   pDev  - pointer to VINETIC device structure
   \param   bEn   - IFX_TRUE : enable / IFX_FALSE : disable

   \return
      IFX_SUCCESS or IFX_ERROR

*/
IFX_int32_t Dsp_SetIdleMode (VINETIC_DEVICE *pDev, IFX_boolean_t bEn)
{
   IFX_int32_t    ret;
   IFX_int16_t    pCmd[3] =  {0x0600, 0xE501, 0x0000};

   /* switch Idle Mode on or off */
   pCmd[2] = (bEn) ? 0x8000 : 0x0000;
   ret = CmdWrite (pDev, &pCmd[0], 1);
   return ret;
}











#endif /* (VIN_FEAT_VIN_C | VIN_FEAT_VIN_M | VIN_FEAT_VIN_2CPE) */

