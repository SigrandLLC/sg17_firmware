#ifndef MS17E_DEBUG_H
#define MS17E_DEBUG_H

#define DEBUG_ON
//#undef DEBUG_ON

#ifndef DEFAULT_LEV 
#	define DEFAULT_LEV 0
#endif

#define PDEBUG(lev,fmt,args...)
#define PDEBUGL(lev,fmt,args...)
#ifdef DEBUG_ON
#       undef PDEBUG
#       define PDEBUG(lev,fmt,args...)									\
	if( lev<=DEFAULT_LEV )												\
		printk(KERN_NOTICE MS17E_MODNAME": %s " fmt " \n",__FUNCTION__, ## args  )

#       undef PDEBUGL
#       define PDEBUGL(lev,fmt,args...)			\
	if( lev<=DEFAULT_LEV )						\
		printk(fmt, ## args  )

#endif



extern int debug_error;
extern int debug_probe;
extern int debug_remove;
extern int debug_net;
extern int debug_read_write;
extern int debug_interrupt;
extern int debug_read_poe_reg;
extern int debug_write_poe_reg;
extern int debug_monitor;

#endif
