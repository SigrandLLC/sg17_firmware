/*******************************************************************************

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

*******************************************************************************/

/**
   \file drv_tapi_kpi.c
   This file contains the implementation of the "Kernel Packet Interface" (KPI).
   The KPI is used to exchange packetised data with other drivers.
*/

/* ========================================================================== */
/*                                 Includes                                   */
/* ========================================================================== */
#include "drv_api.h"
#include "drv_tapi.h"
#include "drv_tapi_kpi.h"
#include <lib_bufferpool.h>
#ifdef LINUX
#include "sys_drv_kthread_linux.h"
#endif /* LINUX */


/* ========================================================================== */
/*                             Macro definitions                              */
/* ========================================================================== */

/* get group which is coded in the upper 4 bit of the channel parameter */
#define KPI_GROUP_GET(channel)    ((channel >> 12) & 0x000F)
/* get the channel number without the group number in the upper 4 bits */
#define KPI_CHANNEL_GET(channel)  (channel & 0x0FFF)


/* ========================================================================== */
/*                             Type definitions                               */
/* ========================================================================== */

/** Struct that holds all data for one KPI group. A group is the interface
    towards one specific driver. */
typedef struct
{
   /** egress fifo */
   FIFO_ID             *pEgressFifo;
   /** egress fifo protection */
   IFXOS_mutex_t        semProtectEgressFifo;
   /** semaphore that signals data in the egress fifo */
   IFXOS_mutex_t        semWaitOnEgressFifo;
   /** ingress fifo */
   FIFO_ID             *pIngressFifo;
   /** ingress fifo protection */
   IFXOS_mutex_t        semProtectIngressFifo;
   /** congestion state of the ingress fifo */
   IFX_boolean_t        bIngressFifoCongested;
   /** Map from KPI channel to the corresponding TAPI channel.
       The KPI channel is the index to this map. */
   TAPI_CHANNEL        *channel_map[IFX_TAPI_KPI_MAX_CHANNEL_PER_GROUP];
   /** Map from KPI channel to the stream that packets belong to. */
   IFX_TAPI_KPI_STREAM_t stream_map[IFX_TAPI_KPI_MAX_CHANNEL_PER_GROUP];
} IFX_TAPI_KPI_GROUP_t;

/** Struct that is put as an element into the fifos and keeps the data
    together with the channel information. The fields for the channels
    are used depending on the direction. In ingress direction only the
    TAPI_CHANNEL is valid and the IFX_TAPI_KPI_CH_t is undefined. In
    egress direction only the IFX_TAPI_KPI_CH_t is valid and the
    TAPI_CHANNEL field is undefined. */
typedef struct
{
   /** KPI channel this buffer is sent on (egress direction) */
   IFX_TAPI_KPI_CH_t    nKpiCh;
   /** TAPI channel this buffer is for (ingress direction) */
   TAPI_CHANNEL        *pTapiCh;
   /** Pointer to a buffer from lib-bufferpool with the payload data */
   void                *pBuf;
   /** Reserved for future use. Pointer to first data in the buffer */
   IFX_void_t          *pData ;
   /** Length of data in the buffer counted in bytes  */
   IFX_uint32_t         nDataLength;
   /** Stream that this packet is for */
   IFX_TAPI_KPI_STREAM_t nStream;
} IFX_TAPI_KPI_FIFO_ELEM_t;


/* ========================================================================== */
/*                             Global variables                               */
/* ========================================================================== */

/** Array with all KPI group specific data */
static IFX_TAPI_KPI_GROUP_t  kpi_group[IFX_TAPI_KPI_MAX_GROUP];
/** One semaphore to signal data in any ingress fifo */
static IFXOS_mutex_t         semWaitOnIngressFifo;
/** Handle of the bufferpool used to allocate wrapper buffers from */
static BUFFERPOOL           *wrapperBufferpool;
#ifdef LINUX
/** Hold information of the ingress worker thread */
static kthread_t             ingressThread;
#ifdef KPI_TESTLOOP
static kthread_t             loopbackThread;
#endif /* KPI_TESTLOOP */
#endif /* LINUX */

