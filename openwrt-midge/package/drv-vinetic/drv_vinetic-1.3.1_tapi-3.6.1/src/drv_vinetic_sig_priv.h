#ifndef _DRV_VINETIC_SIG_PRIV_H
#define _DRV_VINETIC_SIG_PRIV_H
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
   Module      : drv_vinetic_sig_priv.h
   Description :
   Remarks     :
******************************************************************************/

/* ============================= */
/* Includes                      */
/* ============================= */
#include "ifx_types.h"
#include "sys_drv_ifxos.h"
#include "drv_vinetic.h"

#include "drv_tapi_io.h"
#include "drv_tapi_ll_interface.h"
#include "drv_vinetic_api.h"
#include "drv_vinetic_sig.h"

/* ============================= */
/* Global Defines                */
/* ============================= */
#define VIN_SIG_TX 1
#define VIN_SIG_RX 2

#define LL_TAPI_TONE_MAXRES 2

/** return the maximum */
#define MAX(a, b) ( (a)>(b) ? (a) : (b) )


/* ============================= */
/* Global Types                  */
/* ============================= */
typedef struct
{
   IFX_uint16_t rising;
   IFX_uint16_t falling;
} VIN_IntMask_t;

typedef union
{
   IFX_uint16_t value[3];
   struct
   {
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
      unsigned i2                            : 6;
      unsigned res2                          : 2;
      unsigned i1                            : 6;
      unsigned res1                          : 1;
      unsigned en                            : 1;
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
      unsigned res1                          : 1;
      unsigned i1                            : 6;
      unsigned res2                          : 2;
      unsigned i2                            : 6;
#endif /* BIG_ENDIAN */
   } bit;
} FWM_SIG_CH;

typedef union
{
   IFX_uint16_t value [3];
   struct {
#if (__BYTE_ORDER == __LITTLE_ENDIAN)
      /* cmd 1 */
      unsigned ch                            : 4;
      unsigned res                           : 4;
      unsigned cmd                           : 5;
      unsigned bc                            : 1;
      unsigned sc                            : 1;
      unsigned rw                            : 1;
      /* cmd 2 */
      unsigned length                        : 8;
      unsigned ecmd                          : 5;
      unsigned mod                           : 3;
      /* data 1 */
      unsigned resnr                         : 4;
      unsigned is                            : 2;
      unsigned md                            : 2;
      unsigned res2                          : 6;
      unsigned et                            : 1;
      unsigned en                            : 1;
#endif /* LITTLE_ENDIAN */
#if (__BYTE_ORDER == __BIG_ENDIAN)
      /* cmd 1 */
      unsigned rw                            : 1;
      unsigned sc                            : 1;
      unsigned bc                            : 1;
      unsigned cmd                           : 5;
      unsigned res                           : 4;
      unsigned ch                            : 4;
      /* cmd 2 */
      unsigned mod                           : 3;
      unsigned ecmd                          : 5;
      unsigned length                        : 8;
      /* data 1 */
      unsigned en                            : 1;
      unsigned et                            : 1;
      unsigned res2                          : 6;
      unsigned md                            : 2;
      unsigned is                            : 2;
      unsigned resnr                         : 4;
#endif /* BIG_ENDIAN */
   } bit;
} FWM_SIG_XTD;

