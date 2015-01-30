#ifndef _DRV_TAPI_KPI_H
#define _DRV_TAPI_KPI_H
/******************************************************************************

                               Copyright (c) 2007
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
   \file drv_tapi_kpi.h
   This file contains the declaration of the "Kernel Packet Interface" (KPI).
   The KPI is used to exchange packetised data with other drivers.
*/

/* ========================================================================== */
/*                                 Includes                                   */
/* ========================================================================== */
#include "drv_api.h"
#include "drv_tapi_kpi_io.h"
#include <lib_fifo.h>

/* ========================================================================== */
/*                               Configuration                                */
/* ========================================================================== */


/* ========================================================================== */
/*                             Macro definitions                              */
/* ========================================================================== */


/* ========================================================================== */
/*                             Type definitions                               */
/* ========================================================================== */

/** Struct used in the TAPI_CHANNEL for each stream to set the target.
    The values are set by the ioctl \ref IFX_TAPI_KPI_CH_CFG_SET. */
typedef struct
{
   /** KPI channel where the stream is sent to */
   IFX_TAPI_KPI_CH_t    nKpiCh;
   /** Direct reference to the egress fifo in the target KPI group */
   FIFO_ID             *pEgressFifo;
} IFX_TAPI_KPI_STREAM_SWITCH;


/* ========================================================================== */
/*                           Function prototypes                              */
/* ========================================================================== */

/** Initialise the Kernel Packet Interface */
extern IFX_return_t IFX_TAPI_KPI_Init (IFX_void_t);

/** Shutdown the Kernel Packet Interface */
extern IFX_void_t IFX_TAPI_KPI_Cleanup (IFX_void_t);

/** Handler for the ioctl IFX_TAPI_KPI_CH_CFG_SET */
extern IFX_int32_t IFX_TAPI_KPI_ChCfgSet (TAPI_CHANNEL *pChannel,
                                          IFX_TAPI_KPI_CH_CFG_t const *pCfg);

#endif /* _DRV_TAPI_KPI_H */
