#ifndef DRV_TAPI_H
#define DRV_TAPI_H
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
   \file drv_tapi.h
   Contains TAPI functions declaration und structures.
*/

/* ============================= */
/* Includes                      */
/* ============================= */
/* it includes lots of files */
#include "drv_api.h"
#include <sys_drv_fifo.h>
#include <lib_fifo.h>
#include <lib_bufferpool.h>
#include "drv_tapi_io.h"
#include "drv_tapi_event.h"
#include "drv_tapi_qos.h"


/* ============================= */
/* Global defines                */
/* ============================= */

#define VMMC_IOC_MAGIC 'M'
#define VINETIC_IOC_MAGIC 'V'
#define SVIP_IOC_MAGIC 'S'
#define DUS_IOC_MAGIC 'D'

#ifndef PULSE_DIAL_DETECTION
#define PULSE_DIAL_DETECTION
#endif

/* number of internal tones for others driver purposes */
#define TAPI_MAX_RESERVED_TONES       10
/* maximum number of tones codes, comprising the maximum user tones and the
   maximum interna reserved tones. */
#define TAPI_MAX_TONE_CODE           (IFX_TAPI_TONE_INDEX_MAX + \
                                      TAPI_MAX_RESERVED_TONES)

/** Maximum parameters for the hook state machine */
#define MAX_HOOKVT_PARAMS 7

#define TAPI_MIN_FLASH                80
#define TAPI_MAX_FLASH               200
#define TAPI_MIN_FLASH_MAKE          200
#define TAPI_MIN_DIGIT_LOW            30
#define TAPI_MAX_DIGIT_LOW            80
#define TAPI_MIN_DIGIT_HIGH           30
#define TAPI_MAX_DIGIT_HIGH           80
#define TAPI_MIN_OFF_HOOK             40
#define TAPI_MIN_ON_HOOK             400
#define TAPI_MIN_INTERDIGIT          300

/** time in ms while the line is in power down after the first Ground Key High */
#define GNDKH_RECHECKTIME1            10
/** time in ms to check the next Ground Key High after reenabling the line */
#define GNDKH_RECHECKTIME2            20

/** maximum number of allowed LL drivers */
#ifndef TAPI_MAX_LL_DEVICES
#define TAPI_MAX_LL_DEVICES            5
#endif /* TAPI_MAX_LL_DEVICES */

/* Upper limit for used channels as used in IFX_TAPI_CH_STATUS_GET */
#define TAPI_MAX_CHANNELS            128

#ifndef TAPI_TONE_MAXRES
/** Maximum tone generators that can run in parallel: 3 modules ALM, PCM,
    COD and CONF and 2 tone generators */
#define TAPI_TONE_MAXRES 3
#endif

#ifndef IFX_TAPI_EVENT_POOL_INITIAL_SIZE
/** Initial number of events structures allocated by the driver. If more
    event structures are needed the pool grows automatically in steps of
    IFX_TAPI_EVENT_POOL_GROW_SIZE */
#define IFX_TAPI_EVENT_POOL_INITIAL_SIZE 50
#endif

#ifndef IFX_TAPI_EVENT_POOL_GROW_SIZE
/** Number of events that the event structure pool grows every time it gets
    depleted. */
#define IFX_TAPI_EVENT_POOL_GROW_SIZE 50
#endif

#define IFX_TAPI_DTMF_FIFO_SIZE                   20
#ifndef IFX_TAPI_EVENT_FIFO_SIZE
/** Event Fifo Size */
   #define IFX_TAPI_EVENT_FIFO_SIZE               10
#endif /* IFX_TAPI_EVENT_FIFO_SIZE */

#ifdef EVENT_LOGGER_DEBUG
/* Event Logger (debugging) macros, defining more
   meaningful names to Event Logger macros */
#endif /* EVENT_LOGGER_DEBUG */

#ifdef TAPI_EXT_KEYPAD
/*This info is used to set flag "nDtmfInfo" for DTMF incase of external key event*/
enum DTMF_INFO
{
/*
   DTMF_EV_OOB_DEAFULT = 0x01,
   DTMF_EV_OOB_NO      = 0x02,
   DTMF_EV_OOB_ONLY    = 0x04,
   DTMF_EV_OOB_ALL     = 0x08,
   DTMF_EV_OOB_BLOCK   = 0x10,
*/
   DTMF_EV_LOCAL_PLAY  = 0x80
};
#endif /* TAPI_EXT_KEYPAD */

/* ============================= */
/* Global variables declaration  */
/* ============================= */

/* Declarations for debug interface */
DECLARE_TRACE_GROUP  (TAPI_DRV);
DECLARE_LOG_GROUP    (TAPI_DRV);
/* global high level driver context used in system interface only */
extern struct _IFX_TAPI_HL_DRV_CTX gHLDrvCtx [TAPI_MAX_LL_DEVICES];
/* Counter of registered low level device drivers */
extern int ifx_tapi_drvctx_count;

/* =================================== */
/* Global typedef forward declarations */
/* =================================== */
typedef struct _TAPI_DEV      TAPI_DEV;
typedef struct _TAPI_CHANNEL  TAPI_CHANNEL;


/* ================================= */
/* Defines for tapi operating modes  */
/* ================================= */

/** Bit 0 used for packets and bit 1 for events. */
typedef enum _TAPI_WORKING_MODE
{
   /** Interrupt mode for packets */
   TAPI_INTERRUPT_MODE_PACKETS = 0,
   /** Interrupt mode for events */
   TAPI_INTERRUPT_MODE_EVENTS = 0,
   /** Polling mode for packets */
   TAPI_POLLING_MODE_PACKETS = 1,
   /** Polling mode for events */
   TAPI_POLLING_MODE_EVENTS = 2
} TAPI_WORKING_MODE;

/* =============================== */
/* Defines for CID operations      */
/* =============================== */