typedef union
{
   IFX_uint16_t value [5];
   struct {
#if (__BYTE_ORDER == __LITTLE_ENDIAN)
      /* cmd 1 */
      unsigned ch                            : 4;
      unsigned res                           : 4;
      unsigned cmd                           : 5;
      unsigned bc                            : 1;
      unsigned sc                            : 1;
      unsigned rw                            : 1;
      /* cmd 2 */
      unsigned length                        : 8;
      unsigned ecmd                          : 5;
      unsigned mod                           : 3;
      /* data 1 */
      unsigned resnr                         : 4;
      unsigned res3                          : 3;
      unsigned mh                            : 1;
      unsigned vad                           : 1;
      unsigned res2                          : 3;
      unsigned etc                           : 1;
      unsigned etd                           : 1;
      unsigned eta                           : 1;
      unsigned en                            : 1;
      /* data 2  (upstream) */
      unsigned single1                       : 8;
      unsigned res5                          : 1;
      unsigned dis1                          : 1;
      unsigned atd1                          : 2;
      unsigned dual1                         : 3;
      unsigned res4                          : 1;
      /* data 3 (downstream) */
      unsigned single2                       : 8;
      unsigned res7                          : 1;
      unsigned dis2                          : 1;
      unsigned atd2                          : 2;
      unsigned dual2                         : 3;
      unsigned res6                          : 1;
#endif /* LITTLE_ENDIAN */
#if (__BYTE_ORDER == __BIG_ENDIAN)
      /* cmd 1 */
      unsigned rw                            : 1;
      unsigned sc                            : 1;
      unsigned bc                            : 1;
      unsigned cmd                           : 5;
      unsigned res                           : 4;
      unsigned ch                            : 4;
      /* cmd 2 */
      unsigned mod                           : 3;
      unsigned ecmd                          : 5;
      unsigned length                        : 8;
      /* data 1 */
      unsigned en                            : 1;
      unsigned eta                           : 1;
      unsigned etd                           : 1;
      unsigned etc                           : 1;
      unsigned res2                          : 3;
      unsigned vad                           : 1;
      unsigned mh                            : 1;
      unsigned res3                          : 3;
      unsigned resnr                         : 4;
      /* data 2  (upstream) */
      unsigned res4                          : 1;
      unsigned dual1                         : 3;
      unsigned atd1                          : 2;
      unsigned dis1                          : 1;
      unsigned res5                          : 1;
      unsigned single1                       : 8;
      /* data 3 (downstream) */
      unsigned res6                          : 1;
      unsigned dual2                         : 3;
      unsigned atd2                          : 2;
      unsigned dis2                          : 1;
      unsigned res7                          : 1;
      unsigned single2                       : 8;
#endif /* BIG_ENDIAN */
   } bit;
} FWM_SIG_MFTD;

typedef union
{
   IFX_uint16_t value [CMD_HEADER_CNT + CMD_SIG_DTMFGEN_LEN];
   struct {
#if (__BYTE_ORDER == __LITTLE_ENDIAN)
      /* cmd 1 */
      unsigned ch                            : 4;
      unsigned res                           : 4;
      unsigned cmd                           : 5;
      unsigned bc                            : 1;
      unsigned sc                            : 1;
      unsigned rw                            : 1;
      /* cmd 2 */
      unsigned length                        : 8;
      unsigned ecmd                          : 5;
      unsigned cmdmod                        : 3;
      /* data 1 */
      unsigned resnr                         : 4;
      unsigned addb                          : 2;
      unsigned adda                          : 2;
      unsigned res3                          : 1;
      unsigned fg                            : 1;
      unsigned mod                           : 1;
      unsigned ad                            : 1;
      unsigned res2                          : 2;
      unsigned et                            : 1;
      unsigned en                            : 1;
#endif /* LITTLE_ENDIAN */
#if (__BYTE_ORDER == __BIG_ENDIAN)
      /* cmd 1 */
      unsigned rw                            : 1;
      unsigned sc                            : 1;
      unsigned bc                            : 1;
      unsigned cmd                           : 5;
      unsigned res                           : 4;
      unsigned ch                            : 4;
      /* cmd 2 */
      unsigned cmdmod                        : 3;
      unsigned ecmd                          : 5;
      unsigned length                        : 8;
      /* data 1 */
      unsigned en                            : 1;
      unsigned et                            : 1;
      unsigned res2                          : 2;
      unsigned ad                            : 1;
      unsigned mod                           : 1;
      unsigned fg                            : 1;
      unsigned res3                          : 1;
      unsigned adda                          : 2;
      unsigned addb                          : 2;
      unsigned resnr                         : 4;
#endif /* BIG_ENDIAN */
   } bit;
} FWM_SIG_DTMFGEN;

