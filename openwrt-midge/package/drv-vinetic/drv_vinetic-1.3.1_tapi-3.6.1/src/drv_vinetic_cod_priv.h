#ifndef _DRV_VINETIC_COD_PRIV_H
#define _DRV_VINETIC_COD_PRIV_H
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
   Module      : drv_vinetic_cod_priv.h
   Description : This file contains the defines and structures declarations
                 of Coder module
*******************************************************************************/
/* includes */
#include "ifx_types.h"
#include "sys_drv_ifxos.h"
#include "drv_vinetic.h"
#include "drv_vinetic_api.h"



#define CMD_COD_CH_LEN 5

/* header words amount */
#define CMD_HEADER_CNT 2

typedef enum
{
   /* coder is idle */
   vinetic_cod_OPMODE_IDLE  = 0,
   /* coder is running voice */
   vinetic_cod_OPMODE_VOICE,
   /* coder is running room-noise-detection */
   vinetic_cod_OPMODE_ROOMNOISE
} vinetic_cod_OPMODE_t;


/* Struture definitions */

typedef union
{
   IFX_uint16_t value[CMD_HEADER_CNT + CMD_COD_CH_LEN];
   struct {
#if (__BYTE_ORDER == __LITTLE_ENDIAN)
      /* cmd 1 */
      unsigned ch                            : 4;
      unsigned res                           : 4;
      unsigned cmd                           : 5;
      unsigned bc                            : 1;
      unsigned res0                          : 1;
      unsigned rw                            : 1;
      /* cmd 2 */
      unsigned length                        : 8;
      unsigned ecmd                          : 5;
      unsigned mod                           : 3;
      /* data */
      unsigned i1                            : 6;
      unsigned em                            : 1;
      unsigned ns                            : 1;
      unsigned codnr                         : 4;
      unsigned res1                          : 3;
      unsigned en                            : 1;

      unsigned enc                           : 5;
      unsigned pte                           : 3;
      unsigned sc                            : 1;
      unsigned pst                           : 1;
      unsigned im                            : 1;
      unsigned dec                           : 1;
      unsigned bfi                           : 1;
      unsigned cng                           : 1;
      unsigned pf                            : 1;
      unsigned hp                            : 1;

      unsigned gain2                         : 8;
      unsigned gain1                         : 8;

      unsigned i3                            : 6;
      unsigned res4                          : 2;
      unsigned i2                            : 6;
      unsigned bitalign_enc_aal              : 1;
      unsigned bitalign_dec_aal              : 1;

      unsigned i5                            : 6;
      unsigned res6                          : 2;
      unsigned i4                            : 6;
      unsigned res5                          : 2;
#endif /* LITTLE_ENDIAN */
#if (__BYTE_ORDER == __BIG_ENDIAN)
      /* cmd 1 */
      unsigned rw                            : 1;
      unsigned res0                          : 1;
      unsigned bc                            : 1;
      unsigned cmd                           : 5;
      unsigned res                           : 4;
      unsigned ch                            : 4;
      /* cmd 2 */
      unsigned mod                           : 3;
      unsigned ecmd                          : 5;
      unsigned length                        : 8;
      /* data */
      unsigned en                            : 1;
      unsigned res1                          : 3;
      unsigned codnr                         : 4;
      unsigned ns                            : 1;
      unsigned em                            : 1;
      unsigned i1                            : 6;

      unsigned hp                            : 1;
      unsigned pf                            : 1;
      unsigned cng                           : 1;
      unsigned bfi                           : 1;
      unsigned dec                           : 1;
      unsigned im                            : 1;
      unsigned pst                           : 1;
      unsigned sc                            : 1;
      unsigned pte                           : 3;
      unsigned enc                           : 5;

      unsigned gain1                         : 8;
      unsigned gain2                         : 8;

      unsigned bitalign_dec_aal              : 1;
      unsigned bitalign_enc_aal              : 1;
      unsigned i2                            : 6;
      unsigned res4                          : 2;
      unsigned i3                            : 6;

      unsigned res5                          : 2;
      unsigned i4                            : 6;
      unsigned res6                          : 2;
      unsigned i5                            : 6;
#endif /* BIG_ENDIAN */
   } bit;
} FWM_COD_CH;

/** Structure for the coder channel
   including firmware message cache */
struct VINETIC_CODCH
{
   /** firmware message cache */
   FWM_COD_CH        fw_cod_ch;
   /** local storage for the configured encoder. Stored outside the fw message
       so that decoder can be started while encoder is stopped. Value is set
       in 'cod_ch' when the coder gets started and upstream is not muted */
   IFX_uint8_t       enc_conf;
   /** local storage for RTP statistics */
   IFX_uint16_t      rtcp[14];
   IFX_boolean_t     rtcp_update;
   /** store currently configured SSRC */
   IFX_uint32_t      nSsrc;
   /** store currently configured sequence number */
   IFX_uint32_t      nSeqNr;
   /** stores the number of received bytes after jitter buffer statistics
       command. Because the firmware does not provide the high dword part
       the driver has to handle the overflow. This is done by comparing the
       new and the old value. If the new value is smaller, there is
       an overflow and high is increased */
   IFX_uint32_t      nRecBytesH;
   /** stores the low DWORD count for comparism */
   IFX_uint32_t      nRecBytesL;
   /** stores the number of received bytes after event statistics
       command. Because the firmware does not provide the high DWORD part
       the driver has to handle the overflow. This is done by comparing the
       new and the old value. If the new value is smaller, there is
       an overflow and high is increased */
   IFX_uint32_t      nRecEvtBytesH;
   /** stores the low DWORD count for comparison */
   IFX_uint32_t      nRecEvtBytesL;
   /** stores the current operating mode */
   vinetic_cod_OPMODE_t  nOpMode;
   /** threshold counter for room-noise detection */
   IFX_uint32_t      nThresholdCtr;
   /** count of consecutive voice packets needed for an event */
   IFX_uint8_t       nVoicePktCnt;
   /** count of consecutive silence packets needed for an event */
   IFX_uint8_t       nSilencePktCnt;
   /** last PT for room-noise detection */
   IFX_uint32_t      nLastPktPt;
   /** last PT that was reported in an event of room-noise detection */
   IFX_uint32_t      nLastEventPt;
   /** storage for PT table while room-noise is configured */
   IFX_uint16_t      pRtpPtCmd[20];
};

#if (VIN_CFG_FEATURES & VIN_FEAT_VOICE)
IFX_uint8_t  VINETIC_COD_trans_cod_tapi2fw (IFX_TAPI_COD_TYPE_t nCoder);
IFX_return_t VINETIC_COD_Voice_Enable (VINETIC_CHANNEL *pCh, IFX_uint8_t nMode);
#endif /* (VIN_CFG_FEATURES & VIN_FEAT_VOICE) */

#endif /* _DRV_VINETIC_COD_PRIV_H */