/* Default indexes for CID alert tones */
enum _TAPI_CID_ALERTTONE_INDEX_DEFAULT
{
   /* Alert Tone default index, onhook */
   TAPI_CID_ATONE_INDEXDEF_ONHOOK       = IFX_TAPI_TONE_INDEX_MAX + 1,
   /* Alert Tone default index, offhook */
   TAPI_CID_ATONE_INDEXDEF_OFFHOOK      = IFX_TAPI_TONE_INDEX_MAX + 2,
   /* Alert Tone default index, AS NTT */
   TAPI_CID_ATONE_INDEXDEF_OFFHOOKNTT   = IFX_TAPI_TONE_INDEX_MAX + 3,
   /* define others cid reserved tones,
      should be in the range of reserved tones */
   TAPI_CID_ATONE_INDEXDEF_MAX          = TAPI_MAX_TONE_CODE
};

typedef enum
{
   /** ready to start sequence */
   TAPI_CID_STATE_IDLE,
   /** alert active */
   TAPI_CID_STATE_ALERT,
   /** waiting for ack */
   TAPI_CID_STATE_ACK,
   /** fsk or dtmf sending active */
   TAPI_CID_STATE_SENDING,
   /** fsk or dtmf sending completed (or timed out) */
   TAPI_CID_STATE_SENDING_COMPLETE
} TAPI_CID_STATE;

typedef enum
{
   TAPI_CID_ALERT_NONE,
   /** first ring burst */
   TAPI_CID_ALERT_FR,
   /** DTAS */
   TAPI_CID_ALERT_DTAS,
   /** Line Reversal with DTAS */
   TAPI_CID_ALERT_LR_DTAS,
   /** Ring Pulse */
   TAPI_CID_ALERT_RP,
   /** Open Switch Interval */
   TAPI_CID_ALERT_OSI,
   /** CPE Alert Signal */
   TAPI_CID_ALERT_CAS,
   /** Alert Signal (NTT) */
   TAPI_CID_ALERT_AS_NTT,
   /** CAR Signal (NTT) */
   TAPI_CID_ALERT_CAR_NTT
} TAPI_CID_ALERT_TYPE;

/* ============================= */
/* Enumeration for hook state    */
/* Finite State Machine          */
/* ============================= */
typedef enum
{
   /* phone is offhook, possibly waiting for interdigit timer */
   TAPI_HOOK_STATE_OFFHOOK,
   /* phone is offhook, waiting for next dialing pulse */
   TAPI_HOOK_STATE_PULSE_H_CONFIRM,
   /* phone has gone offhook: it may just be noise */
   TAPI_HOOK_STATE_OFFHOOK_VAL,
   /* phone has gone offhook while collecting pulses */
   TAPI_HOOK_STATE_PULSE_H_VAL,
   /* phone has gone offhook with an overlap of flash hook min time and digit
      low max time. In this state both digit 1 and flash are reported */
   TAPI_HOOK_STATE_PULSE_H_FLASH_VAL,
   /* phone is onhook, and no timers are running */
   TAPI_HOOK_STATE_ONHOOK,
   /* phone has remained onhook long enough to be low pulse: wait for next offhook to confirm */
   TAPI_HOOK_STATE_PULSE_L_CONFIRM,
   /* overlap of flash hook min time and digit low max time. In this
      state both digit 1 and flash are reported */
   TAPI_HOOK_STATE_PULSE_L_FLASH_CONFIRM,
   /* phone onhook (too long to be digit pulse): could be flash or final onhook */
   TAPI_HOOK_STATE_FLASH_WAIT,
   /* phone onhook (long enough to be flash): wait for next offhook to confirm this */
   TAPI_HOOK_STATE_FLASH_CONFIRM,
   /* validation of the flash hook time after off hook */
   TAPI_HOOK_STATE_FLASH_VAL,
   /* phone onhook (long enough to be final onhook): wait for onhook timer to confirm */
   TAPI_HOOK_STATE_ONHOOK_CONFIRM,
   /* phone has gone onhook: it may just be noise */
   TAPI_HOOK_STATE_ONHOOK_VAL,
   /* phone has gone onhook during pulse dialing: wait to validate */
   TAPI_HOOK_STATE_DIAL_L_VAL
}TAPI_HOOK_STATE;

/** \internal */

/* ============================= */
/* Structure for CID data        */
/* ============================= */
typedef struct
{
   /* sets CID type 1 (ON-) or type 2 (OFF-HOOK) */
   IFX_TAPI_CID_HOOK_MODE_t    txHookMode;
   /* Buffer for CID parameter coding  */
   IFX_uint8_t             cidParam [IFX_TAPI_CID_TX_SIZE_MAX];
   /* number of CID parameters octets */
   IFX_uint16_t            nCidParamLen;


   /* use the state machine to play a sequence or send data without sequence */
   IFX_boolean_t           bUseSequence;
   /* flag to know if transmission/sequence is active (and data is new) */
   IFX_boolean_t           bActive;

   /* reverse line mode for standards with Line Reversal */
   IFX_uint32_t            nLineModeReverse;
   /* current line mode that is set when CID tx is started */
   IFX_uint32_t            nLineModeInitial;

   /* Timer for caller id handling */
   Timer_ID                CidTimerID;

   /* CID transmission state */
   TAPI_CID_STATE          nCidState;
   /* common sub state (used by alert,...) */
   IFX_uint32_t            nCidSubState;

   /* alert type for sequence */
   TAPI_CID_ALERT_TYPE     nAlertType;
   /* Calculated transmission time according to FSK/DTMF buffer size */
   IFX_uint32_t            nTxTime;

   /* required acknowledge tone as chip specific code */
   IFX_uint8_t             ackToneCode;
   /* acknowledge flags */
   volatile IFX_boolean_t  bAck;
   volatile IFX_boolean_t  bAck2;

   /* flag, if muting for off hook cid was done */
   IFX_boolean_t           bMute;
   /* flag for starting periodical ringing */
   IFX_boolean_t           bRingStart;

   /* Instance of phone channel mapped to
      this data channel */
   TAPI_CHANNEL         *pPhoneCh;
} TAPI_CID_DATA_t;

typedef struct
{
   /* status */
   IFX_TAPI_CID_RX_STATUS_t  stat;
   /* ptr to actual cid rx buffer */
   IFX_TAPI_CID_RX_DATA_t   *pData;
   /* cid rx fifo */
   FIFO               TapiCidRxFifo;
} TAPI_CIDRX_t;

