#ifndef _DRV_ERRNO_H
#define _DRV_ERRNO_H
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
   Module      : drv_vinetic_errno.h
*******************************************************************************/

/** \file drv_vinetic_errno.h This file contains error number definitions
 and macros for setting the error code.
\note The macros must be used for reporting errors */

/* ============================= */
/* Global Defines                */
/* ============================= */

/** macro to signal and set an error. The error may also generate a
   trigger signal */
#ifndef SET_ERROR

#ifdef TESTING
/** Set non device or driver specific error.
\remarks The macro also reports the error to the high level TAPI
\param no error number */
#define SET_ERROR(no)                              \
   do {                                            \
      pCh->pParent->err = (IFX_int32_t) (no);      \
      pCh->pParent->nErrLine = __LINE__;           \
      strcpy (pCh->pParent->sErrFile, __FILE__);   \
      VINETIC_ChErrorEvent (pCh, no);              \
   } while(0)

/** Set device or driver specific error
\param no error number */
#define SET_DEV_ERROR(no)                 \
   do {                                   \
      pDev->err = (IFX_int32_t) (no);     \
      pDev->nErrLine = __LINE__;          \
      strcpy (pDev->sErrFile, __FILE__);  \
      VINETIC_DevErrorEvent (pDev, no);   \
   } while(0)

#else /* #ifdef TESTING */
   /* Customer specific definition.
      This solution has to be a provisional solution
      and has to be changed */
#define SET_ERROR(no)                     \
   do {                                   \
      pCh->pParent->err = (IFX_int32_t) (no);     \
      VINETIC_ChErrorEvent (pCh, no);     \
      IFXOS_ASSERT(IFX_FALSE);            \
   } while(0)

#define SET_DEV_ERROR(no)                 \
   do {                                   \
      pDev->err = (IFX_int32_t) (no);     \
      VINETIC_DevErrorEvent (pDev, no);   \
      IFXOS_ASSERT(IFX_FALSE);            \
   } while(0)
#endif /* #else #ifdef TESTING */

#endif /* SET_ERROR */

/** \defgroup ErrorCodes Driver and Chip Error Codes
 * @{
 */
