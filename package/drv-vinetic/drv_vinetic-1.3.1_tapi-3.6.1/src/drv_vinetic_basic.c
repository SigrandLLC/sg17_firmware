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
   Module      : drv_vinetic_basic.c
                 This file contains the implementation of basic VINETIC
                 access functions as WriteCmd, ReadCmd, register access
                 and so on.
*******************************************************************************/

/* ============================= */
/* Includes                      */
/* ============================= */
#include "drv_vinetic_api.h"
#include "drv_vinetic_basic.h"

/* ============================= */
/* Local Macros & Definitions    */
/* ============================= */

/* ============================= */
/* Global variable definition    */
/* ============================= */

/* array of partial values used to approach the function 10^X */
const IFX_uint32_t  tens  [28][2] =
{
   {    1,  100231},
   {    2,  100462},
   {    3,  100693},
   {    4,  100925},
   {    5,  101158},
   {    6,  101391},
   {    7,  101625},
   {    8,  101859},
   {    9,  102094},
   {   10,  102329},
   {   20,  104713},
   {   30,  107152},
   {   40,  109648},
   {   50,  112202},
   {   60,  114815},
   {   70,  117490},
   {   80,  120226},
   {   90,  123027},
   {  100,  125893},
   {  200,  158489},
   {  300,  199526},
   {  400,  251189},
   {  500,  316228},
   {  600,  398107},
   {  700,  501187},
   {  800,  630957},
   {  900,  794328},
   { 1000, 1000000}
};

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
#ifdef FW_ETU
/**
   This function maps the EPOU field of the VINETIC firmware message
   "Read_EPOU_Trigger (RTP Support)" to an equivalent RFC2833 event number.

\param
    epou - EPOU field of the VINETIC firmware message
\return
      returns the RFC2833 event number. Returns (-1) if unknown event
\remarks
*/
IFX_LOCAL IFX_int32_t MapEpouTriggerToRfc2833(IFX_uint16_t epou)
{
   IFX_int32_t rfcEvent;

   if (epou & SIG_EPOUTRIG_DTMF)
   {
      /* DTMF tone received */
      rfcEvent = (epou & SIG_EPOUTRIG_DTMFCODE) >> SIG_EPOUTRIG_DTMFBIT;
   }
   else if (epou & SIG_EPOUTRIG_ANS)
   {
      /* ANS tone received */
      rfcEvent = IFX_TAPI_PKT_EV_NUM_ANS;
   }
   else if (epou & SIG_EPOUTRIG_NANS)
   {
      /* /ANS tone received */
      rfcEvent = IFX_TAPI_PKT_EV_NUM_NANS;
   }
   else if (epou & SIG_EPOUTRIG_ANSAM)
   {
      /* ANSam tone received */
      rfcEvent = IFX_TAPI_PKT_EV_NUM_ANSAM;
   }
   else if (epou & SIG_EPOUTRIG_NANSAM)
   {
      /* /ANSam tone received */
      rfcEvent = IFX_TAPI_PKT_EV_NUM_NANSAM;
   }
   else if (epou & SIG_EPOUTRIG_CNG)
   {
      /* CNG tone received */
      rfcEvent = IFX_TAPI_PKT_EV_NUM_CNG;
   }
   else if (epou & SIG_EPOUTRIG_DIS)
   {
      /* DIS tone received */
      rfcEvent = IFX_TAPI_PKT_EV_NUM_DIS;
   }
   else
   {
      rfcEvent = (-1);
   }

   return rfcEvent;
}
#endif /* FW_ETU */


/* ============================= */
/* Global function definition    */
/* ============================= */

