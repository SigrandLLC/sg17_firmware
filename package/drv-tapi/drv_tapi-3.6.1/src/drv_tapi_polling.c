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

*****************************************************************************
   Module      : drv_tapi_polling.c

   This file implements a set of functions necessary for packet/event
   polling functionality provided in TAPI high-level.

*/


/*
   POLLING CONCEPT
   For downstream direction, HL TAPI sorts all packets into LL device specific
   buffer queues. In case of Linux for all packets, free buffers are taken from
   the kernel level buffer pool and the packets from user space are copied to
   kernel space. Then the individual LL downstreaming routines are called in turn
   for all devices which have buffers in the downstreaming queues. Upon completion
   of writing a packet to the device the buffer is returned back to its respective
   buffer pool.

   For upstream direction, all LL device are in turn polled for new packets.
   In case a newly packet is available the LL device gets new buffer from the
   respective bufferpool. The LL device reads packets from the firmware as long as
   they are available.
*/

/* ============================= */
/* Includes                      */
/* ============================= */

#include "drv_tapi.h"
#include "drv_tapi_io.h"

/* ============================= */
/* Local Macros & Definitions    */
/* ============================= */

/** Used for creating linked lists of TAPI devices */
typedef struct _TAPI_DEV_LIST
{
   TAPI_DEV *pTapiDev;
   struct _TAPI_DEV_LIST *pNext;
} TAPI_DEV_LIST_t;

/** Global polling configuration container */
typedef struct _TAPI_POLL_CTRL
{
   /** Contains a list of TAPI devices that have been registerred for
       packets polling using the IFX_TAPI_POLL_PKTS_ADD ioctl. */
   TAPI_DEV_LIST_t *pDevPollPkts;

   /** Contains a list of TAPI devices that have been registerred for
       events polling using the IFX_TAPI_POLL_EVTS_ADD ioctl */
   TAPI_DEV_LIST_t *pDevPollEvts;
} TAPI_POLL_CTRL_t;

/** Buffer queue length allocated for handling of user space buffers (Linux) */
enum { IFX_TAPI_POLL_QUEUE = 64 };

/* ============================= */
/* Global variable definition    */
/* ============================= */

/* ============================= */
/* Local function declaration    */
/* ============================= */

LOCAL TAPI_DEV* TAPI_Poll_GetTapiDev(IFX_int32_t nDevID);

/* ============================= */
/* Local variable definition     */
/* ============================= */

/** Holding get, put and bufferpoll handle. */
LOCAL IFX_TAPI_POLL_CONFIG_t *pPollCfg = IFX_NULL;

/** Holding list of devices which are in polling mode
    either packets or events. */
LOCAL TAPI_POLL_CTRL_t *pPollCtrl = IFX_NULL;

/* ============================= */
/* Local function definition     */
/* ============================= */

/**
   Retrieve TAPI_DEV according to device ID.

   \param   nDevID - ID number od TAPI device

   \return IFX_NULL if not found in list, otherwise pointer to TAPI_DEV.

*/
LOCAL TAPI_DEV* TAPI_Poll_GetTapiDev(IFX_int32_t nDevID)
{
   TAPI_DEV_LIST_t* pList = IFX_NULL;


   if (IFX_NULL == pPollCtrl)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
           ("Polling control information not available. "
            "(File: %s, line: %d)\n",
            __FILE__, __LINE__));
      return IFX_NULL;
   }

   /* get first element from the list of event-polling devices */
   pList = pPollCtrl->pDevPollEvts;
   /* do it for all devices on the list */
   while (IFX_NULL != pList)
   {
      if (nDevID == pList->pTapiDev->nDevID)
      {
         /* Found this TAPI_DEV in list */
         return pList->pTapiDev;
      }

      /* advance to the next element on the list */
      pList = pList->pNext;
   }

   /* TAPI_DEV with this ID not found */
   return IFX_NULL;
}


/* ============================= */
/* Global function declaration   */
/* ============================= */


/* ============================= */
/* Global function definition    */
/* ============================= */


/**
   Global configuration for polling mode.

   In case of VxWorks, routines are registerred for getting new buffers from,
   and putting used buffers back to the buffer pool. Also a pointer to the
   buffer pool control data strucutre is provided. It is an obligation of the
   application to setup the buffer pool beforehand.

   In case of Linux this routine registers the kernel level buffer pool. The
   same way as for VxWorks routines are registerred for getting/putting buffers
   from/to the kernel space buffer pool. The buffer pool control data structure
   if registerred as well.

   \param    pGlobalPollCfg - handle to IFX_TAPI_POLL_CONFIG_t data structure

   \return IFX_SUCCESS or IFX_ERROR.

   \remark
*/
IFX_return_t TAPI_IrqPollConf(IFX_TAPI_POLL_CONFIG_t* pGlobalPollCfg)
{
#ifdef LINUX
   IFX_int32_t nPktLen;
#endif


   if (IFX_NULL == pGlobalPollCfg)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
           ("TAPI_IrqPollConf -> pGlobalPollCfg is NULL. "
            "(File: %s, line: %d)\n",
            __FILE__, __LINE__));
      return IFX_ERROR;
   }

   /** allocate the TAPI polling control data structure the first time around */
   if (pPollCfg == IFX_NULL)
   {
      if ((pPollCfg = IFXOS_MALLOC(sizeof(IFX_TAPI_POLL_CONFIG_t))) == NULL)
      {
         TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
              ("Could not allocate memory for TAPI polling control"
               " data structure. (File: %s, line: %d)\n",
               __FILE__, __LINE__));
         return IFX_ERROR;
      }
      memset(pPollCfg, 0, sizeof(IFX_TAPI_POLL_CONFIG_t));
   }

#ifdef VXWORKS
   /* initialize the buffer pool related pointers */
   pPollCfg->pBufPool = pGlobalPollCfg->pBufPool;
   pPollCfg->get = pGlobalPollCfg->get;
   pPollCfg->put = pGlobalPollCfg->put;
#endif /* VXWORKS */

