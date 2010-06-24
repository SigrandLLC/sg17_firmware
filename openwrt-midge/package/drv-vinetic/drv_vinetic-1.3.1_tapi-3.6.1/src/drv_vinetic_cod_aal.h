#ifndef _DRV_VINETIC_COD_AAL_H
#define _DRV_VINETIC_COD_AAL_H
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
   Module      : drv_vinetic_cod_aal.h
   Date        : 2006-08-31
   Description : This file contains the declaration of the functions
                 for AAL operations
*******************************************************************************/

/* ============================= */
/* Global Defines                */
/* ============================= */


/* ============================= */
/* Global Structures             */
/* ============================= */

/* ============================= */
/* Global function declaration   */
/* ============================= */

IFX_return_t IFX_TAPI_LL_COD_AAL_Profile_Cfg (IFX_TAPI_LL_CH_t *pChannel,
                                      IFX_TAPI_PCK_AAL_PROFILE_t const *pProfile);

IFX_return_t IFX_TAPI_LL_COD_AAL_Cfg (IFX_TAPI_LL_CH_t *pChannel,
                                   IFX_TAPI_PCK_AAL_CFG_t const *pAalConf);

#endif /* _DRV_VINETIC_COD_AAL_H */
