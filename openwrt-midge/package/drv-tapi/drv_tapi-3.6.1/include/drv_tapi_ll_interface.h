#ifndef _DRV_TAPI_LL_INTERFACE_H
#define _DRV_TAPI_LL_INTERFACE_H

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
   \file drv_tapi_ll_interface.h
   Contains the structures which are shared between the high-level TAPI and
    low-level TAPI drivers.
   These structures mainly contains the function pointers which will be exported
   by the low level driver.

   This file is divided in different sections depending on the firmware modules:
   - INTERRUPT_AND_PROTECTION_MODULE
   - CODER_MODULE
   - PCM_MODULE
   - SIG_MODULE
   - ALM_MODULE
   - CON_MODULE
   - MISC_MODULE

*/
/* ============================= */
/* Includes                      */
/* ============================= */

#include "ifx_types.h"
#include "drv_tapi_io.h"
#include "drv_tapi_kpi_io.h"
#ifdef TAPI_PACKET
#include "lib_fifo.h"
#endif /* TAPI_PACKET */
#ifdef QOS_SUPPORT
#include "drv_tapi_qos.h"
#endif /* QOS_SUPPORT */

/* ============================= */
/* Local Macros  Definitions    */
/* ============================= */
#define TAPI_MAX_DEVFS_HANDLES 20


#ifdef LINUX_2_6
#undef CONFIG_DEVFS_FS
#endif /* LINUX_2_6 */

/* Channel 255 indicates the control device */
#define IFX_TAPI_DEVICE_CH_NUMBER    255

/* channel flags */
enum CH_FLAGS
{
   /* indicates a non-blocking read driver's function */
   CF_NONBLOCK             = 0x00000008,
   /** Tapi Init mode voice */
   CF_TAPI_VOICEMODE       = 0x00000100,
   /** Tapi Init mode PCM */
   CF_TAPI_PCMMODE         = 0x00000200,
   /** Tapi Init mode PCM VINTIC S */
   CF_TAPI_S_PCMMODE       = 0x00000400,
   /* Indicates that a task is pending via select on this device */
   /* Only used in vxWorks */
   CF_NEED_WAKEUP          = 0x00100000,
   /* indication the wakeup source of this channel */
   /* only used in vxworks */
   CF_WAKEUPSRC_TAPI       = 0x00200000,
   CF_WAKEUPSRC_STREAM     = 0x00400000,
   CF_WAKEUPSRC_GR909      = 0x00800000
};

/** MFTD Answering tone detector phase reversal threshold
    Defines on which occurence of a phase reversal the signal is sent when
    enabled. Allowed range: 1-3. */
#define IFX_TAPI_SIG_MFTD_PR_THRESHOLD      (2)

/* Signal detection masks */

/** V.25 2100Hz (CED) Modem/Fax Tone mask */
#define IFX_TAPI_SIG_CEDMASK              (IFX_TAPI_SIG_CED  |         \
                                          IFX_TAPI_SIG_CEDTX |         \
                                          IFX_TAPI_SIG_CEDRX)
/** CNG Fax Calling Tone (1100 Hz) mask */
#define IFX_TAPI_SIG_CNGFAXMASK           (IFX_TAPI_SIG_CNGFAX  |      \
                                          IFX_TAPI_SIG_CNGFAXRX |      \
                                          IFX_TAPI_SIG_CNGFAXTX)
/** CNG Modem Calling Tone (1300 Hz) mask */
#define IFX_TAPI_SIG_CNGMODMASK           (IFX_TAPI_SIG_CNGMOD  |      \
                                          IFX_TAPI_SIG_CNGMODRX |      \
                                          IFX_TAPI_SIG_CNGMODTX)

/** Phase reversal detection mask */
#define IFX_TAPI_SIG_PHASEREVMASK         (IFX_TAPI_SIG_PHASEREV  |    \
                                          IFX_TAPI_SIG_PHASEREVRX |    \
                                          IFX_TAPI_SIG_PHASEREVTX)
/** Mask the paths for DIS */
#define IFX_TAPI_SIG_DISMASK              (IFX_TAPI_SIG_DIS  |         \
                                          IFX_TAPI_SIG_DISRX |         \
                                          IFX_TAPI_SIG_DISTX)

/** Amplitude modulation mask */
#define IFX_TAPI_SIG_AMMASK               (IFX_TAPI_SIG_AM  |          \
                                          IFX_TAPI_SIG_AMRX |          \
                                          IFX_TAPI_SIG_AMTX)

/** Modem tone holding signal stopped mask */
#define IFX_TAPI_SIG_TONEHOLDING_ENDMASK  (IFX_TAPI_SIG_TONEHOLDING_END  |  \
                                          IFX_TAPI_SIG_TONEHOLDING_ENDRX |  \
                                          IFX_TAPI_SIG_TONEHOLDING_ENDTX)
/** End of signal CED detection mask */
#define IFX_TAPI_SIG_CEDENDMASK           (IFX_TAPI_SIG_CEDEND |       \
                                          IFX_TAPI_SIG_CEDENDRX |      \
                                          IFX_TAPI_SIG_CEDENDTX)
/** V8bis mask */
#define IFX_TAPI_SIG_V8BISMASK            (IFX_TAPI_SIG_V8BISRX |      \
                                          IFX_TAPI_SIG_V8BISTX)

/** V.21L mark sequence mask */
#define IFX_TAPI_SIG_V21LMASK             (IFX_TAPI_SIG_EXT_V21L |     \
                                          IFX_TAPI_SIG_EXT_V21LRX |    \
                                          IFX_TAPI_SIG_EXT_V21LTX)

/** V.18A mark sequence mask */
#define IFX_TAPI_SIG_V18AMASK             (IFX_TAPI_SIG_EXT_V18A |     \
                                          IFX_TAPI_SIG_EXT_V18ARX |    \
                                          IFX_TAPI_SIG_EXT_V18ATX)

/** V.27, V.32 carrier mask */
#define IFX_TAPI_SIG_V27MASK              (IFX_TAPI_SIG_EXT_V27 |      \
                                          IFX_TAPI_SIG_EXT_V27RX |     \
                                          IFX_TAPI_SIG_EXT_V27TX)

/** Bell answering tone mask */
#define IFX_TAPI_SIG_BELLMASK             (IFX_TAPI_SIG_EXT_BELL |     \
                                          IFX_TAPI_SIG_EXT_BELLRX |    \
                                          IFX_TAPI_SIG_EXT_BELLTX)

/** V.22 unsrambled binary ones mask */
#define IFX_TAPI_SIG_V22MASK              (IFX_TAPI_SIG_EXT_V22 |      \
                                          IFX_TAPI_SIG_EXT_V22RX |     \
                                          IFX_TAPI_SIG_EXT_V22TX)

/* 2225 Hz or 2250 Hz single tone mask */
#define IFX_TAPI_SIG_V22ORBELLMASK        (IFX_TAPI_SIG_EXT_V22ORBELL |   \
                                          IFX_TAPI_SIG_EXT_V22ORBELLRX |  \
                                          IFX_TAPI_SIG_EXT_V22ORBELLTX)

/** V.32 AC mask */
#define IFX_TAPI_SIG_V32AXMASK            (IFX_TAPI_SIG_EXT_V32AC |    \
                                          IFX_TAPI_SIG_EXT_V32ACRX |   \
                                          IFX_TAPI_SIG_EXT_V32ACTX)

/* ================================ */
/* Enumerations                     */
/* ================================ */

/** Hook validation time array offsets for IFX_TAPI_LL_ALM_HookVt */
enum HOOKVT_IDX
{
   HOOKVT_IDX_HOOK_OFF = 0,
   HOOKVT_IDX_HOOK_ON,
   HOOKVT_IDX_HOOK_FLASH,
   HOOKVT_IDX_HOOK_FLASHMAKE,
   HOOKVT_IDX_DIGIT_LOW,
   HOOKVT_IDX_DIGIT_HIGH,
   HOOKVT_IDX_INTERDIGIT
};

/** currently unused in TAPI v3 */
typedef enum
{
   IFX_TAPI_MODULE_TYPE_NONE = 0
} IFX_TAPI_MODULE_TYPE_t;

typedef enum
{
   IFX_TAPI_LL_TONE_DIR_NONE = 0
} IFX_TAPI_LL_TONE_DIR_t;

typedef void            IFX_TAPI_LL_DEV_t;
typedef void            IFX_TAPI_LL_CH_t;
typedef IFX_uint16_t    IFX_TAPI_LL_ERR_t;

#ifndef DRV_TAPI_H
typedef struct _TAPI_DEV      TAPI_DEV;
typedef struct _TAPI_CHANNEL  TAPI_CHANNEL;
#endif /* DRV_TAPI_H */

typedef struct
{
   IFX_TAPI_COD_TYPE_t   dec_type;
   IFX_TAPI_COD_LENGTH_t dec_framelength;
} IFX_TAPI_DEC_DETAILS_t;


/** \defgroup TAPI_LL_INTERFACE TAPI Low-Level driver interface
   Lists all the functions which are registered by the low level TAPI driver */
/*@{*/

/** \defgroup INTERRUPT_AND_PROTECTION_MODULE Protection service
   This service is used to lock and unlock the interrupts for protecting the
   access to shared data structures.   */

/** \defgroup CODER_MODULE  Coder service
   This service is used to access the coder module functionalities like encoding
   and decoding, RTP, RTCP etc., */

/** \defgroup PCM_MODULE PCM module services
   The PCM - */

/** \defgroup SIG_MODULE Signaling module service
   This service includes the functionalities like DTMF receiver, Tone detection/
   Generation, CID receiver and sender */

