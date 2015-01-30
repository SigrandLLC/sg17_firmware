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
   Module      : drv_vinetic_con.c
   Date        : 2005-02-10
******************************************************************************/

/**
   \file drv_vinetic_con.c VINETIC DSP module interconnection managment module.
   \remarks
  This module provides functions to connect different DSP modules. It is used
  for conferencing, but also for basic dynamic connections */

/* ============================= */
/* Includes                      */
/* ============================= */
#include "ifx_types.h"

#ifdef HAVE_CONFIG_H
#include "drv_config.h"
#endif

#include "drv_vinetic_con_priv.h"
#include "drv_vinetic_con.h"
#include "drv_vinetic_api.h"
#include "drv_vinetic_sig.h"

/* ============================= */
/* Local Macros & Definitions    */
/* ============================= */

#ifdef DEBUG
const IFX_char_t signalName [52][10] = {
   "Null\0",
   "PCM0-Out\0",
   "PCM1-Out\0",
   "PCM2-Out\0",
   "PCM3-Out\0",
   "PCM4-Out\0",
   "PCM5-Out\0",
   "PCM6-Out\0",
   "PCM7-Out\0",
   "PCM8-Out\0",
   "PCM9-Out\0",
   "PCM10-Out\0",
   "PCM11-Out\0",
   "PCM12-Out\0",
   "PCM13-Out\0",
   "PCM14-Out\0",
   "PCM15-Out\0",
   "ALM0-Out\0",
   "ALM1-Out\0",
   "ALM2-Out\0",
   "ALM3-Out\0",
   "\0","\0","\0",
   "COD0-Out\0",
   "COD1-Out\0",
   "COD2-Out\0",
   "COD3-Out\0",
   "COD4-Out\0",
   "COD5-Out\0",
   "COD6-Out\0",
   "COD7-Out\0",
   "\0","\0","\0","\0","\0","\0","\0","\0",
   "SIG0-OutA\0",
   "SIG0-OutB\0",
   "SIG1-OutA\0",
   "SIG1-OutB\0",
   "SIG2-OutA\0",
   "SIG2-OutB\0",
   "SIG3-OutA\0",
   "SIG3-OutB\0",
   "SIG4-OutA\0",
   "SIG4-OutB\0",
   "SIG5-OutA\0",
   "SIG5-OutB\0"
};
#endif /* DEBUG */

/* ============================= */
/* Local function declaration    */
/* ============================= */

#if (VIN_CFG_FEATURES & VIN_FEAT_VOICE)
static IFX_return_t IFX_TAPI_LL_Data_Channel_Add (IFX_TAPI_LL_CH_t *pLLChannel,
                                                  IFX_TAPI_MAP_DATA_t const *pMap);

static IFX_return_t IFX_TAPI_LL_Data_Channel_Remove (IFX_TAPI_LL_CH_t *pLLChannel,
                                                     IFX_TAPI_MAP_DATA_t const *pMap);

static IFX_return_t IFX_TAPI_LL_PCM_Channel_Add (IFX_TAPI_LL_CH_t *pLLChannel,
                                                 IFX_TAPI_MAP_PCM_t const *pMap);

static IFX_return_t IFX_TAPI_LL_PCM_Channel_Remove (IFX_TAPI_LL_CH_t *pLLChannel,
                                                    IFX_TAPI_MAP_PCM_t const *pMap);

static IFX_return_t IFX_TAPI_LL_Phone_Channel_Add (IFX_TAPI_LL_CH_t *pLLChannel,
                                                   IFX_TAPI_MAP_PHONE_t const *pMap);

static IFX_return_t IFX_TAPI_LL_Phone_Channel_Remove (IFX_TAPI_LL_CH_t *pLLChannel,
                                                      IFX_TAPI_MAP_PHONE_t const *pMap);

#ifdef TAPI_CID
static IFX_return_t IFX_TAPI_LL_Data_Channel_Mute (IFX_TAPI_LL_CH_t *pLLChannel,
                                                   IFX_boolean_t nMute);
#endif /* TAPI_CID */

static IFX_return_t LOCAL_Module_Connect (IFX_TAPI_CONN_ACTION_t nAction,
                                          VINETIC_DEVICE *pDev,
                                          IFX_uint32_t nCh1,
                                          IFX_TAPI_MAP_TYPE_t nChType1,
                                          IFX_uint32_t nCh2,
                                          IFX_TAPI_MAP_TYPE_t nChType2);
#endif /* (VIN_CFG_FEATURES & VIN_FEAT_VOICE) */

static IFX_return_t LOCAL_ConnectPrepare (VINDSP_MODULE_t* pSrc,
                                          VINDSP_MODULE_t* pDst,
                                          SIG_OUTPUT_SIDE nSide);
static IFX_return_t LOCAL_DisconnectPrepare (VINDSP_MODULE_t* pSrc,
                                             VINDSP_MODULE_t* pDst,
                                             SIG_OUTPUT_SIDE nSide);


/* ============================= */
/* Function definitions          */
/* ============================= */

/** \defgroup VINETIC_CONNECTIONS VINETIC Connection Module */

/**
   Allocate data structure of the CON module for the given channel.

   \param   VINETIC_CHANNEL*   Pointer to the VINETIC channel structure
   \return  IFX_SUCCESS or IFX_ERROR in case the stucture could not be created
*/
IFX_return_t VINETIC_CON_Allocate_Ch_Structures (VINETIC_CHANNEL *pCh)
{
   VINETIC_CON_Free_Ch_Structures (pCh);

   pCh->pCON = IFXOS_MALLOC(sizeof(VINETIC_CON_t));

   if (pCh->pCON == IFX_NULL)
   {
      return IFX_ERROR;
   }
   memset(pCh->pCON, 0, sizeof(VINETIC_CON_t));

   return IFX_SUCCESS;
}

/**
  Free data structure of the CON module for the given channel

   \param   VINETIC_CHANNEL*   Pointer to the VINETIC channel structure
   \return  none
*/
IFX_void_t VINETIC_CON_Free_Ch_Structures (VINETIC_CHANNEL *pCh)
{
   if (pCh->pCON != IFX_NULL)
   {
      IFXOS_FREE(pCh->pCON);
   }
}

/**
   function called during initialisation of channel, fills up
   the connection module related structure for ALM channel

   \param   VINETIC_CHANNEL*   Pointer to the VINETIC channel structure
   \return  none
*/
IFX_void_t VINETIC_CON_Init_AlmCh (VINETIC_CHANNEL *pCh)
{
   VINDSP_MODULE_t *pModAlm = &(pCh->pCON->modAlm);
   IFX_uint8_t ch = pCh->nChannel - 1;
   IFX_uint8_t i;

   pModAlm->nModType = VINDSP_MT_ALM;
   pModAlm->nSignal  = ECMD_IX_ALM_OUT0 + ch;

#ifdef DEBUG
   strcpy (pModAlm->name, signalName[pModAlm->nSignal]);
#endif /* DEBUG */
   for (i = 0; i < MAX_MODULE_SIGNAL_INPUTS; ++i)
      pModAlm->in[i].pParent = pModAlm;
}

