#ifndef _DRV_VINETIC_CON_PRIV_H
#define _DRV_VINETIC_CON_PRIV_H
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
   Module      : drv_vinetic_con_priv.h
   Date        : 2005-02-16
   Description : This file contains the declaration of all Connection module
******************************************************************************/
#include "ifx_types.h"

/* define signal array inputs */
#define MAX_MODULE_SIGNAL_INPUTS 5

/** Data structure for one DSP signal input.
 It contains the signal array number for its input, a pointer to the next
 input connected to the same signal, the muting information and a
 pointer to the module to which the input is connected (used as previous
 pointer */
typedef struct _MODULE_SIGNAL
{
   /** Signal array index value from ECMD_IX_SIG connected to this input. */
   IFX_uint8_t i;
   /** Signal array index value used when this input is muted. */
   IFX_uint8_t i_mute;
   /** If not 0 this input is in muted state and uses the index from i_mute
       instead of i.  */
   IFX_uint8_t mute;
   /* Parent module. In case of muting, it must be set to modified */
   struct VINDSP_MODULE *pParent;
   /** Which output this input is connected to. */
   struct VINDSP_MODULE *pOut;
   /** Points to the next input to which the outSig is connected to.
       This input signal is within a linked list of input signals that are
       all connected to one output signal. */
   struct _MODULE_SIGNAL *pNext;
} VINDSP_MODULE_SIGNAL;

/** Data structure for one DSP module, like analog line module (ALM), Coder,
    PCM. The signaling module inherits the member and adds an additional
    input list */
struct VINDSP_MODULE
{
   /** array of structures for each input of this module */
   VINDSP_MODULE_SIGNAL in[MAX_MODULE_SIGNAL_INPUTS];
   /** flag that indicates changes to the inputs (0 means no change) */
   IFX_uint8_t modified;
   /** the signal array index of this module's standard output */
   IFX_uint8_t nSignal;
   /** pointer to the first input signal which connects to the output */
   VINDSP_MODULE_SIGNAL* pInputs;
   /** flag that indicates that the standard output is muted */
   IFX_uint8_t nMute;
   /** the signal array index of this module's second output
       (only used for signaling modules) */
   IFX_uint8_t nSignal2;
   /** pointer to the first input signal which connects to the second output
       (only used for signaling modules) */
   VINDSP_MODULE_SIGNAL* pInputs2;
   /** flag that indicates that the second output is muted
       (only used for signaling modules) */
   IFX_uint8_t nMute2;
   /** defines the module type, value out of \ref VINDSP_MT */
   IFX_uint8_t nModType;
#ifdef DEBUG
   IFX_char_t name[10];
#endif /* DEBUG */
};

struct VINETIC_CON
{
   struct VINDSP_MODULE  modAlm;
   struct VINDSP_MODULE  modCod;
   struct VINDSP_MODULE  modSig;
   struct VINDSP_MODULE  modPcm;
};

#endif
