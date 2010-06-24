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
   Module      : drv_tapi_init.c
   Date        : 2006/06/06
   Description : Common initialization functions for the TAPI Driver

   \remark

******************************************************************************/

/* ============================= */
/* Includes                      */
/* ============================= */

#include "drv_api.h"
#include "drv_tapi.h"
#include "drv_tapi_api.h"
#include "drv_tapi_errno.h"
#include "drv_tapi_stream.h"

/* ============================= */
/* Local Macros & Definitions    */
/* ============================= */

/* ============================= */
/* Global variable definition    */
/* ============================= */
/* Maximum number of Low level TAPI devices which can register */
IFX_TAPI_HL_DRV_CTX_t gHLDrvCtx [TAPI_MAX_LL_DEVICES];

/* ============================= */
/* Local function declaration    */
/* ============================= */

/* ============================= */
/* Local variable definition     */
/* ============================= */

static IFX_uint32_t nGlobalTapiID = 0;

/* ============================= */
/* Local function definition     */
/* ============================= */

/**
   Check if driver with this major number is registered.

\param nMajor - {major} number of driver

\return IFX_SUCCESS if driver is registered otherwise IFX_ERROR
*/
IFX_return_t IFX_TAPI_Is_Device_Registered(IFX_int32_t nMajor)
{
   IFX_int32_t i = 0;


   for (i = 0; i < TAPI_MAX_LL_DEVICES; i++)
   {
      if (gHLDrvCtx[i].pDrvCtx != IFX_NULL)
      {
         if (nMajor == gHLDrvCtx[i].pDrvCtx->majorNumber)
         {
            return IFX_SUCCESS;
         }
      }
   }

   return IFX_ERROR;
}

/* ============================= */
/* Global function definition    */
/* ============================= */


/**
   TAPI initialisation upon loading the driver.

   \return
   Returns IFX_ERROR in case of an error, otherwise returns IFX_SUCCESS.
*/
IFX_int32_t IFX_TAPI_On_Driver_Load(IFX_void_t)
{
   return IFX_TAPI_Event_On_Driver_Load();
}

/**
   TAPI initialisation upon unloading the driver.
*/
IFX_void_t IFX_TAPI_On_Driver_Unload(IFX_void_t)
{
   IFX_TAPI_Event_On_Driver_Unload();
}