typedef union
{
   IFX_uint16_t value [3];
   struct {
#if (__BYTE_ORDER == __LITTLE_ENDIAN)
      /* cmd 1 */
      unsigned ch                            : 4;
      unsigned res                           : 4;
      unsigned cmd                           : 5;
      unsigned bc                            : 1;
      unsigned sc                            : 1;
      unsigned rw                            : 1;
      /* cmd 2 */
      unsigned length                        : 8;
      unsigned ecmd                          : 5;
      unsigned mod                           : 3;
      /* data 1 */
      unsigned utgnr                         : 4;
      unsigned a2                            : 2;
      unsigned a1                            : 2;
      unsigned res2                          : 4;
      unsigned log                           : 1;
      unsigned sq                            : 1;
      unsigned sm                            : 1;
      unsigned en                            : 1;
#endif /* LITTLE_ENDIAN */
#if (__BYTE_ORDER == __BIG_ENDIAN)
      /* cmd 1 */
      unsigned rw                            : 1;
      unsigned sc                            : 1;
      unsigned bc                            : 1;
      unsigned cmd                           : 5;
      unsigned res                           : 4;
      unsigned ch                            : 4;
      /* cmd 2 */
      unsigned mod                           : 3;
      unsigned ecmd                          : 5;
      unsigned length                        : 8;
      /* data 1 */
      unsigned en                            : 1;
      unsigned sm                            : 1;
      unsigned sq                            : 1;
      unsigned log                           : 1;
      unsigned res2                          : 4;
      unsigned a1                            : 2;
      unsigned a2                            : 2;
      unsigned utgnr                         : 4;
#endif /* BIG_ENDIAN */
   } bit;
} FWM_SIG_UTG;

typedef union
{
   IFX_uint16_t value;
   struct {
#if (__BYTE_ORDER == __LITTLE_ENDIAN)
      /* data 1 */
      unsigned dtrnr                         : 4;
      unsigned as                            : 1;
      unsigned is                            : 1;
      unsigned res1                          : 8;
      unsigned et                            : 1;
      unsigned en                            : 1;
#endif /* LITTLE_ENDIAN */
#if (__BYTE_ORDER == __BIG_ENDIAN)
      /* data 1 */
      unsigned en                            : 1;
      unsigned et                            : 1;
      unsigned res1                          : 8;
      unsigned is                            : 1;
      unsigned as                            : 1;
      unsigned dtrnr                         : 4;
#endif /* BIG_ENDIAN */
   } bit;
} FWM_SIG_DTMFREC;

typedef union
{
   IFX_uint16_t value [4];
   struct {
#if (__BYTE_ORDER == __LITTLE_ENDIAN)
      /* cmd 1 */
      unsigned ch                            : 4;
      unsigned res                           : 4;
      unsigned cmd                           : 5;
      unsigned bc                            : 1;
      unsigned sc                            : 1;
      unsigned rw                            : 1;
      /* cmd 2 */
      unsigned length                        : 8;
      unsigned ecmd                          : 5;
      unsigned mod                           : 3;
      /* data 1 */
      unsigned twist                         : 8;
      unsigned level                         : 8;
      /* data 2 */
      unsigned gain                          : 8;
      unsigned res1                          : 8;
#endif /* LITTLE_ENDIAN */
#if (__BYTE_ORDER == __BIG_ENDIAN)
      /* cmd 1 */
      unsigned rw                            : 1;
      unsigned sc                            : 1;
      unsigned bc                            : 1;
      unsigned cmd                           : 5;
      unsigned res                           : 4;
      unsigned ch                            : 4;
      /* cmd 2 */
      unsigned mod                           : 3;
      unsigned ecmd                          : 5;
      unsigned length                        : 8;
      /* data 1 */
      unsigned level                         : 8;
      unsigned twist                         : 8;
      /* data 2 */
      unsigned res1                          : 8;
      unsigned gain                          : 8;
#endif /* BIG_ENDIAN */
   } bit;
} FWM_RES_DTMFREC_COEFF;


/** Structure for the signaling channel
   including firmware message cache */
struct VINETIC_SIGCH
{
   FWM_SIG_CH        fw_sig_ch;
   FWM_SIG_CH_CONF   fw_sig_ch_cfg;
   FWM_SIG_XTD       fw_sig_atd1;
   FWM_SIG_XTD       fw_sig_atd2;
   FWM_SIG_XTD       fw_sig_utd1;
   FWM_SIG_XTD       fw_sig_utd2;
   FWM_SIG_MFTD      fw_sig_mftd;
   FWM_SIG_DTMFGEN   fw_sig_dtmfgen;
   IFX_uint16_t      nCpt;
   FWM_SIG_UTG       fw_sig_utg[LL_TAPI_TONE_MAXRES];
   IFX_uint16_t      cid_sender[3];
   /* data word without command words for DTMF receiver */
   FWM_SIG_DTMFREC   fw_dtmf_rec;
   FWM_RES_DTMFREC_COEFF fw_dtmfr_coeff;
   /* Event transmission status to be programmed on coder start */
   SIG_MOD_STATE     et_stat;

