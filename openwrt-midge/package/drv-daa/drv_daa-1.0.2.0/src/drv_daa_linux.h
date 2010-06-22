#ifndef _DRV_DAA_LINUX_H
#define _DRV_DAA_LINUX_H
/****************************************************************************
       Copyright (c) 2001, Infineon Technologies.  All rights reserved.

                               No Warranty
   Because the program is licensed free of charge, there is no warranty for
   the program, to the extent permitted by applicable law.  Except when
   otherwise stated in writing the copyright holders and/or other parties
   provide the program "as is" without warranty of any kind, either
   expressed or implied, including, but not limited to, the implied
   warranties of merchantability and fitness for a particular purpose. The
   entire risk as to the quality and performance of the program is with
   you.  should the program prove defective, you assume the cost of all
   necessary servicing, repair or correction.

   In no event unless required by applicable law or agreed to in writing
   will any copyright holder, or any other party who may modify and/or
   redistribute the program as permitted above, be liable to you for
   damages, including any general, special, incidental or consequential
   damages arising out of the use or inability to use the program
   (including but not limited to loss of data or data being rendered
   inaccurate or losses sustained by you or third parties or a failure of
   the program to operate with any other programs), even if such holder or
   other party has been advised of the possibility of such damages.
 ****************************************************************************
   Module      : drv_daa_linux.h
   Date        : 2006-11-14
   Description : This file contains the includes and the defines
                 specific to the linux OS
   Remarks     : Please use the compiler switches here if you have
                 more than one OS.
*******************************************************************************/

/* ============================= */
/* Global Includes               */
/* ============================= */

/* Linux Includes*/

#ifdef __KERNEL__
#include <linux/kernel.h>
#endif

#ifdef MODULE
#include <linux/module.h>
#endif

/*several includes*/
#include <linux/fs.h>
#include <linux/errno.h>
/*proc-file system*/
#include <linux/proc_fs.h>
/*polling */
#include <linux/poll.h>
/* check-request-release region*/
#include <linux/ioport.h>
/*ioremap-kmalloc*/
#include <linux/vmalloc.h>
/* mdelay - udelay */
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/param.h>
#include <linux/timer.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <asm/semaphore.h>
/* Interrupts */
#include <asm/irq.h>
/* ppc includes */


#define START_ABORT_TIMER       (round = jiffies);
#define IS_ABORT_TIMER_EXPIRED  ((jiffies - round) > WAIT*HZ/1000) /* = WAIT ms  */

/* timer corresponding declaration */
typedef struct timer_list   TIMER_ID;

#define INIT_TIMER          init_timer
#define TIMER_ADD           add_timer

/* interrupt lock */
#define INT_LOCK_T          spinlock_t


/* ============================= */
/* Global Structures             */
/* ============================= */

/* ============================= */
/* Global function declaration   */
/* ============================= */

#endif /* _DRV_DAA_LINUX_H */