/**
   function called during initialisation of channel, fills up
   the connection module related structure for PCM channel

   \param   VINETIC_CHANNEL*   Pointer to the VINETIC channel structure
   \param   IFX_uint8_t        Number of PCM resource
   \return  none
*/
IFX_void_t VINETIC_CON_Init_PcmCh (VINETIC_CHANNEL *pCh, IFX_uint8_t pcmCh)
{
   VINDSP_MODULE_t *pModPcm = &(pCh->pCON->modPcm);
   IFX_uint8_t i;

   pModPcm->nModType = VINDSP_MT_PCM;
   pModPcm->nSignal  = ECMD_IX_PCM_OUT0 + pcmCh;

#ifdef DEBUG
   strcpy (pModPcm->name, signalName[pModPcm->nSignal]);
#endif /* DEBUG */
   for (i = 0; i < MAX_MODULE_SIGNAL_INPUTS; ++i)
      pModPcm->in[i].pParent = pModPcm;
}

/**
   function called during initialisation of channel, fills up
   the connection module related structure for COD channel

   \param   VINETIC_CHANNEL*   Pointer to the VINETIC channel structure
   \return  none
*/
IFX_void_t VINETIC_CON_Init_CodCh (VINETIC_CHANNEL *pCh)
{
   VINDSP_MODULE_t *pModCod = &(pCh->pCON->modCod);
   IFX_uint8_t ch = pCh->nChannel - 1;
   IFX_uint8_t i;

   pModCod->nModType = VINDSP_MT_COD;
   pModCod->nSignal  = ECMD_IX_COD_OUT0 + ch;

#ifdef DEBUG
   strcpy (pModCod->name, signalName[pModCod->nSignal]);
#endif /* DEBUG */
   for (i = 0; i < MAX_MODULE_SIGNAL_INPUTS; ++i)
      pModCod->in[i].pParent = pModCod;
}

/**
   function called during initialisation of channel, fills up
   the connection module related structure for SIG channel

   \param   VINETIC_CHANNEL*   Pointer to the VINETIC channel structure
   \return  none
*/
IFX_void_t VINETIC_CON_Init_SigCh (VINETIC_CHANNEL *pCh)
{
   VINDSP_MODULE_t *pModSig = &(pCh->pCON->modSig);
   IFX_uint8_t ch = pCh->nChannel - 1;
   IFX_uint8_t i;

   pModSig->nModType = VINDSP_MT_SIG;
   pModSig->nSignal  = ECMD_IX_SIG_OUTA0 + (IFX_uint8_t)(2*ch);
   pModSig->nSignal2 = ECMD_IX_SIG_OUTB0 + (IFX_uint8_t)(2*ch);

#ifdef DEBUG
   strcpy (pModSig->name, signalName[pModSig->nSignal]);
#endif /* DEBUG */
   for (i = 0; i < MAX_MODULE_SIGNAL_INPUTS; ++i)
      pModSig->in[i].pParent = pModSig;
}


IFX_uint8_t VINETIC_CON_Get_ALM_SignalInput (VINETIC_CHANNEL *pCh,
                                             IFX_uint8_t index)
{
   VINDSP_MODULE_t *pMod = &pCh->pCON->modAlm;

   /* Return either the normal or the muted signal index */
   return pMod->in[index].mute ? pMod->in[index].i_mute : pMod->in[index].i;
}

IFX_uint8_t VINETIC_CON_Get_PCM_SignalInput (VINETIC_CHANNEL *pCh,
                                             IFX_uint8_t index)
{
   VINDSP_MODULE_t *pMod = &pCh->pCON->modPcm;

   /* Return either the normal or the muted signal index */
   return pMod->in[index].mute ? pMod->in[index].i_mute : pMod->in[index].i;
}

IFX_uint8_t VINETIC_CON_Get_COD_SignalInput (VINETIC_CHANNEL *pCh,
                                             IFX_uint8_t index)
{
   VINDSP_MODULE_t *pMod = &pCh->pCON->modCod;

   /* Return either the normal or the muted signal index */
   return pMod->in[index].mute ? pMod->in[index].i_mute : pMod->in[index].i;
}

IFX_uint8_t VINETIC_CON_Get_SIG_SignalInput (VINETIC_CHANNEL *pCh,
                                             IFX_uint8_t index)
{
   VINDSP_MODULE_t *pMod = &pCh->pCON->modSig;

   /* Return either the normal or the muted signal index */
   return pMod->in[index].mute ? pMod->in[index].i_mute : pMod->in[index].i;
}



#if (VIN_CFG_FEATURES & VIN_FEAT_VOICE)

/**
   Function called by the init_module of device, fills up CONnection module
   Function pointers which are passed to HL TAPI during registration
\param pCON     - pointer to CON module
\return
   IFX_SUCCESS
Remarks
*/
IFX_return_t VINETIC_CON_Func_Register (IFX_TAPI_DRV_CTX_CON_t *pCON)
{
   pCON->Data_Channel_Add     = IFX_TAPI_LL_Data_Channel_Add;
   pCON->Data_Channel_Remove  = IFX_TAPI_LL_Data_Channel_Remove;
   pCON->Phone_Channel_Add    = IFX_TAPI_LL_Phone_Channel_Add;
   pCON->Phone_Channel_Remove = IFX_TAPI_LL_Phone_Channel_Remove;
   pCON->PCM_Channel_Add      = IFX_TAPI_LL_PCM_Channel_Add;
   pCON->PCM_Channel_Remove   = IFX_TAPI_LL_PCM_Channel_Remove;
#ifdef TAPI_CID
   pCON->Data_Channel_Mute    = IFX_TAPI_LL_Data_Channel_Mute;
#endif /* TAPI_CID */

   return IFX_SUCCESS;
}