#ifdef LINUX
   /* calculate max packet length, architecture word size aligned */
   nPktLen = (sizeof(IFX_TAPI_POLL_PKT_t) + sizeof(IFX_uint32_t)) &
            ~(sizeof(IFX_uint32_t));

   if ((pPollCfg->pBufPool =
         (void *)bufferPoolInit(nPktLen, IFX_TAPI_POLL_QUEUE, 0)) == IFX_NULL)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
           ("Failure allocating buffer pool. (File: %s, line: %d)\n",
            __FILE__, __LINE__));

      IFXOS_FREE(pPollCfg);
      return IFX_ERROR;
   }

   /* set the function pointer for retrieving and returning buffers */
   pPollCfg->get = (void *(*) (void *))bufferPoolGet;
   pPollCfg->put = bufferPoolPut;
#endif /* LINUX */

   return IFX_SUCCESS;
} /* TAPI_IrqPollConf() */


/******************************************************************************/
/**
   Executes the service identified by IFX_TAPI_POLL_PKTS_ADD ioctl.

   This function registers a TAPI device for packets polling. Basically it
   appends a pointer of the TAPI device structure to the end of the list
   of devices registerred for packets polling.

   \param pDrvCtx     - handle to interface between high and low level
   \param pTapiDev    - handle to the TAPI device

   \return
      IFX_ERROR on error,
      IFX_SUCCESS on success
*******************************************************************************/
IFX_return_t TAPI_AddDev_PollPkts(IFX_TAPI_DRV_CTX_t *pDrvCtx,
                                  TAPI_DEV *pTapiDev)
{
   TAPI_DEV_LIST_t *pList = IFX_NULL;
   TAPI_DEV_LIST_t *pPrevList = IFX_NULL;   
   TAPI_DEV_LIST_t *pListNew = IFX_NULL;


   /* Polling control parameters are necessary to be provided
      before a device is started in polling mode */
   if (pPollCfg == IFX_NULL)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
           ("Polling configuration not yet performed. (File: %s, line: %d)\n",
            __FILE__, __LINE__));
      return IFX_ERROR;
   }

   if (pPollCtrl == IFX_NULL)
   {
      if ((pPollCtrl = IFXOS_MALLOC(sizeof(TAPI_POLL_CTRL_t))) == IFX_NULL)
      {
         TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
              ("Failure allocating polling control container. "
               "(File: %s, line: %d)\n",
               __FILE__, __LINE__));
         return IFX_ERROR;
      }
      memset(pPollCtrl, 0, sizeof(TAPI_POLL_CTRL_t));
   }

   /* Check if the the device is already registerred */
   /* Get the first element in the list */
   pList = pPollCtrl->pDevPollPkts;
   pPrevList = pList;
   while (IFX_NULL != pList)
   {
      if (pTapiDev == pList->pTapiDev)
      {
         /* Device already registerred for packet polling */
         TRACE(TAPI_DRV, DBG_LEVEL_NORMAL,
              ("Device already registered for packet polling.\n"));

         return IFX_SUCCESS;
      }
      pPrevList = pList;      
      pList = pList->pNext;
   }

   TRACE(TAPI_DRV, DBG_LEVEL_NORMAL,
        ("Now register it %d, %d\n", pTapiDev->nDevID, TAPI_MAX_LL_DEVICES));

   /* Register the device by placing it onto the list of devices used for
      PACKET polling */
   if ((pListNew = IFXOS_MALLOC(sizeof(TAPI_DEV_LIST_t))) == IFX_NULL)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
           ("Failure getting memory for new device list item in polling. "
            "(File: %s, line: %d)\n",
            __FILE__, __LINE__));

      return IFX_ERROR;
   }

   pTapiDev->pPollCfg = pPollCfg;
   pListNew->pTapiDev = pTapiDev;

   if (IFX_NULL != pPollCtrl->pDevPollPkts)
   {
      pPrevList->pNext = pListNew;
   }
   else
   {
      pPollCtrl->pDevPollPkts = pListNew;
   }
   pListNew->pNext = IFX_NULL;

   /* Execute registerred LL routine to disable Voice related interrupts */
   if (ptr_chk(pDrvCtx->POLL.pktsIRQCtrl, "pDrvCtx->POLL.pktsIRQCtrl"))
   {
      pDrvCtx->POLL.pktsIRQCtrl((IFX_TAPI_LL_DEV_t *) pTapiDev, IFX_FALSE);
   }

   /* Set flag that this device is in polling mode for packets. */
   pTapiDev->fWorkingMode |= TAPI_POLLING_MODE_PACKETS;

   return IFX_SUCCESS;
} /* TAPI_AddDev_PollPkts() */


