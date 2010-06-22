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

   Changes     : 2006-06-18
                 Support for TAPI v3. Splitted into two parts, low level
                 and high level, tasklets were removed. Schedulling is not
                 used, because 10 msec is a lot of time to waste.
*******************************************************************************/

#ifndef LINUX
#error This feature is actually only available under Linux.
#endif

/* ============================= */
/* Includes                      */
/* ============================= */
#include "drv_tapi.h"
#include "ifx_udp_redirect.h"

/* ============================= */
/* Local definitions             */
/* ============================= */

/* ============================= */
/* Local structures              */
/* ============================= */

/* ============================= */
/* Local variable definition     */
/* ============================= */


/** Pointer to array of ptr to TAPI_CHANNELS, local copy. */
IFX_LOCAL TAPI_CHANNEL** prgQos_TapiCh = IFX_NULL;

/** Number of channels */
IFX_LOCAL IFX_int32_t nMax_Ch = 0;


/* ============================= */
/* Global variable definition    */
/* ============================= */

/* ============================= */
/* Local function declaration    */
/* ============================= */

/* Initiates a session by creating a udp filter into filter table */
IFX_LOCAL IFX_return_t Qos_StartSession(TAPI_CHANNEL* const pCh,
                                        const QOS_INIT_SESSION* const pInit);
/* activate a previous initiated session */
IFX_LOCAL IFX_return_t Qos_ActivateSession(TAPI_CHANNEL* const pCh,
                                           const IFX_int32_t nPort);
/* Delete a session previously initiated according to port */
IFX_LOCAL IFX_return_t Qos_StopSession(TAPI_CHANNEL* const pCh,
                                       const IFX_int32_t nPort);

/** Callback function for bufferpool get. */
IFX_LOCAL IFX_void_t* Qos_BufferPoolGet(const IFX_void_t* const pBuffPool);

/** Ingress high level func. */
IFX_void_t Qos_HL_PktIngress(IFX_int32_t nCh,
                             IFX_int32_t nLen,
                             IFX_void_t* pData);

/* ============================= */
/* Local function definition     */
/* ============================= */


/*
  Lock the TAPI channel.
  
  \param pCH - TAPI Channel
  
  \return IFX_SUCCESS if locked, otherwise IFX_FALSE 
*/
IFX_return_t Qos_LockChannel(TAPI_CHANNEL* const pCh)
{
   IFX_TAPI_DRV_CTX_t* pDrvCtx = IFX_NULL;
   IFX_TAPI_LL_DEV_t* pDev = IFX_NULL;

   TRACE(TAPI_DRV, DBG_LEVEL_LOW, ("Qos_LockChannel()\n"));

   if (IFX_NULL == pCh)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
           ("Invalid input argument(s). (File: %s, line: %d)\n",
            __FILE__, __LINE__));
      return IFX_ERROR;
   }

   pDrvCtx = (IFX_TAPI_DRV_CTX_t *) pCh->pTapiDevice->pDevDrvCtx;
   pDev = pCh->pTapiDevice->pLLDev;

   /* Lock channel. */
   IFXOS_MutexLock(pCh->semTapiChDataLock);

   /* Protect access to status structure, lock interrupts */
   if (ptr_chk(pDrvCtx->IRQ.LockDevice, "pDrvCtx->IRQ.LockDevice"))
   {
       pDrvCtx->IRQ.LockDevice(pDev);
       return IFX_SUCCESS;
   }

   return IFX_SUCCESS;
}


