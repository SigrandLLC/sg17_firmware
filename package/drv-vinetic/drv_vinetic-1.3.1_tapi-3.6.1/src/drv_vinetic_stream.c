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
   Module      : drv_vinetic_stream.c
   Date        : 2002-11-08
   Description :
      This file contains the implementation of the voice streaming.
*******************************************************************************/

/* ============================= */
/* Includes                      */
/* ============================= */
#include "drv_vinetic_api.h"

#if (VIN_CFG_FEATURES & VIN_FEAT_PACKET)

#include "drv_vinetic_int.h"
#include "drv_vinetic_stream.h"
#ifdef TAPI_CID
#include "drv_vinetic_sig_cid.h"
#endif /* TAPI_CID */

/* ============================= */
/* Local Macros & Definitions    */
/* ============================= */


/* timeout for mailbox packet read */
/* used from File and VoIP upstream functions */
#define MBX_RD_TIMEOUT 0xFFFF

#if (VIN_CFG_FEATURES& VIN_FEAT_FAX_T38)
/* Defines for Fax Upstream/Downstream */
/* end of modulation data */
/* Instruction 1 status bit indicating end of demodulation data */
#define FAX_T38_PAYLOAD_END      0x8000
/* Instruction 1 status bit indicating status packet(new format) */
#define FAX_T38_PAYLOAD_STATS    0x4000

#endif /* (VIN_CFG_FEATURES& VIN_FEAT_FAX_T38) */

/* ============================= */
/* Global variable definition    */
/* ============================= */

/* ============================= */
/* Global function declaration   */
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

/*******************************************************************************
Description :
   Write RTP or AAL packets to the VINETIC
Arguments:
   pCh    -  device control structure, must be valid
   buf    -  destination buffer
   count  -  size of data bytes to write
Return Value:
   Actual written bytes
Remarks:
   the mailbox access must be protected against tasks and interrupt
*******************************************************************************/
/*******************************************************************************
Description :
   Write RTP or AAL packets to the VINETIC
Arguments:
   pCh    -  device control structure, must be valid
   buf    -  destination buffer
   count  -  size of data bytes to write
Return Value:
   Actual written bytes
Remarks:
   the mailbox access must be protected against tasks and interrupt
*******************************************************************************/
IFX_int32_t VoIP_DownStream(VINETIC_CHANNEL *pCh, const IFX_uint8_t *buf,
                            IFX_int32_t count)
{
   IFX_uint16_t nRegBoxVlen, *pData;
   VINETIC_DEVICE *pDev = pCh->pParent;
   IFX_uint16_t nVinboxCnt;
   IFX_int32_t err = count;

   /* protect the whole routine */
   VIN_HOST_PROTECT(pDev);
   /* read BOX_VLEN register */
   REG_READ_UNPROT(pDev, V2CPE_BOX_VLEN, &nRegBoxVlen);
   CHECK_HOST_ERR (pDev, {err = IFX_ERROR; goto EXIT_RELEASE;});
   /* calculate number of words, account for odd number of bytes, too */
   nVinboxCnt = (count >> 1) + (count & 0x1);
   /* if not sufficient free space in voice-outbox, we return immidiately */
   if (V2CPE_BOX_VLEN_WLEN_GET(nRegBoxVlen) < (nVinboxCnt + CMD_HEADER_CNT))
   {
      err = 0;
      goto EXIT_RELEASE;
   }

   pData = (IFX_uint16_t *)((IFX_void_t *)buf);
   pData[0] = CMD1_WR | (pCh->nChannel - 1); /* setup command header */

   /* check payload type in case of RTP */
   if ((pDev->nEdspVers[0] & ECMD_VERS_EDSP_PRT) == ECMD_VERS_EDSP_PRT_RTP)
   {
      /* for dynamic event payload type assignment */
#if (VIN_CFG_FEATURES & VIN_FEAT_VOICE)
      /* check payload. pData is in network byte order */
      if ((ntohs(pData[2]) & RTP_PT) == pCh->nEvtPT)
      {
         pData[0] |= CMD1_EVT;
      }
      else
#endif
      {
         pData[0] |= CMD1_VOP;
      }
   }
   else
   {
      /* for AAL no event support currently implemented */
      pData[0] |= CMD1_VOP;
   }
   if (count & 0x01) /* in case of odd byte count, set ODD bit */
      pData[1] = CMD2_ODD;
   pData[1] |= (IFX_uint16_t)nVinboxCnt; /* set packet length in words */

   nVinboxCnt += CMD_HEADER_CNT;

   /* write packet to voice inbox in a protected way against concurent tasks
      and interrupts */
   VIN_UNPROT_VOICE_MBX_WRITE(pDev, pData, nVinboxCnt);
   CHECK_HOST_ERR (pDev, {err = IFX_ERROR; goto EXIT_RELEASE;});

EXIT_RELEASE:
   /* release protection */
   VIN_HOST_RELEASE(pDev);
   return err;
}