/** \defgroup ALM_MODULE Analong Line interface Module service
   Contains the functionalities of ALM module */

/** \defgroup DECT_MODULE DECT module service
   This services provides access to the special coders/decoders that are used
   in conjunction with DECT packet streams. Also a special tone generator is
   provided for DECT channels. */

/** \defgroup CON_MODULE Connection Module
  This module provides functions to connect different DSP modules. It is used
  for conferencing, but also for basic dynamic connections */

/*@}*/

/* ============================= */
/* Structure for LEC data        */
/*                               */
/* ============================= */
typedef struct
{
   /* LEC operating mode */
   IFX_TAPI_WLEC_TYPE_t nOpMode;
   /* Non Linear Processing on or off */
   char bNlp;
   /* Gain for input or LEC off */
   char nGainIn;
   /* Gain for ouput or LEC off */
   char nGainOut;
   /** LEC tail length - unused only a storage needed for get function */
   char nLen;
   /** Size of the near-end window in narrowband sampling mode. */
   IFX_TAPI_WLEC_WIN_SIZE_t   nNBNEwindow;
   /** Size of the far-end window in narrowband sampling mode.
       Note: this is used only if nOpMode is set to IFX_TAPI_LEC_TYPE_NFE */
   IFX_TAPI_WLEC_WIN_SIZE_t   nNBFEwindow;
   /** Size of the near-end window in wideband sampling mode. */
   IFX_TAPI_WLEC_WIN_SIZE_t   nWBNEwindow;
}TAPI_LEC_DATA_t;

/** tone play destinations */
typedef enum
{
   TAPI_TONE_DST_DEFAULT  = 0x0,
   TAPI_TONE_DST_LOCAL    = 0x1,
   TAPI_TONE_DST_NET      = 0x2,
   TAPI_TONE_DST_NETLOCAL = 0x3
}TAPI_TONE_DST;


/** Specifies the capability of the tone generator regarding tone
    sequence support. */
typedef enum
{
   /** Plays out a frequency or silence. No tone sequence with
      cadences are supported */
   IFX_TAPI_TONE_RESSEQ_FREQ = 0x0,
   /** Plays out a full simple tone including cadences and loops */
   IFX_TAPI_TONE_RESSEQ_SIMPLE = 0x1,
}IFX_TAPI_TONE_RESSEQ_t;

/** Tone resource information. */
typedef struct
{
   /** Resource ID or number of the generator. Used as index in the tone status array and
   must be a number between 0 and TAPI_TONE_MAXRES */
   IFX_uint8_t nResID;
   /** Number of maximum supported frequencies at one time */
   IFX_uint8_t nFreq;
   /** Specifies the capability of the tone generator regarding tone
       sequence support. See \ref IFX_TAPI_TONE_RESSEQ_t for details. */
   IFX_TAPI_TONE_RESSEQ_t sequenceCap;
} IFX_TAPI_TONE_RES_t;

/* =============================== */
/* Defines for complex tone states */
/* =============================== */
typedef enum
{
   /** Initialization state */
   TAPI_CT_IDLE = 0,
   /** UTG tone sequence is not active, but the tone is in pause state */
   TAPI_CT_ACTIVE_PAUSE,
   /** Tone is currently playing out on the tone generator */
   TAPI_CT_ACTIVE,
   /* UTG tone sequence is deactived by the LL driver automatically. Afterwards a
   next step can be programmed or a new tone can be started. */
   TAPI_CT_DEACTIVATED

}TAPI_CMPLX_TONE_STATE_t;

typedef enum
{
   /** RTP protocol */
   IFX_TAPI_PRT_TYPE_RTP,
   /** AAL protocol */
   IFX_TAPI_PRT_TYPE_AAL
}IFX_TAPI_PRT_TYPE_t;

/* ============================= */
/* Structure for CID data        */
/* ============================= */

#ifndef DRV_TAPI_H
typedef struct
{
   /* sets CID type 1 (ON-) or type 2 (OFF-HOOK) */
   IFX_TAPI_CID_HOOK_MODE_t    txHookMode;
   /* Buffer for CID parameter coding  */
   IFX_uint8_t             cidParam [IFX_TAPI_CID_TX_SIZE_MAX];
   /* number of CID parameters octets */
   IFX_uint16_t            nCidParamLen;
} TAPI_CID_DATA_t;
#endif /* DRV_TAPI_H */

typedef struct
{
   /** The configured CID standard */
   IFX_TAPI_CID_STD_t          nStandard;

   /** Type of ETSI Alert of onhook services associated to ringing.
      Default IFX_TAPI_CID_ALERT_ETSI_FR */
   IFX_TAPI_CID_ALERT_ETSI_t   nETSIAlertRing;
   /** Type of ETSI Alert of onhook services not associated to ringing.
      Default IFX_TAPI_CID_ALERT_ETSI_RP. */
   IFX_TAPI_CID_ALERT_ETSI_t   nETSIAlertNoRing;
   /** Tone table index for the alert tone to be used.
      Required for automatic CID/MWI generation. Default XXXXXd. */
   unsigned int            nAlertToneOnhook;
   unsigned int            nAlertToneOffhook;
  /* time needed to play alert tone */
   unsigned int            nAlertToneTime;
   /** Ring Pulse on time interval. */
   unsigned int            ringPulseOnTime;
   /** Ring Pulse off time interval. */
   unsigned int            ringPulseOffTime;
   /** Ring Pulse Loops. */
   unsigned int            ringPulseLoop;
   /** DTMF ACK after CAS, used for offhook transmission. Default DTMF 'D'. */
   char                    ackTone;
   /** Usage of OSI for offhook transmission. Default "no use" */
   unsigned int            OSIoffhook;
   /** Lenght of the OSI signal in ms. Default 200 ms. */
   unsigned int            OSItime;
   /** Cadence Ring Burst time, used for TAPI_CID_ALERT_FR */
   unsigned int            cadenceRingBurst;
   /** Cadence Ring Pause time, used for TAPI_CID_ALERT_FR */
   unsigned int            cadenceRingPause;
   /** Reception of 2nd ack signal timeout (after data transmission) (NTT)*/
   unsigned int            ack2Timeout;


   IFX_TAPI_CID_ABS_REASON_t   TapiCidDtmfAbsCli;
   IFX_TAPI_CID_TIMING_t       TapiCidTiming;
   IFX_TAPI_CID_FSK_CFG_t      TapiCidFskConf;
   IFX_TAPI_CID_DTMF_CFG_t     TapiCidDtmfConf;
} TAPI_CID_CONF_t;

/* =============================== */
/* Defines for connection control  */
/* =============================== */

typedef enum _IFX_TAPI_CONN_ACTION
{
   IFX_TAPI_CONN_ACTION_CREATE = 0,
   IFX_TAPI_CONN_ACTION_REMOVE = 1
} IFX_TAPI_CONN_ACTION_t;

/** Clasify error when reported via event dispatcher. For internal use */
typedef enum
{
   /** Report TAPI channel specific error */
   IFX_TAPI_ERRSRC_TAPI_CH    = 0,
   /** Report TAPI device or global error */
   IFX_TAPI_ERRSRC_TAPI_DEV   = 0x1000,
   /** Report low level global error */
   IFX_TAPI_ERRSRC_LL_DEV     = 0x2000,
   /** Report low level channel specific error */
   IFX_TAPI_ERRSRC_LL_CH      = 0x4000,
   /** Bit mask for modification of low level driver error codes. This bit
       is set for low level driver error codes */
   IFX_TAPI_ERRSRC_LL         = 0x8000,
   /** Maks of error sources used for clearing */
   IFX_TAPI_ERRSRC_MASK       = (IFX_TAPI_ERRSRC_LL |
                                 IFX_TAPI_ERRSRC_LL_CH |
                                 IFX_TAPI_ERRSRC_LL_DEV |
                                 IFX_TAPI_ERRSRC_TAPI_DEV |
                                 IFX_TAPI_ERRSRC_TAPI_CH)
}IFX_TAPI_ERRSRC;


/** Interrupt and Protection Module */
/** \addtogroup INTERRUPT_AND_PROTECTION_MODULE */
/** Used for data protection by higher layer */
/*@{*/
typedef struct
{
   /** This function disables the irq line if the driver is in interrupt mode
   \param
   pLLDev   - Low-level device pointer */
   IFX_void_t (*LockDevice)                 (IFX_TAPI_LL_DEV_t *);
   /** This function enables the irq line if the driver is in interrupt mode
   \param
   pLLDev   - Low-level device pointer */
   IFX_void_t (*UnlockDevice)               (IFX_TAPI_LL_DEV_t *);
   IFX_void_t (*IrqEnable)                  (IFX_TAPI_LL_DEV_t *);
   IFX_void_t (*IrqDisable)                 (IFX_TAPI_LL_DEV_t *);
} IFX_TAPI_DRV_CTX_IRQ_t;

/*@}*/ /* INTERRUPT_AND_PROTECTION_MODULE */


