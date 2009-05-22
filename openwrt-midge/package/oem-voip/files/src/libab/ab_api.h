#ifndef __AB_API_H__
#define __AB_API_H__

/** Maximum channels on the boards */
#define CHANS_MAX 32

typedef enum ab_dev_type_e ab_dev_type_t;
typedef enum cod_type_e cod_type_t;
typedef struct codec_s codec_t;
typedef struct wlec_s wlec_t;
typedef struct rtp_session_prms_s rtp_session_prms_t;
typedef struct ab_chan_s ab_chan_t;
typedef struct ab_dev_s ab_dev_t;
typedef struct ab_s ab_t;
typedef struct ab_fw_s ab_fw_t;
typedef struct ab_dev_event_s ab_dev_event_t;

enum cod_type_e {
	cod_type_NONE,
	cod_type_ALAW,
	cod_type_G729,
	cod_type_G729E,
	cod_type_ILBC_133,
	cod_type_G723,
	cod_type_G726_16,
	cod_type_G726_24,
	cod_type_G726_32,
	cod_type_G726_40,
};

enum cod_pkt_size_e {
	cod_pkt_size_2_5,
	cod_pkt_size_5,
	cod_pkt_size_5_5,
	cod_pkt_size_10,
	cod_pkt_size_11,
	cod_pkt_size_20,
	cod_pkt_size_30,
	cod_pkt_size_40,
	cod_pkt_size_50,
	cod_pkt_size_60,
};

enum bitpack_e {
	bitpack_RTP,
	bitpack_AAL2,
};

struct codec_s {
	enum cod_type_e 	type;
	enum cod_pkt_size_e pkt_size;
	enum bitpack_e bpack;
	int user_payload;			/**< User preset to sdp payload. */
	int sdp_selected_payload;	/**< Selected in sdp session payload. */
};

enum wlec_mode_e {
	wlec_mode_UNDEF,
	wlec_mode_OFF,
	wlec_mode_NE,
	wlec_mode_NFE,
};
enum wlec_nlp_e {
	wlec_nlp_ON,
	wlec_nlp_OFF,
};
enum wlec_window_size_e {
	wlec_window_size_4 = 4,
	wlec_window_size_6 = 6,
	wlec_window_size_8 = 8,
	wlec_window_size_16 = 16,
};
struct wlec_s {
	enum wlec_mode_e mode;
	enum wlec_nlp_e nlp;
	enum wlec_window_size_e ne_nb;
	enum wlec_window_size_e fe_nb;
	enum wlec_window_size_e ne_wb;
};
enum vad_cfg_e {
	vad_cfg_OFF,
	vad_cfg_ON,
	vad_cfg_G711,
	vad_cfg_CNG_only,
	vad_cfg_SC_only
};
	
/** RTP session configuration parameters */
struct rtp_session_prms_s {
	struct cod_volume_s {
		int enc_dB;
		int dec_dB;
	} cod_volume; /**< Coder volume in both directions */
	enum vad_cfg_e VAD_cfg; /**< Voice Activity Detector configuration */
	unsigned char HPF_is_ON; /**< High Pass Filter is ON? */
};

enum ab_dev_type_e {
	ab_dev_type_FXO,   /**< Device type is FXO */
	ab_dev_type_FXS,   /**< Device type is FXS */
	ab_dev_type_TF     /**< Device type is Tonal Frequency */
};

enum ab_chan_tone_e {
	ab_chan_tone_MUTE, /**< Mute any tone */
	ab_chan_tone_DIAL,   /**< Play dial tone */
	ab_chan_tone_BUSY,   /**< Play busy tone */
	ab_chan_tone_RINGBACK   /**< Play ringback tone */
};

enum ab_chan_ring_e {
	ab_chan_ring_MUTE, /**< Mute the ring */
	ab_chan_ring_RINGING   /**< Make ring */
};

enum ab_chan_hook_e {
	ab_chan_hook_ONHOOK, /**< onhook state */
	ab_chan_hook_OFFHOOK /**< offhook state */
};

enum ab_chan_linefeed_e {
	ab_chan_linefeed_DISABLED, /**< Set linefeed to disabled */
	ab_chan_linefeed_STANDBY, /**< Set linefeed to standby */
	ab_chan_linefeed_ACTIVE /**< Set linefeed to active */
};

enum ab_dev_event_e {
	ab_dev_event_NONE, /**< No event. */
	ab_dev_event_UNCATCHED, /**< Unknown event. */
	ab_dev_event_FXO_RINGING, /**< Ring on FXO. */
	ab_dev_event_FXS_DIGIT_TONE, /**< Dial a digit on FXS in tone mode. */
	ab_dev_event_FXS_DIGIT_PULSE, /**< Dial a digit on FXO in pulse mode. */
	ab_dev_event_FXS_ONHOOK, /**< Onhook on FXS. */
	ab_dev_event_FXS_OFFHOOK, /**< Offhook on FXS. */
	ab_dev_event_FM_CED, /**< CED and CEDEND FAX events. */
	ab_dev_event_COD, /**< Coder event. */
	ab_dev_event_TONE, /**< Tone generator event. */
};

