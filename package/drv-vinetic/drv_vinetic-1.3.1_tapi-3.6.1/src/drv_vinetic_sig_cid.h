#ifndef _DRV_VINETIC_CID_H
#define _DRV_VINETIC_CID_H
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
   Module      : drv_vinetic_cid.h
   Description : This file contains the declaration of the functions
                 for CID operations
*******************************************************************************/

/* ============================= */
/* Check if feature is enabled   */
/* ============================= */
#ifdef HAVE_CONFIG_H
#include <drv_config.h>
#endif

#ifdef TAPI_CID

/* ============================= */
/* Global Defines                */
/* ============================= */

/* ============================= */
/* Global Structures             */
/* ============================= */

/* ============================= */
/* Global function declaration   */
/* ============================= */

extern IFX_int32_t  VINETIC_CidFskMachine  (VINETIC_CHANNEL *pCh);
extern IFX_void_t   VINETIC_CidDtmfMachine (VINETIC_CHANNEL *pCh);
extern IFX_return_t VINETIC_SIG_CID_Sender_MSG_Reset (VINETIC_CHANNEL *pCh);

extern IFX_return_t VINETIC_SIG_CID_RX_Data_Collect (TAPI_CHANNEL *pChannel);

/* Prototypes for functions which are exported via the function 
   VINETIC_SIG_Func_Register(). The prototype for the function pointers are 
   defined in drv_tapi_ll_interface.h and must be identical to the prototypes 
   below. */

extern IFX_int32_t IFX_TAPI_LL_SIG_CID_TX_Start (IFX_TAPI_LL_CH_t *pLLChannel,
                                                 TAPI_CID_DATA_t const *pCid,
                                                 TAPI_CID_CONF_t *pCidConf,
                                                 IFX_TAPI_CID_DTMF_CFG_t *pDtmfConf,
                                                 IFX_TAPI_CID_FSK_CFG_t *pCidFskConf);

extern IFX_int32_t IFX_TAPI_LL_SIG_CID_TX_Stop (IFX_TAPI_LL_CH_t *pLLChannel,
                                                TAPI_CID_CONF_t *pCidConf);

extern IFX_return_t IFX_TAPI_LL_SIG_CID_RX_Start (IFX_TAPI_LL_CH_t *pLLChannel,
                                                  IFX_TAPI_CID_HOOK_MODE_t cidHookMode,
                                                  IFX_TAPI_CID_FSK_CFG_t *pCidFskCfg);

extern IFX_return_t IFX_TAPI_LL_SIG_CID_RX_Stop (IFX_TAPI_LL_CH_t *pLLChannel,
                                                 IFX_TAPI_CID_FSK_CFG_t *pCidFskCfg);

#endif /* TAPI_CID */
#endif /* _DRV_VINETIC_CID_H */