/** CODer Module */  /* ***************************************************** */
/** \addtogroup CODER_MODULE */
/** Used for Coder services higher layer */
/*@{*/
typedef struct
{
   /** Starts the coder in Upstream / record data
   \param pLLCh       Pointer to low-level channel structure
   \return
   IFX_SUCCESS if successful
   IFX_ERROR if an error occured
   */
   IFX_return_t (*ENC_Start)                (IFX_TAPI_LL_CH_t *pLLCh);

   /** Stops the coder in Upstream / record data
   \param pLLCh       Pointer to low-level channel structure
   \return
   IFX_SUCCESS if successful
   IFX_ERROR if an error occured
   */
   IFX_return_t (*ENC_Stop)                 (IFX_TAPI_LL_CH_t *pLLCh);

   /**
      Put encoder into hold or unhold state.

      \param pLLCh       Pointer to Low-level channel structure
      \param nOnHold     Hold state (IFX_ENABLE - hold, IFX_DISABLE - unhold)

      \return  IFX_SUCCESS if successful
               IFX_ERROR if an error occured

      \remark
   */
   IFX_return_t (*ENC_Hold)                 (IFX_TAPI_LL_CH_t *pLLChannel,
                                             IFX_operation_t nOnHold);

   /** Sets the Recording codec and frame length
   \param pLLCh      Pointer to low-level channel structure
   \param nCoder     Selected coder type
   \param nFrameLength    Length of frames to be generated by the coder
   \param nBitPack   normal or AAL bit alignment (not supported by all devices)
   \return
   IFX_SUCCESS if successful
   IFX_ERROR if an error occured
   */
   IFX_return_t (*ENC_Cfg)                  (IFX_TAPI_LL_CH_t *pLLChannel,
                                             IFX_TAPI_COD_TYPE_t nCoder,
                                             IFX_TAPI_COD_LENGTH_t nFrameLength,
                                             IFX_TAPI_COD_AAL2_BITPACK_t nBitPack);

   /** Sets Decoder specific parameters
   \param pLLCh      Pointer to low-level channel structure
   \param nBitPack   normal or AAL bit alignment (not supported by all devices)
   \return
   IFX_SUCCESS if successful
   IFX_ERROR if an error occured
   */
   IFX_return_t (*DEC_Cfg)                  (IFX_TAPI_LL_CH_t *pLLChannel,
                                             IFX_TAPI_COD_AAL2_BITPACK_t nBitPack);



   /** Sets the Recording codec                          - replaced by ENC_Cfg
   \param pLLCh      Pointer to low-level channel structure
   \param nCodec  Codec for the record audio channel
   \return
   IFX_SUCCESS if successful
   IFX_ERROR if an error occured
   */
   IFX_return_t (*ENC_CoderType_Set)        (IFX_TAPI_LL_CH_t *pLLCh,
                                             IFX_int32_t nCodec);

   /** Sets the frame length for the voice packets       - replaced by ENC_Cfg
   \param pLLCh       Pointer to Low-level channel structure
   \param nFrameLength    Length of frames in milliseconds
   \return
   IFX_SUCCESS if successful
   IFX_ERROR if an error occured
   */
   IFX_return_t (*ENC_FrameLength_Set)      (IFX_TAPI_LL_CH_t *pLLCh,
                                             IFX_int32_t nFrameLength);

   /** Retrieves the saved frame length for the audio packets, as saved when applied
        to the channel
   \param pLLCh           Pointer to Low-level channel structure
   \param pFrameLength    Pointer to length of frames in milliseconds

   \return
   IFX_SUCCESS if successful
   IFX_ERROR if an error occured    */
   IFX_return_t (*ENC_FrameLength_Get)      (IFX_TAPI_LL_CH_t *pLLCh,
                                             IFX_int32_t *nFrameLength);

   /** Turns the room noise detection mode on or off
   \param pLLCh           Pointer to Low-level channel structure
   \param bEnable         IFX_TRUE to enable or IFX_FALSE to disable
   \param nThreshold      detection level in minus dB
   \param nVoicePktCnt    count of consecutive voice packets required for event
   \param nSilencePktCnt  count of consecutive silence packets required for event */
   IFX_return_t (*ENC_RoomNoise)            (IFX_TAPI_LL_CH_t *pLLCh,
                                             IFX_boolean_t bEnable,
                                             IFX_uint32_t nThreshold,
                                             IFX_uint8_t nVoicePktCnt,
                                             IFX_uint8_t nSilencePktCnt);

   /** Starts the playing.
   \param pLLCh           Pointer to Low-level channel structure
   \return
   IFX_SUCCESS if successful
   IFX_ERROR if an error occured */
   IFX_return_t (*DEC_Start)                (IFX_TAPI_LL_CH_t *pLLCh);

   /** Stops the playing.
   \param pLLCh           Pointer to Low-level channel structure
   \return
   IFX_SUCCESS if successful
   IFX_ERROR if an error occured */
   IFX_return_t (*DEC_Stop)                 (IFX_TAPI_LL_CH_t *pLLCh);

   /** Sets the Voice Activity Detection mode
   \param pLLCh           Pointer to Low-level channel structure
   \param nVAD         Switch On or Off
   \return
   IFX_SUCCESS if successful
   IFX_ERROR if an error occured */
   IFX_return_t (*VAD_Cfg)                  (IFX_TAPI_LL_CH_t *pLLCh,
                                             IFX_int32_t nVAD);

   /**
      Enable/Disable the AGC resource inside the device

      \param pLLCh       Pointer to Low-level channel structure
      \param agcMode     AGC mode to be configured (enable/disable)

      \return  IFX_SUCCESS if successful
               IFX_ERROR if an error occured

      \remark
   */
   IFX_return_t (*AGC_Enable)               (IFX_TAPI_LL_CH_t *pLLCh,
                                             IFX_TAPI_ENC_AGC_MODE_t agcMode);

   /**
      Enable/Disable the AGC resource inside the device

      \param pLLCh           Pointer to Low-level channel structure
      \param pAGC_Cfg        New AGC parameters
      \return  IFX_SUCCESS if successful
               IFX_ERROR if an error occured


      \remark
   */
   IFX_return_t (*AGC_Cfg)                  (IFX_TAPI_LL_CH_t *pLLCh,
                                             IFX_TAPI_ENC_AGC_CFG_t *pAGC_Cfg);


   /** Configures the jitter buffer
   \param pLLCh           Pointer to Low-level channel structure
   \param pJbConf      Pointer to the Jitter buffer configuration
   \return
   IFX_SUCCESS if successful
   IFX_ERROR if an error occured */
   IFX_return_t (*JB_Cfg)                   (IFX_TAPI_LL_CH_t *pLLCh,
                                             IFX_TAPI_JB_CFG_t const *pJbConf);

   /** Query the Jitter buffer statistics
   \param pLLCh           Pointer to Low-level channel structure
   \param pJbData      Pointer to the Jitter buffer data
   \return
   IFX_SUCCESS if successful
   IFX_ERROR if an error occured */
   IFX_return_t (*JB_Stat_Get)              (IFX_TAPI_LL_CH_t *pLLCh,
                                             IFX_TAPI_JB_STATISTICS_t *pJbData);

   /** Reset the Jitter buffer statistics
   \param pLLCh           Pointer to Low-level channel structure
   \return
   IFX_SUCCESS if successful
   IFX_ERROR if an error occured */
   IFX_return_t (*JB_Stat_Reset)            (IFX_TAPI_LL_CH_t *pLLCh);

   /** Configure RTP and RTCP for a connection
   \param pLLCh           Pointer to Low-level channel structure
   \param pRtpConf        Pointer to IFX_TAPI_PKT_RTP_CFG_t, RTP configuraton
   \return
   IFX_SUCCESS if successful
   IFX_ERROR if an error occured */
   IFX_return_t (*RTP_Cfg)                  (IFX_TAPI_LL_CH_t *pLLCh, IFX_TAPI_PKT_RTP_CFG_t const *pRtpConf);

   /** Configure a new payload type
   \param pLLCh           Pointer to Low-level channel structure
   \param pRtpPTConf      Pointer to RTP payload configuraton
   \return
   IFX_SUCCESS if successful
   IFX_ERROR if an error occured */
   IFX_return_t (*RTP_PayloadTable_Cfg)     (IFX_TAPI_LL_CH_t *pLLCh, IFX_TAPI_PKT_RTP_PT_CFG_t const *pRtpPTConf);

   /** Start or stop generation of RTP event packets
   \param pLLCh           Pointer to Low-level channel structure
   \param nEvent          Event code as defined in RFC2833
   \param nStart          Start (true) or stop (false)
   \param nDuration       Duration of event in units of 10 ms (0 = forever)
   \return
   IFX_SUCCESS if successful
   IFX_ERROR if an error occured */
   IFX_return_t (*RTP_EV_Generate)          (IFX_TAPI_LL_CH_t *pLLCh, IFX_uint8_t nEvent, IFX_boolean_t bStart, IFX_uint8_t nDuration);

   /** Gets the RTCP statistic information for the addressed channel
   \param pLLCh           Pointer to Low-level channel structure
   \param pRTCP           Pointer to RTCP Statistics structure
   \return
   IFX_SUCCESS if successful
   IFX_ERROR if an error occured */
   IFX_return_t (*RTCP_Get)                 (IFX_TAPI_LL_CH_t *pLLCh, IFX_TAPI_PKT_RTCP_STATISTICS_t *pRTCP);

   /**  Resets  RTCP statistics
   \param pLLCh           Pointer to Low-level channel structure
   \return
   IFX_SUCCESS if successful
   IFX_ERROR if an error occured */
   IFX_return_t (*RTCP_Reset)               (IFX_TAPI_LL_CH_t *pLLCh);

   /* CURRENTLY NOT SUPPORTED
      the following pair of function pointers can be used to retrieve ("gather") the RTCP statistics non blocking,
      therefore RTCP_Prepare must be called within a noninterruptible context
    */
   IFX_return_t (*RTCP_Prepare_Unprot)      (IFX_TAPI_LL_CH_t *pLLCh);
   IFX_return_t (*RTCP_Prepared_Get)        (IFX_TAPI_LL_CH_t *pLLCh, IFX_TAPI_PKT_RTCP_STATISTICS_t *pRTCP);

   IFX_return_t (*AAL_Cfg)                  (IFX_TAPI_LL_CH_t *pLLCh, IFX_TAPI_PCK_AAL_CFG_t const *pAalConf);
   IFX_return_t (*AAL_Profile_Cfg)          (IFX_TAPI_LL_CH_t *pLLCh, IFX_TAPI_PCK_AAL_PROFILE_t const *pProfile);

   /*  Configures the Datapump for Modulation
   \param pLLCh           Pointer to Low-level channel structure
   \param pFaxMod         Pointer
   \return
   IFX_SUCCESS if successful
   IFX_ERROR if an error occured */
   IFX_return_t (*T38_Mod_Enable)           (IFX_TAPI_LL_CH_t *pLLCh, IFX_TAPI_T38_MOD_DATA_t const *pFaxMod);

   /*  Configures the Datapump for Demodulation
   \param pLLCh           Pointer to Low-level channel structure
   \param pFaxMod         Pointer
   \return
   IFX_SUCCESS if successful
   IFX_ERROR if an error occured */
   IFX_return_t (*T38_DeMod_Enable)         (IFX_TAPI_LL_CH_t *pLLCh, IFX_TAPI_T38_DEMOD_DATA_t const *pFaxDemod);

   /** Disables the Fax datapump
   \param pLLCh           Pointer to Low-level channel structure
   \return
   IFX_SUCCESS if successful
   IFX_ERROR if an error occured */
   IFX_return_t (*T38_Datapump_Disable)     (IFX_TAPI_LL_CH_t *pLLCh);

   /** Query the Fax Status
   \param pLLCh           Pointer to Low-level channel structure
   \param pFaxStatus      Pointer to T38 status structure where the status will
   be copied
   \return
   IFX_SUCCESS if successful
   IFX_ERROR if an error occured */
   IFX_return_t (*T38_Status_Get)           (IFX_TAPI_LL_CH_t *pLLCh, IFX_TAPI_T38_STATUS_t *pFaxStatus);

   /** Set the Fax Status
   \param pLLCh           Pointer to Low-level channel structure
   \param status          The status which to be set
   \return
   IFX_SUCCESS if successful
   IFX_ERROR if an error occured */
   IFX_return_t (*T38_Status_Set)           (IFX_TAPI_LL_CH_t *pLLCh, unsigned char status);

   /** Set the Fax Error Status
   \param pLLCh           Pointer to Low-level channel structure
   \param error           The error status which to be set
   \return
   IFX_SUCCESS if successful
   IFX_ERROR if an error occured */
   IFX_return_t (*T38_Error_Set)            (IFX_TAPI_LL_CH_t *pLLCh, unsigned char error);

   /** Decoder Change event reporting enable / disable
   \param pLLCh           pointer to low-level channel structure
   \param bEn             enable or disable reporting
   \return
      IFX_SUCCESS if successful
      IFX_ERROR if an error occured */
   IFX_TAPI_LL_ERR_t (*DEC_Chg_Evt_Enable)  (IFX_TAPI_LL_CH_t *pLLCh, IFX_boolean_t bEn);

   /** Decoder Change event reporting request new decoder details
   \param pLLCh           pointer to low-level channel structure
   \param pDecDetails     pointer to a decoder details structure
   \return
      IFX_SUCCESS if successful
      IFX_ERROR if an error occured */
   IFX_TAPI_LL_ERR_t (*DEC_Chg_Evt_Detail_Req) (IFX_TAPI_LL_CH_t *pLLCh, IFX_TAPI_DEC_DETAILS_t *pDec);

   /** Switches on/off HP filter of decoder path
   \param pLLCh           Pointer to Low-level channel structure
   \param bHp             IFX_FALSE to switch HP off, IFX_TRUE to switch HP on
   \return
   IFX_SUCCESS if successful
   IFX_ERROR if an error occured */
   IFX_return_t (*DEC_HP_Set)                   (IFX_TAPI_LL_CH_t *pLLCh, IFX_boolean_t bHp);

   /** Sets the COD interface volume
   \param pLLCh           Pointer to Low-level channel structure
   \param pVol            Pointer to the IFX_TAPI_PKT_VOLUME_t structure
   \return
   IFX_SUCCESS if successful
   IFX_ERROR if an error occured */
   IFX_return_t (*Volume_Set)               (IFX_TAPI_LL_CH_t *pLLCh,
                                             IFX_TAPI_PKT_VOLUME_t const *pVol);

} IFX_TAPI_DRV_CTX_COD_t;

