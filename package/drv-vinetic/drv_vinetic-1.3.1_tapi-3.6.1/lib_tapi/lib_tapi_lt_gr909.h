#ifndef _LIB_TAPI_LT_GR909_H
#define _LIB_TAPI_LT_GR909_H
/****************************************************************************
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
 ****************************************************************************
   Module      : lib_tapi_lt_gr909.h
   Description :
*******************************************************************************/

/**
   \file lib_tapi_lt_gr909.h
   This file contains the interface declaration .
*/

/** \defgroup TAPI_INTERFACE_LT_GR909 Line Testing Interfaces
   This section describes line testing interfaces.*/

/** \addtogroup TAPI_INTERFACE_LT_GR909 */
/*@{*/

/* ============================= */
/* Global Defines & enums        */
/* ============================= */

/** GR909 tests masks.*/
typedef enum
{
   /** Mask to select HPT. */
   IFX_LT_GR909_HPT_MASK  = (1 << 0),
   /** Mask to select FEMF. */
   IFX_LT_GR909_FEMF_MASK = (1 << 1),
   /** Mask to select RFT. */
   IFX_LT_GR909_RFT_MASK  = (1 << 2),
   /** Mask to select ROH. */
   IFX_LT_GR909_ROH_MASK  = (1 << 3),
   /** Mask to select RIT. */
   IFX_LT_GR909_RIT_MASK  = (1 << 4)
} IFX_LT_GR909_MASK_t;

/* ============================= */
/* Global Structures             */
/* ============================= */

/** Hazardous potential test results. */
typedef struct
{
   /** HPT result, passed or failed. */
   IFX_boolean_t b_result;
   /** HPT AC ring wire to gnd value, [Vrms]. */
   IFX_float_t   f_hpt_ac_r2g;
   /** HPT AC TIP wire to GND value, [Vrms]. */
   IFX_float_t   f_hpt_ac_t2g;
   /** HPT AC tip wire to ring wire value, [Vrms]. */
   IFX_float_t   f_hpt_ac_t2r;
   /** HPT DC ring wire to gnd value, [V]. */
   IFX_float_t   f_hpt_dc_r2g;
   /** HPT DC tip wire to gnd value, [V]. */
   IFX_float_t   f_hpt_dc_t2g;
   /** HPT DC tip wire to ring wire value, [V].*/
   IFX_float_t   f_hpt_dc_t2r;
} IFX_LT_GR909_HPT_t;

/** Foreign Electromotive Forces test results. */
typedef struct
{
   /** FEMF result, passed or failed. */
   IFX_boolean_t b_result;
   /** FEMF ac ring wire to gnd value, [Vrms]. */
   IFX_float_t   f_femf_ac_r2g;
   /** fFEMF AC tip wire to gnd value, [Vrms.] */
   IFX_float_t   f_femf_ac_t2g;
   /** FEMF AC tip wire to ring wire value, [Vrms]. */
   IFX_float_t   f_femf_ac_t2r;
   /** FEMF DC ring wire to gnd value, [V]. */
   IFX_float_t   f_femf_dc_r2g;
   /** FEMF DC tip wire to gnd value, [V]. */
   IFX_float_t   f_femf_dc_t2g;
   /** FEMF DC tip wire to ring wire value, [V]. */
   IFX_float_t   f_femf_dc_t2r;
} IFX_LT_GR909_FEMF_t;

/** Resistive Faults Test results. */
typedef struct
{
   /** RFT result, passed or failed. */
   IFX_boolean_t b_result;
   /** RFT ring wire to gnd value, [Ohm]. */
   IFX_float_t   f_rft_r2g;
   /** RFT tip wire to gnd value, [Ohm]. */
   IFX_float_t   f_rft_t2g;
   /** RFT tip wire to ring wire value, [Ohm]. */
   IFX_float_t   f_rft_t2r;
} IFX_LT_GR909_RFT_t;

