#ifndef _DRV_DAA_COMMON_H
#define _DRV_DAA_COMMON_H
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
   \file drv_daa_common.h
   Common functions available to other levels
   global defines

   this file shouldn't be modified

*/


/* ============================= */
/* Macros & Definitions          */
/* ============================= */
typedef struct
{
   IFX_return_t (*hookSet) (IFX_TAPI_FXO_HOOK_t);
   IFX_return_t (*hookGet) (IFX_TAPI_FXO_HOOK_t *);
   IFX_return_t (*batGet)  (IFX_enDis_t *);
   IFX_return_t (*apohGet) (IFX_enDis_t *);
   IFX_return_t (*polGet)  (IFX_enDis_t *);
   IFX_return_t (*init)    (void);
} DAA_FUNCTION_MAP_t;


/** default hook flash time is 100ms, can be configured via
    \ref IFX_TAPI_FXO_FLASH_CFG_SET ioctl */
#define DRV_DAA_DEFAULT_HOOK_FALSH_TIME            100   /* ms */
/** default maximum OSI time is 200ms, can be configured via
    IFX_TAPI_FXO_OSI_CFG_SET ioctl */
#define DRV_DAA_DEFAULT_MAX_OSI_TIME               200   /* ms */

/* ============================= */
/* Global function definition    */
/* ============================= */
void irq_DAA_COM_ring_cb         (IFX_uint16_t nDaa);
void irq_DAA_COM_bat_cb          (IFX_uint16_t nDaa, DAA_BAT_EVT_t env);
void irq_DAA_COM_pol_cb          (IFX_uint16_t nDaa, DAA_POL_EVT_t evt);
void irq_DAA_COM_apoh_cb         (IFX_uint16_t nDaa, DAA_APOH_EVT_t evt);
void DAA_COM_FlashHookTimeSet    (IFX_uint32_t nFlashHookTime);
void DAA_COM_MaxOsiTimeSet       (IFX_uint32_t nMaxOsiTime);
IFX_return_t DAA_COM_init        (IFX_uint16_t nDaa);
IFX_return_t DAA_COM_InitStates  (IFX_uint16_t nDaa);
IFX_return_t DAA_COM_FlashHook   (IFX_uint16_t nDaa);
IFX_return_t DAA_COM_RingGet     (IFX_uint16_t nDaa, IFX_enDis_t *);
IFX_return_t DAA_COM_BatGet      (IFX_uint16_t nDaa, IFX_enDis_t *);
IFX_return_t DAA_COM_ApohGet     (IFX_uint16_t nDaa, IFX_enDis_t *pApoh);
IFX_return_t DAA_COM_PolGet      (IFX_uint16_t nDaa, IFX_enDis_t *pPol);
IFX_return_t DAA_COM_OnInsmod    (void);
IFX_return_t DAA_COM_OnRmmod     (void);

#endif /* _DRV_DAA_COMMON_H */
