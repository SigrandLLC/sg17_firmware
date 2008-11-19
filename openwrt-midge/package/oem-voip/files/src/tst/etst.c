#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>

#include "ab_api.h"

#define WAIT_INTERVAL 500000
#define EVENTS_PRECLEAN 8

enum action_e {action_PUT, action_GET};
enum ch_e {ch_FXS, ch_FXO};

struct ch_couple_s {
	enum ch_e chans[2];
} * g_cps = NULL;

unsigned int g_cps_num = 0;
int g_verbose = 0;

void
events_clean(ab_chan_t const * const chan)
{/*{{{*/
	ab_dev_event_t evt;
	int err = 0;
	int iter_N = EVENTS_PRECLEAN;

	memset(&evt, 0, sizeof(evt));

	do {
		unsigned char ch_av;
		err = ab_dev_event_get(chan->parent, &evt, &ch_av);
		if(err){
			break;
		}
		iter_N--;
	} while (evt.more && iter_N);

	return;
}/*}}}*/

int
make_a_couple(ab_chan_t const * const chan_fxs, 
		ab_chan_t const * const chan_fxo)
{/*{{{*/
	int i;

	/* is this chans in any couples allready */
	for (i=0; i<g_cps_num; i++){
		int cross_couple = 
			(
				(chan_fxs->abs_idx == g_cps [i].chans[ch_FXS]) &&
				(chan_fxo->abs_idx != g_cps [i].chans[ch_FXO])
			) || (
				(chan_fxs->abs_idx != g_cps [i].chans[ch_FXS]) &&
				(chan_fxo->abs_idx == g_cps [i].chans[ch_FXO])
			);
		int allready_in = 
			(chan_fxs->abs_idx == g_cps [i].chans[ch_FXS]) &&
			(chan_fxo->abs_idx == g_cps [i].chans[ch_FXO]);
		if( cross_couple ){
			fprintf(stderr,"[%d] and [%d] are in different couples\n",
				chan_fxs->abs_idx, chan_fxo->abs_idx);
			return -1;
		} else if(allready_in) {
			return 0;
		}
	} 

	/* create new couple */
	g_cps [g_cps_num].chans[ch_FXS] = chan_fxs->abs_idx;
	g_cps [g_cps_num].chans[ch_FXO] = chan_fxo->abs_idx;
	fprintf(stdout,"[%d] and [%d] are in couple\n",
			g_cps [g_cps_num].chans[ch_FXS],
			g_cps [g_cps_num].chans[ch_FXO]);
	g_cps_num++;

	return 0;
}/*}}}*/

int
find_couple (ab_chan_t const * const chan_fxs, ab_chan_t const * const chan_fxo)
{/*{{{*/
	ab_chan_t const * source_chan = NULL;
	int idx_to_find;
	int source_idx;
	int check = 0;
	int i;


	if( chan_fxs && chan_fxo){
		/* check that couple 
		 * return 0 - success
		 * 1 - fail
		 * */
		check = 1;
	} else if( !chan_fxs || !chan_fxo){
		if(chan_fxs){
			/* should find fxo by fxs */	
			idx_to_find = ch_FXO;
			source_idx = ch_FXS;
			source_chan = chan_fxs;
		} else if(chan_fxo){
			/* should find fxs by fxo */	
			idx_to_find = ch_FXS;
			source_idx = ch_FXO;
			source_chan = chan_fxo;
		} else {
			/* error - nothing to find */	
			return -1;
		}
	}

	/* is this chans in any couples allready */
	for (i=0; i<g_cps_num; i++){
		if(check){
			int allready_in = 
				(chan_fxs->abs_idx == g_cps [i].chans[ch_FXS]) &&
				(chan_fxo->abs_idx == g_cps [i].chans[ch_FXO]);
			if(allready_in){
				return 0; /*couple is in*/
			}
		} else {
			if(source_chan->abs_idx == g_cps [i].chans[source_idx]){
				return g_cps [i].chans[idx_to_find];
			}
		}
	} 
	return -1;
}/*}}}*/