/* ========================================================================== */
/*                           Function prototypes                              */
/* ========================================================================== */
#ifdef LINUX
static IFX_int32_t ifx_tapi_KPI_LinuxIngressThread (kthread_t *pThread);
#ifdef KPI_TESTLOOP
static IFX_int32_t ifx_tapi_KPI_LinuxTestloopThread (kthread_t *pThread);
#endif /* KPI_TESTLOOP */
#endif /* LINUX */
static IFX_int32_t ifx_tapi_KPI_IngressThread (void);


/* ========================================================================== */
/*                         Function implementation                            */
/* ========================================================================== */

/**
   Initialise the Kernel Packet Interface (KPI)

   \return Return values are defined within the \ref IFX_return_t definition
   - IFX_SUCCESS  in case of success
   - IFX_ERROR if operation failed
*/
IFX_return_t IFX_TAPI_KPI_Init (IFX_void_t)
{
   IFX_uint8_t  i;

   /* Loop over all groups in the KPI */
   for (i = 0; i < IFX_TAPI_KPI_MAX_GROUP; i++)
   {
      /* set structure defined to zero */
      memset (&(kpi_group[i]), 0x00, sizeof(IFX_TAPI_KPI_GROUP_t));
      /* create semaphores for protecting the fifos */
      IFXOS_MutexInit (kpi_group[i].semProtectEgressFifo);
      IFXOS_MutexInit (kpi_group[i].semProtectIngressFifo);
      /* create semaphore to signal data in the egress fifo */
      IFXOS_MutexInit (kpi_group[i].semWaitOnEgressFifo);
      /* inital state of the semaphore should be locked so take it */
      IFXOS_MutexLock (kpi_group[i].semWaitOnEgressFifo);
      /* create data fifos for ingress and egress direction */
      kpi_group[i].pEgressFifo =  fifoInit (IFX_TAPI_KPI_EGRESS_FIFO_SIZE);
      kpi_group[i].pIngressFifo = fifoInit (IFX_TAPI_KPI_INGRESS_FIFO_SIZE);
   }

   /* create semaphore to signal data is in the ingress fifos */
   IFXOS_MutexInit (semWaitOnIngressFifo);
   /* inital state of the semaphore should be locked so take it */
   IFXOS_MutexLock (semWaitOnIngressFifo);

   /* create a bufferpool for wrapper structs passed in the fifos.
      The number of elements in the pool is set so that all slots
      in all fifos can be filled. The buffer is only allocated if the
      init is called for the first time because we cannot delete the
      buffer in the cleanup. So we keep it. */
   if (!wrapperBufferpool)
      wrapperBufferpool = bufferPoolInit(sizeof(IFX_TAPI_KPI_FIFO_ELEM_t),
                                         (IFX_TAPI_KPI_EGRESS_FIFO_SIZE +
                                          IFX_TAPI_KPI_INGRESS_FIFO_SIZE) *
                                            IFX_TAPI_KPI_MAX_GROUP, 10);

#ifdef LINUX
   /* start a task working on the ingress queues */
   start_kthread((void (*)(kthread_t *))ifx_tapi_KPI_LinuxIngressThread,
                 &ingressThread, "kpi_in" );

#ifdef KPI_TESTLOOP
   /* LOOPBACK TEST task */
   start_kthread((void (*)(kthread_t *))ifx_tapi_KPI_LinuxTestloopThread,
                 &loopbackThread, "kpi_loop" );
#endif /* KPI_TESTLOOP */
#endif /* LINUX */

   return IFX_SUCCESS;
}


