/* Unique interface to reports all events as messages.

NOTE:
* It reports only one event at time
* The same struct can be used also for enable/disable events
* In order to avoid queue overflow, driver reports only one time the error condition.
* If the driver is able to recognize the end of the error condition, this should be also reported */
#ifndef _DRV_TAPIEVENT_H
#define _DRV_TAPIEVENT_H

/** \defgroup event handling
   @{ */
/* =============================== */
/* Macros                          */
/* =============================== */

/* First two bit stands for direction */
#define IFX_TAPI_EVENT_DIR_LOCAL        0x80000000
#define IFX_TAPI_EVENT_DIR_NETWORK      0x40000000
#define IFX_TAPI_EVENT_MORE_EVENT       0x20000000

/* Disable or Enable event. */
#define IFX_EVENT_DISABLE 1
#define IFX_EVENT_ENABLE 0

/* ============================= */
/* Event Fifo structure    */
/* ============================= */
/** This structure is used by event fifo. */
typedef struct _TAPI_EVENT_FIFO
{
   FIFO_ID   *pEventFifo;
   BUFFERPOOL   *pBufHandler;
}IFX_TAPI_EventFifo_t;

/* These structures is used by disabling event. */
typedef struct
{
   IFX_uint32_t reserved:32;
}IFX_TAPI_EVENT_IO_GENERAL_BITS_t;

typedef union
{
   IFX_TAPI_EVENT_IO_GENERAL_BITS_t bits;
   IFX_uint32_t status;
}IFX_TAPI_EVENT_IO_GEN_DIS_t;

typedef struct
{
   IFX_uint32_t reserved:32;
}IFX_TAPI_EVENT_IO_INTERRUPT_BITS_t;

typedef union
{
   IFX_TAPI_EVENT_IO_INTERRUPT_BITS_t bits;
   IFX_uint32_t status;
}IFX_TAPI_EVENT_IO_INT_DIS_t;

typedef struct
{
   IFX_uint32_t reserved:26;
   IFX_uint32_t flash:1;
   IFX_uint32_t offhook:1;
   IFX_uint32_t onhook:1;
   IFX_uint32_t ringing_end:1;
   IFX_uint32_t ringburst_end:1;
   IFX_uint32_t ring:1;
}IFX_TAPI_EVENT_FXS_BITS_t;

typedef union
{
   IFX_TAPI_EVENT_FXS_BITS_t bits;
   IFX_uint32_t status;
}IFX_TAPI_EVENT_FXS_DIS_t;

typedef struct
{
   IFX_uint32_t reserved:27;
   IFX_uint32_t bat_feeded:1;
   IFX_uint32_t bat_dropped:1;
   IFX_uint32_t polarity:1;
   IFX_uint32_t ring_start:1;
   IFX_uint32_t ring_stop:1;
}IFX_TAPI_EVENT_FXO_BITS_t;

typedef union
{
   IFX_TAPI_EVENT_FXO_BITS_t bits;
   IFX_uint32_t status;
}IFX_TAPI_EVENT_FXO_DIS_t;

typedef struct
{
   IFX_uint32_t reserved:29;
   IFX_uint32_t gr909_rdy:1;
}IFX_TAPI_EVENT_LT_BITS_t;


typedef union
{
   IFX_TAPI_EVENT_LT_BITS_t bits;
   IFX_uint32_t status;
}IFX_TAPI_EVENT_LT_DIS_t;


typedef struct
{
   IFX_uint32_t reserved:31;
   IFX_uint32_t digit:1;
}IFX_TAPI_EVENT_PULSE_BITS_t;

typedef union
{
   IFX_TAPI_EVENT_PULSE_BITS_t bits;
   IFX_uint32_t status;
}IFX_TAPI_EVENT_PULSE_DIS_t;

typedef struct
{
   IFX_uint32_t reserved:30;
   IFX_uint32_t digit_network:1;
   IFX_uint32_t digit_local:1;
}IFX_TAPI_EVENT_DTMF_BITS_t;

typedef union
{
   IFX_TAPI_EVENT_DTMF_BITS_t bits;
   IFX_uint32_t status;
}IFX_TAPI_EVENT_DTMF_DIS_t;