/*
  Unlock the TAPI channel.
  
  \param pCH - TAPI Channel
  
  \return IFX_SUCCESS if unlocked, otherwise IFX_FALSE 
*/
IFX_return_t Qos_UnlockChannel(TAPI_CHANNEL* const pCh)
{
   IFX_TAPI_DRV_CTX_t* pDrvCtx = IFX_NULL;
   IFX_TAPI_LL_DEV_t* pDev = IFX_NULL;


   TRACE(TAPI_DRV, DBG_LEVEL_LOW, ("Wrong input arguments.\n"));

   if (IFX_NULL == pCh)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
           ("Invalid input argument(s). (File: %s, line: %d)\n",
            __FILE__, __LINE__));

      return IFX_ERROR;
   }

   pDrvCtx = (IFX_TAPI_DRV_CTX_t *) pCh->pTapiDevice->pDevDrvCtx;
   pDev = pCh->pTapiDevice->pLLDev;

   /* Unlock channel. */
   IFXOS_MutexUnlock(pCh->semTapiChDataLock);

   /* Unlock interrupts */
   if (ptr_chk(pDrvCtx->IRQ.UnlockDevice, "pDrvCtx->IRQ.UnlockDevice"))
   {
       pDrvCtx->IRQ.UnlockDevice(pDev);
       return IFX_SUCCESS;
   }

   return IFX_SUCCESS;
}


/**
   Initiates a session by creating a filter into filter table.

   \param pCh    - handle to the channel device
   \param init   - handle to QOS_INIT_SESSION structure

   \return IFX_SUCCESS or IFX_ERROR.

   \remark
      This function initiates a session with mk_session. After successfull
      initiation, egress packets will be managed by a tasklet calling
      vtou_redirectrtp. This function must not be executed for local streaming.

   \see mk_session()
   \see vtou_redirectrtp()
*/
IFX_LOCAL IFX_return_t Qos_StartSession(TAPI_CHANNEL* const pCh,
                                        const QOS_INIT_SESSION* const pInit)
{
   IFX_return_t ret = IFX_SUCCESS;


   TRACE(TAPI_DRV, DBG_LEVEL_LOW, ("Qos_StartSession\n"));

   if ((IFX_NULL == pCh) || (IFX_NULL == pInit))
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
           ("Invalid input argument(s). (File: %s, line: %d)\n",
            __FILE__, __LINE__));

      ret = IFX_ERROR;
   }

   /* initiate a session only if calback was done already on this channel */
   if (QOS_STAT_INIT == pCh->QosCtrl.qosStat)
   {
      /* protect channel from mutual access */
      Qos_LockChannel(pCh);
      TRACE(TAPI_DRV, DBG_LEVEL_LOW, ("Will make session.\n"));
      ret = mk_session(pCh->nChannel, pInit->srcPort,
                       pInit->srcAddr, pInit->destPort,
                       pInit->destAddr);
      if (NO_ERROR == ret)
      {
         /* Make the driver ready to redirect packets
            if session successfully initialized. */

         pCh->QosCtrl.egressFlag = QOS_EGRESS_REDIR;
         ret = IFX_SUCCESS;
         TRACE(TAPI_DRV, DBG_LEVEL_LOW, ("Session is started.\n"));
      }
      else
      {
         TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("Session NOT started.\n"));
         ret = IFX_ERROR;
      }
      /* release channel */
      Qos_UnlockChannel(pCh);
   }

   return ret;
} /* Qos_StartSession() */


/**
   Activate a session previously initiated .

   \param pCh    - handle to the channel device
   \param port   - port of sesion to be activated

   \return IFX_SUCCESS or IFX_ERROR.

   \remark
      This function transparently calls the function active_session if a
      session was previously initiated on that channel. if packets aren't
      redirected any more, packets will be redirected after successfull
      activation

   \see active_session()
*/
IFX_LOCAL IFX_return_t Qos_ActivateSession(TAPI_CHANNEL* const pCh,
                                           const IFX_int32_t nPort)
{
   IFX_int32_t ret = IFX_SUCCESS;


   TRACE(TAPI_DRV, DBG_LEVEL_LOW, ("Qos_ActivateSession()\n"));

   if (IFX_NULL == pCh)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
           ("Invalid input argument(s). (File: %s, line: %d)\n",
            __FILE__, __LINE__));
      return IFX_ERROR;
   }

   /* Delete a session only if previously initiated. */
   if (QOS_STAT_INIT == pCh->QosCtrl.qosStat)
   {
      /* Protect channel from mutual access. */
      Qos_LockChannel(pCh);

      ret = active_session(pCh->nChannel, nPort);
      if (NO_ERROR == ret)
      {
         /* redirect packets of this channel if
            actually not redirected */
         if (QOS_EGRESS_REDIR != pCh->QosCtrl.egressFlag)
         {
            pCh->QosCtrl.egressFlag = QOS_EGRESS_REDIR;
         }
         TRACE(TAPI_DRV, DBG_LEVEL_LOW, ("Session activated.\n"));
         ret = IFX_SUCCESS;
      }
      else
      {
         TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("Session NOT activated.\n"));
         ret = IFX_ERROR;
      }
      /* release channel */
      Qos_UnlockChannel(pCh);
   }

   return ret;
}