/**
   TAPI channel structure initialization
\param
   pTapiDev     - Pointer to TAPI device structure
\param
   pDrvCtx      - Pointer to device driver context
\param
   dev_num       - Device number
\return
   Returns IFX_ERROR in case of an error, otherwise returns IFX_SUCCESS.
\remarks
   Initializes all needed fifos, event queues  and sets all needed
   members.
*/
IFX_return_t TAPI_Prepare_Ch (TAPI_DEV* pTapiDev,
                              IFX_TAPI_DRV_CTX_t *pDrvCtx, IFX_uint32_t dev_num)
{
   IFX_uint8_t             nCh;
   IFX_uint32_t            nMaxChannel = pDrvCtx->maxChannels;
   TAPI_CHANNEL            *pTapiCh    = IFX_NULL;
   IFX_TAPI_LL_CH_t  *pCh        = IFX_NULL;
   IFX_int32_t            ret;

   if (pTapiDev == IFX_NULL)
   {
      return IFX_ERROR;
   }

   /* initialize the structure for the channel connections */
   for (nCh = 0; nCh < nMaxChannel; nCh++)
   {
      pTapiCh = &(pTapiDev->pTapiChanelArray[nCh]);
      /* initialize the pointer to the tapi device structure */
      pTapiCh->pTapiDevice = pTapiDev;
      /* initialize each channel with it's channel number */
      pTapiCh->nChannel = nCh;
      /* mask all exceptions -> disable exceptions */
      pTapiCh->TapiMiscData.nExceptionMask.Status = 0xFFFFFFFF;
      /* Initialise the high level channel access mutex */
      IFXOS_MutexInit (pTapiCh->semTapiChSingleIoctlAccess);
      /* Initialise the event dispatcher */
      if (IFX_TAPI_EventDispatcher_Init(pTapiCh) == IFX_ERROR)
         return IFX_ERROR;
#ifdef TAPI_CID
      /* allocate memory for caller id receiver data */
      {
         IFX_void_t *pEnd;
         IFX_void_t *pStart;

         pStart = IFXOS_MALLOC(IFX_TAPI_CID_RX_FIFO_SIZE *
                               sizeof (IFX_TAPI_CID_RX_DATA_t));
         if (pStart == IFX_NULL)
         {
            return IFX_ERROR;
         }
         pEnd = ((IFX_TAPI_CID_RX_DATA_t *)pStart) +
                  IFX_TAPI_CID_RX_FIFO_SIZE - 1;
         Fifo_Init(&((pTapiDev->pTapiChanelArray + nCh)->
                   TapiCidRx.TapiCidRxFifo),
                   pStart, pEnd, sizeof(IFX_TAPI_CID_RX_DATA_t));
      }
#endif
#ifdef TAPI_PACKET
      if (pDrvCtx->Read != IFX_NULL)
      /* Initialize upstream voice fifo */
      TAPI_InitUpStreamFifo(pTapiCh);
#endif /* TAPI_PACKET */
   } /* for */

   /* Set the number of maximum channels */
   pTapiDev->nMaxChannel = nMaxChannel;

   for (nCh=0; nCh < pDrvCtx->maxChannels ; nCh++)
   {
      pTapiCh  = &(pTapiDev->pTapiChanelArray[nCh]);

      if (pDrvCtx->Prepare_Ch == IFX_NULL)
      {
         TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
               ("TAPI_DRV: pDrvCtx->LL_Prepare_Ch is NULL !!!\n\r"));
         return IFX_ERROR;
      }
      pCh = pDrvCtx->Prepare_Ch (pTapiCh, dev_num, nCh);
      if (pCh == IFX_NULL)
      {
         TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
               ("TAPI_DRV: Couldnt allocate the low level driver\n\r"));
         return IFX_ERROR;
      }
      /* select wait queue for reading data */
      IFXOS_Init_WakeList (pTapiCh->wqRead);
      /* Initialization time : No Fax data request. */
      pTapiCh->bFaxDataRequest = IFX_FALSE;

      /* allocate timer and status for tone resources */
      pTapiCh->pToneRes = IFXOS_MALLOC(sizeof(TAPI_TONERES) * TAPI_TONE_MAXRES);
      memset (pTapiCh->pToneRes, 0, sizeof(TAPI_TONERES) * TAPI_TONE_MAXRES);

      /* Store the coresponding low level Channel pointer */
      pTapiCh->pLLChannel = (IFX_TAPI_LL_CH_t*) pCh;
      pTapiCh->TapiOpControlData.nLineMode = IFX_TAPI_LINE_FEED_DISABLED;
      if (pTapiCh->nInUse == 0)
      {
         if (pDrvCtx->UpdateChMember == IFX_NULL)
         {
            TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
                  ("TAPI_DRV:pDrvCtx->LL_UpdateChMember is NULL !!!\n\r"));
            return IFX_ERROR;
         }
         /* Low level Channel Update Member function */
         ret = pDrvCtx->UpdateChMember (pTapiCh->pLLChannel);

#ifdef TAPI_PACKET
         /* Reset UpStream FIFO */
         TAPI_ResetUpStreamFifo(pTapiCh);
#endif /* TAPI_PACKET */
      }
      pTapiCh->nInUse++;
   }

   return IFX_SUCCESS;
}