/*******************************************************************************
Description :
   Read RTP or AAL packets from channel FIFO
Arguments   :
   pCh   -  device control structure, must be valid
   buf      -  destination buffer
   count   -  maximum data bytes to read
Result      :
   Actual read bytes maybe zero or error code. Error will be returned in case
   of count is smaller then the actual read bytes. If zero is returned no bytes
   are currently in the buffer and the read function can be called again
   later.
   The Fifo access must be protected against interrupt.
*******************************************************************************/
IFX_int32_t VoIP_UpStream (VINETIC_CHANNEL *pCh,  IFX_uint8_t* buf,
                           IFX_int32_t count)
{
   int len = 0;
   PACKET *pPacket = NULL;
   VINETIC_DEVICE *pDev = pCh->pParent;


#ifdef DEBUG
   CHK_CH_MAGIC;
#endif /* DEBUG */
   /* lock interrupts, as fifo can be changed within interrupt routine */
   Vinetic_IrqLockDevice(pDev);
   /* check if there is data in read Fifo */
   if (IFX_TRUE == fifoEmpty(TAPI_UpStreamFifo_Get(pCh->pTapiCh)))
   {
      Vinetic_IrqUnlockDevice (pDev);
      /* non-blocking read call */
      /* if there is no data, it returns immediately with a return value of 0  */
      if (pCh->pTapiCh->nFlags & CF_NONBLOCK)
      {
         return 0;
      }
      /* blocking read call */
      /* waits MBX_RD_TIMEOUT milliseconds for data to arrive */
      else
      {
         /* timeout has expired without arrival of data */
         if (IFXOS_WaitEvent_timeout(pCh->pTapiCh->semReadBlock, MBX_RD_TIMEOUT) == IFX_ERROR)
         {
            return 0;
         }
      }
      /* before accessing the mailbox - lock again. */
      Vinetic_IrqLockDevice (pDev);
   }
   /* read data from the fifo if already available or if
      there is data arrival within timeout in blocking read call*/
   /* now data should be available, otherwise timeout occured */
   IFXOS_ASSERT(IFX_FALSE == fifoEmpty(TAPI_UpStreamFifo_Get(pCh->pTapiCh)));
   pPacket = (PACKET*) fifoGet(TAPI_UpStreamFifo_Get(pCh->pTapiCh), &len);
   IFXOS_ASSERT (pPacket != NULL);
   if (pPacket == NULL)
   {
      Vinetic_IrqUnlockDevice (pDev);
      return 0;
   }

#if 1
   /* get the length in Bytes */
   len = (pPacket->cmd2 & CMD2_LEN);
   /* check if the ODD bit is set */
   if (pPacket->cmd2 & CMD2_ODD)
      len = 2 * len - 1;
   else
      len = (2 * len);
#endif

   if (len > (IFX_uint32_t)count)
   {
      SET_ERROR (VIN_ERR_FUNC_PARM);
      Vinetic_IrqUnlockDevice (pDev);
      return IFX_ERROR;
   }

   /* unmap data */
   IFXOS_CPY_KERN2USR (buf, pPacket->pData, (IFX_uint32_t)len);

   /* Should be protected */
   if (IFX_ERROR == bufferPoolPut((IFX_void_t *) pPacket))
   {
      TRACE(VINETIC, DBG_LEVEL_HIGH,
           ("Put back packet err for ch %d\n\r", (pCh->nChannel - 1)));
   }
   Vinetic_IrqUnlockDevice(pDev);

   return (IFX_int32_t)len;
}

