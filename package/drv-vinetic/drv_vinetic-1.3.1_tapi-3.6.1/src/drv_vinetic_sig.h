#ifndef _DRV_VINETIC_SIG_H
#define _DRV_VINETIC_SIG_H
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
   Module      : drv_vinetic_sig.h
   Description : This file contains the declaration of the functions for
                 the Signaling module
   Remarks     :
******************************************************************************/

/* ============================= */
/* Includes                      */
/* ============================= */
#include "ifx_types.h"

/* ============================= */
/* Global Defines                */
/* ============================= */

/* ============================= */
/* Local Macros & Definitions    */
/* ============================= */

/* ============================= */
/* Global Structures             */
/* ============================= */

/* ============================= */
/* Global function declaration   */
/* ============================= */
extern void VINETIC_SIG_Func_Register (IFX_TAPI_DRV_CTX_SIG_t *pSig);

extern IFX_return_t VINETIC_SIG_Allocate_Ch_Structures (VINETIC_CHANNEL *pCh);
extern IFX_void_t   VINETIC_SIG_Free_Ch_Structures (VINETIC_CHANNEL *pCh);
extern IFX_void_t   VINETIC_SIG_Init_Ch (VINETIC_CHANNEL *pCh);
extern IFX_return_t VINETIC_SIG_Set_Inputs (VINETIC_CHANNEL *pCh);
extern IFX_int32_t  VINETIC_SIG_baseConf (VINETIC_CHANNEL *pCh);

extern IFX_return_t VINETIC_SIG_RTP_OOB_Cfg (VINETIC_CHANNEL *pCh,
                                             IFX_TAPI_PKT_RTP_CFG_t const *pRtpConf);
extern IFX_int32_t  Dsp_SigActStatus (VINETIC_DEVICE *pDev, IFX_uint8_t ch,
                                      IFX_boolean_t bOn,
                       IFX_int32_t (*pCmdWrite) (VINETIC_DEVICE *pDev, IFX_uint16_t* pCmd, IFX_uint8_t count));

extern IFX_uint8_t  VINETIC_SIG_CPTD_Get_Status (VINETIC_DEVICE *pDev,
                                                 IFX_uint8_t ch);
extern IFX_int32_t  VINETIC_SIG_DTMFG_Get_Status (VINETIC_DEVICE *pDev,
                                                  IFX_uint8_t ch);
extern IFX_int32_t  VINETIC_SIG_DTMFG_Enable  (VINETIC_DEVICE *pDev,
                                               IFX_uint8_t ch);
extern IFX_int32_t  VINETIC_SIG_DTMFG_Disable (VINETIC_DEVICE *pDev,
                                               IFX_uint8_t ch);
extern IFX_uint16_t VINETIC_SIG_Event_Tx_Status_Get (VINETIC_DEVICE *pDev,
                                                     IFX_uint8_t ch);

/* Functions to serve interrupt events */

extern IFX_int32_t  irq_VINETIC_SIG_DtmfOnRequest  (VINETIC_CHANNEL *pCh);
extern IFX_int32_t  irq_VINETIC_SIG_DtmfOnActivate (VINETIC_CHANNEL *pCh,
                                                    IFX_boolean_t bAct);
extern IFX_int32_t  irq_VINETIC_SIG_DtmfOnUnderrun (VINETIC_CHANNEL *pCh);
extern IFX_int32_t  irq_VINETIC_SIG_DtmfStop       (VINETIC_CHANNEL *pCh,
                                                    IFX_int32_t nIsr);
extern void         irq_VINETIC_SIG_MFTD_Event     (VINETIC_CHANNEL *pCh,
                                                    IFX_uint8_t nVal,
                                                    IFX_boolean_t bRx);
extern IFX_uint8_t  irq_VINETIC_SIG_DTMF_encode_fw2tapi (IFX_uint8_t fwDtmfCode);
extern IFX_char_t   irq_VINETIC_SIG_DTMF_encode_fw2ascii (IFX_uint8_t fwDtmfCode);


/* Prototypes for functions which are exported via the function
   VINETIC_SIG_Func_Register(). The prototype for the function pointers are
   defined in drv_tapi_ll_interface.h and must be identical to the prototypes
   below. */

extern IFX_int32_t IFX_TAPI_LL_SIG_UTG_Start (IFX_TAPI_LL_CH_t *pLLChannel,
                                              IFX_TAPI_TONE_SIMPLE_t const *pSimpleTone,
                                              TAPI_TONE_DST dst,
                                              IFX_uint8_t res);

extern IFX_int32_t IFX_TAPI_LL_SIG_UTG_Stop (IFX_TAPI_LL_CH_t *pLLChannel,
                                             IFX_uint8_t res);