typedef struct
{
   IFX_uint32_t reserved:19;
   IFX_uint32_t rx_error2:1;
   IFX_uint32_t rx_error1:1;
   IFX_uint32_t rx_err_read:1;
   IFX_uint32_t rx_cd:1;
   IFX_uint32_t rx_end:1;
   IFX_uint32_t tx_error2:1;
   IFX_uint32_t tx_ringcad_err:1;
   IFX_uint32_t tx_noack_err:1;
   IFX_uint32_t tx_info_end:1;
   IFX_uint32_t tx_info_start:1;
   IFX_uint32_t tx_seq_end:1;
   IFX_uint32_t tx_seq_start:1;
}IFX_TAPI_EVENT_CID_BITS_t;

typedef union
{
   IFX_TAPI_EVENT_CID_BITS_t bits;
   IFX_uint32_t status;
}IFX_TAPI_EVENT_CID_DIS_t;

typedef struct
{
   IFX_uint32_t reserved:29;
   IFX_uint32_t end_network:1;
   IFX_uint32_t end_local:1;
   IFX_uint32_t busy:1;
}IFX_TAPI_EVENT_TONE_GEN_BITS_t;

typedef union
{
   IFX_TAPI_EVENT_TONE_GEN_BITS_t bits;
   IFX_uint32_t status;
}IFX_TAPI_EVENT_TONE_GEN_DIS_t;

typedef struct
{
   IFX_uint32_t reserved:29;
   IFX_uint32_t receive:1;
   IFX_uint32_t transmit:1;
   IFX_uint32_t cpt:1;
}IFX_TAPI_EVENT_TONE_DET_BITS_t;

typedef union
{
   IFX_TAPI_EVENT_TONE_DET_BITS_t bits;
   IFX_uint32_t status;
}IFX_TAPI_EVENT_TONE_DET_DIS_t;

typedef struct
{
   IFX_uint32_t reserved:30;
   IFX_uint32_t v21h_network:1;
   IFX_uint32_t v21h_local:1;
   IFX_uint32_t cas_network:1;
   IFX_uint32_t cas_local:1;
   IFX_uint32_t hold_network:1;
   IFX_uint32_t hold_local:1;
   IFX_uint32_t v8bis_network:1;
   IFX_uint32_t v8bis_local:1;
   IFX_uint32_t v32ac_network:1;
   IFX_uint32_t v32ac_local:1;
   IFX_uint32_t v22orbell_network:1;
   IFX_uint32_t v22orbell_local:1;
   IFX_uint32_t v22_network:1;
   IFX_uint32_t v22_local:1;
   IFX_uint32_t bell_network:1;
   IFX_uint32_t bell_local:1;
   IFX_uint32_t v27_network:1;
   IFX_uint32_t v27_local:1;
   IFX_uint32_t v18a_network:1;
   IFX_uint32_t v18a_local:1;
   IFX_uint32_t v21l_network:1;
   IFX_uint32_t v21l_local:1;
   IFX_uint32_t cngmod_network:1;
   IFX_uint32_t cngmod_local:1;
   IFX_uint32_t cngfax_network:1;
   IFX_uint32_t cngfax_local:1;
   IFX_uint32_t am_network:1;
   IFX_uint32_t am_local:1;
   IFX_uint32_t pr_network:1;
   IFX_uint32_t pr_local:1;
   IFX_uint32_t ced_network:1;
   IFX_uint32_t ced_local:1;
   IFX_uint32_t dis_network:1;
   IFX_uint32_t dis_local:1;
}IFX_TAPI_EVENT_FAX_SIG_BITS_t;

typedef union
{
   IFX_TAPI_EVENT_FAX_SIG_BITS_t bits;
   IFX_uint32_t status[2];
}IFX_TAPI_EVENT_FAX_SIG_DIS_t;

typedef struct
{
   IFX_uint32_t reserved:29;
   IFX_uint32_t room_noise:1;
   IFX_uint32_t room_silence:1;
   IFX_uint32_t dec_chg:1;
}IFX_TAPI_EVENT_CODER_BITS_t;

