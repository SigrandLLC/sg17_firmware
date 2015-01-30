#ifndef _VINETIC_IO_H
#define _VINETIC_IO_H
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
   Module      : vinetic_io.h
   Date        : 2002-03-15
   Description : This file contains defines, structures declarations for
                 the driver user interface
   Remarks:
      uses ANSI c types: char for 8 bit, short for 16 bit, long for 32 bit
*******************************************************************************/

/** \file
   This file contains the types and the defines specific to the VINETIC
   driver interface and is used by applications */

/* error information from outside, to make this file more clean */
#include "drv_vinetic_errno.h"

/**
   \defgroup VIN_DRIVER_POLLING_INTERFACE Polling Interface
   Control the device and channels in polling mode
 */

/* ============================= */
/* Global Defines                */
/* ============================= */


/** VINETIC Chip Revision */
typedef enum
{
   /** Version V1.3 */
   VINETIC_V13      = 0x42,
   /** Version V1.4 */
   VINETIC_V14      = 0x84,
   /** Version V1.5, no longer available */
   VINETIC_V15      = 0x85,
   /** Version V1.6, internal version */
   VINETIC_V16      = 0x86,
   /** Version V2.1, VINETIC 4M */
   VINETIC_V21      = 0x90,
   /** Version V2.2, VINETIC 4M/C */
   VINETIC_V22      = 0xA0,
   /** Version V2.1 of VINETIC 4S */
   VINETIC_V21_S    = 0xB0,
   /** VINETIC-CPE v2.1 */
   VINETIC_2CPE_V21 = 0x60,
   /** VINETIC-CPE v2.2 */
   VINETIC_2CPE_V22 = 0x66,
   /** VINETIC-CPE-AMR v2.1 */
   VINETIC_2CPE_AMR = 0x68
} VINETIC_IO_CHIP_REVISION;

/** VINETIC Chip Major Revision */
typedef enum
{
   /** Version V1.x */
   VINETIC_V1x  = 1,
   /** Version V2.x */
   VINETIC_V2x  = 2
} VINETIC_IO_CHIP_MAJOR_REVISION;

/** VINETIC chip types, depending on register REVISION and
    firmware download */
typedef enum
{
   /** chip type S */
   VINETIC_TYPE_S    = 0x0,
   /** chip type M */
   VINETIC_TYPE_M    = 0x1,
   /** chip type VIP */
   VINETIC_TYPE_VIP  = 0x2,
   /** chip type C */
   VINETIC_TYPE_C    = 0x4,
   /** chip type 2CPE */
   VINETIC_TYPE_2CPE = 0x5
} VINETIC_IO_CHIP_TYPE;

#define VINETIC_MB_ACC_REG_NWD   0x1
#define VINETIC_MB_ACC_REG_EOM   0x2

/** Structure that describes one write mailbox access for the mailbox access
    trace feature. It contains the register name and the written value */
typedef struct
{
   unsigned short VinReg;
   unsigned short Value;
}VINETIC_MB_ACC_TRACE;

/** This structure is used for the IOCTL command 'FIO_VINETIC_MB_ACC_TRACE_READ'
    to read out the write mailbox trace buffer. */
typedef struct
{
   VINETIC_MB_ACC_TRACE *pArray;
   unsigned int         MbSize;
   unsigned int         number;
}VINETIC_MB_ACC_TRACE_READ;

#define VINETIC_MB_ACC_DEFAULT_ENTRIES 10000

/** VINETIC maximum channel number for 4 VIP */
#define VINETIC_4VIP_CH_NR       8
/** VINETIC maximum channel number for 2 CPE */
#define VINETIC_2CPE_CH_NR       8
/** VINETIC maximum analog channel number for 4VIP */
#define VINETIC_4VIP_ANA_CH_NR   4
/** VINETIC maximum analog channel number for 2CPE */
#define VINETIC_2CPE_ANA_CH_NR   2

/** Maximal GR909 test values in a state machine */
#define MAX_GR909_VAL       15

/** @defgroup VIN_DRIVER_INTERFACE Driver Interface
    Lists the entire interface to the VINETIC Driver except TAPI
  @{  */

/** @defgroup VIN_DRIVER_INTERFACE_BASIC Basic Interface
    Basic VINETIC Access routines as command read and write and initilazation
  @{
  */

/** Flag for \ref VINETIC_IO_INIT to avoid no board configuration.
    Internal use only and will be removed */
#define NO_BCONF            0x00000001
/** Flag for \ref VINETIC_IO_INIT to avoid EDSP start.
    Internal use only */
#define NO_EDSP_START       0x00000002
#define VOIP_FILE_MODE      0x00000004
/** Flag for \ref VINETIC_IO_INIT to avoid PHI download in case of V1.4.
    Internal use only */
#define NO_PHI_DWLD         0x00000008
/** Flag for \ref VINETIC_IO_INIT to avoid CRAM download in case of V1.4.  */
#define NO_CRAM_DWLD        0x00000010
/** Flag for \ref VINETIC_IO_INIT to send the auto download command
    in case of V1.4   */
#define FW_AUTODWLD         0x00000020
/** Flag for \ref VINETIC_IO_INIT to avoid AC download in case of V1.4.
    No effect with any other chip version */
#define NO_AC_DWLD          0x00000080
/** Flag for \ref VINETIC_IO_INIT to avoid firmware download */
#define NO_FW_DWLD          0x00000100
/** Flag for \ref VINETIC_IO_INIT to perform a DC control download in case
   of V1.4. Needed for testing  */
#define DC_DWLD             0x00000200

/** reports device reset to TAPI device specific status */
#define TAPI_DEVRESET 0x01

/* ============================= */
/* VINETIC ioctl access          */
/* ============================= */

/* general mode settings (set or get information) */
#define  IOSET    0
#define  IOGET    1
#define  IOMODIFY 2

/* ============================= */
/* VINETIC ioctl Defines         */
/* ============================= */

/* magic number */
#define VINETIC_IOC_MAGIC 'V'

/* IOCMD global defines */

/** Driver configuration.
   The parameter points to a \ref VINETIC_IO_DRVCTRL structure */
#define FIO_VINETIC_DRV_CTRL              _IO(VINETIC_IOC_MAGIC,  1)
/** @} */

/** Driver configuration.
   Due to the call of this function, the device will be added to
   the list of Vinetic devices to poll */
#define FIO_VINETIC_POLL_DEV_ADD          _IO(VINETIC_IOC_MAGIC,  2)
/** @} */

/** This interface is to call periodically to read the VINETIC
   status registers for polling mode
   \ingroup VIN_DRIVER_POLLING_INTERFACE */
#define FIO_VINETIC_POLL_EVT              _IO(VINETIC_IOC_MAGIC,  3)


/** \addtogroup VIN_DRIVER_INTERFACE_BASIC
  @{  */

/** Write short command.
   The parameter points to a \ref VINETIC_IO_WRITE_SC  structure
   \note This interface is only for debugging and testing purposes.
   The use of this interface may disturb the driver operation */
#define FIO_VINETIC_WSC                   _IO(VINETIC_IOC_MAGIC, 11)
/** Read short command.
   The parameter points to a \ref VINETIC_IO_READ_SC structure
   \note This interface is only for debugging and testing purposes.
   The use of this interface may disturb the driver operation.
   \code
   VINETIC_IO_READ_SC ioCmd;
   memset (&ioCmd, 0, sizeof (ioCmd));
   ioCmd.nCmd = 0xC100;
   ret = ioctl (fd, FIO_VINETIC_RSC, (INT) &ioCmd);
   if (ioCmd.pData[0] == 0x1FFF) return 1;
   \endcode */
#define FIO_VINETIC_RSC                   _IO(VINETIC_IOC_MAGIC,  4)

#define FIO_VINETIC_CRC                   _IO(VINETIC_IOC_MAGIC,  5)
/** Read relevant version information.
   The parameter points to a \ref VINETIC_IO_VERSION structure  */
#define FIO_VINETIC_VERS                  _IO(VINETIC_IOC_MAGIC,  6)
#define FIO_VINETIC_DRVVERS               _IO(VINETIC_IOC_MAGIC,  7)
/** PHI download using structure \ref VINETIC_IO_PHI */
#define FIO_VINETIC_DOW_PHI               _IO(VINETIC_IOC_MAGIC,  9)

/** Set the driver report levels if the driver is compiled with
    ENABLE_TRACE
    \remarks
    valid arguments are
    \arg 0: off
    \arg 1: low, high output
    \arg 2: normal, general information and warnings
    \arg 3: high, only errors are reported      */
#define FIO_VINETIC_REPORT_SET            _IO(VINETIC_IOC_MAGIC, 13)
/** Write a VINETIC command.
   The parameter points to a \ref VINETIC_IO_MB_CMD structure.
   \note This interface is only for debugging and testing purposes.
   The use of this interface may disturb the driver operation */
#define FIO_VINETIC_WCMD                  _IO(VINETIC_IOC_MAGIC, 14)
/** Read command.
   The parameter points to a \ref VINETIC_IO_MB_CMD structure
   \note This interface is only for debugging and testing purposes.
   The use of this interface may disturb the driver operation

   \code
   VINETIC_IO_MB_CMD ioCmd;
   int err;

   ioCmd.cmd1 = 0x8300 | ch;
   // read OPMODE_CUR
   ioCmd.cmd2 = 0x2101;

   err = ioctl (fd, FIO_VINETIC_RCMD, (INT) &ioCmd);
   \endcode
    */
#define FIO_VINETIC_RCMD                  _IO(VINETIC_IOC_MAGIC, 15)
/** FPI download, only for internal use */
#define FIO_VINETIC_DOWNFPI               _IO(VINETIC_IOC_MAGIC, 16)

/** Vinetic Initialization. If the driver is compiled with TAPI support
    the interface IFX_TAPI_CH_INIT should be used.
    This interface expects a pointer to a \ref VINETIC_IO_INIT structure */
#define FIO_VINETIC_INIT                  _IO(VINETIC_IOC_MAGIC, 17)
/** Driver runtime tracing of register accesses */
#define FIO_VINETIC_RUNTIME_TRACE_SET     _IO(VINETIC_IOC_MAGIC, 18)
/** Download cram coefficients to one or all VINETIC channels.
   The parameter is a pointer to a \ref VINETIC_IO_CRAM structure
   \code
   VINETIC_IO_CRAM         ioCram;
   unsigned  int           nCh;
   int                     err = IFX_SUCCESS;

   memset(&ioCram, 0, sizeof(VINETIC_IO_CRAM));

   ioCram.bBroadCast  = 1;
   ioCram.nFormat     = VINETIC_IO_CRAM_FORMAT_2_1;
   ioCram.nStartAddr  = VINETIC_COP_START_ADDRESS;
   ioCram.nLength     = sizeof(VINETIC_COEFF) / 2;
   ioCram.nCRC        = VINETIC_CRAM_CRC;
   ioCram.bcr1.nData  = VINETIC_COEFF_BCR[0][0];
   ioCram.bcr1.nMask  = VINETIC_COEFF_BCR[0][1];
   ioCram.bcr2.nData  = VINETIC_COEFF_BCR[1][0];
   ioCram.bcr2.nMask  = VINETIC_COEFF_BCR[1][1];
   ioCram.tstr2.nData = VINETIC_COEFF_TSTR2[0][0];
   ioCram.tstr2.nMask = VINETIC_COEFF_TSTR2[0][1];

   memcpy (ioCram.aData, VINETIC_COEFF, sizeof(VINETIC_COEFF));
   err = ioctl (fd, FIO_VINETIC_DOWNLOAD_CRAM, (INT) &ioCram);
   \endcode
   */
