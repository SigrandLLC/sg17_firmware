#ifndef _DRV_VINETIC_PCM_PRIV_H
#define _DRV_VINETIC_PCM_PRIV_H
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
   Module      : drv_vinetic_pcm_priv.h
   Description : This file contains the defines, the structures declarations
                  for PCM module.
*******************************************************************************/

/* includes */
#include "ifx_types.h"
#include "sys_drv_ifxos.h"
#include "drv_vinetic.h"


/******************************************************************************
** Firmware PCM Interface Channel Cmd
**
*******************************************************************************/
typedef union
{
   IFX_uint16_t value[CMD_HEADER_CNT + CMD_PCM_CH_LEN];
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
      unsigned bp                            : 1;
      unsigned hp                            : 1;
      unsigned codnr                         : 4;
      unsigned cod                           : 3;
      unsigned en                            : 1;

      unsigned rts                           : 7;
      unsigned r_hw                          : 1;
      unsigned xts                           : 7;
      unsigned x_hw                          : 1;

      unsigned gain_2                        : 8;
      unsigned gain_1                        : 8;

      unsigned i3                            : 6;
      unsigned res2                          : 2;
      unsigned i2                            : 6;
      unsigned res1                          : 2;

      unsigned i5                            : 6;
      unsigned res4                          : 2;
      unsigned i4                            : 6;
      unsigned res3                          : 2;
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
      unsigned cod                           : 3;
      unsigned codnr                         : 4;
      unsigned hp                            : 1;
      unsigned bp                            : 1;
      unsigned i1                            : 6;

      unsigned x_hw                          : 1;
      unsigned xts                           : 7;
      unsigned r_hw                          : 1;
      unsigned rts                           : 7;

      unsigned gain_1                        : 8;
      unsigned gain_2                        : 8;

      unsigned res1                          : 2;
      unsigned i2                            : 6;
      unsigned res2                          : 2;
      unsigned i3                            : 6;

      unsigned res3                          : 2;
      unsigned i4                            : 6;
      unsigned res4                          : 2;
      unsigned i5                            : 6;
#endif /* BIG_ENDIAN */
   } bit;
} FWM_PCM_CH;



/** Structure for the PCM channel
   including firmware message cache */
struct VINETIC_PCMCH
{
   FWM_PCM_CH        fw_pcm_ch;
   IFX_uint16_t      pcm_nelec;
   /* The LEC window length specification is channel specific */
   IFX_uint8_t       lec_window_coefs[3];
};



#endif /* VINETIC_PCM_PRIV_H */