/**
   Delete a session previously initiated .

   \param pCh    - handle to the channel device
   \param port   - port of session to be deleted

   \return IFX_SUCCESS or IFX_ERROR.

   \remark
      This function deletes a session with del_session. After successfull
      deletion, packets will not be redirected anymore

   \see del_session()
*/
IFX_LOCAL IFX_return_t Qos_StopSession(TAPI_CHANNEL* const pCh,
                                       const IFX_int32_t nPort)
{
   IFX_return_t ret = IFX_ERROR;


   TRACE(TAPI_DRV, DBG_LEVEL_LOW, ("Qos_StopSession()\n"));

   if (IFX_NULL == pCh)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
           ("Invalid input argument(s). (File: %s, line: %d)\n",
            __FILE__, __LINE__));
      return IFX_ERROR;
   }

   /* Delete a session only if previously initiated. */
   if ((QOS_STAT_INIT == pCh->QosCtrl.qosStat) &&
       (QOS_EGRESS_REDIR == pCh->QosCtrl.egressFlag))
   {
      /* Protect channel from mutual access. */
      Qos_LockChannel(pCh);

      ret = del_session (pCh->nChannel, nPort);
      /* Reset internal flag if necessary. */
      if (NO_ERROR == ret)
      {
         /* Stop redirection for ports equal or greater than
            QOS_PORT_CLEAN. */
         if (QOS_PORT_CLEAN <= nPort)
         {
            pCh->QosCtrl.egressFlag = QOS_EGRESS_NOREDIR;
         }
         TRACE(TAPI_DRV, DBG_LEVEL_LOW, ("Session was stopped.\n"));
         ret = IFX_SUCCESS;
      }
      else
      {
         TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("Session NOT stopped.\n"));
         ret = IFX_ERROR;
      }
      /* release channel */
      Qos_UnlockChannel(pCh);
   }

   return ret;
}


/**
   Will write packet data into mailbox of the device, but this will be done
   in the LL driver.

   \param nCh - channel number
   \param nLen - packet len
   \param pData - packet data
 */
IFX_void_t Qos_HL_PktIngress(IFX_int32_t nCh,
                             IFX_int32_t nLen,
                             IFX_void_t* pData)
{
   IFX_TAPI_DRV_CTX_t* pDrvCtx = IFX_NULL;
   TAPI_CHANNEL* pCh = IFX_NULL;
   IFX_return_t ret = IFX_SUCCESS;


   TRACE(TAPI_DRV, DBG_LEVEL_LOW, ("Qos_HL_PktIngress()\n"));

   if ((0 > nCh) || (nMax_Ch < nCh)
       || (0 >= nLen) || (IFX_NULL == pData))
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
           ("Invalid input argument(s). (File: %s, line: %d)\n",
            __FILE__, __LINE__));
   }

   pCh = prgQos_TapiCh[nCh];

   pDrvCtx = (IFX_TAPI_DRV_CTX_t*) pCh->pTapiDevice->pDevDrvCtx;

   if (IFX_NULL != pDrvCtx)
   {
      /* Calling low level ingress. */
      ret = pDrvCtx->Ingress(pCh->pLLChannel, pData, nLen);
   }
   else
   {
      /* Error getting pDrvCtx. */
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("Err getting ptr to low level "
            "driver.\n"));
   }

   return;
}


