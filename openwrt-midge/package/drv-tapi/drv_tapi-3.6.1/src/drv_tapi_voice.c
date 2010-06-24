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

/**
   \file  drv_tapi_voice.c
   Contains TAPI Voice Services : Play, Recording, Conferencing.
*/

/* ============================= */
/* Includes                      */
/* ============================= */

#include "drv_tapi.h"
#include "drv_tapi_ll_interface.h"

#ifdef TAPI_VOICE

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
   This interface adds a data channel to an analog phone device

   \param pChannel      Pointer to TAPI_CHANNEL structure.
   \param pMap          Pointer to IFX_TAPI_MAP_DATA_t structure.

   \return
   \ref IFX_SUCCESS or \ref IFX_ERROR
*/
IFX_int32_t TAPI_Data_Channel_Add (TAPI_CHANNEL *pChannel,
                                   IFX_TAPI_MAP_DATA_t const *pMap)
{
   IFX_TAPI_DRV_CTX_t *pDrvCtx = pChannel->pTapiDevice->pDevDrvCtx;
   TAPI_CHANNEL *pPhoneChannel = IFX_NULL;
   IFX_int32_t ret = IFX_SUCCESS;

   /* Implement generic part here if */
   switch (pMap->nPlayStart)
   {
      case IFX_TAPI_MAP_DATA_UNCHANGED:
         /* do nothing */
         break;
      case IFX_TAPI_MAP_DATA_START:
         if (ptr_chk(pDrvCtx->COD.DEC_Start, "pDrvCtx->COD.DEC_Start"))
            ret = pDrvCtx->COD.DEC_Start (pChannel->pLLChannel);
         break;
      case IFX_TAPI_MAP_DATA_STOP:
         if (ptr_chk(pDrvCtx->COD.DEC_Stop, "pDrvCtx->COD.DEC_Stop"))
            ret = pDrvCtx->COD.DEC_Stop (pChannel->pLLChannel);
         break;
   }
   if (ret == IFX_SUCCESS)
   {
      switch (pMap->nRecStart)
      {
         case IFX_TAPI_MAP_DATA_UNCHANGED:
            /* do nothing */
            break;
         case IFX_TAPI_MAP_DATA_START:
            if (ptr_chk(pDrvCtx->COD.ENC_Start, "pDrvCtx->COD.ENC_Start"))
               ret = pDrvCtx->COD.ENC_Start (pChannel->pLLChannel);
            break;
         case IFX_TAPI_MAP_DATA_STOP:
            if (ptr_chk(pDrvCtx->COD.ENC_Stop, "pDrvCtx->COD.ENC_Stop"))
               ret = pDrvCtx->COD.ENC_Stop (pChannel->pLLChannel);
            break;
      }
   }

   /* call low level function here if */
   if (ret == IFX_SUCCESS)
   {
      if (ptr_chk(pDrvCtx->CON.Data_Channel_Add,
                 "pDrvCtx->CON.Data_Channel_Add"))
      {
         ret = pDrvCtx->CON.Data_Channel_Add (pChannel->pLLChannel, pMap);
      }
   }

   if (ret == IFX_SUCCESS)
   {
      pPhoneChannel = &pChannel->pTapiDevice->pTapiChanelArray[pMap->nDstCh];
      switch ( pMap->nChType )
      {
      case IFX_TAPI_MAP_TYPE_AUDIO_AUX:
         /* lint-fallthrough */
      case IFX_TAPI_MAP_TYPE_AUDIO:
         /* Set appropriate dsp data channel on according audio channel */
         pPhoneChannel->nAudioDataChannels |= (1 << pChannel->nChannel);
         /* Set appropriate phone channel on according dsp data channel */
         pChannel->nDataPhoneChannels      |= (1 << pMap->nDstCh);
         break;
      case IFX_TAPI_MAP_TYPE_PHONE:
         /* lint-fallthrough */
      case IFX_TAPI_MAP_TYPE_DEFAULT:
         /* Set appropriate dsp data channel on according phone channel */
         pPhoneChannel->nPhoneDataChannels |= (1 << pChannel->nChannel);
         /* Set appropriate phone channel on according dsp data channel */
         pChannel->nDataPhoneChannels      |= (1 << pMap->nDstCh);
         break;
      case IFX_TAPI_MAP_TYPE_PCM:
         /* do nothing */
         break;
      default:
         /* do nothing */
         break;
      }
   }

   return ret;
}

