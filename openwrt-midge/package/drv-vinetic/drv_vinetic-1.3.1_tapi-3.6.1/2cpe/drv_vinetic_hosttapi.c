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
   Module      : drv_vinetic_tapi.c
   Date        : 2003-10-28
   Description : This file contains the implementations of vinetic related
                 tapi low level functions.
*******************************************************************************/

/* ============================= */
/* includes                      */
/* ============================= */
#include "drv_vinetic_api.h"
#include "drv_vinetic_dcctl.h"
#include "drv_vinetic_dspconf.h"
#include "drv_vinetic_tone.h"

/* ============================= */
/* Local Macros & Definitions    */
/* ============================= */

/* ============================= */
/*             Enums             */
/* ============================= */

/* ============================= */
/* Local function declaration    */
/* ============================= */


/* ============================= */
/* Local variable definition     */
/* ============================= */


/* ============================= */
/* Local functions definitions   */
/* ============================= */


/* ============================= */
/* Global functions definitions  */
/* ============================= */

/*******************************************************************************
Description:
   Stops the predifined tone currently played
Arguments:
   pChannel        - handle to TAPI_CHANNEL structure
Return:
   IFX_SUCCESS or IFX_ERROR
*******************************************************************************/
IFX_int32_t TAPI_LL_Host_Phone_Tone_Off(TAPI_CHANNEL *pChannel)
{
   TRACE (VINETIC, DBG_LEVEL_HIGH,
         ("Error, 2CPE has no TG : Stop simple tone "
          "via tone api instead\r\n"));

   return IFX_ERROR;
}


/*******************************************************************************
Description:
   Sets the tone level of the tone currently played.
Arguments:
   pChannel        - handle to TAPI_CHANNEL structure
   pToneLevel      - pointer to IFX_TAPI_PREDEF_TONE_LEVEL_t structure
Return:
   IFX_SUCCESS or IFX_ERROR
Remark:

*******************************************************************************/
IFX_return_t IFX_TAPI_LL_Tone_Set_Level (IFX_TAPI_LL_CH_t *pLLChannel,
                                         IFX_TAPI_PREDEF_TONE_LEVEL_t const *pToneLevel)
{
   TRACE (VINETIC, DBG_LEVEL_HIGH, ("Error, 2CPE has no TG : "
          "Set tone level via tone table configuration instead\r\n"));

   return IFX_ERROR;
}

/*******************************************************************************
Description:
   Configures the predefined tone of given index and start the playing.
Arguments:
   pChannel       - handle to TAPI_CHANNEL structure
   nToneIndex     - user selected tone index
Return:
   IFX_SUCCESS or IFX_ERROR
Remarks:
   For 2CPE, predefined tones are also played via the DSP, as no ALM tone
   generator is available.
*******************************************************************************/
IFX_int32_t TAPI_LL_Host_Phone_Tone_On (TAPI_CHANNEL *pChannel,
                                        IFX_int32_t nToneIndex)
{
   TRACE (VINETIC, DBG_LEVEL_HIGH,
         ("Error, 2CPE has no TG : Play tone as simple "
          "tone via tone api instead\r\n"));

   return IFX_ERROR;
}
