#ifndef _KTHREAD_H
#define _KTHREAD_H
#include <linux/config.h>
#include <linux/version.h>

#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/wait.h>

#include <asm/unistd.h>
#include <asm/semaphore.h>

#include "ifx_types.h"

#ifndef KERNEL_VERSION
   #define KERNEL_VERSION(a,b,c) (((a) << 16) + ((b) << 8) + (c))
#endif

/* Maximum lenght of kernel thread name */
#define KTHREAD_NAME_LEN 16

/* a structure to store all information we need
   for our thread */
typedef struct kthread_struct
{
   /* Completion struct for thread completion detection */
   struct completion thrCompletion;
   /* A name of the kernel thread */
   IFX_char_t        name[KTHREAD_NAME_LEN];
   /* "Thread is running" flag */
   IFX_boolean_t     running;
   /* "Terminate thread" signalling flag */
   IFX_boolean_t     terminate;
   /* Kernel thread process ID */
   IFX_int32_t       pid;
   /* Function activated on thread creation */
   void              (*function)(struct kthread_struct *kthread);
   /* Argument for the "function" */
   void              *arg;

} kthread_t;


/* prototypes */

/* start new kthread (called by creator) */
void start_kthread(void (*func)(kthread_t *), kthread_t *kthread, char *name);

/* stop a running thread (called by "killer") */
void stop_kthread(kthread_t *kthread);

/* setup thread environment (called by new thread) */
void init_kthread(kthread_t *kthread);

/* cleanup thread environment (called by thread upon receiving termination signal) */
void exit_kthread(kthread_t *kthread);

#endif
