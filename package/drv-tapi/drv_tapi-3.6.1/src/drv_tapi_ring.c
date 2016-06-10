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

/** \file drv_tapi_ring.c
    Contains TAPI Ringing Services.
    Implementation of the TAPI ring state machine which controls ringing on
    analog lines.
    \remarks
    All operations done by functions in this module are operating on analog
    phone lines and require a file descriptor with an ALM module. */

/** \defgroup RING_IMPLEMENTATION Ringing implementation
    Implementation of the ringing services. */
/* @{ */

/* ============================= */
/* Includes                      */
/* ============================= */
#include "drv_tapi.h"
#include "drv_tapi_ll_interface.h"
#ifdef TAPI_CID
#include "drv_tapi_cid.h"
#endif /* TAPI_CID */

/* ============================= */
/* Local macros and definitions  */
/* ============================= */


/* ============================= */
/* Local function declaration    */
/* ============================= */
static IFX_uint32_t ifx_tapi_ring_get_next_time (TAPI_RING_DATA_t *pRingData,
                                                 IFX_boolean_t *bIsBurst,
                                                 IFX_boolean_t *bLast);
static IFX_int32_t  ifx_tapi_ring_cadence_play  (TAPI_CHANNEL *pChannel);
static IFX_void_t   ifx_tapi_ring_OnTimer       (Timer_ID Timer,
                                                 IFX_int32_t nArg);


/* ============================= */
/* Local function declaration    */
/* ============================= */

/**
   Gets the time of the next sequence.

   Each cadence consists of multiple sequences of bits of the same value 1/0.
   This function counts the number of bits in the sequence that starts at the
   current position. The minimum count is in all cases at least one bit.
   Each bit represents 50 ms time.
   When the entire cadence consists only of set or cleared bits the time
   will be set to 0 to indicate that the sequence is infinite. To detect
   such a condition the algorithm aborts on the second reload of a cadence.

   \param   pRingData   Pointer to the ring data used for this call.
   \param   bIsBurst    Pointer to return if this sequence is a burst.
   \param   bLast       Pointer to return if this is the last sequence in this
                        cadence.

   \return  Time of the sequence in ms.
            The value 0 is special and indicates an infinite sequence.
*/
static IFX_uint32_t ifx_tapi_ring_get_next_time(TAPI_RING_DATA_t *pRingData,
                                                IFX_boolean_t *bIsBurst,
                                                IFX_boolean_t *bLast)
{
   /* Cadence has a maximum number of 320 bits so we use 16-bit uint. */
   register IFX_uint16_t nBitCount = 0,
                         nBitPos = pRingData->nBitPos,
                         nBitsInCadence,
                         nReloadCount = 0;
   IFX_uint8_t           nPattern,
                         nCurrent,
                         *pBuf;

   /* Set to false until proven otherwise */
   *bLast = IFX_FALSE;

   /* Get the current cadence bit pattern */
   pBuf = pRingData->pCurrentCadence;
   /* nBitsInCadence has a range from 1 to n */
   nBitsInCadence = pRingData->BitsInCurrentCadence;

   /* Get the first byte of the current sequence
      and position on the first bit of the sequence */
   nCurrent = (pBuf[nBitPos >> 3] << (nBitPos & 0x07));
   /* Determine the first bit of the sequence */
   nPattern = nCurrent & 0x80;

   /* loop while the current bit matches the first bit of the sequence */
   /* abort if we reloaded the cadence pattern for the second time - this
      indicates that there was no change in the pattern and also the reloaded
      pattern will not contain a change meaning the sequence is infinite */
   while ( ((nCurrent & 0x80) == nPattern) && (nReloadCount < 2) )
   {
      /* check if we can skip whole bytes in one step */
      if (((nBitPos + 8) < nBitsInCadence) &&
          ((nBitPos & 0x07) == 0x00) &&
          ((nCurrent == 0xFF) || (nCurrent == 0x00)))
      {
         /* skip whole byte if all bits are set / unset */
         nBitPos += 8;
         nBitCount += 8;
      }
      else
      {
         /* otherwise advance bit by bit */
         nBitPos++;
         nBitCount++;
      }

      /* nBitsInCadence has a range from 1 to n while nBitPos counts from 0
         to n-1 so we use a check of greater equal */
      if (nBitPos >= nBitsInCadence)
      {
         /* if we want to support max ring abort with periodic cadences that
            start with pause abort here if the repeat count is zero */

         /* end of this cadence reload the periodic cadence pattern */
         pBuf = pRingData->pCurrentCadence = pRingData->RingCadence.data;
         nBitsInCadence =
            pRingData->BitsInCurrentCadence = pRingData->RingCadence.nr;

         /* return that this was the last time in this sequence */
         *bLast = IFX_TRUE;

         /* reset the bit counter within the cadence */
         nBitPos = 0;

         /* count this cadence pattern reload */
         nReloadCount++;
      }

      /* load the next position in the current cadence */
      nCurrent = (pBuf[nBitPos >> 3] << (nBitPos & 0x07));
   }

   /* Store the bit counter */
   pRingData->nBitPos = nBitPos;

   /* Return whether this is a ring-burst or ring-pause sequence */
   *bIsBurst = nPattern ? IFX_TRUE : IFX_FALSE;

   /* Return the time of this sequence.
      - if infinite sequence is detected 0 is returned.
      - for finite sequence returns time in milliseconds. Each bit is 50ms */
   return (nReloadCount < 2) ? (IFX_uint32_t)nBitCount * 50 : 0;
}


