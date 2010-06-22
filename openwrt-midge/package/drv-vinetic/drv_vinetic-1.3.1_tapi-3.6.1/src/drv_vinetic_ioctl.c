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
   Module      : drv_vinetic_ioctl.c
   Desription  : Contains ioctl specific implementations according to io type.
*******************************************************************************/

/* ============================= */
/* Includes                      */
/* ============================= */

#include "drv_vinetic_api.h"
#include "drv_vinetic_main.h"
#include "drv_vinetic_basic.h"
#include "drv_vinetic_dwnld.h"
#include "drv_vinetic_int.h"
#include "drv_vinetic_misc.h"
#include "drv_vinetic_stream.h"
#include "drv_vinetic_sig_cid.h"
#include "drv_vinetic_gpio_user.h"

#ifdef VIN_2CPE
#include "drv_vinetic_bbd.h"
#else
/* currently nothing */
#endif /* VIN_2CPE */

#if (VIN_CFG_FEATURES & VIN_FEAT_LT)
#include "drv_vinetic_lt.h"
#endif /* VIN_CFG_FEATURES & VIN_FEAT_LT */

#if ((VIN_CFG_FEATURES & VIN_FEAT_GR909) && defined (VIN_V14_SUPPORT))
#include "drv_vinetic_gr909.h"
#endif /* VIN_CFG_FEATURES & VIN_FEAT_GR909  &&  VIN_V14_SUPPORT*/


/* ============================= */
/* Local macros                  */
/* ============================= */


/**
   Dispatches ioctrl commands which doesn't need data management

\param   pContext - context pointer, may be device or channel pointer.
\param   msg      - ioctrl command id
\param   func     - function name
\param   arg      - argument for the function, cast before if needed.
*/
#define  ON_DIR_IOCTL(pContext,msg,func,arg)\
            case (msg):\
               ret = func((pContext),arg);\
               break

/**
   Dispatches ioctrl commands which doesn't need arguments

\param   pContext - context pointer, may be device or channel pointer.
\param   msg      - ioctrl command id
\param   func     - function name
*/
#define  ON_IOCTL_NOARG(pContext,msg,func)\
            case (msg):\
               ret = func((pContext));\
               break

/**
   Dispatches ioctrl commands and manages user space data handling in both
   directions.

\param   pContext - context pointer, may be device or channel pointer.
\param   msg      - ioctrl command id
\param   func     - function name
\param   arg      - structure identifier used as argument for the function
                    call and for copying user data
\remarks
   As an alternative use a transfer structure VINETIC_IO_USR and
   DoDataExchange for handling data exchange.

\code
   // call function VINETIC_Read_Cmd if id VINETIC_RMAIL received
   IFX_uint8_t IOcmd [660];

   switch (nCmd)
   {
      ON_IOCTL(VINETIC_RMAIL, VINETIC_Read_Cmd, IO_READ_CMD);
   }
\endcode
*/
#ifdef LINUX
#define ON_IOCTL(pContext,msg,func,arg)\
           case (msg):\
              {\
              arg* p_arg = IFXOS_MALLOC(sizeof(arg));\
              IFXOS_ASSERT (p_arg != IFX_NULL);\
              copy_from_user (p_arg, (IFX_uint8_t*)ioarg, sizeof(arg));\
              ret = func((pContext), p_arg);\
              copy_to_user ((IFX_uint8_t*)ioarg, p_arg, sizeof(arg));\
              IFXOS_FREE (p_arg);\
              }\
              break
#else
#define ON_IOCTL(pContext,msg,func,arg)\
           case (msg):\
              ret = func((pContext),(arg*)ioarg);\
              break
#endif /* LINUX */

/**
   Dispatches ioctrl commands and manages user space data handling in
   driver direction.

\param   pContext - context pointer, may be device or channel pointer.
\param   msg      - ioctrl command id
\param   func     - function name
\param   arg      - structure identifier used as argument for the function
                    call and for copying user data
*/
#ifdef LINUX
#define ON_IOCTL_FRUSR(pContext,msg,func,arg)\
         case (msg):\
            {\
              arg* p_arg = IFXOS_MALLOC(sizeof(arg));\
              IFXOS_ASSERT (p_arg != IFX_NULL);\
              copy_from_user (p_arg, (IFX_uint8_t*)ioarg, sizeof(arg));\
              ret = func((pContext), p_arg);\
              IFXOS_FREE (p_arg);\
            }\
            break