/**
   Dispatches the command read, by copying it into the appropriate pDev/pCh
   command caching buffer.
\param
   pDev      - pointer to the device
\param
   pData     - data containing the command
\return
   none
\remarks
   this function must be protected from calling function against concurent
   access or interrupts susceptible to modify the contain of cached fw messages
   The firmware semaphore should be used for locking.
*/
IFX_void_t VINETIC_DispatchCmd (VINETIC_DEVICE *pDev, IFX_uint16_t const *pData)
{
/*FIXME: Olaf*/
#if 0
   IFX_uint8_t i, len, ch;
   ch = pData[0] & CMD1_CH;
   switch (pData[0] & CMD1_CMD)
   {
   case  CMD1_EOP:
      len = (pData[1] & CMD2_LEN);
      switch ((pData[1] & ~CMD2_LEN) & (CMD2_ECMD | CMD2_MOD) )
      {
      case ECMD_CID_SEND:
         /* check if ch matches to number of signalling ressources */
         if (ch >= pDev->caps.nSIG)
            break;
         pDev->pSigCh[ch].cid_sender [CMD_HEADER_CNT] = pData[CMD_HEADER_CNT];
         break;
      case ECMD_SIG_CH:
         /* check if ch matches to number of signalling ressources */
         if (ch >= pDev->caps.nSIG)
            break;
         /* This message has a minimum length of one data word... */
         pDev->pSigCh[ch].sig_ch.value[CMD_HEADER_CNT] = pData[CMD_HEADER_CNT];
         break;
      case ECMD_ALM_CH:
         /* check if ch matches to number of analog ressources */
         if (ch >= pDev->caps.nALI)
            break;
         for (i = 0; (i < len) && (i < CMD_ALM_CH_LEN); i++)
            pDev->pAlmCh[ch].ali_ch.value[CMD_HEADER_CNT + i] = pData[i];
         break;
      case ECMD_PCM_CH:
         /* check if ch matches to number of analog ressources */
         if (ch >= pDev->caps.nPCM)
            break;
         for (i = 0; (i < len) && (i < CMD_PCM_CH_LEN); i++)
            pDev->pPcmCh[ch].pcm_ch.value[CMD_HEADER_CNT + i] = pData[i];
         break;
      case ECMD_COD_CH:
         /* check if ch matches to number of coder ressources */
         if (ch >= pDev->caps.nCOD)
            break;
         for (i = 0; (i < len) && (i < CMD_COD_CH_LEN); i++)
            pDev->pCodCh[ch].cod_ch.value[CMD_HEADER_CNT + i] = pData[i];
         break;
      case ECMD_COD_CHRTCP:
         /* check if ch matches to number of coder ressources */
         if (ch >= pDev->caps.nCOD)
            break;
         for (i = 0; (i < len) && (i < CMD_COD_RTCP_LEN); i++)
            pDev->pCodCh[ch].rtcp[i] = pData[CMD_HEADER_CNT + i];
         /* generate event ? */
         pDev->pCodCh[ch].rtcp_update = IFX_TRUE;
         break;
#ifdef FW_ETU
      case ECMD_EPOU_TRIG:
     {
         IFX_TAPI_EVENT_t tapiEvent;
         IFX_return_t ret;
         TAPI_CHANNEL *pChannel;
         IFX_uint32_t rfc2833Event;

         /* check if ch matches to number of tapi ressources */
         if (ch >= pDev->TapiDev.nMaxChannel)
            break;

         rfc2833Event = (IFX_uint32_t)MapEpouTriggerToRfc2833(pData[CMD_HEADER_CNT]);
         pChannel = &pDev->TapiDev.pTapiChanelArray[ch];

         /* Fill event structure. */
         memset(&tapiEvent, 0, sizeof(IFX_TAPI_EVENT_t));
         tapiEvent.id = IFX_TAPI_EVENT_RFC2833_EVENT;
         tapiEvent.data.rfc2833.event = rfc2833Event;

         ret = IFX_TAPI_Event_Dispatch(pChannel,&tapiEvent);
         if(ret == IFX_ERROR)
         {
            /* \todo if dispatcher error?? */
         }
         break;
      }
#endif /* FW_ETU */
      default:
         /* nothing of interest */
         break;
      }
      break;
   default:
      break;
   }
#else
 pDev = pDev;
 pData = pData;
#endif /* Olaf */
}


#ifdef VIN_2CPE
/**
   Ioctl handling function used to write a single host register (2CPE)
\param
   pDev  - pointer to device data
\param
   pCmd  - pointer to command data
\return
   returns IFX_SUCCESS, always
*/
IFX_int32_t VINETIC_HostRegWr_Cmd(VINETIC_DEVICE *pDev,
                                  VINETIC_IO_REG_ACCESS *pCmd)
{
   REG_WRITE_PROT_MULTI(pDev, pCmd->offset, pCmd->pData, pCmd->count);
   if (pDev->err != VIN_ERR_OK)
      return IFX_ERROR;

   return IFX_SUCCESS;
}

