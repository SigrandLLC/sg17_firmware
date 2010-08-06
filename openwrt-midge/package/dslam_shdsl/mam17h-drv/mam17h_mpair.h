#ifndef MAM_17H_MPAIR_H
#define MAM_17H_MPAIR_H


#include "mam17h_pi.h"
#include "mam17h_main.h"
#include "mam17h_debug.h"

#define MPAIR01    0x00000100
#define MPAIR021   0x00010200
#define MPAIR0321  0x01020300
#define MPAIR03    0x01000000
#define MPAIR01_23 0x01000100

int is_chan_in_mpair(struct mam17_card * card, int ch);
int mpair_master(struct mam17_card *card);
int mpair_slave(struct mam17_card *card);

#endif
