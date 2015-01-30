#ifndef _drv_vinetic_con_H
#define _drv_vinetic_con_H
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
   Module      : drv_vinetic_con.h
   Date        : 2005-02-16
   Description : This file contains the declaration of the connection module
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

#ifndef _DRV_VINETIC_CON_PRIV_H
struct VINDSP_MODULE;
#endif

typedef struct VINDSP_MODULE VINDSP_MODULE_t;

/** defines which output of the signaling module is to be used
    ALM and PCM are normally attached on local side and COD on remote side */
typedef enum
{
   REMOTE_SIG_OUT = 0,
   LOCAL_SIG_OUT = 1
} SIG_OUTPUT_SIDE;

/** which signaling input is used for local (with auto suppression) and remote
    (with event playout) connections. This is fixed by firmware */
enum
{
   REMOTE_SIG_IN = 1,
   LOCAL_SIG_IN = 0
};

/** Module types of the firmware */
typedef enum
{
   VINDSP_MT_ALM,
   VINDSP_MT_SIG,
   VINDSP_MT_COD,
   VINDSP_MT_PCM
} VINDSP_MT;


/* ============================= */
/* Global Variables              */
/* ============================= */

#ifdef DEBUG
extern const IFX_char_t signalName [52][10];
#endif /* DEBUG */


/* ============================= */
/* Global function declaration   */
/* ============================= */

extern IFX_return_t VINETIC_CON_Allocate_Ch_Structures (VINETIC_CHANNEL *pCh);
extern IFX_void_t   VINETIC_CON_Free_Ch_Structures (VINETIC_CHANNEL *pCh);

extern IFX_void_t VINETIC_CON_Init_AlmCh (VINETIC_CHANNEL *pCh);
extern IFX_void_t VINETIC_CON_Init_PcmCh (VINETIC_CHANNEL *pCh,
                                          IFX_uint8_t pcmCh);
extern IFX_void_t VINETIC_CON_Init_CodCh (VINETIC_CHANNEL *pCh);
extern IFX_void_t VINETIC_CON_Init_SigCh (VINETIC_CHANNEL *pCh);


extern IFX_uint8_t      VINETIC_CON_Get_ALM_SignalInput (VINETIC_CHANNEL *pCh,
                                                         IFX_uint8_t index);
extern IFX_uint8_t      VINETIC_CON_Get_PCM_SignalInput (VINETIC_CHANNEL *pCh,
                                                         IFX_uint8_t index);
extern IFX_uint8_t      VINETIC_CON_Get_COD_SignalInput (VINETIC_CHANNEL *pCh,
                                                         IFX_uint8_t index);
extern IFX_uint8_t      VINETIC_CON_Get_SIG_SignalInput (VINETIC_CHANNEL *pCh,
                                                         IFX_uint8_t index);

#if (VIN_CFG_FEATURES & VIN_FEAT_VOICE)
extern IFX_return_t VINETIC_CON_Func_Register (IFX_TAPI_DRV_CTX_CON_t *pCON);
#endif /* (VIN_CFG_FEATURES & VIN_FEAT_VOICE) */


extern IFX_return_t VINETIC_CON_ConnectPrepare    (VINETIC_DEVICE *pDev,
                                                   IFX_uint8_t ch,
                                                   VINDSP_MT src,
                                                   VINDSP_MT dst,
                                                   SIG_OUTPUT_SIDE nSide);
extern IFX_return_t VINETIC_CON_DisconnectPrepare (VINETIC_DEVICE *pDev,
                                                   IFX_uint8_t ch,
                                                   VINDSP_MT src,
                                                   VINDSP_MT dst,
                                                   SIG_OUTPUT_SIDE nSide);
extern IFX_return_t VINETIC_CON_ConnectConfigure  (VINETIC_DEVICE *pDev);

#endif /* _drv_vinetic_con_H */