/******************************************************************************/
/**
   Executes the service identified by IFX_TAPI_POLL_EVTS_ADD ioctl.

   This function registers a TAPI device for packets polling. Basically it
   appends a pointer of the TAPI device structure to the end of the list
   of devices registerred for packets polling.

   \param pDrvCtx     - handle to interface between high and low level
   \param pTapiDev    - handle to the TAPI device

   \return
      IFX_ERROR on error,
      IFX_SUCCESS on success
*******************************************************************************/
IFX_return_t TAPI_AddDev_PollEvts(IFX_TAPI_DRV_CTX_t *pDrvCtx,
                                  TAPI_DEV *pTapiDev)
{
   TAPI_DEV_LIST_t *pList = IFX_NULL;
   TAPI_DEV_LIST_t *pPrevList = IFX_NULL;   
   TAPI_DEV_LIST_t* pListNew = IFX_NULL;


   /* Polling control parameters are necessary to be provided before
      a device is started in polling mode */
   if (pPollCfg == IFX_NULL)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
           ("Polling configuration not yet performed. (File: %s, line: %d)\n",
            __FILE__, __LINE__));
      return IFX_ERROR;
   }

   if (pPollCtrl == IFX_NULL)
   {
      if ((pPollCtrl = IFXOS_MALLOC(sizeof(TAPI_POLL_CTRL_t))) == IFX_NULL)
      {
         TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
              ("Failure allocating polling control container. "
               "(File: %s, line: %d)\n",
               __FILE__, __LINE__));
         return IFX_ERROR;
      }
      memset(pPollCtrl, 0, sizeof(TAPI_POLL_CTRL_t));
   }

   /* Check if the the device is already registerred */
   /* Get the first element in the list */
   pList = pPollCtrl->pDevPollEvts;
   pPrevList = pList;
   while (IFX_NULL != pList)
   {
      if (pTapiDev == pList->pTapiDev)
      {
         /* Device already registerred for event polling */
         TRACE(TAPI_DRV, DBG_LEVEL_NORMAL,
              ("Device already registered for event polling.\n"));

         return IFX_SUCCESS;
      }
      pPrevList = pList;      
      pList = pList->pNext;
   }

   /* Register the device by placing it onto the list of devices used for
      EVENT polling */
   if ((pListNew = IFXOS_MALLOC(sizeof(TAPI_DEV_LIST_t))) == IFX_NULL)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
           ("Failure getting memory for new device list item in polling. "
            "(File: %s, line: %d)\n",
            __FILE__, __LINE__));
      return IFX_ERROR;
   }

   pTapiDev->pPollCfg = pPollCfg;
   pListNew->pTapiDev = pTapiDev;

   if (IFX_NULL != pPollCtrl->pDevPollEvts)
   {
      pPrevList->pNext = pListNew;
   }
   else
   {
      pPollCtrl->pDevPollEvts = pListNew;
   }
   pListNew->pNext = IFX_NULL;

   /* Execute registerred LL routine to disable Events related interrupts */
   if (ptr_chk(pDrvCtx->POLL.evtsIRQCtrl, "pDrvCtx->POLL.evtsIRQCtrl"))
   {
      pDrvCtx->POLL.evtsIRQCtrl((IFX_TAPI_LL_DEV_t *)pTapiDev, IFX_FALSE);
   }

   /* Set flag that this device is in polling mode for events. */
   pTapiDev->fWorkingMode |= TAPI_POLLING_MODE_EVENTS;

   return IFX_SUCCESS;
} /* TAPI_AddDev_PollEvts() */


/******************************************************************************/
/**
   Executes the service identified by IFX_TAPI_POLL_EVTS_ADD ioctl.

   This function registers a TAPI device for packets polling. Basically it
   appends a pointer of the TAPI device structure to the end of the list
   of devices registerred for packets polling.

   \param pDrvCtx     - handle to interface between high and low level
   \param pTapiDev    - handle to the TAPI device

   \return
      IFX_ERROR on error,
      IFX_SUCCESS on success
*******************************************************************************/
IFX_return_t TAPI_RemDev_PollPkts(IFX_TAPI_DRV_CTX_t *pDrvCtx,
                                  TAPI_DEV *pTapiDev)
{
   TAPI_DEV_LIST_t *pList = IFX_NULL;
   TAPI_DEV_LIST_t *pPrevList = IFX_NULL;
   TAPI_DEV *pTmpDev = IFX_NULL;


   /* Polling configuration parameters are necessary to be provided,
      ioctl(IFX_TAPI_POLL_CONFIG,...), before a device is started
      in polling mode */
   if (pPollCfg == IFX_NULL)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
           ("Polling configuration not yet performed. (File: %s, line: %d)\n",
            __FILE__, __LINE__));
      return IFX_ERROR;
   }

   if (pPollCtrl == IFX_NULL)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
           ("Polling control information not available. "
            "(File: %s, line: %d)\n",
            __FILE__, __LINE__));
      return IFX_ERROR;
   }

   /* Check if device already registerred for PACKET polling */
   pList = pPollCtrl->pDevPollPkts;
   pPrevList = pList;
   while (IFX_NULL != pList)
   {
      if (pTapiDev == pList->pTapiDev)
      {
         /* Matching device found on the list, remove it from the list */
         pTmpDev = pList->pTapiDev;
         if (pList == pPrevList)
         {
            /* We are deleting first element in list */
            pPrevList = pList->pNext;
         }
         else
         {
            pPrevList->pNext = pList->pNext;
         }

         IFXOS_FREE(pList);
         pList = pPrevList;
         break;
      }
      /* Get the pointer to the next list entry */
      pPrevList = pList;
      pList = pList->pNext;
   }

   if (pTmpDev == IFX_NULL)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
           ("Failure unregisterring device (DevID %d) from packet polling. "
            "(File: %s, line: %d)\n",
            pTapiDev->nDevID, __FILE__, __LINE__));
      return IFX_ERROR;
   }

   /* Execute registerred LL routine to re-enable PACKET related interrupts */
   if (ptr_chk(pDrvCtx->POLL.pktsIRQCtrl, "pDrvCtx->POLL.pktsIRQCtrl"))
   {
      pDrvCtx->POLL.pktsIRQCtrl((IFX_TAPI_LL_DEV_t *)pTapiDev, IFX_TRUE);
   }

   /* Set flag that this device is NOT in polling mode for packets. */
   pTapiDev->fWorkingMode &= ~TAPI_POLLING_MODE_PACKETS;

   return IFX_SUCCESS;
} /* TAPI_RemDev_PollPkts */


