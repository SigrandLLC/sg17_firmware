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
	//ab_dev_type_UNDEFINED,   /**< Device type is not defined yet */
	ab_dev_type_FXO,   /**< Device type is FXO */
	ab_dev_type_FXS    /**< Device type is FXS */
	};

enum ab_chan_tone_e {
	ab_chan_tone_MUTE, /**< */
	ab_chan_tone_DIAL,   /**< */
	ab_chan_tone_BUSY,   /**< */
	ab_chan_tone_RINGBACK   /**< */
	};

enum ab_chan_ring_e {
	ab_chan_ring_MUTE, /**< */
	ab_chan_ring_RINGING   /**< */
	};

enum ab_chan_hook_e {
	ab_chan_hook_ONHOOK, /**< */
	ab_chan_hook_OFFHOOK /**< */
	};

enum ab_chan_linefeed_e {
	ab_chan_linefeed_DISABLED, /**< */
	ab_chan_linefeed_STANDBY, /**< */
	ab_chan_linefeed_ACTIVE /**< */
	};

enum ab_dev_event_e {
	ab_dev_event_NONE, /**< */
	ab_dev_event_UNCATCHED, /**< */
	ab_dev_event_FXO_RINGING, /**< */
	ab_dev_event_FXS_DIGIT_TONE, /**< */
	ab_dev_event_FXS_DIGIT_PULSE, /**< */
	ab_dev_event_FXS_ONHOOK, /**< */
	ab_dev_event_FXS_OFFHOOK /**< */
	};

struct ab_chan_status_s {
	enum ab_chan_tone_e	tone;	/**< */
	enum ab_chan_ring_e	ring;	/**< */
	enum ab_chan_hook_e	hook;	/**< */
	enum ab_chan_linefeed_e	linefeed; /**< */
	};

struct ab_dev_event_s {
	enum ab_dev_event_e id;
	unsigned char ch;
	unsigned char more;
	long data;
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

#define AB_DEFAULT_CFG_PATH "/etc/sgatab.conf"

/* ERROR HANDLING */
#define AB_ERR_NO_ERR 		0
#define AB_ERR_UNKNOWN		1
#define AB_ERR_NO_MEM		2
#define AB_ERR_NO_FILE		3
#define AB_ERR_BAD_PARAM 	4

extern int ab_g_err_idx;
extern char const * ab_g_err_str;

#define ab_err_get_idx(objp) \
	(objp) ? (objp)->err : ab_g_err_idx
#define ab_err_get_str(objp) \
	(objp) ? (objp)->err_s : ab_g_err_str

#define ab_err_get_idx_last	ab_g_err_idx
#define ab_err_get_str_last	ab_g_err_str

/** @defgroup AB_BASIC ACTIONS Basic libab interface
	Basic interface.
  @{ */

/**
	Create the ab_t object. 
\return
	Pointer to created object or NULL if something nasty happens.
\remark
	This function:
	- allocates memory
	- make nessesary initializations
*/
ab_t* ab_create( void );

/**
	Destroy the ab_t object. 
\param
	ab - pointer to pointer to destroying object.
		pointer to object will set to NULL
		after destroying
\remark
	After all ab = NULL.
*/
void ab_destroy( ab_t ** ab );

/** @} */

/** @defgroup AB_RINGS_TONES Ringing and toneplay libab interface.
	Rings and Tones.
  @{ */
int ab_FXS_line_ring( ab_chan_t * const chan, enum ab_chan_ring_e ring );
int ab_FXS_line_tone( ab_chan_t * const chan, enum ab_chan_tone_e tone );
int ab_FXS_line_feed( ab_chan_t * const chan, enum ab_chan_linefeed_e feed );

int ab_FXO_line_hook( ab_chan_t * const chan, enum ab_chan_hook_e hook );
int ab_FXO_line_digit( 
		ab_chan_t * const chan, 
		char const data_length, char const * const data,
		char const nInterDigitTime, char const nDigitPlayTime );
/** @} */

int ab_dev_event_get( 
		ab_dev_t * const dev, 
		ab_dev_event_t * const evt, 
		unsigned char * const chan_available );

// ... MEDIA 

int ab_chan_media_activate( ab_chan_t * const chan );
int ab_chan_media_deactivate( ab_chan_t * const chan );

// ... INTERNAL CALLS ( FROM CHAN TO CHAN / CONFERENCE )
//

// ... 

#endif /* __AB_API_H__ */