#define FIO_VINETIC_DOWNLOAD_CRAM         _IO(VINETIC_IOC_MAGIC, 19)

/** Get the last occured error. The error codes are listed in
   drv_vinetic_errno.h and are enumerated in \ref DEV_ERR
   \code
   ioctl (fd, FIO_VINETIC_LASTERR, (INT) &err);
   \endcode
    */
#define FIO_VINETIC_LASTERR               _IO(VINETIC_IOC_MAGIC, 20)

/** Test the basic access of VINETIC device
   \param IFX_uint16_t number of access tests
   \code
   ioctl (fd, FIO_VINETIC_TCA, 10);
   \endcode
    */
#define FIO_VINETIC_TCA                   _IO(VINETIC_IOC_MAGIC, 10)

/** Read out the mailbox trace buffer of this specific device. All write
    accesses to the register NWD and EOM are logged inside this trace buffer.
    This feature helps to find mailbox command error with an access history.
*/
#define FIO_VINETIC_MB_ACC_TRACE_READ     _IO(VINETIC_IOC_MAGIC, 202)

/** Read out the mailbox trace buffer size of this specific device. This
    function returns the number of entries. Each entry has the size of
    'VINETIC_MB_ACC_TRACE'
*/
#define FIO_VINETIC_MB_ACC_TRACE_GETSIZE  _IO(VINETIC_IOC_MAGIC, 203)

/** Set  the mailbox trace buffer size of this specific device. This
    function set the number of entries. Each entry has the size of
    'VINETIC_MB_ACC_TRACE'
*/
#define FIO_VINETIC_MB_ACC_TRACE_SETSIZE  _IO(VINETIC_IOC_MAGIC, 204)
/** @} */

/* vxworks provides no methode to differentiate between read and  */
/* exception filedescriptors, which is necessary e.g. for H.323   */
/* After wakeup vxworks has to determine what the source of the   */
/* wakeup() call was, which is done by calling the                */
/* FIO_VINETIC_POLL-ioctl() after returning from select()
   no longer supported!        */
#define FIO_VINETIC_POLL                  _IO(VINETIC_IOC_MAGIC, 190)

/** \defgroup VIN_DRIVER_INTERFACE_CPE_LT Line Testing Interface
    Interfaces for low level line testing functions for CPE driver. */
/** \defgroup VIN_DRIVER_INTERFACE_LT Line Testing Interface
    Interfaces for low level line testing functions for Line Card driver. */
/** @{ */

/* Linetesting Ioctl Commands */

/* GR909 realtime measurements */
/** Start a GR909 Measurement.
    parameter is a pointer to \ref VINETIC_IO_GR909 structure for Line Card
    driver or to \ref VINETIC_IO_GR909_Start_t for CPE driver .
    \ingroup VIN_DRIVER_INTERFACE_CPE_LT
*/
#define FIO_VINETIC_GR909_START           _IO(VINETIC_IOC_MAGIC, 21)
/** Stop a GR909 Measurement.
    \ingroup VIN_DRIVER_INTERFACE_CPE_LT
*/
#define FIO_VINETIC_GR909_STOP            _IO(VINETIC_IOC_MAGIC, 22)
/** Read Results of a GR909 Measurement.
    parameter is a pointer to \ref VINETIC_IO_GR909 structure for Line Card
    driver or to \ref VINETIC_IO_GR909_Result_t for CPE driver.
    \ingroup VIN_DRIVER_INTERFACE_CPE_LT
*/
#define FIO_VINETIC_GR909_RESULT          _IO(VINETIC_IOC_MAGIC, 23)
/* FAULT current detection */
#define FIO_VINETIC_FAULTCURR_SWITCHSLIC  _IO(VINETIC_IOC_MAGIC, 24)
/** Offset calibration.
    parameter is a pointer to a \ref VINETIC_IO_OFFSET_CALIBRATION structure */
#define FIO_VINETIC_OFFSET_CALIBRATION    _IO(VINETIC_IOC_MAGIC, 25)
/* Linetesting 2.0 Ioctl Commands */
/** linetesting current measurement.
    parameter is a pointer to a \ref VINETIC_IO_LT_CURRENT structure.
*/
#define FIO_VINETIC_LT_CURRENT            _IO(VINETIC_IOC_MAGIC, 26)
/** linetesting voltage measurement.
    parameter is a pointer to a \ref VINETIC_IO_LT_VOLTAGE structure.
*/
#define FIO_VINETIC_LT_VOLTAGE            _IO(VINETIC_IOC_MAGIC, 27)
/** linetesting resistance measurement.
    parameter is a pointer to a \ref VINETIC_IO_LT_RESISTANCE structure.
*/
#define FIO_VINETIC_LT_RESISTANCE         _IO(VINETIC_IOC_MAGIC, 28)
/** linetesting capacitance measurement.
    parameter is a pointer to a \ref VINETIC_IO_LT_CAPACITANCE structure.
*/
#define FIO_VINETIC_LT_CAPACITANCE        _IO(VINETIC_IOC_MAGIC, 29)
/** linetesting impedance measurement.
    parameter is a pointer to a \ref VINETIC_IO_LT_IMPEDANCE structure.
*/
#define FIO_VINETIC_LT_IMPEDANCE          _IO(VINETIC_IOC_MAGIC, 30)
/** linetesting AC measurements (PCM4-like).
    parameter is a pointer to a \ref VINETIC_IO_LT_AC_MEASUREMENTS structure.
*/
#define FIO_VINETIC_LT_AC_MEASUREMENTS    _IO(VINETIC_IOC_MAGIC, 31)
/** linetesting AC measurement (most parameters configurable).
    parameter is a pointer to a \ref VINETIC_IO_LT_AC_DIRECT structure.
*/
#define FIO_VINETIC_LT_AC_DIRECT          _IO(VINETIC_IOC_MAGIC, 32)
/* linetesting network resistance measurement (reserved for future use) */
#define FIO_VINETIC_LT_RESISTANCE_NET     _IO(VINETIC_IOC_MAGIC, 33)
/** linetesting resistance RC measurement.
    parameter is a pointer to a \ref VINETIC_IO_LT_RESISTANCE_RC structure.
*/
#define FIO_VINETIC_LT_RESISTANCE_RC      _IO(VINETIC_IOC_MAGIC, 34)

/** linetesting network measurement (Rring, Cring, Rtipring).
    parameter is a pointer to a \ref VINETIC_IO_LT_NETWORK structure.
*/
#define FIO_VINETIC_LT_NETWORK_TIP_RING   _IO(VINETIC_IOC_MAGIC, 35)
/** linetesting network to ground measurement (Rring, Cring, Rtipring).
    parameter is a pointer to a \ref VINETIC_IO_LT_NETWORK_GND structure.
*/
#define FIO_VINETIC_LT_NETWORK_GND        _IO(VINETIC_IOC_MAGIC, 36)
/** linetesting, discharge function
    parameter is a pointer to a \ref VINETIC_IO_LT_DISCHARGE structure.
*/
#define FIO_VINETIC_LT_DISCHARGE          _IO(VINETIC_IOC_MAGIC, 37)
/** linetesting, charging function
    parameter is a pointer to a \ref VINETIC_IO_LT_CHARGE structure.
*/
#define FIO_VINETIC_LT_CHARGE             _IO(VINETIC_IOC_MAGIC, 38)
/** linetesting, network measurement helper function
    parameter is a pointer to a \ref VINETIC_IO_LT_NETWORK structure.
*/
#define FIO_VINETIC_LT_NETWORK            _IO(VINETIC_IOC_MAGIC, 39)

                                               /* reserved until 40
                                                  for Linetesting */
/** linetesting configuration.
    parameter is a pointer to a \ref VINETIC_IO_LT_CONFIG structure.
*/
#define FIO_VINETIC_LT_CONFIG             _IO(VINETIC_IOC_MAGIC, 40)
/** @} */

/** \addtogroup VIN_DRIVER_INTERFACE_GPIO GPIO Interface */
/** @{ */

/** Configure Configure device GPIO pins 0..7. Will be replaced by
   \ref FIO_VINETIC_GPIO_CONFIG.
   Use structure \ref VINETIC_IO_DEV_GPIO_CFG */
#define FIO_VINETIC_DEV_GPIO_CFG          _IO(VINETIC_IOC_MAGIC, 41)
/** Set GPIO pins values. Will be replaced by
   \ref FIO_VINETIC_GPIO_CONFIG. */
#define FIO_VINETIC_DEV_GPIO_SET          _IO(VINETIC_IOC_MAGIC, 42)

/** Reserve GPIO pins for use. Use structure \ref VINETIC_IO_GPIO_CONTROL
   \code
   VINETIC_IO_GPIO_CONTROL ioCmd;
   int err;
   memset (&ioCmd, 0, sizeof(ioCmd));
   ioCmd.nGpio = 0x00FF;
   err = ioctl (fd, FIO_VINETIC_GPIO_RESERVE, (INT) &ioCmd);
   myGpioHandle = ioCmd.ioHandle;
   \endcode
*/
#define FIO_VINETIC_GPIO_RESERVE          _IO(VINETIC_IOC_MAGIC, 43)

/** Configure GPIO pins. Use structure \ref VINETIC_IO_GPIO_CONTROL
   \note You have to store the ioHandle for later use, this example expects
         that you have declared myGpioHandle somewhere in your code.
   \arg nMask each bit defines the direction (0 - Input, 1 - Output)
   \arg nGpio each bit defines which io
   \code
   VINETIC_IO_GPIO_CONTROL ioCmd;
   int err;
   memset (&ioCmd, 0, sizeof(ioCmd));
   ioCmd.ioHandle = myGpioHandle;
   ioCmd.nGpio    = 0x00FF;
   // define pin 0..3 as input, 4..7 as output
   ioCmd.nMask    = 0x00F0;
   err = ioctl (fd, FIO_VINETIC_GPIO_CONFIG, (INT) &ioCmd);
   \endcode
*/
#define FIO_VINETIC_GPIO_CONFIG           _IO(VINETIC_IOC_MAGIC, 44)

/** Set GPIO pin values. Use structure \ref VINETIC_IO_GPIO_CONTROL
   \note You have to store the ioHandle for later use, this example expects
         that you have declared myGpioHandle somewhere in your code.
   \arg nMask each bit defines the IOs to be configured
   \arg nGpio each bit defines the desired state
   \code
   VINETIC_IO_GPIO_CONTROL ioCmd;
   int err;
   memset (&ioCmd, 0, sizeof(ioCmd));
   ioCmd.ioHandle = myGpioHandle;
   ioCmd.nMask    = 0x00FF;
   // set pin 0..3 off, 4..7 on
   ioCmd.nGpio    = 0x00F0;

   err = ioctl (fd, FIO_VINETIC_GPIO_SET, (INT) &ioCmd);
   \endcode
*/
#define FIO_VINETIC_GPIO_SET              _IO(VINETIC_IOC_MAGIC, 45)

/** Get GPIO pin values. Use structure \ref VINETIC_IO_GPIO_CONTROL
   \arg nMask each bit defines the IOs to be read out
   \arg nGpio each bit returns the current state
   \code
   VINETIC_IO_GPIO_CONTROL ioCmd;
   int err;
   memset (&ioCmd, 0, sizeof(ioCmd));
   ioCmd.ioHandle = myGpioHandle;
   ioCmd.nMask    = 0x00FF;
   err = ioctl (fd, FIO_VINETIC_GPIO_GET, (INT) &ioCmd);
   printf ("GPIO state 0x%04X\n", ioCmd.nGpio);
*/
#define FIO_VINETIC_GPIO_GET              _IO(VINETIC_IOC_MAGIC, 46)