/**
   callback function for ingress packet to register at Qos_LL_Init.

   \param channel - channel number
   \param data    - ptr to data buffer
   \param len     - buffer length in bytes

   \return NO_ERROR , CALLBACK_ERR.

   \remark
    This function takes a channel device ptr from a global array and
    checks if the pointer is valid, meaning that Qos_LL_Init was done before.
    This function doesn't takes care of swapping the buffer accordingly before
    writing, asumming the data comes in the writable order. For little endian
    machines, the firmware swaps the data accordingly after a specific fw
    configuration is done (Endianness Control)

    As this function is invoked from the switch interrupt routine, it schould
    not block. Nevertheless, vinetic access should be protected against
    interrupts

   \see Qos_LL_Init()

*/
IFX_LOCAL IFX_int32_t Qos_PktIngressSched(IFX_int32_t nCh,
                                          IFX_void_t* pData,
                                          size_t nLen)
{
   TRACE(TAPI_DRV, DBG_LEVEL_LOW, ("Qos_PktIngressSched()\n"));

   if ((0 > nCh) || (nMax_Ch < nCh)
       || (0 >= nLen) || (IFX_NULL == pData))
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
           ("Invalid input argument(s). (File: %s, line: %d)\n",
            __FILE__, __LINE__));

      return IFX_ERROR;
   }

   Qos_HL_PktIngress(nCh, nLen, pData);

   return IFX_SUCCESS;
}


/**
   Callback for egress packets redirect.

   \param chanDev - handle to the channel device

   \return none

   \remark
   Packets are read via appropriate driver call. 
   This function also takes care of swapping the buffer accordingly before
   calling vtou_redirectrtp. During the data handling, interrupt must be
   locked to avoid data corruption (fifo pointers might get corrupted if not).

   Note: No disable/enable interrupts should be done if this function happens
         to be called from the interrupt routine.

   \see vtou_redirectrtp()
   \see Qos_HL_PktEgressSched()
*/
IFX_void_t irq_IFX_TAPI_Qos_HL_PktEgress(IFX_int32_t nCh,
                                         IFX_void_t* pData,
                                         IFX_int32_t nLen)
{
   TRACE(TAPI_DRV, DBG_LEVEL_LOW, ("Qos_HL_PktEgress()\n"));


   if ((0 > nCh) || (nMax_Ch < nCh)
       || (IFX_NULL == pData) || (0 > nLen))
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
           ("Invalid input argument(s). (File: %s, line: %d)\n",
            __FILE__, __LINE__));
      return;
   }

   /* Redirect packet. packet is a stack variable and can't be accessed
      from outside this function, so calling vtou_redirectrtp is now save,
      allthough interrupts are enabled. */

   if (0 < nLen)
   {
      vtou_redirectrtp(nCh, pData, nLen);
   }
   else
   {
      TRACE(TAPI_DRV, DBG_LEVEL_NORMAL, ("Got data with length 0.\n"));
   }
}


/* ============================= */
/* Global function definition    */
/* ============================= */

