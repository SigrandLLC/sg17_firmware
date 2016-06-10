#ifndef SG_DEBUG_H
#define SG_DEBUG_H

#ifndef DEBUG_LEV
#	define DEBUG_LEV 0
#endif

#define PDEBUG(lev,fmt,args...)
#define PDEBUGL(lev,fmt,args...)
#ifdef DEBUG_ON
#       undef PDEBUG
#       define PDEBUG(lev,fmt,args...)									\
	if( lev<=DEBUG_LEV )												\
		printk(KERN_NOTICE "MS17E_V2(%s): " fmt " \n",__FUNCTION__, ## args  )

#       undef PDEBUGL
#       define PDEBUGL(lev,fmt,args...)			\
	if( lev<=DEBUG_LEV )						\
		printk(fmt, ## args  )

#endif

extern int debug_xmit;
extern int debug_recv;
extern int debug_irq;
extern int debug_init;
extern int debug_tty;
extern int debug_hw;
extern int debug_error;
extern int debug_sysfs;

#endif