/**
   Plays the next sequence of a cadence.

   Sets the linemode according to the next sequence and starts a timer for
   the duration of the sequence.

   \param   pChannel    Pointer to TAPI_CHANNEL structure.

   \return
     - \ref IFX_SUCCESS: if successful
     - \ref IFX_ERROR: in case of an error
*/
static IFX_int32_t ifx_tapi_ring_cadence_play(TAPI_CHANNEL *pChannel)
{
   TAPI_RING_DATA_t *pRingData = &pChannel->TapiRingData;
   IFX_boolean_t bBurst, bLast;
   IFX_uint32_t nTime = 0;

   /* The final flag indicates if this call was done after the last sequence
      of the last cadence was played and so final actions need to be performed.
      If not the next sequence is determined the linemode set and the timer
      started for the duration of the sequence. */
   if (pRingData->bFinal == IFX_FALSE)
   {
      /* get the duration of the sequence and if this is a burst or a pause */
      nTime = ifx_tapi_ring_get_next_time(pRingData, &bBurst, &bLast);

      /* shows the values of each step that is done
      TRACE(TAPI_DRV, DBG_LEVEL_LOW,
            ("RINGING: time:%d, burst:%d, last:%d\n\r", nTime, bBurst, bLast));
      */

      if (bBurst)
      {
         /* ring burst */
         pRingData->bIsRingBurstState = IFX_TRUE;
         TAPI_Phone_Set_Linefeed (pChannel, IFX_TAPI_LINE_FEED_RING_BURST);
      }
      else
      {
         /* ring pause */
         pRingData->bIsRingBurstState = IFX_FALSE;
         TAPI_Phone_Set_Linefeed (pChannel, IFX_TAPI_LINE_FEED_RING_PAUSE);
      }

      /* if this is the last sequence of one cadence decrease ringing count */
      if ((bLast == IFX_TRUE) && (pRingData->nRingsLeft > 0))
      {
         /* decrement the number of rings still to play */
         pRingData->nRingsLeft--;
         if (pRingData->nRingsLeft == 0)
         {
            /* Set flag that this is the final sequence */
            pRingData->bFinal = IFX_TRUE;
         }
      }
#ifdef TAPI_CID
      /* if this is the last pause sequence of one cadence notify CID */
      if ((bLast == IFX_TRUE) && (bBurst == IFX_FALSE))
      {
         TAPI_Phone_CID_OnRingpause(pChannel);
      }
#endif /* TAPI_CID */

      /* if the time is 0 this sequence is infinite so no timer is needed
         otherwise start the timer for the duration of the sequence */
      if (nTime > 0)
      {
         /* start the timer for the duration of this sequence */
         TAPI_SetTime_Timer (pRingData->RingTimerID,
                             nTime, IFX_FALSE, IFX_FALSE);
      }
   }
   else
   {
      IFX_TAPI_EVENT_t tapiEvent;

      /* set linemode back to standby */
      TAPI_Phone_Set_Linefeed(pChannel, IFX_TAPI_LINE_FEED_STANDBY);

      /* set flag that ringing stopped */
      pRingData->bRingingMode = IFX_FALSE;

      IFXOS_WakeUpEvent(pChannel->TapiRingEvent);

      /* send the event that the ringing phase has ended */
      memset(&tapiEvent, 0, sizeof(IFX_TAPI_EVENT_t));
      tapiEvent.id = IFX_TAPI_EVENT_FXS_RINGING_END;
      IFX_TAPI_Event_Dispatch(pChannel,&tapiEvent);
   }

   return IFX_SUCCESS;
}


