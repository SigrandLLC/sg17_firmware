#ifndef DRV_VINETIC_QOS_H
#define DRV_VINETIC_QOS_H

#ifdef QOS_SUPPORT
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

 ****************************************************************************
   Module      : drv_vinetic_qos.c
   Date        : 2004-09-18
*******************************************************************************/

#ifndef LINUX
#error This feature is actually only available under Linux.
#endif

/* ============================= */
/* Includes                      */
/* ============================= */
#include "drv_vinetic_api.h"
#include "drv_tapi_ll_interface.h"

/* ============================= */
/* Local definitions             */
/* ============================= */

/* ============================= */
/* Local structures              */
/* ============================= */


/* ============================= */
/* Local variable definition     */
/* ============================= */

/* ============================= */
/* Global variable definition    */
/* ============================= */

/* ============================= */
/* Local function declaration    */
/* ============================= */

IFX_return_t Qos_LL_PktIngress(IFX_TAPI_LL_CH_t* pLLCh, IFX_void_t* pData,
                               IFX_uint32_t nLen);

IFX_return_t Qos_LL_PktEgress(IFX_TAPI_LL_CH_t* pLLCh);

IFX_return_t Qos_LL_Init(IFX_uint32_t devHandle);

/* ============================= */
/* Local function definition     */
/* ============================= */

#endif /* QOS_SUPPORT */

#endif /* DRV_VINETIC_QOS_H */