enum VIN_DEV_ERR {
   /** 0x0: no error */
   VIN_ERR_OK = 0,
   /** command error reported by vinetic, see last command */
   VIN_ERR_CERR = 0x01,
   /** command inbox overflow reported by vinetic */
   VIN_ERR_CIBX_OF = 0x2,
   /** host error reported by vinetic */
   VIN_ERR_HOST = 0x3,
   /** MIPS overload */
   VIN_ERR_MIPS_OL = 0x4,
   /** no command data received event within timeout.
       This error is obsolete, since the driver used a polling mode */
   VIN_ERR_NO_COBX = 0x5,
   /** no command data received within timeout */
   VIN_ERR_NO_DATA = 0x6,
   /** not enough inbox space for writing command */
   VIN_ERR_NO_FIBXMS = 0x7,
   /** more data then expected in outbox */
   VIN_ERR_MORE_DATA = 0x8,
   /** Mailbox was not empty after timeout. This error occurs while the
       driver tries to switch the mailbox sizes before and after the
       firmware download. The timeout is given in the constant WAIT_MBX_EMPTY */
   VIN_ERR_NO_MBXEMPTY = 0x9,
   /** download ready event has not occured */
   VIN_ERR_NO_DLRDY = 0xA,
   /** register read: expected values do not match */
   VIN_ERR_WRONGDATA = 0xB,
   /** OBXML is zero after COBX-DATA event, wrong behaviour of VINETIC
       After event OBXML must indicate data. This error is obsolete,
       since the driver used a polling mode */
   VIN_ERR_OBXML_ZERO = 0xC,
   /** Test chip access failed */
   VIN_ERR_TEST_FAIL = 0xD,
   /** Internal EDSP hardware error reported by
       VINETIC in HWSR1:HW-ERR */
   VIN_ERR_HW_ERR = 0xE,
   /** Mailbox Overflow Error */
   VIN_ERR_PIBX_OF = 0xF,
   /** invalid parameter in function call */
   VIN_ERR_FUNC_PARM = 0x10,
   /** timeout while waiting on channel status change */
   VIN_ERR_TO_CHSTATE = 0x11,
   /** buffer underrun in evaluation downstreaming */
   VIN_ERR_BUF_UN = 0x12,
   /** no memory by memory allocation */
   VIN_ERR_NO_MEM = 0x13,
   /** board previously not initialized  */
   VIN_ERR_NOINIT = 0x14,
   /** interrupts cannot be cleared  */
   VIN_ERR_INTSTUCK = 0x15,
   /** line testing measurement is running */
   VIN_ERR_LT_ON = 0x16,
   /** PHI patch wasn't successfully downloaded. The problem was an chip
       access problem */
   VIN_ERR_NOPHI = 0x17,
   /** EDSP Failures */
   VIN_ERR_EDSP_FAIL = 0x18,
   /** CRC Fail while download */
   VIN_ERR_FWCRC_FAIL = 0x19,
   /** TAPI not initialized */
   VIN_ERR_NO_TAPI = 0x1A,
   /** Error while using SPI Inteface  */
   VIN_ERR_SPI = 0x1B,
   /** inconsistent or invalid parameters were provided  */
   VIN_ERR_INVALID = 0x1C,
   /** no Data to copy to user space for GR909 measurement */
   VIN_ERR_GR909 = 0x1D,
   /** CRC Fail while ac download for V1.4 */
   VIN_ERR_ACCRC_FAIL = 0x1E,
   /** couldn't read out chip version */
   VIN_ERR_NO_VERSION = 0x1F,
   /** CRC Fail in DCCTRL download */
   VIN_ERR_DCCRC_FAIL = 0x20,
   /** unknown chip version */
   VIN_ERR_UNKNOWN_VERSION = 0x21,
   /** Linetesting, line is in Power Down High Impedance, measurement not possible */
   VIN_ERR_LT_LINE_IS_PDNH = 0x22,
   /** Linetesting, unknown Parameter */
   VIN_ERR_LT_UNKNOWN_PARAM = 0x23,
   /** Error while sending CID */
   VIN_ERR_CID_TRANSMIT = 0x24,
   /** Linetesting, timeout waiting for LM_OK */
   VIN_ERR_LT_TIMEOUT_LM_OK = 0x25,
   /** linetesting, timeout waiting for RAMP_RDY */
   VIN_ERR_LT_TIMEOUT_LM_RAMP_RDY = 0x26,
   /** PRAM firmware not ok */
   VIN_ERR_PRAM_FW = 0x27,
   /** no firmware specified and not included in driver  */
   VIN_ERR_NOFW = 0x28,
   /** PHI CRC is zero */
   VIN_ERR_PHICRC0 = 0x29,
   /** Embedded Controller download failed */
   VIN_ERR_EMBDCTRL_DWLD_FAIL = 0x2A,
   /** Embedded Controller boot failed after download */
   VIN_ERR_EMBDCTRL_DWLD_BOOT = 0x2B,
   /** Firmware binary is invalid  */
   VIN_ERR_FWINVALID = 0x2C,
   /** Firmware version could not be read, no answer to command  */
   VIN_ERR_NOFWVERS = 0x2D,
   /** Maximize mailbox failed  */
   VIN_ERR_NOMAXCBX = 0x2E,
   /** Signaling module not enabled */
   VIN_ERR_SIGMOD_NOTEN = 0x2F,
   /** Signaling channel not enabled */
   VIN_ERR_SIGCH_NOTEN = 0x30,
   /** coder configuration not valid */
   VIN_ERR_CODCONF_NOTVALID = 0x31,
   /** Linetesting, optimum result routine failed */
   VIN_ERR_LT_OPTRES_FAILED = 0x32,
   /** No free input found while connecting cod, sig, pcm and alm modules */
   VIN_ERR_NO_FREE_INPUT_SLOT = 0x33,
   /** feature or combination not supported  */
   VIN_ERR_NOTSUPPORTED = 0x34,
   /** resource not available  */
   VIN_ERR_NORESOURCE = 0x35,
   /** Event payload type mismatch */
   VIN_ERR_WRONG_EVPT = 0x36,
   /** connection not valid on remove */
   VIN_ERR_CON_INVALID = 0x37,
   /** host register access failure [2CPE] */
   VIN_ERR_HOSTREG_ACCESS = 0x38,
   /** no packet buffers available */
   VIN_ERR_NOPKT_BUFF = 0x39,
   /** At least one parameter is not possible to apply when the coder is
       running. Event payload types can not be changed on the fly. */
   VIN_ERR_COD_RUNNING = 0x3A,
   /* Tone is already played out on this channel */
   VIN_ERR_TONE_PLAYING = 0x3B,
   /** Tone resource is not capable playing out a certain tone. This
       error should not occur -> internal mismatch */
   VIN_ERR_INVALID_TONERES = 0x3C,
   /** Invalid state for switching off signaling modules. Internal error */
   VIN_ERR_INVALID_SIGSTATE = 0x3D,
   VIN_ERR_INVALID_UTGSTATE = 0x3E,
   /** Cid sending is ongoing in this channel */
   VIN_ERR_CID_RUNNING = 0x3F,
   /** Some internal state occured, that could not be handled. This error
       should never occur */
   VIN_ERR_UNKNOWN = 0x40,
   /** Action not supported with this TAPI initialisation mode */
   VIN_ERR_WRONG_CHANNEL_MODE = 0x41,
   /** No acutal signaling channel found. Could be an internal initialization
       problem or the resource is not available on this channel */
   VIN_ERR_NO_SIGCH = 0x42,
   /** bufferpool buffer free error */
   VIN_ERR_BUFPUT = 0x43,
   /** mailbox write error */
   VIN_ERR_MBXWRITE = 0x44,
   /** Coder activation is not possible while the T.38 data pump is running. */
   VIN_ERR_T38_RUNNING = 0x45,
   /* add here ^ */
   /*---------------------------- severe errors ------------------------------*/
   /** driver initialization failed */
   VIN_ERR_DRVINIT_FAIL = 0x80,
   /** general access error, RDQ bit is always 1 */
   VIN_ERR_DEV_ERR = 0x81
};

/* }@ */
#endif /* _DRV_ERRNO_H */