/******************************************************************************/
/**
   Executes the service identified by IFX_TAPI_POLL_EVTS_ADD ioctl.

   This function registers a TAPI device for packets polling. Basically it
   appends a pointer of the TAPI device structure to the end of the list
   of devices registerred for packets polling.

   \param pDrvCtx     - handle to interface between high and low level
   \param pTapiDev    - handle to the TAPI device

   \return
      IFX_ERROR on error,
      IFX_SUCCESS on success
*******************************************************************************/
IFX_return_t TAPI_RemDev_PollEvts(IFX_TAPI_DRV_CTX_t *pDrvCtx,
                                  TAPI_DEV *pTapiDev)
{
   TAPI_DEV_LIST_t *pList = IFX_NULL;
   TAPI_DEV_LIST_t *pPrevList = IFX_NULL;
   TAPI_DEV *pTmpDev = IFX_NULL;

   /* Polling control parameters are necessary to be provided before
      a device is started in polling mode */
   if (pPollCfg == IFX_NULL)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
           ("Polling configuration not yet performed. (File: %s, line: %d)\n",
            __FILE__, __LINE__));
      return IFX_ERROR;
   }

   if (pPollCtrl == IFX_NULL)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
           ("Polling control information not available. "
            "(File: %s, line: %d)\n",
            __FILE__, __LINE__));
      return IFX_ERROR;
   }

   /* Check if device already registerred for EVENT polling */
   pList = pPollCtrl->pDevPollEvts;
   pPrevList = pList;
   while (IFX_NULL != pList)
   {
      if (pTapiDev == pList->pTapiDev)
      {
         /* Matching device found on the list, remove it from the list */
         pTmpDev = pList->pTapiDev;
         if (pList == pPrevList)
         {
            /* We are deleting first element in list */
            pPrevList = pList->pNext;
         }
         else
         {
            pPrevList->pNext = pList->pNext;
         }
         IFXOS_FREE(pList);
         pList = pPrevList;
         break;
      }
      /* Get the pointer to the next list entry */
      pPrevList = pList;
      pList = pList->pNext;
   }

   if (pTmpDev == IFX_NULL)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
           ("Failure unregisterring device (DevID %d) from event polling. "
            "(File: %s, line: %d)\n",
            pTapiDev->nDevID, __FILE__, __LINE__));
      return IFX_ERROR;
   }

   /* Execute registerred LL routine to enable EVENT related interrupts */
   if (ptr_chk(pDrvCtx->POLL.evtsIRQCtrl, "pDrvCtx->POLL.evtsIRQCtrl"))
   {
      pDrvCtx->POLL.evtsIRQCtrl((IFX_TAPI_LL_DEV_t *)pTapiDev, IFX_TRUE);
   }

   /* Set flag that this device is NOT in polling mode for events. */
   pTapiDev->fWorkingMode &= ~TAPI_POLLING_MODE_EVENTS;

   return IFX_SUCCESS;
} /* TAPI_RemDev_PollEvts() */


/******************************************************************************/
/**
   Reads interrupt status registers and updates events for each device

   \remark
   This function is called by a client who has disabled interrupts and is to
   call periodically to read the VINETIC status registers.

   \remark
   Mailbox access and shared variables are protected against
   concurent access / tasks
*/
IFX_void_t  TAPI_Poll_Events(IFX_void_t)
{
   IFX_TAPI_DRV_CTX_t   *pDrvCtx = IFX_NULL;
   TAPI_DEV             *pTapiDev = IFX_NULL;
   TAPI_DEV_LIST_t      *pList = IFX_NULL;


   if (pPollCtrl == IFX_NULL)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
           ("Polling control information not available. "
            "(File: %s, line: %d)\n",
            __FILE__, __LINE__));
      return;
   }

   /* Get first element from the list of event-polling devices */
   pList = pPollCtrl->pDevPollEvts;
   /* Do it for all devices on the list */
   while (IFX_NULL != pList)
   {
      pTapiDev = pList->pTapiDev;
      pDrvCtx = (IFX_TAPI_DRV_CTX_t *) pTapiDev->pDevDrvCtx;

      if (ptr_chk(pDrvCtx->POLL.pollEvents, "pDrvCtx->POLL.pollEvents"))
      {
         pDrvCtx->POLL.pollEvents((IFX_TAPI_LL_DEV_t *) pTapiDev);
      }

      /* Advance to the next element on the list */
      pList = pList->pNext;
   }
} /* TAPI_Poll_Events() */


/******************************************************************************/
/**
   Send packets to the Vinetic mailbox (non interleaved mode)

   \param   **ppPktsDown - pointer to the array of buffer pointers
   \param   pPktsDownNum   - number of valid buffer pointers in the array

   \return  IFX_SUCCESS or IFX_ERROR

   \remarks
   Mailbox access is protected against concurent access / tasks

   Function sends packets the the VINETIC mailbox. An array of pointers
   provides the reference to the packets (ppPktsDown). pPktsDownNum is the
   number of valid pointers (packets) to be sent. NULL pointers in the
   array are allowed and will be ignored (i.e. NULL pointer do not count
   as packets in pPktsDownNum).
   The packets are written to the Vinetic in packet by packet. After sending
   the packets to the VINETIC mailbox, the buffers will be returned to
   the bufferpool.
   The application is responsible to populate the cmd2 header fields such
   as packet length and odd bit!

   TODO discribe here the NEW packet structure (Scarfone's input)

                pCurPkt   ---> +------------------+  packetLen
                            |> +  CMD HDR Word 1  +  -2
                            |> +  CMD HDR Word 2  +  -1
                            |> +------------------+
                pCurPktPos--|> +  RTP data        +  46 (depending on coder)
                            |> +                  +  45
                            |> +       ..         +   .
                            |> +                  +   0
                            |> +      0000        +
          end of buffer        +------------------+
          check pattern        +      AAAA        +
          of bufferPool        +------------------+
*******************************************************************************/
IFX_return_t TAPI_Poll_Down (IFX_void_t **ppPktsDown, IFX_int32_t *pPktsDownNum)
{
   IFX_return_t         ret = IFX_SUCCESS;
   IFX_int32_t          nDev, i;
   IFX_TAPI_POLL_PKT_t  *pPkt = IFX_NULL;
   IFX_int32_t          nPktsDone, nPktsWr;
   IFX_int32_t          nPktsNum = 0;
   TAPI_DEV             *pTapiDev = IFX_NULL;
   IFX_TAPI_DRV_CTX_t   *pDrvCtx = IFX_NULL;
   TAPI_DEV_LIST_t      *pList = IFX_NULL;
   IFX_void_t           **ppPkts = IFX_NULL;
   IFX_int32_t          nCorruptedPkts = 0;
   IFX_boolean_t        fErrWriteToDev = IFX_FALSE;

#ifdef LINUX
   IFX_void_t           *pPktsArray[IFX_TAPI_POLL_QUEUE] = {0};
   IFX_void_t           *pPktDst = IFX_NULL, *pPktSrc = IFX_NULL;
#endif /* LINUX */

   if (pPollCfg == IFX_NULL)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
           ("Polling configuration not yet performed. (File: %s, line: %d)\n",
            __FILE__, __LINE__));
      return IFX_ERROR;
   }

   if ((IFX_NULL == ppPktsDown) || (IFX_NULL == pPktsDownNum))
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
           ("Wrong input arguments. (File: %s, line: %d)\n",
            __FILE__, __LINE__));
      return IFX_ERROR;
   }

