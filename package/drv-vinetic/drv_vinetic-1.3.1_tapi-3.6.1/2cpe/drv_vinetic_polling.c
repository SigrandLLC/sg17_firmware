#ifdef TAPI_POLL
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
   Module      : drv_vinetic_polling.c

   This file implements a set of functions necessary for packet/event
   polling functionality provided over TAPI high-level.

*/

/** \defgroup VINETIC polling low level TAPI interface
\remarks
   This file implements a set of functions necessary for packet/event
   polling functionality provided over TAPI high-level.
*/
/* @{ */

/* ============================= */
/* Includes                      */
/* ============================= */
#include "ifx_types.h"
#include "drv_tapi_io.h"
#include "drv_vinetic_host.h"
#include "drv_vinetic_api.h"
/* for CID rx data get */
#include "drv_vinetic_sig_cid.h"

/* ============================= */
/* Local Macros & Definitions    */
/* ============================= */

/* ============================= */
/* Local function declaration    */
/* ============================= */

/* ============================= */
/* Global function declarations  */
/* ============================= */

IMPORT IFX_void_t reset_voice_outbox(VINETIC_DEVICE *pDev);
IMPORT IFX_void_t VINETIC_interrupt_routine(VINETIC_DEVICE *pDev);
IMPORT IFX_void_t serve_voice_outbox_data(VINETIC_DEVICE *pDev);

/* ============================= */
/* Local function definitions    */
/* ============================= */