/**
   Ring-timer callback function.

   \param   Timer       TimerID of timer that expired.
   \param   nArg        Argument of timer. This argument is a pointer to the
                        TAPI_CHANNEL structure.

   \return  none.
*/
static IFX_void_t ifx_tapi_ring_OnTimer(Timer_ID Timer, IFX_int32_t nArg)
{
   TAPI_CHANNEL *pChannel  = (TAPI_CHANNEL *) nArg;

   /* lock channel */
   IFXOS_MutexLock(pChannel->semTapiChDataLock);

   /* Avoid calling the playout function after ringing was stopped. This might
      occur when the timer is not executed immediately by the framework. */
   if (pChannel->TapiRingData.bRingingMode != IFX_FALSE)
   {
      /* set the line to the next state and start the timer for the duration
         of the cadence */
      ifx_tapi_ring_cadence_play (pChannel);
   }

   /* unlock channel */
   IFXOS_MutexUnlock(pChannel->semTapiChDataLock);
}


/* ============================= */
/* Global function definition    */
/* ============================= */

/**
   Initialise ringing on the given channel.

   Initialise the data structures and resources needed for the ringing
   state machine.

   \param   pChannel    Pointer to TAPI_CHANNEL structure.

   \return
     - \ref IFX_SUCCESS: if successful
     - \ref IFX_ERROR: in case of an error
*/
IFX_int32_t IFX_TAPI_Ring_Initialise(TAPI_CHANNEL *pChannel)
{
   TAPI_RING_DATA_t *pTapiRingData = &pChannel->TapiRingData;
   IFX_int32_t ret;

   /* create ring engine timer if not already existing */
   IFXOS_MutexLock(pChannel->semTapiChDataLock);
   if (pTapiRingData->RingTimerID == 0)
   {
      /* initialize (create) ring timer */
      pTapiRingData->RingTimerID =
         TAPI_Create_Timer((TIMER_ENTRY)ifx_tapi_ring_OnTimer, (IFX_int32_t)pChannel);
      if(pTapiRingData->RingTimerID == 0)
      {
         IFXOS_MutexUnlock(pChannel->semTapiChDataLock);
         return IFX_ERROR;
      }
   }
   IFXOS_MutexUnlock(pChannel->semTapiChDataLock);

   /* stop ringing if already running */
   ret = IFX_TAPI_Ring_Stop(pChannel);

   IFXOS_MutexLock(pChannel->semTapiChDataLock);
   /* set default ringing cadence: 2 sec burst + 2 sec pause */
   memset(&pTapiRingData->RingCadence, 0x00, sizeof(pTapiRingData->RingCadence));
   pTapiRingData->RingCadence.initialNr = 0;  /* no initial cadence */
   pTapiRingData->RingCadence.nr        = 80; /* 10 bytes with 8 bit each */
   memset(pTapiRingData->RingCadence.data, 0xFF, 5);
   /* set no limits on ring repitition */
   pTapiRingData->nMaxRings = 0;
   IFXOS_MutexUnlock(pChannel->semTapiChDataLock);

   return ret;
}


