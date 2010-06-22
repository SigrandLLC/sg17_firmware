#ifndef _DRV_VINETIC_COD_H
#define _DRV_VINETIC_COD_H
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
   Module      : drv_vinetic_cod.h
   Description : This file contains the defines
                 and the global functions declarations of Coder module
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
extern void VINETIC_COD_Func_Register (IFX_TAPI_DRV_CTX_COD_t *pCOD);

extern IFX_return_t VINETIC_COD_Allocate_Ch_Structures (VINETIC_CHANNEL *pCh);
extern IFX_void_t   VINETIC_COD_Free_Ch_Structures (VINETIC_CHANNEL *pCh);
extern IFX_void_t   VINETIC_COD_Init_Ch (VINETIC_CHANNEL *pCh);
extern IFX_return_t VINETIC_COD_Set_Inputs (VINETIC_CHANNEL *pCh);
extern IFX_int32_t  VINETIC_COD_baseConf (VINETIC_CHANNEL *pCh);

extern IFX_boolean_t VINETIC_COD_ChStatusGet(VINETIC_CHANNEL *pCh);

/* VINETIC_FAX_T38 related functions */
/* =============================================================== */

/* Configures the Datapump for Modulation */
IFX_return_t IFX_TAPI_LL_COD_T38_Mod_Enable (IFX_TAPI_LL_CH_t *pLLChannel,
                                             IFX_TAPI_T38_MOD_DATA_t const  *pFaxMod);

/* Configures the Datapump for Demodulation */
IFX_return_t IFX_TAPI_LL_COD_T38_DeMod_Enable (IFX_TAPI_LL_CH_t *pLLChannel,
                                               IFX_TAPI_T38_DEMOD_DATA_t const *pFaxDemod);

/*  disables the Fax datapump */
IFX_return_t IFX_TAPI_LL_COD_T38_Datapump_Disable (IFX_TAPI_LL_CH_t *pLLChannel);

/* query Fax Status */
IFX_return_t IFX_TAPI_LL_COD_T38_Status_Get (IFX_TAPI_LL_CH_t *pLLChannel,
                                             IFX_TAPI_T38_STATUS_t *pFaxStatus);

/* Set Fax Status */
IFX_return_t IFX_TAPI_LL_COD_T38_Status_Set (IFX_TAPI_LL_CH_t *pLLChannel,
                                             unsigned char status);

/* Set Fax Error status */
IFX_return_t IFX_TAPI_LL_COD_T38_Error_Set (IFX_TAPI_LL_CH_t *pLLChannel,
                                            unsigned char error);

/* =============================================================== */


/* CODER functions */
/* =============================================================== */
extern IFX_return_t IFX_TAPI_LL_COD_ENC_Cfg_Set (IFX_TAPI_LL_CH_t *pLLChannel,
                                                 IFX_TAPI_COD_TYPE_t nCoder,
                                                 IFX_TAPI_COD_LENGTH_t nFrameLength,
                                                 IFX_TAPI_COD_AAL2_BITPACK_t nBitPack);

extern IFX_return_t IFX_TAPI_LL_COD_DEC_Cfg_Set (IFX_TAPI_LL_CH_t *pLLChannel,
                                                 IFX_TAPI_COD_AAL2_BITPACK_t nBitPack);

IFX_return_t IFX_TAPI_LL_COD_ENC_CoderType_Set (IFX_TAPI_LL_CH_t *pLLChannel,
                                                IFX_int32_t nCoder);

IFX_return_t IFX_TAPI_LL_COD_ENC_FrameLength_Set (IFX_TAPI_LL_CH_t *pLLChannel,
                                                  IFX_int32_t nFrameLength);

IFX_return_t IFX_TAPI_LL_COD_ENC_FrameLength_Get (IFX_TAPI_LL_CH_t *pLLChannel,
                                                  IFX_int32_t *pFrameLength);

IFX_return_t IFX_TAPI_LL_COD_VAD_Cfg (IFX_TAPI_LL_CH_t *pLLChannel,
                                      IFX_int32_t nVAD);