/**
   Clean-up the Kernel Packet Interface (KPI)

   \return none
   \remarks
   This code is cannot clean up the buffer pool because the library does not
   support this. So the buffer is kept and used again when the KPI is
   initialised again.
*/
IFX_void_t IFX_TAPI_KPI_Cleanup (IFX_void_t)
{
   IFX_uint8_t  i;

   /* stop the task working on the ingress queues */
#ifdef LINUX
   stop_kthread(&ingressThread);
#ifdef KPI_TESTLOOP
   stop_kthread(&loopbackThread);
#endif /* KPI_TESTLOOP */
#endif /* LINUX */

   /* Loop over all groups in the KPI */
   for (i = 0; i < IFX_TAPI_KPI_MAX_GROUP; i++)
   {
      /* delete semaphores for protecting the fifos */
      IFXOS_MutexDelete (kpi_group[i].semProtectEgressFifo);
      IFXOS_MutexDelete (kpi_group[i].semProtectIngressFifo);
      /* delete semaphore to signal data in the egress fifo */
      IFXOS_MutexDelete (kpi_group[i].semWaitOnEgressFifo);
      /* flush the data fifos for ingress and egress direction */

      /**\todo: flush the data from the fifos (locking?) */

      /* delete data fifos for ingress and egress direction */
      fifoFree (kpi_group[i].pEgressFifo);
      fifoFree (kpi_group[i].pIngressFifo);
   }

   IFXOS_MutexDelete (semWaitOnIngressFifo);

   /* free the buffer pool for the wrapper structs. */
   bufferPoolFree(wrapperBufferpool);
}


/**
   Sleep until data is available for reading with IFX_TAPI_KPI_ReadData().

   \param  nKpiGroup    KPI group to wait on for new data.

   \return
   Return values are defined within the \ref IFX_return_t definition
   - IFX_SUCCESS  in case of success
   - IFX_ERROR    if operation failed
*/
IFX_return_t IFX_TAPI_KPI_WaitForData( IFX_TAPI_KPI_CH_t nKpiGroup )
{
   /* Get the KPI-group number */
   nKpiGroup = KPI_GROUP_GET(nKpiGroup);
   /* Reject group values which are out of configured range */
   if ((nKpiGroup == 0) || (nKpiGroup > IFX_TAPI_KPI_MAX_GROUP))
      return IFX_ERROR;
   /* Take the semaphore - this is blocking until data is available or
      a signal is sent to the process. */
   if (IFXOS_MutexLockInterruptible(
         kpi_group[nKpiGroup-1].semWaitOnEgressFifo) == 0)
   {
      return IFX_SUCCESS;
   }
   else
   {
      /* unsuccessful or aborted */
      return IFX_ERROR;
   }
}


/**
   Read function for KPI clients to read a packet from TAPI KPI.

   \param  nKpiGroup    KPI group that should provide data.
   \param  *nKpiChannel Returns the KPI channel number within the given
                        group where the packet was received.
   \param  **pPacket    Returns a pointer to a bufferpool element with the
                        received data.
   \param  *nPacketLength  Returns the length of the received data.
   \param  *nMore       Signals that more packets are ready to be read.

   \return
   Returns the number of data bytes successfully read or IFX_ERROR otherwise.

   \remarks
   The ownership of the returned bufferpool element is passed to the client
   calling this interface. It is responsibility of the client to free this
   element by calling bufferPoolPut() after having processed the data.
*/
IFX_int32_t IFX_TAPI_KPI_ReadData( IFX_TAPI_KPI_CH_t nKpiGroup,
                                   IFX_TAPI_KPI_CH_t *nKpiChannel,
                                   void **pPacket,
                                   IFX_uint32_t *nPacketLength,
                                   IFX_uint8_t *nMore)
{
   IFX_uint32_t    lock;
   IFX_TAPI_KPI_FIFO_ELEM_t *pElem;

   /* Get the KPI-group number */
   nKpiGroup = KPI_GROUP_GET(nKpiGroup);
   /* Reject group values which are out of the configured range */
   if ((nKpiGroup == 0) || (nKpiGroup > IFX_TAPI_KPI_MAX_GROUP))
      return IFX_ERROR;
   /* Adjust group values from channel notation to internal representation */
   nKpiGroup--;

   /* The read access to the fifo is protected in two ways:
       First it is protected from concurrent reads with this function
       second it is protected from writing of new data in irq context. */

   /* take protection semaphore */
   if (!IFXOS_IN_INTERRUPT())
      IFXOS_MutexLock(kpi_group[nKpiGroup].semProtectEgressFifo);
   /* global irq lock */
   IFXOS_LOCKINT(lock);
   /* read element from fifo */
    pElem = fifoGet (kpi_group[nKpiGroup].pEgressFifo, NULL);
   /* set the more flag */
   *nMore = fifoEmpty(kpi_group[nKpiGroup].pEgressFifo) ? 0 : 1;
   /* global irq unlock */
   IFXOS_UNLOCKINT(lock);

   /* when there was data in the fifo return values and discard wrapper */
   if (pElem != NULL)
   {
      /* store return values in the parameters */
      *nKpiChannel = pElem->nKpiCh;
      *pPacket = pElem->pBuf;
      *nPacketLength = pElem->nDataLength;
      /* release the wrapping structure for the data buffer */
      if (bufferPoolPut(pElem) != IFX_SUCCESS)
      {
         /* This should never happen! */
         TRACE (TAPI_DRV, DBG_LEVEL_HIGH, ("\nBuffer put-back error\n"));
      }
   }

   /* release protection semaphore */
   if (!IFXOS_IN_INTERRUPT())
      IFXOS_MutexUnlock(kpi_group[nKpiGroup].semProtectEgressFifo);

   /* pElem is still valid but the buffer it points to is not valid any more */
   return (pElem != NULL) ? *nPacketLength : IFX_ERROR;
}