/**
   Cleanup ringing on the given channel.

   Free the resources needed for the ringing state machine.

   \param   pChannel    Pointer to TAPI_CHANNEL structure.

   \return
     - \ref IFX_SUCCESS: if successful
*/
IFX_int32_t IFX_TAPI_Ring_Cleanup(TAPI_CHANNEL *pChannel)
{
   TAPI_RING_DATA_t *pTapiRingData = &pChannel->TapiRingData;

   /* todo: Check why protection semaphore is already zero in some cases.
   IFXOS_MutexLock(pChannel->semTapiChDataLock);
   */

   /* unconditionally destruct the ring engine timer if existing */
   if (pTapiRingData->RingTimerID != 0)
   {
      TAPI_Delete_Timer (pTapiRingData->RingTimerID);
      pTapiRingData->RingTimerID = 0;
   }

   /*
   IFXOS_MutexUnlock(pChannel->semTapiChDataLock);
   */

   return IFX_SUCCESS;
}


/**
   Starts the ring engine.

   \param   pChannel    Pointer to TAPI_CHANNEL structure.
   \param   bStartWithInitial   Select whether to start initial cadence if
                        possible or if to start with periodic cadence.

   \return
     - \ref IFX_SUCCESS: if successful
     - \ref IFX_ERROR: in case of an error
*/
IFX_int32_t IFX_TAPI_Ring_Engine_Start(TAPI_CHANNEL *pChannel,
                                       IFX_boolean_t bStartWithInitial)
{
   TAPI_RING_DATA_t *pRingData = &pChannel->TapiRingData;

   /* set the first cadence to be played */
   if ((pRingData->RingCadence.initialNr != 0) &&
       (bStartWithInitial == IFX_TRUE))
   {
      /* first cadence is taken from initial cadence */
      pRingData->pCurrentCadence = pRingData->RingCadence.initial;
      pRingData->BitsInCurrentCadence = pRingData->RingCadence.initialNr;
   }
   else
   {
      /* use periodic cadence as first cadence */
      pRingData->pCurrentCadence = pRingData->RingCadence.data;
      pRingData->BitsInCurrentCadence = pRingData->RingCadence.nr;
   }

   /* start with bit 0 */
   pRingData->nBitPos = 0;
   /* reset flag that indicates the final sequence of the last cadence */
   pRingData->bFinal = IFX_FALSE;
   /* get the maximum number of rings from configuration (0 means forever) */
   pRingData->nRingsLeft = pRingData->nMaxRings;

   /* set the line to the first state and start the timer for the duration
      of the cadence. */
   ifx_tapi_ring_cadence_play(pChannel);

   /* set flag that ringing is running */
   pRingData->bRingingMode = IFX_TRUE;

   return IFX_SUCCESS;
}

/**
   This service sets the ring cadence. (Old style interface)

   \param   pChannel    Pointer to TAPI_CHANNEL structure.
   \param   nCadence    Contains the encoded cadence sequence.

   \return
     - \ref IFX_SUCCESS: if successful
     - \ref IFX_ERROR: in case of an error

   \remarks
     - This function codes the 32 cadence bits into the 320 bits of the
       high resolution buffer.
     - Each bit in this cadence represents 50 ms time.
*/
IFX_int32_t  IFX_TAPI_Ring_SetCadence(TAPI_CHANNEL *pChannel,
                                      IFX_uint32_t nCadence)
{
   TAPI_RING_DATA_t *pRingData = &pChannel->TapiRingData;
   IFX_char_t  nBuffer      = 0,
               *pCadence    = pRingData->RingCadence.data;
   IFX_uint8_t nByteCounter = 0,
               nBitCounter  = 0;
   IFX_int8_t  i, n;

   /* check if the starting bit of periodic cadence is set */
   /* (it is not allowed that cadence starts with zero bit) */
   if ((nCadence & 0x80000000L) == 0)
   {
      TRACE(TAPI_DRV,DBG_LEVEL_HIGH,
            ("\n\rDRV_ERROR: ring cadence may not start with 0\n\r"));
      return IFX_ERROR;
   }
   /* return error if cadence is 0 */
   if (nCadence == 0x0)
   {
      TRACE(TAPI_DRV,DBG_LEVEL_HIGH,
            ("\n\rDRV_ERROR: ring cadence may not be 0 at all\n\r"));
      return IFX_ERROR;
   }
   /* abort if the line is currently ringing */
   if (pRingData->bRingingMode != IFX_FALSE)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
            ("\n\rDRV_ERROR: cannot set cadence while channel is ringing\n\r"));
      return IFX_ERROR;
   }

   for (i = 0; i < 32; i++)
   {
      if (nCadence & 0x80000000)
      {
         nBuffer = 0x01;
      }
      else
      {
         nBuffer = 0x00;
      }
      for (n = 0; n < 10; n++)
      {
         pCadence [nByteCounter]  = (IFX_char_t)((IFX_uint8_t)pCadence [nByteCounter] << 1);
         pCadence [nByteCounter] |= nBuffer;
         if (nBitCounter == 7)
         {
            nByteCounter++;
            nBitCounter = 0;
         }
         else
         {
            nBitCounter++;
         }
      }
      nCadence <<= 1;
   }
   /* no initial non periodic cadence */
   pRingData->RingCadence.initialNr = 0;
   /* length of periodic cadence : 320 bits = 40 Bytes */
   pRingData->RingCadence.nr = 320;

   return IFX_SUCCESS;
}