/******************************************************************************/
/**
   Read voice/fax/event packets from the device

   \param pLLDev     - Pointer to low-level device structure
   \param ppPkts     - Array of free buffer pointers

                       In case of Linux the buffers have to be taken from the
                       common buffer pool for reading newly available packets
                       into. It is then an obligation of TAPI HL to put the used
                       buffer back to the buffer pool after copying the data to
                       user-space.

                       In case of VxWorks buffers are already available in
                       ppPkts and here simply these buffers are used for reading
                       the newly available packets.

   \param pPktsNum   - On entry identifies the number of packets to be read,
                       on return it contains the number of packets read
   \param nDevID     - device ID from which the packet has been received,
                       reported to the application
   \return
      IFX_SUCCESS on success
      IFX_ERROR on error
*******************************************************************************/
LOCAL IFX_return_t IFX_TAPI_LL_POLL_rdPkts(IFX_TAPI_LL_DEV_t *pLLDev,
                                           IFX_void_t **ppPkts,
                                           IFX_int32_t *pPktsNum,
                                           IFX_int32_t nDevID)
{
   IFX_int32_t err = IFX_SUCCESS;
   TAPI_DEV *pTapiDev = (TAPI_DEV *) pLLDev;
   VINETIC_DEVICE *pDev = (VINETIC_DEVICE *) pTapiDev->pLLDev;
   IFX_int32_t nPktsUp = 0;
   IFX_void_t **ppPktsUp = IFX_NULL;
   IFX_uint16_t nRegBoxVlen, nVoutboxCnt;
   IFX_uint8_t nCh = 0, nLen = 0;
   VINETIC_CHANNEL *pCh = IFX_NULL;
   /* Packet command header container */
   IFX_uint16_t pPktCmd[CMD_HEADER_CNT];
   PACKET *pPkt = IFX_NULL;


   /* Read voice box data count register (VBOX_CNT) */
   REG_READ_UNPROT(pDev, V2CPE_BOX_VLEN, &nRegBoxVlen);
   CHECK_HOST_ERR(pDev, return IFX_ERROR);

   /* Get voice outbox data count in words, exit if none */
   if ((nVoutboxCnt = V2CPE_BOX_VLEN_RLEN_GET(nRegBoxVlen)) == 0)
   {
      /* No packet available. */
      return IFX_ERROR;
   }

   ppPktsUp = ppPkts;

   do
   {
         /* Read packet command header, consisting of two words */
         VIN_LL_UNPROT_VOICE_MBX_READ (pDev, pPktCmd, CMD_HEADER_CNT);
         CHECK_HOST_ERR(pDev, return IFX_ERROR);

         nLen = pPktCmd[1] & CMD2_LEN; /* Get packet length */
         nCh  = pPktCmd[0] & CMD1_CH;  /* Get packet channel number */

         /* Sanity check channel number */
         if (nCh >= VINETIC_2CPE_MAX_EDSP)
         {
            TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
                  ("VIN%d, IRQ: corrupt voice outbox, outbox reseted"
                   " (Cmd1 : 0x%04X Cmd2: 0c%04X).",
                   "(File: %s, line: %d)\n",
                   pDev->nDevNr, pPktCmd[0], pPktCmd[1], __FILE__, __LINE__));

            reset_voice_outbox(pDev);
            return IFX_ERROR;
         }
         /* If channel number is valid, get the channel context. */
         pCh = &pDev->pChannel[nCh];

#ifdef TAPI_CID
         /* Get the next available packet buffer */
         if ((pPktCmd[0] & CMD1_CIDRX) == CMD1_CIDRX)
         {
            pPkt = (PACKET *)&pCh->cidRxPacket;
         }
         else
#endif /* TAPI_CID */
         {
            pPkt = (PACKET *) bufferPoolGet(TAPI_VoiceBufferPoolHandle_Get());

            /* No packet buffers available */
            if (pPkt == IFX_NULL)
            {
               PACKET Pkt;

               /* Dummy read of the rest of the packet, just to free voice
                  outbox. */
               VIN_LL_UNPROT_VOICE_MBX_READ (pDev, &Pkt.pData[0], nLen);
               CHECK_HOST_ERR(pDev, return);

               return IFX_ERROR;
            }
         }

         /* Copy packet command header into packet buffer */
         pPkt->cmd1 = pPktCmd[0];
         pPkt->cmd2 = pPktCmd[1];

         if (nLen)
         {
            /* Read the rest of the packet into packet buffer */
            VIN_LL_UNPROT_VOICE_MBX_READ (pDev, pPkt->pData, nLen);
            CHECK_HOST_ERR(pDev, return IFX_ERROR);
         }


#ifdef QOS_SUPPORT
         /* nLen is size of words:
            even number -> num of bytes = 2 * word
            odd number -> num of bytes = 2 * word - 1 */
         fifoPut(TAPI_UpStreamFifo_Get(pCh->pTapiCh), pPkt, 2 * nLen);
#endif /* QOS_SUPPORT */

#ifdef TAPI_CID
         /* Handle CID packet */
         if ((pPktCmd[0] & CMD1_CIDRX) == CMD1_CIDRX)
         {
            VINETIC_SIG_CID_RX_Data_Collect (pCh->pTapiCh);
         }
         else
#endif /* TAPI_CID */
#ifndef QOS_SUPPORT
         {
            IFX_uint16_t nPktType = 0;
            IFX_TAPI_POLL_PKT_t *pPktTapi = IFX_NULL;


            pPktTapi = (IFX_TAPI_POLL_PKT_t *) pPkt;

            pPktTapi->ch = pPktCmd[0] & CMD1_CH;
            pPktTapi->dev = nDevID;
            pPktTapi->len = (pPktCmd[1] & CMD2_LEN) * 2;
            if (pPktCmd[1] & CMD2_ODD)
               pPktTapi->len--;
            nPktType = pPktCmd[0] & CMD1_CMD;
            switch (nPktType)
            {
               case CMD1_VOP:
               case CMD1_EVT:
                  pPktTapi->type = IFX_TAPI_POLL_PKT_TYPE_VOICE;
                  break;
               case CMD1_FRD:
                  pPktTapi->type = IFX_TAPI_POLL_PKT_TYPE_FRD;
                  break;
               case CMD1_FRS:
                  pPktTapi->type = IFX_TAPI_POLL_PKT_TYPE_FRS;
                  break;
               default:
                  pPktTapi->type = IFX_TAPI_POLL_PKT_TYPE_VOICE;
            }
            /* Store this packet into array. */
            ppPktsUp[nPktsUp] = pPktTapi;

            nPktsUp++;

            /* Increase also packet counter which is input argument */
            (*pPktsNum)++;
         }
#endif

      /* Read voice box data count register (VBOX_CNT) */
      REG_READ_UNPROT(pDev, V2CPE_BOX_VLEN, &nRegBoxVlen);
      CHECK_HOST_ERR(pDev, return IFX_ERROR);

      /* Get voice outbox data count in words, exit if none */
      nVoutboxCnt = V2CPE_BOX_VLEN_RLEN_GET(nRegBoxVlen);
   } while (nVoutboxCnt);


   /* Reaching this point, we should have received packets and now need to
      wakeup further processing */
