#ifndef DRV_TAPI_STREAM_H
#define DRV_TAPI_STREAM_H
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

******************************************************************************/

/**
   \file drv_tapi_stream.h
   Contains TAPI functions declaration for fifo, bufferpool.
*/

/* ============================= */
/* Includes                      */
/* ============================= */


/* ============================= */
/* Local definitions             */
/* ============================= */

/* ============================= */
/* Local structures              */
/* ============================= */

/* ============================= */
/* Local variable definition     */
/* ============================= */

/* ============================= */
/* Global variable definition    */
/* ============================= */

/* ============================= */
/* Local function declaration    */
/* ============================= */


/* Used only on TAPI side */
IFX_return_t TAPI_InitUpStreamFifo(TAPI_CHANNEL* pTapiCh);
IFX_return_t TAPI_ResetUpStreamFifo(TAPI_CHANNEL* pTapiCh);
IFX_return_t TAPI_PrepareVoiceBufferPool(IFX_void_t);
IFX_return_t TAPI_Free_FifoBufferPool(TAPI_DEV* pTapiDev, IFX_int32_t nChCnt);
IFX_return_t TAPI_InitDownStreamFifo(TAPI_DEV* pTapiDev);
IFX_return_t TAPI_ResetDownStreamFifo(TAPI_DEV* pTapiDev);

#endif /* DRV_TAPI_STREAM_H */