/**
   This service sets the ring high resolution cadence for the ringing services.

   \param   pChannel    Pointer to TAPI_CHANNEL structure.
   \param   pCadence    Pointer to struct with cadence definition.

   \return
     - \ref IFX_SUCCESS: cadence is set
     - \ref IFX_ERROR: cadence contains errors and is not set

   \remarks
     - The initial cadence may have a zero length while the periodic cadence
       must have at least a length of one bit.
     - The initial as well as the periodic cadence may not consists of all
       0 bits but all bits set to 1 is allowed.
*/
IFX_int32_t  IFX_TAPI_Ring_SetCadenceHighRes(TAPI_CHANNEL *pChannel,
                                             IFX_TAPI_RING_CADENCE_t const *pCadence)
{
   IFX_int32_t  i, j;
   IFX_uint8_t nTestZero;

   /* abort if the line is currently ringing */
   if (pChannel->TapiRingData.bRingingMode != IFX_FALSE)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
            ("\n\rDRV_ERROR: cannot set cadence while channel is ringing\n\r"));
      return IFX_ERROR;
   }
   /* abort if the given counter of initial bits or data bits is out of range */
   if ((pCadence->initialNr < 0) ||
       (pCadence->initialNr > (IFX_TAPI_RING_CADENCE_MAX_BYTES * 8)) ||
       (pCadence->nr < 1) ||
       (pCadence->nr > (IFX_TAPI_RING_CADENCE_MAX_BYTES * 8)))
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
            ("\n\rDRV_ERROR: count of initial or data bits out of range\r\n"));
      return IFX_ERROR;
   }
   /* abort if the initial count is set but the entire cadence is zero */
   if (pCadence->initialNr != 0)
   {
      for (i = 0, nTestZero = 0; i < (pCadence->initialNr / 8); i++)
      {
         nTestZero |= pCadence->initial[i];
      }
      for (j = 0; j < (pCadence->initialNr % 8); j++)
      {
         nTestZero |= ((IFX_uint8_t)pCadence->initial[i] << j) & 0x80;
      }
      if (nTestZero == 0)
      {
         TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
               ("\n\rDRV_ERROR: no bit is set in the initial cadence\r\n"));
         return IFX_ERROR;
      }
   }
   /* abort if the entire periodic cadence is zero or one */
   for (i = 0, nTestZero = 0; i < (pCadence->nr / 8); i++)
   {
      nTestZero |= pCadence->data[i];
   }
   for (j = 0; j < (pCadence->nr % 8); j++)
   {
      nTestZero |= ((IFX_uint8_t)pCadence->data[i] << j) & 0x80;
   }
   if (nTestZero == 0)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
            ("\n\rDRV_ERROR: no bit is set in the periodic cadence\r\n"));
      return IFX_ERROR;
   }

   /* copy cadence data into TAPIstructure */
   /* to round up to the next byte seven is added to the bit counters */
   /* maybe copying the full buffer is better than the calculation below? */
   memcpy(pChannel->TapiRingData.RingCadence.initial,
          pCadence->initial, ((pCadence->initialNr + 7) / 8));
   memcpy(pChannel->TapiRingData.RingCadence.data,
          pCadence->data, ((pCadence->nr + 7) / 8));

   /* length of cadence data for the initial cadence */
   pChannel->TapiRingData.RingCadence.initialNr = pCadence->initialNr;
   /* length of cadence data for the periodic cadence */
   pChannel->TapiRingData.RingCadence.nr        = pCadence->nr;

   return IFX_SUCCESS;
}