/*@}*/ /* CODER_MODULE*/

/** PCM Module */  /* ***************************************************** */
/** \addtogroup PCM_MODULE */
/** Used for PCM services higher layer */
/*@{*/
typedef struct
{
   /** Configure and enable the PCM interface
   \param pLLCh           Pointer to Low-level device structure
   \param pCfg            Pointer to the configuration structure
   */
   IFX_return_t (*ifCfg)                    (IFX_TAPI_LL_DEV_t *pLLDev, const IFX_TAPI_PCM_IF_CFG_t *pCfg);

   /** Prepare parameters and call the target function to activate PCM module
   \param pLLCh           Pointer to Low-level channel structure
   \param nMode           Activation mode
   \param pPcmCfg         Pointer to the PCM configuration structure
   \return
   IFX_SUCCESS if successful
   IFX_ERROR if an error occured */
   IFX_return_t (*Enable)                   (IFX_TAPI_LL_CH_t *pLLCh, IFX_uint32_t nMode, IFX_TAPI_PCM_CFG_t *);

   /** Prepare parameters and call the target function to Configure the PCM module
   \param pLLCh           Pointer to Low-level channel structure
   \param pPcmCfg         Pointer to the PCM configuration structure
   \return
   IFX_SUCCESS if successful
   IFX_ERROR if an error occured */
   IFX_return_t (*Cfg)                      (IFX_TAPI_LL_CH_t *pLLCh, IFX_TAPI_PCM_CFG_t const *pPCMConfig);

   /** Sets the LEC configuration on the PCM
   \param pLLCh           Pointer to Low-level channel structure
   \param pLecConf        Pointer to the LEC configuration structure
   \return
   IFX_SUCCESS if successful
   IFX_ERROR if an error occured */
   IFX_return_t (*Lec_Cfg)                  (IFX_TAPI_LL_CH_t *pLLCh, TAPI_LEC_DATA_t const *pLecConf);

   /** Sets the PCM interface volume
   \param pLLCh           Pointer to Low-level channel structure
   \param pVol            Pointer to the IFX_TAPI_LINE_VOLUME_t structure
   \return
   IFX_SUCCESS if successful
   IFX_ERROR if an error occured */
   IFX_return_t (*Volume_Set)               (IFX_TAPI_LL_CH_t *pLLCh, IFX_TAPI_LINE_VOLUME_t const *pVol);

   /** Switches on/off HP filter of decoder path
   \param pLLCh           Pointer to Low-level channel structure
   \param bHp             IFX_FALSE to switch HP off, IFX_TRUE to switch HP on
   \return
   IFX_SUCCESS if successful
   IFX_ERROR if an error occured */
   IFX_return_t (*DEC_HP_Set)                (IFX_TAPI_LL_CH_t *pLLCh, IFX_boolean_t bHp);

} IFX_TAPI_DRV_CTX_PCM_t;
/*@}*/ /* PCM_MODULE*/