/* include definitions of the low level driver interface which require
   some of the definitions above */
#include "drv_tapi_ll_interface.h"
/* include for KPI requires the TAPI_CHANNEL so include it here */
#ifdef KPI_SUPPORT
#include "drv_tapi_kpi.h"
#endif /* KPI_SUPPORT */

typedef struct _IFX_TAPI_HL_DRV_CTX
{
   IFX_boolean_t       bInUse;
   IFX_TAPI_DRV_CTX_t *pDrvCtx;
#ifdef LINUX
   /* buffer to store the registered device driver name, Linux references to
      this string, e.g. on cat /proc/devices - please don't remove /Olaf */
   IFX_char_t          registeredDrvName[20];
#ifdef CONFIG_DEVFS_FS
   devfs_handle_t      TAPI_devfs_handle [TAPI_MAX_DEVFS_HANDLES];
#endif /* CONFIG_DEVFS_FS */
#endif /* LINUX */
} IFX_TAPI_HL_DRV_CTX_t;

/* ============================= */
/** Structure for operation control
    \internal */
/* ============================= */
typedef struct
{
   /* interrupt routine save hookstatus here */
   unsigned char              bHookState;
   /* polarity status from the line; 1 = reversed; 0 = normal */
   unsigned char             nPolarity;
   /* automatic battery switch; 1 = automatic; 0 = normal */
   unsigned char             nBatterySw;
   /* last line feed mode; is set after ringing stops */
   unsigned char             nLineMode;
   /* Last line type; changed when new line type changed with success. */
   IFX_TAPI_LINE_TYPE_t      nLineType;
   /** number of the assigned daa instance (LineType FXO),
       handled by the daa abstraction driver */
   IFX_uint16_t              nDAA;
   /** status of DAA channel initialisation
       (GPIO reservation, etc is done only once) */
   IFX_boolean_t             bDaaInitialized;
   /* set when the fault condition occurs of the device. It will be reseted
      when the line mode is modified */
   unsigned char             bFaulCond;
} TAPI_OPCONTROL_DATA_t;

/* ============================= */
/** Structure for ring data
    \internal */
/* ============================= */
typedef struct
{
   /* timer id for ringing service */
   Timer_ID                   RingTimerID;
   /* is channel in ringing mode? */
   volatile IFX_boolean_t     bRingingMode;
   /* is channel in ring_burst mode? */
   IFX_boolean_t              bIsRingBurstState;
   /* cadence data */
   IFX_TAPI_RING_CADENCE_t    RingCadence;
   /* ring configuration data */
   IFX_TAPI_RING_CFG_t        RingConfig;
   /* counter to keep the current position in the cadence */
   IFX_uint16_t               nBitPos;
   /* pointer to the cadence we currently use */
   IFX_uint8_t                *pCurrentCadence;
   /* copy of the number of bits in the cadence we currently use */
   IFX_uint16_t               BitsInCurrentCadence;
   /* indicates that this is the final sequence of the last ring */
   IFX_boolean_t              bFinal;
   /* ringing will stop automatically after the maximum ring was reached */
   IFX_uint32_t               nMaxRings;
   /* keep remaining rings until nMaxRings */
   IFX_uint32_t               nRingsLeft;
}TAPI_RING_DATA_t;

/*@{*/

/* ============================= */
/* Structure for pcm data        */
/* ============================= */
typedef struct
{
   /* configuration data for pcm services */
   IFX_TAPI_PCM_CFG_t   PCMConfig;
   /* save activation status */
   IFX_boolean_t        bTimeSlotActive;
}TAPI_PCM_DATA_t;


/* ============================= */
/* Structure for tone data       */
/*                               */
/* ============================= */
typedef struct
{
   /* network tone on/off */
   IFX_boolean_t     bNetTone;
   /* off time duration */
   IFX_uint32_t      nOffTime;
   /* on time duration */
   IFX_uint32_t      nOnTime;
   /* tone playing state */
   IFX_uint32_t      nToneState;
   IFX_boolean_t     bUseSecondTG;
}TAPI_TG_TONE_DATA_t;


/* ============================= */
/* Structure for miscellaneous   */
/* data                          */
/* ============================= */
typedef enum
{
   /** no ground key detected */
   GNDKH_STATE_READY,
   /** first ground key high detected and line set to power down */
   GNDKH_STATE_POWDN,
   /** timer 1 expired and line is set to orign mode to recheck
       ground key high again */
   GNDKH_STATE_RECHECK,
   /** line is in fault state, cause ground key came twice */
   GNDKH_STATE_FAULT
}GNDKH_STATE;

typedef struct
{
   /* exception status */
   IFX_TAPI_EXCEPTION_t    nException;
   /* exception status mask */
   IFX_TAPI_EXCEPTION_t    nExceptionMask;
   IFX_int32_t       line;
   /** line signals for fax and modem detection */
   /* Moved to low layer TAPI */
   IFX_uint32_t      signal;
   IFX_uint32_t      signalExt;
   /** Currently enabled line signals, zero means enabled.
      The current signal mask is set by the interface enable/disable signal */
   /*IFX_uint32_t      sigMask;*/
   /*IFX_uint32_t      sigMaskExt;*/
   /** device specific information. Bit oriented */
   IFX_uint32_t      device;
   /** Ground key high validation timer */
   Timer_ID          GndkhTimerID;
   /** current state of ground key high validation */
   IFX_int32_t       GndkhState;
   /** RFC2833 event played out */
   IFX_uint32_t      event;
   /** runtime errors */
   IFX_uint32_t      error;
}TAPI_MISC_DATA_t;

/* ============================= */
/* Structure for dial data       */
/*                               */
/* ============================= */
typedef struct
{
   /* state of the hookstate Finite State Machine */
   TAPI_HOOK_STATE   nHookState;
   /* timer id for dial service */
   Timer_ID          DialTimerID;
   /* number of hook changes */
   IFX_uint8_t       nHookChanges;
   struct
   {
      /* a flash hook might has been detected in case of on overlap
         with digit validation time */
      unsigned int bProbablyFlash:1;
   }state;
}TAPI_DIAL_DATA_t;