/**
   TAPI device pointer initialization
\param
   pDrvCtx - Pointer to device driver context
\param
   devId - Pointer to device driver context
\return
   Returns TAPI_statusOk in case of success, otherwise error code.
\remarks
   Initializes the main pointers and structures for all TAPI devices and
   channels.
*/
IFX_int32_t TAPI_Init_Dev (IFX_TAPI_DRV_CTX_t *pDrvCtx, IFX_uint8_t devId)
{
   TAPI_DEV *pTapiDev;
   IFX_TAPI_LL_DEV_t *pDev;
   int i = devId;

#ifdef TAPI_ONE_DEVNODE
   /* one open initializes all tapi devs */
   for (i=0; i < pDrvCtx->maxDevs; i++)
   {
#endif /* TAPI_ONE_DEVNODE */
      pTapiDev = &(pDrvCtx->pTapiDev[i]);
      /* initialize pTapiDev members */
      if (TAPI_Prepare_Dev (pTapiDev, pDrvCtx) != IFX_SUCCESS)
      {
         RETURN_DEVSTATUS (TAPI_statusInitFail, 0);
      }
      IFXOS_Init_WakeList(pTapiDev->wqEvent);
      pTapiDev->bNeedWakeup = IFX_TRUE;

      /* store the device driver context */
      pTapiDev->pDevDrvCtx = pDrvCtx;
      if (pDrvCtx->Prepare_Dev == IFX_NULL)
      {
         RETURN_DEVSTATUS (TAPI_statusInitFail, 0);
      }
      /* we prepare all devices on register and
         store the coresponding low level device pointer */
      pDev = pDrvCtx->Prepare_Dev (pTapiDev, i);
      if (pDev == IFX_NULL)
      {
         RETURN_DEVSTATUS (TAPI_statusInitFail, 0);
      }
      /* Store the coresponding low level device pointer */
      pTapiDev->pLLDev = (IFX_TAPI_LL_DEV_t *) pDev;

      /* Channel initialization part, initialize pTapiCh members */
      if (TAPI_Prepare_Ch (pTapiDev, pDrvCtx, i) != IFX_SUCCESS)
      {
         RETURN_DEVSTATUS (TAPI_statusInitFail, 0);
      }
      pTapiDev->bInitialized = IFX_TRUE;

#ifdef TAPI_ONE_DEVNODE
      /**\todo Remove me after shift to one dev nodes was successful */
      TRACE (TAPI_DRV,DBG_LEVEL_LOW, ("%d pTapiDev[%d]->pChannel[0]=%p [1]=%p\n\r", __LINE__,
            i, &pTapiDev->pTapiChanelArray[0],
            &pTapiDev->pTapiChanelArray[1]));
   }
#endif /* TAPI_ONE_DEVNODE */

   return TAPI_statusOk;
}

/**
   TAPI device structure initialization
\param
   pTapiDev    - Pointer to TAPI device structure
\param
   pDrvCtx - Pointer to device driver context
\return
   Returns IFX_ERROR in case of an error, otherwise returns IFX_SUCCESS.
\remarks
   Initializes all needed fifos, event queues, memory and sets all needed
   members.
*/
IFX_return_t TAPI_Prepare_Dev (TAPI_DEV* pTapiDev, IFX_TAPI_DRV_CTX_t *pDrvCtx)
{
   pTapiDev->nChannel = IFX_TAPI_DEVICE_CH_NUMBER;
   /*pTapiDev->pToneTbl = (COMPLEX_TONE *)IFXOS_MALLOC(sizeof(COMPLEX_TONE));
   if (pTapiDev->pToneTbl == IFX_NULL)
   {
      return IFX_ERROR;
   }
   memset(pTapiDev->pToneTbl,0x00,sizeof(COMPLEX_TONE));*/

   IFXOS_MutexInit (pTapiDev->semTapiDevSingleIoctlAccess);
#ifdef TAPI_PACKET
   if (pDrvCtx->Write != IFX_NULL)
   {
   /* Prepare bufferpool for voice packets. */
   TAPI_PrepareVoiceBufferPool();
   /* Initialize downstream voice fifo */
   TAPI_InitDownStreamFifo(pTapiDev);
   }
#endif /* TAPI_PACKET */
   /* assign a unique TAPI device ID */
   pTapiDev->nDevID = nGlobalTapiID++;

   return IFX_SUCCESS;
}

/**
   Allocates the TAPI device structure
\param
   pDrvCtx - Pointer to device driver context
\param
   maxDevs - Maximum devices supported by the low-level driver
\return
   IFX_ERROR on error, otherwise IFX_SUCCESS
\remarks
   This function allocates the High-level TAPI device structures
   for corresponding low-level devices and stores the pointer
   in the device driver context.*/