#ifdef LINUX
   ppPkts   = ppPktsDown;
   nPktsNum = *pPktsDownNum;

   for (i = 0; i < nPktsNum; i++)
   {
      /* Skip any NULL pointers in the array of packet pointers */
      while (*ppPkts == IFX_NULL)
      {
         ppPkts++;
      }
      pPktSrc = *ppPkts++;
      /* Get a new buffer from the buffer pool */
      if ((pPktDst = pPollCfg->get(pPollCfg->pBufPool)) == IFX_NULL)
      {
         ret = IFX_ERROR;
         TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
              ("Failed to retrieve buffer from buferpool. "
               "(File: %s, line: %d)\n",
               __FILE__, __LINE__));
         goto error;
      }
      /* Copy packet to the new buffer */
      if (copy_from_user (pPktDst, pPktSrc,
                          ((IFX_TAPI_POLL_PKT_t *) pPktSrc)->len) > 0)
      {
         ret = IFX_ERROR;
         TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
              ("Failde to copy packet to new buffer. "
               "(File: %s, line: %d)\n",
               __FILE__, __LINE__));
         goto error;
      }
      pPktsArray[i] = pPktDst;
   }
   ppPkts = pPktsArray;
#endif /* LINUX */

#ifdef VXWORKS
   ppPkts = ppPktsDown;
   nPktsNum = *pPktsDownNum;
#endif /* VXWORKS */

   nPktsDone = 0;
   /* Sort the packets per device specific queues */
   for (i = 0; i < nPktsNum; i++)
   {
#ifdef VXWORKS
      /* Skip any NULL pointers in the array of packet pointers */
      while (*ppPkts == NULL)
      {
         ppPkts++;
      }
#endif
      pPkt = ((IFX_TAPI_POLL_PKT_t *) *ppPkts);
      ppPkts++;
      nDev = pPkt->dev; /* Get the TAPI device ID */
      pTapiDev = TAPI_Poll_GetTapiDev(pPkt->dev);

      if ((nDev < TAPI_MAX_LL_DEVICES) && (pTapiDev != IFX_NULL))
      {
         /* Place the packet onto the device specific FIFO */
         fifoPut(pTapiDev->pDownStreamFifo, (IFX_void_t *) pPkt, pPkt->len);
      }
      else
      {
         /* Discarding of the packet because of invalid device ID,
            or the device is not registerred for packet polling.
            Return the packet back to the buffer pool */

         if (pPollCfg->put((void *)pPkt) != IFX_SUCCESS)
         {
            /* Increase counter of corrupted packets. */
            nCorruptedPkts++;
            TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
                 ("Corrupted packet in downstream for device %d. "
                  "(File: %s, line: %d)\n",
                  nDev, __FILE__, __LINE__));
         }
      }
   }

   /* Check for outgoing packets in the downstream FIFOs for all devices and
      write any enqueued packets to the respective LL devices. The devices are
      served in round-robin fashion. In case a LL device fails to write all
      enqueued packets to the firmware, that device will not be retried until
      the next itteration of this routine. Hence the packet(s) will stay in the
      device specific FIFO.
   */
   pList = pPollCtrl->pDevPollPkts;
   fErrWriteToDev = IFX_FALSE;
   while (IFX_NULL != pList)
   {
      pTapiDev = pList->pTapiDev;
      nDev = pTapiDev->nDevID;

      /* We perform only single attempt to write packets to the LL device
         per polling itteration */
      if (!fifoEmpty(pTapiDev->pDownStreamFifo))
      {
         pDrvCtx = (IFX_TAPI_DRV_CTX_t *) pTapiDev->pDevDrvCtx;

         /* LL packet write routine shall attempt to write all packets
            enqueued onto the device specific FIFO */
         nPktsWr = pTapiDev->pDownStreamFifo->fifoElements;
         if (ptr_chk(pDrvCtx->POLL.wrPkts, "pDrvCtx->POLL.wrPkts"))
         {
            ret = pDrvCtx->POLL.wrPkts(pTapiDev, &nPktsWr);
         }
         else
         {
            ret = IFX_ERROR;
         }

         if (ret == IFX_SUCCESS)
         {
            nPktsDone += nPktsWr;
         }
         else
         {
            /* Error writing packet, return buffer to buffer pool. */
            fifoPut(pTapiDev->pDownStreamFifo, (void *) pPkt, pPkt->len);
            pPollCfg->put((void *) pPkt);

            TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
                 ("Could not write packet to device %d. "
                  "(File: %s, line: %d)\n",
                  nDev, __FILE__, __LINE__));

            /* Set flag indicating error when trying to write
               packet to device. */
            fErrWriteToDev = IFX_TRUE;
         }
      }
      /* Advance to the next TAPI device in the list */
      pList = pList->pNext;
   }

   if ((0 < nCorruptedPkts) || (IFX_TRUE == fErrWriteToDev))
   {
      /* Some packets were corrupted or could not be written to device. */
      ret = IFX_ERROR;
   }

   *pPktsDownNum = nPktsDone;
   return ret;
} /* TAPI_Poll_Down() */