/**
   Write function for KPI clients to write a packet to TAPI KPI.

   \param  nKpiChannel  KPI channel the data is written to.
   \param  *pPacket     Pointer to a bufferpool element with the data to be
                        written.
   \param  nPacketLength  Length of the data to be written.

   \return
   Returns the number of data bytes successfully written or IFX_ERROR otherwise.

   \remarks
   The ownership of the bufferpool element is only passed to the KPI if this
   call is successful. When this write fails the buffer still belongs to the
   caller and he has to discard or write it again.
*/
IFX_int32_t IFX_TAPI_KPI_WriteData( IFX_TAPI_KPI_CH_t nKpiChannel,
                                    void *pPacket,
                                    IFX_uint32_t nPacketLength)
{
   IFX_TAPI_KPI_FIFO_ELEM_t *pElem;
   TAPI_CHANNEL             *pTapiCh;
   IFX_uint32_t              lock;
   IFX_int32_t               ret;

   /* Get the KPI-group number */
   IFX_TAPI_KPI_CH_t nKpiGroup = KPI_GROUP_GET(nKpiChannel);
   /* reject group values which are out of the configured range */
   if ((nKpiGroup == 0) || (nKpiGroup > IFX_TAPI_KPI_MAX_GROUP))
      return IFX_ERROR;
   /* strip the group from the channel parameter */
   nKpiChannel = KPI_CHANNEL_GET(nKpiChannel);
   /* reject channel values which are out of the configured range */
   if (nKpiChannel >= IFX_TAPI_KPI_MAX_CHANNEL_PER_GROUP)
      return IFX_ERROR;
   /* Adjust group values from channel notation to internal representation */
   nKpiGroup--;

   /* lookup the tapi channel associated with the kpi channel */
   pTapiCh = kpi_group[nKpiGroup].channel_map[nKpiChannel];
   /* abort if no channel association exists */
   if (pTapiCh == IFX_NULL)
      return IFX_ERROR;

   /* allocate a wrapper struct from the bufferpool for wrappers */
   pElem = bufferPoolGet (wrapperBufferpool);
   /* abort if buffer for the wrapping struct cannot be allocated */
   if (pElem == NULL)
      return IFX_ERROR;

   /* The write access to the fifo is protected in two ways:
       First it is protected from concurrent reads by the ingress thread
       second it is protected from writing of new data in irq context. */

   /* store the tapi channel and pointer to the data buffer in the wrapper */
   pElem->pTapiCh = pTapiCh;
   pElem->nKpiCh = nKpiChannel;
   pElem->pBuf = pElem->pData = pPacket;
   pElem->nDataLength = nPacketLength;

   /* if ! in irq take protection semaphore */
   if (!IFXOS_IN_INTERRUPT())
      IFXOS_MutexLock(kpi_group[nKpiGroup].semProtectIngressFifo);

   /* here we are protected from changes from the ioctl so copy this data */
   pElem->nStream = kpi_group[nKpiGroup].stream_map[nKpiChannel];

   /* global irq lock */
   IFXOS_LOCKINT(lock);
   /* store data to fifo */
   ret = fifoPut(kpi_group[nKpiGroup].pIngressFifo, (IFX_void_t *)pElem, 0);
   /* global irq unlock */
   IFXOS_UNLOCKINT(lock);
   /* if ! in irq release protection semaphore */
   if (!IFXOS_IN_INTERRUPT())
      IFXOS_MutexUnlock(kpi_group[nKpiGroup].semProtectIngressFifo);

   /* if putting to fifo succeeded set flag that there is data in one of
      the ingress fifos otherwise cleanup by releasing the wrapper buffer */
   if (ret == IFX_SUCCESS)
   {
      /* set flag that there is data in one of the ingress fifos */
      IFXOS_MutexUnlock(semWaitOnIngressFifo);
      /* fifo state is: not congested */
      kpi_group[nKpiGroup].bIngressFifoCongested = IFX_FALSE;
   }
   else
   {
      /* This case happens if the ingress thread is not fast enough to read
         the data from the fifo and put it into the firmware data mailbox.
         Send an event to the application to notify that we were too slow. */
      /* Report congestion of the fifo to the application - but only once */
      if (kpi_group[nKpiGroup].bIngressFifoCongested == IFX_FALSE) {
         IFX_TAPI_EVENT_t  tapiEvent;
         memset(&tapiEvent, 0, sizeof(IFX_TAPI_EVENT_t));
         tapiEvent.id = IFX_TAPI_EVENT_KPI_INGRESS_FIFO_FULL;
         IFX_TAPI_Event_Dispatch(pTapiCh, &tapiEvent);
         /* fifo state is: congested */
         kpi_group[nKpiGroup].bIngressFifoCongested = IFX_TRUE;
      }
      if (bufferPoolPut(pElem) != IFX_SUCCESS)
      {
         /* This should never happen! */
         TRACE (TAPI_DRV, DBG_LEVEL_HIGH, ("\nBuffer put-back error\n"));
      }
      TRACE (TAPI_DRV, DBG_LEVEL_LOW,
             ("INFO: KPI-group 0x%X ingress fifo full\n", nKpiGroup+1));
   }

   return (ret == IFX_SUCCESS) ? nPacketLength : IFX_ERROR;
}