/**
   Connect a data channel to an analog phone device
\param pLLChannel Handle to IFX_TAPI_LL_CH_t structure
\param pMap        Handle to IFX_TAPI_MAP_DATA_t structure
\return Return value according to IFX_return_t
   - IFX_ERROR if an error occured
   - IFX_SUCCESS if successful
\remarks
   By definition a data channel consists of the COD-module and the SIG-module
   of the same TAPI connection.
*/
IFX_return_t IFX_TAPI_LL_Data_Channel_Add (IFX_TAPI_LL_CH_t *pLLChannel,
                                           IFX_TAPI_MAP_DATA_t const *pMap)
{
   VINETIC_CHANNEL *pCh = (VINETIC_CHANNEL *)pLLChannel;
   VINETIC_DEVICE *pDev = (VINETIC_DEVICE*) (pCh->pParent);
   IFX_uint32_t nThisCh = pCh->nChannel - 1;
   int err, cnt = 0, i, j;
   VINDSP_MODULE_t *pDstMod = IFX_NULL,
                   *tmpSigs[MAX_MODULE_SIGNAL_INPUTS] = {IFX_NULL};
   VINETIC_CON_t   *pThisChModules = pDev->pChannel[nThisCh].pCON;
   VINDSP_MODULE_SIGNAL *pInput = IFX_NULL;

   switch (pMap->nChType)
   {
   case IFX_TAPI_MAP_TYPE_PCM:
      if (pMap->nDstCh < pDev->caps.nPCM)
         pDstMod = &pDev->pChannel[pMap->nDstCh].pCON->modPcm;
      break;
   case IFX_TAPI_MAP_TYPE_PHONE:
   case IFX_TAPI_MAP_TYPE_DEFAULT:
      if (pMap->nDstCh < pDev->caps.nALI)
         pDstMod = &pDev->pChannel[pMap->nDstCh].pCON->modAlm;
      break;
   case IFX_TAPI_MAP_TYPE_CODER:
   default:
      SET_ERROR (VIN_ERR_NOTSUPPORTED);
      return IFX_ERROR;
   }
   if ((pDstMod == IFX_NULL) ||
       (nThisCh >= pDev->caps.nSIG) || (nThisCh >= pDev->caps.nCOD))
   {
      SET_ERROR (VIN_ERR_FUNC_PARM);
      return IFX_ERROR;
   }
   /* Here pDstMod points to a valid module which should be connected to the
      data channel. It can either be an ALM or an PCM module. */

   /* Verify that this is a data channel by making sure SIG is getting it's
      input from the COD-module of the same channel. */
   if (pThisChModules->modSig.in[REMOTE_SIG_IN].i !=
       pThisChModules->modCod.nSignal)
   {
      SET_ERROR (VIN_ERR_WRONG_CHANNEL_MODE);
      return IFX_ERROR;
   }

   /* connect SIG output to Dst (ALM or PCM) input, if not already done */
   err = LOCAL_ConnectPrepare (&pThisChModules->modSig, pDstMod, LOCAL_SIG_OUT);
   if (err == IFX_SUCCESS)
   {
      /* Connect the Dst (ALM or PCM) to the data channel, if not already done.
         If the upstream input (I1) of the signalling module is already in
         use by another ALM or PCM output connect the new output to the next
         free upstream input on the coder of the same data channel instead. */
      if (pThisChModules->modSig.in[LOCAL_SIG_IN].i == ECMD_IX_EMPTY)
      {
         /* connect Dst (ALM or PCM) output to SIG input (I1) */
         err = LOCAL_ConnectPrepare (pDstMod,
                                     &pThisChModules->modSig, 0);
      }
      else
      {
         /* Connect the Dst (ALM or PCM) output to COD input because the
            SIG input (I1) is already in use. This happens when adding more
            than one phone/PCM to a data channel. But prevent connecting
            Dst to the coder when it is already connected to signalling. */
         if (pThisChModules->modSig.in[LOCAL_SIG_IN].i != pDstMod->nSignal)
            err = LOCAL_ConnectPrepare (pDstMod,
                                        &pThisChModules->modCod, 0);
      }
   }

   if (err == IFX_SUCCESS)
   {
      /* Check if the Dst (ALM or PCM) is now connected to more than one
         data channel. If this is the case then connect these data channels
         so that all the data channels can talk to each other */

      /* Count to how many data channels the analog module is connected
         and store a pointer to each signalling module in a temporary list. */
      for (pInput = pDstMod->pInputs, cnt = 0;
           pInput != IFX_NULL; pInput = pInput->pNext )
      {
         if (pInput->pParent->nModType == VINDSP_MT_SIG)
            tmpSigs[cnt++] = pInput->pParent;
         if (pInput->pParent->nModType == VINDSP_MT_COD)
            tmpSigs[cnt++] = pInput->pParent->pInputs->pParent;
      }

      if (cnt > 1)
      {
         /* Conference with more than one data channel. So now connect each
            data channel to all the others that are stored in the list above.
            For this each signaling output is connected to one input on every
            coder taking part in the conference. ConnectPrepare may be called
            even if the input is already connected. */
         for (i = 0; i <  cnt; i++)
            for (j = 0; j < cnt; j++)
               if (i != j)
                  err = LOCAL_ConnectPrepare (tmpSigs[i],
                           tmpSigs[j]->pInputs->pParent, LOCAL_SIG_OUT);
         /* PS: The pointer is implicitly valid by a check done above */
      }
   }

   if (err == IFX_SUCCESS)
       err = VINETIC_CON_ConnectConfigure (pDev);
   else
   {
       SET_ERROR(VIN_ERR_NO_FREE_INPUT_SLOT);
   }

   return (err == IFX_SUCCESS) ? IFX_SUCCESS : IFX_ERROR;
}

/**
   Removes a data channel from an analog phone device
\param pLLChannel  Handle to IFX_TAPI_LL_CH_t structure
\param pMap        Handle to IFX_TAPI_MAP_DATA_t structure
\return Return value according to IFX_return_t
   - IFX_ERROR if an error occured
   - IFX_SUCCESS if successful
\remarks
   Does now support conferencing
*/
IFX_return_t IFX_TAPI_LL_Data_Channel_Remove (IFX_TAPI_LL_CH_t *pLLChannel,
                                              IFX_TAPI_MAP_DATA_t const *pMap)
{
   VINETIC_CHANNEL *pCh = (VINETIC_CHANNEL *)pLLChannel;
   VINETIC_DEVICE *pDev = (VINETIC_DEVICE*) (pCh->pParent);
   IFX_uint32_t nThisCh = pCh->nChannel - 1;
   IFX_int32_t err = IFX_SUCCESS, i, cnt = 0, still_required;
   VINDSP_MODULE_t *pTmpMod;
   VINDSP_MODULE_SIGNAL *pInput = IFX_NULL, *pDstInput;
   VINDSP_MODULE_t *pDstMod = IFX_NULL,
                   *tmpSigs[MAX_MODULE_SIGNAL_INPUTS] = {IFX_NULL};
   VINETIC_CON_t   *pThisChModules = pDev->pChannel[nThisCh].pCON;

   switch (pMap->nChType)
   {
   case IFX_TAPI_MAP_TYPE_PCM:
      if (pMap->nDstCh < pDev->caps.nPCM)
         pDstMod = &pDev->pChannel[pMap->nDstCh].pCON->modPcm;
      break;
   case IFX_TAPI_MAP_TYPE_PHONE:
   case IFX_TAPI_MAP_TYPE_DEFAULT:
      if (pMap->nDstCh < pDev->caps.nALI)
         pDstMod = &pDev->pChannel[pMap->nDstCh].pCON->modAlm;
      break;
   case IFX_TAPI_MAP_TYPE_CODER:
   default:
      SET_ERROR (VIN_ERR_NOTSUPPORTED);
      return IFX_ERROR;
   }

   if ((pDstMod == IFX_NULL) ||
       (nThisCh >= pDev->caps.nSIG) || (nThisCh >= pDev->caps.nCOD))
   {
      SET_ERROR (VIN_ERR_FUNC_PARM);
      return IFX_ERROR;
   }

#ifdef TAPI_CID
   /* Prevent disconnection of Dst from SIG while CID is running.
      All other connections can be disconnected. This minimises blocking.
      With this lockout we also prevent difficulties with the otherwise
      possible reroute of signals to the local signalling input below. */
   if (TAPI_Cid_IsActive(pCh->pTapiCh) &&
       (pThisChModules->modSig.in[LOCAL_SIG_IN].i == pDstMod->nSignal))
   {
      SET_ERROR (VIN_ERR_CID_RUNNING);
      return IFX_ERROR;
   }
#endif /* TAPI_CID */


   /* disconnect SIG output from Dst (ALM or PCM) input */
   err = LOCAL_DisconnectPrepare (&pThisChModules->modSig,
                                  pDstMod, LOCAL_SIG_OUT);
   if (err == IFX_SUCCESS)
   {
      /* Disconnect Dst from either the COD or SIG module depending on where
         the output of the Dst is connected to. */
      if (pThisChModules->modSig.in[LOCAL_SIG_IN].i == pDstMod->nSignal)
         /* disconnect Dst (ALM or PCM) output from SIG input */
         err = LOCAL_DisconnectPrepare (pDstMod,
                                        &pThisChModules->modSig, 0);
      else
         /* disconnect Dst (ALM or PCM) output from COD input */
         err = LOCAL_DisconnectPrepare (pDstMod,
                                        &pThisChModules->modCod, 0);
   }

   /* Now the local input on the SIG on the data channel may be empty
      so reroute a signal from an input of the COD on the same channel when
      possible. Do not reroute signals coming from signalling modules */
   for (i = 0;
        err == IFX_SUCCESS && i <  MAX_MODULE_SIGNAL_INPUTS &&
        pThisChModules->modSig.in[LOCAL_SIG_IN].i == ECMD_IX_EMPTY; i++)
   {
      pTmpMod = pThisChModules->modCod.in[i].pOut;
      if (pTmpMod != IFX_NULL && pTmpMod->nModType != VINDSP_MT_SIG)
      {
         /* disconnect Dst (ALM or PCM) output from COD input */
         err = LOCAL_DisconnectPrepare (pTmpMod,
                                        &pThisChModules->modCod, 0);
         if (err == IFX_SUCCESS)
            /* connect Dst (ALM or PCM) output to SIG input (I1) */
            err = LOCAL_ConnectPrepare (pTmpMod,
                                        &pThisChModules->modSig, 0);
      }
   }

   /* Finally the output of our signalling module may still be connected to
      the coder on other data channels because the channels have been
      connected to the Dst (ALM or PCM) module that we just disconnected.
      So now check all data channels that we are connected to because of the
      Dst and if the connection is still needed because we are still in a
      conference where another ALM or PCM module connects our data channel
      with another one. */

   /* Store a pointer to each signalling module in a data channel that
      the Dst (ALM or PCM) still connects to in a temporary list. */
   for (pInput = pDstMod->pInputs, cnt = 0;
        pInput != IFX_NULL; pInput = pInput->pNext )
   {
      if (pInput->pParent->nModType == VINDSP_MT_SIG)
         tmpSigs[cnt++] = pInput->pParent;
      if (pInput->pParent->nModType == VINDSP_MT_COD)
         tmpSigs[cnt++] = pInput->pParent->pInputs->pParent;
   }
   /* Loop over the list just built */
   for (i = 0, still_required = 0;
        err == IFX_SUCCESS && i <  cnt && !still_required; i++)
   {
      /* Loop over all modules connecting to the data channel */
      for (pInput = tmpSigs[i]->pInputs2;
           pInput != IFX_NULL; pInput = pInput->pNext)
      {
         /* Skip COD modules connecting to the data channel */
         if (pInput->pParent->nModType == VINDSP_MT_COD)
            continue;
         /* For remaining modules check if any of the inputs connecting to
            the output is the signalling module we started with. If so then
            mark the connection as still required. */
         for (pDstInput = pInput->pParent->pInputs;
              pDstInput != IFX_NULL; pDstInput = pDstInput->pNext)
            if (pDstInput->pParent == &pThisChModules->modSig ||
                pDstInput->pParent == &pThisChModules->modCod   )
            {
               still_required = 1;
               break;
            }
      }
      /* If no longer required disconnect the data channels */
      if (! still_required)
      {
         err = LOCAL_DisconnectPrepare (&pThisChModules->modSig,
                                        tmpSigs[i]->pInputs->pParent,
                                        LOCAL_SIG_OUT);
         if (err == IFX_SUCCESS)
            err = LOCAL_DisconnectPrepare (tmpSigs[i],
                                           &pThisChModules->modCod,
                                           LOCAL_SIG_OUT);
      }
   }

   /* write the configuration to the chip */
   if (err == IFX_SUCCESS)
       err = VINETIC_CON_ConnectConfigure (pDev);
   else
   {
      SET_ERROR (VIN_ERR_CON_INVALID);
   }

   return (err == IFX_SUCCESS) ? IFX_SUCCESS : IFX_ERROR;
}