void
linefeed_keeper( ab_t * ab, enum action_e action )
{/*{{{*/
	/** */
	static enum ab_chan_linefeed_e * lf = NULL;
	int i;
	int j;
	int err;

	assert( (!action && !lf) || (action && lf) );

	j = ab->chans_num;

	if(action == action_PUT){
		lf = malloc(sizeof(*lf)* j);
		if( !lf){
			fprintf(stderr,"!ERROR : Not enough memory\n");
			exit(EXIT_FAILURE);
		}
		/* remember linefeeds */
		for (i=0; i<j; i++){
			if(ab->chans[i].parent->type == ab_dev_type_FXS){
				lf[i] = ab->chans[i].status.linefeed;
			}
		}
	} else if (action == action_GET){
		/* restore linefeeds */
		for (i=0; i<j; i++){
			if(ab->chans[i].parent->type == ab_dev_type_FXS){
				err = ab_FXS_line_feed (&ab->chans[i], lf[i]);
				if(err){
					fprintf(stderr,"!ERROR : restore linefeed on [%d]\n",
							ab->chans[i].abs_idx);
					exit(EXIT_FAILURE);
				}
			}
		}
		free(lf);
		lf = NULL;
	}
}/*}}}*/

void
digits_test (ab_chan_t * const cFXO, ab_chan_t * const cFXS)
{/*{{{*/
	/* play all digits in tone mode (can`t generate pulse )
	 * and find channel that detects it... if other event on another chan
	 * 	- error */
	ab_t * ab = cFXO->parent->parent;
	ab_dev_event_t evt;
	unsigned char ca;
	int i;
	int event_acceptor_found = 0;
	int seq_length;
	int devs_num;
	int dial_idx;
	char to_dial[] = {'0','1','2','3','4','5','6','7','8','9',
					'*','#','A','B','C','D','\0'};
	int err;

	seq_length=strlen(to_dial);
	for (dial_idx=0; dial_idx<seq_length; dial_idx++){
		/* dial digits all by one */
		err = ab_FXO_line_digit (cFXO, 1, &to_dial [dial_idx], 0, 0);
		if(err){
			/* error happen on event getting ioctl */
			fprintf(stderr,"!ERROR : dial a '%c' on [%d]\n",
					to_dial [dial_idx], cFXO->abs_idx);
			return;
		} else if( g_verbose){
			fprintf(stdout,"[%d] >> tone dial '%c'\n",
					cFXO->abs_idx, to_dial [dial_idx]);
		}
		usleep (WAIT_INTERVAL);

		devs_num=ab->devs_num;
		for (i=0; i<devs_num; i++){
			/* for each dev - test event presense */
			int chan_idx;
			ab_dev_t * cur_dev = &ab->devs[i];

			err = ab_dev_event_get (cur_dev, &evt, &ca);
			if(err){
				/* error happen on event getting ioctl */
				fprintf(stderr,"!ERROR : dev[%d] : %s\n",i,ab_g_err_str);
				return;
			} else if(evt.id == ab_dev_event_NONE){
				/* no event on current device */
				continue;
			}
			if( !ca){
				/* no channel number available */
				fprintf(stderr,"!ERROR !ca : dev[%d] : %s\n",i,ab_g_err_str);
				return;
			}
			event_acceptor_found = 1;

			chan_idx = (cur_dev->idx - 1) * ab->chans_per_dev + evt.ch;

			if(ab->chans[chan_idx].abs_idx != cFXS->abs_idx){
				/* got event on channel not in couple */
				fprintf(stderr,"!ERROR event[%d:0x%lX] on wrong chan[%d]\n",
						evt.id, evt.data, ab->chans[chan_idx].abs_idx);
				return;
			} else if((evt.id != ab_dev_event_FXS_DIGIT_TONE) ||
					((char)evt.data != to_dial[dial_idx])){
				/* wrong event or data */
				fprintf(stderr,"!ERROR wrong event %d or data '%c'"
						" on [%d]\n",
						evt.id, to_dial[dial_idx], ab->chans[chan_idx].abs_idx);
				return;
			} else if(evt.more){
				/* additional events on correct chan */
				fprintf(stderr,"!ERROR additional events while diled '%c'"
						" on [%d]\n",
						to_dial[dial_idx], ab->chans[chan_idx].abs_idx);
				return;
			}
			if( g_verbose){
				fprintf(stdout,"[%d] << tone dial '%c'\n",
						ab->chans[chan_idx].abs_idx, to_dial [dial_idx]);
			}
		}
		if( !event_acceptor_found){
			/* additional events on correct chan */
			fprintf(stderr,"!ERROR [%d] can`t detect digit '%c' from [%d]\n",
					cFXS->abs_idx,to_dial[dial_idx],cFXO->abs_idx);
		}
	} 
}/*}}}*/

