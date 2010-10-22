#ifndef MAM_17H_NET_H
#define MAM_17H_NET_H


#include <linux/netdevice.h>
#include "mam17h_main.h"

struct statistics_all
{
	unsigned int es, ses, crc_anom, losws, uas, es_old, ses_old,
		crc_anom_old, losws_old, uas_old, to_line, from_mii,
		tx_aborted, oversized, error_marked, from_line, crce,
		 rx_aborted, invalid_frames, to_mii, to_miie, overflow;
};

struct net_local
{
	int number;
	struct device *dev;
	struct channel *chan_cfg;
	struct mam17_card *card;
	struct regs_str *regs;
	struct statistics_all stat;
};


int mam17_net_init(struct mam17_card *card);

#endif
