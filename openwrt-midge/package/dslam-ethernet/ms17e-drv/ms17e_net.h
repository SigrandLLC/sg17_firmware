#ifndef MS17E_NET_H
#define MS17E_NET_H

#include <linux/netdevice.h>

struct net_local
{
	int number;
	struct device *dev;
	struct channel *chan_cfg;
	struct ms17e_card *card;
	struct regs_struct *regs;
};

int ms17e_net_init(struct ms17e_card *card);
int ms17e_net_remove(struct ms17e_card *card);

#endif