/** Release GPIO pins. Use structure \ref VINETIC_IO_GPIO_CONTROL

*/
#define FIO_VINETIC_GPIO_RELEASE          _IO(VINETIC_IOC_MAGIC, 47)

/** Read host register (2CPE).
   The parameter points to a \ref VINETIC_IO_REG_ACCESS structure
   \note This interface is only for debugging and testing purposes.
   Using of this interface may disturb the driver operation

   \code
   VINETIC_IO_REG_ACCESS ioCmd;
   int err;

   ioCmd.offset = 0x0C; // register STAT_INT

   err = ioctl (fd, FIO_VINETIC_RDREG, (INT) &ioCmd);
   \endcode
*/
#define FIO_VINETIC_RDREG                 _IO(VINETIC_IOC_MAGIC, 48)
/** Write ro host register (2CPE, only)
   The parameter points to a \ref VINETIC_IO_REG_ACCESS structure
   \note This interface is provided for debugging and testing purposes.
   Using of this interface may disturb the driver operation */
#define FIO_VINETIC_WRREG                 _IO(VINETIC_IOC_MAGIC, 49)

/** @} */

/** \defgroup VIN_DRIVER_INTERFACE_INIT Driver Initialization Interface */
/** @{ */

/** Initialize Vinetic Device driver information for one chip.
    Does some required settings, which must be done before any
    other chip access will work!
    Parameter is a pointer to a \ref VINETIC_BasicDeviceInit_t structure.
*/
#define FIO_VINETIC_BASICDEV_INIT         _IO(VINETIC_IOC_MAGIC, 200)


/** Reset Vinetic Device driver internal structure for one chip.
    This ioctl must be called after each vinetic hard reset not leading
    to a vinetic basic device initialization.
*/
#define FIO_VINETIC_DEV_RESET             _IO(VINETIC_IOC_MAGIC, 201)

/** @} */

/** \defgroup VIN_DRIVER_INTERFACE_INIT Driver Initialization Interface */
/** @{ */

/** Does a download according to bbd format.
    Parameter is a pointer to a \ref bbd_format_t structure which is generic
    bbd library structure. */
#define FIO_VINETIC_BBD_DOWNLOAD             _IO(VINETIC_IOC_MAGIC, 210)
/** @} */

/** \addtogroup VIN_DRIVER_POLLING_INTERFACE */
/** @{ */
/** Packet Header Masks for channel number */
#define VIN_BUF_HDR1_CH         0x0003
/** Packet Header Masks for device number */
#define VIN_BUF_HDR1_DEV        0x00FC
/** Packet Header Masks for payload length in words */
#define VIN_BUF_HDR2_LEN        0x00FF
/** Packet Header Masks for odd bit
    (set if the last byte in the last word is padding) */
#define VIN_BUF_HDR2_ODD        0x2000
/** @} */

/* ============================= */
/* RTP packet defines            */
/* ============================= */

#define RTP_PT 0x007F

#define VINETIC_IO_LINEMODE_MASK 0x0F00
#define VINETIC_IO_LINESUB_MASK  0x00F0

typedef enum
{
   VINETIC_IO_LM_PDNH         = 0x0000,
   VINETIC_IO_LM_RING_PAUSE   = 0x0100,
   VINETIC_IO_LM_ACTIVE       = 0x0200,
   VINETIC_IO_LM_SLEEP_PDNR   = 0x0300,
   VINETIC_IO_LM_RING_BURST   = 0x0400,
   VINETIC_IO_LM_ACT_METERING = 0x0500,
   VINETIC_IO_LM_PDNR         = 0x0700
}  VINETIC_IO_LINEMODES;

/* ============================= */
/* Global Structures             */
/* ============================= */

/** \addtogroup VIN_DRIVER_INTERFACE_INIT*/
/** @{ */

/** VINETIC Access Modes */
typedef enum
{
   /** uc interface is SPI; supported by all devices */
   VIN_ACCESS_SPI,
   VIN_ACCESS_SCI,
   VIN_ACCESS_PAR_16BIT,
   /** uc interface is Motorola 8bit */
   VIN_ACCESS_PAR_8BIT,
   /*VIN_ACCESS_PARINTEL_DMUX16,*/
   VIN_ACCESS_PARINTEL_MUX16,
   /** uc interface is Intel 8bit multiplexed */
   VIN_ACCESS_PARINTEL_MUX8,
   /** uc interface is Intel 8bit demultiplexed */
   VIN_ACCESS_PARINTEL_DMUX8,


   /** special access modes only for certain chip versions ... */
   /** Access parallel intel demux 8 bit big endian. Supported by V1
       chip version. \todo To be removed : Application must configure the
       version before. */
   VIN_ACCESS_PARINTEL_DMUX8_BE,
   /** Access parallel intel demux 8 bit little endian. Supported by V2
       chip version . \todo To be removed : Application must configure the
       version before. */
   VIN_ACCESS_PARINTEL_DMUX8_LE,
   /** Access parallel motorola 8 bit big endian with 16 bit processor
       interface. Supported by V2 chip version */
   VIN_ACCESS_PAR_8BIT_V2
} VIN_ACCESS;

/** VINETIC Basic Device Initialization structure */
typedef struct
{
   /** Access mode for vinetic device. */
   VIN_ACCESS AccessMode;
   /** Vinetic physical base address.
      \remark The corresponding chip select setup must be done outside of
      this driver code.
   */
   unsigned long nBaseAddress;
   /** Vinetic device irq number, as defined by the OS.
      \remark if the value -1 is used, the device will be configured for
      polling mode.
   */
   signed int nIrqNum;
} VINETIC_BasicDeviceInit_t;

/** @} */

/** \addtogroup VIN_DRIVER_INTERFACE_BASIC
 @{ */

/** VINETIC User Interface Structure.
   Used for all firmware messages and if messages
   for input and output are very different, where a
   simple structure will not be optimal  */
typedef struct
{
   /* Byte Data Buffer from Task */
   unsigned char *pWrBuf;
   /* Size of Byte Data Buffer  */
   unsigned long nWrCnt;
   /* Byte Data Buffer to Task  */
   unsigned char *pRdBuf;
   /* Number of Data to Task */
   unsigned long nRdCnt;
} VINETIC_IO_USR;

/* ============================= */
/* Version Request               */
/* ============================= */

/** Version Io structure */
typedef struct
{
   /** chip type */
   unsigned char  nType;
   /** number of supported analog channels */
   unsigned char  nChannel;
   /** chip revision */
   unsigned short nChip;
   /** TAPI version */
   unsigned long  nTapiVers;
   /** driver version */
   unsigned long  nDrvVers;
   /** EDSP major version */
   unsigned short nEdspVers;
   /** EDSP version step */
   unsigned short nEdspIntern;
   /** DC Ctrl version */
   unsigned short nDCCtrlVers;
} VINETIC_IO_VERSION;

/* ============================= */
/* Initialization                */
/* ============================= */

/** structure used for device initialization
 */
typedef struct
{
   /** Firmware PRAM pointer or NULL if not needed */
   unsigned char *pPRAMfw ;
   /** size of PRAM firmware in bytes */
   unsigned long pram_size;
   /** Firmware DRAM pointer or NULL if not needed */
   unsigned char *pDRAMfw ;
   /** size of DRAM firmware in bytes */
   unsigned long dram_size;
   /**  pointer optional PHI program */
   unsigned char *pPHIfw;
   /** size of PHI program in bytes */
   unsigned long phi_size;
   /** pointer to CRAM */
   unsigned char *pCram;
   /** size of CRAM coefficients in bytes */
   unsigned long cram_size;
   /** pointer to block based download format data */
   unsigned char *pBBDbuf ;
   /** size of block based download buffer */
   unsigned long bbd_size;
   /** Flags for initialization. Most of the flags are only used from
       experts to modify the default initialization.
   \arg NO_BCONF       : no board configuration will be done. VINETIC
                         must be properly got out of reset
   \arg NO_PHI_DWLD    : no PHI download will be done, a properly PHI
                         download before is assumed
   \arg NO_EDSP_START  : no EDPS start is done. The VINETIC will not work
                         until that command is given
   \arg NO_CRAM_DWLD : no CRAM coefficients are downloaded. The default
                         ROM coefficients are used
   \arg FW_AUTODWLD    : firmware auto download
   \arg NO_AC_DWLD     : avoid AC download in case of V1.4 or 2CPE, no effect
                         with any other chip version
   \arg DC_DWLD        : do a DC download in case of V1.4
   \arg NO_FW_DWLD     : avoid firmware download in case of V1.5 for example */
   unsigned long nFlags;
   /** return values of PRAM CRC after firmware download */
   unsigned short nPramCRC;
   /** return values of DRAM CRC after firmware download */
   unsigned short nDramCRC;
   /** return values of PHI after PHI program download. 0 if not done */
   unsigned short nPhiCrc;
   /** return values of DC control after DC control download. 0 if not done */
   unsigned short nDcCrc;
   /** return values of AC control after AC control download. 0 if not done */
   unsigned short nAcCrc;
   /** return values of CRAM after CRAM control download. 0 if not done */
   unsigned short nCramCrc;
} VINETIC_IO_INIT;


/* ============================= */
/* Basic Access                  */
/* ============================= */

/** IO structure for read short commands
  */
typedef struct
{
   /** if set to 1 a broadcast on all channel will be done.
      Not available for every command */
   unsigned char nBc;
   /** write command */
   unsigned short nCmd;
   /** channel id */
   unsigned char nCh;
   /** read words count */
   unsigned char count;
   /** read data */
   unsigned short pData [256];
} VINETIC_IO_READ_SC;

/** IO structure for writing short commands */
typedef struct
{
   /** if set to 1 a broadcast on all channel will be done.
      Not available for every command */
   unsigned char nBc;
   /** write command */
   unsigned short nCmd;
   /** channel id */
   unsigned char nCh;
} VINETIC_IO_WRITE_SC;

/** IO structure for write and read chip commands for debugging
    purposes only */
typedef struct
{
   /** command 1 according users manual */
   unsigned short cmd1;
   /** command 2 according users manual */
   unsigned short cmd2;
   /** read or write data */
   unsigned short pData [256];
} VINETIC_IO_MB_CMD;

/** IO structure used for direct register access. Applicable to 2CPE, only */
typedef struct
{
   /** offset to host register */
   unsigned short offset;
   /** number of host registers to read/write */
   unsigned short count;
   /** contains written/read data */
   unsigned short pData[256];
} VINETIC_IO_REG_ACCESS;
/* Download FPI                  */
/* ============================= */

typedef struct
{
   /* FPI Start Address */
   unsigned long nStartAddr;
   /* FPI Stop Address */
   unsigned long nStopAddr;
   /* Data Size*/
   int nSize;
   /* Data Buffer of 16 kB */
   unsigned short  *pData;
   /*return value: CRC*/
   unsigned short  nCrc;
} VINETIC_IO_FPI_DOWNLOAD;


/* ============================= */
/* Download PHI                  */
/* ============================= */
typedef struct
{
   /* Data Buffer for PHI */
   unsigned short *pData;
   /* Data Size */
   unsigned long nSize;
   /* returned PHI CRC read from VINETIC */
   unsigned short nCrc;
} VINETIC_IO_PHI;