/**
   Connect an analog phone channel to another  (ALM or PCM)
\param pLLChannel  Handle to IFX_TAPI_LL_CH_t structure
\param pMap        Handle to IFX_TAPI_MAP_PHONE_t structure
\return Return value according to IFX_return_t
   - IFX_ERROR if an error occured
   - IFX_SUCCESS if successful
\remarks
*/
IFX_return_t IFX_TAPI_LL_Phone_Channel_Add (IFX_TAPI_LL_CH_t *pLLChannel,
                                            IFX_TAPI_MAP_PHONE_t const *pMap)
{
   VINETIC_CHANNEL *pCh = (VINETIC_CHANNEL *)pLLChannel;

   return LOCAL_Module_Connect (IFX_TAPI_CONN_ACTION_CREATE,
                                (VINETIC_DEVICE*)pCh->pParent,
                                pCh->nChannel - 1,
                                IFX_TAPI_MAP_TYPE_PHONE,
                                pMap->nPhoneCh, pMap->nChType);
}

/**
   Disconnect analog phone channels
\param pLLChannel  Handle to IFX_TAPI_LL_CH_t structure
\param pMap        Handle to IFX_TAPI_MAP_PHONE_t structure
\return Return value according to IFX_return_t
   - IFX_ERROR if an error occured
   - IFX_SUCCESS if successful
*/
IFX_return_t IFX_TAPI_LL_Phone_Channel_Remove (IFX_TAPI_LL_CH_t *pLLChannel,
                                               IFX_TAPI_MAP_PHONE_t const *pMap)
{
   VINETIC_CHANNEL *pCh = (VINETIC_CHANNEL *)pLLChannel;

   return LOCAL_Module_Connect (IFX_TAPI_CONN_ACTION_REMOVE,
                                (VINETIC_DEVICE*)pCh->pParent,
                                pCh->nChannel - 1,
                                IFX_TAPI_MAP_TYPE_PHONE,
                                pMap->nPhoneCh, pMap->nChType);
}

/**
   Adds a PCM module to either an analog phone module or another PCM module
\param pLLChannel  Handle to IFX_TAPI_LL_CH_t structure
\param pMap        Handle to IFX_TAPI_MAP_PCM_t structure
\return Return value according to IFX_return_t
   - IFX_ERROR if an error occured
   - IFX_SUCCESS if successful
\remarks
*/
IFX_return_t IFX_TAPI_LL_PCM_Channel_Add (IFX_TAPI_LL_CH_t *pLLChannel,
                                          IFX_TAPI_MAP_PCM_t const *pMap)
{
   VINETIC_CHANNEL *pCh = (VINETIC_CHANNEL *)pLLChannel;

   return LOCAL_Module_Connect (IFX_TAPI_CONN_ACTION_CREATE,
                                (VINETIC_DEVICE*)pCh->pParent,
                                pCh->nChannel - 1,
                                IFX_TAPI_MAP_TYPE_PCM,
                                pMap->nDstCh, pMap->nChType);
}

/**
   Remove a PCM module from either an analog phone module or another PCM module
\param pLLChannel  Handle to IFX_TAPI_LL_CH_t structure
\param pMap        Handle to IFX_TAPI_MAP_PCM_t structure
\return Return value according to IFX_return_t
   - IFX_ERROR if an error occured
   - IFX_SUCCESS if successful
\remarks
*/
IFX_return_t IFX_TAPI_LL_PCM_Channel_Remove (IFX_TAPI_LL_CH_t *pLLChannel,
                                             IFX_TAPI_MAP_PCM_t const *pMap)
{
   VINETIC_CHANNEL *pCh = (VINETIC_CHANNEL *)pLLChannel;

   return LOCAL_Module_Connect (IFX_TAPI_CONN_ACTION_REMOVE,
                                (VINETIC_DEVICE *)pCh->pParent,
                                pCh->nChannel - 1,
                                IFX_TAPI_MAP_TYPE_PCM,
                                pMap->nDstCh, pMap->nChType);
}