/**
   Function to put a packet from irq context into an KPI egress fifo.

   \param  pChannel     Handle to TAPI_CHANNEL structure.
   \param  stream       Stream Type id.
   \param  *pPacket     Pointer to a bufferpool element with the data to be
                        written.
   \param  nPacketLength  Length of the data to be written.

   \return
   Return values are defined within the \ref IFX_return_t definition:
   - IFX_SUCCESS  if buffer was successfully sent
   - IFX_ERROR    if buffer was not sent.

   \remarks
   In case of error the caller still owns the buffer and has to take care to it.
*/
IFX_return_t irq_IFX_TAPI_KPI_PutToEgress(TAPI_CHANNEL *pChannel,
                                          IFX_TAPI_KPI_STREAM_t stream,
                                          void *pPacket,
                                          IFX_uint32_t nPacketLength)
{
   IFX_TAPI_KPI_STREAM_SWITCH *pStreamSwitch = &pChannel->pKpiStream[stream];
   IFX_TAPI_KPI_FIFO_ELEM_t   *pElem;
   IFX_uint32_t                lock;
   IFX_int32_t                 ret = IFX_ERROR;

   /* get a buffer for the wrapping struct from the bufferpool */
   pElem = bufferPoolGet (wrapperBufferpool);

   if (pElem)
   {
      /* fill the wrapping struct with data */
      pElem->nKpiCh = pStreamSwitch->nKpiCh;
      pElem->pBuf = pElem->pData = pPacket;
      pElem->nDataLength = nPacketLength;

      /* protect fifo access from concurrent writes by other drivers in
         irq context by locking irq globally for just the fifo operation. */
      /* global irq lock */
      IFXOS_LOCKINT(lock);
      /* store data to fifo */
      ret = fifoPut(pStreamSwitch->pEgressFifo, (IFX_void_t *)pElem, 0);
      /* global irq unlock */
      IFXOS_UNLOCKINT(lock);
      if (ret == IFX_SUCCESS)
      {
         /* give the signalling semaphore to indicate data in the fifo */
         IFXOS_MutexUnlock(kpi_group[KPI_GROUP_GET(pStreamSwitch->nKpiCh)-1].
                           semWaitOnEgressFifo);
      }
      else
      {
         /* This case is normal when the client for this KPI group fails to get
            the data from the fifo. So do not be too noisy about this. */
         /* if putting to fifo failed release wrapper buffer */
         if (bufferPoolPut(pElem) != IFX_SUCCESS)
         {
            /* This should never happen! */
            TRACE (TAPI_DRV, DBG_LEVEL_HIGH, ("\nBuffer put-back error\n"));
         }
         TRACE (TAPI_DRV, DBG_LEVEL_LOW,
                ("INFO: KPI-ch 0x%X egress fifo full\n", pElem->nKpiCh));
      }
   }

   /* If putting to fifo failed IFX_ERROR is returned to the caller.
      The data buffer then still belongs to the caller who has to free it. */
   return (ret == IFX_SUCCESS) ? IFX_SUCCESS : IFX_ERROR;
}