/**
   Ioctl handling function used to read a single host register (2CPE)
\param
   pDev  - pointer to device data
\param
   pCmd  - pointer to command data
\return
   returns IFX_SUCCESS, always
*/
IFX_int32_t VINETIC_HostRegRd_Cmd(VINETIC_DEVICE *pDev,
                                  VINETIC_IO_REG_ACCESS *pCmd)
{
   REG_READ_PROT_MULTI(pDev, pCmd->offset, pCmd->pData, pCmd->count);
   if (pDev->err != VIN_ERR_OK)
      return IFX_ERROR;

   return IFX_SUCCESS;
}
#else
/**
   Write short command
   pDev: pointer to the device interface
   pCmd: pointer to command data
\return
    IFX_SUCCESS if no error, otherwise IFX_ERROR
*/
IFX_int32_t VINETIC_Write_Sc(VINETIC_DEVICE *pDev, VINETIC_IO_WRITE_SC *pCmd)
{
   IFX_uint16_t cmd = 0;
   IFX_int32_t  ret = IFX_SUCCESS;

   /* Set write cmd and write */
   cmd = CMD1_WR | CMD1_SC | pCmd->nCmd;
   if(pCmd->nBc)
   {
      cmd |= CMD1_BC;
   }
   cmd |= (pCmd->nCh & CMD1_CH);
   ret = ScWrite (pDev, cmd);
   return ret;
}

/**
   Read short command
   pDev: pointer to the device interface
   pCmd: pointer to command data
\return
    IFX_SUCCESS if no error, otherwise IFX_ERROR
*/
IFX_int32_t VINETIC_Read_Sc(VINETIC_DEVICE *pDev, VINETIC_IO_READ_SC *pCmd)
{
   IFX_uint16_t cmd = 0;
   IFX_int32_t  err = IFX_SUCCESS;

   /* set parameters */
   cmd = CMD1_RD | CMD1_SC | pCmd->nCmd;
   if (pCmd->nBc)
   {
      cmd |= CMD1_BC;
   }
   cmd |= (pCmd->nCh & CMD1_CH);
   err = ScRead (pDev, cmd, pCmd->pData, pCmd->count);

   return err;
}
#endif /* VIN_2CPE */

/**
   Write VINETIC command
\param
   pDev  - pointer to the device interface
\param
   pCmd  - pointer to command data
\return
    IFX_SUCCESS if no error, otherwise IFX_ERROR (return value of CmdWrite)
*/
IFX_int32_t VINETIC_Write_Cmd (VINETIC_DEVICE *pDev, VINETIC_IO_MB_CMD *pCmd)
{
   IFX_int32_t  ret;
   IFX_uint16_t *pData = (IFX_uint16_t*)((IFX_void_t *)pCmd);

   if ((pData[0] & CMD1_CMD) == CMD1_EOP && (pData[0] & CMD1_RD) != CMD1_RD)
   {
      IFXOS_MutexLock   (pDev->memberAcc);
      VINETIC_DispatchCmd (pDev, pData);
   }
   ret = CmdWrite (pDev, pData, (pCmd->cmd2 & CMD2_LEN));
   if ((pData[0] & CMD1_CMD) == CMD1_EOP)
   {
      IFXOS_MutexUnlock (pDev->memberAcc);
   }

   return ret;
}

/**
   Write command to read data
\param
   pDev  - pointer to the device interface
\param
   pCmd  - pointer to command data
\return
    IFX_SUCCESS if no error, otherwise IFX_ERROR
*/
IFX_int32_t VINETIC_Read_Cmd (VINETIC_DEVICE *pDev, VINETIC_IO_MB_CMD *pCmd)
{
   IFX_int32_t err;

   err = CmdRead (pDev, (IFX_uint16_t*)((IFX_void_t *)pCmd),
                  (IFX_uint16_t*)((IFX_void_t *)pCmd), pCmd->cmd2 & CMD2_LEN);

   /* RW bit might have been cleared by CmdRead in order to allow subsequent
      write operations (such as read/modify/write) */
   pCmd->cmd1 |= CMD1_RW;

   /* if read failed set the cmd length to 0 */
   if (err != IFX_SUCCESS)
      pCmd->cmd2 = 0;

   return err;
}