#ifdef QOS_SUPPORT
   /* Schedule egress packet forwarding for packet redirection
      at a safe time */
   if (pCh->pTapiCh->QosCtrl.egressFlag == QOS_EGRESS_REDIR)
   {
      Qos_PktEgressSched((IFX_uint32_t) pCh->pTapiCh);
   }
   else
#endif /* QOS_SUPPORT */
   {
         if (!(pCh->pTapiCh->nFlags & CF_NONBLOCK))
         {
            /* Data available, wake up waiting upstream function */
            IFXOS_WakeUpEvent (pCh->pTapiCh->semReadBlock);
         }
         /* If a non-blocking read should be performed just wake up once */
         if (pCh->pTapiCh->nFlags & CF_NEED_WAKEUP)
         {
            pCh->pTapiCh->nFlags |= CF_WAKEUPSRC_STREAM;
            /* Don't wake up any more */
            pCh->pTapiCh->nFlags &= ~CF_NEED_WAKEUP;
            IFXOS_WakeUp (pCh->pTapiCh->wqRead, IFXOS_READQ);
         }
   }

   return err;
} /* IFX_TAPI_LL_POLL_rdPkts() */


/******************************************************************************/
/**
   Write the provided voice/fax/event packets

   \param pLLDev     - Pointer to low-level device structure (TAPI_DEV ?)
   \param pPktsNum   - On entry identifies the number of packets to be written,
                       on return it contains the number of packets successfully
                       written. On success (IFX_SUCCESS) all packets have been
                       successfully written.
   \return
      IFX_SUCCESS on success
      IFX_ERROR on error
   \remarks
   HL TAPI sorts all packets made available by the application into the device
   specific FIFOs. This routine will attemp to write as many packets as specified
   by '*pPktsNum'. If all packet cannot be written to the device, the remaining
   packets stay in the device FIFOs until the next itteration of this routine.
*******************************************************************************/
LOCAL IFX_return_t IFX_TAPI_LL_POLL_wrPkts (IFX_TAPI_LL_DEV_t *pLLDev,
                                            IFX_int32_t *pPktsNum)
{
   IFX_return_t err = IFX_SUCCESS;
   TAPI_DEV *pTapiDev = (TAPI_DEV *) pLLDev;
   VINETIC_DEVICE *pDev = (VINETIC_DEVICE *) pTapiDev->pLLDev;
   IFX_TAPI_POLL_PKT_t *pPkt = IFX_NULL;
   PACKET *pPktVin = IFX_NULL;
   IFX_uint16_t nReg, nVboxLen, nPktLen, nOddBit;
   IFX_int32_t nPktsNum, nPktsDone;


   /* Lock mailbox access */
   IFXOS_MutexLock(pDev->mbxAcc);

   /* Get the size of voice inbox free space */
   REG_READ_UNPROT(pDev, V2CPE_BOX_VLEN, &nReg);
   CHECK_HOST_ERR (pDev, {err = IFX_ERROR; goto error;});
   nVboxLen = V2CPE_BOX_VLEN_WLEN_GET(nReg);

   nPktsDone = 0;
   nPktsNum = *pPktsNum;
   while (nPktsNum > 0)
   {
      /* Get the first packet from FIFO */
      pPkt = fifoGet(TAPI_DownStreamFifo_Get(pTapiDev), IFX_NULL);
      if (pPkt == IFX_NULL)
      {
         TRACE(VINETIC, DBG_LEVEL_NORMAL,
             ("Fifo with voice packets is empty\n"));
         /* No more packets available in the FIFO */
         break;
      }
      /* Sanity check channel number */
      if (pPkt->ch >= VINETIC_2CPE_MAX_EDSP)
      {
         TRACE(VINETIC, DBG_LEVEL_HIGH,
             ("Read packet with ch num %d from fifo. "
              "(File: %s, line: %d)\n",
              pPkt->ch, __FILE__, __LINE__));
         err = IFX_ERROR;
         break;
      }

      /* Convert packet length from bytes to 16 bit words,
         rounding-off to the nearest word */
      nPktLen = (pPkt->len >> 1) + (pPkt->len & 1);
      nOddBit = (pPkt->len & 1) ? CMD2_ODD : 0;

      if (nVboxLen < (nPktLen + CMD_HEADER_CNT))
      {
         /* Insufficient space in the packet inbox */
         TRACE(VINETIC, DBG_LEVEL_HIGH,
             ("Insufficient space in packet inbox. "
              "(File: %s, line: %d)\n",
              __FILE__, __LINE__));
         err = IFX_ERROR;
         break;
      }

      pPktVin = (PACKET *) pPkt;

      /* Check for fax relay packets */
      if (pPkt->type == IFX_TAPI_POLL_PKT_TYPE_FRD)
      {
         pPktVin->cmd1 = CMD1_WR | CMD1_FRD | pPkt->ch;
      }
      /* Check payload type in case of RTP */
      else if ((pDev->nEdspVers[0] & ECMD_VERS_EDSP_PRT)
               == ECMD_VERS_EDSP_PRT_RTP)
      {
         /* For dynamic event payload type assignment */
#if (VIN_CFG_FEATURES & VIN_FEAT_VOICE)
         VINETIC_CHANNEL *pCh = &pDev->pChannel[pPkt->ch];

         /* Check payload. pData is in network byte order */
         if ((ntohs(pPkt->data[0]) & RTP_PT) == pCh->nEvtPT)
         {
            pPktVin->cmd1 = CMD1_WR | CMD1_EVT | pPkt->ch;
         }
         else
#endif
         {
            pPktVin->cmd1 = CMD1_WR | CMD1_VOP | pPkt->ch;
         }
      }
      else
      {
         /* For AAL no event support currently implemented */
         pPktVin->cmd1 = CMD1_WR | CMD1_VOP | pPkt->ch;
      }
      pPktVin->cmd2 = nPktLen | nOddBit;

      nPktLen += CMD_HEADER_CNT;

      /* Write packet to voice inbox */
      VIN_UNPROT_VOICE_MBX_WRITE(pDev, pPktVin, nPktLen);

      CHECK_HOST_ERR (pDev, {err = IFX_ERROR; goto error;});

      bufferPoolPut((IFX_void_t *) pPkt);

      nVboxLen -= nPktLen;
      nPktsNum--;
      nPktsDone++;
   }
   *pPktsNum = nPktsDone;
error:
   /* Release mailbox access lock */
   IFXOS_MutexUnlock(pDev->mbxAcc);
   return err;
} /* IFX_TAPI_LL_POLL_wrPkts() */