#ifdef LINUX
/**
   Linux wrapper to start the ingress thread.

   \param  *kthread     Pointer to linux kernel thread struct.

   \return IFX_SUCCESS  In all cases.

   \remarks
   This thread runs in an endless loop and never returns.
*/
static IFX_int32_t ifx_tapi_KPI_LinuxIngressThread (kthread_t *pThread)
{
   IFX_int32_t          ret = IFX_SUCCESS;

   /* setup the thread environment */
   init_kthread(pThread);

   /* Call the worker function repeatedly while we are not asked to terminate */
   while ((pThread->terminate != 1) && (ret == IFX_SUCCESS))
   {
      ret = ifx_tapi_KPI_IngressThread();
   }

   /* cleanup the thread before leaving */
   exit_kthread(pThread);

   return IFX_SUCCESS;
}


#ifdef KPI_TESTLOOP
/**
   Linux testloop thread.

   This thread loops all data on KPI_GROUP1 from odd to even channel numbers
   and vice versa.

   \param  *pThread     Pointer to linux kernel thread struct.

   \return \ref IFX_SUCCESS  In all cases.

   \remarks
   This function runs as a thread runs in an endless loop until terminated.
   This thread is just meant for test purposes.
*/
static IFX_int32_t ifx_tapi_KPI_LinuxTestloopThread (kthread_t *pThread)
{
   IFX_TAPI_KPI_CH_t         channel;
   void                     *data;
   IFX_uint32_t              data_length;
   IFX_uint8_t               more_flag;
   IFX_int32_t               ret;

   /* setup the thread environment */
   init_kthread(pThread);

   /* loop while we are not asked to terminate */
   while ((pThread->terminate != 1) &&
          (IFX_TAPI_KPI_WaitForData( IFX_TAPI_KPI_GROUP1 ) == IFX_SUCCESS))
   {
      ret = IFX_TAPI_KPI_ReadData( IFX_TAPI_KPI_GROUP1, &channel,
                                   &data, &data_length, &more_flag );
      if (ret < 0)
      {
         TRACE (TAPI_DRV, DBG_LEVEL_HIGH, ("LOOPBACK failed to read data\n"));
      }
      else
      {
         /* toggle the LSB in the channel number for sending */
         channel = channel ^ 0x0001;
         ret = IFX_TAPI_KPI_WriteData( channel, data, data_length );
         if (ret < 0)
         {
            /* On write error we still own the buffer so we have to handle it.
               Here we just drop the data. */
            if (bufferPoolPut(data) != IFX_SUCCESS)
            {
               /* This should never happen! */
               TRACE (TAPI_DRV, DBG_LEVEL_HIGH, ("\nBuffer put-back error\n"));
            }
            TRACE (TAPI_DRV, DBG_LEVEL_HIGH, ("LOOPBACK failed to write data\n"));
         }
      }
   }

   /* cleanup the thread before leaving */
   exit_kthread(pThread);

   return IFX_SUCCESS;
}
#endif /* KPI_TESTLOOP */
#endif /* LINUX */


