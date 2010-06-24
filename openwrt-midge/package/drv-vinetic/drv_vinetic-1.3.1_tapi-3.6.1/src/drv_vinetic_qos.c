#ifdef QOS_SUPPORT
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

 ****************************************************************************
   Module      : drv_vinetic_qos.c
   Date        : 2004-09-18
*******************************************************************************/

#ifndef LINUX
#error This feature is actually only available under Linux.
#endif

/* ============================= */
/* Includes                      */
/* ============================= */
#include "drv_vinetic_api.h"
#include "drv_vinetic_qos.h"

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

/* ============================= */
/* Local function definition     */
/* ============================= */


/**
   callback function for ingress packet to register at Qos_Init.

   \param channel - channel number
   \param data    - ptr to data buffer
   \param len     - buffer length in bytes

   \return NO_ERROR , CALLBACK_ERR.

   \remark
    This function takes a channel device ptr from a global array and
    checks if the pointer is valid, meaning that Qos_Init was done before.
    This function doesn't takes care of swapping the buffer accordingly before
    writing, asumming the data comes in the writable order. For little endian
    machines, the firmware swaps the data accordingly after a specific fw
    configuration is done (Endianness Control)

    As this function is invoked from the switch interrupt routine, it schould
    not block. Nevertheless, vinetic access should be protected against
    interrupts

   \see Qos_Init()

   \todo CALLBACK_ERR is located in ifx_udp_redirect.h 
*/
IFX_return_t Qos_LL_PktIngress(IFX_TAPI_LL_CH_t* pLLCh,
                               IFX_void_t* pData,
                               IFX_uint32_t nLen)
{
   VINETIC_CHANNEL* pCh = (VINETIC_CHANNEL *) pLLCh;
   VINETIC_DEVICE* pDev = IFX_NULL;
   IFX_uint16_t data[MAX_PACKET_WORD] = {0};
   IFX_int32_t wrcnt = 0;
   IFX_uint16_t nRegBoxVlen = 0;
   IFX_uint16_t nVinboxCnt = 0;
   IFXOS_INTSTAT     flags;
   IFX_return_t ret = IFX_SUCCESS;


   TRACE(VINETIC, DBG_LEVEL_LOW, ("Qos_LL_PktIngress\n"));

   if ((IFX_NULL == pCh) || (IFX_NULL == pData) || (0 >= nLen))
   {
      TRACE(VINETIC, DBG_LEVEL_HIGH,
           ("Invalid input argument(s). (File: %s, line: %d)\n",
            __FILE__, __LINE__));

      return IFX_ERROR;
   }
   
   pDev = pCh->pParent;
   
   if (pDev->IrqPollMode & VIN_EVENT_POLL)
   {
      /* POLLING */
      /* Don't write packet into mailbox, but store it into DownStreamFifo. */
      /*fifoPut();*/
   }
   else
   {
      /* Protect host mailbox access. */
      /*VIN_HOST_PROTECT(pDev);*/

      /* In global lock write to mailbox. */
      VIN_DISABLE_IRQGLOBAL(flags);

#ifndef VIN_2CPE
      /* Check free packet in-box size */
      ret = ScRead(pDev, SC_RFIBXMS, &nFIBXMS, 1);
      if (ret == IFX_ERROR)
      {
         TRACE(VINETIC, DBG_LEVEL_HIGH, ("No free packet in voice in-box.\n"));
      
         /* Unprotect host mailbox access. */
         /*VIN_HOST_RELEASE(pDev);*/
         VIN_ENABLE_IRQGLOBAL(flags);

         return IFX_ERROR;
      }

      /* Not enough space available in the mailbox : write 0 bytes */
      if ((nFIBXMS & FIBXMS_PBOX) < ((len >> 1) + (len % 2)))
      {
         TRACE(VINETIC, DBG_LEVEL_HIGH, ("Not enough space in voice out-box.\n"));
         SET_ERROR(VIN_ERR_NO_FIBXMS);
      
         /* Unprotect host mailbox access. */
         /*VIN_HOST_RELEASE(pDev);*/
         VIN_ENABLE_IRQGLOBAL(flags);

         return IFX_ERROR;
      }
#else
      /* Read BOX_VLEN register. */
      REG_READ_UNPROT(pDev, V2CPE_BOX_VLEN, &nRegBoxVlen);
      CHECK_HOST_ERR(pDev, {ret = IFX_ERROR;});
      /* Calculate number of words, account for odd number of bytes, too. */
      nVinboxCnt = (nLen >> 1) + (nLen & 0x1);
      /* If not sufficient free space in voice-outbox, we return immidiately. */
      if (V2CPE_BOX_VLEN_WLEN_GET(nRegBoxVlen) < (nVinboxCnt + CMD_HEADER_CNT))
      {
         TRACE(VINETIC, DBG_LEVEL_HIGH, ("Voice out-box is FULL.\n"));
      
         /* Unprotect host mailbox access. */
         /*VIN_HOST_RELEASE(pDev);*/
         VIN_ENABLE_IRQGLOBAL(flags);

         return IFX_ERROR;
      }
#endif /* VIN_2CPE */

      /* Setup command header. */
      data[0] = CMD1_WR | (pCh->nChannel - 1);
#if (VIN_CFG_FEATURES & VIN_FEAT_VOICE)
      /* For dynamic event payload type assignment */
      /* check payload. pData is aligned to network order, big endian. */
      if ((ntohs(data[2]) & RTP_PT) == pCh->nEvtPT)
      {
         data[0] |= CMD1_EVT;
      }
      else
#endif /* (VIN_CFG_FEATURES & VIN_FEAT_VOICE) */
      {
         data[0] |= CMD1_VOP;
      }
      /* set length to write */
      wrcnt = nLen;
      /* set ODD bit if odd byte count */
      if (nLen & 0x01)
      {
         data[1] = CMD2_ODD;
         wrcnt += 1;
      }
      /* convert length into word number */
      wrcnt = wrcnt >> 1;
      /* set count in words */
      data[1] |= (IFX_uint16_t) wrcnt;
      /* fill rest of data */
      if ((MAX_PACKET_WORD - 2) < nLen)
      {
         /** \todo What happens if some data is cut out? */
         TRACE(VINETIC, DBG_LEVEL_HIGH, ("Data to long %d, will be cut to %d\n",
               (int) nLen, MAX_PACKET_WORD - 2));
      
         nLen = MAX_PACKET_WORD - 2;
      }
   
      memcpy(&data[2], (IFX_uint16_t *) pData, nLen); 

#ifndef VIN_2CPE
      ret = pDev->hostDev.write(pDev, data, wrcnt + 2);
#else /* VIN_2CPE */
      VIN_UNPROT_VOICE_MBX_WRITE(pDev, data, wrcnt + 2);
      CHECK_HOST_ERR(pDev, {ret = IFX_ERROR;});
#endif /* VIN_2CPE */

      /* Unprotect host mailbox access. */
      /*VIN_HOST_RELEASE(pDev);*/
      VIN_ENABLE_IRQGLOBAL(flags);

      /* Free buffer holding data, put it back to bufferpool. */
      if (IFX_NULL != pData)
      {
         /* In global lock free buffer. */
         VIN_DISABLE_IRQGLOBAL(flags);
      
         ret = bufferPoolPut(pData);
      
         VIN_ENABLE_IRQGLOBAL(flags);

         if (IFX_SUCCESS != ret)
         {
            TRACE(VINETIC, DBG_LEVEL_HIGH, ("Data buffer could not be released."));
         }
      }
      else
      {
         TRACE(VINETIC, DBG_LEVEL_HIGH, ("Data buffer is empty."));
      }

   } /* if */

   return IFX_SUCCESS;
}