/******************************************************************************/
/**
   Updates the low-level TAPI device status by reading the hardware status
   registers and taking the appropriate actions upon status change.
   Typically this function executes the device's ISR.

   \param pLLDev     - Pointer to low-level device structure

   \return
      IFX_SUCCESS on success
      IFX_ERROR on error
*******************************************************************************/
LOCAL IFX_return_t IFX_TAPI_LL_POLL_pollEvents (IFX_TAPI_LL_DEV_t *pLLDev)
{
   TAPI_DEV *pTapiDev = (TAPI_DEV *) pLLDev;
   VINETIC_DEVICE *pDev = IFX_NULL;


   pDev = (VINETIC_DEVICE *)pTapiDev->pLLDev;
   /* Protect possible mailbox access and shared data against concurent tasks */
   IFXOS_MutexLock (pDev->mbxAcc);

   /* Process events on this device */
   VINETIC_interrupt_routine(pDev);

   /* Release mailbox access and concurent task protection */
   IFXOS_MutexUnlock (pDev->mbxAcc);

   if (pDev->err == VIN_ERR_HOSTREG_ACCESS)
   {
      return IFX_ERROR;
   }
   else
   {
      return IFX_SUCCESS;
   }
}


/******************************************************************************/
/**
   Used to control the packet-generation related interrupts. In case a
   device is registerred for packet polling it is necessary to disable
   the related interrupts so as to prohibit any unwanted overhead of
   switching to interrupt context.

   \param pLLDev     - Pointer to low-level device structure
   \param bEnable    - IFX_TRUE to enable, IFX_FALSE to disable the
                       related interrupts
   \return
      IFX_SUCCESS on success
      IFX_ERROR on error
*******************************************************************************/
LOCAL IFX_return_t IFX_TAPI_LL_POLL_pktsIRQCtrl (IFX_TAPI_LL_DEV_t *pLLDev,
                                                 IFX_boolean_t bEnable)
{
   TAPI_DEV *pTapiDev = (TAPI_DEV *) pLLDev;
   VINETIC_DEVICE *pDev = (VINETIC_DEVICE *) pTapiDev->pLLDev;


   switch (bEnable)
   {
      case IFX_FALSE:
         /* Set flag that we are in polling mode. */
         pDev->IrqPollMode |= VIN_VOICE_POLL;
         /* TODO disable packets related interrupts, only */
         Vinetic_IrqLockDevice((pDev));
         break;

      case IFX_TRUE:
         /* Clear flag that we are in polling mode. */
         pDev->IrqPollMode &= ~VIN_VOICE_POLL;
         /* TODO enable packets related interrupts, only */
         Vinetic_IrqUnlockDevice((pDev));
         break;
   }
   return IFX_TRUE;
}


