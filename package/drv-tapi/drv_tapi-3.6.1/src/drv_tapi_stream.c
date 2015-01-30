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
   Module      : drv_tapi_stream.c
   Description : Buffer, fifo functions for the TAPI Driver
******************************************************************************/

/** \file drv_tapi_stream.c
    Data stream fifos and buffers for TAPI.
    This module provides management of the fifos and buffers for data transport
    in TAPI. */

/* ============================= */
/* Includes                      */
/* ============================= */
#include "drv_api.h"

#ifdef TAPI_PACKET

#include "drv_tapi.h"
#include "drv_tapi_api.h"
#include "drv_tapi_stream.h"

/* ============================= */
/* Local Macros & Definitions    */
/* ============================= */

/** Memory of free buffers for voice packets. */
IFX_LOCAL BUFFERPOOL* pVoicePktBuffer = IFX_NULL;

/** Sizeof on VOICE PACKET. */
enum { PACKET_SIZE = 512 };

/** Sizeof of UpStream FIFO. */
enum { VOICE_UP_FIFO_SIZE = 250 };

/** Sizeof of DownStream FIFO. */
enum { VOICE_DOWN_FIFO_SIZE = 16 };

/** Startup size of elements in bufferpool. */
enum { PKT_COUNT = 100 };

/** When bufferpool is full, increase it by this size. */
enum { PKT_COUNT_INC = 10 };

/* ============================= */
/* Global variable definition    */
/* ============================= */

/* ============================= */
/* Local function declaration    */
/* ============================= */

/* ============================= */
/* Local variable definition     */
/* ============================= */

/* ============================= */
/* Local function definition     */
/* ============================= */

/* ============================= */
/* Global function definition    */
/* ============================= */



/**
   Reset FIFO, free elements in in fifo and clear it.

   \param pFifo - Pointer to FIFO. Must not be NULL

   \return
         Returns IFX_ERROR in case of an error, otherwise returns IFX_SUCCESS.
*/
IFX_return_t TAPI_ClearFifo(FIFO_ID* pFifo)
{
   IFX_void_t* p_elem = IFX_NULL;
   IFX_int32_t fifo_cnt = 0;
   int len = 0;

   fifo_cnt = fifoSize(pFifo);
   if (fifo_cnt < 0)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
           ("Wrong fifo size. (File: %s, line: %d)\n",
            __FILE__, __LINE__));
      return IFX_ERROR;
   }

   while ((fifoEmpty(pFifo)) != IFX_TRUE && (0 < fifo_cnt))
   {
      p_elem = (IFX_void_t *) fifoGet(pFifo, &len /* not used */);

      if (IFX_NULL != p_elem)
      {
         bufferPoolPut(p_elem);
      }
      else
      {
         /* Strange we have element in FIFO which is NULL? */
         TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("Element NULL retrieved from fifo?\n"));
      }
      fifo_cnt--;
   }

   /* Now reset fifo */
   fifoReset(pFifo);

   return IFX_SUCCESS;
}


/**
   Initialize UpStream FIFO.

   \param pTapiCh - Pointer to TAPI channel. Must not be NULL

   \return
         Returns IFX_ERROR in case of an error, otherwise returns IFX_SUCCESS.
*/
IFX_return_t TAPI_InitUpStreamFifo(TAPI_CHANNEL* pTapiCh)
{
   /* Initialize packet voice fifo. */
   if (IFX_NULL != pTapiCh->pUpStreamFifo)
   {
      /** \todo Handling if fifo already initialized. */
      /* Reset fifo */
      fifoReset(pTapiCh->pUpStreamFifo);
      return IFX_SUCCESS;
   }

   pTapiCh->pUpStreamFifo = fifoInit(VOICE_UP_FIFO_SIZE /* elem count */);
   if (IFX_NULL == pTapiCh->pUpStreamFifo)
   {
      /* ERROR: Packet FIFO initialization failed, check fifiInit
                description. */
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("ERR: Initializing "
            "pUpStreamFifo. (File: %s, line: %d)\n",
            __FILE__, __LINE__));
      return IFX_ERROR;
   }

   return IFX_SUCCESS;
}