IFX_return_t TAPI_Allocate_Dev_Structure (IFX_TAPI_DRV_CTX_t *pDrvCtx,
                                          IFX_uint32_t maxDevs)
{
   /* reserve space for device on registration */
   pDrvCtx->pTapiDev = (TAPI_DEV*) IFXOS_MALLOC (sizeof(TAPI_DEV) * maxDevs);

   if (pDrvCtx->pTapiDev == IFX_NULL)
   {
         TRACE( TAPI_DRV, DBG_LEVEL_LOW,
               ("IFX_TAPI_Allocate_Dev_Structure: malloc failed\n\r"));
         return IFX_ERROR;
   }

   memset (pDrvCtx->pTapiDev, 0x00, sizeof(TAPI_DEV) * maxDevs);
   return IFX_SUCCESS;
}

/**
   Allocates the TAPI channel structure
\param
   pTapiDev - Pointer to High-Level Tapi device
\param
   maxChannels - Maximum channels supported by the low-level driver
\return
   IFX_ERROR on error, otherwise IFX_SUCCESS
\remarks
   This function allocates the High-level TAPI channel structures
   for corresponding low-level channel and stores the pointer in the
   high-level Tapi device structure.
*/
IFX_return_t TAPI_Allocate_Ch_Structure (TAPI_DEV *pTapiDev,
                                         IFX_uint32_t maxChannels)
{
   /* reserve space for tapi channel structure on registration */
   pTapiDev->pTapiChanelArray = (TAPI_CHANNEL *) IFXOS_MALLOC (
                                 sizeof(TAPI_CHANNEL) * maxChannels);
   if (pTapiDev->pTapiChanelArray == IFX_NULL)
   {
      TRACE( TAPI_DRV, DBG_LEVEL_LOW,
            ("IFX_TAPI_Allocate_Ch_Structure: malloc failed for pCh\n\r"));
      return IFX_ERROR;
   }

   memset (pTapiDev->pTapiChanelArray, 0x00, sizeof(TAPI_CHANNEL) *
           maxChannels);
   return IFX_SUCCESS;
}

/**
  Cleanup Tapi channel
*/
IFX_int32_t TAPI_DeallocateCh (TAPI_DEV *pTapiDev)
{
   TAPI_CHANNEL   *pTapiCh = IFX_NULL;
   IFX_int32_t    ret      = IFX_SUCCESS;
   int         i, j;

   for (i = 0; i < pTapiDev->nMaxChannel; i++)
   {
      pTapiCh = &(pTapiDev->pTapiChanelArray[i]);
      IFX_TAPI_EventDispatcher_Exit(pTapiCh);
      /* Improve: "1" to be replaced with pToneGenCnt */
      for (j = 0; j < TAPI_TONE_MAXRES; ++j)
      {
         if (pTapiCh->pToneRes[j].Tone_Timer != 0)
            TAPI_Delete_Timer (pTapiCh->pToneRes[j].Tone_Timer);
      }
      /* free tone resource */
      IFXOS_FREE(pTapiCh->pToneRes);
   }
   for (i= 0; i < pTapiDev->nMaxChannel; i++)
   {
      pTapiCh = &pTapiDev->pTapiChanelArray[i];
#ifdef TAPI_CID
      IFXOS_FREE(pTapiCh->TapiCidRx.TapiCidRxFifo.pStart);
#endif /* TAPI_CID */
      if (pTapiCh->TapiMeterData.MeterTimerID != NULL)
      {
         TAPI_Delete_Timer (pTapiCh->TapiMeterData.MeterTimerID);
         pTapiCh->TapiMeterData.MeterTimerID = 0;
      }
      if (pTapiCh->TapiMiscData.GndkhTimerID != NULL)
      {
         TAPI_Delete_Timer (pTapiCh->TapiMiscData.GndkhTimerID);
         pTapiCh->TapiMiscData.GndkhTimerID = 0;
      }
      if (pTapiCh->TapiDialData.DialTimerID != NULL)
      {
         TAPI_Delete_Timer (pTapiCh->TapiDialData.DialTimerID);
         pTapiCh->TapiDialData.DialTimerID = 0;
      }
      IFX_TAPI_Ring_Cleanup(pTapiCh);
#ifdef TAPI_CID
      if (pTapiCh->TapiCidTx.CidTimerID)
      {
         TAPI_Delete_Timer (pTapiCh->TapiCidTx.CidTimerID);
         pTapiCh->TapiCidTx.CidTimerID = 0;
      }
#endif /* TAPI_CID */
   }
   /* free tapi connection structure */
   IFXOS_FREE(pTapiDev->pTapiChanelArray);

   return ret;

}