#else
#define ON_IOCTL_FRUSR(pContext,msg,func,arg)\
           ON_IOCTL((pContext),msg,func,arg)
#endif /* LINUX */

/**
   Dispatches ioctrl commands and manages user space data handling in
   user space direction.

\param   pContext - context pointer, may be device or channel pointer.
\param   msg      - ioctrl command id
\param   func     - function name
\param   arg      - structure identifier used as argument for the function
                    call and for copying user data
*/
#ifdef LINUX
#define ON_IOCTL_TOUSR(pContext,msg,func,arg)\
           case (msg):\
            {\
              arg* p_arg = IFXOS_MALLOC(sizeof(arg));\
              IFXOS_ASSERT (p_arg != IFX_NULL);\
              ret = func((pContext),(arg*)p_arg);\
              copy_to_user ((IFX_uint8_t*)ioarg, p_arg, sizeof(arg));\
              IFXOS_FREE (p_arg);\
            }\
            break
#else
#define ON_IOCTL_TOUSR(pContext,msg,func,arg)\
            ON_IOCTL((pContext),msg,func,arg)
#endif /* LINUX */

/**
   Dispatches ioctrl commands and manages user space data handling in
   user space direction with exchange structure.

\param   pContext - context pointer, may be device or channel pointer.
\param   msg      - ioctrl command id
\param   func     - function name
\todo    check if still required or used only in evaluation code
*/
#ifdef LINUX
#define ON_EX_IOCTL(pContext,msg,func)\
           case (msg):\
              {\
              VINETIC_IO_USR trans;\
              trans.pWrBuf = IFXOS_MALLOC(2048);\
              trans.pRdBuf = IFXOS_MALLOC(2048);\
              IFXOS_ASSERT ((trans.pWrBuf != IFX_NULL) && \
                            (trans.pRdBuf != IFX_NULL));\
              DoDataExchange (&trans, ioarg, 1);\
              ret = func((pContext), &trans);\
              DoDataExchange (&trans, ioarg, 0);\
              IFXOS_FREE(trans.pWrBuf);\
              IFXOS_FREE(trans.pRdBuf);\
              }\
              break
#else
#define ON_EX_IOCTL(pContext,msg,func)\
            ON_IOCTL((pContext),(msg),func,VINETIC_IO_USR)
#endif /* LINUX */

/* ============================= */
/* Local variable definition     */
/* ============================= */

/* ============================= */
/* Global variable definition    */
/* ============================= */

/* ============================= */
/* Local function declaration    */
/* ============================= */
#if (defined (EVALUATION) && defined (LINUX))
#ifndef VIN_2CPE /* temporary define to avoid caused warning. will be removed
                    when this function will be needed by the 2CPE project */
IFX_LOCAL IFX_void_t DoDataExchange (VINETIC_IO_USR* pTrans, IFX_uint32_t nArg,
                                     IFX_int32_t bFrom);
#endif /* VIN_2CPE */
#endif /* EVALUATION && LINUX */

/* ============================= */
/* Local function definition     */
/* ============================= */

#if (defined (EVALUATION) && defined (LINUX))
#ifndef VIN_2CPE /* temporary define to avoid caused warning. will be removed
                    when this function will be needed by the 2CPE project */