/**
   callback for egress packets redirect.

   \param chanDev - handle to the channel device

   \return IFX_SUCCESS if packet forward to LL driver
 */
IFX_return_t Qos_LL_PktEgress(IFX_TAPI_LL_CH_t* pLLCh)
{
   VINETIC_CHANNEL* pCh  = (VINETIC_CHANNEL *) pLLCh;
   VINETIC_DEVICE*  pDev = IFX_NULL;
   PACKET*          pPkt = IFX_NULL;
   PACKET           packet;
   IFX_int32_t      len  = 0;
   IFX_int32_t      data_len = 0;
   IFXOS_INTSTAT    flags;

   
   TRACE(VINETIC, DBG_LEVEL_LOW, ("Qos_LL_PktEgress()\n"));

   if (IFX_NULL == pCh)
   {
      TRACE(VINETIC, DBG_LEVEL_HIGH,
           ("Invalid input argument(s). (File: %s, line: %d)\n",
            __FILE__, __LINE__));

      return IFX_ERROR;
   }
   
   pDev = pCh->pParent;

   if (IFX_NULL == pDev)
   {
      /* Error, handle to device missing */
      TRACE(VINETIC, DBG_LEVEL_HIGH, ("Handle to vinetic device missing\n"));
      SET_ERROR(VIN_ERR_UNKNOWN);

      return IFX_ERROR;
   }

   /* In global lock read from fifo. */
   VIN_DISABLE_IRQGLOBAL(flags);

   /* fifo empty, exit */
   if (!fifoEmpty(TAPI_UpStreamFifo_Get(pCh->pTapiCh)))
   {
      pPkt = (PACKET *) fifoGet(TAPI_UpStreamFifo_Get(pCh->pTapiCh),
                                                      (int *) &len);

      VIN_ENABLE_IRQGLOBAL(flags);

      if ((IFX_NULL != pPkt) && (0 < len))
      {
         /* Make local copy of packet (put it on stack). */
         packet = *pPkt;
         /* Get total data len in bytes. */
         data_len = (pPkt->cmd2 & CMD2_LEN) * 2;
         /* Check if the ODD bit is set. */
         if (pPkt->cmd2 & CMD2_ODD)
         {
            data_len -= 1;
         }
      }
     
      /* In global lock free buffer. */
      VIN_DISABLE_IRQGLOBAL(flags);

      if ((IFX_NULL != pPkt) && (0 < len))
      {
         /* Put buffer back to BUFFERPOOL. Release memory. */
         bufferPoolPut(pPkt);
      }
   }

   VIN_ENABLE_IRQGLOBAL(flags);

   /* redirect packet. packet is a stack variable and can't be accessed
      from outside this function, so calling vtou_redirectrtp is now save,
      allthough interrupts are enabled */
   if (0 < data_len)
   {
      irq_IFX_TAPI_Qos_HL_PktEgress(pCh->pTapiCh->nChannel,
                                   (IFX_void_t *) packet.pData, data_len);
   }

   return IFX_SUCCESS;
}