/** SIG Module */ /* ********************************************************/
/** \addtogroup SIG_MODULE */
/** Signalling module services*/
/*@{*/
typedef struct
{
   /** Do low level UTG configuration and activation
   \param pLLCh         Pointer to Low-level channel structure
   \param pSimpleTone   Internal simple tone table entry
   \param dst           Destination
   \param nResID          Low level resource ID queried before by
                          ToneGen_ResIdGet. Otherwise TAPI defaults
   \return
   IFX_SUCCESS if successful
   IFX_ERROR if an error occured */
   IFX_int32_t  (*UTG_Start)                (IFX_TAPI_LL_CH_t *pLLCh,
                                             IFX_TAPI_TONE_SIMPLE_t const *pSimpleTone,
                                             TAPI_TONE_DST dst,
                                             IFX_uint8_t nResID);

   /** Stop playing the tone with the given tone definition
   \param pLLCh         Pointer to Low-level channel structure
   \param nResID          Low level resource ID queried before by
                          ToneGen_ResIdGet. Otherwise TAPI defaults
   \return
   IFX_SUCCESS if successful
   IFX_ERROR if an error occured */
   IFX_int32_t  (*UTG_Stop)                 (IFX_TAPI_LL_CH_t *pLLCh,
                                             IFX_uint8_t nResID);

   IFX_return_t (*UTG_Level_Set)            (IFX_TAPI_LL_CH_t *pLLCh, IFX_TAPI_PREDEF_TONE_LEVEL_t const *pToneLevel);

   /** Returns the total number of UTGs per channel
   \param pChannel      Handle to TAPI_CHANNEL structure
   \return Returns the total number of UTGs per channel */
   IFX_uint8_t  (*UTG_Count_Get)            (IFX_TAPI_LL_CH_t *pLLCh);


   IFX_void_t   (*UTG_Event_Deactivated)    (IFX_TAPI_LL_CH_t *pLLCh,
                                             IFX_uint8_t utgNum );

   /** Do low level UTG configuration and activation
   \param pLLCh           Pointer to Low-level channel structure
   Allocate a tone generation resource on a specific module "modType".

   \param pLLChannel  Handle to TAPI low level channel structure
   \param nResID The resouce identifier returned
   \param genType Describes the module type where to play the tone.
   \param genDir Tone generation direction on the module.

   \return
      IFX_SUCCESS if successful
      IFX_ERROR if an error occured
   \remarks
   nResID is returned and stored inside LL driver if necessary. It is used
   during tone event report to TAPI HL. TAPI HL uses nResID as an index for
   event dispatch functionality.
   This index is used for IFX_TAPI_LL_ToneStart, IFX_TAPI_LL_ToneStop
   and IFX_TAPI_LL_ToneGenResRelease.
   This API only supports to allocate a resource with genDir set to
   IFX_TAPI_LL_TONE_EXTERNAL or IFX_TAPI_LL_TONE_INTERNAL. It does not support
   to allocate a resource for both directions IFX_TAPI_LL_TONE_BOTH.
   TAPI LL disables all tone generator events in case the return value is
   not success.
   */
   IFX_int32_t (*ToneGen_ResIdGet)          (IFX_TAPI_LL_CH_t * pLLChannel,
                                             IFX_TAPI_MODULE_TYPE_t genType,
                                             IFX_TAPI_LL_TONE_DIR_t genDir,
                                             IFX_TAPI_TONE_RES_t* pRes);

   /** Configure the DTMF tone generator
   \param nInterDigitTime Inter-digit-time in ms
   \param nDigitPlayTime  Active digit-play-time in ms
   \return
   IFX_SUCCESS if successful
   IFX_ERROR if an error occured */
   IFX_int32_t  (*DTMFG_Cfg  )              (IFX_TAPI_LL_CH_t *pLLCh,
                                             IFX_uint16_t nInterDigitTime,
                                             IFX_uint16_t nDigitPlayTime);

   /** Start the DTMF tone generator
   \param pLLCh           Pointer to Low-level channel structure
   \param nDigits         Number of digits in the data string to be sent
   \param *data           String with the digits (ascii 0-9 A-D) to be sent
   \return
   IFX_SUCCESS if successful
   IFX_ERROR if an error occured */
   IFX_int32_t  (*DTMFG_Start)              (IFX_TAPI_LL_CH_t *pLLCh,
                                             IFX_uint8_t nDigits,
                                             IFX_char_t  *data);

   /** Stop the DTMF tone generator
   \param pLLCh           Pointer to Low-level channel structure
   \return
   IFX_SUCCESS if successful
   IFX_ERROR if an error occured */
   IFX_int32_t  (*DTMFG_Stop)               (IFX_TAPI_LL_CH_t *pLLCh);

   /** Start the DTMF tone detector
   \param pLLCh           Pointer to Low-level channel structure */
   IFX_void_t   (*DTMFD_Start)              (IFX_TAPI_LL_CH_t *pLLCh);

   /** Controls the DTMF sending mode.
   \param pLLCh           Pointer to Low-level channel structure
   \param nOobMode        Mode of DTMFD (Inband or Out of band transmission
                          of RFC2833 event packets)
   \return
   IFX_SUCCESS if successful
   IFX_ERROR if an error occured */
   IFX_int32_t (*DTMFD_OOB)                (IFX_TAPI_LL_CH_t *pLLCh, IFX_TAPI_PKT_EV_OOB_t nOobMode);

   /** Sets/Gets DTMF receiver coefficients.
   \param pLLCh           Pointer to Low-level channel structure
   \param bRW             IFX_FALSE to write, IFX_TRUE to read settings
   \param pDtmfRxCoeff    Pointer to DTMF Rx coefficients settings

   \return
   IFX_SUCCESS if successful
   IFX_ERROR if an error occured */
   IFX_return_t (*DTMF_RxCoeff)             (IFX_TAPI_LL_CH_t *pLLCh, IFX_boolean_t bRW, IFX_TAPI_DTMF_RX_CFG_t *pDtmfRxCoeff);

   /** Starts Call Progress Tone Detection
   \param pLLCh           Pointer to Low-level channel structure
   \param pTone           Pointer to simple tone structure
   \param signal          The type of signal and direction to detect
   \return
   IFX_SUCCESS if successful
   IFX_ERROR if an error occured */
   IFX_return_t (*CPTD_Start)               (IFX_TAPI_LL_CH_t *pLLCh, IFX_TAPI_TONE_SIMPLE_t const *pTone, IFX_int32_t signal);

   /** Stops the Call Progress Tone Detection
   \param pLLCh           Pointer to Low-level channel structure
   \return
   IFX_SUCCESS if successful
   IFX_ERROR if an error occured */
   IFX_return_t (*CPTD_Stop)                (IFX_TAPI_LL_CH_t *pLLCh);

   /** Enables signal detection
   \param pLLCh           Pointer to Low-level channel structure
   \param pSig            Pointer to IFX_TAPI_SIG_DETECTION_t structure
   \return
   IFX_SUCCESS if successful
   IFX_ERROR if an error occured */
   IFX_return_t (*MFTD_Enable)              (IFX_TAPI_LL_CH_t *pLLCh, IFX_TAPI_SIG_DETECTION_t const *pSig);

   /** Disables signal detection
   \param pLLCh           Pointer to Low-level channel structure
   \param pSig            Pointer to IFX_TAPI_SIG_DETECTION_t structure
   \return
   IFX_SUCCESS if successful
   IFX_ERROR if an error occured */
   IFX_return_t (*MFTD_Disable)             (IFX_TAPI_LL_CH_t *pLLCh, IFX_TAPI_SIG_DETECTION_t const *pSig);

   IFX_return_t (*MFTD_Signal_Set)          (IFX_TAPI_LL_CH_t *pLLCh, IFX_uint32_t signal);
   IFX_return_t (*MFTD_Signal_Get)          (IFX_TAPI_LL_CH_t *pLLCh, IFX_uint32_t *signal);
   IFX_return_t (*MFTD_Signal_Ext_Set)      (IFX_TAPI_LL_CH_t *pLLCh, IFX_uint32_t Extsignal);
   IFX_return_t (*MFTD_Signal_Ext_Get)      (IFX_TAPI_LL_CH_t *pLLCh, IFX_uint32_t *Extsignal);
   IFX_return_t (*MFTD_Signal_Enable)       (IFX_TAPI_LL_CH_t *pLLCh, IFX_TAPI_SIG_t signal);
   IFX_return_t (*MFTD_Signal_Disable)      (IFX_TAPI_LL_CH_t *pLLCh, IFX_TAPI_SIG_t signal);

   /** Start CID data transmission
   \param pLLCh           Pointer to Low-level channel structure
   \param pCid            Contains the CID data
   \param pCidConf        Pointer to CID configuration data
   \param pDtmfConf       Pointer to the DTMF configuration data
   \param pcidFskConf     Pointer to the CID FSK configuration data
   \return
   Device specific return code.
   In case of success the return code must correspond to IFX_SUCCESS */
   IFX_int32_t (*CID_TX_Start)         (IFX_TAPI_LL_CH_t *pLLCh,
                                        TAPI_CID_DATA_t const *pCid,
                                        TAPI_CID_CONF_t
                                        *pCidConf,
                                        IFX_TAPI_CID_DTMF_CFG_t *pDtmfConf,
                                        IFX_TAPI_CID_FSK_CFG_t *pcidFskConf);

   /** Stop CID data transmission
   \param pLLChannel      Handle to TAPI low level channel structure
   \param pCidConf        Pointer to the global CID configuration
   \return
   Device specific return code.
   In case of success the return code must correspond to IFX_SUCCESS */
   IFX_int32_t (*CID_TX_Stop)              (IFX_TAPI_LL_CH_t *pLLChannel,
                                             TAPI_CID_CONF_t *pCidConf);

   /** Start the CID receiver
   \param pLLCh           Pointer to Low-level channel structure
   \param pCidFskCid      Pointer to Cid Fsk Configuration structure
   \return
   IFX_SUCCESS if successful
   IFX_ERROR if an error occured */
   IFX_return_t (*CID_RX_Start)             (IFX_TAPI_LL_CH_t *pLLCh,
                                             IFX_TAPI_CID_HOOK_MODE_t cidHookMode,
                                             IFX_TAPI_CID_FSK_CFG_t *pCidFskCfg);

   /** Stop the CID receiver
   \param pLLCh           Pointer to Low-level channel structure
   \param pCidFskCid      Pointer to Cid Fsk Configuration structure
   \return
   IFX_SUCCESS if successful
   IFX_ERROR if an error occured */
   IFX_return_t (*CID_RX_Stop)              (IFX_TAPI_LL_CH_t *pLLCh,
                                             IFX_TAPI_CID_FSK_CFG_t *pCidFskCfg);
} IFX_TAPI_DRV_CTX_SIG_t;


/** AUDIO Module */ /* ******************************************************** */
typedef struct
{
   IFX_return_t  (*Volume_Set)            (IFX_TAPI_LL_CH_t *pLLCh, IFX_uint32_t level);
   IFX_return_t  (*Room_Set)              (IFX_TAPI_LL_CH_t *pLLCh, IFX_uint32_t type);
   IFX_return_t  (*Mode_Set)              (IFX_TAPI_LL_CH_t *pLLCh, IFX_uint32_t mode);
   IFX_return_t  (*Mute_Set)              (IFX_TAPI_LL_CH_t *pLLCh, IFX_uint32_t action);
   IFX_return_t  (*Ring_Start)            (IFX_TAPI_LL_CH_t *pLLCh, IFX_uint32_t index);
   IFX_return_t  (*Ring_Stop)             (IFX_TAPI_LL_CH_t *pLLCh, IFX_uint32_t index);
   IFX_return_t  (*Ring_Volume_Set)       (IFX_TAPI_LL_CH_t *pLLCh, IFX_uint32_t level);
   IFX_return_t  (*Incall_Anouncement)    (IFX_TAPI_LL_CH_t *pLLCh, IFX_uint32_t action);
   IFX_return_t  (*AFE_Cfg_Set)           (IFX_TAPI_LL_CH_t *pLLCh, IFX_TAPI_AUDIO_AFE_CFG_SET_t *pAFECfg);
   IFX_return_t  (*Test_Mode_Set)         (IFX_TAPI_LL_CH_t *pLLCh, IFX_uint32_t action);
} IFX_TAPI_DRV_CTX_AUDIO_t;


