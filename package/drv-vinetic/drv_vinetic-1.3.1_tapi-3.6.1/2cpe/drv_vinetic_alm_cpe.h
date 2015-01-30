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
   Module      : drv_vinetic_alm_cpe.h
   Date        : 2003-10-28
   Description : This file contains the declarations of vinetic 2CPE related tapi low level functions for ALM module.
*******************************************************************************/

#include "drv_tapi_ll_interface.h"

/* Switch polarity. */
IFX_int32_t IFX_TAPI_LL_ALM_CPE_Line_Polarity_Set (IFX_TAPI_LL_CH_t *pLLChannel);

/* Enable / disable the automatic battery switch. */
IFX_int32_t IFX_TAPI_LL_ALM_CPE_AutoBatterySwitch(IFX_TAPI_LL_CH_t *pLLChannel);

/*   Prepare parameters and call the Target Configuration Function to switch
 *   the  line mode */
IFX_int32_t IFX_TAPI_LL_ALM_CPE_Line_Mode_Set (IFX_TAPI_LL_CH_t *pLLChannel,
                                               IFX_int32_t nMode,
                                               IFX_uint8_t nTapiLineMode);

/** Switch line type either FXS or FXO. */
IFX_int32_t IFX_TAPI_LL_ALM_CPE_Line_Type_Set(IFX_TAPI_LL_CH_t *pLLChannel,
                                               IFX_TAPI_LINE_TYPE_t nType);

/* This service enables or disables a high level path of a phone channel. */
IFX_int32_t IFX_TAPI_LL_ALM_CPE_Volume_High_Level (IFX_TAPI_LL_CH_t *pLLChannel,
                                                   IFX_int32_t bEnable);

/*
   Prepare parameters and call the Target Configuration Function to set
   the ring configuration. */
IFX_int32_t IFX_TAPI_LL_ALM_CPE_Ring_Cfg (IFX_TAPI_LL_CH_t *pLLChannel,
                                           IFX_TAPI_RING_CFG_t const *pRingConfig);

/*   Configure metering mode of chip */
IFX_int32_t IFX_TAPI_LL_ALM_CPE_Metering_Cfg (IFX_TAPI_LL_CH_t *pLLChannel,
                                              IFX_uint8_t nMode,
                                              IFX_uint8_t nFreq);

/*   Restores the line state back after fault */
IFX_int32_t IFX_TAPI_LL_ALM_CPE_FaultLine_Restore (IFX_TAPI_LL_CH_t *pLLChannel);

void VINETIC_ALM_CPE_Func_Register (IFX_TAPI_DRV_CTX_ALM_t *pAlm);