/******************************************************************************/
/**
   Used to control the TAPI event-generation related interrupts. In case a
   device is registerred for events polling it is necessary to disable
   the related interrupts so as to prohibit any unwanted overhead of
   switching to interrupt context.

   \param pLLDev     - Pointer to low-level device structure
   \param bEnable    - IFX_TRUE to enable, IFX_FALSE to disable the
                       related interrupts
   \return
      IFX_SUCCESS on success
      IFX_ERROR on error
*******************************************************************************/
LOCAL IFX_return_t IFX_TAPI_LL_POLL_evtsIRQCtrl (IFX_TAPI_LL_DEV_t *pLLDev,
                                                 IFX_boolean_t bEnable)
{
   TAPI_DEV *pTapiDev = (TAPI_DEV *)pLLDev;
   VINETIC_DEVICE *pDev = (VINETIC_DEVICE *)pTapiDev->pLLDev;


   switch (bEnable)
   {
      case IFX_FALSE:
         /* Set flag that we are in polling mode */
         pDev->IrqPollMode |= VIN_EVENT_POLL;
         /* TODO disable events related interrupts, only */
         Vinetic_IrqLockDevice((pDev));
         break;

      case IFX_TRUE:
         /* Clear flag that we are in polling mode */
         pDev->IrqPollMode &= ~VIN_EVENT_POLL;
         /* TODO enable events related interrupts, only */
         Vinetic_IrqUnlockDevice((pDev));
         break;
   }
   return IFX_TRUE;
}


/******************************************************************************/
/**
   Function called by the init_module of the LL device that fills up the
   POLLing interface LL function pointers. These function pointers are then
   are passed to HL TAPI during device registration.

   \param pPoll      - pointer to POLL interface data structure

   \return
      none
*******************************************************************************/
IFX_void_t VINETIC_POLL_Func_Register (IFX_TAPI_DRV_CTX_POLL_t *pPoll)
{
   /* Register LL function pointers that are exported to TAPI HL */
   pPoll->rdPkts        = IFX_TAPI_LL_POLL_rdPkts;
   pPoll->wrPkts        = IFX_TAPI_LL_POLL_wrPkts;
   pPoll->pollEvents    = IFX_TAPI_LL_POLL_pollEvents;
   pPoll->pktsIRQCtrl   = IFX_TAPI_LL_POLL_pktsIRQCtrl;
   pPoll->evtsIRQCtrl   = IFX_TAPI_LL_POLL_evtsIRQCtrl;
}

/* }@ */
#endif /* TAPI_POLL */