/* ============================= */
/* Structure for metering data   */
/*                               */
/* ============================= */
typedef struct
{
   /* is metering active actually ? */
   IFX_boolean_t      bMeterActive;
   /* timer id for metering */
   Timer_ID           MeterTimerID;
   /* metering coniguration data */
   IFX_TAPI_METER_CFG_t  MeterConfig;
   /* last linemode befor metering start */
   IFX_uint8_t        nLastLineMode;
   /* is channel in metering mode? */
   IFX_boolean_t      bMeterBurstMode;
   /* elapsed count */
   IFX_uint32_t       nElapsedCnt;
}TAPI_METER_DATA_t;

/* ============================= */
/* Structure for internal tone   */
/* coefficients table            */
/* ============================= */
typedef enum
{
   TAPI_TONE_TYPE_NONE,
   TAPI_TONE_TYPE_SIMPLE,
   TAPI_TONE_TYPE_COMP,
   TAPI_TONE_TYPE_PREDEF,
   TAPI_TONE_TYPE_DUAL
}TAPI_TONE_TYPE;

typedef struct
{
   TAPI_TONE_TYPE          type;
   union
   {
      IFX_int32_t          nPredefined;
      IFX_TAPI_TONE_DUAL_t       dual;
      IFX_TAPI_TONE_SIMPLE_t     simple;
      IFX_TAPI_TONE_COMPOSED_t   composed;
   } tone;
}COMPLEX_TONE;

/* ============================= */
/* Structure for complex tone    */
/* data                          */
/* ============================= */
typedef struct
{
   /* type of tone sequence SIMPLE or COMPOSED */
   TAPI_TONE_TYPE          nType;
   IFX_uint32_t            nSimpleMaxReps;
   /* current repetition counter for simple tone sequence */
   IFX_uint32_t            nSimpleCurrReps;
   /* maximum number of repetitions for composed tone sequence */
   IFX_uint32_t            nComposedMaxReps;
   /* current repetition counter for composed tone sequence */
   IFX_uint32_t            nComposedCurrReps;
   /* pause time in ms betweensimple tones */
   IFX_uint32_t            nPauseTime;
   /* alternate voice timein ms between composed tones */
   IFX_uint32_t            nAlternateVoiceTime;
   /* current simple tone codeplaying */
   IFX_uint32_t            nSimpleToneCode;
   /* current composed tone code playing */
   IFX_uint32_t            nComposedToneCode;
   /* maximum simple toneswithin composed tone */
   IFX_uint32_t            nMaxToneCount;
   /* current simple tone within composed tone */
   IFX_uint32_t            nToneCounter;
   /* complex tone playing state */
   TAPI_CMPLX_TONE_STATE_t nToneState;
   /** Specifies the capability of the tone generator regarding tone
       sequence support. See \ref IFX_TAPI_TONE_RESSEQ_t for details. */
   IFX_TAPI_TONE_RESSEQ_t sequenceCap;
   /** which direction to play the tone (net or local) */
   TAPI_TONE_DST dst;
   /** stores the index of the tone currently played */
   IFX_int32_t       nToneIndex;
}TAPI_TONE_DATA_t;


/** tone resource structure as time argument */
typedef struct
{
   /* timer id for voice path establisment */
   Timer_ID          Tone_Timer;
   TAPI_CHANNEL      *pTapiCh;
   /** Resource number of the tone play unit */
   IFX_uint32_t      nRes;
}TAPI_TONERES;


typedef struct
{
   /* startup on-hook timer value */
   IFX_uint32_t            nOnHookMinTime;
   IFX_uint32_t            nOnHookMaxTime;
   IFX_boolean_t           bOnHookRestore;
   /* startup hookflash timer value */
   IFX_uint32_t            nHookFlashMinTime;
   IFX_uint32_t            nHookFlashMaxTime;
   IFX_boolean_t           bHookFlashRestore;
}TAPI_VALIDATION_START_t;


/* ============================= */
/* channel specific structure    */
/* ============================= */
struct _TAPI_CHANNEL
{
   /* channel number */
   /* ATTENTION, nChannel must be the first element */
   IFX_uint8_t                   nChannel;
   /* pointer to the Low level driver channel */
   IFX_TAPI_LL_CH_t             *pLLChannel;
   /* pointer to the tapi device structure */
   TAPI_DEV                     *pTapiDevice;
   /* semaphore used only in blocking read access,
      in this case given from interrupt context */
   IFXOS_event_t                 semReadBlock;
   /* wakeup queue for select on read */
   IFXOS_wakelist_t              wqRead;
   /* wakeup queue for select on write */
   IFXOS_wakelist_t              wqWrite;
   /* stores the current fax status */
   volatile IFX_boolean_t        bFaxDataRequest;

   /* PS: moved from low level channel structure */
   /* flags for different purposes, see CH_FLAGS */
   IFX_uint32_t                  nFlags;

   /* channel is initialized */
   IFX_boolean_t                 bInitialized;

   /* locking semaphore for protecting data */
   IFXOS_mutex_t                 semTapiChDataLock;

   /* overall channel protection ( read/write/ioctl level)
   PS: Avoid nested locking of this mutex. It can lead to a deadlock */
   IFXOS_mutex_t                 semTapiChSingleIoctlAccess;


   /* data structures for services */
   TAPI_LEC_DATA_t               TapiLecAlmData;
   TAPI_LEC_DATA_t               TapiLecPcmData;
   TAPI_METER_DATA_t             TapiMeterData;
   TAPI_RING_DATA_t              TapiRingData;
   TAPI_OPCONTROL_DATA_t         TapiOpControlData;
   TAPI_PCM_DATA_t               TapiPCMData;
   TAPI_TG_TONE_DATA_t           TapiTgToneData;
   TAPI_MISC_DATA_t              TapiMiscData;
   TAPI_DIAL_DATA_t              TapiDialData;

   TAPI_TONERES                  *pToneRes;


   /* locking semaphore for ringing wait */
   IFXOS_event_t                 TapiRingEvent;
   /* cache jitter buffer configuration */
   IFX_TAPI_JB_CFG_t             TapiJbData;

   /* caller id config */
   TAPI_CID_CONF_t               TapiCidConf;
   /* caller id receiver data */
   TAPI_CIDRX_t                  TapiCidRx;
   /* caller id transmit data */
   TAPI_CID_DATA_t               TapiCidTx;