/**
   Create or remove a symetric connection between two local modules
   (PCM or ALM) on the same device
\param nAction     Action to be performed IFX_TAPI_CONN_ACTION_t
\param pDev        Pointer to the Vinetic device structure
\param nCh1        Resource number of the first module
\param nChType1    Which module type to use from channel 1
\param nCh2        Resource number of the second module
\param nChType2    Which module type to use from channel 2
\return Return value according to IFX_return_t
   - IFX_ERROR if an error occured
   - IFX_SUCCESS if successful
\remarks
*/
static IFX_return_t LOCAL_Module_Connect (IFX_TAPI_CONN_ACTION_t nAction,
                                          VINETIC_DEVICE *pDev,
                                          IFX_uint32_t nCh1,
                                          IFX_TAPI_MAP_TYPE_t nChType1,
                                          IFX_uint32_t nCh2,
                                          IFX_TAPI_MAP_TYPE_t nChType2)
{
   VINDSP_MODULE_t  *pMod1 = IFX_NULL,
                    *pMod2 = IFX_NULL;
   IFX_int32_t      err = IFX_SUCCESS;
   /* needed for SET_ERROR macro */
   VINETIC_CHANNEL* pCh = &pDev->pChannel[nCh1];

   /* get pointers to the signal struct of
      the selected modules in the channels */
   switch (nChType1)
   {
   case IFX_TAPI_MAP_TYPE_PCM:
      if (nCh1 < pDev->caps.nPCM)
         pMod1 = &pDev->pChannel[nCh1].pCON->modPcm;
      break;
   case IFX_TAPI_MAP_TYPE_PHONE:
   case IFX_TAPI_MAP_TYPE_DEFAULT:
      if (nCh1 < pDev->caps.nALI)
         pMod1 = &pDev->pChannel[nCh1].pCON->modAlm;
      break;
   case IFX_TAPI_MAP_TYPE_CODER:
   default:
      SET_ERROR (VIN_ERR_NOTSUPPORTED);
      return IFX_ERROR;
   }

   switch (nChType2)
   {
   case IFX_TAPI_MAP_TYPE_PCM:
      if (nCh2 < pDev->caps.nPCM)
         pMod2 = &pDev->pChannel[nCh2].pCON->modPcm;
      break;
   case IFX_TAPI_MAP_TYPE_PHONE:
   case IFX_TAPI_MAP_TYPE_DEFAULT:
      if (nCh2 < pDev->caps.nALI)
         pMod2 = &pDev->pChannel[nCh2].pCON->modAlm;
      break;
   case IFX_TAPI_MAP_TYPE_CODER:
   default:
      SET_ERROR (VIN_ERR_NOTSUPPORTED);
      return IFX_ERROR;
   }

   if ((pMod1 == IFX_NULL) || (pMod2 == IFX_NULL))
   {
      SET_ERROR (VIN_ERR_FUNC_PARM);
      return IFX_ERROR;
   }

   /* now pMod1 and pMod2 point to the modules to be connected/disconnected */

   if (nAction == IFX_TAPI_CONN_ACTION_CREATE)
   {
      /* connect the forward direction */
      err = LOCAL_ConnectPrepare (pMod1, pMod2, 0);
      if (err == IFX_SUCCESS)
      {
         /* connect the reverse direction */
         err = LOCAL_ConnectPrepare (pMod2, pMod1, 0);
      }
      if (err != IFX_SUCCESS)
      {
         SET_ERROR(VIN_ERR_NO_FREE_INPUT_SLOT);
      }
   }
   if (nAction == IFX_TAPI_CONN_ACTION_REMOVE)
   {
      /* disconnect the forward direction */
      err = LOCAL_DisconnectPrepare (pMod1, pMod2, 0);
      if (err == IFX_SUCCESS)
      {
         /* disconnect the reverse direction */
         err = LOCAL_DisconnectPrepare (pMod2, pMod1, 0);
      }
      if (err != IFX_SUCCESS)
      {
         SET_ERROR(VIN_ERR_CON_INVALID);
      }
   }

   if (err == IFX_SUCCESS)
      err = VINETIC_CON_ConnectConfigure (pDev);

   return (err == IFX_SUCCESS) ? IFX_SUCCESS : IFX_ERROR;
}

#ifdef TAPI_CID
/**
   Mute/Unmute all connections to modules which are attached to a data channel
   that is currently sending CID2 and should not listen to the CID.
\param pLLChannel      Handle to IFX_TAPI_LL_CH_t structure
\param nMute           IFX_TRUE: mute / IFX_FALSE: unmute
\return Return value according to IFX_return_t
   - IFX_ERROR if an error occured
   - IFX_SUCCESS if successful
\remarks
   This function is used to temporarily mute connections in the signalling
   array. This function finds all connections which need to be muted and
   mutes/unmutes them so that modules taking part in a conference will not
   hear the CID signal which is not intended from them.
*/
IFX_return_t IFX_TAPI_LL_Data_Channel_Mute (IFX_TAPI_LL_CH_t *pLLChannel,
                                            IFX_boolean_t nMute)
{
   VINETIC_CHANNEL *pCh = (VINETIC_CHANNEL *)pLLChannel;
   VINETIC_DEVICE *pDev = (VINETIC_DEVICE*) (pCh->pParent);
   IFX_uint32_t nThisCh = pCh->nChannel - 1;
   VINDSP_MODULE_t *pSigMod = IFX_NULL,
                   *pAnaMod = IFX_NULL;
   VINDSP_MODULE_SIGNAL *pTmpSig;
   IFX_int8_t mute = nMute ? 1 : -1;
   unsigned i;
   IFX_int32_t ret;

   pSigMod = &pDev->pChannel[nThisCh].pCON->modSig;
   /* find the analog module that is the one providing the input to SIG */
   for (pTmpSig = pSigMod->pInputs2;
        pTmpSig != NULL && (pAnaMod = pTmpSig->pParent,
        pSigMod->in[LOCAL_SIG_IN].i != pAnaMod->nSignal);
        pTmpSig = pTmpSig->pNext);
   if ( pAnaMod == NULL )
      /* no analogue module connected to this SIG module */
      return IFX_ERROR;

   /* 1. Flag the outputs on SIG and Analog module as muted so that all
         connections created from now on are also created in muted mode. */
   pSigMod->nMute = pSigMod->nMute2 = pAnaMod->nMute = nMute ? 1 : 0;
   pSigMod->modified = pAnaMod->modified = IFX_TRUE;

   /* 2. Find all inputs getting the local output of SIG and set them to muted
         mode but not the Analog module. Set then the  muted inputs to
         receive the input of the SIG module. */
   for (pTmpSig = pSigMod->pInputs2;
        pTmpSig != NULL;
        pTmpSig = pTmpSig->pNext)
   {
      if (pTmpSig->pParent != pAnaMod)
      {
         pTmpSig->mute += mute;
         if (pTmpSig->mute == 1 && pTmpSig->pParent->nMute == 0)
            pTmpSig->i_mute = pSigMod->in[REMOTE_SIG_IN].i;
         pTmpSig->pParent->modified = IFX_TRUE;
      }
   }

   /* 3. Mute the remote input of the SIG module */
   pSigMod->in[REMOTE_SIG_IN].mute += mute;

   /* 4. Mute all inputs of the Analog module except the one connected to
         the SIG module. */
   for (i = 0; i < MAX_MODULE_SIGNAL_INPUTS; i++)
   {
      if (pAnaMod->in[i].i != pSigMod->nSignal2)
      {
         pAnaMod->in[i].mute += mute;
         pAnaMod->in[i].i_mute = ECMD_IX_EMPTY;
      }
   }

   /* 5. Mute all inputs connected to the output of the Analog module but
         not the SIG module itself */
   for (pTmpSig = pAnaMod->pInputs;
        pTmpSig != NULL;
        pTmpSig = pTmpSig->pNext)
   {
      if (pTmpSig->pParent != pSigMod)
      {
         pTmpSig->mute += mute;
         pTmpSig->pParent->modified = IFX_TRUE;
      }
   }

   /* 6. Mute all inputs that receive their input from SIG out remote */
   for (pTmpSig = pSigMod->pInputs;
        pTmpSig != NULL;
        pTmpSig = pTmpSig->pNext)
   {
      pTmpSig->mute += mute;
      pTmpSig->pParent->modified = IFX_TRUE;
   }

   /* 7. For more comfort in the unmute case write the muted signal index on
         now only once muted inputs of analog modules that have been muted
         multiple times before. */
   for (i = 0; i < MAX_MODULE_SIGNAL_INPUTS; i++)
   {
      if ((pAnaMod->in[i].i != ECMD_IX_EMPTY) &&
          (pAnaMod->in[i].mute == 1) &&
          (pAnaMod->in[i].i_mute == ECMD_IX_EMPTY) &&
          (pAnaMod->in[i].pOut->nModType == VINDSP_MT_SIG) &&
          (pAnaMod->in[i].pOut->in[REMOTE_SIG_IN].mute != 0))
      {
         pAnaMod->in[i].i_mute = pAnaMod->in[i].pOut->in[REMOTE_SIG_IN].i;
      }
   }

   /* write the changed configuration to the chip */
   ret = VINETIC_CON_ConnectConfigure (pDev);

   return (ret == IFX_SUCCESS) ? IFX_SUCCESS : IFX_ERROR;
}
#endif /* TAPI_CID */

