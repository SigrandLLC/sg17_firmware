#ifndef _DRV_VINETIC_PCM_H
#define _DRV_VINETIC_PCM_H
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
   Module      : drv_vinetic_pcm.h
   Description : This file contains the defines and the global functions
                 declarations of PCM module
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

/* ============================= */
/* Global function declaration   */
/* ============================= */
extern IFX_void_t   VINETIC_PCM_Func_Register   (IFX_TAPI_DRV_CTX_PCM_t *pPCM);

extern IFX_return_t VINETIC_PCM_Allocate_Ch_Structures (VINETIC_CHANNEL *pCh);
extern IFX_void_t   VINETIC_PCM_Free_Ch_Structures (VINETIC_CHANNEL *pCh);
extern IFX_void_t   VINETIC_PCM_Init_Ch         (VINETIC_CHANNEL *pCh,
                                                 IFX_uint8_t pcmCh);
extern IFX_return_t VINETIC_PCM_Set_Inputs      (VINETIC_CHANNEL *pCh);
extern IFX_int32_t  VINETIC_PCM_baseConf        (VINETIC_CHANNEL *pCh);

extern IFX_return_t IFX_TAPI_LL_PCM_Cfg         (IFX_TAPI_LL_CH_t *pLLChannel,
                                                 IFX_TAPI_PCM_CFG_t const *pPCMConfig);
extern IFX_return_t IFX_TAPI_LL_PCM_Enable      (IFX_TAPI_LL_CH_t *pLLChannel,
                                                 IFX_uint32_t nMode,
                                                 IFX_TAPI_PCM_CFG_t *pPcmCfg);
extern IFX_return_t IFX_TAPI_LL_PCM_Volume_Set  (IFX_TAPI_LL_CH_t *pLLChannel,
                                                 IFX_TAPI_LINE_VOLUME_t const *pVol);
extern IFX_return_t IFX_TAPI_LL_PCM_Lec_Cfg     (IFX_TAPI_LL_CH_t *pLLChannel,
                                                 TAPI_LEC_DATA_t const *pLecConf);

#endif