/* ============================= */
/* Download CRAM                 */
/* ============================= */
/** maximum size */
#define VINETIC_IO_CRAM_DATA_SIZE     250    /* Words */
/** CRAM file format version */
typedef enum
{
   /** CRAM download format for V1.6 and older */
   VINETIC_IO_CRAM_FORMAT_1,
   /** CRAM download format 2.1 for VINETIC V2.x */
   VINETIC_IO_CRAM_FORMAT_2_1,
   /** CRAM download format 2.2 for VINETIC V2.x */
   VINETIC_IO_CRAM_FORMAT_2_2
} VINETIC_IO_CRAM_FORMAT;

/** CRAM download related setting of register values (used for activation) */
typedef struct
{
   /** contains the values of the bits that should be changed (see nMask) */
   unsigned short             nData;
   /** defines which bits should be modified to the corresponding
       value in nData by setting them to 1 */
   unsigned short             nMask;
} VINETIC_IO_CRAM_REG_CFG;

/** Structure for CRAM download with the V1.4 and the V2.x format.
   This structure is used for the ioctl \ref FIO_VINETIC_DOWNLOAD_CRAM. */
typedef struct
{
   /** set to 1 if the CRAM coefficients should be downloaded
       to all channels, if set to 0, the CRAM download is only done on
       one channel (according to the filedescriptor) */
   unsigned short             bBroadCast;
   /** defines the CRAM download format version */
   VINETIC_IO_CRAM_FORMAT     nFormat;
   /** start offset of the CRAM coefficients in this download */
   unsigned char              nStartAddr;
   /** number of coefficients to be downloaded in (16bit) Words */
   unsigned long              nLength;
   /** coefficients array to be downloaded */
   unsigned short             aData [VINETIC_IO_CRAM_DATA_SIZE];
   /** CRAM coefficient specific settings for bcr1 register */
   VINETIC_IO_CRAM_REG_CFG    bcr1;
   /** CRAM coefficient specific settings for bcr2 register */
   VINETIC_IO_CRAM_REG_CFG    bcr2;
   /** CRAM coefficient specific settings for tstr2 register */
   VINETIC_IO_CRAM_REG_CFG    tstr2;
   /** CRAM CRC checksum */
   unsigned short             nCRC;
} VINETIC_IO_CRAM;

/** Embedded Controller Download structure */
typedef struct
{
   /** Data buffer */
   unsigned long *pData;
   /** Amount of Data Size in bytes */
   unsigned long nSize;
} VINETIC_IO_EMBDCTRL;

/** @} */

#ifndef VIN_2CPE
/* ============================= */
/*    L i n e t e s t i n g      */
/* ============================= */

/** \addtogroup VIN_DRIVER_INTERFACE_LT
 @{ */

/** Linetesting Configuration Mode */
typedef enum
{
  /** read out the current linetesting configuration */
  VINETIC_IO_LT_CONFIG_READ,
  /** configure the driver with new parameters */
  VINETIC_IO_LT_CONFIG_WRITE
} VINETIC_IO_LT_CONFIG_MODE;

/** Linetesting Configuration - application level */
typedef struct
{
   /** set or retrieve the configured integration time for
       current measurement (Vinetic 2.x only) in ms.
       Valid values are 0..1000, whereas 0 configures a non
       integrated measurement */
   unsigned short             nCurrentIntegrationTime;
   /** set or retrieve the configured integration time for
       voltage measurements (Vinetic 2.x only) in ms.
       Valid values are 0..1000, whereas 0 configures a non
       integrated measurement */
   unsigned short             nVoltageIntegrationTime;
   /** set or retrieve the configured integration time for
       resistance measurements (does not apply for resistance RC measurement)
       (Vinetic 2.x only) in ms.
       Valid values are 0..1000, whereas 0 configures a non
       integrated measurement */
   unsigned short             nResistanceIntegrationTime;
} VINETIC_IO_LL_LT_CONFIG;

/** Linetesting Configuration - driver level */
typedef struct
{
   /** set or retrieve information if LPF is used between the Codec and SLIC.
       (0 if no LPF is used, 1 if LPF is used) */
   unsigned short             nLowPassFilter;
   /** set or retrieve information of the SLIC type used. The SLIC type
       identifiers are according the enum TAPI_LT_SLIC */
   unsigned short             nSlicType;
} VINETIC_IO_TAPI_LT_CONFIG;

/** Linetesting Configuration */
typedef struct
{
   /** select read or write access to the linetesting configuration */
   VINETIC_IO_LT_CONFIG_MODE  access;
   VINETIC_IO_LL_LT_CONFIG    ltDrvCfg;
   VINETIC_IO_TAPI_LT_CONFIG  ltTapiCfg;
} VINETIC_IO_LT_CONFIG;

/** Linetesting Magnitude Processing */
typedef enum
{  /* do not change the order of elements */
   /** activate a squarer before the levelmeter */
   VINETIC_IO_LT_RS_SQUARER,
   /** activate a rectifier before the levelmeter */
   VINETIC_IO_LT_RS_RECTIFIER,
   /** deactivate level meter magnitude processing */
   VINETIC_IO_LT_RS_NONE
} VINETIC_IO_LT_RS;

/** Linetesting Measurement Result */
typedef struct
{
      /** Number of Samples */
      unsigned short    nSamples;
      /** Shift factor used for the measurement\n
          Attention: for Vinetic 2.1 a negative shift factor means 1/nShift */
      int               nShift;
      /** Level Meter result */
      int               nLMRES;
      /** Gain factor (only used for Vinetic 1.x) */
      unsigned short    nGain;
      /** Normal or Reverse Polarity */
      unsigned char     bRevPol;
      /** Rectifier, Squarer or None of them was set */
      VINETIC_IO_LT_RS  rs;
} VINETIC_IO_LT_RESULT;

/** SLIC type identifiers matching those in BCR1.SEL_SLIC field.
   Valid values are in range 0x0..0xf */
typedef enum
{
   VINETIC_IO_SLIC_TYPE_S                    = 0x00,
   VINETIC_IO_SLIC_TYPE_E                    = 0x01,
   VINETIC_IO_SLIC_TYPE_P                    = 0x02,
   VINETIC_IO_SLIC_TYPE_P_POWER_SENSITIVE    = 0x03,
   VINETIC_IO_SLIC_TYPE_P_CURRENT_LIMITED    = 0x06,
   VINETIC_IO_SLIC_TYPE_LCP                  = 0x08,
   VINETIC_IO_SLIC_TYPE_LCP_CURRENT_LIMITED  = 0x09,
   VINETIC_IO_SLIC_TYPE_END                  = 0x100
} VINETIC_IO_SLIC_TYPES;

/** Network linetesting result structure */
typedef struct _VINETIC_IO_LT_NET_TIP_RING_RESULT
{
   /** levelmeter result from current measurement, stage PDR1_ACT1 */
   VINETIC_IO_LT_RESULT I_ACT1;
   /** levelmeter result from voltage measurement, stage PDR1_ACT1 */
   VINETIC_IO_LT_RESULT U_ACT1;
   /** levelmeter result from voltage measurement, stage PDR1_ACT1 */
   VINETIC_IO_LT_RESULT U_PDR1;
   /** levelmeter result from current measurement, stage ACT2 */
   VINETIC_IO_LT_RESULT I_ACT2;
   /** levelmeter result from current measurement, stage RAMP3 */
   VINETIC_IO_LT_RESULT I_RAMP3;
   /** set of current measurements reaching steady state, stage ACT4 */
   VINETIC_IO_LT_RESULT pIt4[6];
   /** number of current measurements reaching steady state, stage ACT4 */
   IFX_uint32_t nIt4_num;
   /** set of timestamps, stage ACT4 */
   IFX_uint32_t pT4[6];
   /** levelmeter result from current measurement, stage ACT4 */
   VINETIC_IO_LT_RESULT I_ACT4;
   /** levelmeter result from voltage measurement, stage ACT4 */
   VINETIC_IO_LT_RESULT U_ACT4;
   /** levelmeter result from current measurement, stage RAMP5 */
   VINETIC_IO_LT_RESULT I_RAMP5;
   /** set of current measurements reaching steady state, stage ACT6 */
   VINETIC_IO_LT_RESULT pIt6[6];
   /** number of current measurements reaching steady state, stage ACT6 */
   IFX_uint32_t nIt6_num;
   /** set of timestamps, stage ACT6 */
   IFX_uint32_t pT6[6];
   /** levelmeter result from current measurement, stage ACT6 */
   VINETIC_IO_LT_RESULT I_ACT6;
   /** levelmeter result from voltage measurement, stage ACT6 */
   VINETIC_IO_LT_RESULT U_ACT6;
   /** levelmeter result from current measurement, stage RAMP7 */
   VINETIC_IO_LT_RESULT I_RAMP7;
   /** set of current measurements reaching steady state, stage ACT8 */
   VINETIC_IO_LT_RESULT pIt8[6];
   /** number of current measurements reaching steady state, stage ACT8 */
   IFX_uint32_t nIt8_num;
   /** set of timestamps, stage ACT8 */
   IFX_uint32_t pT8[6];
   /** levelmeter result from current measurement, stage ACT8 */
   VINETIC_IO_LT_RESULT I_ACT8;
   /** levelmeter result from voltage measurement, stage ACT8 */
   VINETIC_IO_LT_RESULT U_ACT8;
   /** levelmeter result from current measurement, stage RAMP9 */
   VINETIC_IO_LT_RESULT I_RAMP9;
   /** set of current measurements reaching steady state, stage ACT10 */
   VINETIC_IO_LT_RESULT pIt10[6];
   /** number of current measurements reaching steady state, stage ACT10 */
   IFX_uint32_t nIt10_num;
   /** set of timestamps, stage ACT10 */
   IFX_uint32_t pT10[6];
   /** levelmeter result from current measurement, stage ACT10 */
   VINETIC_IO_LT_RESULT I_ACT10;
   /** levelmeter result from voltage measurement, stage ACT10 */
   VINETIC_IO_LT_RESULT U_ACT10;
   /** levelmeter result from current measurement, stage RAMP11 */
   VINETIC_IO_LT_RESULT I_RAMP11;
   /** set of current measurements reaching steady state, stage ACT12 */
   VINETIC_IO_LT_RESULT pIt12[6];
   /** number of current measurements reaching steady state, stage ACT12 */
   IFX_uint32_t nIt12_num;
   /** set of timestamps, stage ACT12 */
   IFX_uint32_t pT12[6];
   /** levelmeter result from current measurement, stage ACT12 */
   VINETIC_IO_LT_RESULT I_ACT12;
   /** levelmeter result from voltage measurement, stage ACT12 */
   VINETIC_IO_LT_RESULT U_ACT12;
   /** levelmeter result from current measurement, stage RAMP13 */
   VINETIC_IO_LT_RESULT I_RAMP13;
   /** ramp slope as used in stage RAMP5 */
   IFX_uint32_t nSlope;
   /** SLIC identifier used for the channel */
   VINETIC_IO_SLIC_TYPES nSlic;
} VINETIC_IO_LT_NET_TIP_RING_RESULT;

typedef struct _VINETIC_IO_LT_INIT_ACT
{
   /** levelmeter result from current measurement IT, stage ACT1 */
   VINETIC_IO_LT_RESULT IT;
   /** levelmeter result from voltage measurement UTR, stage ACT1 */
   VINETIC_IO_LT_RESULT UTR;
   /** levelmeter result from voltage measurement UTG, stage ACT1 */
   VINETIC_IO_LT_RESULT UTG;
   /** levelmeter result from voltage measurement URG, stage ACT1 */
   VINETIC_IO_LT_RESULT URG;
} VINETIC_IO_LT_INIT_ACT;