void
process_FXS(ab_chan_t * const chan)
{/*{{{*/
	ab_t * ab = chan->parent->parent;
	ab_dev_event_t evt;
	unsigned char ca;
	int i;
	int j;
	int err;
	int couple_has_been_found = 0;
	int couple_idx;
	int self_idx;
	int ch;
	int chan_idx;

	for(self_idx=0;
			(self_idx < ab->chans_num) && 
			(chan->abs_idx != ab->chans[self_idx].abs_idx); 
			self_idx++);

	/* emits ring and mutes it afrterwords {{{ */
	err = ab_FXS_line_ring(chan, ab_chan_ring_RINGING);
	if( err){
		fprintf(stderr,"!ERROR : chan [%d] : %s\n",
				chan->abs_idx,ab_g_err_str);
		return;
	} else if( g_verbose){
		fprintf(stdout,"[%d] >> RING\n",chan->abs_idx);
	}
	usleep (WAIT_INTERVAL);
	err = ab_FXS_line_ring(chan, ab_chan_ring_MUTE);
	if( err){
		fprintf(stderr,"!ERROR : chan [%d] : %s\n",
				chan->abs_idx,ab_g_err_str);
		return;
	} else if( g_verbose){
		fprintf(stdout,"[%d] >> RING (mute)\n",chan->abs_idx);
	}/*}}}*/

	usleep (WAIT_INTERVAL);

	/* test all devices on event get to find a couple (it can exists) {{{*/ 
	j=ab->devs_num;
	for (i=0; i<j; i++){
		ab_dev_t * cur_dev = &ab->devs[i];
		err = ab_dev_event_get (cur_dev, &evt, &ca);
		if(err){
			/* error happen on event getting ioctl */
			fprintf(stderr,"!ERROR : dev[%d] : %s\n",i,ab_g_err_str);
			return;
		} else if(evt.id == ab_dev_event_NONE){
			/* no event on current device */
			continue;
		}
		if( !ca){
			/* no channel number available */
			fprintf(stderr,"!ERROR !ca : dev[%d] : %s\n",i,ab_g_err_str);
			return;
		}

		ch = evt.ch;
		chan_idx = i * ab->chans_per_dev + evt.ch;

		/* got right event on right device type */
		if(	evt.id == ab_dev_event_FXO_RINGING &&
				cur_dev->type == ab_dev_type_FXO){
			if( g_verbose){
				fprintf(stdout,"[%d] << RING(or mute)\n",
						ab->chans[chan_idx].abs_idx);
			}
			/* make them a couple, 
			 * it emits error if channels in differ couples*/
			err = make_a_couple(chan, &ab->chans[chan_idx]);
			if(err){
				return;
			} 
			couple_has_been_found = 1;
			couple_idx = chan_idx;

			/* parse other events on this device */
			while(evt.more){
				err = ab_dev_event_get(cur_dev, &evt, &ca);
				if(err){
					fprintf(stderr,"!ERROR : %s\n",ab_g_err_str);
					return;
				}
				chan_idx = i * ab->chans_per_dev + evt.ch;
				/* event on unexpected channel */
				if(evt.ch != ch){
					fprintf(stderr,"!ERROR Event on wrong "
							"channel [%d]\n",
							ab->chans[chan_idx].abs_idx);
					return;
				}
				/* unexpected event on channel in couple */
				if(evt.id != ab_dev_event_FXO_RINGING){
					fprintf(stderr,"!ERROR Wrong event [%d/0x%lX] "
							"on channel [%d]\n",
							evt.id, evt.data,
							ab->chans[chan_idx].abs_idx);
					return;
				}
				if( g_verbose){
					fprintf(stdout,"[%d] << RING(or mute)\n",
							ab->chans[chan_idx].abs_idx);
				}
			}
		} else {
			fprintf(stderr,"!ERROR FXS[%d] >> RING and [%d]"
					" detcts [%d/0x%lX]\n",
					chan->abs_idx, ab->chans[chan_idx].abs_idx,
					evt.id, evt.data);
			return;
		}
	}/*}}}*/
	/* if no couple - tell about it {{{*/
	if( !couple_has_been_found){
		int rez;
		rez = find_couple(chan, NULL);
		if(rez != -1){
			fprintf(stdout,"!ERROR [%d] can`t detect events from [%d]\n", 
					rez, chan->abs_idx);
		} else {
			fprintf(stdout,"!ATT no couple found for [%d]\n", chan->abs_idx);
		}
		return;
	} /*}}}*/
}/*}}}*/