struct ab_chan_status_s {
	enum ab_chan_tone_e	tone;	/**< tone state */
	enum ab_chan_ring_e	ring;	/**< ring state */
	enum ab_chan_hook_e	hook;	/**< hoot state */
	enum ab_chan_linefeed_e	linefeed; /**< linefeed state*/
};

struct ab_dev_event_s {
	enum ab_dev_event_e id; /**< Event identificator */
	unsigned char ch;	/**< Ret Channel of event */
	unsigned char more;	/**< is there more events */
	long data;		/**< Event specific data */
};

struct ab_chan_s {
	unsigned int idx;   /**< Channel index on device (from 1) */
	unsigned char abs_idx; /**< Channel index on boards (from 0) */
	ab_dev_t * parent;  /**< device that channel belongs */
	int rtp_fd;         /**< Channel file descriptor */
	struct ab_chan_status_s status;  /**< Channel status info */
	struct rtp_session_prms_s rtp_cfg; /**< Channel RTP configuration */
	void * ctx; /**< Channel context pointer (for user app) */
};

struct ab_dev_s {
	unsigned int idx;	/**< Device index on boards (from 1) */
	ab_dev_type_t type;	/**< Device type */
	ab_t * parent;		/**< Parent board pointer */
	int cfg_fd;             /**< Device config file descriptor */
	void * ctx; /**< Device context (for user app) */
};

struct ab_s {
	unsigned int devs_num;	/**< Devices number on the boards */
	ab_dev_t * devs;	/**< Devices of the boards */
	unsigned int chans_num;	/**< Channels number on the boards */
	ab_chan_t * chans;	/**< Channels of the boards according to idx */
	ab_chan_t * pchans[CHANS_MAX]; /**< Pointers to channels according 
									to abs_idx*/
	unsigned int chans_per_dev;/**< Channels number per device */
};

/* ERROR HANDLING */
/** No errors */
#define AB_ERR_NO_ERR 		0
/** In most cases ioctl error */
#define AB_ERR_UNKNOWN		1
/** Not enough memory */
#define AB_ERR_NO_MEM		2
/** error on file operation */
#define AB_ERR_NO_FILE		3
/** Bad parameter set */
#define AB_ERR_BAD_PARAM 	4

/** global error characteristic string length */
#define ERR_STR_LENGTH		256

/** global error index */
extern int ab_g_err_idx;
/** global error characteristic string */
extern char ab_g_err_str[ERR_STR_LENGTH];
/** global error extra value (using in some cases) */
extern int ab_g_err_extra_value;

/** @defgroup AB_BASIC ACTIONS Basic libab interface
	Basic interface.
  @{ */
/** Create the ab_t object. */
ab_t* ab_create (void);
/** Destroy the ab_t object. */
void ab_destroy (ab_t ** ab);
/** @} */


/** @defgroup AB_RINGS_TONES Ringing and toneplay libab interface.
	Rings and Tones.
  @{ */
/** Play ring or mute it */
int ab_FXS_line_ring( ab_chan_t * const chan, enum ab_chan_ring_e ring );
/** Play tone or mute it */
int ab_FXS_line_tone( ab_chan_t * const chan, enum ab_chan_tone_e tone );
/** Change linefeed mode */
int ab_FXS_line_feed( ab_chan_t * const chan, enum ab_chan_linefeed_e feed );
/** Onhook or offhook on FXO line */
int ab_FXO_line_hook( ab_chan_t * const chan, enum ab_chan_hook_e hook );
/** Dial a digit on FXO line */
int ab_FXO_line_digit( 
		ab_chan_t * const chan, 
		char const data_length, char const * const data,
		char const nInterDigitTime, char const nDigitPlayTime, 
		char const pulseMode);
/** Play DTMF or busy/dial/ringing to the network connection (rtp-flow) */
int ab_FXS_netlo_play( ab_chan_t * const chan, char tone, char local );
/** @} */

/** @defgroup AB_EVENTS Events libab interface.
	Events.
  @{ */
/** Get the events occures on given device */
int ab_dev_event_get( 
		ab_dev_t * const dev, 
		ab_dev_event_t * const evt, 
		unsigned char * const chan_available );
/** @} */


/** @defgroup AB_MEDIA Media libab interface.
	Codecs RTP-frames etc.
  @{ */
/** Tune rtp parameters for fax transmittion */
int ab_chan_fax_pass_through_start( ab_chan_t * const chan );
/** Tune rtp parameters on selected chan */
int ab_chan_media_rtp_tune( ab_chan_t * const chan, codec_t const * const cod,
		codec_t const * const fcod);
/** Tune wlec parameters on selected chan */
int ab_chan_media_wlec_tune (ab_chan_t * const chan, wlec_t const * const wp);
/** Switch on/off media on selected chan */
int ab_chan_media_switch( ab_chan_t * const chan,
		unsigned char const enc_on, unsigned char const dec_on );
/** @} */

#endif /* __AB_API_H__ */

