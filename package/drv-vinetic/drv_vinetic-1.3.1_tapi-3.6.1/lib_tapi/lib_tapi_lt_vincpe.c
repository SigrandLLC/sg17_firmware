/*******************************************************************************
                  Copyright (c) 2005  Infineon Technologies AG
                 St. Martin Strasse 53; 81669 Munich, Germany

   THE DELIVERY OF THIS SOFTWARE AS WELL AS THE HEREBY GRANTED NON-EXCLUSIVE,
   WORLDWIDE LICENSE TO USE, COPY, MODIFY, DISTRIBUTE AND SUBLICENSE THIS
   SOFTWARE IS FREE OF CHARGE.

   THE LICENSED SOFTWARE IS PROVIDED "AS IS" AND INFINEON EXPRESSLY DISCLAIMS
   ALL REPRESENTATIONS AND WARRANTIES, WHETHER EXPRESS OR IMPLIED, INCLUDING
   WITHOUT LIMITATION, WARRANTIES OR REPRESENTATIONS OF WORKMANSHIP,
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, DURABILITY, THAT THE
   OPERATING OF THE LICENSED SOFTWARE WILL BE ERROR FREE OR FREE OF ANY
   THIRD PARTY CLAIMS, INCLUDING WITHOUT LIMITATION CLAIMS OF THIRD PARTY
   INTELLECTUAL PROPERTY INFRINGEMENT.

   EXCEPT FOR ANY LIABILITY DUE TO WILFUL ACTS OR GROSS NEGLIGENCE AND
   EXCEPT FOR ANY PERSONAL INJURY INFINEON SHALL IN NO EVENT BE LIABLE FOR
   ANY CLAIM OR DAMAGES OF ANY KIND, WHETHER IN AN ACTION OF CONTRACT,
   TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
   SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
********************************************************************************
   Module       : lib_tapi_lt_vincpe.c
   Description  :
*******************************************************************************/

/* ============================= */
/* Includes                      */
/* ============================= */

/* system includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <math.h>

#ifdef VXWORKS
#include <ioLib.h>
#endif /* VXWORKS */

#include "ifx_types.h"
#include "vinetic_io.h"
#include "lib_tapi_lt_gr909.h"

#if (defined (VIN_2CPE) && defined (TAPI_GR909))

/* ============================= */
/* Defines                       */
/* ============================= */


/* HPT Formulas */
/* formula for HPT AC calculations :
   R2G, T2G : Vresult[Vrms] = Value * (R2 + R1) / 32768 / 1.8844 / sqrt(2) / R2
        T2R : Vresult[Vrms] = Value * (R3 + R1) / 32768 / 1.8844 / sqrt(2) / R3
*/
#define HPT_AC_CALC(res,val,r_a,r_b) \
   (res)=(val)*((r_b)+(r_a))/32768.0/1.8844/sqrt(2)/(r_b);

/* formula for HPT DC calculations : R2G, T2G
   Vresult[V] = (Value * (R2 + R1) / R2 / 32768 / 1.8844) + 0.7
*/
#define HPT_DC_X2G_CALC(res,val,r_a,r_b) \
   (res)=((val)*((r_b)+(r_a))/(r_b)/32768.0/1.8844) + 0.7;

/* formula for HPT DC calculations : T2R
   Vresult[V] = (Value * (R3 + R1) / R3 / 32768 / 1.8844)
*/
#define HPT_DC_T2R_CALC(res,val,r_a,r_b) \
   (res)=((val)*((r_b)+(r_a))/(r_b)/32768.0/1.8844);

/* FEMF Formulas */
/* formula for HPT AC calculations :
   R2G, T2G : Vresult[Vrms] = Value * (R2 + R1) / 32768 / 1.8844 / sqrt(2) / R2
        T2R : Vresult[Vrms] = Value * (R3 + R1) / 32768 / 1.8844 / sqrt(2) / R3
*/
#define FEMF_AC_CALC(res,val,r_a,r_b) HPT_AC_CALC(res,val,r_a,r_b)

/* formula for HPT DC calculations : R2G, T2G
   Vresult[V] = (Value * (R2 + R1) / R2 / 32768 / 1.8844) + 0.7
*/
#define FEMF_DC_X2G_CALC(res,val,r_a,r_b) HPT_DC_X2G_CALC(res,val,r_a,r_b)

/* formula for HPT DC calculations : T2R
   Vresult[V] = (Value * (R3 + R1) / R3 / 32768 / 1.8844)
*/
#define FEMF_DC_T2R_CALC(res,val,r_a,r_b) HPT_DC_T2R_CALC(res,val,r_a,r_b)