/** ALM Module */ /* ********************************************************/
/** \addtogroup ALM_MODULE */
/** Analog line module services*/
/*@{*/
typedef struct
{
   /** Set line type and sampling operation mode of the analog line.
   \param pLLCh         Pointer to Low-level channel structure
   \param nType         Line type and sampling mode to be set
   \return
   IFX_SUCCESS if successful
   IFX_ERROR if an error occured */
   IFX_int32_t  (*Line_Type_Set)            (IFX_TAPI_LL_CH_t *pLLCh,
                                             IFX_TAPI_LINE_TYPE_t nType);

   /** Set the line mode of the analog line
   \param pLLCh         Pointer to Low-level channel structure
   \param nMode
   \param nTapiLineMode
   \return
   IFX_SUCCESS if successful
   IFX_ERROR if an error occured */
   IFX_int32_t   (*Line_Mode_Set)           (IFX_TAPI_LL_CH_t *pLLCh,
                                             IFX_int32_t nMode,
                                             IFX_uint8_t nTapiLineMode);

   /** Switch Line Polarity
   \param pLLCh           Pointer to Low-level channel structure
   \return
   IFX_SUCCESS if successful
   IFX_ERROR if an error occured */
   IFX_int32_t   (*Line_Polarity_Set)       (IFX_TAPI_LL_CH_t *pLLCh);


   IFX_int32_t  (*Volume_Set)              (IFX_TAPI_LL_CH_t *pLLCh,
                                             IFX_TAPI_LINE_VOLUME_t const *pVol);

   /** This service enables or disables a high level path of a phone channel.
   \param pLLCh           Pointer to Low-level channel structure
   \param bEnable         Enable or disable
   \return
   IFX_SUCCESS if successful
   IFX_ERROR if an error occured */
   IFX_int32_t   (*Volume_High_Level)       (IFX_TAPI_LL_CH_t *pLLCh,
                                             IFX_int32_t bEnable);

   /** Ring configuration
   \param pLLCh           Pointer to Low-level channel structure
   \param pRingConfig     Pointer to ring config structure
   \return
   IFX_SUCCESS if successful
   IFX_ERROR if an error occured */
   IFX_int32_t  (*Ring_Cfg)                (IFX_TAPI_LL_CH_t *pLLCh,
                                             IFX_TAPI_RING_CFG_t const *pRingConfig);

   /** Enable/Disalbe Auto battery switch
   \param pLLCh           Pointer to Low-level channel structure
   \return
   IFX_SUCCESS if successful
   IFX_ERROR if an error occured */
   IFX_int32_t   (*AutoBatterySwitch)       (IFX_TAPI_LL_CH_t *pLLCh);

   /** Configure metering mode of chip
   \param pLLCh           Pointer to Low-level channel structure
   \param nMode       - use TTX (0) or reverse polarity (1)
   \param nFreq       - Default/12 KHz/16 KHz
   \return
   IFX_SUCCESS if successful
   IFX_ERROR if an error occured */
   IFX_int32_t   (*Metering_Cfg)            (IFX_TAPI_LL_CH_t *pLLCh,
                                             IFX_uint8_t,
                                             IFX_uint8_t);

   /** Restores the line state back after fault
   \param pLLCh           Pointer to Low-level channel structure
   \return
   IFX_SUCCESS if successful
   IFX_ERROR if an error occured */
   IFX_int32_t   (*FaultLine_Restore)       (IFX_TAPI_LL_CH_t *pLLCh);

   /** Sets the LEC configuration on the ALM
   \param pLLCh           Pointer to Low-level channel structure
   \param pLecConf        Pointer to LEC configuration structure
   \return
   IFX_SUCCESS if successful
   IFX_ERROR if an error occured */
   IFX_int32_t  (*Lec_Cfg)                  (IFX_TAPI_LL_CH_t *pLLCh,
                                             TAPI_LEC_DATA_t const *pLecConf);

   /** Configure the echo suppressor
   \param pLLCh      Pointer to low-level channel structure
   \param nEnable    Enable or disable the Echo suppressor
   \return
   IFX_SUCCESS if successful
   IFX_ERROR if an error occured */
   IFX_int32_t  (*EchoSuppressor)           (IFX_TAPI_LL_CH_t *pLLCh,
                                             IFX_uint8_t nEnable);

   /** Starts playing out a tone on the ALM tone generator.
   \param pLLCh         Pointer to Low-level channel structure
   \param res           Resource number used for playing the tone.
   \param pToneSimple   Pointer to the tone definition to play
   \param dst           Destination where to play the tone
   \return
   IFX_SUCCESS if successful
   IFX_ERROR if an error occured */
   IFX_int32_t  (*TG_Play)         (IFX_TAPI_LL_CH_t *pLLCh,
                                    IFX_uint8_t res,
                                    IFX_TAPI_TONE_SIMPLE_t const *pToneSimple,
                                    TAPI_TONE_DST dst);

   /** Stop playing the tone with the given tone definition
   \param pLLCh      Pointer to Low-level channel structure
   \param res        Resource number used for playing the tone
   \return
   IFX_SUCCESS if successful
   IFX_ERROR if an error occured */
   IFX_int32_t  (*TG_Stop)                  (IFX_TAPI_LL_CH_t *pLLCh,
                                             IFX_uint8_t res);

   /** Starts playing out the next tone of the simple tone definition.
   \param pLLCh         Pointer to Low-level channel structure
   \param pTone         Pointer to the current simple tone definition
   \param res           Resource number used for playing the tone
   \param nToneStep     Identifies the next tone step of the simple tone
   \return
   IFX_SUCCESS if successful
   IFX_ERROR if an error occured */
   IFX_int32_t  (*TG_ToneStep)              (IFX_TAPI_LL_CH_t *pLLCh,
                                             IFX_TAPI_TONE_SIMPLE_t const *pTone,
                                             IFX_uint8_t res, IFX_uint8_t *nToneStep);

   /** Simulate Hook generation (for debug use only)
   */
   IFX_int32_t  (*TestHookGen)              (IFX_TAPI_LL_CH_t *pLLCh,
                                             IFX_boolean_t arg);

   /** ALM 8kHz test loop switch (for debug use only)
   */
   IFX_int32_t  (*TestLoop)                 (IFX_TAPI_LL_CH_t *pLLCh,
                                             IFX_TAPI_TEST_LOOP_t* pLoop);
} IFX_TAPI_DRV_CTX_ALM_t;
/*@}*/

/** DECT Module */  /* **************************************************** */
/** \addtogroup DECT_MODULE */
/** Used for DECT services higher layer */
/*@{*/
typedef struct
{
   /** Sets the encoder and decoder start delay
   \param pLLCh      Pointer to low-level channel structure
   \param nEncDelay  Delay from the start of the decoder to the start of
                     the encoder in steps of 2.5ms. Range 0ms - 10ms.
   \param nDecDelay  Delay from the arrival of the first packet to the start
                     of the decoder in steps of 2.5ms. Range 0ms - 10ms.
   \return
   IFX_SUCCESS if successful
   IFX_ERROR if an error occured */
   IFX_return_t (*Ch_Cfg)                   (IFX_TAPI_LL_CH_t *pLLCh,
                                             IFX_uint8_t nEncDelay,
                                             IFX_uint8_t nDecDelay);

   /** Sets the coder type and frame length for the DECT encoding path
   \param pLLCh      Pointer to low-level channel structure
   \param nCoder     Selected coder type
   \param nFrameLength  length of packets to be generated by the coder
   \return
   IFX_SUCCESS if successful
   IFX_ERROR if an error occured
   */
   IFX_return_t (*ENC_Cfg)                  (IFX_TAPI_LL_CH_t *pLLCh,
                                             IFX_TAPI_DECT_ENC_TYPE_t nCoder,
                                             IFX_TAPI_DECT_ENC_LENGTH_t nFrameLength);

   /** Prepare parameters and call the target function to activate DECT module
   \param pLLCh      Pointer to low-level channel structure
   \param nEnable    Enable or disable the module
   \return
   IFX_SUCCESS if successful
   IFX_ERROR if an error occured */
   IFX_return_t (*Enable)                   (IFX_TAPI_LL_CH_t *pLLCh,
                                             IFX_uint8_t nEnable);

   /** Sets the gains for the DECT en-/decoding path.
   \param pLLCh      Pointer to low-level channel structure
   \param pVol       Pointer to IFX_TAPI_LINE_VOLUME_t structure
   \return
   IFX_SUCCESS if successful
   IFX_ERROR if an error occured */
   IFX_return_t (*Gain_Set)                 (IFX_TAPI_LL_CH_t *pLLCh,
                                             IFX_TAPI_PKT_VOLUME_t const *pVol);

   /** Get the statistic data from the DECT coder channel
   \param pLLCh      Pointer to low-level channel structure
   \param pStatistic pointer to struct where to store the statistic data
   \return
   IFX_SUCCESS if successful
   IFX_ERROR if an error occured */
   IFX_return_t (*Statistic)                (IFX_TAPI_LL_CH_t *pLLCh,
                                             IFX_TAPI_DECT_STATISTICS_t *pStatistic);

   /** Do low level UTG configuration and activation
   \param pLLCh         Pointer to Low-level channel structure
   \param pSimpleTone   Internal simple tone table entry
   \param dst           Destination (unused)
   \param utgNum        UTG number  (always 0)
   \return
   IFX_SUCCESS if successful
   IFX_ERROR if an error occured */
   IFX_int32_t  (*UTG_Start)                (IFX_TAPI_LL_CH_t *pLLCh,
                                             IFX_TAPI_TONE_SIMPLE_t const *pSimpleTone,
                                             TAPI_TONE_DST dst,
                                             IFX_uint8_t res);

   /** Stop playing the tone with the given tone definition
   \param pLLCh         Pointer to Low-level channel structure
   \param res           Resource number used for playing the tone (always 0)
   \return
   IFX_SUCCESS if successful
   IFX_ERROR if an error occured */
   IFX_int32_t  (*UTG_Stop)                 (IFX_TAPI_LL_CH_t *pLLCh,
                                             IFX_uint8_t res);

} IFX_TAPI_DRV_CTX_DECT_t;
/*@}*/ /* DECT_MODULE*/