/**
   Transfers data from or to user space.

\param   pTrans - transfer structure which then holds the current data
\param   nArg   - argument from IOCtl
\param   bFrom  - if 1 transfers data from user to kernel space, otherwise
                  from kernel to user space
\remarks
   - Uses two global arrays for kernel space data: IOcmd for write data and
     IORd for returned data.
   - DoDataExchange must be called with bFrom = 1 before any driver function
     call, cause transfer structure fields are initialized there.
     A second call after the function call is only necessary if data is
     returned.
   - No copy actions are done, if number of write or read bytes is 0.
*/
IFX_LOCAL IFX_void_t DoDataExchange(VINETIC_IO_USR* pTrans,
                                 IFX_uint32_t  nArg, IFX_int32_t bFrom)
{
   VINETIC_IO_USR* pUsr = (VINETIC_IO_USR*)nArg;

   if (bFrom)
   {
      copy_from_user (pTrans, (IFX_uint8_t*)nArg,
                      sizeof (VINETIC_IO_USR));
      if (pTrans->pWrBuf == NULL)
         pTrans->nWrCnt = 0;
      if (pTrans->nWrCnt > 0)
         copy_from_user (pTrans->pWrBuf, pUsr->pWrBuf, pTrans->nWrCnt);
   }
   else
   {
      if (pUsr->pRdBuf == NULL)
         pTrans->nRdCnt = 0;
      if (pTrans->nRdCnt > 0)
         copy_to_user (pUsr->pRdBuf, pTrans->pRdBuf, pTrans->nRdCnt);
      pUsr->nRdCnt = pTrans->nRdCnt;
   }
}
#endif /* VIN_2CPE */
#endif /* EVALUATION && LINUX */

/* ============================= */
/* Global function definition    */
/* ============================= */