/**
   Register the low level driver
\param
   pLLDrvCtx Pointer to device driver context passed by low-level driver.
   The memory must be allocated before and must be freed by the low level
   driver on unregister.
\return
   IFX_ERROR on error, otherwise IFX_SUCCESS
\remarks
   This function is called when the low-level driver is inserted and does the following
   functions:
      - Stores the low-level device driver context in a global array
      - Registers the device nodes for the low-level driver being added
      - Allocate the high-level TAPI device structure and store it in device driver context
      - Allocate the high-level TAPI channel structures and store it in high-level
        TAPI device structure
*/
IFX_return_t IFX_TAPI_Register_LL_Drv (IFX_TAPI_DRV_CTX_t* pLLDrvCtx)
{
   IFX_uint32_t i;
   IFX_uint32_t maxDevices;
   TAPI_DEV* pTapiDev = IFX_NULL;
   IFX_TAPI_HL_DRV_CTX_t* pHLDrvCtx = IFX_NULL;
   IFX_return_t ret = IFX_ERROR;

   if (pLLDrvCtx == IFX_NULL)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("pLLDrvCtx is NULL. "
            "(File: %s, line: %d)\n", __FILE__, __LINE__));
      return IFX_ERROR;
   }

   /* Don't allow more than one device driver to use same Major number. */
   ret = IFX_TAPI_Is_Device_Registered (pLLDrvCtx->majorNumber);
   /* Here IFX_SUCCESS means somebody is using the given major number. */
   if (ret == IFX_SUCCESS)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("LL driver already registered. "
            "(File: %s, line: %d)\n", __FILE__, __LINE__));
      return IFX_ERROR;
   }

   /* Get a free index in gHLDrvCtx. */
   for (i = 0; i < TAPI_MAX_LL_DEVICES; i++)
   {
      if (gHLDrvCtx[i].bInUse == IFX_FALSE)
      {
         /* set the index after free check */
         pHLDrvCtx = &gHLDrvCtx[i];
         break;
      }
   }

   /* Could not find free index. */
   if (pHLDrvCtx == IFX_NULL)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("Could not find free index for LL "
            "driver in gHLDrvCtx[]. (File: %s, line: %d)\n",
            __FILE__, __LINE__));
      return IFX_ERROR;
   }

   /* Copy registration info from Low level driver. */
   if (strncmp (pLLDrvCtx->hlLLInterfaceVersion, DRV_TAPI_LL_IF_VER_STR, 7))
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
         ("TAPI: ATTENTION - mismatch of HL-LL Interface\n\r"));
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
         ("TAPI: please check that drv_tapi and drv_%s driver match.\n\r",
            pLLDrvCtx->drvName));
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
         ("Version set in LL Driver = %s\n", pLLDrvCtx->hlLLInterfaceVersion));
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
         ("Version expected by HL   = %s\n", DRV_TAPI_LL_IF_VER_STR));
      return IFX_ERROR;
   }

   maxDevices  = pLLDrvCtx->maxDevs;
   /* Allocate TAPI device structure store pointers of driver
     context in each TAPI device structure. */
   if (TAPI_Allocate_Dev_Structure(pLLDrvCtx, maxDevices) == IFX_ERROR)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("Err allocate mem for TAPI_DEV."
            " (File: %s, line: %d)\n", __FILE__, __LINE__));
      return IFX_ERROR;
   }

   /* Allocate TAPI channel structure, store pointers of TAPI
     device structure in each TAPI channel structure.  */
   for (i=0; i < maxDevices; i++)
   {
      pTapiDev = &(pLLDrvCtx->pTapiDev[i]);
      if (TAPI_Allocate_Ch_Structure(pTapiDev, pLLDrvCtx->maxChannels)
          == IFX_ERROR)
      {
         IFXOS_FREE (pLLDrvCtx->pTapiDev);

         TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("Err allocate mem for TAPI_CHANNEL."
               " (File: %s, line: %d)\n", __FILE__, __LINE__));
         return IFX_ERROR;
      }
   }

   /* Store the low-level driver context. */
   pHLDrvCtx->pDrvCtx = pLLDrvCtx;
   pHLDrvCtx->bInUse  = IFX_TRUE;

   ret = TAPI_OS_RegisterLLDrv (pLLDrvCtx, pHLDrvCtx);
   if (ret != IFX_SUCCESS)
   {
      /* dealloc pTapiDev/ch structures */
      IFXOS_FREE (pLLDrvCtx->pTapiDev->pTapiChanelArray);
      IFXOS_FREE (pLLDrvCtx->pTapiDev);
   }

   return ret;
}