/** CONnection Module */
/** \addtogroup CON_MODULE */
/** Connection Module services*/
/*@{*/
typedef struct
{
   /** Adds a connection between a data channel and an analog phone channel or PCM channel.
   \param pLLCh           Pointer to Low-level channel structure
   \param pMap            Pointer to IFX_TAPI_MAP_DATA_t structure
   \return
   IFX_SUCCESS if successful
   IFX_ERROR if an error occured */
   IFX_return_t (*Data_Channel_Add)         (IFX_TAPI_LL_CH_t *pLLCh, IFX_TAPI_MAP_DATA_t const *pMap);

   /** Removes a connection between a data channel and an analog phone channel
       or PCM channel.
   \param pLLCh           Pointer to Low-level channel structure
   \param pMap            Pointer to IFX_TAPI_MAP_DATA_t structure
   \return
   IFX_SUCCESS if successful
   IFX_ERROR if an error occured */
   IFX_return_t (*Data_Channel_Remove)      (IFX_TAPI_LL_CH_t *pLLCh, IFX_TAPI_MAP_DATA_t const *pMap);

   /** Adds a connection between a PCM channel and another PCM channel or
       an analog phone.
   \param pLLCh           Pointer to Low-level channel structure
   \param pMap            Pointer to IFX_TAPI_MAP_PCM_t structure
   \return
   IFX_SUCCESS if successful
   IFX_ERROR if an error occured */
   IFX_return_t (*PCM_Channel_Add)          (IFX_TAPI_LL_CH_t *pLLCh, IFX_TAPI_MAP_PCM_t const *pMap);

   /** Removes a connection between a PCM channel and another PCM channel or
       an analog phone.
   \param pLLCh           Pointer to Low-level channel structure
   \param pMap            Pointer to IFX_TAPI_MAP_PCM_t structure
   \return
   IFX_SUCCESS if successful
   IFX_ERROR if an error occured */
   IFX_return_t (*PCM_Channel_Remove)       (IFX_TAPI_LL_CH_t *pLLCh, IFX_TAPI_MAP_PCM_t const *pMap);

   /** Adds a connection between an analog phone channel and another analog
       phone channel or an PCM channel.
   \param pLLCh           Pointer to Low-level channel structure
   \param pMap            Pointer to IFX_TAPI_MAP_PHONE_t structure
   \return
   IFX_SUCCESS if successful
   IFX_ERROR if an error occured */
   IFX_return_t (*Phone_Channel_Add)        (IFX_TAPI_LL_CH_t *pLLCh, IFX_TAPI_MAP_PHONE_t const *pMap);

   /** Removes a connection between an analog phone channel and another analog
       phone channel or an PCM channel.
   \param pLLCh           Pointer to Low-level channel structure
   \param pMap            Pointer to IFX_TAPI_MAP_PHONE_t structure
   \return
   IFX_SUCCESS if successful
   IFX_ERROR if an error occured */
   IFX_return_t (*Phone_Channel_Remove)     (IFX_TAPI_LL_CH_t *pLLCh, IFX_TAPI_MAP_PHONE_t const *pMap);

   /** Adds a connection between a DECT module and an analog phone module,
       a PCM module, an Audio module or another DECT module.
   \param pLLCh           Pointer to Low-level channel structure
   \param pMap            Pointer to IFX_TAPI_MAP_DECT_t structure
   \return
   IFX_SUCCESS if successful
   IFX_ERROR if an error occured */
   IFX_return_t (*DECT_Channel_Add)        (IFX_TAPI_LL_CH_t *pLLCh,
                                            IFX_TAPI_MAP_DECT_t const *pMap);

   /** Removes a connection between a DECT module and analog phone module,
       a PCM module, an Audio module or another DECT module.
   \param pLLCh           Pointer to Low-level channel structure
   \param pMap            Pointer to IFX_TAPI_MAP_DECT_t structure
   \return
   IFX_SUCCESS if successful
   IFX_ERROR if an error occured */
   IFX_return_t (*DECT_Channel_Remove)     (IFX_TAPI_LL_CH_t *pLLCh,
                                            IFX_TAPI_MAP_DECT_t const *pMap);

   /** Mute/Unmute all connections to modules which are attached to the given
      data channel except the first module connected to the local side.
   \param pLLCh           Pointer to Low-level channel structure
   \param nMute           IFX_TRUE: mute / IFX_FALSE: unmute
   \return
   IFX_SUCCESS if successful
   IFX_ERROR if an error occured */
   IFX_return_t (*Data_Channel_Mute) (IFX_TAPI_LL_CH_t *pLLCh, IFX_boolean_t nMute);

} IFX_TAPI_DRV_CTX_CON_t;
/*@}*/

/** Polling Interface */ /* ****************************************************/
/** \addtogroup POLL_INTERFACE */
/** List of required low-level polling services */
/*@{*/
typedef struct
{
   /** Read voice/fax/event packets from the device
   \param pLLDev           Pointer to low-level device structure
   \param ppPkts           Array of free buffer pointers
   \param pPktsNum         On entry identifies the number of packets to be read,
                           on return it contains the number of packets read
   \return
   IFX_SUCCESS on success
   IFX_ERROR on error */
   IFX_return_t (*rdPkts) (IFX_TAPI_LL_DEV_t *pLLDev,
                           IFX_void_t **ppPkts,
                           IFX_int32_t *pPktsNum,
                           IFX_int32_t nDevID);

   /** Write voice/fax/event packets available
   \param pLLDev           Pointer to low-level device structure
   \param ppPkts           Array of packet buffer pointers to be written
   \param pPktsNum         On entry identifies the number of packets to be written,
                           on return it contains the number of packets successfully
                           written. On success (IFX_SUCCESS) all packets have been
                           successfully written.
   \return
   IFX_SUCCESS on success
   IFX_ERROR on error */
   IFX_return_t (*wrPkts) (IFX_TAPI_LL_DEV_t *pLLDev, IFX_int32_t *pPktsNum);

   /** Updates the low-level TAPI device status by reading the hardware status
       registers and taking the appropriate actions upon status change.
       Typically this function executes the device's ISR.

   \param pLLDev           Pointer to low-level device structure
   \return
   IFX_SUCCESS on success
   IFX_ERROR on error */
   IFX_return_t (*pollEvents) (IFX_TAPI_LL_DEV_t *pLLDev);

   /** Used to control the packet-generation related interrupts. In case a
       device is registerred for packet polling it is necessary to disable
       the related interrupts so as to prohibit any unwanted overhead of
       switching to interrupt context.

   \param pLLDev           Pointer to low-level device structure
   \param bEnable          IFX_TRUE to enable, IFX_FALSE to disable the
                           related interrupts
   \return
   IFX_SUCCESS on success
   IFX_ERROR on error */
   IFX_return_t (*pktsIRQCtrl) (IFX_TAPI_LL_DEV_t *pLLDev,
                                IFX_boolean_t bEnable);

   /** Used to control the TAPI event-generation related interrupts. In case a
       device is registerred for events polling it is necessary to disable
       the related interrupts so as to prohibit any unwanted overhead of
       switching to interrupt context.

   \param pLLDev           Pointer to low-level device structure
   \param bEnable          IFX_TRUE to enable, IFX_FALSE to disable the
                           related interrupts
   \return
   IFX_SUCCESS on success
   IFX_ERROR on error */
   IFX_return_t (*evtsIRQCtrl) (IFX_TAPI_LL_DEV_t *pLLDev,
                                IFX_boolean_t bEnable);

} IFX_TAPI_DRV_CTX_POLL_t;
/*@}*/

/**
   report the reason and details of a cmd error to drv_tapi
*/
typedef struct _IFX_TAPI_DBG_CERR {
   /** reason code */
   IFX_uint32_t      cause;
   /** cmd header */
   IFX_uint32_t      cmd;
} IFX_TAPI_DBG_CERR_t;