   /* stores the validation timer settings */
   IFX_TAPI_LINE_HOOK_VT_t       TapiHookOffTime;
   IFX_TAPI_LINE_HOOK_VT_t       TapiHookOnTime;
   IFX_TAPI_LINE_HOOK_VT_t       TapiHookFlashTime;
   IFX_TAPI_LINE_HOOK_VT_t       TapiHookFlashMakeTime;
   IFX_TAPI_LINE_HOOK_VT_t       TapiDigitLowTime;
   IFX_TAPI_LINE_HOOK_VT_t       TapiDigitHighTime;
   IFX_TAPI_LINE_HOOK_VT_t       TapiInterDigitTime;
   /* complex tone data */
   TAPI_TONE_DATA_t              TapiComplexToneData [TAPI_TONE_MAXRES];

   /* If this structure is used as a phone channel, keep track of
      connected dsp data channels in the following bitmask variable */
   IFX_uint32_t                  nPhoneDataChannels;
   /* If this structure is used as a dsp data channel, keep track of
      connected phone channels in the following bitmask variable */
   IFX_uint32_t                  nDataPhoneChannels;
   /* If this structure is used as a audio channel, keep track of
      connected dsp data channels in the following bitmask variable */
   IFX_uint32_t                  nAudioDataChannels;
   /* Handler of event dispatcher. */
   IFX_TAPI_EVENT_HANDLER_t      eventHandler;

   /** for external keypad (INCA-IP2 only) */
   IFX_uint8_t                   nDtmfInfo;

   /* In Use counter */
   IFX_uint16_t                  nInUse;

#ifdef TAPI_PACKET
   FIFO_ID*                      pUpStreamFifo;
#endif /* TAPI_PACKET */

#ifdef QOS_SUPPORT
   /** Control structure used for UDP redirection. */
   QOS_CTRL                      QosCtrl;
#endif /* QOS_SUPPORT */

#ifdef KPI_SUPPORT
   /** Control structure for the Kernel Packet Interface (KPI) */
   IFX_TAPI_KPI_STREAM_SWITCH    pKpiStream[IFX_TAPI_KPI_STREAM_MAX];
#endif /* KPI_SUPPORT */
};

/* ============================= */
/* tapi structure                */
/* ============================= */
struct _TAPI_DEV
{
   /* channel number IFX_TAPI_DEVICE_CH_NUMBER indicates the control device */
   /* ATTENTION, nChannel must be the first element */
   IFX_uint8_t               nChannel;
   /* TODO: required modularization, access  to low level device
            should be a pointer to void -> see list of AI */
   IFX_TAPI_LL_DEV_t        *pLLDev;
   /* number of channels for which memory is allocated, does not reflect
      the number of analog, signaling, pcm or coder channels */
   IFX_uint8_t               nMaxChannel;
   /* link to the device driver context */
   IFX_TAPI_DRV_CTX_t       *pDevDrvCtx;
   /* array of tapi channel structures */
   TAPI_CHANNEL             *pTapiChanelArray;

   /** Internal tone coefficients table. We use the zero values as flags to indicate
       that a tone table entry is unconfigured */
   COMPLEX_TONE             *pToneTbl;

   /* usage counter, counts the number of open fds */
   IFX_uint16_t              nInUse;
   /* already opened or not */
   IFX_boolean_t             bInitialized;
#ifdef TAPI_ONE_DEVNODE
   /** if set to IFX_TRUE an open was performed on a single dev node.
      This is important for the select wait */
   IFX_boolean_t             bSingleFd;
#endif /* TAPI_ONE_DEVNODE */
#ifdef TAPI_PACKET
   /* Device specific fifo for downstream direction. */
   FIFO_ID*                  pDownStreamFifo;
#endif /* TAPI_PACKET */

   /* TODO: wqEvent and bNeedWakeup should be in a separate structure */
   /* Event wakeup queue for select, this one is used to report all kind of
      device and channel events to the application. Its something like VxWorks
      version of select() call. */
   IFXOS_wakelist_t          wqEvent;

   /* Additional flag for vxWorks if a real wakeup is required */
   volatile IFX_boolean_t    bNeedWakeup;

   /* overall channel protection (ioctl level)
   PS: Avoid nested locking of this mutex. It can lead to a deadlock */
   IFXOS_mutex_t             semTapiDevSingleIoctlAccess;

#ifdef VXWORKS
   /** Driver specific parameter, info about driver. */
   DEV_HDR DevHdr;

   /** To see first open. */
   IFX_boolean_t bNotFirst;

   /** Exclusive open of device. */
   IFX_boolean_t bOpen;

   /* wakeup queue for select on read */
   /*IFXOS_wakelist_t          EventList;*/
#endif /* VXWORKS */
#ifdef TAPI_POLL
   IFX_TAPI_POLL_CONFIG_t *pPollCfg;
#endif
   /** unique TAPI device ID [0,1,...] */
   IFX_int32_t              nDevID;

   /** Flag for oparting mode of tapi (interrupt - default or polling). */
   TAPI_WORKING_MODE             fWorkingMode;
   /** last error code and error stack */
   IFX_TAPI_Error_t error;

   /* Timer for power saving feature if supported by low level device*/
   Timer_ID                      PwrSaveTimerID;
};

/* internal structure used by the event dispatcher to transport the event
   details from timer/interrupt context into process context
   Note:
   this structure requires the definition of the TAPI_CHANNEL above
   while dependencies to drv_tapi_event.h exist in parallel, the
   best place (although not nice) for this structure is here.
*/
typedef struct
{
#ifdef LINUX
#ifdef LINUX_2_6
   /* !!! important, work struct/tq_struct must be the first element,
          because we need to cast it later on to its surrounding structure
          IFX_TAPI_EXT_EVENT_PARAM_t */
   struct work_struct tapiWs;
#else
   struct tq_struct   tapiTq;
#endif /* LINUX_2_6 */
#endif /* LINUX */
   TAPI_CHANNEL     *pChannel;
   IFX_TAPI_EVENT_t *pTapiEvent;
   void             (*pFunc) (void*);
} IFX_TAPI_EXT_EVENT_PARAM_t;


