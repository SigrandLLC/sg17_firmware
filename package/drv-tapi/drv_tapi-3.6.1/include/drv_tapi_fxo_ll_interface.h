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
   \file drv_tapi_fxo_io.h
   TAPI FXO interface to the DAA abstraction driver
*/

#ifndef _DRV_TAPI_FXO_IO_H
#define _DRV_TAPI_FXO_IO_H

#include <ifx_types.h>

typedef struct {
   IFX_return_t (*Init)    (IFX_uint16_t nDAA);
   IFX_return_t (*osiCfg)  (IFX_uint32_t nOsiMaxTime);
   IFX_return_t (*fhCfg)   (IFX_uint32_t nFlashHookTime);
   IFX_return_t (*fhSet)   (IFX_uint16_t nDAA);
   IFX_return_t (*hookSet) (IFX_uint16_t nDAA, IFX_TAPI_FXO_HOOK_t  nHookState);
   IFX_return_t (*hookGet) (IFX_uint16_t nDAA, IFX_TAPI_FXO_HOOK_t *pHookState);
   IFX_return_t (*apohGet) (IFX_uint16_t nDAA, IFX_enDis_t *pApohState);
   IFX_return_t (*polGet)  (IFX_uint16_t nDAA, IFX_enDis_t *pPolState);
   IFX_return_t (*batGet)  (IFX_uint16_t nDAA, IFX_enDis_t *pBatState);
   IFX_return_t (*ringGet) (IFX_uint16_t nDAA, IFX_enDis_t *pRingState);
} IFX_TAPI_DRV_CTX_DAA_t;


extern IFX_return_t IFX_TAPI_Register_DAA_Drv(IFX_TAPI_DRV_CTX_DAA_t *pDAA);
extern IFX_return_t IFX_TAPI_FXO_Event_Dispatch(int nDAA, IFX_TAPI_EVENT_t *pEvent);

#endif /* _DRV_TAPI_FXO_IO_H */