#ifdef EVALUATION
/**
   Vinetic Evaluation ioctl handling.

\param pCh     - handle to VINETIC_CHANNEL structure
\param iocmd   - ioctl command
\param ioarg   - ioctl argument

\return

\remark
   In case the device ptr handle is needed, it will be accessed via
   the pParent of the first channel ptr pChannel[0].pParent. The valid ptr to
   that channel structure will then be passed by the overlying function.
*/
IFX_int32_t VINETIC_Eval_Ioctl (VINETIC_CHANNEL *pCh, IFX_uint32_t iocmd,
                                IFX_uint32_t ioarg)
{
   IFX_int32_t ret = IFX_SUCCESS;
   VINETIC_DEVICE *pDev = pCh->pParent;

   /* do not allow other ioctl if basic init is missing! */
   if ((pDev->nDevState & DS_BASIC_INIT) != DS_BASIC_INIT)
   {
      SET_ERROR(VIN_ERR_NOINIT);
      return IFX_ERROR;
   }

   switch (iocmd) /* split in 2cpe and non 2cpe parts */
   {
   /* Common eval ioctls  */
   ON_IOCTL       (pDev, FIO_VINETIC_LEC_TEST, VINETIC_LecTest,
                   VINETIC_IO_LEC);
   ON_IOCTL       (pDev, FIO_VINETIC_INTCOUNT,
                   VINETIC_Interrupt_Count, VINETIC_IO_INT_COUNT);
#ifdef VIN_2CPE
   /* 2CPE eval ioctls */
   ON_DIR_IOCTL   (pDev, FIO_VINETIC_INTCONFIG, VINETIC_IntEvtCtrl, ioarg);
   ON_IOCTL_TOUSR (pDev, FIO_VINETIC_INTINFO, VINETIC_GetIntEvt,
                   VINETIC_IntEvt_t);
   ON_IOCTL_FRUSR (pCh, FIO_VINETIC_VOICESTREAM_CTRL, VINETIC_VoiceStream_Ctrl,
                   VINETIC_IO_VOICESTREAM_t);
   ON_IOCTL       (pDev, FIO_VINETIC_MBX_TEST, VINETIC_MbxTest_Ctrl,
                   VINETIC_IO_MBX_TESTS);
#else
   /* Non 2cpe eval ioctls */
   /* TODO: irrelevant evaluation ioctls for 2CPE.
      Check relevance for 2CPE and include ioctls accordingly, as this is
      actually only an assumption
   */
   /* PCM Module */
   ON_EX_IOCTL    (pDev, FIO_VINETICPCM_INTCONT,  VINETIC_Pcm_Control);
   ON_EX_IOCTL    (pDev, FIO_VINETICPCM_INTCHAN,  VINETIC_PcmCh);
   ON_EX_IOCTL    (pDev, FIO_VINETICPCM_NELEC,    VINETIC_Pcm_NeLec);
   ON_EX_IOCTL    (pDev, FIO_VINETICPCM_FELEC,    VINETIC_Pcm_FeLec);
   ON_EX_IOCTL    (pDev, FIO_VINETICPCM_RBS,      VINETIC_Pcm_Rbs);
   /* Line Interface Module */
   ON_EX_IOCTL    (pDev, FIO_VINETICSLI_INTCON,   VINETIC_Ali_Control);
   ON_EX_IOCTL    (pDev, FIO_VINETICSLI_CHAN,     VINETIC_AliCh);
   ON_EX_IOCTL    (pDev, FIO_VINETICSLI_NELEC,    VINETIC_Ali_NeLec);
   ON_EX_IOCTL    (pDev, FIO_VINETICSLI_FELEC,    VINETIC_Ali_FeLec);
   /* Signalling Module*/
   ON_EX_IOCTL    (pDev, FIO_VINETICSIG_CON,      VINETIC_Sign_Control);
   ON_EX_IOCTL    (pDev, FIO_VINETICSIG_CHAN,     VINETIC_SignCh);
   ON_EX_IOCTL    (pDev, FIO_VINETICSIG_CIDTX,    VINETIC_Sign_CidTx);
   ON_EX_IOCTL    (pDev, FIO_VINETICSIG_DTMFAT,   VINETIC_Sign_Dtmf_At);
   ON_EX_IOCTL    (pDev, FIO_VINETICSIG_DTMFRX,   VINETIC_Sign_Dtmf_Rx);
   ON_EX_IOCTL    (pDev, FIO_VINETICSIG_ATD1,     VINETIC_Sign_Atd1);
   ON_EX_IOCTL    (pDev, FIO_VINETICSIG_ATD2,     VINETIC_Sign_Atd2);
   ON_EX_IOCTL    (pDev, FIO_VINETICSIG_UTD1 ,    VINETIC_Sign_Utd1);
   ON_EX_IOCTL    (pDev, FIO_VINETICSIG_UTD2,     VINETIC_Sign_Utd2);
   ON_EX_IOCTL    (pDev, FIO_VINETIC_SIGCHACO,    VINETIC_SignCh_Config_RTP);
   ON_EX_IOCTL    (pDev, FIO_VINETIC_SIGCHACOAAL, VINETIC_SignCh_Config_AAL);
   /* Coder Module */
   ON_EX_IOCTL    (pDev, FIO_VINETICCOD_CON,      VINETIC_CoderControl);
   ON_EX_IOCTL    (pDev, FIO_VINETICCOD_CHAN,     VINETIC_CoderCh);
   ON_EX_IOCTL    (pDev, FIO_VINETICCOD_FELEC,    VINETIC_Coder_Felec);
   ON_EX_IOCTL    (pDev, FIO_VINETICCOD_AGC,      VINETIC_CODER_Agc);
   ON_EX_IOCTL    (pDev, FIO_VINETIC_COCHCORTP,   VINETIC_CoderCh_Conf_RTP);
   ON_EX_IOCTL    (pDev, FIO_VINETIC_COCHCOAAL,   VINETIC_CoderCh_Conf_AAL);
   ON_EX_IOCTL    (pDev, FIO_VINETIC_CCJB,        VINETIC_CoderCh_Conf_JB);
   ON_EX_IOCTL    (pDev, FIO_VINETIC_COCHJBSTAT,  VINETIC_CoderCh_JB_Stats);
   ON_EX_IOCTL    (pDev, FIO_VINETIC_COCONRTP,    VINETIC_Coder_Conf_RTP);
   ON_EX_IOCTL    (pDev, FIO_VINETIC_COCHSTAT,    VINETIC_CoderCh_Stats);
   /* Tone Module */
   ON_IOCTL       (pDev, FIO_VINETIC_DTMFCOUNT, VINETIC_Tone_Detection,
                   VINETIC_IO_TONE_DET);
   ON_IOCTL       (pDev, FIO_VINETIC_DIAL_DETECTION, VINETIC_Dial_Detection,
                   VINETIC_IO_DIAL_DETECTION);
   /* CID Module */
   ON_IOCTL       (pDev, FIO_VINETIC_CIDTX,
                   VINETIC_CID_Sending, VINETIC_IO_CID_SEND);
   /* Tone Module */
   ON_IOCTL       (pDev, FIO_VINETIC_DTMF_TOGEN, VINETIC_DTMF_ToneGen,
                   VINETIC_IO_DTMF_TONGEN);
#if (defined(ADDED_SUPPORT) || defined (EVALUATION))
   /* misc module */
   ON_IOCTL(pDev, FIO_VINETIC_MASK, VINETIC_Interrupt_Masking,
         VINETIC_IO_INT_MASKING);
   ON_IOCTL       (pDev, FIO_VINETIC_INTINFO, VINETIC_Interrupt_Information,
                   VINETIC_IO_USR);
   ON_IOCTL       (pDev, FIO_VINETIC_INTCONFIG,
                   VINETIC_Interrupt_Config, VINETIC_IO_INT_CONF);
#ifdef ADDED_SUPPORT
   ON_IOCTL       (pDev, FIO_VINETIC_INTEVENT,  VINETIC_Interrupt_Event,
                   VINETIC_IO_INT_EVT);
   ON_IOCTL       (pDev, FIO_VINETIC_INTERROR,  VINETIC_Interrupt_Error_Status,
                   VINETIC_IO_USR);
#endif /* ADDED_SUPPORT */
#endif /* ADDED_SUPPORT || EVALUATION*/
   /* Basic Module */
   ON_IOCTL       (pDev, FIO_VINETIC_MS, VINETIC_Modify_AIm,
                   VINETIC_IO_MODIFY_ALM);
   ON_IOCTL       (pDev, FIO_VINETIC_WDSP, VINETIC_Write_EDSP,
                   VINETIC_IO_EDSP);
   ON_IOCTL       (pDev, FIO_VINETIC_RDSP,
                   VINETIC_Read_EDSP, VINETIC_IO_EDSP);
   ON_IOCTL_FRUSR (pDev, FIO_VINETIC_WR_TRAN,
                   VINETIC_Write_Trans, VINETIC_IO_TRANS_WR);
#if 0
   ON_IOCTL       (pDev, FIO_TEST_CHIP_ACCESS,
                   VINETIC_Test_Chip, VINETIC_IO_TEST_CHIP);
#endif
   ON_IOCTL       (pDev, FIO_VINETIC_SS,
                   VINETIC_Send_Sequence, VINETIC_IO_SEND_SEQ);
   ON_IOCTL       (pDev, FIO_VINETIC_DIOP,
                   VINETIC_Read_DIOP, VINETIC_IO_READ_DIOP);
   ON_DIR_IOCTL   (pDev, FIO_VINETIC_DUMP_PRAM, VINETIC_PRAM_Dump,
                   (VINETIC_IO_USR*)ioarg);
   ON_DIR_IOCTL   (pDev, FIO_VINETIC_DUMP_DRAM, VINETIC_DRAM_Dump,
                   (VINETIC_IO_USR*)ioarg);
   ON_IOCTL       (pDev, FIO_VINETIC_CRC,
                   VINETIC_ReadEdspCrc,   VINETIC_IO_CRC);
   ON_IOCTL       (pDev, FIO_VINETIC_EDSP,
                   VINETIC_StartEdsp, VINETIC_IO_STARTEDSP);
   ON_EX_IOCTL    (pDev, FIO_VINETIC_GET_CRAMCHECKSUM,
                   VINETIC_GetCramCrc);
   /* misc module */
   ON_IOCTL       (pDev, FIO_VINETIC_SWITCH_MODEM_COEFF,
                   VINETIC_Switch_Modem_Coeff, VINETIC_IO_MODEM);
   /* Analog Line module */
   ON_IOCTL       (pDev, FIO_VINETIC_ALM_SWITCH,
                   VINETIC_ALM_Switch, VINETIC_IO_ALM_SWITCH);
   ON_EX_IOCTL    (pDev, FIO_VINETIC_ALM_CONFIG,  VINETIC_Get_Alm_Conf);
#ifdef VIN_V21_SUPPORT
   ON_IOCTL_FRUSR (pDev, FIO_VINETIC_DNLD_EMBDCTRL,
                   VINETIC_Eval_DownloadEmbdCtrl,   VINETIC_IO_EMBDCTRL);
#endif /* VIN_V21_SUPPORT */
#ifdef TESTING
   /* driver config for testing purposes */
   ON_IOCTL_FRUSR (pDev, FIO_VINETIC_DRV_CFG, VINETIC_DrvConfig,
                   VINETIC_IO_DRVCFG);
   ON_IOCTL_FRUSR (pDev, FIO_VINETIC_DIOPBURST,VINETIC_DIOP_Burst,
                   VINETIC_IO_DIOPBURST);
   ON_IOCTL_FRUSR (pDev, FIO_VINETIC_DIOPPATTERN, VINETIC_DIOP_Pattern,
                   VINETIC_IO_DIOPPATTERN);
   ON_IOCTL_FRUSR (pDev, FIO_VINETIC_DIABURST, VINETIC_DIA_Burst,
                   VINETIC_IO_DIABURST);
   ON_DIR_IOCTL   (pDev, FIO_VINETIC_TESTSTART, VINETIC_TestStart, ioarg);
   ON_IOCTL_FRUSR (pDev, FIO_VINETIC_TESTPARM, VINETIC_TestParm,
                   VINETIC_IO_TESTPARM);
   case FIO_VINETIC_TESTRESULTS:
      IFXOS_CPY_KERN2USR ((IFX_uint8_t*)ioarg, &pDev->TestResult,
                          sizeof (pDev->TestResult));
      return IFX_SUCCESS;
#endif /* TESTING */
#endif /* VIN_2CPE */
   }

   return ret;
}