/**
   Reset UpStream FIFO.

   \param pTapiCh - Pointer to TAPI channel. Must not be NULL

   \return
         Returns IFX_ERROR in case of an error, otherwise returns IFX_SUCCESS.
*/
IFX_return_t TAPI_ResetUpStreamFifo(TAPI_CHANNEL* pTapiCh)
{
   TAPI_DEV *pTapiDev = pTapiCh->pTapiDevice;
   IFX_void_t *pPkt; int len;

   if (pTapiCh->pUpStreamFifo == IFX_NULL)
      return IFX_ERROR;

   if (ptr_chk(pTapiDev->pDevDrvCtx->IRQ.LockDevice,
              "pTapiDev->pDevDrvCtx->IRQ.LockDevice"))
   {
      pTapiDev->pDevDrvCtx->IRQ.LockDevice(pTapiDev->pLLDev);
   }
   else
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
          ("TAPI IRQ.LockDevice not set in LL driver\n\r"));
   }

   while ( (pPkt = fifoGet(pTapiCh->pUpStreamFifo, &len)) != IFX_NULL )
      bufferPoolPut(pPkt);
   fifoReset(pTapiCh->pUpStreamFifo);

   if (ptr_chk(pTapiDev->pDevDrvCtx->IRQ.UnlockDevice,
              "pTapiDev->pDevDrvCtx->IRQ.UnlockDevice"))
   {
      pTapiDev->pDevDrvCtx->IRQ.UnlockDevice(pTapiDev->pLLDev);
   }
   else
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
          ("TAPI IRQ.UnlockDevice not set in LL driver\n\r"));
   }

   return IFX_SUCCESS;
}


/**
   Retrieve handle to UpStream FIFO.

   \param pTapiCh - Pointer to TAPI channel. Must not be NULL

   \return
         Returns IFX_NULL in case of an error, otherwise returns fifo handle.
*/
FIFO_ID* TAPI_UpStreamFifo_Get(TAPI_CHANNEL* pTapiCh)
{
   return pTapiCh->pUpStreamFifo;
}


/**
   Initialize DownStream FIFO.

   \param pTapiDev - Pointer to TAPI device. Must not be NULL

   \return
         Returns IFX_ERROR in case of an error, otherwise returns IFX_SUCCESS.
*/
IFX_return_t TAPI_InitDownStreamFifo(TAPI_DEV* pTapiDev)
{
   /* Initialize packet voice fifo. */
   if (IFX_NULL != pTapiDev->pDownStreamFifo)
   {
      /** \todo Handling if fifo already initialized. */
      /* Reset fifo */
      fifoReset(pTapiDev->pDownStreamFifo);
      return IFX_SUCCESS;
   }

   pTapiDev->pDownStreamFifo = fifoInit(VOICE_DOWN_FIFO_SIZE /* elem count */);
   if (IFX_NULL == pTapiDev->pDownStreamFifo)
   {
      /* ERROR: Packet FIFO initialization failed, check fifiInit
                description. */
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("ERR: Initializing "
            "pDownStreamFifo. (File: %s, line: %d)\n",
            __FILE__, __LINE__));
      return IFX_ERROR;
   }

   return IFX_SUCCESS;
}


/**
   Reset DownStream FIFO.

   \param pTapiDev - Pointer to TAPI device. Must not be NULL

   \return
         Returns IFX_ERROR in case of an error, otherwise returns IFX_SUCCESS.
*/
IFX_return_t TAPI_ResetDownStreamFifo(TAPI_DEV* pTapiDev)
{

   if (IFX_NULL != pTapiDev->pDownStreamFifo)
   {
      return TAPI_ClearFifo(pTapiDev->pDownStreamFifo);
   }

   return IFX_ERROR;
}


/**
   Retrieve handle to UpStream FIFO.

   \param pTapiDev - Pointer to TAPI device

   \return
         Returns IFX_NULL in case of an error, otherwise returns fifo handle.
*/
FIFO_ID* TAPI_DownStreamFifo_Get(TAPI_DEV* pTapiDev)
{
   TRACE(TAPI_DRV, DBG_LEVEL_LOW, ("Return downstream fifo handle.\n"));

   if (IFX_NULL == pTapiDev)
   {
      /* Wrong input arguments */
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
           ("Invalid input argument(s). (File: %s, line: %d)\n",
            __FILE__, __LINE__));
      return IFX_NULL;
   }

   return pTapiDev->pDownStreamFifo;
}