/* ============================= */
/** TAPI  Global functions       */
/* ============================= */

/* driver initialisation */
extern IFX_int32_t IFX_TAPI_On_Driver_Load     (IFX_void_t);
extern IFX_void_t  IFX_TAPI_On_Driver_Unload   (IFX_void_t);
extern IFX_int32_t IFX_TAPI_Event_On_Driver_Load   (IFX_void_t);
extern IFX_void_t  IFX_TAPI_Event_On_Driver_Unload (IFX_void_t);

/* Line services */
extern IFX_int32_t TAPI_Phone_Set_Linefeed     (TAPI_CHANNEL *pChannel, IFX_int32_t nMode);
extern IFX_return_t TAPI_FXO_Register_DAA      (int nDAA, TAPI_CHANNEL *pChannel);
extern IFX_return_t TAPI_FXO_Init_DAA          (TAPI_CHANNEL *pChannel);

/* Ringing Services */
extern IFX_int32_t IFX_TAPI_Ring_Initialise    (TAPI_CHANNEL *pChannel);
extern IFX_int32_t IFX_TAPI_Ring_Cleanup       (TAPI_CHANNEL *pChannel);
extern IFX_int32_t IFX_TAPI_Ring_Prepare       (TAPI_CHANNEL *pChannel);
extern IFX_int32_t IFX_TAPI_Ring_Start         (TAPI_CHANNEL *pChannel);
extern IFX_int32_t IFX_TAPI_Ring_Stop          (TAPI_CHANNEL *pChannel);
extern IFX_int32_t IFX_TAPI_Ring_DoBlocking    (TAPI_CHANNEL *pChannel);
extern IFX_int32_t IFX_TAPI_Ring_SetCadence    (TAPI_CHANNEL *pChannel, IFX_uint32_t nCadence);
extern IFX_int32_t IFX_TAPI_Ring_SetCadenceHighRes (TAPI_CHANNEL *pChannel, IFX_TAPI_RING_CADENCE_t const *pCadence);
extern IFX_int32_t IFX_TAPI_Ring_SetConfig     (TAPI_CHANNEL *pChannel, IFX_TAPI_RING_CFG_t const *pRingConfig);
extern IFX_int32_t IFX_TAPI_Ring_GetConfig     (TAPI_CHANNEL *pChannel, IFX_TAPI_RING_CFG_t *pRingConfig);
extern IFX_int32_t IFX_TAPI_Ring_SetMaxRings   (TAPI_CHANNEL *pChannel, IFX_uint32_t nArg);
extern IFX_int32_t IFX_TAPI_Ring_Engine_Start  (TAPI_CHANNEL *pChannel, IFX_boolean_t bStartWithInitial);

/* Operation Control Services */
extern IFX_int32_t TAPI_Phone_Hookstate        (TAPI_CHANNEL *pChannel, IFX_int32_t *pState);
extern IFX_int32_t TAPI_Phone_LecConf_Alm      (TAPI_CHANNEL *pChannel, IFX_TAPI_LEC_CFG_t *pLecConf);
extern IFX_int32_t TAPI_Phone_LecConf_Pcm      (TAPI_CHANNEL *pChannel, IFX_TAPI_LEC_CFG_t *pLecConf);
extern IFX_int32_t TAPI_Phone_GetLecConf_Alm   (TAPI_CHANNEL *pChannel, IFX_TAPI_LEC_CFG_t *pLecConf);
extern IFX_int32_t TAPI_Phone_GetLecConf_Pcm   (TAPI_CHANNEL *pChannel, IFX_TAPI_LEC_CFG_t *pLecConf);
extern IFX_int32_t TAPI_Phone_LecMode_Alm_Set  (TAPI_CHANNEL *pChannel, IFX_TAPI_WLEC_CFG_t *pLecConf);
extern IFX_int32_t TAPI_Phone_LecMode_Alm_Get  (TAPI_CHANNEL *pChannel, IFX_TAPI_WLEC_CFG_t *pWLecConf);
extern IFX_int32_t TAPI_Phone_LecMode_Pcm_Set  (TAPI_CHANNEL *pChannel, IFX_TAPI_WLEC_CFG_t *pLecConf);
extern IFX_int32_t TAPI_Phone_LecMode_Pcm_Get  (TAPI_CHANNEL *pChannel, IFX_TAPI_WLEC_CFG_t *pLecConf);
extern IFX_void_t  TAPI_Power_Save_OnTimer     (Timer_ID Timer, IFX_int32_t nArg);

/* Tone Services */
IFX_int32_t TAPI_Phone_Tone_Play               (IFX_TAPI_DRV_CTX_t*, TAPI_CHANNEL *pChannel,
                                                IFX_int32_t nToneIndex, TAPI_TONE_DST dst);
IFX_int32_t TAPI_Phone_Tone_Play_Unprot        (IFX_TAPI_DRV_CTX_t*, TAPI_CHANNEL *pChannel,
                                                IFX_int32_t nToneIndex, TAPI_TONE_DST dst);
IFX_int32_t TAPI_Phone_Tone_Stop               (IFX_TAPI_DRV_CTX_t*, TAPI_CHANNEL *pChannel, IFX_int32_t nToneIndex, TAPI_TONE_DST nDirection);
IFX_int32_t TAPI_DECT_Tone_Stop                (IFX_TAPI_DRV_CTX_t *pDrvCtx, TAPI_CHANNEL *pChannel);

IFX_int32_t TAPI_Phone_Tone_Ringback           (IFX_TAPI_DRV_CTX_t*, TAPI_CHANNEL *pChannel);
IFX_int32_t TAPI_Phone_Tone_Busy               (IFX_TAPI_DRV_CTX_t*, TAPI_CHANNEL *pChannel);
IFX_int32_t TAPI_Phone_Tone_Dial               (IFX_TAPI_DRV_CTX_t*, TAPI_CHANNEL *pChannel);

IFX_int32_t TAPI_Phone_Tone_Set_Level          (IFX_TAPI_DRV_CTX_t*, TAPI_CHANNEL *pChannel, IFX_TAPI_PREDEF_TONE_LEVEL_t const *pToneLevel);
IFX_int32_t TAPI_Phone_Tone_Get_State          (TAPI_CHANNEL *pChannel, IFX_uint32_t *pToneState);

