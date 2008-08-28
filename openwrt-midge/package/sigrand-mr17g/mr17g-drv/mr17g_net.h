#include "mr17g.h"

// mr16g_ioctl
#define SIOCGLRATE	(SIOCDEVPRIVATE+14)

int __devinit mr17g_net_init(struct mr17g_chip *chip);
int __devexit mr17g_net_uninit(struct mr17g_chip *chip);
void mr17g_transceiver_setup(struct mr17g_channel *ch);
void mr17g_net_link(struct net_device *ndev);
// TODO: uncomment static
/*static */int mr17g_start_xmit( struct sk_buff *skb, struct net_device *ndev );