/**
   Function to be executed from a thread to serve all ingress fifos of all
   KPI groups.

   This function sleeps until data in an ingress fifo is signalled with a
   semaphore. Upon wakeup the ingress fifos are searched in a round-robin
   manner for data and the first data found is written to the FW downstream
   mailbox. This function exits after having processed one packet and needs
   to be called again while packets should be processed. This is typically
   done by some thread in an endless loop.
*/

static IFX_int32_t ifx_tapi_KPI_IngressThread (void)
{
   IFX_TAPI_KPI_FIFO_ELEM_t *pElem;
   static IFX_uint8_t        nLastGroup = 0;
   IFX_uint8_t               nThisGroup;
   IFX_uint32_t              lock;

   /* wait until some data is available */
   if (IFXOS_MutexLockInterruptible(semWaitOnIngressFifo) == 0)
   {
      /* find a group with data in the ingress fifo
         A round-robin policy is used to give all groups a chance. We exit
         as soon as we found and processed data from one group or after
         checking all groups once if no group contains data. */
      nThisGroup = nLastGroup;
      do
      {
         /* increment group and take care of the wraparound */
         nThisGroup++;
         nThisGroup = (nThisGroup >= IFX_TAPI_KPI_MAX_GROUP) ?  0 : nThisGroup;

         /* check if there is data in the fifo of this group */
         if (!fifoEmpty(kpi_group[nThisGroup].pIngressFifo))
         {
            /* protect fifo get access from other tasks and any irq's
               Locking individual irq's is too costly so we lock globally */
            IFXOS_MutexLock(kpi_group[nThisGroup].semProtectIngressFifo);
            IFXOS_LOCKINT(lock);
            /* read element from fifo */
            pElem = fifoGet (kpi_group[nThisGroup].pIngressFifo, NULL);
            IFXOS_UNLOCKINT(lock);
            IFXOS_MutexUnlock(kpi_group[nThisGroup].semProtectIngressFifo);

            if (pElem)
            {
               /* we got data from the fifo */
               TAPI_CHANNEL *pTapiCh = pElem->pTapiCh;
               IFX_TAPI_DRV_CTX_t *pDrvCtx =
                  (IFX_TAPI_DRV_CTX_t*) pTapiCh->pTapiDevice->pDevDrvCtx;
               IFX_TAPI_LL_CH_t* pCh = pTapiCh->pLLChannel;
               IFX_int32_t size;

               /* write data to the mailbox using LL function */
               if (pDrvCtx->KpiWrite)
               {
                  size = pDrvCtx->KpiWrite (pCh, pElem->pBuf,
                                            pElem->nDataLength, pElem->nStream);
                  if (size < 0)
                  {
                     /* The mailbox congestion event was already sent by the
                        lower layer so no need to do it here again. */
                     /* writing to mailbox failed - discard data */
                     if (bufferPoolPut(pElem->pBuf) != IFX_SUCCESS)
                     {
                        /* This should never happen! */
                        TRACE (TAPI_DRV, DBG_LEVEL_HIGH,
                               ("\nBuffer put-back error\n"));
                     }
                  }
               }
               else
               {
                  TRACE(TAPI_DRV, DBG_LEVEL_LOW,
                        ("TAPI_DRV: pDrvCtx->LL_KpiWrite is NULL !!!\n\r"));
               }

               /* return buffer with wrapper struct back to pool */
               if (bufferPoolPut(pElem) != IFX_SUCCESS)
               {
                  /* This should never happen! */
                  TRACE (TAPI_DRV, DBG_LEVEL_HIGH,
                         ("\nBuffer put-back error\n"));
               }
            }
         }
      } while (nThisGroup != nLastGroup);

      /* save the last checked group as a starting point for the next round */
      nLastGroup = nThisGroup;
   }
   else
   {
      /* Return to the caller that getting the semaphore was interrupted. */
      return IFX_ERROR;
   }

   return IFX_SUCCESS;
}