typedef struct _VINETIC_IO_LT_ACT
{
   /** levelmeter result from current measurement, IT */
   VINETIC_IO_LT_RESULT IT;
   /** levelmeter results from voltage measurement, Ufloat */
   VINETIC_IO_LT_RESULT Ufloat[4];
   /** set of timestamps for Ufloat values */
   IFX_uint32_t Tfloat[4];
   /** levelmeter result from voltage measurement, Ufixed */
   VINETIC_IO_LT_RESULT Ufixed;
} VINETIC_IO_LT_ACT;

/** Network linetesting, Tip Groung, Ring Ground result structure */
typedef struct _VINETIC_IO_LT_NET_GND_RESULT
{
   IFX_uint32_t nSlope;
   /** levelmeter result from current and voltage measurement, stage ACT1 */
   VINETIC_IO_LT_INIT_ACT  ACT1;
   /** levelmeter result from current measurement, stage RAMP3 */
   VINETIC_IO_LT_RESULT    I_RAMP3;
   /** levelmeter result from current and voltage measurement, stage ACT4 */
   VINETIC_IO_LT_ACT       ACT4;
   /** levelmeter result from current measurement, stage RAMP5 */
   VINETIC_IO_LT_RESULT    I_RAMP5;
   /** levelmeter result from current and voltage measurement, stage ACT6 */
   VINETIC_IO_LT_ACT       ACT6;
   /** levelmeter result from current measurement, stage RAMP9 */
   VINETIC_IO_LT_RESULT    I_RAMP9;
   /** levelmeter result from current and voltage measurement, stage ACT10 */
   VINETIC_IO_LT_ACT       ACT10;
   /** levelmeter result from current measurement, stage RAMP11 */
   VINETIC_IO_LT_RESULT    I_RAMP11;
   /** levelmeter result from current and voltage measurement, stage ACT12 */
   VINETIC_IO_LT_ACT       ACT12;
} VINETIC_IO_LT_NET_GND_RESULT;

/** Network linetesting, Discharge function result structure */
typedef struct _VINETIC_IO_LT_DISCHARGE_RESULT
{
   /** initial tip/ring voltage that is discharged from (PDR mode) */
   IFX_int32_t          U_PDR1_dischg;
   /** initial tip/ring voltage that is discharged from (ACT mode) */
   IFX_int32_t          U_ACT1_dischg;
   /** original Vlim setting */
   IFX_uint16_t         orig_VLIM;
   /** original Vk1 setting */
   IFX_uint16_t         orig_VK1;
   /** original Ik1 setting */
   IFX_uint16_t         orig_IK1;
} VINETIC_IO_LT_DISCHARGE_RESULT;

/** Linetesting Measurement Result Configuration */
typedef struct
{
   /** Chip Revision */
   unsigned char        nHwRevision;
   /** Linemode when the measurement was done */
   unsigned short       nLineMode;           /* TBD <- should be an enum */
} VINETIC_IO_LT_RESULT_CONFIG;

/** Linetesting Current Measurement Mode Selection */
typedef enum
{
   /** select DC Current IT */
   VINETIC_IO_LT_CURRENT_MODE_IT,
   /** select DC Current IL */
   VINETIC_IO_LT_CURRENT_MODE_IL
} VINETIC_IO_LT_CURRENT_MODE;

/** Linetesting Current Measurement Submode Selection */
typedef enum
{
   /** measure the DC part of the current */
   VINETIC_IO_LT_CURRENT_SUBMODE_DC,
   /** measure the AC part of the current */
   VINETIC_IO_LT_CURRENT_SUBMODE_AC
} VINETIC_IO_LT_CURRENT_SUBMODE;

/** Linetesting Current Measurement */
typedef struct
{
   /** Current Measurement Mode Selection (IT, IL) */
   VINETIC_IO_LT_CURRENT_MODE       mode;
   /** Current Measurement Submode Selection (DC, AC) */
   VINETIC_IO_LT_CURRENT_SUBMODE    submode;

   /** Level Meter Result Configuration */
   VINETIC_IO_LT_RESULT_CONFIG      cfg;
   /** Level Meter Result */
   VINETIC_IO_LT_RESULT             res;
} VINETIC_IO_LT_CURRENT;


/** Linetesting Voltage Measurement Mode Selection */
typedef enum
{
   /** Select differential voltage measurement IO4 - IO3 */
   VINETIC_IO_LT_VOLTAGE_MODE_IO4_IO3,
   /** Select differential voltage measurement IO4 - IO2 */
   VINETIC_IO_LT_VOLTAGE_MODE_IO4_IO2,
   /** Select differential voltage measurement IO3 - IO2 */
   VINETIC_IO_LT_VOLTAGE_MODE_IO3_IO2,
   /** Select voltage measurement DCN-DCP */
   VINETIC_IO_LT_VOLTAGE_MODE_DCN_DCP,
   /** Select voltage measurement on IO4 */
   VINETIC_IO_LT_VOLTAGE_MODE_IO4,
   /** Select voltage measurement on IO3 */
   VINETIC_IO_LT_VOLTAGE_MODE_IO3,
   /** Select voltage measurement on IO2 */
   VINETIC_IO_LT_VOLTAGE_MODE_IO2,
   /** Select voltage measurement on VDD */
   VINETIC_IO_LT_VOLTAGE_MODE_VDD
} VINETIC_IO_LT_VOLTAGE_MODE;

/** Linetesting Voltage Measurement Submode Selection */
typedef enum
{
   /** Select DC voltage measurement */
   VINETIC_IO_LT_VOLTAGE_SUBMODE_DC,
   /** Select AC voltage measurement */
   VINETIC_IO_LT_VOLTAGE_SUBMODE_AC
} VINETIC_IO_LT_VOLTAGE_SUBMODE;

/** Linetesting Voltage Measurement */
typedef struct
{
   /** Voltage measurement mode selection (LM_SEL) */
   VINETIC_IO_LT_VOLTAGE_MODE       mode;
   /** Voltage measurement submode slection (AC, DC) */
   VINETIC_IO_LT_VOLTAGE_SUBMODE    submode;
   /** Voltage measurement select foreign voltage\n
       (if set to TRUE the slic is set to HIRT mode during the measurement) */
   unsigned char                    bForeignVoltage;

   /** Level Meter Result Configuration */
   VINETIC_IO_LT_RESULT_CONFIG      cfg;
   /** Level Meter Result */
   VINETIC_IO_LT_RESULT             res;
} VINETIC_IO_LT_VOLTAGE;


/** Linetesting Resistance Measurement Mode Selection */
typedef enum
{
   /** Resistance will be measured between Ring and Tip */
   VINETIC_IO_LT_RESISTANCE_MODE_RING_TIP,
   /** Resistance will be measured between Ring and Ground */
   VINETIC_IO_LT_RESISTANCE_MODE_RING_GROUND,
   /** Resistance will be measured between Tip and Ground */
   VINETIC_IO_LT_RESISTANCE_MODE_TIP_GROUND,
   /** Resistance will be measured between Ring and Battery */
   VINETIC_IO_LT_RESISTANCE_MODE_RING_BATTERY,
   /** Resistance will be measured between Tip and Battery */
   VINETIC_IO_LT_RESISTANCE_MODE_TIP_BATTERY
} VINETIC_IO_LT_RESISTANCE_MODE;

/** Linetesting Resistance Measurement */
typedef struct
{
   /** Linetesting Resistance Measurement Mode Selection\n
       (select between Tip/Ring, Tip/Ground, Ring/Ground) */
   VINETIC_IO_LT_RESISTANCE_MODE    mode;
   /** Linetesting Resistance Measurement Submode Selection\n
       (the application has to tell the driver which io pins
        are connected to Tip/Ring)  */
   VINETIC_IO_LT_VOLTAGE_MODE       submode;

   /** Level Meter Result Configuration */
   VINETIC_IO_LT_RESULT_CONFIG      cfg;
   /** Level Meter Result first voltage */
   VINETIC_IO_LT_RESULT             res_U1;
   /** Level Meter Result second voltage
      (not used for off hook resistance measurement) */
   VINETIC_IO_LT_RESULT             res_U2;
   /** Level Meter Result first current */
   VINETIC_IO_LT_RESULT             res_I1;
   /** Level Meter Result second current
      (not used for off hook resistance measurement) */
   VINETIC_IO_LT_RESULT             res_I2;
} VINETIC_IO_LT_RESISTANCE;

/** Linetesting Resistance Measurement via RC network */
typedef struct
{
   /** Linetesting Resistance Measurement Mode Selection\n
       (select between Tip/Ground and Ring/Ground.\n
        Ring/Tip is not supported!) */
   VINETIC_IO_LT_RESISTANCE_MODE    mode;
/*   VINETIC_IO_LT_RESISTANCE_SUBMODE submode;*/
   /** Linetesting Resistance Measurement Submode Selection\n
       (the application has to tell the driver which io pins
        are connected to Tip/Ring)  */
   VINETIC_IO_LT_VOLTAGE_MODE       iopin;

   /** Level Meter Result Configuration */
   VINETIC_IO_LT_RESULT_CONFIG      cfg;
   /** Level Meter Result first voltage */
   VINETIC_IO_LT_RESULT             res;
   /** Reciprocal value of the threshold scaling factor */
   int                              reciprocal_scale;
} VINETIC_IO_LT_RESISTANCE_RC;


/** Linetesting Capacitance Measurement Mode Selection */
typedef enum
{
   /** Capacitance will be measured between Ring and Tip */
   VINETIC_IO_LT_CAPACITANCE_MODE_RING_TIP,
   /** Capacitance will be measured between Ring and Ground */
   VINETIC_IO_LT_CAPACITANCE_MODE_RING_GROUND,
   /** Capacitance will be measured between Tip and Ground */
   VINETIC_IO_LT_CAPACITANCE_MODE_TIP_GROUND
} VINETIC_IO_LT_CAPACITANCE_MODE;


/** Linetesting Capacitance Measurement */
typedef struct
{
   /** Linetesting Capacitance Measurement Mode Selection\n
       (select beween Tip/Ring, Tip/Ground, Ring/Ground) */
   VINETIC_IO_LT_CAPACITANCE_MODE   mode;

   /** Level Meter Result Configuration */
   VINETIC_IO_LT_RESULT_CONFIG      cfg;
   /** Level Meter Result (rising and falling slope current) */
   VINETIC_IO_LT_RESULT             res[2];
   /** returns the ramp slope the driver decided
       to use for this measurement */
   long                             slope;
} VINETIC_IO_LT_CAPACITANCE;


/** Linetesting  Impedance Measurement Mode Selection */
typedef enum
{
   /** Impedance will be measured between Ring and Tip */
   VINETIC_IO_LT_IMPEDANCE_MODE_RING_TIP,
   /** Impedance will be measured between Ring and Ground */
   VINETIC_IO_LT_IMPEDANCE_MODE_RING_GROUND,
   /** Impedance will be measured between Tip and Ground */
   VINETIC_IO_LT_IMPEDANCE_MODE_TIP_GROUND
} VINETIC_IO_LT_IMPEDANCE_MODE;

/** Linetesting  Impedance Measurement Submode Selection */
typedef enum
{
   /** Select the Ring Generator as signal source
       for the Impedance measurement */
   VINETIC_IO_LT_IMPEDANCE_SUBMODE_RG,
   /** Select the Tone Generator as signal source
       for the Impedance measurement */
   VINETIC_IO_LT_IMPEDANCE_SUBMODE_TG
} VINETIC_IO_LT_IMPEDANCE_SUBMODE;