#endif /* (VIN_CFG_FEATURES & VIN_FEAT_VOICE) */



/**
   Add a signal connection to the internal shadow connection structure.
   Call Con_ConnectConfigure() to write the shadow structure to the chip.
\param pSrc      - handle to the source module which contains the output signal
\param pDst      - handle to the destination module where the output
                   is connected to
\param nSide     - specifies which output to use when the source is a
   signaling module, don't care for other module types (set to 0).
   If set to LOCAL_SIG_OUT then the output towards the local side is
   used and with REMOTE_SIG_OUT the output towards the remote side.
\return
   IFX_SUCCESS or IFX_ERROR
\remarks
   This function only connects two given modules if they are not already
   connected. The destination is set to modified if all was successful.
   The connection inherits a mute attribute from the module output that
   serves as the signal source for the connection. So when a connection
   is created on an output set to mute it will itself carry the mute
   attribute.
   As signalling modules have two sides (local/remote) to which other
   modules can be connected the side has to be determined first.
   When the signalling modules acts as an input rules are used to determine
   to which input side a module is connected:
   Local modules like ALM and PCM (not always) support auto suppression and
   get the event playout. They are connected to In 1.
   Remote modules like coder and PCM in case of advanced with DSP are
   connected to In 2.
   When the signalling module acts as an output nSignal is used to select
   which side of the signalling module is used for the connection. The value
   LOCAL_SIG_OUT is used for all analog channels. The value REMOTE_SIG_OUT
   is used for the coder only to connect to the Sig-OutA output.

                          nSignal
                          pInputs
   Signalling module:      |----------------|
                       <-  | Out A     In 1 |  <-
                           |  upstream (tx) |
          remote <==       |  -  -  -  -  - |       ==> local
                           | downstream (rx)|
                       ->  | In 2     Out B |  ->
                           |----------------|
                                      nSignal2
                                      pInputs2

   */
static IFX_return_t LOCAL_ConnectPrepare (VINDSP_MODULE_t* pSrc,
                                          VINDSP_MODULE_t* pDst,
                                          SIG_OUTPUT_SIDE nSide)
{
   int i, in = -1;
   VINDSP_MODULE_SIGNAL **pInput;
   IFX_uint8_t sig;
   IFX_uint8_t mute;

   /* get signal number and pointer to the output of the source module */
   if (nSide == LOCAL_SIG_OUT && pSrc->nModType == VINDSP_MT_SIG)
   {
      /* SIG module: use the local output (Out B) of the signalling module */
      pInput = &pSrc->pInputs2;
      sig    = pSrc->nSignal2;
      mute   = pSrc->nMute2;
   }
   else
   {
      /* PCM-, COD-, ALM-module: use the standard output of given module */
      /* SIG module: use the remote output (Out A) of the signalling module */
      pInput = &pSrc->pInputs;
      sig    = pSrc->nSignal;
      mute   = pSrc->nMute;
   }
   /* *pInput now points to the first input signal of the list of input signals
      connected to this output. sig contains the number of the signal in the
      signal array */

   /* return now if the output is already connected to one of the inputs */
   for (i = 0; i < MAX_MODULE_SIGNAL_INPUTS; i++)
   {
      if (pDst->in[i].i == sig)
         return IFX_SUCCESS;
   }
   /* find the index of a free input on the destination module */
   if (pDst->nModType == VINDSP_MT_SIG)
   {
      /* default input on signaling module is input 1 */
      in = LOCAL_SIG_IN;

      /* exception 1: coder connects always to input 2 */
      if (pSrc->nModType == VINDSP_MT_COD)
         in = REMOTE_SIG_IN;

#ifdef ENABLE_OBSOLETE_PREMAPPING
      /* exception 2 for PCM: in advanced PCM it does not act as ALM, but as
         coder. In that case the ALM must be connected first, which is
         assured in the basic init. Dynamically assignment for PCM with DSP
         is not supported */
      if (pSrc->nModType == VINDSP_MT_PCM && nSide == LOCAL_SIG_OUT)
         if (pDst->in[LOCAL_SIG_IN].i != ECMD_IX_EMPTY)
            in = REMOTE_SIG_IN;
#endif /* ENABLE_OBSOLETE_PREMAPPING */

      /* finally verify that the input selected is currently not in use */
      if (pDst->in[in].i != ECMD_IX_EMPTY)
      {
         TRACE(VINETIC, DBG_LEVEL_NORMAL,("Input signal on SIG already in use\n"));
         return IFX_ERROR;
      }
   }
   else
   {
      /* find free input on the destination module (COD, ALM or PCM) */
      for (i = 0; i < MAX_MODULE_SIGNAL_INPUTS; i++)
      {
         if (pDst->in[i].i == ECMD_IX_EMPTY)
         {
            in = i;
            break;
         }
      }
   }
#ifdef DEBUG
   TRACE(VINETIC, DBG_LEVEL_NORMAL,
          ("adding Signal %-10s to ", signalName[sig]));
   switch (pDst->nModType)
   {
   case VINDSP_MT_ALM:
      TRACE(VINETIC, DBG_LEVEL_NORMAL,("ALM"));
      break;
   case VINDSP_MT_PCM:
      TRACE(VINETIC, DBG_LEVEL_NORMAL,("PCM"));
      break;
   case VINDSP_MT_COD:
      TRACE(VINETIC, DBG_LEVEL_NORMAL,("COD"));
      break;
   case VINDSP_MT_SIG:
      TRACE(VINETIC, DBG_LEVEL_NORMAL,("SIG"));
      break;
   }
   TRACE (VINETIC, DBG_LEVEL_NORMAL,
          (" (Out-Sig %s) [input:%d] %s\n",
           signalName[pDst->nSignal], in, mute ? "muted" : ""));
#endif
   if (in == -1)
   {
      /* placed after the trace to get details on the modules (debug only) */
      TRACE(VINETIC,
            DBG_LEVEL_NORMAL,("No free input on destination module - stop\n"));
      return IFX_ERROR;
   }

   /* Connect output to the input */
   if (pDst->in[in].i != sig)
   {
      pDst->modified = IFX_TRUE;
      pDst->in[in].i = sig;
      pDst->in[in].mute += mute;
   }
   /* Set the pointer showing which output connects to this input */
   pDst->in[in].pOut = pSrc;
   /* Add the input to the linked list of all inputs attached to one output.
      Always add the element to the head of the list.*/
   pDst->in[in].pNext = *pInput;
   *pInput = &pDst->in[in];

   return IFX_SUCCESS;
}