/**
   This interface removes a data channel from an analog phone device

   \param pChannel      Pointer to TAPI_CHANNEL structure.
   \param pMap          Pointer to IFX_TAPI_MAP_DATA_t structure.

   \return
   \ref IFX_SUCCESS or \ref IFX_ERROR
*/
IFX_int32_t TAPI_Data_Channel_Remove (TAPI_CHANNEL *pChannel,
                                      IFX_TAPI_MAP_DATA_t const *pMap)
{
   IFX_TAPI_DRV_CTX_t *pDrvCtx = pChannel->pTapiDevice->pDevDrvCtx;
   TAPI_CHANNEL  *pPhoneChannel = IFX_NULL;
   IFX_int32_t ret = IFX_SUCCESS;

   /* Implement generic part here if */
   switch (pMap->nPlayStart)
   {
      case IFX_TAPI_MAP_DATA_UNCHANGED:
         /* do nothing */
         break;
      case IFX_TAPI_MAP_DATA_START:
         if (ptr_chk(pDrvCtx->COD.DEC_Start, "pDrvCtx->COD.DEC_Start"))
            ret = pDrvCtx->COD.DEC_Start (pChannel->pLLChannel);
         break;
      case IFX_TAPI_MAP_DATA_STOP:
         if (ptr_chk(pDrvCtx->COD.DEC_Stop, "pDrvCtx->COD.DEC_Stop"))
            ret = pDrvCtx->COD.DEC_Stop (pChannel->pLLChannel);
         break;
   }
   if (ret == IFX_SUCCESS)
   {
      switch (pMap->nRecStart)
      {
         case IFX_TAPI_MAP_DATA_UNCHANGED:
            /* do nothing */
            break;
         case IFX_TAPI_MAP_DATA_START:
            if (ptr_chk(pDrvCtx->COD.ENC_Start, "pDrvCtx->COD.ENC_Start"))
               ret = pDrvCtx->COD.ENC_Start (pChannel->pLLChannel);
            break;
         case IFX_TAPI_MAP_DATA_STOP:
            if (ptr_chk(pDrvCtx->COD.ENC_Stop, "pDrvCtx->COD.ENC_Stop"))
               ret = pDrvCtx->COD.ENC_Stop (pChannel->pLLChannel);
            break;
      }
   }

   /* call low level function here if */
   if (ret == IFX_SUCCESS)
   {
      if (ptr_chk(pDrvCtx->CON.Data_Channel_Remove,
                 "pDrvCtx->CON.Data_Channel_Remove"))
         ret = pDrvCtx->CON.Data_Channel_Remove (pChannel->pLLChannel, pMap);
   }

   if (ret == IFX_SUCCESS)
   {
      pPhoneChannel = &pChannel->pTapiDevice->pTapiChanelArray[pMap->nDstCh];
      switch ( pMap->nChType )
      {
      case IFX_TAPI_MAP_TYPE_AUDIO_AUX:
         /* lint-fallthrough */
      case IFX_TAPI_MAP_TYPE_AUDIO:
         /* Clear appropriate dsp data channel on according audio channel */
         pPhoneChannel->nAudioDataChannels &= ~(1 << pChannel->nChannel);
         /* Clear appropriate phone channel on according dsp data channel */
         pChannel->nDataPhoneChannels      &= ~(1 << pMap->nDstCh);
         break;
      case IFX_TAPI_MAP_TYPE_PHONE:
         /* lint-fallthrough */
      case IFX_TAPI_MAP_TYPE_DEFAULT:
         /* Clear appropriate dsp data channel on according phone channel */
         pPhoneChannel->nPhoneDataChannels &= ~(1 << pChannel->nChannel);
         /* Clear appropriate phone channel on according dsp data channel */
         pChannel->nDataPhoneChannels      &= ~(1 << pMap->nDstCh);
         break;
      case IFX_TAPI_MAP_TYPE_PCM:
         /* do nothing */
         break;
      default:
         /* do nothing */
         break;
      }
   }

   return ret;
}