/**
   UnRegister the low level driver
\param
   pDrvCtx - Pointer to device driver context passed by low-level driver
\return
   IFX_ERROR on error, otherwise IFX_SUCCESS
\remarks
   This function will be called when the low-level driver is removed from the
   kernel and the following functions are done:
      - Deregister the device nodes from the kernel
      - Free the high-level TAPI device structures
      - Free the low-device driver context
*/
IFX_return_t IFX_TAPI_Unregister_LL_Drv (int majorNumber)
{
   IFX_TAPI_HL_DRV_CTX_t* pHLDrvCtx = IFX_NULL;
   TAPI_DEV *pTapiDev = IFX_NULL;
   int i;
   IFX_return_t ret;

   TRACE (TAPI_DRV, DBG_LEVEL_HIGH,
      ("INFO: Low level driver Unregistering with major number %d\n",
      majorNumber));

   /* find registered device */
   for (i = 0; i < TAPI_MAX_LL_DEVICES; i++)
   {
      if (gHLDrvCtx [i].pDrvCtx != IFX_NULL)
      {
         if (majorNumber == gHLDrvCtx [i].pDrvCtx->majorNumber)
         {
            pHLDrvCtx = &gHLDrvCtx [i];
         }
      }
   }
   /* not registered or no low level context */
   if (pHLDrvCtx == IFX_NULL)
      return IFX_ERROR;

   ret = TAPI_OS_UnregisterLLDrv (pHLDrvCtx->pDrvCtx, pHLDrvCtx);

   pHLDrvCtx->bInUse = IFX_FALSE;
   for (i=0; i < pHLDrvCtx->pDrvCtx->maxDevs; i++)
   {
      pTapiDev = &(pHLDrvCtx->pDrvCtx->pTapiDev[i]);
      if (pTapiDev->PwrSaveTimerID != 0)
      {
         TAPI_Delete_Timer (pTapiDev->PwrSaveTimerID);
      }

#ifdef QOS_SUPPORT
      Qos_Cleanup((IFX_int32_t) pTapiDev);
#endif /* QOS_SUPPORT */
      /**\todo Verify deallocation of channel structure */
#ifdef TAPI_PACKET
      TAPI_Free_FifoBufferPool(pTapiDev, pTapiDev->nMaxChannel);
#endif /* TAPI_PACKET */
      TAPI_DeallocateCh (pTapiDev);
   }

   IFXOS_FREE (pHLDrvCtx->pDrvCtx->pTapiDev);
   /* Free the Low-Level Device Context */
   pHLDrvCtx->pDrvCtx = IFX_NULL;

   return ret;
}

/**
   Returns the pointer to the low level context
   \param Major - Major number of the device
   \return
      If found it returns the pointer to the low level context, otherwise IFX_NULL
   \remark
   Compares the major number with all the device driver's major numbers and
   if it matches, returns the index.
*/
IFX_TAPI_DRV_CTX_t* IFX_TAPI_Get_Device_Driver_Context (IFX_int32_t Major)
{
   int i;

   for (i = 0; i < TAPI_MAX_LL_DEVICES; i++)
   {
      if (gHLDrvCtx [i].pDrvCtx != IFX_NULL)
      {
         if (Major == gHLDrvCtx [i].pDrvCtx->majorNumber)
         {
            return gHLDrvCtx [i].pDrvCtx;
         }
      }
   }
   return IFX_NULL;
}

