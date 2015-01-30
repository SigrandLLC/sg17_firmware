#ifndef _DRV_VINETIC_ALM_H
#define _DRV_VINETIC_ALM_H
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
   Module      : drv_vinetic_alm.h
   Description : This file contains the defines, the function prototypes and
                 declarations for the ALM module.
******************************************************************************/

/* ============================= */
/* Includes                      */
/* ============================= */
#include "drv_vinetic_api.h"
#include "drv_tapi_ll_interface.h"

/* ============================= */
/* Global Defines                */
/* ============================= */

/* ============================= */
/* Global Types                  */
/* ============================= */

/* ============================= */
/* Global Variables              */
/* ============================= */
/* calculated table to set the Rx/Tx Gain in the ALM module.
   Converts the gain in 'dB' into the firmware values */
/* This table is shared with the PCM module */
extern const  IFX_uint8_t VINETIC_AlmPcmGain [49];

/* ============================= */
/* Global function declaration   */
/* ============================= */
extern void VINETIC_ALM_Func_Register (IFX_TAPI_DRV_CTX_ALM_t *pAlm);

extern IFX_return_t VINETIC_ALM_Allocate_Ch_Structures (VINETIC_CHANNEL *pCh);
extern IFX_void_t   VINETIC_ALM_Free_Ch_Structures (VINETIC_CHANNEL *pCh);
extern IFX_void_t   VINETIC_ALM_Init_Ch (VINETIC_CHANNEL *pCh);
extern IFX_return_t VINETIC_ALM_Set_Inputs (VINETIC_CHANNEL *pCh);
extern IFX_int32_t  VINETIC_ALM_baseConf (VINETIC_CHANNEL *pCh);

/** Linemode services */
extern IFX_int32_t TAPI_LL_Phone_SwitchLine (TAPI_CHANNEL *pChannel,
                                             IFX_int32_t nMode);

extern IFX_int32_t TAPI_LL_Phone_High_Level (TAPI_CHANNEL *pChannel,
                                             IFX_int32_t bEnable);

/** Ringing services */
extern IFX_return_t TAPI_LL_Phone_Ring_Config(TAPI_CHANNEL *pChannel,
                                              IFX_TAPI_RING_CFG_t const *pRingConfig);

/** Set the phone volume */
extern IFX_int32_t IFX_TAPI_LL_ALM_Volume_Set (IFX_TAPI_LL_CH_t *pLLChannel,
                                               IFX_TAPI_LINE_VOLUME_t const *pVol);

/** Set the LEC configuration */
extern IFX_int32_t IFX_TAPI_LL_ALM_Lec_Cfg (IFX_TAPI_LL_CH_t *pLLChannel,
                                             TAPI_LEC_DATA_t const *pLecConf);

#endif /* _DRV_VINETIC_ALM_H */
