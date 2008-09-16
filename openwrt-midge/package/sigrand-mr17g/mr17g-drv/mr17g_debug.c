/* mr17g_debug.c
 *  Sigrand MR17G E1 PCI adapter driver for linux (kernel 2.6.x)
 *
 *	Written 2008 by Artem Y. Polyakov (artpol84@gmail.com)
 *
 *	This driver presents MR17G modem to OS as common hdlc interface.
 *
 *	This software may be used and distributed according to the terms
 *	of the GNU General Public License.
 *
 */

int debug_xmit=40;
int debug_recv=40;
int debug_irq=40;
int debug_sci=40;
int debug_init=40;
int debug_pef=40;
int debug_cur=50;
int debug_net=40;
int debug_link=40;
int debug_error=0;
int debug_sysfs=40;