void
process_FXO(ab_chan_t * const chan)
{/*{{{*/
	ab_t * ab = chan->parent->parent;
	ab_dev_event_t evt;
	unsigned char ca;
	int i;
	int j;
	int err;
	int couple_has_been_found = 0;
	int couple_idx = -1;
	int chan_idx;
	int self_idx;
	int ch;

	for(self_idx=0;
			(self_idx < ab->chans_num) && 
			(chan->abs_idx != ab->chans[self_idx].abs_idx); 
			self_idx++);

	/* emits offhook {{{*/
	err = ab_FXO_line_hook(chan, ab_chan_hook_OFFHOOK);
	if( err){
		fprintf(stderr,"!ERROR : chan [%d] : %s\n",
				chan->abs_idx,ab_g_err_str);
		return;
	} else if( g_verbose){
		fprintf(stdout,"[%d] >> OFFHOOK\n",chan->abs_idx);
	}
	/*}}}*/

	usleep (WAIT_INTERVAL);

	/* test all devices on event get to find a couple (it can exists) {{{*/
	j=ab->devs_num;
	for (i=0; i<j; i++){
		ab_dev_t * cur_dev = &ab->devs[i];
		err = ab_dev_event_get (cur_dev, &evt, &ca);
		if(err){
			/* error happen on event getting ioctl */
			fprintf(stderr,"!ERROR : dev[%d] : %s\n",i,ab_g_err_str);
			return;
		} else if(evt.id == ab_dev_event_NONE){
			/* no event on current device */
			continue;
		}
		if( !ca){
			/* no channel number available */
			fprintf(stderr,"!ERROR !ca : dev[%d] : %s\n",i,ab_g_err_str);
			return;
		}

		ch = evt.ch;
		chan_idx = i * ab->chans_per_dev + evt.ch;
		couple_idx = chan_idx;

		/* got right event on right device type */
		if(	evt.id == ab_dev_event_FXS_OFFHOOK &&
				cur_dev->type == ab_dev_type_FXS){
			if( g_verbose){
				fprintf(stdout,"[%d] << OFFHOOK\n",
						ab->chans[chan_idx].abs_idx);
			}
			/* make them a couple, 
			 * it emits error if channels in differ couples*/
			err = make_a_couple (&ab->chans[chan_idx], chan);
			if(err){
				return;
			} 
			couple_has_been_found = 1;

			/* parse other events on this device */
			/* if they exist -> error */
			if(evt.more){
				err = ab_dev_event_get(cur_dev, &evt, &ca);
				if(err){
					fprintf(stderr,"!ERROR : %s\n",ab_g_err_str);
					return;
				}
				chan_idx = i * ab->chans_per_dev + evt.ch;
				
				/* event on unexpected channel */
				if(evt.ch != ch){
					fprintf(stderr,"!ERROR Wrong event [%d/0x%lX] on "
							"wrong channel [%d]\n",
							evt.id, evt.data,
							ab->chans[chan_idx].abs_idx);
					return;
				}
				/* unexpected event on channel in couple */
				fprintf(stderr,"!ERROR Wrong event [%d/0x%lX] "
						"on channel [%d]\n",
						evt.id, evt.data,
						ab->chans[chan_idx].abs_idx);
				return;
			}
		} else {
			fprintf(stderr,"!ERROR FXS[%d] >> OFFHOOK and [%d]"
					"detcts [%d/0x%lX]\n",
					chan->abs_idx, ab->chans[chan_idx].abs_idx,
					evt.id, evt.data);
			return;
		}
	} /*}}}*/
	/* if no couple - tell about it {{{*/
	if( !couple_has_been_found){
		int rez;
		rez = find_couple(NULL, chan);
		if(rez != -1){
			fprintf(stdout,"!ERROR [%d] can`t detect events from [%d]\n", 
					rez, chan->abs_idx);
		} else {
			fprintf(stdout,"!ATT no couple found for [%d]\n", chan->abs_idx);
		} /*}}}*/
	} else {
		/* then emits digits and onhook for this chan {{{*/
		/* remember the last state and set linefeed to active on all FXSchans */
		linefeed_keeper( ab, action_PUT);
		j=ab->chans_num;
		for (i=0; i<j; i++){
			if(ab->chans[i].parent->type == ab_dev_type_FXS){
				err = ab_FXS_line_feed(&ab->chans[i],ab_chan_linefeed_ACTIVE);
				if(err){
					fprintf(stderr,"!ERROR : set linefeed to active on [%d]\n",
							ab->chans[i].abs_idx);
					return;
				}
			}
		} 

		/* play digits and test events on couple chan */
		digits_test(chan, &ab->chans[couple_idx]);

		/* onhook on the tested chan */
		err = ab_FXO_line_hook(chan, ab_chan_hook_ONHOOK);
		if(err){
			fprintf(stderr,"!ERROR : set hook to onhook [%d]\n",
					ab->chans[couple_idx].abs_idx);
			return;
		} else if( g_verbose){
			fprintf(stdout,"[%d] >> ONHOOK\n",chan->abs_idx);
		}

		/* get onhook from couple */
		usleep(WAIT_INTERVAL);
		err = ab_dev_event_get (ab->chans[couple_idx].parent, &evt, &ca);
		if(err){
			/* error happen on event getting ioctl */
			fprintf(stderr,"!ERROR : chan[%d] : %s\n",
					ab->chans[couple_idx].abs_idx,ab_g_err_str);
			return;
		} else if(evt.id == ab_dev_event_NONE){
			/* no event on current device */
			fprintf(stderr,"!ERROR [%d] can`t dectect ONHOOK\n",
					ab->chans[couple_idx].abs_idx);
			return;
		}
		if( !ca){
			/* no channel number available */
			fprintf(stderr,"!ERROR !ca : chan[%d] : %s\n",
					ab->chans[couple_idx].abs_idx,ab_g_err_str);
			return;
		}
		chan_idx = (ab->chans[couple_idx].parent->idx - 1) * 
				ab->chans_per_dev + evt.ch;

		if((chan_idx != couple_idx) || (evt.id != ab_dev_event_FXS_ONHOOK)){
			/* got unexpected event or(and) the unexpected channel */
			fprintf(stderr,"!ERROR event [%d/0x%lX] on channel [%d]\n",
					evt.id, evt.data,
					ab->chans[chan_idx].abs_idx);
			return;
		} else if( g_verbose){
			fprintf(stdout,"[%d] << ONHOOK\n",
					ab->chans[chan_idx].abs_idx);
		}

		/* set linefeed to previous on all FXSchans */
		linefeed_keeper (ab, action_GET);
	}/*}}}*/
}/*}}}*/