   /** line signals for fax and modem detection */
   IFX_uint32_t      signal;
   IFX_uint32_t      signalExt;
   /** Currently enabled line signals, zero means enabled.
      The current signal mask is set by the interface enable/disable signal */
   IFX_uint32_t      sigMask;
   IFX_uint32_t      sigMaskExt;
   /** stores the last detected tone of the MFTD for tone end detection */
   IFX_uint16_t      lastMftd1ToneIdx;
   IFX_uint16_t      lastMftd2ToneIdx;
   /* DTMF generator configuration */
   IFX_uint16_t       nDtmfInterDigitTime;
   IFX_uint16_t       nDtmfDigitPlayTime;
};


/* ============================= */
/* Global Variables              */
/* ============================= */

/* ============================= */
/* Global function declaration   */
/* ============================= */

/**
   Disables or Enables DTMF receiver according to bEn
*/
extern IFX_int32_t VINETIC_SIG_SetDtmfRec     (VINETIC_CHANNEL const *pCh,
                                               IFX_boolean_t bEn,
                                               IFX_uint8_t dir);

/**
   Configuration of the universal tone detector (UTD)
*/
extern IFX_int32_t VINETIC_SIG_UtdConf        (VINETIC_CHANNEL *pCh,
                                               IFX_uint32_t nSignal);

/**
   Enables signal detection on the universal tone detector (UTD)
*/
extern IFX_int32_t VINETIC_SIG_UtdSigEnable   (VINETIC_CHANNEL *pCh,
                                               IFX_uint32_t nSignal);

/**
   Enables signal detection on the Modem Fax Tone Detector (MFTD)
*/
extern IFX_int32_t VINETIC_SIG_MFTD_SigEnable (VINETIC_CHANNEL *pCh,
                                               IFX_uint32_t nSignal,
                                               IFX_uint32_t nSignalExt);

/**
   Disables signal detection on the Modem Fax Tone Detector (MFTD)
*/
extern IFX_int32_t VINETIC_SIG_MFTD_SigDisable (VINETIC_CHANNEL *pCh,
                                                IFX_uint32_t nSignal,
                                                IFX_uint32_t nSignalExt);

/**
   Firmware DTMF generator configuration
*/
extern IFX_int32_t VINETIC_SIG_SetDtmfCoeff    (VINETIC_CHANNEL *pCh,
                                                IFX_uint8_t nTIMT,
                                                IFX_uint8_t nTIMP);

/**
   Firmware DTMF/AT generator control
*/
extern IFX_int32_t VINETIC_SIG_DtmfGen         (VINETIC_CHANNEL *pCh,
                                                IFX_uint16_t nMode,
                                                IFX_uint16_t nMask,
                                                IFX_boolean_t nIsr);

/**
   Configures signal detection on the answering tone detector
 */
extern IFX_int32_t VINETIC_SIG_AtdConf         (VINETIC_CHANNEL *pCh,
                                                IFX_uint32_t nSignal);

/**
   Enables signal detection on the answering tone detector
*/
extern IFX_int32_t VINETIC_SIG_AtdSigEnable    (VINETIC_CHANNEL *pCh,
                                                IFX_uint32_t nSignal);

/**
   Start the DTMF generator
 */
extern IFX_int32_t VINETIC_SIG_DtmfStart       (VINETIC_CHANNEL *pCh,
                                                IFX_uint16_t *pDtmfData,
                                                IFX_uint16_t nDtmfWords,
                                                IFX_uint32_t nFG,
    IFX_void_t (*cbDtmfStatus) (VINETIC_CHANNEL *pCh), IFX_boolean_t bByteMode);

#endif  /* _DRV_VINETIC_SIG_PRIV_H */
