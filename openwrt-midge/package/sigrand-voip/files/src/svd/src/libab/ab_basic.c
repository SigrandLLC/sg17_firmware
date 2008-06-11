#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <fcntl.h>

#include "ab_internal_v22.h"

#define AB_DRIVER_DEV_NODE "/dev/sgatab"

int ab_g_err_idx;
char const * ab_g_err_str;

static struct ab_init_params_s params;

static int ab_struct_init( ab_t ** const abp );
static int ab_params_read( ab_t * const ab );
static void ab_chan_status_init( ab_chan_t * const chan );

extern int ab_dev_event_clean(ab_dev_t * const dev);

/////////////////////////////////////////////////

ab_t* 
ab_create( void )
{
	ab_t *ab = NULL;
	int err = 0;

	/* AB read config from drv_sgatab.ko */
	err = ab_params_read (ab);
	if( err ) {
		goto ab_create__exit;
	}

	/* AB structure inits */
	err = ab_struct_init (&ab);
	if( err ) {
		goto ab_create__ab_alloc;
	}

	return ab;
	
ab_create__ab_alloc:
	ab_destroy (&ab);
ab_create__exit:
	return NULL;
}; 

void 
ab_destroy( ab_t ** ab )
{
	ab_t * ab_tmp = *ab;
	if(ab_tmp) {
		if(ab_tmp->name) {
			free (ab_tmp->name);
		}
		if(ab_tmp->chans) {
			int i;
			int j;
			j = ab_tmp->chans_num;
			for(i = 0; i < j; i++) {
				ab_chan_t * curr_chan = &ab_tmp->chans[ i ];
				if(curr_chan->rtp_fd > 0) {
					close(curr_chan->rtp_fd);
				}
			}
			free (ab_tmp->chans);
		}
		if(ab_tmp->devs) {
			int i;
			int j;
			j = ab_tmp->devs_num;
			for(i = 0; i < j; i++) {
				ab_dev_t * curr_dev = &ab_tmp->devs[ i ];
				if(curr_dev->cfg_fd > 0) {
					close(curr_dev->cfg_fd);
				}
			}
			free (ab_tmp->devs);
		}
		free (ab_tmp);
		ab_tmp = NULL;
	}
};


/////////////////////////////////////////////////

static int 
ab_struct_init(ab_t ** const abp )
{
	ab_t * ab;
	int devs_count;
	int dev_N;
	int i;
	int j;

	ab = malloc(sizeof(*ab));
	*abp = ab;

	if( ! ab ) {
		ab_err_set(ab, AB_ERR_NO_MEM, NULL);
		goto ab_struct_init__exit;
	}
	memset(ab, 0, sizeof(*ab));

	ab->chans_per_dev = CHANS_PER_DEV;

	for (devs_count=0, i=0; i<DEVS_PER_BOARD_MAX; i++){
		if(params.devices[i] != dev_type_ABSENT){
			devs_count++;
		}
	}

	ab->devs_num = devs_count;
	ab->chans_num = devs_count * CHANS_PER_DEV;
	if((! ab->devs_num) || (! ab->chans_num)) {
		ab_err_set(ab, AB_ERR_BAD_PARAM, 
				"devices or channels number is zero" );
		goto ab_struct_init__exit;
	}

	ab->chans = malloc(sizeof(*(ab->chans)) * ab->chans_num);
	ab->devs = malloc(sizeof(*(ab->devs)) * ab->devs_num);
	if( (! ab->chans) || (! ab->devs) ){
		ab_err_set(ab, AB_ERR_NO_MEM, 
				"no memory for chans or devs " 
				"structures");
		goto ab_struct_init__exit;
	}
	memset(ab->chans, 0, sizeof(*(ab->chans)) * ab->chans_num);
	memset(ab->devs, 0, sizeof(*(ab->devs)) * ab->devs_num);

	/* Devices init */ 
	j = ab->devs_num;
	for (dev_N=0,i=0; i<j; i++,dev_N++){
		ab_dev_t * curr_dev = &ab->devs[ i ];
		int fd_chip;
		char dev_node[ 50 ];

		curr_dev->idx = i + 1;
		curr_dev->parent = ab;

		sprintf(dev_node,"/dev/vin%d0", curr_dev->idx );

		fd_chip = open(dev_node, O_RDWR, 0x644);
		if( fd_chip <= 0 ){
			ab_err_set(curr_dev, AB_ERR_NO_FILE, 
					"opening vinetic device node");
			goto ab_struct_init__exit;
		}
		curr_dev->cfg_fd = fd_chip;

		for(;params.devices[dev_N] == dev_type_ABSENT && 
				dev_N < DEVS_PER_BOARD_MAX; dev_N++ );

		if (params.devices[dev_N] == dev_type_FXS){
			curr_dev->type = ab_dev_type_FXS;
		} else if (params.devices[dev_N] == dev_type_FXO){
			curr_dev->type = ab_dev_type_FXO;
		}
	}

	/* Channels init */ 
	j = ab->chans_num;
	for(i = 0; i < j; i++) {
		ab_chan_t * curr_chan = &ab->chans[ i ];
		int fd_chan;
		char dev_node[ 50 ];

		curr_chan->idx = ( i % CHANS_PER_DEV) + 1;
		curr_chan->parent = &ab->devs[ i / CHANS_PER_DEV ];

		/* Initialize channel */
		sprintf(dev_node, "/dev/vin%d%d", 
				curr_chan->parent->idx, curr_chan->idx);

		fd_chan = open(dev_node, O_RDWR, 0x644);
		if (fd_chan <= 0){
			ab_err_set(curr_chan, AB_ERR_NO_FILE, 
					"opening vinetic channel node");
			goto ab_struct_init__exit;
		}
		curr_chan->rtp_fd = fd_chan;
		curr_chan->abs_idx = params.first_chan_idx + i;

		/* set channel status to initial proper values */
		ab_chan_status_init (curr_chan);
	}

	return 0;

ab_struct_init__exit:
	return -1;
};

