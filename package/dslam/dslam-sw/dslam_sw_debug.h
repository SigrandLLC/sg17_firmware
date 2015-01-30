#ifndef DEBUG_H
#define DEBUG_H

#undef DEBUG_ON
#define PDEBUG(lev,fmt,args...)
#ifdef DEBUG_ON
#       undef PDEBUG
#       define PDEBUG(lev,fmt,args...) \
		if( lev<DEFAULT_LEV ) \
			printk(KERN_ERR "DSLAM switch: %s " fmt " \n",__FUNCTION__, ## args  )
#endif

#endif		    