typedef union
{
   IFX_TAPI_EVENT_CODER_BITS_t bits;
   IFX_uint32_t status;
}IFX_TAPI_EVENT_CODER_DIS_t;

typedef struct
{
   IFX_uint32_t reserved:32;
}IFX_TAPI_EVENT_RTP_BITS_t;

typedef union
{
   IFX_TAPI_EVENT_RTP_BITS_t bits;
   IFX_uint32_t status;
}IFX_TAPI_EVENT_RTP_DIS_t;

typedef struct
{
   IFX_uint32_t reserved:32;
}IFX_TAPI_EVENT_AAL_BITS_t;

typedef union
{
   IFX_TAPI_EVENT_AAL_BITS_t bits;
   IFX_uint32_t status;
}IFX_TAPI_EVENT_AAL_DIS_t;

typedef struct
{
   IFX_uint32_t reserved:30;
   IFX_uint32_t event:1;
}IFX_TAPI_EVENT_RFC2833_BITS_t;

typedef union
{
   IFX_TAPI_EVENT_RFC2833_BITS_t bits;
   IFX_uint32_t status;
}IFX_TAPI_EVENT_RFC2833_DIS_t;

typedef struct
{
   IFX_uint32_t reserved:27;
   IFX_uint32_t error_setup:1;
   IFX_uint32_t error_data:1;
   IFX_uint32_t error_write:1;
   IFX_uint32_t error_read:1;
   IFX_uint32_t error_ovld:1;
   IFX_uint32_t error_gen:1;
}IFX_TAPI_EVENT_T38_BITS_t;

typedef union
{
   IFX_TAPI_EVENT_T38_BITS_t bits;
   IFX_uint32_t status;
}IFX_TAPI_EVENT_T38_DIS_t;

typedef struct
{
   IFX_uint32_t reserved:32;
}IFX_TAPI_EVENT_JB_BITS_t;

typedef union
{
   IFX_TAPI_EVENT_JB_BITS_t bits;
   IFX_uint32_t status;
}IFX_TAPI_EVENT_JB_DIS_t;

typedef struct
{
   IFX_uint32_t reserved:32;
}IFX_TAPI_EVENT_DOWNLOAD_BITS_t;

typedef union
{
   IFX_TAPI_EVENT_DOWNLOAD_BITS_t bits;
   IFX_uint32_t status;
}IFX_TAPI_EVENT_DOWNLOAD_DIS_t;

typedef struct
{
   IFX_uint32_t reserved:32;
}IFX_TAPI_EVENT_INFORMATION_BITS_t;

typedef union
{
   IFX_TAPI_EVENT_INFORMATION_BITS_t bits;
   IFX_uint32_t status;
}IFX_TAPI_EVENT_INFO_DIS_t;

typedef struct
{
   IFX_uint32_t reserved:32;
}IFX_TAPI_EVENT_DEBUG_BITS_t;

typedef union
{
   IFX_TAPI_EVENT_DEBUG_BITS_t bits;
   IFX_uint32_t status;
}IFX_TAPI_EVENT_DEBUG_DIS_t;

typedef struct
{
   IFX_uint32_t reserved:31;
   IFX_uint32_t alive:1;
}IFX_TAPI_EVENT_LL_DRIVER_BITS_t;

typedef union
{
   IFX_TAPI_EVENT_LL_DRIVER_BITS_t bits;
   IFX_uint32_t status;
}IFX_TAPI_EVENT_LL_DRIVER_DIS_t;

typedef struct
{
   IFX_uint32_t reserved:32;
}IFX_TAPI_EVENT_FAULT_GENERAL_BITS_t;

typedef union
{
   IFX_TAPI_EVENT_FAULT_GENERAL_BITS_t bits;
   IFX_uint32_t status;
}IFX_TAPI_EVENT_FAULT_GEN_DIS_t;

typedef struct
{
   IFX_uint32_t reserved:27;
   IFX_uint32_t overtemp:1;
   IFX_uint32_t gk_high:1;
   IFX_uint32_t gk_low:1;
   IFX_uint32_t gk_neg:1;
   IFX_uint32_t gk_pos:1;
}IFX_TAPI_EVENT_FAULT_LINE_BITS_t;

