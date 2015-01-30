#ifndef _DRV_DAA_H
#define _DRV_DAA_H
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
   \file drv_daa.h

   Description : This file contains the defines, the structures declarations
                 the tables declarations and the global functions declarations.
   Remarks:
      Use compiler switch ENABLE_TRACE for trace output, for debugging purposes.
      Compiler switch for OS is needed. Use LINUX for linux and VXWORKS for
      VxWorks..
*******************************************************************************/


/* ============================= */
/* Global Defines                */
/* ============================= */

/* Traces */
/** enable traces */
#define ENABLE_TRACE
/** enable logs, interrupt safe print's */
#define ENABLE_LOG


/* ============================= */
/* includes                      */
/* ============================= */

#include <ifx_types.h>
#include <sys_drv_ifxos.h>

#ifdef LINUX
#include "drv_tapi_linux.h"
#endif /* LINUX */


#include "sys_drv_debug.h"

#ifdef LINUX
#include "drv_daa_linux.h"
#endif /* LINUX */

#ifdef VXWORKS
#include "drv_daa_vxworks.h"
#endif /* VXWORKS */

#ifdef NO_OS
#include "drv_daa_noOS.h"
#endif /* NO_OS */

#include <drv_tapi_io.h>
#include <drv_tapi_event_io.h>
#include <drv_tapi_fxo_ll_interface.h>
#include "drv_daa_boards.h"

extern Timer_ID       TAPI_Create_Timer  (TIMER_ENTRY pTimerEntry, IFX_int32_t nArgument);
extern IFX_boolean_t  TAPI_SetTime_Timer (Timer_ID Timer, IFX_uint32_t nTime, IFX_boolean_t bPeriodically, IFX_boolean_t bRestart);
extern IFX_boolean_t  TAPI_Delete_Timer  (Timer_ID Timer);
extern IFX_boolean_t  TAPI_Stop_Timer    (Timer_ID Timer);

/** DAA ring state machine states */
typedef enum
{
   DAA_RING_STATE_NOT_RINGING,
   DAA_RING_STATE_RINGING
} DAA_RING_STATES_t;
#define DAA_RING_STATE_NO_STATES                2

/** DAA ring state machine events */
typedef enum
{
   DAA_RING_EVT_TIMEOUT,
   DAA_RING_EVT_RING_IRQ
} DAA_RING_EVT_t;

/** DAA battery state machine states */
typedef enum
{
   DAA_BAT_STATE_NOT_FEEDED,
   DAA_BAT_STATE_PRE_FEEDED,
   DAA_BAT_STATE_FEEDED,
   DAA_BAT_STATE_PRE_OSI,
   DAA_BAT_STATE_OSI
} DAA_BAT_STATES_t;
#define    DAA_BAT_STATE_NO_STATES              5

/** DAA battery state machine events */
typedef enum
{
   DAA_BAT_EVT_BATTERY_IRQ,
   DAA_BAT_EVT_BATTERY_DROP_IRQ,
   DAA_BAT_EVT_TIMEOUT
} DAA_BAT_EVT_t;

/** DAA apoh state machine states */
typedef enum
{
   DAA_APOH_STATE_NOPOH,
   DAA_APOH_STATE_PRE_APOH,
   DAA_APOH_STATE_APOH,
   DAA_APOH_STATE_PRE_NOPOH
} DAA_APOH_STATES_t;
#define DAA_APOH_STATE_NO_STATES                4

/** DAA apoh state machine events */
typedef enum
{
   DAA_APOH_EVT_TIMEOUT,
   DAA_APOH_EVT_APOH_IRQ,
   DAA_APOH_EVT_NOPOH_IRQ
} DAA_APOH_EVT_t;

/** DAA pol state machine states */
typedef enum
{
   DAA_POL_STATE_NORMAL,
   DAA_POL_STATE_PRE_REVERSE,
   DAA_POL_STATE_REVERSE,
   DAA_POL_STATE_PRE_NORMAL
} DAA_POL_STATES_t;
#define DAA_POL_STATE_NO_STATES                4

/** DAA pol state machine events */
typedef enum
{
   DAA_POL_EVT_TIMEOUT,
   DAA_POL_EVT_NORMAL_IRQ,
   DAA_POL_EVT_REVERSE_IRQ
} DAA_POL_EVT_t;


/** DAA device specific data storage */
typedef struct
{
   /** daa resource number as registered to TAPI */
   IFX_uint16_t         nDaa;
   /** locking for dev data */
   IFXOS_INTSTAT        lock;
   /** daa ring counter; used to suppress wrong ring events */
   IFX_uint16_t         nRingCnt;
   /** timer id for flashHook timer */
   Timer_ID             tid_flashHook;
   /** timer id for ring state machine */
   Timer_ID             tid_ring;
   /** timer id for battery state machine */
   Timer_ID             tid_bat;
   /** timer id for polarity state machine */
   Timer_ID             tid_pol;
   /** timer id for apoh (another phone off hook) state machine */
   Timer_ID             tid_apoh;
   /** current state of the ring state machine */
   DAA_RING_STATES_t    state_ring;
   /** current state of the battery state machine */
   DAA_BAT_STATES_t     state_bat;
   /** current state of the apoh state machine */
   DAA_APOH_STATES_t    state_apoh;
   /** current state of the pol state machine */
   DAA_POL_STATES_t     state_pol;
} DAA_DEV_t;

#endif /* _DRV_DAA_H */