/** Linetesting Impedance Measurement */
typedef struct
{
   /** Mode Selection\n
       (select between Ring/Tip, Ring/Ground and Tip/Ground) */
   VINETIC_IO_LT_IMPEDANCE_MODE     mode;
   /** Submode Selection\n
       (select signal source for the Impedance measurement) */
   VINETIC_IO_LT_IMPEDANCE_SUBMODE  submode;
   /** signal frequency to be used for impedance measurement */
   long                             freq;
   /** signal level to be used for the impedance measurement */
   long                             level;

   /** Level Meter Result Configuration */
   VINETIC_IO_LT_RESULT_CONFIG      cfg;
   /** Level Meter Result */
   VINETIC_IO_LT_RESULT             res;
} VINETIC_IO_LT_IMPEDANCE;

/** Network measurements status and error codes */
typedef enum
{
   /** measurement successful */
   VINETIC_IO_LT_NET_SUCCESS,
   /** offhook detected during measurement */
   VINETIC_IO_LT_NET_ERR_OFFHOOK_DETECTED,
   /** measurement attempted in neither PDR nor ACT mode */
   VINETIC_IO_LT_NET_ERR_INVALID_OPMODE,
   /** other error occured while performing the test */
   VINETIC_IO_LT_NET_ERR_OTHER,
   /* test not supported for the underlying VINETIC device version */
   VINETIC_IO_LT_NET_ERR_VIN_UNSUPP,
   /* test not supported for the underlying SLIC version */
   VINETIC_IO_LT_NET_ERR_SLIC_UNSUPP
} VINETIC_IO_LT_NET_ERRORS;

/** Allowed operating modes during network measurements */
typedef enum
{
   VINETIC_IO_LT_NET_PDR,
   VINETIC_IO_LT_NET_ACT
} VINETIC_IO_LT_NET_OPMODE;

/** Linetesting Discharge function */
typedef struct _VINETIC_IO_LT_DISCHARGE
{
   /** measurement status/error number [out]\n
      (element position must be preserved) */
   VINETIC_IO_LT_NET_ERRORS         nErr;
   /** flag is set if function called within TAPI_LT_LL_Network function [in] */
   IFX_uint8_t                      bNet;
   /** targeted channel number (0..n-1) [out] */
   IFX_uint8_t                      nChannel;
   /** underlying VINETIC device revision number [out] */
   IFX_uint8_t                      nChip;
   /** underlying SLIC type version number [out] */
   IFX_uint8_t                      nSlic;
   /** operating mode in which the function was started [out] */
   VINETIC_IO_LT_NET_OPMODE         nOpMode;
   /** voltage level to discharge to (V) [in] */
   IFX_int32_t                      nUdischarge;
   /** set of values necessary for the following Charge function [out] */
   VINETIC_IO_LT_DISCHARGE_RESULT   res;
} VINETIC_IO_LT_DISCHARGE;

/** Linetesting Charge function */
typedef struct _VINETIC_IO_LT_CHARGE
{
   /** measurement status/error number [out]\n
      (element position must be preserved) */
   VINETIC_IO_LT_NET_ERRORS         nErr;
   /** flag is set if function called within TAPI_LT_LL_Network function [in] */
   IFX_uint8_t                      bNet;
   /** targeted channel number (0..n-1) [out] */
   IFX_uint8_t                      nChannel;
   /** underlying VINETIC device revision number [out] */
   IFX_uint8_t                      nChip;
   /** underlying SLIC type version number [out] */
   IFX_uint8_t                      nSlic;
   /** operating mode in which the function was started [out] */
   VINETIC_IO_LT_NET_OPMODE         nOpMode;
   /** set of values provided by the preceeding discharge function [in] */
   VINETIC_IO_LT_DISCHARGE_RESULT   res;
} VINETIC_IO_LT_CHARGE;

/** Network measurements status and error codes */
typedef enum
{
   VINETIC_IO_LT_NET_GNDK_OFF,
   VINETIC_IO_LT_NET_GNDK_ON,
} VINETIC_IO_LT_NET_CMD;

/** Linetesting Charge function */
typedef struct _VINETIC_IO_LT_NETWORK
{
   /** targeted channel number (0..n-1) [out] */
   VINETIC_IO_LT_NET_CMD            cmd;
} VINETIC_IO_LT_NETWORK;

/** Linetesting Network Tip/Ring Measurement */
typedef struct
{
   /** measurement status/error number [out]\n
      (element position must be preserved) */
   VINETIC_IO_LT_NET_ERRORS         nErr;
   /** flag is set if function called within TAPI_LT_LL_Network function [in] */
   IFX_uint8_t                      bNet;
   /** enables signature detection sequence [in] */
   IFX_uint8_t                      bSign;
   /** measuring channel number (0..n-1) [out] */
   IFX_uint8_t                      nChannel;
   /** underlying VINETIC device revision number [out] */
   IFX_uint8_t                      nChip;
   /** underlying SLIC type version number [out] */
   IFX_uint8_t                      nSlic;
   /** operating mode in which the function was started [out] */
   VINETIC_IO_LT_NET_OPMODE         nOpMode;
   /** levelmeter results [out] */
   VINETIC_IO_LT_NET_TIP_RING_RESULT res;
} VINETIC_IO_LT_NETWORK_TIP_RING;

/** Linetesting Network to Ground (Tip/Gnd, Ring/Gnd) Measurement */
typedef struct
{
   /** measurement status/error number [out]\n
      (element position must be preserved) */
   VINETIC_IO_LT_NET_ERRORS         nErr;
   /** flag is set if function called within TAPI_LT_LL_Network function [in] */
   IFX_uint8_t                      bNet;
   /** measuring channel number (0..n-1) [out] */
   IFX_uint8_t                      nChannel;
   /** underlying VINETIC device revision number [out] */
   IFX_uint8_t                      nChip;
   /** underlying SLIC type version number [out] */
   IFX_uint8_t                      nSlic;
   /** levelmeter results [out] */
   VINETIC_IO_LT_NET_GND_RESULT     res;
} VINETIC_IO_LT_NETWORK_GND;

/** Bandpass quality selection (Vinetic 1.x only) */
typedef enum
{
   /** Select bandpass quality 1 */
   VINETIC_IO_LT_BP_QUAL_1   = 0,
   /** Select bandpass quality 2 */
   VINETIC_IO_LT_BP_QUAL_2   = 1,
   /** Select bandpass quality 4 */
   VINETIC_IO_LT_BP_QUAL_4   = 2,
   /** Select bandpass quality 8 */
   VINETIC_IO_LT_BP_QUAL_8   = 3,
   /** Select bandpass quality 16 */
   VINETIC_IO_LT_BP_QUAL_16  = 4,
   /** Select bandpass quality 32 */
   VINETIC_IO_LT_BP_QUAL_32  = 5,
   /** Select bandpass quality 64 */
   VINETIC_IO_LT_BP_QUAL_64  = 6,
   /** Select bandpass quality 128 */
   VINETIC_IO_LT_BP_QUAL_128 = 7
} VINETIC_IO_LT_BP_QUAL;


/** Linetesting AC Measurement Mode Selection */
typedef enum
{
   /** Select the PCM4 Level Measurement */
   VINETIC_IO_LT_AC_MODE_LEVEL,
   /** Select the PCM4 GainTracking Measurement */
   VINETIC_IO_LT_AC_MODE_GAINTRACKING,
   /** Select the PCM4 Transhybrid Measurement */
   VINETIC_IO_LT_AC_MODE_TRANSHYBRID,
   /** Select the PCM4 Idle Channel Noise Measurement */
   VINETIC_IO_LT_AC_MODE_IDLENOISE,
   /** Select the PCM4 SNR Measurement */
   VINETIC_IO_LT_AC_MODE_SNR
} VINETIC_IO_LT_AC_MODE;

/** Linetesting AC Measurement */
typedef struct
{
   /** Linetesting AC Measurement Mode Selection\n
       (select the PCM4 measurement to be executed) */
   VINETIC_IO_LT_AC_MODE            mode;
   /** signal frequency to be used for this measurement */
   int                              freq;
   /** signal level to be used for this measurement */
   int                              level;

   /** Level Meter Result Configuration */
   VINETIC_IO_LT_RESULT_CONFIG      cfg;
   /** Level Meter Result */
   VINETIC_IO_LT_RESULT             res;
   /** Level Meter Result (only used for SNR measurement) */
   VINETIC_IO_LT_RESULT             res_noise;
} VINETIC_IO_LT_AC_MEASUREMENTS;


/** Linetesting AC Direct Measurement Transhybrid Filter Configuration */
typedef enum
{  /* do not change the order of elements */
   /** switch transhybrid filter on during the measurement */
   VINETIC_IO_LT_AC_TH_FILTER_ON,
   /** switch transhybrid filter off during the measurement */
   VINETIC_IO_LT_AC_TH_FILTER_OFF
} VINETIC_IO_LT_AC_TH_FILTER;

/** Linetesting AC Direct Measurement Level Meter Filter Structure */
typedef enum
{
   /** select to bypass the level meter filter */
   VINETIC_IO_LT_AC_LM_FILTER_OFF,
   /** select a bandpass filter structure in the level meter filter */
   VINETIC_IO_LT_AC_LM_FILTER_BP,
   /** select a notch filter structure in the level meter filter */
   VINETIC_IO_LT_AC_LM_FILTER_NOTCH,
   /** select a low pass filter structure in the level meter filter */
   VINETIC_IO_LT_AC_LM_FILTER_LP,
   /** select a high pass filter structure in the level meter filter */
   VINETIC_IO_LT_AC_LM_FILTER_HP,
   /** don't change the filter settings */
   VINETIC_IO_LT_AC_LM_FILTER_IGNORE            = 0xFF
} VINETIC_IO_LT_AC_LM_FILTER_TYPE;

/** Linetesting AC Direct Measurement Input Selection */
typedef enum
{
   /** select TX path to be measured */
   VINETIC_IO_LT_AC_LM_SELECT_TX,
   /** select RX path to be measured */
   VINETIC_IO_LT_AC_LM_SELECT_RX,
   /** select sum of RX and TX path to be measured */
   VINETIC_IO_LT_AC_LM_SELECT_RX_TX
} VINETIC_IO_LT_AC_LM_SELECT;

/** Linetesting AC Direct Measurement Level Meter Filter Configuration */
typedef struct
{
   /** select level meter filter structure */
   VINETIC_IO_LT_AC_LM_FILTER_TYPE  type;
   /** select level meter bandpass quality (only Vinetic 1.x) */
   VINETIC_IO_LT_BP_QUAL            qual;
   /** select level meter filter frequency */
   int                              freq;
} VINETIC_IO_LT_AC_LM_FILTER;

/** Linetesting  AC Direct Measurement Tone Generator State */
typedef enum
{  /* do not change the order of elements */
   /** switch tone generator off */
   VINETIC_IO_LT_AC_TG_STATE_OFF,
   /** switch tone generator on */
   VINETIC_IO_LT_AC_TG_STATE_ON,
   /** do not change the tone generator configuration */
   VIENTIC_IO_LT_AC_TG_STATE_IGNORE
} VINETIC_IO_LT_AC_TG_STATE;