IFX_return_t IFX_TAPI_LL_COD_AGC_Cfg(IFX_TAPI_LL_CH_t *pLLChannel,
                                     IFX_TAPI_ENC_AGC_CFG_t *pParam);

IFX_return_t IFX_TAPI_LL_COD_AGC_Enable(IFX_TAPI_LL_CH_t *pLLChannel,
                                        IFX_TAPI_ENC_AGC_MODE_t Param);

IFX_return_t IFX_TAPI_LL_COD_DEC_Start (IFX_TAPI_LL_CH_t *pLLChannel);

IFX_return_t IFX_TAPI_LL_COD_DEC_Stop (IFX_TAPI_LL_CH_t *pLLChannel);

IFX_return_t IFX_TAPI_LL_COD_ENC_Start (IFX_TAPI_LL_CH_t *pLLChannel);

IFX_return_t IFX_TAPI_LL_COD_ENC_Stop (IFX_TAPI_LL_CH_t *pLLChannel);

IFX_return_t IFX_TAPI_LL_COD_ENC_Hold(IFX_TAPI_LL_CH_t *pLLChannel,
                                      IFX_operation_t nOnHold);

IFX_return_t IFX_TAPI_LL_COD_RTCP_Get (IFX_TAPI_LL_CH_t *pLLChannel,
                                       IFX_TAPI_PKT_RTCP_STATISTICS_t *pRTCP);

IFX_return_t IFX_TAPI_LL_COD_RTCP_Prepared_Get (IFX_TAPI_LL_CH_t *pLLChannel,
                                                IFX_TAPI_PKT_RTCP_STATISTICS_t *pRTCP);

IFX_return_t IFX_TAPI_LL_COD_RTCP_Prepare_Unprot (IFX_TAPI_LL_CH_t *pLLChannel);

IFX_return_t IFX_TAPI_LL_COD_RTP_Cfg (IFX_TAPI_LL_CH_t *pLLChannel,
                                      IFX_TAPI_PKT_RTP_CFG_t const *pRtpConf);

IFX_return_t IFX_TAPI_LL_COD_JB_Stat_Reset (IFX_TAPI_LL_CH_t *pLLChannel);

IFX_return_t IFX_TAPI_LL_COD_JB_Stat_Get (IFX_TAPI_LL_CH_t *pLLChannel,
                                          IFX_TAPI_JB_STATISTICS_t *pJbData);

IFX_return_t IFX_TAPI_LL_COD_JB_Cfg (IFX_TAPI_LL_CH_t *pLLChannel,
                                     IFX_TAPI_JB_CFG_t const *pJbConf);

IFX_return_t IFX_TAPI_LL_COD_RTCP_Reset (IFX_TAPI_LL_CH_t *pLLChannel);

IFX_return_t IFX_TAPI_LL_COD_RTP_PayloadTable_Cfg (IFX_TAPI_LL_CH_t *pLLChannel,
                                                   IFX_TAPI_PKT_RTP_PT_CFG_t const *pRtpPTConf);

IFX_return_t IFX_TAPI_LL_COD_RTP_EventGenerate (IFX_TAPI_LL_CH_t *pLLChannel,
                                                IFX_uint8_t nEvent,
                                                IFX_boolean_t bStart,
                                                IFX_uint8_t nDuration);

#if (VIN_CFG_FEATURES & VIN_FEAT_VOICE)
extern IFX_return_t IFX_TAPI_LL_COD_ENC_RoomNoise (IFX_TAPI_LL_CH_t *pLLChannel,
                                                   IFX_boolean_t bEnable,
                                                   IFX_uint32_t nThreshold,
                                                   IFX_uint8_t nVoicePktCnt,
                                                   IFX_uint8_t nSilencePktCnt);

void irq_VINETIC_COD_RoomNoise_PktAnalysis (VINETIC_CHANNEL *pCh,
                                            IFX_uint8_t nPT);
#endif /* (VIN_CFG_FEATURES & VIN_FEAT_VOICE) */

/* =============================================================== */

#endif