/******************************************************************************/
/**
   Polls for packets all devices registerred for packet polling.

   \param   **ppPktsUp     - Points to an array of buffer pointers to free
                             bufers, provided for reading of new packets.
   \param   *ppPktsUpNum  - On entry gives the number od free buffers provided
                             in the array ppPkts. On exit returns the number of
                             packets read.

   \return  IFX_SUCCESS or IFX_ERROR

   \remarks:
   The following rules are applied by this routine:

   1. Upstream HL Polling Rules
    - all registerred LL devices are polled for packets in round-robin fashion
    - LL upstream routines shall read all packets available from
      their mailboxes (see Upstream LL Polling Rules below)
    - LL devices are polled only once per itteration of this routine

   2. Upstream LL Polling Rules
    - LL routine rdPkts(), reads packets from the device
      as long as all of the following are satisfied:
        a) Packets are available
        b) Buffers from the buffer pool are available
        c) The size of the provided by the application buffer pointer array
           is not exceeded (i.e. total number of packets read does not
           exceed *pPktsUpNum)

   Buffers for reading packets are provided by the application in an array of
   pointers referenced by ppPktsUp. The number of buffers available is given in
   *pPktsUpNum.

   In case of Linux, the LL TAPI uses buffers from a kernel level buffer poll
   for reading new packets. When finished polling of all devices the newly
   read packets are copied to user-space into the buffer provided by the
   application.

   In case of VxWorks, because of the linear memory space, the LL TAPI uses the
   buffers provided by the application for direct reading of the newly available
   packets into.

*******************************************************************************/
IFX_return_t TAPI_Poll_Up (IFX_void_t **ppPktsUp, IFX_int32_t *pPktsUpNum)
{
   IFX_return_t         ret = IFX_SUCCESS;
   IFX_int32_t          nBuffRemain, nPktsUp, nPktsUpTotal;
   IFX_TAPI_DRV_CTX_t   *pDrvCtx = IFX_NULL;
   IFX_void_t           **ppPkts = IFX_NULL;
   TAPI_DEV_LIST_t      *pList = IFX_NULL;
   TAPI_DEV             *pTapiDev = IFX_NULL;
   IFX_boolean_t        fErrReadFromDev = IFX_FALSE;

#ifdef LINUX
   IFX_TAPI_POLL_PKT_t  *pPkt = IFX_NULL;
   IFX_void_t           *pPktsArray[IFX_TAPI_POLL_QUEUE] = {0};


   ppPkts = pPktsArray;
#else /* LINUX */
   ppPkts = ppPktsUp;
#endif /* LINUX */


   if (pPollCfg == IFX_NULL)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
           ("Polling configuration not yet performed. (File: %s, line: %d)\n",
            __FILE__, __LINE__));
      return IFX_ERROR;
   }
   
   if ((IFX_NULL == ppPktsUp) || (IFX_NULL == pPktsUpNum))
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
           ("Wrong input arguments. (File: %s, line: %d)\n",
            __FILE__, __LINE__));
      return IFX_ERROR;
   }

   /* Upstream HL Polling Rules (see above) */
   nPktsUpTotal = 0;
   nBuffRemain = *pPktsUpNum;
   pList = pPollCtrl->pDevPollPkts;
   fErrReadFromDev = IFX_FALSE;
   while (IFX_NULL != pList)
   {
      pTapiDev = pList->pTapiDev;
      pDrvCtx = (IFX_TAPI_DRV_CTX_t *) pTapiDev->pDevDrvCtx;
      /* Upstream LL Polling Rules (see above) */
      /*
         ppPkts not modified in rdPkts
         nPktsUp returns the number of packets read
       */

      nPktsUp = 0;
      if (ptr_chk(pDrvCtx->POLL.rdPkts, "pDrvCtx->POLL.rdPkts"))
      {
         ret = pDrvCtx->POLL.rdPkts(pTapiDev, ppPkts,
                                    &nPktsUp, pTapiDev->nDevID);
      }
      else
      {
         ret = IFX_ERROR;
      }

      if (ret == IFX_SUCCESS)
      {
         /* Advance the buffer array past the used buffers */
         ppPkts += nPktsUp;
         /* Account total packets read */
         nPktsUpTotal += nPktsUp;
         /* Account remaining number of available buffers */
         nBuffRemain -= nPktsUp;
         if (nBuffRemain <= 0)
         {
            /* No more buffers left, discontinue reading packets */
            break;
         }
      }
      else
      {
         /* Error reading packets from device, but continue reading on other
            devices. */
         TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
            ("Could not read packets from device %d. (File: %s, line: %d)\n",
             pTapiDev->nDevID, __FILE__, __LINE__));

         fErrReadFromDev = IFX_TRUE;
      }

      pList = pList->pNext;
   }

#ifdef LINUX
   /* Copy all read packets to user space into buffers provided by ppPktsUp */
   ppPkts = ppPktsUp;
   *pPktsUpNum = 0;
   for (i = 0; i < nPktsUpTotal; i++)
   {
      pPkt = (TAPI_PACKET_t *) pPktsArray[i];
      if (copy_to_user (*ppPkts, (void *) pPkt, pPkt->len) > 0)
      {
         /* This condition of writing NULL pointer to the array in case of error
            has to be documented ! */
         pPollCfg->put((void *)pPkt);
         *ppPkts = NULL;
         ret = IFX_ERROR;
         goto error;
      }
      ppPkts++;
      *pPktsUpNum++;
   }
#endif /* LINUX */

   if (IFX_TRUE == fErrReadFromDev)
   {
      /* Error occured when reading packets from device */
      ret = IFX_ERROR;
   }

   /* Return number of read packet */
   *pPktsUpNum = nPktsUpTotal;

   return ret;
} /* TAPI_Poll_Up() */


/* -------------------------------------------------------------------
                      JUST FOR TESTING PURPOSES
*/
 