/**
   Reset TAPI states
   This function is intended to be issued after an HW reset to allow
   silicon reinitialisation.
   \param   pTapiCh  -  reference to a TAPI channel context
   \return  none
*/
IFX_void_t IFX_TAPI_ResetChState (TAPI_CHANNEL *pTapiChannel)
{
   /* reset tapi flags */
   if (pTapiChannel != IFX_NULL)
   {
      pTapiChannel->bInitialized = IFX_FALSE;
      /* reset tapi exeption status */
      pTapiChannel->TapiMiscData.nException.Status = 0;
      /* reset default ringmode */
      pTapiChannel->TapiRingData.RingConfig.nMode =
                        IFX_TAPI_RING_CFG_MODE_INTERNAL_BALANCED;
      pTapiChannel->TapiRingData.RingConfig.nSubmode =
                        IFX_TAPI_RING_CFG_SUBMODE_DC_RNG_TRIP_STANDARD;
   }
}

/**
   Set RTP Payloadtype defaults,

   \param pDevDrvCtx  Pointer to the low-level device driver context.
   \param pChannel    Reference to a TAPI channel.

   \return \ref IFX_SUCCESS or \ref IFX_ERROR

   \note This function should be called on devices with encoders only,
      which should be checked like:
         if (pDrvCtx->COD.RTP_PayloadTable_Cfg != IFX_NULL)
         {
            err = IFX_TAPI_PKT_RTP_PT_Defaults(pChannel);
         }
*/
IFX_int32_t IFX_TAPI_PKT_RTP_PT_Defaults (TAPI_CHANNEL *pChannel)
{
   IFX_TAPI_DRV_CTX_t       *pDrvCtx = pChannel->pTapiDevice->pDevDrvCtx;
   IFX_TAPI_PKT_RTP_PT_CFG_t pt;
   IFX_int32_t               err;

   /* \todo add check for AAL2 mode - in this case we'd simply return */

   memset(&pt, 0, sizeof(IFX_TAPI_PKT_RTP_PT_CFG_t));
   pt.nPTup[IFX_TAPI_COD_TYPE_G723_63]     = 0x04;
   pt.nPTup[IFX_TAPI_COD_TYPE_G723_53]     = 0x04;
   pt.nPTup[IFX_TAPI_COD_TYPE_G728]        = 0x0F; /* fixed acc. RFC 3551 */
   pt.nPTup[IFX_TAPI_COD_TYPE_G729]        = 0x12; /* fixed acc. RFC 3551 */
   pt.nPTup[IFX_TAPI_COD_TYPE_MLAW]        = 0x00; /* fixed acc. RFC 3551 */
   pt.nPTup[IFX_TAPI_COD_TYPE_ALAW]        = 0x08; /* fixed acc. RFC 3551 */
   pt.nPTup[IFX_TAPI_COD_TYPE_G726_16]     = 0x63;
   pt.nPTup[IFX_TAPI_COD_TYPE_G726_24]     = 0x64;
   pt.nPTup[IFX_TAPI_COD_TYPE_G726_32]     = 0x65;
   pt.nPTup[IFX_TAPI_COD_TYPE_G726_40]     = 0x66;
   pt.nPTup[IFX_TAPI_COD_TYPE_G729_E]      = 0x61;
   pt.nPTup[IFX_TAPI_COD_TYPE_ILBC_152]    = 0x67;
   pt.nPTup[IFX_TAPI_COD_TYPE_ILBC_133]    = 0x67;
   pt.nPTup[IFX_TAPI_COD_TYPE_LIN16_16]    = 0x70;
   pt.nPTup[IFX_TAPI_COD_TYPE_LIN16_8]     = 0x71;
   pt.nPTup[IFX_TAPI_COD_TYPE_AMR_4_75]    = 0x72;
   pt.nPTup[IFX_TAPI_COD_TYPE_AMR_5_9]     = 0x72;
   pt.nPTup[IFX_TAPI_COD_TYPE_AMR_5_15]    = 0x72;
   pt.nPTup[IFX_TAPI_COD_TYPE_AMR_6_7]     = 0x72;
   pt.nPTup[IFX_TAPI_COD_TYPE_AMR_7_4]     = 0x72;
   pt.nPTup[IFX_TAPI_COD_TYPE_AMR_7_95]    = 0x72;
   pt.nPTup[IFX_TAPI_COD_TYPE_AMR_10_2]    = 0x72;
   pt.nPTup[IFX_TAPI_COD_TYPE_AMR_12_2]    = 0x72;
   pt.nPTup[IFX_TAPI_COD_TYPE_G722_64]     = 0x09;
   pt.nPTup[IFX_TAPI_COD_TYPE_G7221_24]    = 0x73;
   pt.nPTup[IFX_TAPI_COD_TYPE_G7221_32]    = 0x74;
   pt.nPTup[IFX_TAPI_COD_TYPE_MLAW_VBD]    = 0x75;
   pt.nPTup[IFX_TAPI_COD_TYPE_ALAW_VBD]    = 0x76;

   pt.nPTdown[IFX_TAPI_COD_TYPE_G723_63]   = 0x04;
   pt.nPTdown[IFX_TAPI_COD_TYPE_G723_53]   = 0x04;
   pt.nPTdown[IFX_TAPI_COD_TYPE_G728]      = 0x0F; /* fixed acc. RFC 3551 */
   pt.nPTdown[IFX_TAPI_COD_TYPE_G729]      = 0x12; /* fixed acc. RFC 3551 */
   pt.nPTdown[IFX_TAPI_COD_TYPE_MLAW]      = 0x00; /* fixed acc. RFC 3551 */
   pt.nPTdown[IFX_TAPI_COD_TYPE_ALAW]      = 0x08; /* fixed acc. RFC 3551 */
   pt.nPTdown[IFX_TAPI_COD_TYPE_G726_16]   = 0x63;
   pt.nPTdown[IFX_TAPI_COD_TYPE_G726_24]   = 0x64;
   pt.nPTdown[IFX_TAPI_COD_TYPE_G726_32]   = 0x65;
   pt.nPTdown[IFX_TAPI_COD_TYPE_G726_40]   = 0x66;
   pt.nPTdown[IFX_TAPI_COD_TYPE_G729_E]    = 0x61;
   pt.nPTdown[IFX_TAPI_COD_TYPE_ILBC_152]  = 0x67;
   pt.nPTdown[IFX_TAPI_COD_TYPE_ILBC_133]  = 0x67;
   pt.nPTdown[IFX_TAPI_COD_TYPE_LIN16_16]  = 0x70;
   pt.nPTdown[IFX_TAPI_COD_TYPE_LIN16_8]   = 0x71;
   pt.nPTdown[IFX_TAPI_COD_TYPE_AMR_4_75]  = 0x72;
   pt.nPTdown[IFX_TAPI_COD_TYPE_AMR_5_9]   = 0x72;
   pt.nPTdown[IFX_TAPI_COD_TYPE_AMR_5_15]  = 0x72;
   pt.nPTdown[IFX_TAPI_COD_TYPE_AMR_6_7]   = 0x72;
   pt.nPTdown[IFX_TAPI_COD_TYPE_AMR_7_4]   = 0x72;
   pt.nPTdown[IFX_TAPI_COD_TYPE_AMR_7_95]  = 0x72;
   pt.nPTdown[IFX_TAPI_COD_TYPE_AMR_10_2]  = 0x72;
   pt.nPTdown[IFX_TAPI_COD_TYPE_AMR_12_2]  = 0x72;
   pt.nPTdown[IFX_TAPI_COD_TYPE_G722_64]   = 0x09;
   pt.nPTdown[IFX_TAPI_COD_TYPE_G7221_24]  = 0x73;
   pt.nPTdown[IFX_TAPI_COD_TYPE_G7221_32]  = 0x74;
   pt.nPTdown[IFX_TAPI_COD_TYPE_MLAW_VBD]  = 0x75;
   pt.nPTdown[IFX_TAPI_COD_TYPE_ALAW_VBD]  = 0x76;

   if (ptr_chk(pDrvCtx->COD.RTP_PayloadTable_Cfg,
              "pDrvCtx->COD.RTP_PayloadTable_Cfg"))
   {
      err = pDrvCtx->COD.RTP_PayloadTable_Cfg(pChannel->pLLChannel, &pt);
   }
   else
   {
      err = IFX_ERROR;
   }

   return err;
}
