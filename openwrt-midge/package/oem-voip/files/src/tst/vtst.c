#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <linux/ioctl.h>
#include <fcntl.h>
#include <sys/poll.h>
#include <unistd.h>
#include <errno.h>

#include "ifx_types.h"
#include "drv_tapi_io.h"
#include "vinetic_io.h"
#include "ab_api.h"
#include "ab_ioctl.h"

#define RTP_READ_MAX 512

struct g_status_s {
	int c1_is_offhook;
	int c2_is_offhook;
	int enc_dec_is_on;
	int c1_id;
	int c2_id;
	int poll_fd_num;
} g_status;

/* start connection */
int 
start_connection(ab_t * const ab)
{
	ab_chan_t * c1 = &ab->chans[g_status.c1_id];
	ab_chan_t * c2 = &ab->chans[g_status.c2_id];
	int err = 0;

	/* tune all */
	c1->rtp_cfg.nEvents = 
		c2->rtp_cfg.nEvents =			evts_OOB_ONLY;
	c1->rtp_cfg.nPlayEvents = 
		c2->rtp_cfg.nPlayEvents =		play_evts_PLAY;
	c1->rtp_cfg.evtPT = 
		c2->rtp_cfg.evtPT =				0x62;
	c1->rtp_cfg.evtPTplay = 
		c2->rtp_cfg.evtPTplay =			0x62;
	c1->rtp_cfg.cod_pt = 
		c2->rtp_cfg.cod_pt =			cod_pt_ALAW;
	c1->rtp_cfg.cod_volume.enc_dB = 
		c2->rtp_cfg.cod_volume.enc_dB =	0;
	c1->rtp_cfg.cod_volume.dec_dB = 
		c2->rtp_cfg.cod_volume.dec_dB =	0;
	c1->rtp_cfg.VAD_cfg = 
		c2->rtp_cfg.VAD_cfg =			vad_cfg_OFF;
	c1->rtp_cfg.HPF_is_ON = 
		c2->rtp_cfg.HPF_is_ON =			0;

	err += ab_chan_media_rtp_tune(c1);
	err += ab_chan_media_rtp_tune(c2);

	/* start enc / dec */
	err += ab_chan_media_switch (c1, 1,1);
	err += ab_chan_media_switch (c2, 1,1);

	return err;
}

/* stop connection */
int 
stop_connection(ab_t * const ab)
{
	int err = 0;

	/* stop enc / dec */
	err += ab_chan_media_switch (&ab->chans[g_status.c1_id], 0,0);
	err += ab_chan_media_switch (&ab->chans[g_status.c2_id], 0,0);
	return err;
}

void
rwdata(int ffd,int tfd, unsigned char const f_aid, unsigned char const t_aid)
{
	int rode;
	int written;
	unsigned char buff[RTP_READ_MAX];

	memset(buff, 0, RTP_READ_MAX);
	rode = read(ffd, buff, RTP_READ_MAX);
	if(rode == -1){
		fprintf(stderr,"[%d]: ", f_aid);
		perror("read(): ");
	} else if(rode == 0){
		fprintf(stderr,"[%d]: Unexpected event (nothing to read)\n", f_aid);
	} else {
		written = write (tfd, buff, rode);
		if(written == -1){
			fprintf(stderr,"[%d]: ", t_aid);
			perror("write(): ");
		} else if( written != rode ){
			fprintf(stderr,"[%d]=>[%d]: RWD error: [%d/%d]\n",
					f_aid,t_aid,rode,written);
		} else {
			int i;
			fprintf(stderr,"[%d]=>_",f_aid);
			for(i=0;i<12;i++){
				fprintf(stderr, "%02X_",buff[i]);
			}
			fprintf(stderr,"|");
			for(;(i<rode)&&(i<20);i++){
				fprintf(stderr, "%02X|",buff[i]);
			}
			fprintf(stderr,"...%d=>[%d]\n",rode,t_aid);
		}
	}
}

void
chaev (ab_t * ab, struct pollfd * fds)
{
	if(fds[g_status.c1_id].revents){
		if(fds[g_status.c1_id].revents != POLLIN){
			fprintf(stderr,"[%d]: revents: 0x%X\n",
					ab->chans[g_status.c1_id].abs_idx,
					fds[g_status.c1_id].revents);
		}
		/* data in channel 1 */
		rwdata(fds[g_status.c1_id].fd,fds[g_status.c2_id].fd, 
				ab->chans[g_status.c1_id].abs_idx,
				ab->chans[g_status.c2_id].abs_idx);
	}
	if(fds[g_status.c2_id].revents){
		if(fds[g_status.c2_id].revents != POLLIN){
			fprintf(stderr,"[%d]: revents: 0x%X\n",
					ab->chans[g_status.c2_id].abs_idx,
					fds[g_status.c1_id].revents);
		}
		/* data in channel 2 */
		rwdata(fds[g_status.c2_id].fd,fds[g_status.c1_id].fd, 
				ab->chans[g_status.c2_id].abs_idx,
				ab->chans[g_status.c1_id].abs_idx);
	}
}

