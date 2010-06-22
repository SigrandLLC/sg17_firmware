#ifndef _DRV_DAA_COMMON_PRIV_H
#define _DRV_DAA_COMMON_PRIV_H
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
   \file drv_daa_common_priv.h
   functions and global defines available inside module common

   this file shouldn't be modified

*/

#define DRV_DAA_DEBOUNCE_TIME       70 /* ms */

IFX_void_t daa_tcb_ring (IFX_void_t * timer_id, IFX_uint32_t arg);
IFX_void_t daa_tcb_bat  (IFX_void_t * timer_id, IFX_uint32_t arg);
IFX_void_t daa_tcb_apoh (IFX_void_t * timer_id, IFX_uint32_t arg);
IFX_void_t daa_tcb_pol  (IFX_void_t *timer_id, IFX_uint32_t arg);

IFX_void_t daa_sh_ring  (DAA_DEV_t *pDaa, DAA_RING_EVT_t evt);
IFX_void_t daa_sh_bat   (DAA_DEV_t *pDaa, DAA_BAT_EVT_t evt);
IFX_void_t daa_sh_apoh  (DAA_DEV_t *pDaa, DAA_APOH_EVT_t evt);
IFX_void_t daa_sh_pol   (DAA_DEV_t *pDaa, DAA_POL_EVT_t evt);

IFX_void_t DAA_COM_MaxOsiTimeSet (IFX_uint32_t nMaxOsiTime);

#endif /* _DRV_DAA_COMMON_PRIV_H */

