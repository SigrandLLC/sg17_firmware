#ifndef SG_DEBUG_H
#define SG_DEBUG_H

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
		printk(KERN_NOTICE MAM17H_MODNAME": %s " fmt " \n",__FUNCTION__, ## args  )

#       undef PDEBUGL
#       define PDEBUGL(lev,fmt,args...)			\
	if( lev<=DEFAULT_LEV )						\
		printk(fmt, ## args  )

#endif

extern int debug_recv;
extern int debug_irq;
extern int debug_init;
extern int debug_netcard;
extern int debug_error;
extern int debug_fw;
extern int debug_rs_cmd;
extern int debug_remove;
extern int debug_socrate_init;
extern int debug_mpi_cmd;
extern int debug_load_cfg;
extern int debug_mpair;

#endif
