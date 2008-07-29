#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <fcntl.h>

#include "ab_internal_v22.h"

int 
ab_dev_event_get(ab_dev_t * const dev, 
		ab_dev_event_t * const evt, 
		unsigned char * const chan_available )
{
	IFX_TAPI_EVENT_t ioctl_evt;
	int err = 0;

	memset(&ioctl_evt, 0, sizeof(ioctl_evt));
	memset(evt, 0, sizeof(*evt));

	*chan_available = 0;

	ioctl_evt.ch = IFX_TAPI_EVENT_ALL_CHANNELS;

	err = ioctl(dev->cfg_fd, IFX_TAPI_EVENT_GET, &ioctl_evt);
	if( err ){
		ab_err_set(dev, AB_ERR_UNKNOWN, "Getting event (ioctl)"); 
		goto ab_dev_event_get_exit;
	}

	evt->id = ab_dev_event_NONE;

	if(!ioctl_evt.id){
		goto ab_dev_event_get_exit;
	}

	evt->ch = ioctl_evt.ch;
	evt->more = ioctl_evt.more;

	switch(ioctl_evt.id){
		case IFX_TAPI_EVENT_FXS_ONHOOK: {
			evt->id = ab_dev_event_FXS_ONHOOK;
			break;
		}
		case IFX_TAPI_EVENT_FXS_OFFHOOK: {
			evt->id = ab_dev_event_FXS_OFFHOOK;
			break;
		}
		case IFX_TAPI_EVENT_DTMF_DIGIT: {
			evt->id = ab_dev_event_FXS_DIGIT_TONE;
			evt->data = ioctl_evt.data.dtmf.ascii;
			evt->data += 
				(long )(ioctl_evt.data.dtmf.local) << 9;
			evt->data += 
				(long )(ioctl_evt.data.dtmf.network) << 8;
			break;
		}
		case IFX_TAPI_EVENT_PULSE_DIGIT: {
			evt->id = ab_dev_event_FXS_DIGIT_PULSE;
			if(ioctl_evt.data.pulse.digit == 0xB){
				evt->data = '0';
			} else {
				evt->data = '0' + ioctl_evt.data.pulse.digit;
			}
			break;
		}
		case IFX_TAPI_EVENT_FXO_RING_START: {
			evt->id = ab_dev_event_FXO_RINGING;
			evt->data = 1;
			break;
		}
		case IFX_TAPI_EVENT_FXO_RING_STOP: {
			evt->id = ab_dev_event_FXO_RINGING;
			evt->data = 0;
			break;
		}
		default:{
			evt->id = ab_dev_event_UNCATCHED;
			evt->data = ioctl_evt.id;
		}
	}

	if (ioctl_evt.ch == IFX_TAPI_EVENT_ALL_CHANNELS){
		*chan_available = 0;
	} else {
		*chan_available = 1;
	}

ab_dev_event_get_exit:
	return err;
}

int ab_dev_event_clean(ab_dev_t * const dev)
{
	ab_dev_event_t evt;
	int err = 0;

	memset(&evt, 0, sizeof(evt));

	do {
		unsigned char ch_av;
		err = ab_dev_event_get(dev, &evt, &ch_av);
		if(err){
			goto ab_dev_event_clean__exit;
		}
	} while (evt.more);

	return 0;

ab_dev_event_clean__exit:
	return -1;
}