/**
   Handler for the ioctl IFX_TAPI_KPI_CH_CFG_SET.

   This function sets the internal data structures to associate a
   TAPI packet stream with a KPI channel.

   \param  pChannel     Handle to TAPI_CHANNEL structure.
   \param  *pCfg        Pointer to \ref IFX_TAPI_KPI_CH_CFG_t containing the
                        configuration.

   \return
   Return values are defined within the \ref IFX_return_t definition
   - IFX_SUCCESS  if configuration was successfully set
   - IFX_ERROR    on invalid values in the configuration
*/
IFX_int32_t IFX_TAPI_KPI_ChCfgSet (TAPI_CHANNEL *pChannel,
                                   IFX_TAPI_KPI_CH_CFG_t const *pCfg)
{
   IFX_TAPI_KPI_CH_t         nKpiGroup,
                             nKpiChannel;
   IFX_uint32_t              lock;

   /* Get the KPI-group number */
   nKpiGroup = KPI_GROUP_GET(pCfg->nKpiCh);
   /* reject group values which are out of the configured range */
   if (nKpiGroup > IFX_TAPI_KPI_MAX_GROUP)
      return IFX_ERROR;
   /* strip the group from the channel parameter */
   nKpiChannel = KPI_CHANNEL_GET(pCfg->nKpiCh);
   /* reject channel values which are out of the configured range */
   if (nKpiChannel >= IFX_TAPI_KPI_MAX_CHANNEL_PER_GROUP)
      return IFX_ERROR;
   /* reject source stream identifiers which are out of range */
   if (pCfg->nStream >= IFX_TAPI_KPI_STREAM_MAX)
      return IFX_ERROR;

   /* Currently no flushing of fifos is done. To do this we had to lock global
      interrupts during this time. This is not desirable because it will take
      quite some time to release all the buffers possibly in the fifo.
      But because the data needed to deliver the packet is stored in the
      wrapping buffer when the packet is received we still get the packets
      delivered to the correct destination. */

   /* update the stream switch struct to indicate the new target */
   /* take protection semaphore */
   IFXOS_MutexLock(pChannel->semTapiChDataLock);
   /* global irq lock */
   IFXOS_LOCKINT(lock);
   /* Set the stream switch struct to point to the new KPI channel.
      Group 0 is reserved for sending streams to the application. In this
      group force all channels to 0 to ease checks when using. */
   pChannel->pKpiStream[pCfg->nStream].nKpiCh =
      (nKpiGroup > 0) ? pCfg->nKpiCh : 0;
   pChannel->pKpiStream[pCfg->nStream].pEgressFifo =
      (nKpiGroup > 0) ? kpi_group[nKpiGroup-1].pEgressFifo : IFX_NULL;
   /* global irq unlock */
   IFXOS_UNLOCKINT(lock);
   /* release protection semaphore */
   IFXOS_MutexUnlock(pChannel->semTapiChDataLock);

   if (nKpiGroup > 0)
   {
      /* set tapi channel reference in kpi channel reference array */
      /* take protection semaphore */
      IFXOS_MutexLock(kpi_group[nKpiGroup-1].semProtectIngressFifo);
      /* global irq lock */
      IFXOS_LOCKINT(lock);
      kpi_group[nKpiGroup-1].channel_map[nKpiChannel] = pChannel;
      kpi_group[nKpiGroup-1].stream_map[nKpiChannel] = pCfg->nStream;
      /* global irq unlock */
      IFXOS_UNLOCKINT(lock);
      /* release protection semaphore */
      IFXOS_MutexUnlock(kpi_group[nKpiGroup-1].semProtectIngressFifo);
   }

   return IFX_SUCCESS;
}

/**
   Retrieve the KPI Channel number of a given stream on a given TAPI Channel
   \param  pChannel     Handle to TAPI_CHANNEL structure.
   \param  stream       Stream Type id.

   \return KPI Channel number
*/
IFX_TAPI_KPI_CH_t IFX_TAPI_KPI_ChGet(TAPI_CHANNEL *pChannel,
                                     IFX_TAPI_KPI_STREAM_t stream)
{
   return pChannel->pKpiStream[stream].nKpiCh;
}