/* RFT Formulas */
/* Formula for RFT calculations :
   R2G, T2G : Rresult[Ohm] = Value * (R2 + R1) / R2 / 26.12244898
        T2R : Rresult[Ohm] = Value * (R3 + R1) / R3 / 26.12244898
*/
#define RFT_CALC(res,val,r_a,r_b) \
   (res)=(val)*((r_b)+ (r_a))/(r_b)/26.12244898;

/* ROH Formulas */
/* Formula for ROG calculations :
   T2R_L, T2R_H  : Rresult[Ohm] = Value * (R3 + R1) / R3 / 26.12244898
*/
#define ROH_CALC(res,val,r_a,r_b) \
   (res)=(val)*((r_b)+ (r_a))/(r_b)/26.12244898;

/* RIT Formulas */
/* Formula for RIT calculations :
   Zresult[Ohm] = Value * (R3 + R1) / R3 / 105.3497998 * Git
   Git = 0.9653 for a 20 Hz Ringing signal used here for measurements
*/
#define RIT_CALC(res,val,r_a,r_b) \
   (res)=((val)*((r_b)+(r_a))/(r_b)/105.3497998*0.9653);

/* ============================= */
/* Local variable definition     */
/* ============================= */

/* default system configuration (SLIC DC) */
static IFX_LT_GR909_CFG_t cfg =
{
   /* default :  R1 = 1.5 MOhm */
   1500000.0,
   /* default :  R2 = 3289 Ohm */
   3289.0,
   /* default :  R3 = 3286 Ohm */
   3286.0,
};

/* ============================= */
/* Global variable definition    */
/* ============================= */

/* ============================= */
/* Local function declaration    */
/* ============================= */

static IFX_void_t lt_vincpe_getres_hpt  (VINETIC_IO_GR909_Result_t *p_llres,
                                         IFX_LT_GR909_HPT_t        *p_hpt);
static IFX_void_t lt_vincpe_getres_femf (VINETIC_IO_GR909_Result_t *p_llres,
                                         IFX_LT_GR909_FEMF_t       *p_femf);
static IFX_void_t lt_vincpe_getres_roh  (VINETIC_IO_GR909_Result_t *p_llres,
                                         IFX_LT_GR909_ROH_t        *p_roh);
static IFX_void_t lt_vincpe_getres_rit  (VINETIC_IO_GR909_Result_t *p_llres,
                                         IFX_LT_GR909_RIT_t        *p_rit);

/* ============================= */
/* Local function definition     */
/* ============================= */

/**
   Calculates HPT results out of low level results
\param p_llres - low level results, in
\param p_hpt - HPT results, out
*/
static IFX_void_t lt_vincpe_getres_hpt (VINETIC_IO_GR909_Result_t *p_llres,
                                        IFX_LT_GR909_HPT_t        *p_hpt)
{
   /* test passed ? */
   if (p_llres->passed & VINETIC_IO_GR909_HPT)
      p_hpt->b_result = IFX_TRUE;

   /* HPT AC RING wire to GND result */
   HPT_AC_CALC(p_hpt->f_hpt_ac_r2g, p_llres->HPT_AC_R2G, cfg.f_R1, cfg.f_R2);
   /* HPT AC TIP wire to GND result */
   HPT_AC_CALC(p_hpt->f_hpt_ac_t2g, p_llres->HPT_AC_T2G, cfg.f_R1, cfg.f_R2);
   /* HPT AC TIP wire to RING wire result */
   HPT_AC_CALC(p_hpt->f_hpt_ac_t2r, p_llres->HPT_AC_T2R, cfg.f_R1, cfg.f_R3);
   /* HPT DC RING wire to GND result */
   HPT_DC_X2G_CALC(p_hpt->f_hpt_dc_r2g, p_llres->HPT_DC_R2G, cfg.f_R1, cfg.f_R2);
   /* HPT DC TIP wire to GND result */
   HPT_DC_X2G_CALC(p_hpt->f_hpt_dc_t2g, p_llres->HPT_DC_T2G, cfg.f_R1, cfg.f_R2);
   /* HPT DC TIP wire to RING wire result */
   HPT_DC_T2R_CALC(p_hpt->f_hpt_dc_t2r, p_llres->HPT_DC_T2R, cfg.f_R1, cfg.f_R3);

   return;
}