/** Linetesting AC Direct Measurement Tone Generator Selection */
typedef enum
{  /* do not change the order of elements */
   /** select tone generator 1 */
   VINETIC_IO_LT_TG_1,
   /** select tone generator 2 */
   VINETIC_IO_LT_TG_2
} VINETIC_IO_LT_TG;

/** Linetesting  AC Direct Measurement Tone Generator Configuration */
typedef struct
{
   /** select tone generator state (on, off, unchanged) */
   VINETIC_IO_LT_AC_TG_STATE        state;
   /** select tone generator frequency */
   int                              freq;
   /** select tone generator level */
   int                              level;
   /** select tone generator quality (only used for Vinetic 1.x) */
   VINETIC_IO_LT_BP_QUAL            qual;
} VINETIC_IO_LT_AC_TG;

/** Linetesting  AC Direct Measurement\n
    (all parameters of the measurement can be configured by the application) */
typedef struct
{

   VINETIC_IO_LT_RS                 rsSelect;
   /** Linetesting  AC Direct Measurement Transhybrid Filter Configuration */
   VINETIC_IO_LT_AC_TH_FILTER       thFilter;
   /** Linetesting AC Direct Measurement Level Meter Filter Structure */
   VINETIC_IO_LT_AC_LM_FILTER       lmFilter;
   /** Linetesting AC Direct Measurement Input Selection */
   VINETIC_IO_LT_AC_LM_SELECT       lmSelect;
   /** Linetesting AC Direct Measurement Tone Generator Selection */
   VINETIC_IO_LT_AC_TG              tg[2];

   /** Level Meter Result Configuration */
   VINETIC_IO_LT_RESULT_CONFIG      cfg;
   /** Level Meter Result */
   VINETIC_IO_LT_RESULT             res;
} VINETIC_IO_LT_AC_DIRECT;


/** Offset Calibration Mode Selection */
typedef enum
{
   /** select the DC_SLIC offset calibration to be done */
   VINETIC_IO_OFFSET_CALIBRATION_MODE_DC_SLIC,
   /** select the DC_PREFI offset calibration to be done */
   VINETIC_IO_OFFSET_CALIBRATION_MODE_DC_PREFI,
   /** select the DC_SLIC offset with LM_PREGAIN enabled, VINETIC 2.1 only */
   VINETIC_IO_OFFSET_CALIBRATION_MODE_DC_SLIC_PREGAIN,
   /** select the all offset calibrations to be done */
   VINETIC_IO_OFFSET_CALIBRATION_MODE_ALL            = 0xFF
} VINETIC_IO_OFFSET_CALIBRATION_MODE;

/** Offset Calibration Results */
typedef struct
{
   /** returns the OFR values for the calibrated channels */
   short nOFR [VINETIC_4VIP_CH_NR];
} VINETIC_IO_OFFSET_VALUE;

/** Offset Calibration */
typedef struct
{
   /** select the channel to be offset calibrated (0xff for all
       channels, 0x80 for channel corresponding to the calling
       channel's file descriptor) */
   unsigned char                        nChannel;
   /** select the offset calibration mode (DC_SLIC, DC_PREFI) */
   VINETIC_IO_OFFSET_CALIBRATION_MODE   nMode;

   /** returns the offsets for DC_SLIC calibration */
   VINETIC_IO_OFFSET_VALUE              nSlic;
   /** returns the offsets for DC_PREFI calibration */
   VINETIC_IO_OFFSET_VALUE              nPrefi;
   /** returns the offsets for DC_SLIC calibration with LM_PREGAIN,
       VINETIC 2.1 only */
   VINETIC_IO_OFFSET_VALUE              nSlicPregain;
   /** returns the chip revision */
   unsigned char                        nHwRevision;
   /** returns success or an error code\n
       for each channel the according bit is set in an error case\n
       e.g. bit 0 is set for channel 0, etc. */
   long                                 error;
} VINETIC_IO_OFFSET_CALIBRATION;

/* ============================= */
/* GR909 real time measurements  */
/* ============================= */

/** GR909 possible measurements */
typedef enum
{
   /** GR909 Hazardeous Voltage measurement */
   GR909_HAZARDOUS_VOLTAGE = 0,
   /** GR909 Foreign Force measurement */
   GR909_FOREIGN_FORCE,
   /** GR909 Resistive Fault measurement */
   GR909_RESISTIVE_FAULT,
   /** GR909 Receiver Offhook measurement */
   GR909_RECEIVER_OFF_HOOK,
   /** GR909 Ringer measurement */
   GR909_RINGER,
   /** number of possible GR909 measurements */
   MAX_GR909_TESTS
}GR909_TESTS;

/** GR909 Measurements status values */
typedef enum
{
   /** GR909 Test OK */
   GR909_TEST_OK              = 0,
   /** GR909 Test ongoing */
   GR909_TEST_RUNNING         = 0x1,
   /** GR909 test failed */
   GR909_TEST_FAILED,
   /** GR909 Measurement failed due to an internal error */
   GR909_MEASUREMENT_FAILED   = 0x07
}GR909_TEST_RESULT;

/** Measurement reults parameters alignment in result buffer */
typedef enum
{
   /******************/
   /* Voltage Tests  */
   /******************/
   /* HFV = Hazardeous and Foreign Voltages */
   /** AC scaling factor used to avoid 32-bit overflows */
   HFV_AC_SCALFACT = 0,
   /** Ring/Ground DC */
   HFV_RG_DC,
   /** Ring/Ground DC part of AC */
   HFV_RG_DC_AC,
   /** Ring/Ground AC */
   HFV_RG_AC,
   /** Ring/Ground Period */
   HFV_RG_PERIOD,
   /** Tip/Gnd DC */
   HFV_TG_DC,
   /** Tip/Gnd DC part of AC */
   HFV_TG_DC_AC,
   /** Tip/Gnd AC */
   HFV_TG_AC,
   /** Tip/Gnd Period */
   HFV_TG_PERIOD,
   /** Tip/Ring DC */
   HFV_TR_DC,
   /** Tip/Ring DC part of AC*/
   HFV_TR_DC_AC,
   /** Tip/Ring AC */
   HFV_TR_AC,
   /** Tip/Ring Period */
   HFV_TR_PERIOD,
   /**************************/
   /* Resistive Faults Tests */
   /**************************/
   /* RF = Resistive Faults */
   /** Resistive Fault current normal polarity Tip/Ring */
   RF_IT_TR_NP = 0,
   /** Resistive Fault voltage normal polarity Tip/Ring */
   RF_V_TR_NP,
   /** Resistive Fault current reverse polarity Tip/Ring */
   RF_IT_TR_RP,
   /** Resistive Fault voltage reverse polarity Tip/Ring */
   RF_V_TR_RP,
   /** Resistive Fault current normal polarity Tip/Ground */
   RF_IT_TG_NP,
   /** Resistive Fault voltage normal polarity Tip/Ground */
   RF_V_TG_NP,
   /** Resistive Fault current reverse polarity Tip/Ground */
   RF_IT_TG_RP,
   /** Resistive Fault voltage reverse polarity Tip/Ground */
   RF_V_TG_RP,
   /** Resistive Fault current normal polarity Ring/Gnd */
   RF_IT_RG_NP,
   /** Resistive Fault voltage normal polarity Ring/Gnd */
   RF_V_RG_NP,
   /** Resistive Fault current reverse polarity Ring/Gnd */
   RF_IT_RG_RP,
   /** Resistive Fault voltage reverse polarity Ring/Gnd */
   RF_V_RG_RP,
   /**************************/
   /* Receiver Offhook Tests */
   /**************************/
   /* RO = Receiver Offhook */
   /** Receiver Offhook current for 15 mA DC regulation */
   RO_15MA_IT = 0,
   /** Receiver Offhook DCN-DCP feed voltage for 15 mA DC regulation */
   RO_15MA_VOLT,
   /** Receiver Offhook current for 30 mA DC regulation */
   RO_30MA_IT,
   /** Receiver Offhook DCN-DCP feed voltage for 30 mA DC regulation */
   RO_30MA_VOLT,
   /*****************/
   /* Ringer  Tests */
   /*****************/
   /* RNG = Ringer */
   /** Ringer AC rms */
   RNG_AC_RMS = 0,
   /** Ringer DC */
   RNG_DC_VOLT,
   /** Ringer sample rate */
   RNG_SAMPLNR,
   /** Ringer shift factor */
   RNG_SHIFT,
   /** Ringer current */
   RNG_IT_RESULT
}GR909_RESULT_PARAM;

/** GR909 IO Structure.
   This structure is used for the ioctls \ref FIO_VINETIC_GR909_START and
   \ref FIO_VINETIC_GR909_RESULT.
*/
typedef struct
{
   /** GR909 measurement user control flag
       \arg 0 Single meaasurement will be done.
       \arg 1 All measurements will be done in the user state machine.
   */
   unsigned char     bAll;
   /** Actual measurement mode */
   unsigned char     mode;
   /** buffer for measurements results. Indexes in \ref GR909_RESULT_PARAM
       are used for each measurement */
   long    values[MAX_GR909_VAL];
   /** Measurement status
       \arg GR909_TEST_OK            Measurement processed without error
       \arg GR909_TEST_RUNNING       Measurement on going
       \arg GR909_TEST_FAILED        Test failed.
       \arg GR909_MEASUREMENT_FAILED Measurement failed due to internal error
   */
   unsigned char     status;
   /** Vinetic chip version for which measurement are done */
   unsigned char     nChipRev;
} VINETIC_IO_GR909;

/** @} */
#else /* VIN_2CPE */
/* ============================= */
/*    CPE L i n e t e s t i n g  */
/* ============================= */

/** \addtogroup VIN_DRIVER_INTERFACE_CPE_LT
 @{ */
/** GR909 Powerline frequency selection */
typedef enum _VINETIC_IO_GR909_POWERLINE_FREQ
{
   /** EU/Europe like countries */
   VINETIC_IO_GR909_EU_50HZ = 0,
   /** US like countries */
   VINETIC_IO_GR909_US_60HZ
} VINETIC_IO_GR909_Powerline_Freq_t;

/** GR909 Tests modes */
typedef enum _VINETIC_IO_GR909_TEST
{
   /** Hazardous Potential Test */
   VINETIC_IO_GR909_HPT  = 0x1,
   /** Foreign Electromotive Forces Test */
   VINETIC_IO_GR909_FEMF = 0x2,
   /** Resistive Faults Test */
   VINETIC_IO_GR909_RFT  = 0x4,
   /** Receiver Offhook Test */
   VINETIC_IO_GR909_ROH  = 0x8,
   /** Ringer Impedance Test */
   VINETIC_IO_GR909_RIT  = 0x10
} VINETIC_IO_GR909_Test_t;

/** GR909 Test Start */
typedef struct _VINETIC_IO_GR909_START
{
   /** GR909 Powerline Frequency to use.
       See \ref VINETIC_IO_GR909_Powerline_Freq_t */
   VINETIC_IO_GR909_Powerline_Freq_t   pl_freq;
   /** GR909 Test Mask as value or combination of tests in
       \ref VINETIC_IO_GR909_Test_t */
   IFX_uint32_t                        test_mask;
} VINETIC_IO_GR909_Start_t;

/** GR909 Results */