static int 
ab_params_read( ab_t * const ab )
{
	ab_boards_presence_t bp;
	int ab_fd;
	int err = 0;

	memset (&params, 0, sizeof(params));
	memset (&bp, 0, sizeof(bp));

	/* Get board mount info */
	ab_fd = open(AB_DRIVER_DEV_NODE, O_RDWR, 0x644);
	if (ab_fd < 0) {
		ab_err_set(ab, AB_ERR_NO_FILE, "opening board device node");
		goto ab_params_read__exit;
	}

	err = ioctl(ab_fd, SGAB_GET_BOARDS_PRESENCE, &bp);
	if(err) {
		ab_err_set(ab, AB_ERR_UNKNOWN, 
				"getting ata boards presence info (ioctl)");
		goto ab_params_read__close;
	}

	params.requested_board_slot = bp.slots [0];

	err = ioctl(ab_fd, SGAB_GET_INIT_PARAMS, &params);
	if(err) {
		ab_err_set(ab, AB_ERR_UNKNOWN, 
				"getting ata init board info (ioctl)");
		goto ab_params_read__close;
	}
	close (ab_fd);
	return 0;

ab_params_read__close:
	close (ab_fd);
ab_params_read__exit:
	return -1;
};

static void 
ab_chan_status_init( ab_chan_t * const chan )
{
	if(chan->parent->type == ab_dev_type_FXS){
		/* linefeed to standby */
		ioctl(chan->rtp_fd, IFX_TAPI_LINE_FEED_SET, 
				IFX_TAPI_LINE_FEED_STANDBY);
		chan->status.linefeed = ab_chan_linefeed_STANDBY;

		/* ring to mute */
		ioctl( chan->rtp_fd, IFX_TAPI_RING_STOP, 0 );
		chan->status.ring = ab_chan_ring_MUTE;

		/* tone to mute */
		ioctl( chan->rtp_fd, IFX_TAPI_TONE_LOCAL_PLAY, 0 );
		chan->status.tone = ab_chan_tone_MUTE;

	} else if (chan->parent->type == ab_dev_type_FXO){
		/* hook to onhook */
		ioctl( chan->rtp_fd, IFX_TAPI_FXO_HOOK_SET,
				IFX_TAPI_FXO_HOOK_ONHOOK );
		chan->status.hook= ab_chan_hook_ONHOOK;
	}

	/* TODO : initial onhook detected  (from channel) */
	ab_dev_event_clean(chan->parent);

	/*
	if(chan->parent->type == ab_dev_type_FXS){
		ab_FXS_line_feed(chan, ab_chan_linefeed_DISABLED);
	}
	*/
};