#endif /* EVALUATION */


/**
   Vinetic Tapi ioctl handling

\param pDummyCh - handle to VINETIC_CHANNEL structure (could be also a pointer
                  to a VINETIC_DEVICE structure - so we have to check)
\param iocmd    - ioctl command
\param ioarg    - ioctl argument

\return
   IFX_SUCCESS or IFX_ERROR

\remark
*/

IFX_int32_t VINETIC_Dev_Spec_Ioctl (IFX_TAPI_LL_CH_t *pLLDummyCh,
                                    IFX_uint32_t iocmd,
                                    IFX_uint32_t ioarg)
{
   IFX_int32_t     ret   = IFX_SUCCESS;
   VINETIC_CHANNEL *pCh  = (VINETIC_CHANNEL *) pLLDummyCh;
   VINETIC_DEVICE  *pDev;

   /* distinguish device / channel context */
   if (pCh->nChannel == 0)
   {
      /* initial assumption is wroing - we received a device pointer */
      pDev = (VINETIC_DEVICE *) pLLDummyCh;
      pCh  = IFX_NULL;
   }
   else
   {
      /* initial assumption was correct so we only need to get the
         corresponding device pointer */
      pDev = pCh->pParent;
   }

   /*printk (KERN_INFO "VINETIC_Dev_Spec_Ioctl called with command %x\n", iocmd);*/

   /* this block handles the initialization of the vinetic driver */
   switch (iocmd)
   {
      /* This must be the very first ioctl operation before hardware access. */
      ON_IOCTL_FRUSR (pDev, FIO_VINETIC_BASICDEV_INIT, VINETIC_BasicDeviceInit,
                      VINETIC_BasicDeviceInit_t);
      /* This can be done to reset the device internal structure without doing
         any basic device initialization. Only the initialization state remains
         unchanged. All other states are reset.
      */
      case FIO_VINETIC_DEV_RESET:
         return VINETIC_DeviceReset(pDev);
      /* no basic driver initialization needed for these ioctls */
      case FIO_VINETIC_DRVVERS:
#ifndef LINUX
         ((VINETIC_IO_VERSION*)ioarg)->nDrvVers =
            (MAJORSTEP << 24 | MINORSTEP << 16 | VERSIONSTEP << 8 | VERS_TYPE);
         return IFX_SUCCESS;
#else
         return IFX_ERROR;
#endif /* LINUX */
      case FIO_VINETIC_LASTERR:
         IFXOS_CPY_KERN2USR (ioarg, &pDev->err, sizeof(IFX_int32_t));
         pDev->err = VIN_ERR_OK;
         return IFX_SUCCESS;
      case FIO_VINETIC_REPORT_SET:
         return VINETIC_Report_Set(ioarg);
#ifndef VIN_2CPE
      case FIO_VINETIC_RUNTIME_TRACE_SET:
         return VINETIC_RuntimeTrace_Report_Set(ioarg);
#endif /* VIN_2CPE */
      default:
         /* do not allow other ioctl if basic init is missing! */
         if ((pDev->nDevState & DS_BASIC_INIT) != DS_BASIC_INIT)
         {
            SET_DEV_ERROR(VIN_ERR_NOINIT);
            return IFX_ERROR;
         }
   }

   /*
   * This code block handles ioctls which require previous
   * basic device driver initialization.
   */
   switch (iocmd)
   {
      ON_DIR_IOCTL   (pDev, FIO_VINETIC_TCA, VINETIC_Host_AccessTest, ioarg);
      ON_IOCTL_FRUSR (pDev, FIO_VINETIC_WCMD, VINETIC_Write_Cmd,
                      VINETIC_IO_MB_CMD );
      ON_IOCTL       (pDev, FIO_VINETIC_RCMD, VINETIC_Read_Cmd,
                      VINETIC_IO_MB_CMD);
      ON_IOCTL       (pDev, FIO_VINETIC_VERS, VINETIC_Version,
                     VINETIC_IO_VERSION);
      ON_IOCTL       (pDev, FIO_VINETIC_INIT, VINETIC_Host_InitChip,
                      VINETIC_IO_INIT);
      /* GPIO Module */
      ON_IOCTL       (pDev, FIO_VINETIC_GPIO_RESERVE, VINETIC_GpioReserveUser,
                      VINETIC_IO_GPIO_CONTROL);
      ON_IOCTL_FRUSR (pDev, FIO_VINETIC_GPIO_RELEASE, VINETIC_GpioReleaseUser,
                      VINETIC_IO_GPIO_CONTROL);
      ON_IOCTL_FRUSR (pDev, FIO_VINETIC_GPIO_CONFIG, VINETIC_GpioConfigUser,
                      VINETIC_IO_GPIO_CONTROL);
      ON_IOCTL       (pDev, FIO_VINETIC_GPIO_GET, VINETIC_GpioGetUser,
                      VINETIC_IO_GPIO_CONTROL);
      ON_IOCTL_FRUSR (pDev, FIO_VINETIC_GPIO_SET, VINETIC_GpioSetUser,
                      VINETIC_IO_GPIO_CONTROL);
#ifdef VIN_2CPE
      ON_IOCTL_FRUSR (pDev, FIO_VINETIC_WRREG, VINETIC_HostRegWr_Cmd,
                      VINETIC_IO_REG_ACCESS);
      ON_IOCTL       (pDev, FIO_VINETIC_RDREG, VINETIC_HostRegRd_Cmd,
                      VINETIC_IO_REG_ACCESS);
      ON_IOCTL       (pCh, FIO_VINETIC_BBD_DOWNLOAD, VINETIC_BBD_Download,
                      bbd_format_t);
#else /* !VIN_2CPE */
      ON_IOCTL_FRUSR (pCh, FIO_VINETIC_DOWNLOAD_CRAM, VINETIC_Host_DownloadCram,
                      VINETIC_IO_CRAM);
      /* non VINETIC 2CPE GPIOs */
      ON_IOCTL       (pDev, FIO_VINETIC_DEV_GPIO_CFG, VINETIC_Dev_GPIO_Cfg,
                      VINETIC_IO_DEV_GPIO_CFG);
      ON_IOCTL       (pDev, FIO_VINETIC_DEV_GPIO_SET, VINETIC_Dev_GPIO_Set,
                      VINETIC_IO_DEV_GPIO_SET);
      /* Short Command Interface, not relevant for 2CPE */
      ON_IOCTL_FRUSR (pDev, FIO_VINETIC_WSC, VINETIC_Write_Sc,
                      VINETIC_IO_WRITE_SC);
      ON_IOCTL       (pDev, FIO_VINETIC_RSC, VINETIC_Read_Sc,
                      VINETIC_IO_READ_SC);
      /* Download Module, not relevant for 2CPE */
      ON_IOCTL       (pDev, FIO_VINETIC_DOWNFPI, VINETIC_Host_DownloadFpi,
                      VINETIC_IO_FPI_DOWNLOAD);
      ON_IOCTL       (pDev, FIO_VINETIC_DOW_PHI, VINETIC_Host_DownloadPhi,
                      VINETIC_IO_PHI);
#endif /* VIN_2CPE */
#if (VIN_CFG_FEATURES & VIN_FEAT_LT)
      ON_IOCTL       (pDev, FIO_VINETIC_OFFSET_CALIBRATION,
                      VINETIC_DC_Offset_Calibration, VINETIC_IO_OFFSET_CALIBRATION);
   /* LT 2 ioctls */
      ON_IOCTL       (pCh, FIO_VINETIC_LT_CURRENT, VINETIC_LT_Current,
                      VINETIC_IO_LT_CURRENT);
      ON_IOCTL       (pCh, FIO_VINETIC_LT_VOLTAGE, VINETIC_LT_Voltage,
                      VINETIC_IO_LT_VOLTAGE);
      ON_IOCTL       (pCh, FIO_VINETIC_LT_RESISTANCE,
                      VINETIC_LT_Resistance, VINETIC_IO_LT_RESISTANCE);
      ON_IOCTL       (pCh, FIO_VINETIC_LT_CAPACITANCE,
                      VINETIC_LT_Capacitance, VINETIC_IO_LT_CAPACITANCE);
      ON_IOCTL       (pCh, FIO_VINETIC_LT_IMPEDANCE,
                      VINETIC_LT_Impedance, VINETIC_IO_LT_IMPEDANCE);
      ON_IOCTL       (pCh, FIO_VINETIC_LT_AC_MEASUREMENTS,
                      VINETIC_LT_AC_Measurements, VINETIC_IO_LT_AC_MEASUREMENTS);
      ON_IOCTL       (pCh, FIO_VINETIC_LT_AC_DIRECT,
                      VINETIC_LT_AC_Direct, VINETIC_IO_LT_AC_DIRECT);
      ON_IOCTL       (pCh, FIO_VINETIC_LT_NETWORK_TIP_RING,
                      VINETIC_LT_Network_Tip_Ring, VINETIC_IO_LT_NETWORK_TIP_RING);
      ON_IOCTL       (pCh, FIO_VINETIC_LT_NETWORK_GND,
                      VINETIC_LT_Network_Ground, VINETIC_IO_LT_NETWORK_GND);

#ifdef VIN_V21_SUPPORT
      ON_IOCTL       (pCh, FIO_VINETIC_LT_RESISTANCE_RC,
                      VINETIC_LT_Resistance_RC, VINETIC_IO_LT_RESISTANCE_RC);
#endif /* VIN_V21_SUPPORT */
      case FIO_VINETIC_LT_CONFIG:
         {
            VINETIC_IO_LT_CONFIG *pLtCfg;
#ifdef LINUX
            IFXOS_CPY_USR2KERN (IOcmd, (IFX_uint8_t *)ioarg,
                  sizeof(VINETIC_IO_LT_CONFIG));
            pLtCfg = (VINETIC_IO_LT_CONFIG *)IOcmd;
#else
            pLtCfg = (VINETIC_IO_LT_CONFIG *)ioarg;
#endif /* LINUX */
            ret = VINETIC_LT_Config (pLtCfg);
#ifdef LINUX
            IFXOS_CPY_KERN2USR ((IFX_uint8_t *)ioarg, IOcmd,
                  sizeof(VINETIC_IO_LT_CONFIG));
#endif /* LINUX */
         }
#endif /* (VIN_CFG_FEATURES & VIN_FEAT_LT) */
#if (VIN_CFG_FEATURES & VIN_FEAT_GR909)
      /* GR909 realtime measurements */
#ifdef VIN_2CPE
      ON_IOCTL_FRUSR (pCh, FIO_VINETIC_GR909_START,
                      VINETIC_GR909_Start, VINETIC_IO_GR909_Start_t);
      ON_IOCTL_TOUSR (pCh, FIO_VINETIC_GR909_RESULT, VINETIC_GR909_Result,
                      VINETIC_IO_GR909_Result_t);
#else /* VIN_2CPE */
      ON_IOCTL_FRUSR (pCh, FIO_VINETIC_GR909_START,
                      VINETIC_GR909_Start, VINETIC_IO_GR909);
      ON_DIR_IOCTL   (pCh, FIO_VINETIC_GR909_RESULT, VINETIC_GR909_Result,
                      (VINETIC_IO_GR909 *)ioarg);
      ON_DIR_IOCTL   (pCh, FIO_VINETIC_GR909_STOP, VINETIC_GR909_Stop,
                      (IFX_int32_t)ioarg);
#endif /* VIN_2CPE */
#endif /* (VIN_CFG_FEATURES & VIN_FEAT_GR909) */
   }

   return ret;
}