/**
   Removes a signal connection from the internal shadow connection structure.
   Call Con_ConnectConfigure() to write the shadow structure to the chip.
\param pSrc      - handle to the source module which contains the output signal
\param pDst      - handle to the destination module where the output
                   is connected to
\param nSide     - specifies which output to use when the source is a
   signaling module, don't care for other module types (set to 0).
   If set to LOCAL_SIG_OUT then the output towards the local side is
   used and with REMOTE_SIG_OUT the output towards the remote side.
\return
   IFX_SUCCESS or IFX_ERROR
\remarks
   To remove a signal connection the input in the destination module has to
   be unlinked from the list the source module keeps of all inputs connecting
   to it's output. Then the input struct in the destination can be blanked.
   If all was successful the destination is set to modified.
   Disconnecting a non existing connection returns also IFX_SUCCESS.
*/
static IFX_return_t LOCAL_DisconnectPrepare (VINDSP_MODULE_t* pSrc,
                                             VINDSP_MODULE_t* pDst,
                                             SIG_OUTPUT_SIDE nSide)
{
   int i, in = -1;
   VINDSP_MODULE_SIGNAL **pBase,
                         *pTemp;
   IFX_uint8_t mute;

   /* find the input on the destination which is connected to the source */
   for (i = 0; i < MAX_MODULE_SIGNAL_INPUTS; i++)
   {
      if (pDst->in[i].pOut == pSrc)
      {
         in = i;
         break;
      }
   }
#ifdef DEBUG
   TRACE(VINETIC, DBG_LEVEL_NORMAL,
         ("removing Signal %-10s from ", signalName[pDst->in[in].i]));
   switch (pDst->nModType)
   {
   case VINDSP_MT_ALM:
      TRACE(VINETIC, DBG_LEVEL_NORMAL,("ALM"));
      break;
   case VINDSP_MT_PCM:
      TRACE(VINETIC, DBG_LEVEL_NORMAL,("PCM"));
      break;
   case VINDSP_MT_COD:
      TRACE(VINETIC, DBG_LEVEL_NORMAL,("COD"));
      break;
   case VINDSP_MT_SIG:
      TRACE(VINETIC, DBG_LEVEL_NORMAL,("SIG"));
      break;
   }
   TRACE (VINETIC, DBG_LEVEL_NORMAL,
          (" (Out-Sig %s) [input:%d]\n", signalName[pDst->nSignal], in));
#endif
   if (in == -1)
   {
      /* placed after the trace to get details on the modules (debug only) */
      TRACE(VINETIC, DBG_LEVEL_NORMAL,("Given modules are not connected\n"));
      return IFX_SUCCESS;
   }
   /* Get a pointer to the basepointer of the list of inputs connected. */
   if (nSide == LOCAL_SIG_OUT && pSrc->nModType == VINDSP_MT_SIG)
   {
      /* SIG-module only: inputs connected on local-side */
      pBase = &pSrc->pInputs2;
      mute = pSrc->nMute2;
   }
   else
   {
      /* PCM-, COD-, ALM-module: standard input */
      /* SIG module: inputs connected on remote-side */
      pBase = &pSrc->pInputs;
      mute = pSrc->nMute;
   }
   /* Get a pointer to the first element in the linked list of inputs. */
   pTemp = *pBase;
   /* pTemp now points to the first node in the list of input signals
      connected to this output. Now find the node of the input found
      above and remove this node from the linked list. */
   if (*pBase == &pDst->in[in])
   {
      /* special case: it is the first node so move the base pointer */
      *pBase = pDst->in[in].pNext;
   }
   else
   {
      /* Walk the list and try to find the node to be deleted. */
      while (pTemp != NULL && pTemp->pNext != &pDst->in[in])
         pTemp = pTemp->pNext;
      if (pTemp == NULL)
         /* not found! The data structure is corrupt - this should not happen*/
         return IFX_ERROR;
      /* unlink the node from the list */
      pTemp->pNext = pTemp->pNext->pNext;
   }
   /* clear this input -> connection is now removed */
   pDst->in[in].pNext = NULL;
   pDst->in[in].i     = ECMD_IX_EMPTY;
   pDst->in[in].i_mute= ECMD_IX_EMPTY;
   pDst->in[in].mute -= mute;
   pDst->in[in].pOut  = NULL;
   pDst->modified     = IFX_TRUE;

   return IFX_SUCCESS;
}



/** \addtogroup VINETIC_CONNECTIONS */
/* @{ */

/**
   Add a signal connection to the internal shadow connection structure.
   Call Con_ConnectConfigure() to write the shadow structure to the chip.
\param pDev      - pointer to VINETIC_DEVICE structure
\param ch        - channel number
\param src       - source module which contains the output signal
\param dst       - destination module where the output is connected to
\param nSide     - specifies which output to use when the source is a
   signaling module, don't care for other module types (set to 0).
   If set to LOCAL_SIG_OUT then the output towards the local side is
   used and with REMOTE_SIG_OUT the output towards the remote side.
\return
   IFX_SUCCESS or IFX_ERROR
\remarks
   This function only connects two given modules if they are not already
   connected. The destination is set to modified if all was successful.
   The connection inherits a mute attribute from the module output that
   serves as the signal source for the connection. So when a connection
   is created on an output set to mute it will itself carry the mute
   attribute.
   As signalling modules have two sides (local/remote) to which other
   modules can be connected the side has to be determined first.
   When the signalling modules acts as an input rules are used to determine
   to which input side a module is connected:
   Local modules like ALM and PCM (not always) support auto suppression and
   get the event playout. They are connected to In 1.
   Remote modules like coder and PCM in case of advanced with DSP are
   connected to In 2.
   When the signalling module acts as an output nSide is used to select
   which side of the signalling module is used for the connection. The value
   LOCAL_SIG_OUT is used for all analog channels. The value REMOTE_SIG_OUT
   is used for the coder only to connect to the Sig-OutA output.

                          nSignal
                          pInputs
   Signalling module:      |----------------|
                       <-  | Out A     In 1 |  <-
                           |  upstream (tx) |
          remote <==       |  -  -  -  -  - |       ==> local
                           | downstream (rx)|
                       ->  | In 2     Out B |  ->
                           |----------------|
                                      nSignal2
                                      pInputs2

   */