/**
   Used for testing of polling:
   - testing list of devices (add, remove, retrieve)

   \return IFX_ERROR on error, otherwise IFX_SUCCESS.
*/
IFX_return_t TAPI_Poll_Test(IFX_void_t)
{
   enum { MAX_TEST_DEV = 5 };
   enum { TEST_OFFSET_ID = 1000 };

   TAPI_DEV test_tapi_dev[MAX_TEST_DEV];
   IFX_int32_t i = 0;
   IFX_int32_t j = 0;
   IFX_return_t ret = IFX_SUCCESS;
   /* Just for testing, otherwise empty. */
   IFX_TAPI_DRV_CTX_t test_drv_ctx;
   TAPI_DEV* temp_tapi_dev = IFX_NULL;
   TAPI_DEV_LIST_t* pList = IFX_NULL;
   

   printf("TEST POLL: Perform list of devices testing.\n");
   printf("TEST POLL: Will test adding/removing/getting device from/to list.\n");
   printf("TEST POLL: Have two lists, one for devices handling packets, "
          "the other for devices handling events.\n");

   memset(&test_drv_ctx, 0, sizeof(IFX_TAPI_DRV_CTX_t));

   /* ---------------------------------------------------------------- 
                            SET ID to all devices
    */
   printf("TEST POLL: Set IDs to %d devices.\n", MAX_TEST_DEV);
   
   for (i = 0; i < MAX_TEST_DEV; i++)
   {
      /* Set IDs. */
      test_tapi_dev[i].nDevID = TEST_OFFSET_ID + i;
      printf("TEST POLL: Device %d, get ID %d\n",
             (int) i, (int) (TEST_OFFSET_ID + i));
   }

   /* ---------------------------------------------------------------- 
                                 TEST 0:
                            COUNT OF ALL DEVICES
    */

   printf("\nTEST 0: return count of all devices in list.\n");
   printf("--------------------------------------------\n");
   /* Return count of all in list */
   i = 0;
   pList = pPollCtrl->pDevPollEvts;
   while (pList != IFX_NULL)
   {
      pList = pList->pNext;
      i++;
   };
   j = 0;
   pList = pPollCtrl->pDevPollPkts;
   while (pList != IFX_NULL)
   {
      pList = pList->pNext;
      j++;
   };
   printf("TEST POLL: List of pkts has %d device(s) and list of"
          " evts has %d device(s).\n", (int) j, (int) i);

   /* ---------------------------------------------------------------- 
                            TEST 1: 
                   Add few devices, return count. Afterthat
                      remove them and again return count.
    */

   printf("\nTEST1: Add all devices to both lists, return count.\n");
   printf("       Then remove all and again return count.\n");
   printf("---------------------------------------------------\n");
   
   for (i = 0; i < MAX_TEST_DEV; i++)
   {
      printf("TEST POLL: Adding tapi dev id %d, %0X to polling list for pkts.\n",
             (int) (TEST_OFFSET_ID + i), (int) &test_tapi_dev[i]);
      
      ret = TAPI_AddDev_PollPkts(&test_drv_ctx, &test_tapi_dev[i]);
      if (ret != IFX_SUCCESS)
      {
         printf("Error adding tapi dev id %d, %0X to polling list for pkts. "
               "(File: %s, line: %d)\n",
               (int) (TEST_OFFSET_ID + i),
               (int) &test_tapi_dev[i], __FILE__, __LINE__);

         return ret;
      }

      printf("TEST POLL: Adding tapi dev id %d, %0X to polling list for evts.\n",
            (int) (TEST_OFFSET_ID + i), (int) &test_tapi_dev[i]);

      ret = TAPI_AddDev_PollEvts(&test_drv_ctx, &test_tapi_dev[i]);
      if (ret != IFX_SUCCESS)
      {
         printf("Error adding tapi dev id %d, %0X to polling list for evts. "
               "(File: %s, line: %d)\n",
               (int) (TEST_OFFSET_ID + i), (int) &test_tapi_dev[i],
                __FILE__, __LINE__);
         return ret;
      }
   }


   /* Return count of all in list */
   i = 0;
   pList = pPollCtrl->pDevPollEvts;
   while (pList != IFX_NULL)
   {
      pList = pList->pNext;
      i++;
   };
   j = 0;
   pList = pPollCtrl->pDevPollPkts;
   while (pList != IFX_NULL)
   {
      pList = pList->pNext;
      j++;
   };
   printf("TEST POLL: List of pkts has %d device(s) and list of"
          " evts has %d device(s).\n", (int) j, (int) i);

   /* Now test remove one, then add it, try to get specific one from list,
      remove all, try to get one from list, ... */

   for (i = 0; i < MAX_TEST_DEV; i++)
   {
      printf("TEST POLL: Remove tapi dev id %d, %0X to polling list for pkts.\n",
            (int) (TEST_OFFSET_ID + i), (int) &test_tapi_dev[i]);
      
      ret = TAPI_RemDev_PollPkts(&test_drv_ctx, &test_tapi_dev[i]);
      if (ret != IFX_SUCCESS)
      {
         printf("Error removing tapi dev id %d, %0X to polling list for pkts.\n "
               "(File: %s, line: %d)\n",
               (int) (TEST_OFFSET_ID + i), (int) &test_tapi_dev[i],
                __FILE__, __LINE__);
         return ret;
      }

      printf("TEST POLL: Remove tapi dev id %d, %0X to polling list for evts.\n",
            (int) (TEST_OFFSET_ID + i), (int) &test_tapi_dev[i]);
      
      ret = TAPI_RemDev_PollEvts(&test_drv_ctx, &test_tapi_dev[i]);
      if (ret != IFX_SUCCESS)
      {
         printf("Error removing tapi dev id %d, %0X to polling list for evts.\n "
               "(File: %s, line: %d)\n",
               (int) (TEST_OFFSET_ID + i), (int) &test_tapi_dev[i],
                __FILE__, __LINE__);
         return ret;
      }
   }

   /* Return count of all in list */
   i = 0;
   pList = pPollCtrl->pDevPollEvts;
   while (pList != IFX_NULL)
   {
      pList = pList->pNext;
      i++;
   };
   j = 0;
   pList = pPollCtrl->pDevPollPkts;
   while (pList != IFX_NULL)
   {
      pList = pList->pNext;
      j++;
   };
   printf("TEST POLL: List of pkts has %d device(s) and list of"
          " evts has %d device(s).\n", (int) j, (int) i);

   /* ---------------------------------------------------------------- 
                                 TEST 2: 
          List is has none of out test devices, try to get some devices and 
               we should not get them. Then add one and try to
                       get this one, should get it.
    */

   printf("\nTEST2: List has none of our test devices. Try to get one with\n");
   printf("       ID = -1, then one with our ID. Afterthat add one and try to\n");
   printf("       get this one, should get it, at the end remove it.\n");
   printf("---------------------------------------------------\n");

   /* Try to get tapi dev */
   temp_tapi_dev = TAPI_Poll_GetTapiDev(-1);
   if (temp_tapi_dev == IFX_NULL)
   {
      printf("TEST POLL: Could not get tapi dev with ID -1"
             " -> result is OK.\n");
   }
   else
   {
      printf("TEST POLL: Got tapi dev with ID -1"
             " -> result is ERROR.\n");
   }

   temp_tapi_dev = TAPI_Poll_GetTapiDev(TEST_OFFSET_ID + 4);
   if (temp_tapi_dev == IFX_NULL)
   {
      printf("TEST POLL: Could not get tapi dev with ID %d"
             " -> result is OK.\n", TEST_OFFSET_ID + 4);
   }
   else
   {
      printf("TEST POLL: Got tapi dev with ID %d"
             " -> result is ERROR.\n", TEST_OFFSET_ID + 4);
   }

   printf("TEST POLL: Adding tapi dev id %d, %0X to polling list for evts.\n",
         TEST_OFFSET_ID + 4, (int) &test_tapi_dev[4]);

   ret = TAPI_AddDev_PollEvts(&test_drv_ctx, &test_tapi_dev[4]);
   if (ret != IFX_SUCCESS)
   {
      printf("Error adding tapi dev id %d, %0X to polling list for evts. "
            "(File: %s, line: %d)\n",
            TEST_OFFSET_ID + 4, (int) &test_tapi_dev[4], __FILE__, __LINE__);
      return ret;
   }
   
   temp_tapi_dev = TAPI_Poll_GetTapiDev(TEST_OFFSET_ID + 4);
   if (temp_tapi_dev == IFX_NULL)
   {
      printf("TEST POLL: Could not get tapi dev with ID %d"
             " -> result is ERROR.\n", TEST_OFFSET_ID + 4);
   }
   else
   {
      printf("TEST POLL: Got tapi dev with ID %d"
             " -> result is OK.\n", TEST_OFFSET_ID + 4);
   }

   printf("TEST POLL: Remove tapi dev id %d, %0X from polling list for evts.\n",
          TEST_OFFSET_ID + 4, (int) &test_tapi_dev[4]);
      
   ret = TAPI_RemDev_PollEvts(&test_drv_ctx, &test_tapi_dev[4]);
   if (ret != IFX_SUCCESS)
   {
      printf("TEST POLL: Error removing tapi dev id %d, %0X from polling list for evts. "
             "(File: %s, line: %d)\n",
             TEST_OFFSET_ID + 4, (int) &test_tapi_dev[4], __FILE__, __LINE__);
      return ret;
   }
 
   /* ---------------------------------------------------------------- 
                                 TEST 3:
                    Try to remove the one that does not exists
    */

   printf("\nTEST3: Remove one that does not exists, should get error.\n");
   printf("---------------------------------------------------------\n");
   
   printf("TEST POLL: Remove tapi dev id %d, %0X from polling list for evts.\n",
          TEST_OFFSET_ID + 4, (int) &test_tapi_dev[4]);
      
   ret = TAPI_RemDev_PollEvts(&test_drv_ctx, &test_tapi_dev[4]);
   if (ret != IFX_SUCCESS)
   {
      printf("TEST POLL: Error removing tapi dev id %d, %0X from polling list for evts."
             " -> result is OK.\n",
             TEST_OFFSET_ID + 4, (int) &test_tapi_dev[4]);
      ret = IFX_SUCCESS;
   }
   else
   {
      printf("TEST POLL: Could remove tapi dev id %d, %0X from polling list for evts."
             " -> result is ERROR.\n",
             TEST_OFFSET_ID + 4, (int) &test_tapi_dev[4]);
      ret = IFX_ERROR;
   }

   /* ---------------------------------------------------------------- 
                                 TEST 4:
                        Try to add same device more times.
    */
    
   printf("\nTEST4: Adding same device more times.\n");
   printf("---------------------------------------------------------\n");

   printf("TEST POLL: Adding tapi dev id %d, %0X to polling list for evts - first time.\n",
         TEST_OFFSET_ID + 4, (int) &test_tapi_dev[4]);
    
   ret = TAPI_AddDev_PollEvts(&test_drv_ctx, &test_tapi_dev[4]);
   if (ret != IFX_SUCCESS)
   {
      printf("TEST POLL: Error adding tapi dev id %d, %0X to polling list for evts. "
            "(File: %s, line: %d)\n",
            TEST_OFFSET_ID + 4, (int) &test_tapi_dev[4], __FILE__, __LINE__);
      return ret;
   }

   printf("TEST POLL: Adding tapi dev id %d, %0X to polling list for evts - second time.\n",
         TEST_OFFSET_ID + 4, (int) &test_tapi_dev[4]);

   ret = TAPI_AddDev_PollEvts(&test_drv_ctx, &test_tapi_dev[4]);
   if (ret != IFX_SUCCESS)
   {
      printf("TEST POLL: Error adding tapi dev id %d, %0X to polling list for evts. "
            "Already exists -> result OK.\n",
            TEST_OFFSET_ID + 4, (int) &test_tapi_dev[4]);
      ret = IFX_SUCCESS;
   }
   else
   {
      printf("TEST POLL: Adding tapi dev id %d, %0X to polling list for evts. "
            "Already exists, added twice the same device -> result ERR.\n",
            TEST_OFFSET_ID + 4, (int) &test_tapi_dev[4]);
   
      ret = IFX_ERROR;
   }

   printf("TEST POLL: Remove tapi dev id %d, %0X from polling list for evts.\n",
          TEST_OFFSET_ID + 4, (int) &test_tapi_dev[4]);
      
   ret = TAPI_RemDev_PollEvts(&test_drv_ctx, &test_tapi_dev[4]);
   if (ret != IFX_SUCCESS)
   {
      printf("TEST POLL: Error removing tapi dev id %d, %0X from polling list for evts.\n",
             TEST_OFFSET_ID + 4, (int) &test_tapi_dev[4]);
      return ret;
   }


   /* ---------------------------------------------------------------- */

   
   printf("\nTEST POLL: Return count of devices in list at end of test.\n");
   /* Return count of all in list */
   i = 0;
   pList = pPollCtrl->pDevPollEvts;
   while (pList != IFX_NULL)
   {
      pList = pList->pNext;
      i++;
   };
   j = 0;
   pList = pPollCtrl->pDevPollPkts;
   while (pList != IFX_NULL)
   {
      pList = pList->pNext;
      j++;
   };
   printf("TEST POLL: List of pkts has %d device(s) and list of"
          " evts has %d device(s).\n", (int) j, (int) i);

   return IFX_SUCCESS;
}


#endif /* TAPI_POLL */
