#ifndef _DRV_VINETIC_INIT_H
#define _DRV_VINETIC_INIT_H
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
   Module      : drv_vinetic_init.h
*******************************************************************************/

/* ============================= */
/* Includes                      */
/* ============================= */

#include "drv_vinetic_api.h"


/* ============================= */
/* Global Defines                */
/* ============================= */

/* maximum number of capabilities, used for allocating array */
#define MAX_CAPS              32

/* ============================= */
/* Global Types                  */
/* ============================= */


/* ============================= */
/* Global Variables              */
/* ============================= */


/* ============================= */
/* Global function declaration   */
/* ============================= */

extern IFX_return_t TAPI_LL_Phone_Init(TAPI_CHANNEL *pChannel,
                                       IFX_TAPI_CH_INIT_t const *pInit);
extern IFX_uint32_t TAPI_LL_Phone_Get_Capabilities (IFX_TAPI_LL_DEV_t *pLLDev);
extern IFX_return_t TAPI_LL_Phone_Get_Capability_List (IFX_TAPI_LL_DEV_t *pLLDev,
                                                       IFX_TAPI_CAP_t *pCapList);
extern IFX_int32_t  TAPI_LL_Phone_Check_Capability (IFX_TAPI_LL_DEV_t *pLLDev,
                                                    IFX_TAPI_CAP_t *pCapList);

extern void         VINETIC_Set_DevCaps(VINETIC_DEVICE *pDev);
extern IFX_int32_t  VINETIC_InitDevMember (VINETIC_DEVICE *pDev);
extern IFX_int32_t  VINETIC_InitDevFWData (VINETIC_DEVICE *pDev);
extern IFX_int32_t  VINETIC_UpdateChMember (IFX_TAPI_LL_CH_t *pLLCh);

extern IFX_int32_t  VINETIC_AddCaps (VINETIC_DEVICE *pDev);
extern IFX_return_t VINETIC_Get_FwCap (VINETIC_DEVICE *pDev);

extern IFX_TAPI_LL_DEV_t* VINETIC_Prepare_Dev(TAPI_DEV* pTapiDev,
                                              VINETIC_DEVICE* pDev,
                                              IFX_int32_t devNum);
extern IFX_void_t*  VINETIC_Prepare_Ch(TAPI_CHANNEL* pTapiCh,
                                      VINETIC_DEVICE* pDev,
                                      IFX_uint32_t chNum);
extern IFX_int32_t  VINETIC_DeviceDriverInit(IFX_void_t);

#endif /* _DRV_VINETIC_INIT_H */