/**
   Prepare ringing by checking current state.

   \param   pChannel    Pointer to TAPI_CHANNEL structure.

   \return
     - \ref IFX_SUCCESS: prepare ok
     - \ref IFX_ERROR:   prepare failed

   \remarks
     - It is assumed that this function is called with holding the TapiDataLock!
     - Operation is done on a phone channel but updates also CID in a connected
       data channel.
*/
IFX_int32_t IFX_TAPI_Ring_Prepare(TAPI_CHANNEL *pChannel)
{
   TAPI_RING_DATA_t *pRingData = &pChannel->TapiRingData;
   IFX_int32_t ret = IFX_SUCCESS;

   /* check if ring timer is available */
   if (pRingData->RingTimerID == 0)
   {
      TRACE(TAPI_DRV,DBG_LEVEL_HIGH,
            ("\n\rDRV_ERROR: no ring timer available for this channel\n\r"));
      return IFX_ERROR;
   }
   /* check if cadence is set */
   if (!pRingData->RingCadence.nr)
   {
      TRACE(TAPI_DRV,DBG_LEVEL_HIGH,
            ("\n\rDRV_ERROR: ring cadence is not set\n\r"));
      return IFX_ERROR;
   }
   /* abort if the line is currently ringing */
   if (pRingData->bRingingMode != IFX_FALSE)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
            ("\n\rDRV_ERROR: channel is currently ringing\n\r"));
      return IFX_ERROR;
   }
   /* check if metering is active */
   if (pChannel->TapiMeterData.bMeterActive)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
            ("\n\rDRV_ERROR: metering is active\n\r"));
      return IFX_ERROR;
   }
#ifdef TAPI_CID
   {
      TAPI_CHANNEL *pDataCh;
      IFX_uint8_t   data_ch;
      IFX_boolean_t bBurst, bLast;
      IFX_uint32_t  nTime,
                    nBurstTime = 0;

      TAPI_Phone_Get_Data_Channel (pChannel, &data_ch);
      pDataCh = &pChannel->pTapiDevice->pTapiChanelArray [data_ch];

      if (pDataCh->TapiCidTx.bActive)
      {
         TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("\n\rDRV_ERROR: caller id is "
                "active on data ch %d associated with this phone ch %d\n\r",
                data_ch, pChannel->nChannel));
         return IFX_ERROR;
      }

      /* For CID we need the time from the beginning of the first cadence to
         the last burst in the first cadence and additonally the time of the
         last pause sequence in the first cadence. */

      /* find the first cadence to be played */
      if ((pRingData->RingCadence.initialNr != 0))
      {
         /* first cadence is taken from initial cadence */
         pRingData->pCurrentCadence = pRingData->RingCadence.initial;
         pRingData->BitsInCurrentCadence = pRingData->RingCadence.initialNr;
      }
      else
      {
         /* use periodic cadence as first cadence */
         pRingData->pCurrentCadence = pRingData->RingCadence.data;
         pRingData->BitsInCurrentCadence = pRingData->RingCadence.nr;
      }

      /* start with bit 0 */
      pRingData->nBitPos = 0;
      /* Get first sequence. This cannot be the last pause of a cadence. */
      nTime = ifx_tapi_ring_get_next_time(pRingData, &bBurst, &bLast);

      /* find time without last pause and time of last pause */
      /* stop loop when we find the last sequence of a cadence
         (this is also implicitly true for an infinite sequence) */
      do
      {
         nBurstTime += nTime;
         nTime = ifx_tapi_ring_get_next_time(pRingData, &bBurst, &bLast);
      } while (bLast == IFX_FALSE);

      if (bBurst == IFX_TRUE)
      {
         nBurstTime += nTime;
         nTime = 0;
      }

      pDataCh->TapiCidConf.cadenceRingBurst = nBurstTime;
      pDataCh->TapiCidConf.cadenceRingPause = nTime;
   }
