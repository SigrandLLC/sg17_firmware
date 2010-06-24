#ifndef _DRV_TAPI_QOS_H
#define _DRV_TAPI_QOS_H
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
   Module      : drv_qos
   Date        : 2003-10-29
   Description :
   Remarks     :
               OS Supported : only Linux actually -> -DLINUX
               Driver compilation: -DQOS_SUPPORT mandatory
               Application compilation: -DQOS_SUPPORT -DAPP mandatory
*******************************************************************************/


/* ============================= */
/* Global Defines                */
/* ============================= */

/** magic number for QOS ioctls */
#define QOS_IOC_MAGIC 's'

typedef enum
{
   /** ioctl to initiate a session on a channel */
   FIO_QOS_START    =  _IO(QOS_IOC_MAGIC, 0x00),
   /** ioctl to activate a session with a specific
       port on a channel */
   FIO_QOS_ACTIVATE =  _IO(QOS_IOC_MAGIC, 0x01),
   /** ioctl to stop a session on a channel */
   FIO_QOS_STOP     =  _IO(QOS_IOC_MAGIC, 0x02),
   /** ioctl to cleanup qos support */
   FIO_QOS_CLEAN    =  _IO(QOS_IOC_MAGIC, 0x03)
} QOS_CMD;

/* delete all sessions and stop redirecting packets
   with this port */
#define QOS_PORT_CLEAN     0xFFFF /* or 65535 */

/* ============================= */
/* Global Structures             */
/* ============================= */

typedef struct
{
   IFX_uint16_t srcPort;
   IFX_uint32_t srcAddr;
   IFX_uint16_t destPort;
   IFX_uint32_t destAddr;
} QOS_INIT_SESSION;

#ifndef APP
/* ============================= */
/* Driver Global Structures      */
/* ============================= */

typedef enum
{
   /* No qos support */
   QOS_STAT_NOQOS,
   /* Qos initialization done and
      call back function registered */
   QOS_STAT_INIT
} QOS_STATUS;

typedef enum
{
   /* no packet redirection.
      to be used for local connection */
   QOS_EGRESS_NOREDIR,
   /* packet redirection after
      successfull session initiation */
   QOS_EGRESS_REDIR
} QOS_EGRESS;

typedef struct
{
   /** Qos support status
      \arg QOS_STAT_INIT  : Qos initialization already done
      \arg QOS_STAT_NOQOS : Qos cleanup done or Qos never initialized */
   QOS_STATUS       qosStat;
   /** egress packet redirect flag
      \arg QOS_EGRESS_NOREDIR : No Packet redirection (i.e local connection)
      \arg QOS_EGRESS_REDIR   : Packet redirection */
   QOS_EGRESS       egressFlag;
} QOS_CTRL;

/* ============================= */
/* Driver function declaration   */
/* ============================= */

/* Qos control function */
extern IFX_int32_t Qos_Ctrl(IFX_uint32_t chanDev,
                            QOS_CMD qosCmd,
                            IFX_uint32_t qosArg);
/* Cleanup Qos support using any channel device */
extern IFX_int32_t Qos_Cleanup(IFX_uint32_t chanDev);

#endif /* !APP */
#endif /* _DRV_TAPI_QOS_H */