/**
   Prepare bufferpool for voice packets.

   \param none

   \return
      Returns IFX_ERROR in case of an error, otherwise returns IFX_SUCCESS.
*/
IFX_return_t TAPI_PrepareVoiceBufferPool(IFX_void_t)
{
   TRACE(TAPI_DRV, DBG_LEVEL_LOW, ("TAPI_PrepareBufferPool()\n\r"));

   if (IFX_NULL != pVoicePktBuffer)
   {
      /** \todo Handling if bufferpool already initialized. */
      TRACE(TAPI_DRV, DBG_LEVEL_LOW, ("WARN: pVoicePktBuffer already "
            "initialized.\n\r"));

      return IFX_SUCCESS;
#if 0
      /* Remove bufferpool */
      bufferPoolFree(pVoicePktBuffer);
      pVoicePktBuffer = IFX_NULL;
#endif
   }

   pVoicePktBuffer = bufferPoolInit(PACKET_SIZE, PKT_COUNT, PKT_COUNT_INC);
   if (IFX_NULL == pVoicePktBuffer)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("Failed to init bufferpool handle "
            "pVoicePktBuffer.\n\r"));
      return IFX_ERROR;
   }
   else
   {
      TRACE(TAPI_DRV, DBG_LEVEL_LOW, ("pVoicePktBuffer initialized"
            "%d buffers free.\n\r", bufferPoolAvail(pVoicePktBuffer)));
      return IFX_SUCCESS;
   }
}


/**
   Retrieve buffer for voice packet.

   \param

   \return
      Returns IFX_NULL in case of an error, otherwise returns packet buffer.
*/
/*IFX_void_t* TAPI_VoiceBufferPoolGet(IFX_int32_t nSize,
                                      IFX_int32_t nPriority)*/
IFX_void_t* TAPI_VoiceBufferPoolGet(IFX_void_t)
{
   IFX_void_t* buf = IFX_NULL;


   buf = (IFX_void_t *) bufferPoolGet(pVoicePktBuffer);

   return buf;
}


/**
   Retrieve handle to bufferpool.

   \param  none
*/
IFX_void_t* TAPI_VoiceBufferPoolHandle_Get(IFX_void_t)
{
   return pVoicePktBuffer;
}


/**
   Clear and free bufferpool and fifos.

   \param  pTapiDev     Pointer to TAPI device structure
   \param  nChCnt       unused

   \return
      Returns IFX_ERROR in case of an error, otherwise returns IFX_SUCCESS.
*/
IFX_return_t TAPI_Free_FifoBufferPool(TAPI_DEV* pTapiDev, IFX_int32_t nChCnt)
{
   IFX_uint8_t i = 0;
   IFX_return_t ret = IFX_SUCCESS;

   TRACE(TAPI_DRV, DBG_LEVEL_LOW, ("TAPI_Free_FifoBufferPool()\n\r"));

   if (pTapiDev == IFX_NULL)
   {
      /* Wrong input arguments */
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
           ("Invalid input argument(s). (File: %s, line: %d)\n",
            __FILE__, __LINE__));
      return IFX_ERROR;
   }

   for (i = 0; i < pTapiDev->nMaxChannel; i++)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_LOW, ("Free voice fifo for ch %d.\n\r",
            pTapiDev->pTapiChanelArray[i].nChannel));

      if (pTapiDev->pTapiChanelArray[i].pUpStreamFifo != IFX_NULL)
      {
         ret = TAPI_ClearFifo(pTapiDev->pTapiChanelArray[i].pUpStreamFifo);
         if (ret == IFX_SUCCESS)
         {
            ret = fifoFree (pTapiDev->pTapiChanelArray[i].pUpStreamFifo);
            pTapiDev->pTapiChanelArray[i].pUpStreamFifo = IFX_NULL;
         }
      }
   }

   if (pTapiDev->pDownStreamFifo != IFX_NULL)
   {
      ret = fifoReset(pTapiDev->pDownStreamFifo);
      if (ret == IFX_SUCCESS)
      {
         ret = fifoFree(pTapiDev->pDownStreamFifo);
      }
   }

   if (pVoicePktBuffer != IFX_NULL)
   {
      ret = bufferPoolFree(pVoicePktBuffer);
      pVoicePktBuffer = IFX_NULL;
   }

   return ret;
}




#endif /* TAPI_PACKET */