#endif /* TAPI_CID */

   return ret;
}


/**
   Starts non-blocking ringing.

   \param   pChannel    Pointer to TAPI_CHANNEL structure.

   \return
     - \ref IFX_SUCCESS: ring start successful
     - \ref IFX_ERROR:   ring start failed

   \remarks
   To start ringing with caller id use the function \ref TAPI_Phone_CID_Seq_Tx.
*/
IFX_int32_t IFX_TAPI_Ring_Start(TAPI_CHANNEL *pChannel)
{
   IFX_int32_t    ret;

   /* Abort if phone is currently off-hook */
   if (pChannel->TapiOpControlData.bHookState == IFX_TRUE)
   {
      /* Cannot start ringing while phone is off-hook */
      return IFX_ERROR;
   }

   /* begin of protected area */
   IFXOS_MutexLock(pChannel->semTapiChDataLock);

   ret = IFX_TAPI_Ring_Prepare(pChannel);

   if (ret == IFX_SUCCESS)
   {
      /* start the timer */
      ret = IFX_TAPI_Ring_Engine_Start(pChannel, IFX_TRUE);
   }

   /* end of protected area */
   IFXOS_MutexUnlock(pChannel->semTapiChDataLock);

   return ret;
}


/**
   Stops non-blocking ringing.

   This interface stops ringing both with or without caller id.

   \param   pChannel    Pointer to TAPI_CHANNEL structure.

   \return
     - \ref IFX_SUCCESS: ring stop successful
     - \ref IFX_ERROR:   ring stop failed
*/
IFX_int32_t IFX_TAPI_Ring_Stop(TAPI_CHANNEL *pChannel)
{
   TAPI_RING_DATA_t *pRingData = &pChannel->TapiRingData;
   IFX_int32_t ret = IFX_SUCCESS;

#ifdef TAPI_CID
   {
      TAPI_CHANNEL *pDataCh;
      IFX_uint8_t  data_ch;

      TAPI_Phone_Get_Data_Channel (pChannel, &data_ch);
      pDataCh = &pChannel->pTapiDevice->pTapiChanelArray [data_ch];
      /* in case CID tx is ongoing stop it now this will also prevent the
         start of any periodic ringing after CID has finished */
      if (pDataCh->TapiCidTx.bActive == IFX_TRUE)
      {
         ret = TAPI_Phone_CID_Stop_Tx(pDataCh);
      }
   }
#endif /* TAPI_CID */

   /* lock channel */
   IFXOS_MutexLock(pChannel->semTapiChDataLock);

   /* stop only when phone is ringing (even if CID stop above failed) */
   if (pRingData->bRingingMode != IFX_FALSE)
   {
      TAPI_Stop_Timer(pRingData->RingTimerID);
      /* reset ringing data */
      pRingData->bRingingMode        = IFX_FALSE;
      pRingData->bIsRingBurstState   = IFX_FALSE;

      /* switch off ringing */
      if (TAPI_Phone_Set_Linefeed(pChannel,
                                  IFX_TAPI_LINE_FEED_STANDBY) != IFX_SUCCESS)
      {
         ret = IFX_ERROR;
      }
   }

   /* unlock channel */
   IFXOS_MutexUnlock(pChannel->semTapiChDataLock);

   return ret;
}