IFX_return_t VINETIC_CON_ConnectPrepare (VINETIC_DEVICE *pDev, IFX_uint8_t ch,
                                         VINDSP_MT src, VINDSP_MT dst,
                                         SIG_OUTPUT_SIDE nSide)
{
   VINDSP_MODULE_t *pSrc = IFX_NULL,
                   *pDst = IFX_NULL;

   switch (src)
   {
   case VINDSP_MT_ALM:
      pSrc = &pDev->pChannel[ch].pCON->modAlm;
      break;
   case VINDSP_MT_SIG:
      pSrc = &pDev->pChannel[ch].pCON->modSig;
      break;
   case VINDSP_MT_COD:
      pSrc = &pDev->pChannel[ch].pCON->modCod;
      break;
   case VINDSP_MT_PCM:
      pSrc = &pDev->pChannel[ch].pCON->modPcm;
      break;
   default:
      /* error: not an supported module type */
      TRACE(VINETIC, DBG_LEVEL_HIGH,
            (__FILE__ ":%d: src module type not supported\n", __LINE__));
      return IFX_ERROR;
   }

   switch (dst)
   {
   case VINDSP_MT_ALM:
      pDst = &pDev->pChannel[ch].pCON->modAlm;
      break;
   case VINDSP_MT_SIG:
      pDst = &pDev->pChannel[ch].pCON->modSig;
      break;
   case VINDSP_MT_COD:
      pDst = &pDev->pChannel[ch].pCON->modCod;
      break;
   case VINDSP_MT_PCM:
      pDst = &pDev->pChannel[ch].pCON->modPcm;
      break;
   default:
      /* error: not an supported module type */
      TRACE(VINETIC, DBG_LEVEL_HIGH,
            (__FILE__ ":%d: dst module type not supported\n", __LINE__));
      return IFX_ERROR;
   }

   return LOCAL_ConnectPrepare (pSrc, pDst, nSide);
}

/**
   Removes a signal connection from the internal shadow connection structure.
   Call Con_ConnectConfigure() to write the shadow structure to the chip.
\param pDev      - pointer to VINETIC_DEVICE structure
\param ch        - channel number
\param src       - source module which contains the output signal
\param dst       - destination module where the output is connected to
\param nSide     - specifies which output to use when the source is a
   signaling module, don't care for other module types (set to 0).
   If set to LOCAL_SIG_OUT then the output towards the local side is
   used and with REMOTE_SIG_OUT the output towards the remote side.
\return
   IFX_SUCCESS or IFX_ERROR
\remarks
   To remove a signal connection the input in the destination module has to
   be unlinked from the list the source module keeps of all inputs connecting
   to it's output. Then the input struct in the destination can be blanked.
   If all was successful the destination is set to modified.
   Disconnecting a non existing connection returns also IFX_SUCCESS.
*/
IFX_return_t VINETIC_CON_DisconnectPrepare (VINETIC_DEVICE *pDev, IFX_uint8_t ch,
                                            VINDSP_MT src, VINDSP_MT dst,
                                            SIG_OUTPUT_SIDE nSide)
{

   VINDSP_MODULE_t *pSrc = IFX_NULL,
                   *pDst = IFX_NULL;

   switch (src)
   {
   case VINDSP_MT_ALM:
      pSrc = &pDev->pChannel[ch].pCON->modAlm;
      break;
   case VINDSP_MT_SIG:
      pSrc = &pDev->pChannel[ch].pCON->modSig;
      break;
   case VINDSP_MT_COD:
      pSrc = &pDev->pChannel[ch].pCON->modCod;
      break;
   case VINDSP_MT_PCM:
      pSrc = &pDev->pChannel[ch].pCON->modPcm;
      break;
   default:
      /* error: not an supported module type */
      TRACE(VINETIC, DBG_LEVEL_HIGH,
            (__FILE__ ":%d: src module type not supported\n", __LINE__));
      return IFX_ERROR;
   }

   switch (dst)
   {
   case VINDSP_MT_ALM:
      pDst = &pDev->pChannel[ch].pCON->modAlm;
      break;
   case VINDSP_MT_SIG:
      pDst = &pDev->pChannel[ch].pCON->modSig;
      break;
   case VINDSP_MT_COD:
      pDst = &pDev->pChannel[ch].pCON->modCod;
      break;
   case VINDSP_MT_PCM:
      pDst = &pDev->pChannel[ch].pCON->modPcm;
      break;
   default:
      /* error: not an supported module type */
      TRACE(VINETIC, DBG_LEVEL_HIGH,
            (__FILE__ ":%d: dst module type not supported\n", __LINE__));
      return IFX_ERROR;
   }

   return LOCAL_DisconnectPrepare (pSrc, pDst, nSide);
}

/**
   Write the configuration from the internal shadow connection structure
   to the chip (firmware).
\param pDev     - handle to device structure
\return
   IFX_SUCCESS or IFX_ERROR
*/
IFX_return_t VINETIC_CON_ConnectConfigure (VINETIC_DEVICE *pDev)
{
   VINETIC_CHANNEL *pCh;
   IFX_return_t err = IFX_SUCCESS;
   IFX_uint8_t  ch;

   IFXOS_MutexLock (pDev->memberAcc);

   /* write ALM module inputs */
   for (ch = 0; (err == IFX_SUCCESS) && (ch < pDev->caps.nALI); ++ch)
   {
      pCh = &pDev->pChannel[ch];

      /* call the module specific set function only if something was modified */
      if (pCh->pCON->modAlm.modified != IFX_FALSE)
      {
         /* write ALM module inputs */
         err = VINETIC_ALM_Set_Inputs (pCh);
         /* clear the modified flag now that we updated the cached fw message */
         pCh->pCON->modAlm.modified = IFX_FALSE;
      }
   }

   /* write PCM module inputs */
   for (ch = 0; (err == IFX_SUCCESS) && (ch < pDev->caps.nPCM); ++ch)
   {
      pCh = &pDev->pChannel[ch];

      /* call the module specific set function only if something was modified */
      if (pCh->pCON->modPcm.modified != IFX_FALSE)
      {
         /* write PCM module inputs */
         err = VINETIC_PCM_Set_Inputs (pCh);
         /* clear the modified flag now that we updated the cached fw message */
         pCh->pCON->modPcm.modified = IFX_FALSE;
      }
   }

   /* write SIG module inputs */
   for (ch = 0; (err == IFX_SUCCESS) && (ch < pDev->caps.nSIG); ++ch)
   {
      pCh = &pDev->pChannel[ch];

      /* call the module specific set function only if something was modified */
      if (pCh->pCON->modSig.modified != IFX_FALSE)
      {
         /* write SIG module inputs */
         err = VINETIC_SIG_Set_Inputs (pCh);
         /* clear the modified flag now that we updated the cached fw message */
         pCh->pCON->modSig.modified = IFX_FALSE;
      }
   }

   /* write CODer module inputs */
   for (ch = 0; (err == IFX_SUCCESS) && (ch < pDev->caps.nCOD); ++ch)
   {
      pCh = &pDev->pChannel[ch];

      /* call the module specific set function only if something was modified */
      if (pCh->pCON->modCod.modified != IFX_FALSE)
      {
         /* write COD module inputs */
         err = VINETIC_COD_Set_Inputs (pCh);
         /* clear the modified flag now that we updated the cached fw message */
         pCh->pCON->modCod.modified = IFX_FALSE;
      }
   }

   IFXOS_MutexUnlock (pDev->memberAcc);

   return err;
}

/* @} */