/** Receiver OffHook test results. */
typedef struct
{
   /** ROH result, passed or failed. */
   IFX_boolean_t b_result;
   /** ROH tip wire to ring wire value for low voltage, [Ohm]. */
   IFX_float_t   f_roh_t2r_l;
   /** ROH tip wire to ring wire value for high voltage, [Ohm]. */
   IFX_float_t   f_roh_t2r_h;
} IFX_LT_GR909_ROH_t;

/** Ringer Impedance Test results. */
typedef struct
{
   /** RIT result, passed or failed. */
   IFX_boolean_t b_result;
   /** RIT value, , [Ohm]. */
   IFX_float_t   f_rit_res;
} IFX_LT_GR909_RIT_t;

/** GR909 results structure. */
typedef struct
{
   /** Valid results mask, set with \ref IFX_LT_GR909_MASK_t. */
   IFX_uint32_t         valid_mask;
   /** Hazardous Potential Test results. */
   IFX_LT_GR909_HPT_t   hpt;
   /** Foreign Electromotive Forces test results. */
   IFX_LT_GR909_FEMF_t  femf;
   /** Resistive Faults Test results. */
   IFX_LT_GR909_RFT_t   rft;
   /** Receiver OffHook test results. */
   IFX_LT_GR909_ROH_t   roh;
   /** Ringer Impedance Test results. */
   IFX_LT_GR909_RIT_t   rit;
} IFX_LT_GR909_RESULT_t;

/** GR909 parameter configuration structure, e.g to set parameters suitable to
    the SLIC used. */
typedef struct
{
   /** High resistor of the voltage divider
       connected to the line. */
   IFX_float_t f_R1;
   /** Low resistor of the voltage divider in parallel
       to the internal resistor of 1 MOhm. */
   IFX_float_t f_R2;
   /** Low resistor of the voltage divider in parallel
       to the internal resistor of 750 Ohm. */
   IFX_float_t f_R3;
   /* Add other SLIC settings here. */
} IFX_LT_GR909_CFG_t;

/* ============================= */
/* Global function declaration   */
/* ============================= */

/** Configures system parameters (SLIC) to be used for values calculation,
    e.g Voltage divider resistors.

\param p_cfg Handles to \ref IFX_LT_GR909_CFG_t structure.

 \return Returns value as follows:
   - \ref IFX_SUCCESS: if successful
   - \ref IFX_ERROR: in case of an error

\remark The usage of this function is optional. Default parameters are used
   otherwise.
*/
IFX_return_t Ifxphone_LT_GR909_Config (IFX_LT_GR909_CFG_t *p_cfg);

/** Starts a GR909 test or test sequence according to measument mask set with
 \ref IFX_LT_GR909_MASK_t.

\param fd_line   Line file descriptor.
\param b_euLike  - IFX_TRUE: EU-like powerline frequency (50 Hz).
                 - FX_FALSE: US-like power line frequency (60 Hz).
\param meas_mask Measurement mask set with values out of \ref IFX_LT_GR909_MASK_t.

\return Returns value as follows:
   - \ref IFX_SUCCESS: if successful
   - \ref IFX_ERROR: in case of an error
*/
IFX_return_t Ifxphone_LT_GR909_Start (IFX_int32_t   fd_line,
                                      IFX_boolean_t b_euLike,
                                      IFX_uint32_t  meas_mask);


IFX_return_t Ifxphone_LT_GR909_Stop (IFX_int32_t fd_line);

/** Receives Gr909 measurement results.

\param fd_line Line file descriptor.
\param p_res   Handles to result structure.

\return Returns value as follows:
   - \ref IFX_SUCCESS: if successful
   - \ref IFX_ERROR: in case of an error

\remark  Only evaluates results which are marked as valid.
*/
IFX_return_t Ifxphone_LT_GR909_GetResults (IFX_int32_t  fd_line,
                                           IFX_LT_GR909_RESULT_t *p_res);

/*@{*/ /* TAPI_INTERFACE_LT_GR909 */
#endif /* _LIB_TAPI_LT_GR909_H */