/**
   Sets the ring configuration for the ringing services.

   \param   pChannel    Pointer to TAPI_CHANNEL structure.
   \param   pRingConfig Pointer to struct with ring modes (IFX_TAPI_RING_CFG_t).

   \return
     - \ref IFX_SUCCESS: Setting ring config successful
     - \ref IFX_ERROR:   Setting ring config failed
*/
IFX_int32_t IFX_TAPI_Ring_SetConfig(TAPI_CHANNEL *pChannel,
                                    IFX_TAPI_RING_CFG_t const *pRingConfig)
{
   IFX_TAPI_DRV_CTX_t *pDrvCtx = pChannel->pTapiDevice->pDevDrvCtx;
   TAPI_RING_DATA_t *pRingData = &pChannel->TapiRingData;
   IFX_int32_t ret = IFX_ERROR;

   /* begin of protected area */
   IFXOS_MutexLock(pChannel->semTapiChDataLock);

   /* set only when channel ringing timer not running */
   if (pRingData->bRingingMode != IFX_FALSE)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
            ("\n\rDRV_ERROR: can't set ring configuration.\n\r"));
      IFXOS_MutexUnlock(pChannel->semTapiChDataLock);
      return IFX_ERROR;
   }

   /* save settings in TAPI structure */
   pRingData->RingConfig = *pRingConfig;

   if (ptr_chk(pDrvCtx->ALM.Ring_Cfg, "pDrvCtx->ALM.Ring_Cfg"))
      ret = pDrvCtx->ALM.Ring_Cfg (pChannel->pLLChannel, pRingConfig);

   /* end of protected area */
   IFXOS_MutexUnlock(pChannel->semTapiChDataLock);

   return ret;
}


/**
   Gets the ring configuration for the ringing services.

   \param   pChannel    Pointer to TAPI_CHANNEL structure.
   \param   pRingConfig Pointer to struct to return the ring modes.

   \return \ref IFX_SUCCESS
*/
IFX_int32_t IFX_TAPI_Ring_GetConfig(TAPI_CHANNEL *pChannel,
                                    IFX_TAPI_RING_CFG_t *pRingConfig)
{
   /* begin of protected area */
   IFXOS_MutexLock(pChannel->semTapiChDataLock);

   pRingConfig->nMode = pChannel->TapiRingData.RingConfig.nMode;
   pRingConfig->nSubmode = pChannel->TapiRingData.RingConfig.nSubmode;

   /* end of protected area */
   IFXOS_MutexUnlock(pChannel->semTapiChDataLock);

   return IFX_SUCCESS;
}


/**
   Set the number of cadences after which ringing stops automatically.

   \param   pChannel    Pointer to TAPI_CHANNEL structure.
   \param   nMaxRings   Number of ring periods to be played.
                        Value 0 means infinite.

   \return \ref IFX_SUCCESS
*/
IFX_int32_t IFX_TAPI_Ring_SetMaxRings(TAPI_CHANNEL *pChannel,
                                      IFX_uint32_t nMaxRings)
{
   pChannel->TapiRingData.nMaxRings = nMaxRings;
   return IFX_SUCCESS;
}


/**
   Starts the ringing in blocking mode.

   This function starts ringing and then blocks until either the phone went
   off-hook or the number of cadences set with \ref IFX_TAPI_Ring_SetMaxRings
   have been played.

   \param   pChannel    Pointer to TAPI_CHANNEL structure.

   \return
     - IFX_SUCCESS  Stopped because the configures number of rings were reached.
     - 1            Stopped because phone hooked off.
     - IFX_ERROR    Cannot start because phone is off-hook.
*/
IFX_int32_t IFX_TAPI_Ring_DoBlocking(TAPI_CHANNEL *pChannel)
{
   IFX_int32_t ret;

   /* Abort if phone is currently off-hook */
   if (pChannel->TapiOpControlData.bHookState == IFX_TRUE)
   {
      return IFX_ERROR;
   }

   IFX_TAPI_Ring_Start(pChannel);

   /* clear any previous events that occured */
   IFXOS_ClearEvent(pChannel->TapiRingEvent);
   /* wait until wakeup -> maxrings have reached
                        -> hook off              */
   IFXOS_WaitEvent(pChannel->TapiRingEvent);

   /* if the telephone went off-hook we still need to stop ringing */
   if (pChannel->TapiRingData.bRingingMode != IFX_FALSE)
   {
      IFX_TAPI_Ring_Stop(pChannel);
   }

   /* determine whether telephone went off-hook or max-rings where reached */
   if ((pChannel->TapiRingData.nMaxRings != 0) &&
       (pChannel->TapiRingData.nRingsLeft == 0))
   {
      /* maxrings were reached */
      ret = IFX_SUCCESS;
   }
   else
   {
      /* hook off */
      ret = 1;
   }

   return ret;
}

/* @} */