int 
main (int argc, char *argv[])
{/*{{{*/
	ab_t * ab;
	int i;
	int j;

	if(argc==2){
		if( !strcmp (argv[1], "-v")){
			g_verbose = 1;
		} else{
			fprintf(stdout,"use -v for verbosity output\n");
			exit (EXIT_SUCCESS);
		}
	}

	/* init ab */
	ab = ab_create();
	if( !ab){
		fprintf(stderr,"!ERROR : %s\n",ab_g_err_str);
		exit (EXIT_FAILURE);
	}

	/* allocate memory for g_cps*/
	g_cps = malloc(sizeof(*g_cps)*ab->chans_num);
	if( !g_cps){
		fprintf(stderr,"!ERROR : Not enough memory for cps\n");
		ab_destroy(&ab);
		exit (EXIT_FAILURE);
	}

	usleep (WAIT_INTERVAL);

	/* for initial onhook events from FXO channels */
	j=ab->chans_num;
	for (i=0; i<j; i++){
		events_clean(&ab->chans[i]);
	}

	usleep (WAIT_INTERVAL);

	/* for every chan */
	j=ab->chans_num;
	for (i=0; i<j; i++){
		ab_chan_t * curr_chan = &ab->chans[i];
		if(curr_chan->parent->type == ab_dev_type_FXS){
			/* if FXS - process FXS */
			process_FXS(curr_chan);
		} else if(curr_chan->parent->type == ab_dev_type_FXO){
			/* if FXO - process FXO */
			process_FXO(curr_chan);
		}
	}

	/* destroy ab */
	ab_destroy(&ab);

	/* free g_cps memory */
	free (g_cps);

	return 0;
}/*}}}*/