/**
   This interface adds a phone channel to an analog phone device

   \param pChannel      Pointer to TAPI_CHANNEL structure.
   \param pMap          Pointer to IFX_TAPI_MAP_PHONE_t structure.

   \return
   \ref IFX_SUCCESS or \ref IFX_ERROR
*/
IFX_int32_t TAPI_Phone_Channel_Add (TAPI_CHANNEL *pChannel,
                                    IFX_TAPI_MAP_PHONE_t const *pMap)
{
   IFX_TAPI_DRV_CTX_t *pDrvCtx = pChannel->pTapiDevice->pDevDrvCtx;
   IFX_int32_t ret = IFX_ERROR;

   /* call low level function */
   if (ptr_chk(pDrvCtx->CON.Phone_Channel_Add,
              "pDrvCtx->CON.Phone_Channel_Add"))
      ret = pDrvCtx->CON.Phone_Channel_Add (pChannel->pLLChannel, pMap);

   return ret;
}

/**
   This interface removes a phone channel from an analog phone device

   \param pChannel      Pointer to TAPI_CHANNEL structure.
   \param pMap          Pointer to IFX_TAPI_MAP_PHONE_t structure.

   \return
   \ref IFX_SUCCESS or \ref IFX_ERROR
*/
IFX_int32_t TAPI_Phone_Channel_Remove (TAPI_CHANNEL *pChannel,
                                       IFX_TAPI_MAP_PHONE_t const *pMap)
{
   IFX_TAPI_DRV_CTX_t *pDrvCtx = pChannel->pTapiDevice->pDevDrvCtx;
   IFX_int32_t ret = IFX_ERROR;

   /* call low level function */
   if (ptr_chk(pDrvCtx->CON.Phone_Channel_Remove,
              "pDrvCtx->CON.Phone_Channel_Remove"))
      ret = pDrvCtx->CON.Phone_Channel_Remove (pChannel->pLLChannel, pMap);

   return ret;
}

/**
   This interface returns the phone channel mapped to to the data channel.

   \param pChannel       - handle to TAPI_CHANNEL structure
   \param pPhoneChannel  - pointer to phone channel

   \return
   IFX_SUCCESS / IFX_ERROR

   \remark
   - In case more than one phone channel are mapped to the data channel, the first
     one is retrieved. Therefore the user should make sure that this first channel
     is the one on which his phone activities are run.
   - If no phone channel is found, it will be assumed that phone and data
     channels are identical.
   - Function should not be blocking because it can be used within a high
     priority routine as well (e.g. interrupt routine)
*/
IFX_int32_t TAPI_Data_Get_Phone_Channel (TAPI_CHANNEL *pChannel,
                                         IFX_uint8_t *pPhoneChannel)
{
   IFX_uint8_t i;

   /* reference must be valid */
   if (pPhoneChannel == NULL)
      return IFX_ERROR;

   /* default: data and phone channels are identical */
   *pPhoneChannel = pChannel->nChannel;

   for (i = 0; i < pChannel->pTapiDevice->nMaxChannel ; i++)
   {
      if (((pChannel->nDataPhoneChannels >> i) & 0x1) == 0x1)
      {
         *pPhoneChannel = i;
         break;
      }
   }

   return IFX_SUCCESS;
}

/**
   This interface returns the data channel mapped to the phone channel.

   \param pChannel       - handle to TAPI_CHANNEL structure
   \param pDataChannel   - pointer to phone channel

   \return
   IFX_SUCCESS / IFX_ERROR

   \remark
   - In case more than one data channel are mapped to the data channel,
     the first one is retrieved. Therefore the user should make sure that
     this first channel is the one on which his data activities are run.
   - If no data channel is found, it will be assumed that data and phone
     channels are identical.
   - Function should not be blocking because it can be used within a high
     priority routine as well (e.g. interrupt routine)
*/
IFX_int32_t TAPI_Phone_Get_Data_Channel (TAPI_CHANNEL *pChannel,
                                         IFX_uint8_t *pDataChannel)
{
   IFX_uint8_t i;

   /* reference must be valid */
   if (pDataChannel == IFX_NULL)
      return IFX_ERROR;

   /* default: data and phone channels are identical */
   *pDataChannel = pChannel->nChannel;

   for (i = 0; i < pChannel->pTapiDevice->nMaxChannel ; i++)
   {
      if (((pChannel->nPhoneDataChannels >> i) & 0x1) == 0x1)
      {
         *pDataChannel = i;
         break;
      }
   }

   return IFX_SUCCESS;
}

#endif /* TAPI_VOICE */