/* driver context data structure */
/** \addtogroup TAPI_LL_INTERFACE */
/** Interface between High-Level TAPI and Low-Level TAPI */
/*@{*/
typedef struct
{
   /** high-level and low-level interface API version, keep as first element */
   IFX_char_t                              *hlLLInterfaceVersion;
   /** device nodes prefix (if DEVFS is used) /dev/<devNodeName><number> */
   IFX_char_t                              *devNodeName;
   /** driverName */
   IFX_char_t                              *drvName;
   /** driverVersion */
   IFX_char_t                              *drvVersion;

   IFX_uint16_t                             majorNumber;
   IFX_uint16_t                             minorBase;
   IFX_uint16_t                             maxDevs;
   IFX_uint16_t                             maxChannels;

   /* the following two functions receive a pointer to the
      HL device and channel structures(!) */
   IFX_TAPI_LL_DEV_t* (*Prepare_Dev)        (TAPI_DEV *pTapiDev,
                                             IFX_uint32_t devNum);
   IFX_TAPI_LL_CH_t*  (*Prepare_Ch)         (TAPI_CHANNEL *,
                                             IFX_uint32_t devNum,
                                             IFX_uint32_t chNum);
   IFX_return_t  (*Init_Dev)                (TAPI_CHANNEL *pDev,
                                             IFX_TAPI_CH_INIT_t const *pInit);
   IFX_return_t  (*Init_Ch)                 (IFX_TAPI_LL_CH_t *pCh,
                                             IFX_TAPI_CH_INIT_t const *pInit);
   IFX_int32_t   (*Pwr_Save_Dev)            (IFX_TAPI_LL_DEV_t *pDev);

   /* capabilities */
   IFX_uint32_t  (*CAP_Number_Get)          (IFX_TAPI_LL_DEV_t *pDev);
   IFX_return_t  (*CAP_List_Get)            (IFX_TAPI_LL_DEV_t *pDev,
                                             IFX_TAPI_CAP_t *pCapList);
   IFX_int32_t   (*CAP_Check)               (IFX_TAPI_LL_DEV_t *pDev,
                                             IFX_TAPI_CAP_t *pCapList);

   /* */
   IFX_return_t  (*GetCmdMbxSize)           (IFX_TAPI_LL_DEV_t *pDev,
                                             IFX_uint8_t *cmdmbx_size);
   IFX_int32_t   (*UpdateChMember)          (IFX_TAPI_LL_CH_t *pCh);

   /* forward standard fops to low level driver */
   IFX_int32_t   (*Read)                    (IFX_TAPI_LL_CH_t *pCh,
                                             IFX_char_t* buf,
                                             IFX_int32_t,  IFX_int32_t*);
   IFX_int32_t   (*Write)                   (IFX_TAPI_LL_CH_t *pCh,
                                             const char *buf,
                                             IFX_int32_t, IFX_int32_t*);
   IFX_int32_t   (*Ioctl)                   (IFX_TAPI_LL_CH_t *pCh,
                                             IFX_uint32_t, IFX_uint32_t);

   /* Coder related functions for the HL TAPI */
   IFX_TAPI_DRV_CTX_COD_t                   COD;

   /* PCM related functions for the HL TAPI */
   IFX_TAPI_DRV_CTX_PCM_t                   PCM;

   /* Signalling Module related functions for HL TAPI */
   IFX_TAPI_DRV_CTX_SIG_t                   SIG;

   /* Analog Line Module related functions for HL TAPI */
   IFX_TAPI_DRV_CTX_ALM_t                   ALM;

   /* DECT Module related functions for HL TAPI */
   IFX_TAPI_DRV_CTX_DECT_t                  DECT;

   /* Connection related functions for the HL TAPI */
   IFX_TAPI_DRV_CTX_CON_t                   CON;

   /* Protection andn Interrupt module functions for the HL TAPI */
   IFX_TAPI_DRV_CTX_IRQ_t                   IRQ;

   /* Audio Module related functions for HL TAPI */
   IFX_TAPI_DRV_CTX_AUDIO_t                 AUDIO;

   /* Polling interface related LL routines to be used by HL TAPI */
   IFX_TAPI_DRV_CTX_POLL_t                  POLL;

   /** array of pTapiDev pointers associated with this driver context */
   TAPI_DEV                                *pTapiDev;

   IFX_int32_t (*Open)    (IFX_int32_t, IFX_void_t *);
   IFX_int32_t (*Release) (IFX_int32_t, IFX_void_t *, IFX_TAPI_LL_DEV_t *);

   IFX_int32_t (*Dbg_CErr_Handler) (IFX_TAPI_LL_DEV_t *pLLDev, IFX_TAPI_DBG_CERR_t *pData);

   /** Low level func Ingress --> writting to packet inbox. */
   IFX_return_t (*Ingress)(IFX_TAPI_LL_CH_t *pLLCh, IFX_void_t* pData, IFX_uint32_t nLen);

   /** Low level func Egress --> reading from packet FIFO. */
   IFX_return_t (*Egress)(IFX_TAPI_LL_CH_t *pLLCh);

   IFX_int32_t  (*KpiWrite)                 (IFX_TAPI_LL_CH_t *pCh,
                                             const char *buf,
                                             IFX_int32_t, IFX_uint32_t);

} IFX_TAPI_DRV_CTX_t;
/*@}*/

#ifndef DRV_TAPI_H
struct _TAPI_DEV
{
   /* channel number IFX_TAPI_DEVICE_CH_NUMBER indicates the control device */
   /* ATTENTION, nChannel must be the first element */
   IFX_uint8_t               nChannel;
   /* TODO: required modularization, access  to low level device
            should be a pointer to void -> see list of AI */
   IFX_TAPI_LL_DEV_t        *pLLDev;
};

struct _TAPI_CHANNEL
{
   /* channel number */
   /* ATTENTION, nChannel must be the first element */
   IFX_uint8_t                   nChannel;
   /* pointer to the Low level driver channel */
   IFX_TAPI_LL_CH_t             *pLLChannel;
   /* pointer to the tapi device structure */
   TAPI_DEV                     *pTapiDevice;
   /**\todo To be removed */
#ifdef TAPI_PACKET
   /* semaphore used only in blocking read access,
      in this case given from interrupt context */
   IFXOS_event_t                 semReadBlock;
   /* wakeup queue for select on read */
   IFXOS_wakelist_t              wqRead;
   /* wakeup queue for select on write */
   IFXOS_wakelist_t              wqWrite;
   /* stores the current fax status */
   volatile IFX_boolean_t        bFaxDataRequest;
   /* flags for different purposes, see CH_FLAGS */
   IFX_uint32_t                  nFlags;
#endif /* TAPI_PACKET */
};
#endif /* DRV_TAPI_H */

typedef struct IFX_TAPI_DRV_CTX_t TAPI_LOW_LEVEL_DRV_CTX_t;

/* Registration function for the Low Level TAPI driver */
extern IFX_return_t IFX_TAPI_Register_LL_Drv   (IFX_TAPI_DRV_CTX_t*);
extern IFX_return_t IFX_TAPI_Unregister_LL_Drv (int majorNumber);
extern IFX_void_t   IFX_TAPI_ResetChState      (TAPI_CHANNEL *);

#ifdef TAPI_PACKET
extern IFX_void_t*   TAPI_VoiceBufferPoolGet (void);
extern IFX_void_t*   TAPI_VoiceBufferPoolHandle_Get (void);
extern FIFO_ID*      TAPI_UpStreamFifo_Get(TAPI_CHANNEL* pTapiCh);
extern FIFO_ID*      TAPI_DownStreamFifo_Get(TAPI_DEV* pTapiDev);
extern IFX_return_t  TAPI_ClearFifo(FIFO_ID* pFifo);
#endif /* TAPI_PACKET */

extern IFX_return_t IFX_TAPI_Event_Dispatch      (TAPI_CHANNEL *,
                                                  IFX_TAPI_EVENT_t *);
extern IFX_void_t   TAPI_Tone_Set_Source         (TAPI_CHANNEL *,
                                                  IFX_uint8_t res,
                                                  IFX_int32_t src);
extern IFX_void_t TAPI_Cid_Abort                 (TAPI_CHANNEL *pChannel);
extern IFX_boolean_t TAPI_Cid_IsActive           (TAPI_CHANNEL *pChannel);
extern IFX_boolean_t TAPI_Cid_UseSequence        (TAPI_CHANNEL *pChannel);
extern TAPI_CMPLX_TONE_STATE_t TAPI_ToneState    (TAPI_CHANNEL *pChannel,
                                                  IFX_uint8_t res);

/** Put packet received in the irq handler into the egress fifo */
extern IFX_return_t irq_IFX_TAPI_KPI_PutToEgress(TAPI_CHANNEL *pChannel,
                                                 IFX_TAPI_KPI_STREAM_t stream,
                                                 void *pPacket,
                                                 IFX_uint32_t nPacketLength);

/** Retrieve the KPI Channel number of a given stream on a given TAPI Channel */
extern IFX_TAPI_KPI_CH_t IFX_TAPI_KPI_ChGet     (TAPI_CHANNEL *pChannel,
                                                 IFX_TAPI_KPI_STREAM_t stream);

/* ======================================== */
/*                  QOS                     */
/* ======================================== */
#ifdef QOS_SUPPORT
IFX_return_t IFX_TAPI_Qos_HL_Init(TAPI_DEV* pTapiDev);
IFX_return_t irq_IFX_TAPI_Qos_PktEgressSched(TAPI_CHANNEL* pTapiCh);
IFX_void_t   irq_IFX_TAPI_Qos_HL_PktEgress(IFX_int32_t nCh,
		                                     IFX_void_t *pData,
						                         IFX_int32_t nLen);
IFX_boolean_t irq_IFX_TAPI_Qos_PacketRedirection(TAPI_CHANNEL* pTapiCh);
#endif /* QOS_SUPPORT */


#endif /* _DRV_TAPI_LL_INTERFACE_H */