typedef union
{
   IFX_TAPI_EVENT_FAULT_LINE_BITS_t bits;
   IFX_uint32_t status;
}IFX_TAPI_EVENT_FAULT_LINE_DIS_t;

typedef struct
{
   IFX_uint32_t reserved:28;
   IFX_uint32_t spi_access:1;
   IFX_uint32_t clock_fail:1;
   IFX_uint32_t clock_fail_end:1;
   IFX_uint32_t hw_fault:1;
}IFX_TAPI_EVENT_FAULT_HW_BITS_t;

typedef union
{
   IFX_TAPI_EVENT_FAULT_HW_BITS_t bits;
   IFX_uint32_t status;
}IFX_TAPI_EVENT_FAULT_HW_DIS_t;

typedef struct
{
   IFX_uint32_t reserved:32;
}IFX_TAPI_EVENT_FAULT_FW_BITS_t;

typedef union
{
   IFX_TAPI_EVENT_FAULT_FW_BITS_t bits;
   IFX_uint32_t status;
}IFX_TAPI_EVENT_FAULT_FW_DIS_t;

typedef struct
{
   IFX_uint32_t reserved:32;
}IFX_TAPI_EVENT_FAULT_SW_BITS_t;

typedef union
{
   IFX_TAPI_EVENT_FAULT_SW_BITS_t bits;
   IFX_uint32_t status;
}IFX_TAPI_EVENT_FAULT_SW_DIS_t;


typedef struct
{
   IFX_TAPI_EVENT_IO_GEN_DIS_t io_general;
   IFX_TAPI_EVENT_IO_INT_DIS_t io_interrupt;
   IFX_TAPI_EVENT_FXS_DIS_t fxs;
   IFX_TAPI_EVENT_FXO_DIS_t fxo;
   IFX_TAPI_EVENT_LT_DIS_t lt;
   IFX_TAPI_EVENT_PULSE_DIS_t pulse;
   IFX_TAPI_EVENT_DTMF_DIS_t dtmf;
   IFX_TAPI_EVENT_CID_DIS_t cid;
   IFX_TAPI_EVENT_TONE_GEN_DIS_t tone_gen;
   IFX_TAPI_EVENT_TONE_DET_DIS_t tone_det;
   IFX_TAPI_EVENT_FAX_SIG_DIS_t fax_sig;
   IFX_TAPI_EVENT_CODER_DIS_t coder;
   IFX_TAPI_EVENT_RTP_DIS_t rtp;
   IFX_TAPI_EVENT_AAL_DIS_t aal;
   IFX_TAPI_EVENT_RFC2833_DIS_t rfc2833;
   IFX_TAPI_EVENT_T38_DIS_t t38;
   IFX_TAPI_EVENT_JB_DIS_t jb;
   IFX_TAPI_EVENT_DOWNLOAD_DIS_t download;
   IFX_TAPI_EVENT_INFO_DIS_t information;
   IFX_TAPI_EVENT_DEBUG_DIS_t debug;
   IFX_TAPI_EVENT_LL_DRIVER_DIS_t ll_driver;
   IFX_TAPI_EVENT_FAULT_GEN_DIS_t fault_general;
   IFX_TAPI_EVENT_FAULT_LINE_DIS_t fault_line;
   IFX_TAPI_EVENT_FAULT_HW_DIS_t fault_hw;
   IFX_TAPI_EVENT_FAULT_FW_DIS_t fault_fw;
   IFX_TAPI_EVENT_FAULT_SW_DIS_t fault_sw;
}IFX_TAPI_EVENT_DISABLE_t;

typedef struct
{
   /* Tapi event fifo -- High proity event*/
   FIFO_ID              *pTapiEventFifoHi;
   /* Tapi event fifo -- Low proity event*/
   FIFO_ID              *pTapiEventFifoLo;
   /* Tapi event enable/disable. */
   IFX_TAPI_EVENT_DISABLE_t eventDisable;
   /* Fifo concurent access protection mutex.
   PS: Avoid nested locking of this mutex. It can lead to a deadlock */
   IFXOS_mutex_t             fifoAcc;
}IFX_TAPI_EVENT_HANDLER_t;

#endif /* _DRV_TAPIEVENT_H */

/*@}*/