/**
   Calculates FEMF results out of low level results
\param p_llres - low level results, in
\param p_hpt - FEMF results, out
*/
static IFX_void_t lt_vincpe_getres_femf (VINETIC_IO_GR909_Result_t *p_llres,
                                         IFX_LT_GR909_FEMF_t       *p_femf)
{
   /* test passed ? */
   if (p_llres->passed & VINETIC_IO_GR909_FEMF)
      p_femf->b_result = IFX_TRUE;

   /* FEMF AC RING wire to GND result */
   FEMF_AC_CALC(p_femf->f_femf_ac_r2g, p_llres->FEMF_AC_R2G, cfg.f_R1, cfg.f_R2);
   /* FEMF AC TIP wire to GND result */
   FEMF_AC_CALC(p_femf->f_femf_ac_t2g, p_llres->FEMF_AC_T2G, cfg.f_R1, cfg.f_R2);
   /* FEMF AC TIP wire to RING wire result */
   FEMF_AC_CALC(p_femf->f_femf_ac_t2r, p_llres->FEMF_AC_T2R, cfg.f_R1, cfg.f_R3);
   /* FEMF DC RING wire to GND result */
   FEMF_DC_X2G_CALC(p_femf->f_femf_dc_r2g, p_llres->FEMF_DC_R2G, cfg.f_R1, cfg.f_R2);
   /* FEMF DC TIP wire to GND result */
   FEMF_DC_X2G_CALC(p_femf->f_femf_dc_t2g, p_llres->FEMF_DC_T2G, cfg.f_R1, cfg.f_R2);
   /* FEMF DC TIP wire to RING wire result */
   FEMF_DC_T2R_CALC(p_femf->f_femf_dc_t2r, p_llres->FEMF_DC_T2R, cfg.f_R1, cfg.f_R3);

   return;
}

/**
   Calculates RFT results out of low level results
\param p_llres - low level results, in
\param p_hpt - RFT results, out
*/
static IFX_void_t lt_vincpe_getres_rft (VINETIC_IO_GR909_Result_t *p_llres,
                                        IFX_LT_GR909_RFT_t        *p_rft)
{
   /* test passed ? */
   if (p_llres->passed & VINETIC_IO_GR909_RFT)
      p_rft->b_result = IFX_TRUE;

   /* RFT RING wire to GND result */
   RFT_CALC(p_rft->f_rft_r2g, p_llres->RFT_R2G, cfg.f_R1, cfg.f_R2);
   /* RFT TIP wire to GND result */
   RFT_CALC(p_rft->f_rft_t2g, p_llres->RFT_T2G, cfg.f_R1, cfg.f_R2);
   /* RFT TIP wire to RING wire result */
   RFT_CALC(p_rft->f_rft_t2r, p_llres->RFT_T2R, cfg.f_R1, cfg.f_R3);

   return;
}

/**
   Calculates ROH results out of low level results
\param p_llres - low level results, in
\param p_hpt - ROH results, out
*/
static IFX_void_t lt_vincpe_getres_roh (VINETIC_IO_GR909_Result_t *p_llres,
                                        IFX_LT_GR909_ROH_t        *p_roh)
{
   /* test passed ? */
   if (p_llres->passed & VINETIC_IO_GR909_ROH)
      p_roh->b_result = IFX_TRUE;

   /* ROH TIP wire to RING wire result for low voltage */
   ROH_CALC(p_roh->f_roh_t2r_l, p_llres->ROH_T2R_L, cfg.f_R1, cfg.f_R3);
   /* ROH TIP wire to RING wire result for high voltage */
   ROH_CALC(p_roh->f_roh_t2r_h, p_llres->ROH_T2R_H, cfg.f_R1, cfg.f_R3);

   return;
}

/**
   Calculates RIT results out of low level results
\param p_llres - low level results, in
\param p_hpt   - RIT results, out
*/
static IFX_void_t lt_vincpe_getres_rit (VINETIC_IO_GR909_Result_t *p_llres,
                                        IFX_LT_GR909_RIT_t        *p_rit)
{
   /* test passed ? */
   if (p_llres->passed & VINETIC_IO_GR909_RIT)
      p_rit->b_result = IFX_TRUE;

   /* RIT result */
   RIT_CALC(p_rit->f_rit_res, p_llres->RIT_RES, cfg.f_R1, cfg.f_R3);

   return;
}

/* ============================= */
/* Global function definition    */
/* ============================= */

/**
   Configure system parameters (SLIC) to use for values calculation,
    e.g Voltage divider resistors.

\param p_cfg - handle to IFX_LT_GR909_CFG_t structure
\return
   IFX_SUCCESS / IFX_ERROR
\remark
   Usage of this function is optional. Default parameters are used
   otherwise.
*/
IFX_return_t Ifxphone_LT_GR909_Config (IFX_LT_GR909_CFG_t *p_cfg)
{
   /* check config integrity to insure software safety */
   if ((p_cfg->f_R1 == 0.0) || (p_cfg->f_R2 == 0.0) || (p_cfg->f_R3 == 0.0))
      return IFX_ERROR;

   /* use user config */
   cfg = *p_cfg;

   return IFX_SUCCESS;
}