/**
   Initializes the whole device inclusive all channels for Qos support .

   \param pTapiDev - handle to TAPI device

   \return IFX_SUCCESS or IFX_ERROR.

   \remark
      This function will be called once at initialization time. Its checks
      if RTP Firmware was downloaded for Qos Services. If yes, it registers a
      callback function for the ingress packet redirection, initializes tasklets
      for egress redirection and  stores the channel pointer in a global array
      for a later use by the callback function.

   \see reg_callback()
   \see Qos_PktIngress()
*/
IFX_return_t IFX_TAPI_Qos_HL_Init(TAPI_DEV* pTapiDev)
{
   TAPI_CHANNEL* pCh = IFX_NULL;
   IFX_return_t ret = IFX_ERROR;
   IFX_int32_t i = 0;

   TRACE(TAPI_DRV, DBG_LEVEL_LOW, ("Qos_HL_Init()\n"));

   if (IFX_NULL == pTapiDev)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
           ("Invalid input argument(s). (File: %s, line: %d)\n",
            __FILE__, __LINE__));
      return IFX_ERROR;
   }

   nMax_Ch = pTapiDev->nMaxChannel;

   /* Register ingress handling packet routine and
      number of channels supported. */
   ret = reg_ingress(Qos_PktIngressSched, nMax_Ch);

   /* Register bufferpool get and buffer pool id. */
   ret = reg_buffer_pool_get(Qos_BufferPoolGet,
                             TAPI_VoiceBufferPoolHandle_Get());

   /* initialize channels */
   if (ret == NO_ERROR)
   {
      prgQos_TapiCh = (IFX_void_t *) IFXOS_MALLOC(sizeof(IFX_void_t) * nMax_Ch);
      if (IFX_NULL == prgQos_TapiCh)
      {
         TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
              ("Failed allocation mem for prgQos_TapiCh.\n\r"));
         return IFX_ERROR;
      }

      memset(prgQos_TapiCh, 0, sizeof(IFX_void_t) * nMax_Ch);

      for (i = 0; i < nMax_Ch; i++)
      {
         pCh = &pTapiDev->pTapiChanelArray[i];
         pCh->QosCtrl.egressFlag = QOS_EGRESS_NOREDIR;
         pCh->QosCtrl.qosStat = QOS_STAT_INIT;

         /* Cache channel pointer. */
         prgQos_TapiCh[i] = pCh;
      }
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("QOS init at highlevel.\n"));

      ret = IFX_SUCCESS;
   }
   else
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("QOS init FAILED at highlevel.\n"));
      ret = IFX_ERROR;
   }

   return ret;
}

/**
   does appropriate action on channel device for Qos support .

   \param chanDev - handle to the channel device
   \param qosCmd  - qos command
   \param chanDev - qos argument

   \return IFX_SUCCESS or IFX_ERROR.

   \remark This function is called from the OS Ioctl to process qos actions.

*/
IFX_int32_t Qos_Ctrl(IFX_uint32_t chanDev,
                     QOS_CMD qosCmd,
                     IFX_uint32_t qosArg)
{
   TAPI_CHANNEL* pCh = (TAPI_CHANNEL *)chanDev;
   IFX_int32_t ret = IFX_ERROR;
   QOS_INIT_SESSION _init;

   if (IFX_NULL == pCh)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
           ("Invalid input argument(s). (File: %s, line: %d)\n",
            __FILE__, __LINE__));
      return ret;
   }

   switch (qosCmd)
   {
      case FIO_QOS_START:
         IFXOS_CPY_USR2KERN(&_init, qosArg, sizeof(QOS_INIT_SESSION));
         ret = Qos_StartSession(pCh, &_init);
      break;
      case FIO_QOS_ACTIVATE:
         ret = Qos_ActivateSession(pCh, (IFX_int32_t) qosArg);
      break;
      case FIO_QOS_STOP:
         ret = Qos_StopSession(pCh, (IFX_int32_t) qosArg);
      break;
      case FIO_QOS_CLEAN:
         ret = Qos_Cleanup((IFX_uint32_t) pCh);
      break;
      default:
         /* Wrong QOS command. */
         TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("Wrong QOS command.\n"));
      break;
   }

   return ret;
}


/**
   Check if packet should be redirected via UDP redirection.

   \param pTapiCh - handle to TAPI_CHANNEL

   \return IFX_TRUE if packet should be redirected by UDP, otherwise IFX_FALSE
*/
IFX_boolean_t irq_IFX_TAPI_Qos_PacketRedirection(TAPI_CHANNEL* pTapiCh)
{
   if (IFX_NULL == pTapiCh)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
           ("Invalid input argument(s). (File: %s, line: %d)\n",
            __FILE__, __LINE__));
      return IFX_FALSE;
   }
   
   if (pTapiCh->QosCtrl.egressFlag == QOS_EGRESS_REDIR)
   {
      return IFX_TRUE;
   }
   
   return IFX_FALSE;
}