IFX_int32_t TAPI_Phone_Add_SimpleTone          ( COMPLEX_TONE* toneCoefficients,
                                                IFX_TAPI_TONE_SIMPLE_t const *pSimpleTone);

IFX_int32_t TAPI_Phone_Add_ComposedTone        (COMPLEX_TONE* toneCoefficients,
                                                IFX_TAPI_TONE_COMPOSED_t const *pComposedTone);
IFX_int32_t TAPI_Phone_Tone_TableConf          (COMPLEX_TONE* toneCoefficients,
                                                IFX_TAPI_TONE_t const *pTone);

IFX_void_t  Tone_OnTimer                       (Timer_ID Timer, IFX_int32_t nArg);
IFX_int32_t TAPI_Phone_Tone_Set_On_Time        (TAPI_CHANNEL *pChannel, IFX_uint32_t nTime);
IFX_int32_t TAPI_Phone_Tone_Set_Off_Time       (TAPI_CHANNEL *pChannel, IFX_uint32_t nTime);
IFX_int32_t TAPI_Phone_Tone_Get_On_Time        (TAPI_CHANNEL *pChannel, IFX_uint32_t *pOnTime);
IFX_int32_t TAPI_Phone_Tone_Get_Off_Time       (TAPI_CHANNEL *pChannel, IFX_uint32_t *pOffTime);

IFX_int32_t TAPI_Phone_DetectToneStart         (IFX_TAPI_DRV_CTX_t*, TAPI_CHANNEL *pChannel, IFX_TAPI_TONE_CPTD_t const *signal);
IFX_int32_t TAPI_Phone_DetectToneStop          (IFX_TAPI_DRV_CTX_t*, TAPI_CHANNEL *pChannel);

/* PCM Services */
extern IFX_int32_t TAPI_Phone_PCM_IF_Set_Config (TAPI_DEV *pTAPIDev, IFX_TAPI_PCM_IF_CFG_t const *pPCMif);
extern IFX_int32_t TAPI_Phone_PCM_Set_Config    (TAPI_CHANNEL *pChannel, IFX_TAPI_PCM_CFG_t const *pPCMConfig);
extern IFX_int32_t TAPI_Phone_PCM_Get_Config    (TAPI_CHANNEL *pChannel, IFX_TAPI_PCM_CFG_t *pPCMConfig);
extern IFX_int32_t TAPI_Phone_PCM_Set_Activation(TAPI_CHANNEL *pChannel, IFX_uint32_t nMode);
extern IFX_int32_t TAPI_Phone_PCM_Get_Activation(TAPI_CHANNEL *pChannel, IFX_boolean_t *pbAct);

/* Miscellaneous Services */
IFX_int32_t TAPI_Phone_Get_Version             (IFX_char_t * version_string);
IFX_int32_t TAPI_Phone_Check_Version           (IFX_TAPI_VERSION_t const *vers);
IFX_uint32_t TAPI_Phone_Exception              (TAPI_CHANNEL *pChannel);
IFX_int32_t TAPI_Phone_Mask_Exception          (TAPI_CHANNEL *pChannel, IFX_uint32_t nException);
IFX_int32_t TAPI_Phone_Init                    (IFX_TAPI_DRV_CTX_t*, TAPI_CHANNEL *pChannel, IFX_TAPI_CH_INIT_t const *pInit);
#ifdef ENABLE_OBSOLETE_BITFIELD_EVENT_INTERFACE
IFX_int32_t TAPI_Phone_GetStatus               (TAPI_CHANNEL *pChannel, IFX_TAPI_CH_STATUS_t *status);
#endif /* ENABLE_OBSOLETE_BITFIELD_EVENT_INTERFACE */
IFX_void_t  TAPI_Gndkh_OnTimer                 (Timer_ID Timer, IFX_int32_t nArg);

/* Dial Services */
extern IFX_void_t  TAPI_Phone_Event_HookState  (TAPI_CHANNEL *pChannel, IFX_uint8_t bHookState);
extern IFX_int32_t TAPI_Phone_Validation_Time  (TAPI_CHANNEL *pChannel, IFX_TAPI_LINE_HOOK_VT_t const *pTime);
extern IFX_void_t  TAPI_Phone_Dial_OnTimer     (Timer_ID Timer, IFX_int32_t nArg);

/* Metering Services */
extern IFX_int32_t TAPI_Phone_Meter_Config     (TAPI_CHANNEL *pChannel, IFX_TAPI_METER_CFG_t const *pMeterConfig);
extern IFX_int32_t TAPI_Phone_Meter_Start      (TAPI_CHANNEL *pChannel);
extern IFX_int32_t TAPI_Phone_Meter_Stop       (TAPI_CHANNEL *pChannel);
extern IFX_void_t  TAPI_Phone_Meter_OnTimer    (Timer_ID Timer, IFX_int32_t nArg);

#ifdef TAPI_VOICE
/* Connection services */
extern IFX_int32_t TAPI_Data_Channel_Add       (TAPI_CHANNEL *pChannel, IFX_TAPI_MAP_DATA_t const *pMap);
extern IFX_int32_t TAPI_Data_Channel_Remove    (TAPI_CHANNEL *pChannel, IFX_TAPI_MAP_DATA_t const *pMap);
extern IFX_int32_t TAPI_Phone_Channel_Add      (TAPI_CHANNEL *pChannel, IFX_TAPI_MAP_PHONE_t const *pMap);
extern IFX_int32_t TAPI_Phone_Channel_Remove   (TAPI_CHANNEL *pChannel, IFX_TAPI_MAP_PHONE_t const *pMap);
extern IFX_int32_t TAPI_Phone_Get_Data_Channel (TAPI_CHANNEL *pChannel, IFX_uint8_t *pDataChannel);
extern IFX_int32_t TAPI_Data_Get_Phone_Channel (TAPI_CHANNEL *pChannel, IFX_uint8_t *pPhoneChannel);