/**
    Start a GR909 test or test sequence according to measument mask
    set with \ref IFX_LT_GR909_MASK_t.

\param fd_line   - line file descriptor
\param b_euLike  - IFX_TRUE  : EU like powerline frequency (50 Hz)
                   IFX_FALSE : US like power line frequency (60 Hz)
\param meas_mask - Measurement mask set with values out
                   of \ref IFX_LT_GR909_MASK_t
\return
   IFX_SUCCESS / IFX_ERROR
*/
IFX_return_t Ifxphone_LT_GR909_Start (IFX_int32_t   fd_line,
                                      IFX_boolean_t b_euLike,
                                      IFX_uint32_t  meas_mask)
{
   IFX_return_t             ret;
   VINETIC_IO_GR909_Start_t meas_start;
   IFX_uint32_t             *mask = &meas_start.test_mask;

   memset (&meas_start, 0, sizeof (meas_start));

   /* setup powerline frequency  */
   meas_start.pl_freq = ((b_euLike == IFX_TRUE) ? VINETIC_IO_GR909_EU_50HZ :
                          VINETIC_IO_GR909_US_60HZ);
   /* setup driver test mask according to user mask */
   *mask |= ((meas_mask & IFX_LT_GR909_HPT_MASK)  ? VINETIC_IO_GR909_HPT  : 0);
   *mask |= ((meas_mask & IFX_LT_GR909_FEMF_MASK) ? VINETIC_IO_GR909_FEMF : 0);
   *mask |= ((meas_mask & IFX_LT_GR909_RFT_MASK)  ? VINETIC_IO_GR909_RFT  : 0);
   *mask |= ((meas_mask & IFX_LT_GR909_ROH_MASK)  ? VINETIC_IO_GR909_ROH  : 0);
   *mask |= ((meas_mask & IFX_LT_GR909_RIT_MASK)  ? VINETIC_IO_GR909_RIT  : 0);

   /* no test ? */
   if (*mask == 0)
      return IFX_ERROR;

   ret = ioctl (fd_line, FIO_VINETIC_GR909_START, (IFX_int32_t) &meas_start);

   return ret;
}

/**
    Stop a GR909 test or test sequence

\param fd_line   - line file descriptor
\return
   IFX_SUCCESS / IFX_ERROR
*/
IFX_return_t Ifxphone_LT_GR909_Stop (IFX_int32_t fd_line)
{
   IFX_return_t ret = IFX_ERROR;
   /* currently not implemented for VINETIC-CPE */
   /*ret = ioctl (fd_line, FIO_VINETIC_GR909_STOP, 0);*/

   return ret;
}

/**
   Gets Gr909 measurement results.

\param fd_line  - line file descriptor
\param p_res  - handle to user result structure
\return
   IFX_SUCCESS / IFX_ERROR
\remark
   Only evaluate results which are marked as valid.
*/
IFX_return_t Ifxphone_LT_GR909_GetResults (IFX_int32_t            fd_line,
                                           IFX_LT_GR909_RESULT_t *p_res)
{
   IFX_return_t               ret;
   VINETIC_IO_GR909_Result_t  meas_res;

   /* reset structures */
   memset (&meas_res, 0, sizeof (meas_res));
   memset (p_res, 0, sizeof (*p_res));

   /* read results */
   ret = ioctl (fd_line, FIO_VINETIC_GR909_RESULT, (IFX_int32_t)&meas_res);
   if (ret == IFX_ERROR)
      return IFX_ERROR;

   /* HPT results valid ? */
   if (meas_res.valid & VINETIC_IO_GR909_HPT)
   {
      p_res->valid_mask |= IFX_LT_GR909_HPT_MASK;
      lt_vincpe_getres_hpt (&meas_res, &p_res->hpt);
   }
   /* FEMF results valid ? */
   if (meas_res.valid & VINETIC_IO_GR909_FEMF)
   {
      p_res->valid_mask |= IFX_LT_GR909_FEMF_MASK;
      lt_vincpe_getres_femf (&meas_res, &p_res->femf);
   }
   /* RFT results valid ? */
   if (meas_res.valid & VINETIC_IO_GR909_RFT)
   {
      p_res->valid_mask |= IFX_LT_GR909_RFT_MASK;
      lt_vincpe_getres_rft (&meas_res, &p_res->rft);
   }
   /* ROH results valid ? */
   if (meas_res.valid & VINETIC_IO_GR909_ROH)
   {
      p_res->valid_mask |= IFX_LT_GR909_ROH_MASK;
      lt_vincpe_getres_roh (&meas_res, &p_res->roh);
   }
   /* RIT results valid ? */
   if (meas_res.valid & VINETIC_IO_GR909_RIT)
   {
      p_res->valid_mask |= IFX_LT_GR909_RIT_MASK;
      lt_vincpe_getres_rit (&meas_res, &p_res->rit);
   }

   return IFX_SUCCESS;
}

#endif /* VIN_2CPE && TAPI_GR909 */