/**
   Schedules egress packet forwarding for packet redirection at a safe time.

   \param pTapiCh - handle to TAPI CHANNEL

   \return none.
*/
IFX_return_t irq_IFX_TAPI_Qos_PktEgressSched(TAPI_CHANNEL* pTapiCh)
{
   IFX_TAPI_DRV_CTX_t* pDrvCtx = IFX_NULL;


   TRACE(TAPI_DRV, DBG_LEVEL_LOW, ("Qos_PktEgressSched()\n"));

   if (IFX_NULL == pTapiCh)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
           ("Invalid input argument(s). (File: %s, line: %d)\n",
            __FILE__, __LINE__));
      return IFX_ERROR;
   }

   if (nMax_Ch < pTapiCh->nChannel)
   {
      /* Ch idx outside of bound. */
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("PktEgressSched --> "
            "channel %d index outside bounds.\n", pTapiCh->nChannel));
      return IFX_ERROR;
   }

   pDrvCtx = (IFX_TAPI_DRV_CTX_t*) pTapiCh->pTapiDevice->pDevDrvCtx;

   pDrvCtx->Egress(pTapiCh->pLLChannel);

   return IFX_SUCCESS;
}


/**
   Cleanup every ressource used by QOS.

   \param chanDev - handle to the any channel device

   \return IFX_SUCCESS or IFX_ERROR.

   \remark
       This function clears up the udp filter table, reset all channels and
      stops packet redirection. The device is locked during the reset process
      This function can be executed with any channel device.

   \see close_redirect()
*/
IFX_int32_t Qos_Cleanup(IFX_uint32_t chanDev)
{
   TAPI_CHANNEL* pCh = IFX_NULL;
   TAPI_DEV* pDev = (TAPI_DEV *) chanDev;
   IFX_int32_t      i = 0;
   IFX_return_t ret = IFX_SUCCESS;
   IFX_boolean_t qos_init = IFX_FALSE;

   TRACE(TAPI_DRV, DBG_LEVEL_LOW, ("Qos_Cleanup()\n"));

   if (IFX_NULL == pDev)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
           ("Invalid input argument(s). (File: %s, line: %d)\n",
            __FILE__, __LINE__));
      return IFX_ERROR;
   }

   /* Reset all channels. */
   for (i = 0; i < nMax_Ch; i++)
   {
      pCh = &pDev->pTapiChanelArray[i];
      if (IFX_NULL == pCh)
      {
         TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("Could not get tapi channel,"
               " ch idx %d\n", i));
         return IFX_ERROR;
      }

      if (QOS_STAT_INIT == pCh->QosCtrl.qosStat)
      {
         qos_init = IFX_TRUE;
      }

      /* Lock device : no task, no interrupt. */
      Qos_LockChannel(pCh);

      pCh->QosCtrl.qosStat = QOS_STAT_NOQOS;
      pCh->QosCtrl.egressFlag = QOS_EGRESS_NOREDIR;

      /* Release tasks and interrupts locks. */
      Qos_UnlockChannel(pCh);
   }

   if (IFX_TRUE == qos_init)
   {
      /* Clear all filter tables. */
      ret = close_redirect();

      /* Free memory */
      if (IFX_NULL != prgQos_TapiCh)
      {
         IFXOS_FREE(prgQos_TapiCh);
      }
      prgQos_TapiCh = IFX_NULL;
   }

   nMax_Ch = 0;

   return ret;
}


/**
  Callback function for bufferpool get.
  
  \param pBuffPool - pointer to bufferpool
  
  \return buffer if successfull, otherwise IFX_NULL 
 */
IFX_LOCAL IFX_void_t* Qos_BufferPoolGet(const IFX_void_t* const pBuffPool)
{
   IFX_void_t* buff = IFX_NULL;


   if (IFX_NULL == pBuffPool)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("NULL handle to BUFFERPOOL.\n"));
      return IFX_NULL;
   }

   buff = (IFX_void_t *)bufferPoolGet((IFX_void_t*) pBuffPool);

   return buff;
}

#endif /* QOS_SUPPORT */