extern IFX_return_t IFX_TAPI_LL_Tone_Set_Level (IFX_TAPI_LL_CH_t *pLLCh,
                                                IFX_TAPI_PREDEF_TONE_LEVEL_t const *pToneLevel);

extern IFX_uint8_t IFX_TAPI_LL_SIG_UTG_Count_Get (IFX_TAPI_LL_CH_t *pLLChannel);

extern IFX_void_t IFX_TAPI_LL_UTG_Event_Deactivated (IFX_TAPI_LL_CH_t *pLLChannel,
                                                     IFX_uint8_t utgNum);

extern IFX_int32_t  IFX_TAPI_LL_SIG_DTMFG_Cfg (IFX_TAPI_LL_CH_t *pLLChannel,
                                               IFX_uint16_t nInterDigitTime,
                                               IFX_uint16_t nDigitPlayTime);

extern IFX_int32_t  IFX_TAPI_LL_SIG_DTMFG_Start (IFX_TAPI_LL_CH_t *pLLChannel,
                                                 IFX_uint8_t nDigits,
                                                 IFX_char_t  *data);

extern IFX_int32_t  IFX_TAPI_LL_SIG_DTMFG_Stop (IFX_TAPI_LL_CH_t *pLLChannel);

extern IFX_int32_t IFX_TAPI_LL_SIG_DTMFD_OOB (IFX_TAPI_LL_CH_t *pLLChannel,
                                               IFX_TAPI_PKT_EV_OOB_t nOobMode);

extern IFX_return_t IFX_TAPI_LL_SIG_DTMF_RX_CFG (IFX_TAPI_LL_CH_t *pLLChannel,
                                                 IFX_boolean_t bRW,
                                                 IFX_TAPI_DTMF_RX_CFG_t *pDtmfRxCoeff);

extern IFX_return_t VINETIC_SIG_DTMF_encode_ascii2fw (IFX_char_t nChar,
                                                       IFX_uint8_t *pDtmfCode);

extern IFX_return_t IFX_TAPI_LL_SIG_CPTD_Start (IFX_TAPI_LL_CH_t *pLLChannel,
                                                IFX_TAPI_TONE_SIMPLE_t const *pTone,
                                                IFX_int32_t signal);

extern IFX_return_t IFX_TAPI_LL_SIG_CPTD_Stop  (IFX_TAPI_LL_CH_t *pLLChannel);

extern IFX_return_t IFX_TAPI_LL_SIG_MFTD_Enable (IFX_TAPI_LL_CH_t *pLLChannel,
                                                 IFX_TAPI_SIG_DETECTION_t const *pSig);
extern IFX_return_t IFX_TAPI_LL_SIG_MFTD_Disable (IFX_TAPI_LL_CH_t *pLLChannel,
                                                  IFX_TAPI_SIG_DETECTION_t const *pSig);

extern IFX_return_t IFX_TAPI_LL_SIG_MFTD_Signal_Set (IFX_TAPI_LL_CH_t *pLLChannel,
                                                     IFX_uint32_t signal);

extern IFX_return_t IFX_TAPI_LL_SIG_MFTD_Signal_Get (IFX_TAPI_LL_CH_t *pLLChannel,
                                                     IFX_uint32_t *signal);

extern IFX_return_t IFX_TAPI_LL_SIG_MFTD_Signal_Ext_Set (IFX_TAPI_LL_CH_t *pLLChannel,
                                                         IFX_uint32_t signalExt);

extern IFX_return_t IFX_TAPI_LL_SIG_MFTD_Signal_Ext_Get (IFX_TAPI_LL_CH_t *pLLChannel,
                                                         IFX_uint32_t *signalExt);

extern IFX_return_t IFX_TAPI_LL_SIG_MFTD_Signal_Enable (IFX_TAPI_LL_CH_t *pLLChannel,
                                                        IFX_TAPI_SIG_t signal);

extern IFX_return_t IFX_TAPI_LL_SIG_MFTD_Signal_Disable (IFX_TAPI_LL_CH_t *pLLChannel,

                                                         IFX_TAPI_SIG_t signal);

/* this structure is used to store status information of
   all signaling modules. It is used to store the event transmission
   status in the sigch structure and to switch off and on the modules */
/* \todo use different structures for internal storage and function parameter */
typedef union
{
   IFX_uint16_t value;
   struct {
      unsigned atd1                          : 1;
      unsigned atd2                          : 1;
      unsigned utd1                          : 1;
      unsigned utd2                          : 1;
      unsigned dtmf_rec                      : 1;
      unsigned dtmfgen                       : 1;
   } flag;
} SIG_MOD_STATE;

IFX_int32_t VINETIC_SIG_UpdateEventTrans (VINETIC_DEVICE *pDev, IFX_uint8_t ch,
                                          SIG_MOD_STATE *pState, SIG_MOD_STATE* apply);

#endif /* _DRV_VINETIC_SIG_H */
