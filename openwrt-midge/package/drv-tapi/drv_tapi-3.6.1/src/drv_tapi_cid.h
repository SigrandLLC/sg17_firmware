#ifndef _DRV_TAPI_CID_H
#define _DRV_TAPI_CID_H
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

*******************************************************************************/

/**
   \file drv_tapi_cid.h
   Interface of the TAPI caller id implementation.
   This file contains the declaration of the functions for the CID module.
*/

extern IFX_int32_t TAPI_Phone_CID_SetDefaultConfig    (TAPI_CHANNEL *pChannel);
extern IFX_int32_t TAPI_Phone_CID_SetConfig           (TAPI_CHANNEL *pChannel, IFX_TAPI_CID_CFG_t const *pCidConf);

extern IFX_int32_t TAPI_Phone_CID_Info_Tx             (TAPI_CHANNEL *pChannel, IFX_TAPI_CID_MSG_t const *pCidInfo);
extern IFX_int32_t TAPI_Phone_CID_Seq_Tx              (TAPI_CHANNEL *pChannel, IFX_TAPI_CID_MSG_t const *pCidInfo);
extern IFX_int32_t TAPI_Phone_CID_Stop_Tx             (TAPI_CHANNEL *pChannel);

extern IFX_int32_t TAPI_Phone_CidRx_Start             (TAPI_CHANNEL *pChannel, IFX_TAPI_CID_HOOK_MODE_t cidHookMode);
extern IFX_int32_t TAPI_Phone_CidRx_Stop              (TAPI_CHANNEL *pChannel);
extern IFX_int32_t TAPI_Phone_Get_CidRxData           (TAPI_CHANNEL *pChannel, IFX_TAPI_CID_RX_DATA_t *pCidRxData);

/* timer callback function */
extern IFX_void_t  TAPI_Phone_CID_OnTimer             (Timer_ID Timer, IFX_int32_t nArg);
/* trigger function called by ringing to indicate a ring pause */
extern IFX_void_t  TAPI_Phone_CID_OnRingpause         (TAPI_CHANNEL *pChannel);

/* exported for use by LL driver */
extern IFX_TAPI_CID_RX_DATA_t *TAPI_Phone_GetCidRxBuf (TAPI_CHANNEL *pChannel, IFX_uint32_t nLen);
#endif /* _DRV_TAPI_CID_H */