/**
   Initializes the whole device inclusive all channels for Qos support .

   \param devHandle  - handle to the control device

   \return IFX_SUCCESS or IFX_ERROR.

   \remark
      This function will be called once at initialization time. Its checks
      if RTP Firmware was downloaded for Qos Services. If yes, it registers a
      callback function for the ingress packet redirection, initializes callback
      function for egress redirection and  stores the channel pointer in
      a global array for a later use by the callback function.
 */   
IFX_return_t Qos_LL_Init(IFX_uint32_t devHandle)
{
   VINETIC_DEVICE *pDev = (VINETIC_DEVICE *)devHandle;
   IFX_return_t ret = IFX_ERROR;

   
   TRACE(VINETIC, DBG_LEVEL_LOW, ("Qos_LL_Init()\n"));

   if (IFX_NULL == pDev)
   {
      TRACE(VINETIC, DBG_LEVEL_HIGH,
           ("Invalid input argument(s). (File: %s, line: %d)\n",
            __FILE__, __LINE__));
      
      return IFX_ERROR;
   }

   /* Check if we have an RTP fw. */
   if (((pDev->nEdspVers[0] & ECMD_VERS_EDSP_PRT)
       == ECMD_VERS_EDSP_PRT_RTP))
   {
      pDev->nDevState |= DS_QOS_INIT;
      
      /* Call also QOS init in upper layer with TAPI_DEV. */
      ret = IFX_TAPI_Qos_HL_Init(pDev->pTapiDev);
   }
   else 
   {
      TRACE(VINETIC, DBG_LEVEL_HIGH, ("FW is not supporting RTP."
       "File: %s, line: %d", __FILE__, __LINE__));
   }

   return ret;
}


#endif /* QOS_SUPPORT */