extern IFX_int32_t TAPI_Phone_Fax_JB_Set       (TAPI_CHANNEL *pChannel, IFX_TAPI_JB_CFG_t *pJbConf);
#endif /* TAPI_VOICE */

/* FXO services */
IFX_int32_t TAPI_FXO_Ioctl                     (IFX_TAPI_DRV_CTX_t *pDrvCtx, TAPI_CHANNEL *pChannel, IFX_uint32_t cmd, IFX_uint32_t arg);

/* ======================================== */
/**  Event Handling                         */
/* ======================================== */

IFX_void_t TAPI_Phone_Event_GNKH               (TAPI_CHANNEL *pChannel);
IFX_void_t TAPI_Phone_Event_Device             (TAPI_CHANNEL *pChannel, IFX_uint32_t devEvt);
extern IFX_int32_t IFX_TAPI_Event_Dispatch_ProcessCtx (
                        IFX_TAPI_EXT_EVENT_PARAM_t *pParam);
#if 0
IFX_void_t TAPI_Phone_Event_Line               (TAPI_CHANNEL *pChannel, IFX_TAPI_LINE_STATUS_t stat);
#endif
#ifdef TAPI_FAX_T38
IFX_void_t TAPI_FaxT38_Event_Update            (TAPI_CHANNEL *pChannel,
                           IFX_uint8_t status, IFX_uint8_t error);
#endif /* TAPI_FAX_T38 */


/* ======================================== */
/**  Timer functions                        */
/* ======================================== */

Timer_ID          TAPI_Create_Timer      (TIMER_ENTRY pTimerEntry,
                                          IFX_int32_t nArgument);
IFX_boolean_t     TAPI_SetTime_Timer     (Timer_ID Timer, IFX_uint32_t nTime,
                         IFX_boolean_t bPeriodically, IFX_boolean_t bRestart);
IFX_boolean_t     TAPI_Delete_Timer      (Timer_ID Timer);
IFX_boolean_t     TAPI_Stop_Timer        (Timer_ID Timer);

/* ======================================== */
/**  Operating system specific functions    */
/* ======================================== */

/* Function Declaration*/
extern IFX_return_t IFX_TAPI_EventDispatcher_Init  (TAPI_CHANNEL *);
extern IFX_int32_t  IFX_TAPI_EventDispatcher_Exit  (TAPI_CHANNEL * pChannel);
extern IFX_int32_t  IFX_TAPI_EventFifoGet          (TAPI_CHANNEL *, IFX_TAPI_EVENT_t *);
extern IFX_uint8_t  IFX_TAPI_EventFifoEmpty        (TAPI_CHANNEL *);

/* Global function prototypes */
extern IFX_int32_t TAPI_Init_Dev (IFX_TAPI_DRV_CTX_t *pDrvCtx,
                                IFX_uint8_t devId);
extern IFX_return_t TAPI_Prepare_Dev (TAPI_DEV* pTapiDev,
                                IFX_TAPI_DRV_CTX_t *pDrvCtx);

extern IFX_return_t TAPI_Allocate_Dev_Structure  (IFX_TAPI_DRV_CTX_t *pDrvCtx,
                                                  IFX_uint32_t nMaxDevs);
extern IFX_return_t TAPI_Allocate_Ch_Structure   (TAPI_DEV *pTapiDev,
                                                  IFX_uint32_t nMaxChannels);
extern IFX_return_t TAPI_Prepare_Ch              (TAPI_DEV* pTapiDev,
                                                  IFX_TAPI_DRV_CTX_t *pDrvCtx,
                                                  IFX_uint32_t dev_num);
extern IFX_int32_t  TAPI_DeallocateCh            (TAPI_DEV *pTapiDev);
extern IFX_return_t TAPI_Phone_Set_Event_Disable (TAPI_CHANNEL *pChannel,
                                                  IFX_TAPI_EVENT_t *pDisEvnet,
                                                  IFX_uint32_t value);
extern IFX_int32_t  TAPI_Phone_GetEvent          (TAPI_DEV *pTapiDev,
                                                  IFX_TAPI_EVENT_t *pEvent);
extern IFX_return_t TAPI_Phone_CidRx_Status      (TAPI_CHANNEL *pChannel,
                                  IFX_TAPI_CID_RX_STATUS_t *pCidRxStatus);
extern IFX_int32_t  TAPI_Phone_Set_LineType      (IFX_TAPI_DRV_CTX_t *,
                                               TAPI_CHANNEL *pChannel,
                                               IFX_TAPI_LINE_TYPE_CFG_t *pCfg);
extern IFX_return_t TAPI_EVENT_PKT_EV_Generate   (TAPI_CHANNEL *pChannel,
                                  IFX_TAPI_PKT_EV_GENERATE_t *pPacketEvent);
extern IFX_int32_t TAPI_Phone_Dtmf_RxCoeff_Cfg (IFX_TAPI_DRV_CTX_t *pDrvCtx,
                                                TAPI_CHANNEL *pChannel,
                                                IFX_boolean_t bRW,
                                                IFX_TAPI_DTMF_RX_CFG_t *pDtmfRxCoeff);
extern IFX_boolean_t ptr_chk(IFX_void_t *ptr, const IFX_char_t *pPtrName);

extern IFX_return_t IFX_TAPI_Register_LL_Drv (IFX_TAPI_DRV_CTX_t* pLLDrvCtx);
extern IFX_return_t IFX_TAPI_Is_Device_Registered(IFX_int32_t nMajor);

extern IFX_return_t TAPI_OS_RegisterLLDrv (IFX_TAPI_DRV_CTX_t* pLLDrvCtx,
                                    IFX_TAPI_HL_DRV_CTX_t* pHLDrvCtx);

extern IFX_return_t TAPI_OS_UnregisterLLDrv (IFX_TAPI_DRV_CTX_t* pLLDrvCtx,
                                    IFX_TAPI_HL_DRV_CTX_t* pHLDrvCtx);
extern IFX_TAPI_DRV_CTX_t* IFX_TAPI_Get_Device_Driver_Context
                                   (IFX_int32_t Major);
extern IFX_int32_t IFX_TAPI_PKT_RTP_PT_Defaults (TAPI_CHANNEL *pChannel);

#endif  /* DRV_TAPI_H */