#if (VIN_CFG_FEATURES & VIN_FEAT_FAX_T38)
/*******************************************************************************
Description :
   Write Fax data to VINETIC EDSP Fax Datapump
Arguments:
   pCh    -  channel control structure, must be valid
   buf    -  data buffer
   count  -  size of data bytes to write
Return Value:
   Actual written bytes when success
   error code when error
Remarks:
   -  This function is not blocking and is invoked by the user when the fax
      datapump requires more data.
   - This function just send data out without checkin if the user has the rights
     to invoke the function now. Therefore the user must be aware of what he is
     doing
   - The mailbox access must be protected against tasks and interrupt.
      All shared variables with interrupt routine must also be protected.
   - Odd packets (count & 1 != 0) are not specially handled. The firmware is
     assumed to do the appropriate handling in that case.
*******************************************************************************/
IFX_int32_t Fax_DownStream (VINETIC_CHANNEL *pCh, const IFX_uint8_t *buf,
                            IFX_int32_t count)
{
   VINETIC_DEVICE *pDev = pCh->pParent;
   IFX_uint16_t   *pFaxBuf;
   IFX_uint16_t   voice_box_len = 0, nWordCnt = (count >> 1) + (count & 0x1);
   IFX_int32_t    err = count;
   /* Event handler if error. */
   IFX_TAPI_EVENT_t tapiEvent;
   TAPI_CHANNEL *pChannel = (TAPI_CHANNEL *) pCh->pTapiCh;
   IFX_return_t ret;

#ifdef TAPI_FAX_T38_PACKET_NON_UNIFIED
   /* data already copied from user space with space for 2 command words.
      In case of FAX they have been transmitted */
   pFaxBuf = (IFX_uint16_t *)((IFX_void_t *)&buf[4]);
#else
   /* a headroom of 4 bytes already provided, we reuse it here */
   PACKET *pPacket = (PACKET *)buf;

   pPacket->cmd1 = CMD1_FRD | (pCh->nChannel - 1);
   pPacket->cmd2 = nWordCnt;
   if ((count & 0x1))
      pPacket->cmd2 |= CMD2_ODD;

   pFaxBuf = (IFX_uint16_t *)pPacket;
   nWordCnt += CMD_HEADER_CNT;
#endif

   /* protect against concurrent usage of the mailbox */
   VIN_HOST_PROTECT(pDev);
   /* check voice inbox space */
   REG_READ_UNPROT(pDev, V2CPE_BOX_VLEN, &voice_box_len);
   CHECK_HOST_ERR(pDev, {err = IFX_ERROR; goto EXIT_RELEASE;});

   if (V2CPE_BOX_VLEN_WLEN_GET(voice_box_len) < nWordCnt)
   {
      /* no enough space in the inbox. user should try again */
      err = 0;
      goto EXIT_RELEASE;
   }
   /* protect write access against tasks and interrupts  */
   VIN_UNPROT_VOICE_MBX_WRITE (pDev, pFaxBuf, nWordCnt);
   CHECK_HOST_ERR (pDev, {err = IFX_ERROR; goto EXIT_RELEASE;});

   /* time to clear the flag */
   pCh->pTapiCh->bFaxDataRequest = IFX_FALSE;

EXIT_RELEASE:
   if (err == IFX_ERROR)
   {
      /* set error and issue tapi exception */
     /* Fill event structure. */
      /* FIXME, need protection. */
      memset(&tapiEvent, 0, sizeof(IFX_TAPI_EVENT_t));
      tapiEvent.id = IFX_TAPI_EVENT_T38_ERROR_WRITE;
      IFX_TAPI_Event_Dispatch(pChannel,&tapiEvent);
      if(ret == IFX_ERROR)
      {
         /* \todo if dispatcher error?? */
      }

      TRACE (VINETIC, DBG_LEVEL_HIGH,
            ("Error in Modulation ch%d\n\r", (pCh->nChannel - 1)));
   }

   /* release protection */
   VIN_HOST_RELEASE(pDev);
   return err;
}

