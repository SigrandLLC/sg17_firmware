				/*
				IFX_boolean_t hook;
				err = ioctl (ab->chans[2].rtp_fd,
					IFX_TAPI_FXO_HOOK_GET, &hook);
				*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <linux/ioctl.h>
#include <fcntl.h>
#include <errno.h>

#include "ifx_types.h"
#include "drv_tapi_io.h"
#include "vinetic_io.h"
#include "ab_api.h"
#include "ab_ioctl.h"

#define FXS_DEV_ID 0
#define FXS_CHAN_ID 0
#define FXO_DEV_ID 1
#define FXO_CHAN_ID 2

int main (int argc, char *argv[])
{
	ab_t * ab;
	int choise;
	char str[ 10 ];
	int err = 1234;

	ab = ab_create();
	if(!ab) {
		fprintf(stderr,"Iinitialization FAILED\n");
		fprintf(stderr,"ERROR: %s\n", ab_g_err_str);
		return -1;
	}

	fprintf(stderr,"Iinitialization SUCCESSFUL\n");

	choise = 0;
	while(1) {
		printf("\n");
		printf("=====================\n");
		printf("[1] Event test.\n");
		printf("[21] Ring START.\n");
		printf("[22] Ring STOP.\n");
		printf("[23] PLAY DIAL.\n");
		printf("[24] PLAY BUSY.\n");
		printf("[25] PLAY RINGBACK.\n");
		printf("[26] PLAY MUTE.\n");
		printf("[31] OFFHOOK.\n");
		printf("[32] ONHOOK.\n");
		printf("[41] LINEFEED DISABLED.\n");
		printf("[42] LINEFEED STANDBY.\n");
		printf("[43] LINEFEED ACTIVE.\n");
		printf("[51] DIAL #*1234.\n");
		printf("[52] DIAL 567890.\n");
		printf("[53] DIAL ABCD.\n");

		printf("[0] Exit.\n");
		printf("=====================\n");
		printf("=> ");
		fgets(str, 9, stdin);
		choise = strtol(str, NULL, 10);
		switch(choise){
			case 1: {
				ab_dev_event_t evt;

		fprintf(stderr,"\t---=== FXS_DEVICE_EVENTS ===---\n");	
				do {
					unsigned char chan_avail;
					err = ab_dev_event_get( 
							&ab->devs[FXS_DEV_ID], &evt, &chan_avail);
					if( !chan_avail){
						fprintf(stderr,"No CHAN AVAILABLE\n");
						break;
					}
		switch(evt.id){
			case ab_dev_event_NONE: {
				fprintf(stderr,"NONE\n");
				break;
			}	
			case ab_dev_event_UNCATCHED: {
				fprintf(stderr,"[%d]-UNCATCHED : 0x%lX\n", 
						evt.ch, evt.data);	
				break;
			}	
			case ab_dev_event_FXO_RINGING: {
				fprintf(stderr,"[%d]-FXO_RINGING : 0x%lX\n", 
						evt.ch, evt.data);	
				break;
			}	
			case ab_dev_event_FXS_DIGIT_TONE: {
				fprintf(stderr,"[%d]-FXS_DIGIT_TONE : '%c'\n", 
						evt.ch, (unsigned char)evt.data);	
				break;
			}	
			case ab_dev_event_FXS_DIGIT_PULSE: {
				fprintf(stderr,"[%d]-FXS_DIGIT_PULSE : '%c'\n", 
						evt.ch, (char)evt.data);	
				break;
			}	
			case ab_dev_event_FXS_ONHOOK: {
				fprintf(stderr,"[%d]-FXS_ONHOOK\n", evt.ch);	
				break;
			}	
			case ab_dev_event_FXS_OFFHOOK: {
				fprintf(stderr,"[%d]-FXS_OFFHOOK\n", evt.ch);	
				break;
			}	
			case ab_dev_event_FXS_FM_CED: {
				fprintf(stderr,"[%d]-FXS_FM_CED\n", evt.ch);	
				break;
			}	
			case ab_dev_event_COD: {
				fprintf(stderr,"[%d]-COD\n", evt.ch);	
				break;
			}	
			case ab_dev_event_TONE: {
				fprintf(stderr,"[%d]-TONE\n", evt.ch);	
				break;
			}	
		}
				} while(evt.more);

		fprintf(stderr,"\t---=========================---\n");	
		fprintf(stderr,"\t---=== FXO_DEVICE_EVENTS ===---\n");	
				do {
					unsigned char chan_avail;
					err = ab_dev_event_get( 
							&ab->devs[FXO_DEV_ID], &evt, &chan_avail);
					if( !chan_avail){
						fprintf(stderr,"No CHAN AVAILABLE\n");
						break;
					}
		switch(evt.id){
			case ab_dev_event_NONE: {
				fprintf(stderr,"NONE\n");
				break;
			}	
			case ab_dev_event_UNCATCHED: {
				fprintf(stderr,"[%d]-UNCATCHED : 0x%lX\n", 
						evt.ch, evt.data);	
				break;
			}	
			case ab_dev_event_FXO_RINGING: {
				fprintf(stderr,"[%d]-FXO_RINGING : 0x%lX\n", 
						evt.ch, evt.data);	
				break;
			}	
			case ab_dev_event_FXS_DIGIT_TONE: {
				fprintf(stderr,"[%d]-FXS_DIGIT_TONE : '%c'\n", 
						evt.ch, (char)evt.data);	
				break;
			}	
			case ab_dev_event_FXS_DIGIT_PULSE: {
				fprintf(stderr,"[%d]-FXS_DIGIT_PULSE : '%c'\n", 
						evt.ch, (char)evt.data);	
				break;
			}	
			case ab_dev_event_FXS_ONHOOK: {
				fprintf(stderr,"[%d]-FXS_ONHOOK\n", evt.ch);	
				break;
			}	
			case ab_dev_event_FXS_OFFHOOK: {
				fprintf(stderr,"[%d]-FXS_OFFHOOK\n", evt.ch);
				break;
			}	
			case ab_dev_event_FXS_FM_CED: {
				fprintf(stderr,"[%d]-FXS_FM_CED\n", evt.ch);	
				break;
			}	
			case ab_dev_event_COD: {
				fprintf(stderr,"[%d]-COD\n", evt.ch);	
				break;
			}	
			case ab_dev_event_TONE: {
				fprintf(stderr,"[%d]-TONE\n", evt.ch);	
				break;
			}	
		}
				} while(evt.more);
		fprintf(stderr,"\t---=========================---\n");	
				break;
			}

			case 21: {
 				err = ab_FXS_line_ring( &ab->chans[FXS_CHAN_ID], 
						ab_chan_ring_RINGING );
				fprintf(stderr,"ERR = %d\n",err);	

				break;
			}
			case 22: {
 				err = ab_FXS_line_ring( &ab->chans[FXS_CHAN_ID], 
						ab_chan_ring_MUTE );
				fprintf(stderr,"ERR = %d\n",err);	
				break;
			}
			case 23: {
 				err = ab_FXS_line_tone( &ab->chans[FXS_CHAN_ID], 
						ab_chan_tone_DIAL);
				fprintf(stderr,"ERR = %d\n",err);	
				break;
			}
			case 24: {
 				err = ab_FXS_line_tone( &ab->chans[FXS_CHAN_ID], 
						ab_chan_tone_BUSY);
				fprintf(stderr,"ERR = %d\n",err);	
				break;
			}
			case 25: {
 				err = ab_FXS_line_tone( &ab->chans[FXS_CHAN_ID], 
						ab_chan_tone_RINGBACK);
				fprintf(stderr,"ERR = %d\n",err);	
				break;
			}
			case 26: {
 				err = ab_FXS_line_tone( &ab->chans[FXS_CHAN_ID], 
						ab_chan_tone_MUTE);
				fprintf(stderr,"ERR = %d\n",err);	
				break;
			}
			case 31: {
				err = ab_FXO_line_hook( &ab->chans[FXO_CHAN_ID], 
						ab_chan_hook_OFFHOOK);
				fprintf(stderr,"ERR = %d\n",err);	
				break;
			}
			case 32: {
				err = ab_FXO_line_hook( &ab->chans[FXO_CHAN_ID], 
						ab_chan_hook_ONHOOK);
				fprintf(stderr,"ERR = %d\n",err);	
				break;
			}
			case 41: {
				err = ab_FXS_line_feed( &ab->chans[FXS_CHAN_ID], 
						ab_chan_linefeed_DISABLED );
				fprintf(stderr,"ERR = %d\n",err);	
				break;
			}
			case 42: {
				err = ab_FXS_line_feed( &ab->chans[FXS_CHAN_ID], 
						ab_chan_linefeed_STANDBY );
				fprintf(stderr,"ERR = %d\n",err);	
				break;
			}
			case 43: {
				err = ab_FXS_line_feed( &ab->chans[FXS_CHAN_ID], 
						ab_chan_linefeed_ACTIVE );
				fprintf(stderr,"ERR = %d\n",err);	
				break;
			}
			case 51: {
				char data[] = {'#','*','1','2','3','4'};
				err = ab_FXO_line_digit( &ab->chans[FXO_CHAN_ID],
						6, data, 100,100 );
				fprintf(stderr,"ERR = %d\n",err);	
				break;
			}
			case 52: {
				char data[] = {'5','6','7','8','9','0'};
				err = ab_FXO_line_digit( &ab->chans[FXO_CHAN_ID],
						6, data, 100,100 );
				fprintf(stderr,"ERR = %d\n",err);	
				break;
			}
			case 53: {
				char data[] = {'A','B','C','D'};
				err = ab_FXO_line_digit( &ab->chans[FXO_CHAN_ID],
						4, data, 100,100 );
				fprintf(stderr,"ERR = %d\n",err);	
				break;
			}
			case 0: {
				goto main_exit;
			}
			default: {
				printf("Error: enter the valid value!\n");
			}
		}
	}

main_exit:
	fprintf(stderr,"THE END : %d : %s\n",ab_g_err_idx, ab_g_err_str);
	ab_destroy(&ab);
	return 0;
};

