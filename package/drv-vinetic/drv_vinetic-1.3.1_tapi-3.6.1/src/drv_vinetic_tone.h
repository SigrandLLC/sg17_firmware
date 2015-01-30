#ifndef _DRV_VINETIC_TONE_H
#define _DRV_VINETIC_TONE_H
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
   Module      : drv_vinetic_tone.h
   Date        : 2005-03-30
   Description : This file contains the declaration of the functions
                 for Tone operations for TAPI
*******************************************************************************/

/* ============================= */
/* Global function declaration   */
/* ============================= */

IFX_void_t TAPI_LL_Tone_EventUTG_Deactivated (VINETIC_CHANNEL *pCh,
                                              IFX_uint8_t nChannel,
                                              IFX_uint8_t utgNum);
TAPI_CMPLX_TONE_STATE_t TG_ToneStep (VINETIC_CHANNEL *pCh,
                                     IFX_TAPI_TONE_SIMPLE_t const *pTone,
                                     IFX_uint8_t res,
                                     IFX_uint8_t *nToneStep);

#ifndef VIN_2CPE
extern IFX_void_t  Tone_PredefOnTimer (Timer_ID Timer, IFX_int32_t nArg);
extern IFX_int32_t Tone_TG_Play          (VINETIC_CHANNEL *pCh);
extern IFX_int32_t Tone_TG_SetCoeff      (VINETIC_CHANNEL *pCh, IFX_uint8_t vers,
#endif /* VIN_2CPE */
#endif /* _DRV_VINETIC_TONE_H */

