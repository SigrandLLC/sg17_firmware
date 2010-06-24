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
   \file drv_tapi_errno.h Private file for error handling

   \remarks
******************************************************************************/

/* ============================= */
/* Includes                      */
/* ============================= */

/* ============================= */
/* Local Macros  Definitions    */
/* ============================= */

/*Errorblock*/
/** Enumeration for function return status. The upper four bits are reserved for
    error clasification */
typedef enum
{
   TAPI_statusOk = TAPI_statusClassSuccess,
      /** Setting line mode failed */
   TAPI_statusLineModeFail = TAPI_statusClassErr | 0x1,
   /** Starting CID Info failed */
   TAPI_statusCidInfoStartFail,
   /** Invalid parameter */
   TAPI_statusParam,
   /** Invalid ioctl call */
   TAPI_statusInvalidIoctl,
   /** Unknown or unsupported ioctl call */
   TAPI_statusNoIoctl,
   /** Desired action is not supported */
   TAPI_statusNotSupported,
   /** Service is not supported by the low level driver */
   TAPI_statusLLNotSupp,
   /** No tone resource found or error in finding it for playing the tone */
   TAPI_statusToneNoRes,
   /** Invalid resource for tone service specified */
   TAPI_statusInvalidToneRes,
   /** Service called with wrong context fd */
   TAPI_statusCtxErr,
   /** Service not supported on called channel context */
   TAPI_statusInvalidCh,
   /** Copy to of from user space not successful (Linux only) */
   TAPI_statusErrKernCpy,
   /** General initialization failed */
   TAPI_statusInitFail,
   /** Reference to unconfigured tone code entry  */
   TAPI_statusToneNotAvail,
   /** Stopping tone failed */
   TAPI_statusToneStop,
   /** Playing tone in LL driver failed */
   TAPI_statusTonePlayLLFailed,
  /** LL driver returned an error */
   TAPI_statusLLFailed
}TAPI_Status_t;

/**\todo the code is always shifted even when a TAPI function returned it and
   has been already shifted */
#define RETURN_STATUS(code, llcode)                                        \
   do{                                                                     \
      if (TAPI_SUCCESS(code) == IFX_FALSE)                                 \
         TAPI_ErrorStatus (pChannel, code, llcode, __LINE__, __FILE__);    \
      return (code & 0xFFFF0000) ? code | llcode : (code << 16) | llcode;  \
   }while(0)
#define RETURN_DEVSTATUS(code, llcode)                                     \
   do{                                                                     \
      if (TAPI_SUCCESS(code) == IFX_FALSE)                                 \
         TAPI_ErrorStatus (&pTapiDev->pTapiChanelArray[0],                 \
                           code, llcode, __LINE__, __FILE__);              \
      return (code << 16) | llcode;                                        \
   }while(0)
#define GET_HL_ERROR(code)                                                 \
   ((code >> 16) & 0x0000FFFF)

/* ============================= */
/* Global variable definition    */
/* ============================= */

/* ============================= */
/* Global function declaration   */
/* ============================= */

extern void TAPI_ErrorStatus (TAPI_CHANNEL * pChannel, TAPI_Status_t nHlCode,
                              IFX_int32_t nLlCode,
                              IFX_uint32_t nLine, const IFX_char_t* sFile);
