#ifndef __AB_API_H__
#define __AB_API_H__

typedef enum ab_dev_type_e 	ab_dev_type_t;
typedef struct ab_chan_s 	ab_chan_t;
typedef struct ab_dev_s 	ab_dev_t;
typedef struct ab_s 		ab_t;
typedef struct ab_fw_s 		ab_fw_t;
typedef struct ab_dev_event_s 	ab_dev_event_t;

#include "../svd.h"

#ifndef AB_CMAGIC_T
	#define AB_CMAGIC_T void
#endif
typedef AB_CMAGIC_T ab_cmagic_t;
	
#ifndef AB_DMAGIC_T
	#define AB_DMAGIC_T void
#endif
typedef AB_DMAGIC_T ab_dmagic_t;

enum ab_dev_type_e {
	ab_dev_type_FXO,   /**< Device type is FXO */
	ab_dev_type_FXS    /**< Device type is FXS */
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
	ab_chan_hook_ONHOOK, /**< Do onhook */
	ab_chan_hook_OFFHOOK /**< Do offhook */
	};

enum ab_chan_linefeed_e {
	ab_chan_linefeed_DISABLED, /**< Set linefeed to disabled */
	ab_chan_linefeed_STANDBY, /**< Set linefeed to standby */
	ab_chan_linefeed_ACTIVE /**< Set linefeed to active */
	};

enum ab_dev_event_e {
	ab_dev_event_NONE, /**< No event */
	ab_dev_event_UNCATCHED, /**< Unknown event */
	ab_dev_event_FXO_RINGING, /**< Ring on FXO */
	ab_dev_event_FXS_DIGIT_TONE, /**< Dial a digit on FXS in tone mode */
	ab_dev_event_FXS_DIGIT_PULSE, /**< Dial a digit on FXO in pulse mode */
	ab_dev_event_FXS_ONHOOK, /**< Onhook on FXS */
	ab_dev_event_FXS_OFFHOOK /**< Offhook on FXS */
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
	unsigned char abs_idx; /**< Channel index on boards (from 1) */
	ab_dev_t * parent;  /**< device that channel belongs */
	int rtp_fd;         /**< Channel file descriptor */
	struct ab_chan_status_s status;  /**< Channel status info */
	ab_cmagic_t * data; /**< Channel magic data (for user app) */
	/* for internal purposes */
	int err;	/**< Last error on this channel index */
	char const * err_s;/**< Last error on this channel message string */
	};

struct ab_dev_s {
	unsigned int idx;	/**< Device index on board (from 1) */
	ab_dev_type_t type;	/**< Device type */
	ab_t * parent;		/**< Parent board pointer */
	int cfg_fd;             /**< Device config file descriptor */
	ab_dmagic_t * data; /**< Device magic data (for user app) */
	/* for internal purposes */
	int err;	/**< Last error on this device index */
	char const * err_s;	/**< Last error on this device message string */
	};

struct ab_s {
	char * name;		/**< Board name */
	unsigned int devs_num;	/**< Devices number on the board */
	ab_dev_t * devs;	/**< Devices of the board */
	unsigned int chans_num;	/**< Channels number on the board */
	ab_chan_t * chans;	/**< Channels of the board */
	unsigned int chans_per_dev;/**< Channels number per device */
	/* for internal purposes */
	int err;	/**< Last error on this board index */
	char const * err_s;	/**< Last error on this board message string */
	};

/* ERROR HANDLING */
/** no error happens */
#define AB_ERR_NO_ERR 		0
/** In most cases ioctl error */
#define AB_ERR_UNKNOWN		1
/** Not enough memory */
#define AB_ERR_NO_MEM		2
/** error on file operation */
#define AB_ERR_NO_FILE		3
/** Bad parameter set */
#define AB_ERR_BAD_PARAM 	4

/** global error index */
extern int ab_g_err_idx;
/** global error characteristic string */
extern char const * ab_g_err_str;

/** get error index from object or global */
#define ab_err_get_idx(objp) \
	(objp) ? (objp)->err : ab_g_err_idx
/** get error string from object or global */
#define ab_err_get_str(objp) \
	(objp) ? (objp)->err_s : ab_g_err_str
/** get global error index (last for all objects) */
#define ab_err_get_idx_last	ab_g_err_idx
/** get global error string (last for all objects) */
#define ab_err_get_str_last	ab_g_err_str

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
		char const nInterDigitTime, char const nDigitPlayTime );
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
/** Activate media on selected chan */
int ab_chan_media_activate( ab_chan_t * const chan );
/** De-Activate media on selected chan */
int ab_chan_media_deactivate( ab_chan_t * const chan );
/** @} */

#endif /* __AB_API_H__ */