/*******************************************************************************
Description :
   Read Fax data from VINETIC EDSP Fax Datapump
Arguments   :
   pCh   -  channel control structure, must be valid
   buf   -  destination buffer
   count -  maximum data bytes to read
Result      :
    Actual read bytes when success
    error code when error
Remarks     :
   - This function is now implemented non blocking and will be invoked every time
     there are new data in Fifo. Error Conditions are :
        - amount of data to read greater than max packet size (256 Words)
        - no fifo available or Fifo read ptr is null
        - size read > size awaited (error case)
   -  The Fifo access must be protected against interrupt.
      All shared variables with interrupt must also be protected.
*******************************************************************************/
IFX_int32_t Fax_UpStream (VINETIC_CHANNEL *pCh, IFX_uint8_t *buf,
                          IFX_int32_t count)
{
   VINETIC_DEVICE *pDev = pCh->pParent;
   PACKET         *pPacket;
   IFX_uint32_t    pos = 0;
   IFX_int32_t     err = IFX_SUCCESS;
   int             len = 0;
   /* Event handler if error. */
   IFX_TAPI_EVENT_t tapiEvent;
   TAPI_CHANNEL *pChannel = (TAPI_CHANNEL *)pCh->pTapiCh;
   IFX_return_t ret;

   /* protect against interrupts because fifo is filled
      within the interrupt routine */
   Vinetic_IrqLockDevice (pDev);
   /* check if Fifo isn't empty. Must be protected from interrupts */
   if (IFX_FALSE == fifoEmpty(TAPI_UpStreamFifo_Get(pCh->pTapiCh)))
   {
      /* read data from fifo ... Interrupts schould be locked */
      pPacket = (PACKET *)fifoGet(TAPI_UpStreamFifo_Get(pCh->pTapiCh), &len);
      if (pPacket != NULL)
      {
         /* get and check the length in Bytes */
         len = (pPacket->cmd2 & CMD2_LEN) * 2;
         if (pPacket->cmd2 & CMD2_ODD)
            len--;
#ifdef TAPI_FAX_T38_PACKET_NON_UNIFIED
         if (len > count - 4)
         {
            TRACE (VINETIC, DBG_LEVEL_HIGH,
                   ("Ch%d: Fax upstream read, insufficient buffer space provided\n\r",
                   pCh->nChannel - 1));
            err = IFX_ERROR;
            goto error;
         }
         /* copy commands and data to user space and save line status if  */
         IFXOS_CPY_KERN2USR (buf, &pPacket->cmd1, 2);
         pos += 2;
         IFXOS_CPY_KERN2USR (&buf[pos], &pPacket->cmd2, 2);
         pos += 2;
#else
         if (len > count)
         {
            TRACE (VINETIC, DBG_LEVEL_HIGH,
                   ("Ch%d: Fax upstream read, insufficient buffer space provided\n\r",
                   pCh->nChannel - 1));
            err = IFX_ERROR;
            goto error;
         }
         if ((pPacket->cmd1 & CMD1_CMD) == CMD1_FRS)
            pPacket->pData[0] |= FAX_T38_PAYLOAD_STATS;
#endif
         IFXOS_CPY_KERN2USR (&buf[pos], pPacket->pData, (IFX_uint32_t)len);
         len += (IFX_int32_t)pos;

         /* return the buffer to the pool */
         if (bufferPoolPut((void *)pPacket) == IFX_ERROR)
         {
            TRACE (VINETIC, DBG_LEVEL_HIGH, ("Put Back error\n\r"));
            SET_ERROR (VIN_ERR_BUFPUT);
            memset(&tapiEvent, 0, sizeof(IFX_TAPI_EVENT_t));
            tapiEvent.id = IFX_TAPI_EVENT_T38_ERROR_READ;
            IFX_TAPI_Event_Dispatch(pChannel, &tapiEvent);
         }

         /* end of demodulation. user should stop the datapump */
         if (((pPacket->cmd1 & CMD1_CMD) == CMD1_FRD) &&
              (ntohs(pPacket->pData[0]) & FAX_T38_PAYLOAD_END))
         {
            /* Fill event structure. */
            memset(&tapiEvent, 0, sizeof(IFX_TAPI_EVENT_t));
            tapiEvent.id = IFX_TAPI_EVENT_T38_NONE;
            ret = IFX_TAPI_Event_Dispatch(pChannel, &tapiEvent);
            if(ret == IFX_ERROR)
            {
               /* \todo if dispatcher error?? */
            }
         }
      }
      else
      {
         TRACE (VINETIC, DBG_LEVEL_HIGH,
               ("Error : No Fifo element, ch%d\n\r", (pCh->nChannel - 1)));
         err = IFX_ERROR;
      }
   }

error:
   /* release interrupt lock */
   Vinetic_IrqUnlockDevice (pDev);
   /* check errors */
   if (err == IFX_ERROR)
   {
      /* set error and issue tapi exception */
      /* Fill event structure. */
      memset(&tapiEvent, 0, sizeof(IFX_TAPI_EVENT_t));
      tapiEvent.id = IFX_TAPI_EVENT_T38_ERROR_READ;
      ret = IFX_TAPI_Event_Dispatch(pChannel,&tapiEvent);
      if(ret == IFX_ERROR)
      {
         /* \todo if dispatcher error?? */
      }

      TRACE (VINETIC, DBG_LEVEL_HIGH,
            ("Error in Demodulation ch%d\n\r", (pCh->nChannel - 1)));
      return IFX_ERROR;
   }

   return len;
}
#endif /* (VIN_CFG_FEATURES & VIN_FEAT_FAX_T38) */
#endif /* (VIN_CFG_FEATURES & VIN_FEAT_PACKET) */