void
devact(ab_t * ab, int dev_id)
{
	ab_dev_t * dev = &ab->devs[dev_id];
	ab_dev_event_t evt;
	unsigned char ca;
	int chan_id;
	int err;

	/* data on device */
	err = ab_dev_event_get(dev, &evt, &ca);
	if(err || !ca || evt.more){
		fprintf(stderr,">> DEV: %s [e%d/c%d/m%d/d0x%lX]\n",
				ab_g_err_str,err,ca, evt.more, evt.data);
		return;
	} 

	/* if evt.ch == 0 -> chans[i+1] if 1 -> chans[i] */
	chan_id = ((evt.ch + 1) % 2) + dev_id*ab->chans_per_dev;

	if(evt.id == ab_dev_event_FXS_OFFHOOK){
		/* OFFHOOK on some chan */
		err = ab_FXS_line_feed (&ab->chans[chan_id],ab_chan_linefeed_ACTIVE);
		if(err){
			fprintf(stderr,"LFA_%d ERROR",ab->chans[chan_id].abs_idx);
			exit(EXIT_FAILURE);
		} else {
			fprintf(stderr,"LFA_%d\n",ab->chans[chan_id].abs_idx);	
		}
		
		if(g_status.c1_is_offhook && g_status.c2_is_offhook){
			/* third actor - ignore event */
			fprintf(stderr,"(offhook) there is no conference "
					"implementation :) [%d]\n",
					chan_id);
		} else if(!g_status.c1_is_offhook && g_status.c2_is_offhook){
			/* c1 was onhook -> it will be the first chan */
			g_status.c1_id = chan_id;
			g_status.c1_is_offhook = 1;
		} else if(!g_status.c1_is_offhook && !g_status.c2_is_offhook){
			/* c1 and c2 was onhook -> it will be the first chan */
			g_status.c1_id = chan_id;
			g_status.c1_is_offhook = 1;
		} else if(g_status.c1_is_offhook && !g_status.c2_is_offhook){
			/* c2 was onhook -> it will be the second chan */
			g_status.c2_id = chan_id;
			g_status.c2_is_offhook = 1;
		}

		if(g_status.c1_is_offhook && g_status.c2_is_offhook){
			err = start_connection (ab);
			if(err){
				fprintf(stderr,"UP ERROR");
				exit(EXIT_FAILURE);
			} else {
				g_status.enc_dec_is_on = 1;
				fprintf(stderr,"UP\n");	
			}
		}
	} else if(evt.id == ab_dev_event_FXS_ONHOOK){
		int conf_err = 0;
		/* ONHOOK on some chan */
		err = ab_FXS_line_feed (&ab->chans[chan_id],ab_chan_linefeed_STANDBY);
		if(err){
			fprintf(stderr,"LFS_%d ERROR",ab->chans[chan_id].abs_idx);
			exit(EXIT_FAILURE);
		} else {
			fprintf(stderr,"LFS_%d\n",ab->chans[chan_id].abs_idx);	
		}

		if(chan_id == g_status.c1_id){
			g_status.c1_is_offhook = 0;
		} else if(chan_id == g_status.c2_id){
			g_status.c2_is_offhook = 0;
		} else {
			fprintf(stderr,"(onhook) there is no conference "
					"implementation :) [%d]\n",
					chan_id);
			conf_err = 1;
		}

		if(!conf_err && g_status.enc_dec_is_on){
			err = stop_connection(ab);
			if(err){
				fprintf(stderr,"DOWN ERROR");
				exit(EXIT_FAILURE);
			} else {
				g_status.enc_dec_is_on = 0;
				fprintf(stderr,"DOWN\n");	
			}
		}
	} else {
		fprintf(stderr,"(%d,%d) [%d | 0x%lX]\n", 
				dev_id, evt.ch, evt.id, evt.data);
	}
}

void
devev(ab_t * ab, struct pollfd * fds)
{
	int i;
	int j;

	/* test event for all devices */
	j=g_status.poll_fd_num;
	for (i=ab->chans_num; i<j; i++){
		if(fds[i].revents){
			if(fds[i].revents != POLLIN){
				fprintf(stderr,"revents on dev[%d] is 0x%X\n",
						i-ab->chans_num, fds[i].revents);
			}
			devact(ab, i-ab->chans_num);
		}
	} 
}

void
start_polling(ab_t * const ab)
{
	struct pollfd * fds;
	int i;
	int j;

	g_status.poll_fd_num = ab->chans_num + ab->devs_num;
	fds = malloc(sizeof(*fds)*g_status.poll_fd_num);

	j=ab->chans_num;
	for (i=0; i<j; i++){
		fds[i].fd = ab->chans[i].rtp_fd;
		fds[i].events = POLLIN;
	} 
	for(j=0;i<g_status.poll_fd_num;i++,j++){
		fds[i].fd = ab->devs[j].cfg_fd;
		fds[i].events = POLLIN;
	}

	/* poll on chans and dev */
	while(1){
		if(poll(fds, g_status.poll_fd_num, -1) == -1){
			perror("poll : ");
			return;
		}
		/* test events on channels */
		chaev (ab,fds);
		/* test events on devices */
		devev (ab,fds);
	}
}

int 
main (int argc, char *argv[])
{
	ab_t * ab;

	ab = ab_create();
	if(!ab) {
		fprintf(stderr,"ERROR: %s\n", ab_g_err_str);
		return -1;
	}

	memset(&g_status, 0, sizeof(g_status));
	/*
 	// before set in devev()
	g_status.c1_is_offhook = 0;
	g_status.c2_is_offhook = 0;
	g_status.enc_dec_is_on = 0;
	g_status.c1_id = 0; // to indicate, that it not has been set
	g_status.c2_id = 0;
	*/

	fprintf(stderr,"Iinitialization SUCCESSFUL\n");

	start_polling (ab);

	fprintf(stderr,"THE END: %d:%s\n",ab_g_err_idx, ab_g_err_str);
	ab_destroy(&ab);
	return 0;
}

