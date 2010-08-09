#ifndef MAM_17H_NET_H
#define MAM_17H_NET_H


#include <linux/netdevice.h>
#include "mam17h_main.h"

struct net_local
{
	int number;
	struct device *dev;
	struct channel *chan_cfg;
	struct mam17_card *card;
	struct regs_str *regs;
};


int mam17_net_init(struct mam17_card *card);

#endif