/** GR909 Results */
typedef struct _VINETIC_IO_GR909_RESULT
{
   /** Valid results flag. See \ref VINETIC_IO_GR909_Test_t */
   IFX_uint32_t valid;
   /** Passed flag according to valid flag. See \ref VINETIC_IO_GR909_Test_t */
   IFX_uint32_t passed;
   /** HPT AC RING wire to GND result */
   IFX_int16_t HPT_AC_R2G;
   /** HPT AC TIP wire to GND result */
   IFX_int16_t HPT_AC_T2G;
   /** HPT AC TIP wire to RING wire result */
   IFX_int16_t HPT_AC_T2R;
   /** HPT DC RING wire to GND result */
   IFX_int16_t HPT_DC_R2G;
   /** HPT DC TIP wire to GND result */
   IFX_int16_t HPT_DC_T2G;
   /** HPT DC TIP wire to RING wire result */
   IFX_int16_t HPT_DC_T2R;
   /** FEMF AC RING wire to GND result */
   IFX_int16_t FEMF_AC_R2G;
   /** FEMF AC TIP wire to GND result */
   IFX_int16_t FEMF_AC_T2G;
   /** FEMF AC TIP wire to RING wire result */
   IFX_int16_t FEMF_AC_T2R;
   /** FEMF DC RING wire to GND result */
   IFX_int16_t FEMF_DC_R2G;
   /** FEMF DC TIP wire to GND result */
   IFX_int16_t FEMF_DC_T2G;
   /** FEMF DC TIP wire to RING wire result */
   IFX_int16_t FEMF_DC_T2R;
   /** RFT RING wire to GND result */
   IFX_int16_t RFT_R2G;
   /** RFT TIP wire to GND result */
   IFX_int16_t RFT_T2G;
   /** RFT TIP wire to RING wire result */
   IFX_int16_t RFT_T2R;
   /** ROH TIP wire to RING wire result for low voltage */
   IFX_int16_t ROH_T2R_L;
   /** ROH TIP wire to RING wire result for high voltage */
   IFX_int16_t ROH_T2R_H;
   /** RIT result */
   IFX_int16_t RIT_RES;
} VINETIC_IO_GR909_Result_t;
/** @} */
#endif /* VIN_2CPE */

/* =============================== */
/*    G P I O  I n t e r f a c e   */
/* =============================== */
#ifndef VIN_2CPE
/* ************************************************************************** */
typedef enum
{
   VINETIC_IO_DEV_GPIO_0               = 0x01,
   VINETIC_IO_DEV_GPIO_1               = 0x02,
   VINETIC_IO_DEV_GPIO_2               = 0x04,
   VINETIC_IO_DEV_GPIO_3               = 0x08,
   VINETIC_IO_DEV_GPIO_4               = 0x10,
   VINETIC_IO_DEV_GPIO_5               = 0x20,
   VINETIC_IO_DEV_GPIO_6               = 0x40,
   VINETIC_IO_DEV_GPIO_7               = 0x80
} VINETIC_IO_DEV_GPIO;

typedef enum
{
   VINETIC_IO_DEV_GPIO_CONTROLL_PHI,
   VINETIC_IO_DEV_GPIO_CONTROLL_RESERVED
} VINETIC_IO_DEV_GPIO_CONTROLL;

typedef enum
{
   VINETIC_IO_DEV_GPIO_DRIVER_DISABLED,
   VINETIC_IO_DEV_GPIO_DRIVER_ENABLED
} VINETIC_IO_DEV_GPIO_DRIVER;

typedef struct
{
   VINETIC_IO_DEV_GPIO                   nGPIO;
   VINETIC_IO_DEV_GPIO_CONTROLL          ctrld;
   VINETIC_IO_DEV_GPIO_DRIVER            drOut;
   VINETIC_IO_DEV_GPIO_DRIVER            drIn;
} VINETIC_IO_DEV_GPIO_CFG;

typedef struct
{
   unsigned char                         mask;
   unsigned char                         value;
} VINETIC_IO_DEV_GPIO_SET;
#endif /* VIN_2CPE */

/* ************************************************************************** */
/** \defgroup VIN_DRIVER_INTERFACE_GPIO GPIO Interface
    Control the device and channel specific IO pins */
/** @{ */

/** GPIO Control Structure */
typedef struct
{
   /** GPIO handle. The GPIO handle is returned with the successful reservation
       of GPIO pins. The handle has to be passed to all other GPIO commands. */
   int ioHandle;
   /** GPIO status, refer to the GPIO command for a description of this parameter */
   unsigned short nGpio;
   /** GPIO mask, refer to the GPIO command for a description of this parameter  */
   unsigned short nMask;
} VINETIC_IO_GPIO_CONTROL;

/** }@ */

/** @} */

/** @defgroup VIN_KERNEL_INTERFACE Driver Kernel Interface
    Kernel Interface for GPIO/IO pin handling
  @{  */

/* ======================================= */
/*    E x p o r t e d  V a r i a b l e s   */
/* ======================================= */
/** GPIO Configuration Structure */
typedef struct
{
    /** Mask for GPIO resources */
   unsigned short nGpio;
   /** GPIO mode (input, output, interrupt) */
   unsigned int nMode;
    /** Callback for interrupt routine */
   void (*callback)(int nDev, int nCh, unsigned short nEvt);
} VINETIC_GPIO_CONFIG;

/** GPIO pin configuration modes */

typedef enum
{
   GPIO_MODE_INPUT   =  0x0100,
   GPIO_MODE_OUTPUT =   0x0200,
   GPIO_MODE_INT =      0x0400,
   GPIO_INT_RISING   =  0x1000,
   GPIO_INT_FALLING =   0x2000,
   GPIO_INT_DUP_05 =    0x0000,
   GPIO_INT_DUP_45 =    0x0001,
   GPIO_INT_DUP_85 =    0x0002,
   GPIO_INT_DUP_125 =   0x0003,
   GPIO_INT_DUP_165 =   0x0004,
   GPIO_INT_DUP_205 =   0x0005,
   GPIO_INT_DUP_245 =   0x0006,
   GPIO_INT_DUP_285 =   0x0007,
   GPIO_INT_DUP_325 =   0x0008,
   GPIO_INT_DUP_365 =   0x0009,
   GPIO_INT_DUP_405 =   0x000A,
   GPIO_INT_DUP_445 =   0x000B,
   GPIO_INT_DUP_485 =   0x000C,
   GPIO_INT_DUP_525 =   0x000D,
   GPIO_INT_DUP_565 =   0x000E,
   GPIO_INT_DUP_605 =   0x000F
} VINETIC_GPIO_MODE;

/* ======================================= */
/*    E x p o r t e d  F u n c t i o n s   */
/* ======================================= */
/**
   Open the device from kernel mode.
\param
   nDev - index of the VINETIC device
\param
   nCh - index of the VINETIC channel (1 = channel 0, ...)
\return
   handle to device/channel or IFX_ERROR on error
\remarks
  If not already done this will
   - allocate internal memory for each new device
   - allocate io memory
   - initialize the device
   - set up the interrupt
*/
int VINETIC_OpenKernel(int nDev, int nCh);

/**
  Release a VINETIC IO or GPIO pin resource
\param
   nHandle - handle returned by VINETIC_GpioReserve
\return
   IFX_SUCCESS if releasing targeted IO/GPIO is possible, else IFX_ERROR
*/
int VINETIC_ReleaseKernel(int nHandle);

/**
  Reserve a VINETIC IO or GPIO pin resource.
\param
   devHandle - handle to either VINETIC device or channel structure
\param
   nGpio - mask for GPIOs to reserve (0 = free, 1 = reserve)
\return
   ioHandle if requested IO/GPIO is available, else IFX_ERROR
*/
int VINETIC_GpioReserve(int devHandle, unsigned short nGpio);

/**
  Release a VINETIC IO or GPIO pin resource
\param
   ioHandle - handle returned by VINETIC_GpioReserve
\return
   IFX_SUCCESS if releasing targeted IO/GPIO is possible, else IFX_ERROR
*/
int VINETIC_GpioRelease(int ioHandle);

/**
  Configure a VINETIC IO or GPIO pin
\param
   ioHandle - handle returned by VINETIC_GpioReserve
\param
   pCfg - handle to the GPIO configuration structure
\return
   IFX_SUCCESS if requested IO/GPIO is available and could be configured, else IFX_ERROR
*/
int VINETIC_GpioConfig(int ioHandle, VINETIC_GPIO_CONFIG *pCfg);

/**
  Set the value of a VINETIC IO or GPIO pin
\param
   ioHandle - handle returned by VINETIC_GpioReserve
\param
  nSet - values to store
\param
  nMask - only bits set to '1' will be stored
\return
   IFX_SUCCESS if requested IO/GPIO is available and could be written, else IFX_ERROR
*/
int VINETIC_GpioSet(int ioHandle, unsigned short nSet, unsigned short nMask);

/**
  Read the value from a VINETIC IO or GPIO pin
\param
   ioHandle - handle returned by VINETIC_GpioReserve
\param
  nGet - pointer where the read value shall be stored
\param
  nMask - only bits set to '1' will be stored
\return
   IFX_SUCCESS if requested IO/GPIO is available and could be read, else IFX_ERROR
*/
int VINETIC_GpioGet(int ioHandle, unsigned short *nGet, unsigned short nMask);

/**
   Set the interrupt enable mask
\param
   ioHandle - handle returned by VINETIC_GpioReserve
\param
  nSet - bitmask for interrupts to mask (0 = unmasked, 1 = masked)
\param
   nMask - mask to write to interrupt enable register
\param
  nMode - mode according to VINETIC_GPIO_MODE
\return
   IFX_SUCCESS if interrupt mask could be written, else IFX_ERROR
*/
int VINETIC_GpioIntMask(int ioHandle, unsigned short nSet, unsigned short nMask,
                        unsigned int nMode);

/** }@ */

/* =============================== */
/*    Polling  Interface           */
/* =============================== */

/** \addtogroup VIN_DRIVER_POLLING_INTERFACE  */
/** @{ */

/** Driver global configuration structure */
typedef struct
{
   /** driver polling/interrupt mode control flag
       \arg 0 configure driver for interrupt mode
       \arg 1 configure driver for polling   mode
   */
   int bPoll;
   /** packets control
       \arg 0 packets read by normal/interleave polling routines
       \arg 1 packets read on event polling
   */
   int bDataEvt;
   /** report events mode
       \arg 0 event read by application
       \arg 1 event read via poll/select
   */
   int bEvtSel;

   /** Pointer to buffer pool control structure. The pointer is used for
       the retrieve and return functions to identify the buffer handlers
       control structure. The buffer pool is expected to be preinitialized */
   void *pBufferPool;
   /** function pointer to retrieve a empty buffer from the pool. The buffer
       is protected and can be exclusively used by the user. The size is
       predefined by the buffer pool handler. */
   void * (* getBuf) (void *pBuf);
   /** function pointer to return a used buffer to the pool */
   int    (* putBuf) (void *pBuf);
} VINETIC_IO_DRVCTRL;

/* ======================================= */
/*    E x p o r t e d  F u n c t i o n s   */
/* ======================================= */

/** polling control function */
extern int VINETIC_IrqPollConf  (VINETIC_IO_DRVCTRL *pCtrl);
/** polling event handling */
extern void  VINETIC_Poll_Events (void);
/** Normal polling downstream interface functions */
extern int VINETIC_Poll_Down (const int nElements, void **ppBuffers);
/** Normal polling upstream interface functions */
extern int VINETIC_Poll_Up   (int *pElements, void **ppBuffers);
/** Interleaved polling downstream interface functions */
extern int VINETIC_Poll_DownIntlv (const int nElements, void **ppBuffers);
/** Interleaved polling upstream interface functions */
extern int VINETIC_Poll_UpIntlv   (int *pElements, void **ppBuffers);
/** }@ */
#endif /* _VINETIC_IO_H */
