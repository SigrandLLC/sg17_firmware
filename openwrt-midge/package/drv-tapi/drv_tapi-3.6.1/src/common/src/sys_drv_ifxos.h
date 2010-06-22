#ifndef _SYS_DRV_IFXOS_H
#define _SYS_DRV_IFXOS_H
/******************************************************************************
                               Copyright (c) 2007
                            Infineon Technologies AG
                     Am Campeon 1-12; 81726 Munich, Germany

  THE DELIVERY OF THIS SOFTWARE AS WELL AS THE HEREBY GRANTED NON-EXCLUSIVE,
  WORLDWIDE LICENSE TO USE, COPY, MODIFY, DISTRIBUTE AND SUBLICENSE THIS
  SOFTWARE IS FREE OF CHARGE.

  THE LICENSED SOFTWARE IS PROVIDED "AS IS" AND INFINEON EXPRESSLY DISCLAIMS
  ALL REPRESENTATIONS AND WARRANTIES, WHETHER EXPRESS OR IMPLIED, INCLUDING
  WITHOUT LIMITATION, WARRANTIES OR REPRESENTATIONS OF WORKMANSHIP,
  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, DURABILITY, THAT THE
  OPERATING OF THE LICENSED SOFTWARE WILL BE ERROR FREE OR FREE OF ANY THIRD
  PARTY CLAIMS, INCLUDING WITHOUT LIMITATION CLAIMS OF THIRD PARTY INTELLECTUAL
  PROPERTY INFRINGEMENT.

  EXCEPT FOR ANY LIABILITY DUE TO WILFUL ACTS OR GROSS NEGLIGENCE AND EXCEPT
  FOR ANY PERSONAL INJURY INFINEON SHALL IN NO EVENT BE LIABLE FOR ANY CLAIM
  OR DAMAGES OF ANY KIND, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
  ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
  DEALINGS IN THE SOFTWARE.

******************************************************************************
   Module      : sys_drv_ifxos.h
   Description : This file contains the includes and the defines
                 specific to the OS
   Remarks     :
      The driver makes use of the following system resources:
      - malloc and free of memory
      - interrupt functions
      - timers
      - mutex semaphores
      - semaphores or events for internal communication
      - queues/semaphores for external waiting tasks in case of select
        or polling call
      - wait (with scheduling) and delay (without scheduling) functions
      - system time function to retrieve the system time
      - debug print out
      Some features are implemented in a project specific C module, as for
      example timers and interrupt related functions.
*******************************************************************************/

/** \file
SYS_DRV_IFXOS: operating system abstraction
*/

/* ============================= */
/* Global Includes               */
/* ============================= */

/* include operating system specific files */
#ifdef VXWORKS
   #include <vxWorks.h>
   #include <selectLib.h>
   #include <iosLib.h>
   #include <stdlib.h>
   #include <stdio.h>
   #include <semLib.h>
   #include <time.h>
   #include <taskLib.h>
   #include <cacheLib.h>
   #include <logLib.h>
   #include <intLib.h>
   #include <string.h>
   #include <sys/ioctl.h>
   #include <tickLib.h>
   #include <netinet/in.h> /* ntohs(), etc. */
   #include "sys_drv_delaylib_vxworks.h"

/*   #include <drv/timer/timerDev.h>*/
   /* use direct declaration instead of header */
   IMPORT	int	sysClkRateGet (void);

#if 0
   /* Math routines should not be used at driver level!
      Add to your project files if necessary! */
   #include <math.h>
#endif

#endif /* VXWORKS */
#ifdef LINUX
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
   #include <linux/interrupt.h>
   #include <asm/uaccess.h>
   #include <asm/io.h>
   #include <asm/semaphore.h>
   /* little / big endian */
   #include <asm/byteorder.h>
   /* Interrupts */
   #include <linux/irq.h>
   #include <linux/list.h>
   #include <asm/system.h>
#endif /* LINUX */
#ifdef WINDOWS
   #include <stdio.h>
   #include <stdlib.h>
   #include <memory.h>
   #include <windows.h>
   #include <winbase.h>
   #include <WinSock.h>
#endif /* WINDOWS */
#ifdef NO_OS
   #include <sys_noOSLib.h>
#endif /* NO_OS */
#ifdef OSE
   #warning not defined
#endif /* OSE */
#ifdef XAPI
   #include <xapi.h>
   #include <xapi_extension.h>
#endif /* XAPI */

/******************************************************************************
                                 Basics
******************************************************************************/

/** \defgroup SYS_DRV_IFXOS_BASIC  Basic Macros
*/
/**@{*/

#ifdef DEBUG
   /* debug feature for memory tracking */
   #define IFXOS_MALLOC(size)    sys_dbgMalloc(size, __LINE__, __FILE__)
   #define IFXOS_FREE(ptr)       sys_dbgFree((void*)ptr, __LINE__, __FILE__);ptr = NULL
   #define IFXOS_VMALLOC(size) vmalloc(size)
   #define IFXOS_VFREE(ptr) if (ptr != NULL) {vfree((void*)ptr); ptr = NULL;}
   #ifdef LINUX
      /** allocate memory for debug feature */
      #define sysdebug_malloc(size) \
         (in_interrupt()?kmalloc(size, GFP_ATOMIC):kmalloc(size, GFP_KERNEL))
      /** free memory */
      #define sysdebug_free(ptr)       if (ptr != NULL) {kfree((void*)ptr); ptr = NULL;}
   #elif defined (XAPI)
      /** allocate memory */
      #define sysdebug_malloc(size)    xapi_malloc (size)
      /** free memory */
      #define sysdebug_free(ptr)       if (ptr != NULL) {xm_dealloc(((void*)ptr); ptr = NULL;}
   #elif defined (NO_OS)
      /** allocate memory */
      #define sysdebug_malloc(size)    malloc(size)
      /** free memory */
      #define sysdebug_free(ptr)       if (ptr != NULL) {free((void*)ptr); ptr = NULL;}
   #else /* XAPI */
      /** allocate memory */
      #define sysdebug_malloc(size)    malloc (size)
      /** free memory */
      #define sysdebug_free(ptr)       if (ptr != NULL) {free((void*)ptr); ptr = NULL;}
   #endif /* LINUX */
#else /* DEBUG */
   #ifdef LINUX /* LINUX */
      /** allocate memory */
      #define IFXOS_MALLOC(size) \
         (in_interrupt()?kmalloc(size, GFP_ATOMIC):kmalloc(size, GFP_KERNEL))
      /** free memory */
      #define IFXOS_FREE(ptr)       if (ptr != NULL) {kfree((void*)ptr); ptr = NULL;}
      #define IFXOS_VMALLOC(size) vmalloc(size)
      #define IFXOS_VFREE(ptr) if (ptr != NULL) {vfree((void*)ptr); ptr = NULL;}
   #elif defined (NO_OS)
      /** allocate memory */
      #define IFXOS_MALLOC(size)    malloc(size)
      /** free memory */
      #define IFXOS_FREE(ptr)       if (ptr != NULL) {free((void*)ptr); ptr = NULL;}
   #elif defined (XAPI)
      /** allocate memory */
      #define IFXOS_MALLOC(size)    xapi_malloc (size)
      /** free memory */
      #define IFXOS_FREE(ptr)       if (ptr != NULL) {xm_dealloc((void*)ptr); ptr = NULL;}
   #else /* XAPI */
      /** allocate memory */
      #define IFXOS_MALLOC(size)    malloc (size)
      /** free memory */
      #define IFXOS_FREE(ptr)       if (ptr != NULL) {free((void*)ptr); ptr = NULL;}
      #define IFXOS_VMALLOC(size)   malloc(size)
      #define IFXOS_VFREE(ptr)      if (ptr != NULL) {free((void*)ptr); ptr = NULL;}
   #endif /* LINUX */

#endif /* DEBUG */

/** For local functions */
#ifndef IFX_LOCAL
#define IFX_LOCAL  static
#endif

#ifdef DEBUG
#ifdef LINUX
/** assert in debug code
\param expr - expression to be evaluated. If expr != TRUE assert is printed
              out with line number */
#define IFXOS_ASSERT(expr) \
	if(!(expr)) { \
		printk ( "\n\r" __FILE__ ":%d: Assertion " #expr " failed!\n\r",__LINE__); \
	}
#elif defined (VXWORKS)
/** assert in debug code
\param expr - expression to be evaluated. If expr != TRUE assert is printed
              out with line number */
#define IFXOS_ASSERT(expr) \
   if(!(expr)) { \
      logMsg ( "\n" __FILE__ ":%d: Assertion " #expr " failed!\n\r",__LINE__,0,0,0,0,0); \
   }
#elif defined (NO_OS)
/** assert in debug code
\param expr - expression to be evaluated. If expr != TRUE assert is printed
              out with line number */
#define IFXOS_ASSERT(expr) \
   if(!(expr)) { \
      printf ( "\n\r" __FILE__ ":%d: Assertion " #expr " failed!\n\r",__LINE__); \
   }
#elif defined (XAPI)
/** assert in debug code
\param expr - expression to be evaluated. If expr != TRUE assert is printed
              out with line number */
#define IFXOS_ASSERT(expr) \
   if(!(expr)) { \
      xapi_logMsg ( "\n" __FILE__ ":%d: Assertion " #expr " failed!\n\r",__LINE__,0,0,0,0,0); \
   }
#else /* unknown/other systems */
#ifdef ASSERT
/* if they have an own assert macro: */
#define IFXOS_ASSERT(expr)    ASSERT(expr)
#else
#define IFXOS_ASSERT(expr) \
	if(!(expr)) { \
		printf ( "\n\r" __FILE__ ":%d: Assertion " #expr " failed!\n\r",__LINE__); \
	}
#endif /* ASSERT */
#endif /* OS */
#else /* DEBUG */
/** assert in debug code
\param expr - expression to be evaluated. If expr != TRUE assert is printed
              out with line number */
#define IFXOS_ASSERT(expr)
#endif /* DEBUG */

/** time stamp type */
typedef int IFXOS_TIMESTAMP;


#ifdef VXWORKS
/** returns magic number of ioctl command, see <sys/ioctl.h>.
\param iocmd - ioctl command of which magic number is decoded */
#define IFXOX_IO_GETMAGIC(iocmd) \
          (((iocmd) >> 8) & 0xFF)
#endif /* VXWORKS */
#ifdef LINUX
/** returns magic number of ioctl command, see <asm/ioctl.h>.
\param iocmd - ioctl command of which magic number is decoded */
#define IFXOX_IO_GETMAGIC(iocmd) \
                 _IOC_TYPE((iocmd))
#endif /* LINUX */
#ifdef WINDOWS
/** returns magic number of ioctl command, see <sys/ioctl.h>.
\param iocmd - ioctl command of which magic number is decoded */
#define IFXOX_IO_GETMAGIC(iocmd) \
          (((iocmd) >> 8) & 0xFF)
#endif /* WINDOWS */
#ifdef NO_OS
/** returns magic number of ioctl command, see <sys/ioctl.h>.
\param iocmd - ioctl command of which magic number is decoded */
#define IFXOX_IO_GETMAGIC(iocmd) \
          (((iocmd) >> 8) & 0xFF)
#endif /* NO_OS */
#ifdef OSE
   #warning not defined
#endif /* OSE */
#ifdef XAPI
/** returns magic number of ioctl command.
\param iocmd - ioctl command of which magic number is decoded */
#define IFXOX_IO_GETMAGIC(iocmd) \
          (((iocmd) >> 8) & 0xFF)
#endif /* XAPI */

/*@}*/

/******************************************************************************
                                Hardware related
******************************************************************************/
/** \defgroup SYS_DRV_IFXOS_HW  Hardware Related
 */
/**@{*/

/** Gets the processors base address. Avoid to use! */
#define IFXOS_UC_BASE

#ifdef VXWORKS
   #ifndef __LITTLE_ENDIAN
   #define __LITTLE_ENDIAN _LITTLE_ENDIAN
   #endif
   #ifndef __BIG_ENDIAN
   #define __BIG_ENDIAN    _BIG_ENDIAN
   #endif

   #define IFXOS_LITTLE_ENDIAN  _LITTLE_ENDIAN
   #define IFXOS_BIG_ENDIAN    _BIG_ENDIAN

   #if (_BYTE_ORDER == _LITTLE_ENDIAN)
      /** byte order is little endian */
      #define __BYTE_ORDER __LITTLE_ENDIAN
      #define IFX_BYTE_ORDER IFXOS_LITTLE_ENDIAN
   #elif (_BYTE_ORDER == _BIG_ENDIAN )
      /** byte order is big endian */
      #define __BYTE_ORDER __BIG_ENDIAN
      #define IFX_BYTE_ORDER IFXOS_BIG_ENDIAN
   #else
      #error "Unknown Byteorder!"
   #endif
#endif /* VXWORKS */
#ifdef LINUX
   #define IFXOS_LITTLE_ENDIAN  __LITTLE_ENDIAN
   #define IFXOS_BIG_ENDIAN     __BIG_ENDIAN

   #if defined ( __LITTLE_ENDIAN )
      #define __BYTE_ORDER __LITTLE_ENDIAN
      #define IFX_BYTE_ORDER IFXOS_LITTLE_ENDIAN
   /** byte order is big endian */
   #elif defined ( __BIG_ENDIAN )
      #define __BYTE_ORDER __BIG_ENDIAN
      #define IFX_BYTE_ORDER IFXOS_BIG_ENDIAN
   #endif
   /* as for VxWORKS the code expects both defines */
   #ifndef __LITTLE_ENDIAN
   #define __LITTLE_ENDIAN 1234
   #endif
   #ifndef __BIG_ENDIAN
   #define __BIG_ENDIAN    4321
   #endif
#endif /* LINUX */
#ifdef WINDOWS
   #define IFXOS_LITTLE_ENDIAN 4321
   #define IFXOS_BIG_ENDIAN 1234
   #define IFX_BYTE_ORDER IFXOS_LITTLE_ENDIAN
   #define __LITTLE_ENDIAN 4321
   #define __BIG_ENDIAN 1234
   #define __BYTE_ORDER __LITTLE_ENDIAN
#endif /* WINDOWS */
#ifdef NO_OS
   #ifdef __BYTE_ORDER
   #undef __BYTE_ORDER
   #endif
   /* add endianness to your compiler switches */
   #ifdef NO_OS_LITTLE_ENDIAN
      /** byte order is little endian */
      #define __BYTE_ORDER __LITTLE_ENDIAN
   #elif defined (NO_OS_BIG_ENDIAN)
      /** byte order is big endian */
      #define __BYTE_ORDER __BIG_ENDIAN
   #else
      #error "Unknown Byteorder!"
   #endif
#endif /* NO_OS */
#ifdef OSE
   #warning not defined
#endif /* OSE */
#ifdef XAPI
   #ifndef __LITTLE_ENDIAN
   #define __LITTLE_ENDIAN _LITTLE_ENDIAN
   #endif
   #ifndef __BIG_ENDIAN
   #define __BIG_ENDIAN    _BIG_ENDIAN
   #endif

   #define IFXOS_LITTLE_ENDIAN  _LITTLE_ENDIAN
   #define IFXOS_BIG_ENDIAN    _BIG_ENDIAN

   #if (_BYTE_ORDER == _LITTLE_ENDIAN)
      /** byte order is little endian */
      #define __BYTE_ORDER __LITTLE_ENDIAN
      #define IFX_BYTE_ORDER IFXOS_LITTLE_ENDIAN
   #elif (_BYTE_ORDER == _BIG_ENDIAN )
      /** byte order is big endian */
      #define __BYTE_ORDER __BIG_ENDIAN
      #define IFX_BYTE_ORDER IFXOS_BIG_ENDIAN
   #else
      #error "Unknown Byteorder!"
   #endif
#endif /* XAPI */
/*@}*/

/** \defgroup SYS_DRV_IFXOS_INT Interrupt Handling
*/
/**@{*/

#ifdef VXWORKS
/** type for interrupt status store used in IFXOS_LOCKINT */
typedef int IFXOS_INTSTAT;
#endif /* VXWORKS */
#ifdef WINDOWS
/** type for interrupt status store used in IFXOS_LOCKINT */
typedef int IFXOS_INTSTAT;
#endif /* WINDOWS */
#ifdef LINUX
/** type for interrupt status store used in IFXOS_LOCKINT */
typedef unsigned long  IFXOS_INTSTAT;
#endif /* LINUX */
#ifdef NO_OS
typedef INT IFXOS_INTSTAT;
#endif /* NO_OS */
#ifdef OSE
   #warning not defined
#endif /* OSE */
#ifdef XAPI
typedef unsigned int IFXOS_INTSTAT;
#endif /* XAPI */

/** Determine if the current stat is in interrupt or task contect
 */
#ifdef VXWORKS
#define IFXOS_IN_INTERRUPT() \
   ((intContext() == TRUE) ? IFX_TRUE : IFX_FALSE)
#endif /* VXWORKS */
#ifdef WINDOWS
#define IFXOS_IN_INTERRUPT() \
   IFX_FALSE
#endif /* WINDOWS */
#ifdef LINUX
#define IFXOS_IN_INTERRUPT() \
   (in_interrupt() ? IFX_TRUE : IFX_FALSE)
#endif /* LINUX */
#ifdef NO_OS
   #warning not defined
#endif /* NO_OS */
#ifdef OSE
   #warning not defined
#endif /* OSE */
#ifdef XAPI
   #warning not defined
#endif /* XAPI */

#ifdef VXWORKS
/** Lock interrupt handling
\param var - status variable of type IFXOS_INTSTAT
 */
#define IFXOS_LOCKINT(var) \
   var = intLock()
#endif /* VXWORKS */
#ifdef WINDOWS
#define IFXOS_LOCKINT(var)

#endif /* WINDOWS */
#ifdef LINUX
#define IFXOS_LOCKINT(var) \
   local_irq_save(var)
#endif /* LINUX */
#ifdef NO_OS
#define IFXOS_LOCKINT(var) \
   var = No_OS_LockInt()
#endif /* NO_OS */
#ifdef OSE
   #warning not defined
#endif /* OSE */
#ifdef XAPI
#define IFXOS_LOCKINT(var) \
   var = 0, xi_mask()
#endif /* XAPI */

/** Unlock interrupt handling
\param var - interrupt status variable of type IFXOS_INTSTAT
 */
#ifdef VXWORKS
#define IFXOS_UNLOCKINT(var) \
   intUnlock(var)
#endif /* VXWORKS */
#ifdef WINDOWS
#define IFXOS_UNLOCKINT(var)

#endif /* WINDOWS */
#ifdef LINUX
#define IFXOS_UNLOCKINT(var) \
   local_irq_restore(var)
#endif /* LINUX */
#ifdef NO_OS
#define IFXOS_UNLOCKINT(var) \
   No_OS_UnlockInt(var)
#endif /* NO_OS */
#ifdef OSE
   #warning not defined
#endif /* OSE */
#ifdef XAPI
#define IFXOS_UNLOCKINT(var) \
   xi_umask()
#endif /* XAPI */


/** Disable interrupts.
\param irq - interrupt number
 */
#ifdef VXWORKS
#define IFXOS_IRQ_DISABLE(irq)      \
   intDisable(irq)
#endif /* VXWORKS */
#ifdef LINUX
#define IFXOS_IRQ_DISABLE(irq)      \
   disable_irq(irq)
#endif /* LINUX */
#ifdef WINDOWS
#define IFXOS_IRQ_DISABLE(irq)
#endif /* WINDOWS */
#ifdef NO_OS
#define IFXOS_IRQ_DISABLE(irq)      \
   No_OS_IntDisable(irq)
#endif /* NO_OS */
#ifdef OSE
   #warning not defined
#endif /* OSE */
#ifdef XAPI
#define IFXOS_IRQ_DISABLE(irq)      \
   xapi_irq_disable(irq)
#endif /* XAPI */

/** Enable interrupts
\param irq - interrupt status variable of type IFXOS_INTSTAT
 */
#ifdef VXWORKS
#define IFXOS_IRQ_ENABLE(irq)      \
   intEnable(irq)
#endif /* VXWORKS */
#ifdef LINUX
#define IFXOS_IRQ_ENABLE(irq)      \
   enable_irq (irq)
#endif /* LINUX */
#ifdef WINDOWS
#define IFXOS_IRQ_ENABLE(irq)
#endif /* WINDOWS */
#ifdef NO_OS
#define IFXOS_IRQ_ENABLE(irq)      \
   No_OS_IntEnable (irq)
#endif /* NO_OS */
#ifdef OSE
   #warning not defined
#endif /* OSE */
#ifdef XAPI
#define IFXOS_IRQ_ENABLE(irq)      \
   xapi_irq_enable(irq)
#endif /* XAPI */

/** lock task switching
 */
#ifdef VXWORKS
#define IFXOS_TASK_LOCK() \
   taskLock()
#endif /* VXWORKS */
#ifdef WINDOWS

#endif /* WINDOWS */
#ifdef LINUX
#define IFXOS_TASK_LOCK()
   /* nothing */
#endif /* LINUX */
#ifdef NO_OS
#define IFXOS_TASK_LOCK()
#endif /* NO_OS */
#ifdef OSE
   #warning not defined
#endif /* OSE */
#ifdef XAPI
#define IFXOS_TASK_LOCK() \
   xt_entercritical()
#endif /* XAPI */

/** unlock task switching
*/
#ifdef VXWORKS
#define IFXOS_TASK_UNLOCK() \
   taskUnlock()
#endif /* VXWORKS */
#ifdef WINDOWS

#endif /* WINDOWS */
#ifdef LINUX
#define IFXOS_TASK_UNLOCK()
#endif /* LINUX */
#ifdef NO_OS
#define IFXOS_TASK_UNLOCK()
#endif /* NO_OS */
#ifdef OSE
   #warning not defined
#endif /* OSE */
#ifdef XAPI
#define IFXOS_TASK_UNLOCK() \
   xt_exitcritical()
#endif /* XAPI */
/*@}*/


/******************************************************************************
                                 Time related
******************************************************************************/
/** \defgroup SYS_DRV_IFXOS_TIME Time Related
   */
/**@{*/

/** Delay excution with task schedule
\param time - time to wait in ms */
#ifdef VXWORKS
#define IFXOS_Wait(time)                     \
   taskDelay((time==0)?0:               \
   (time <= (1000/sysClkRateGet()))? \
   1:time * sysClkRateGet() / 1000)
#endif /* VXWORKS */
#ifdef LINUX
#define IFXOS_Wait(time)   \
   do { \
		current->state = TASK_INTERRUPTIBLE; \
		schedule_timeout(HZ * (time) / 1000);\
   } while(0)
#endif /* LINUX */
#ifdef WINDOWS
#define IFXOS_Wait(time)   \
   Sleep(time)
#endif /* WINDOWS */
#ifdef NO_OS
#define IFXOS_Wait(time)   \
   No_OS_Wait(time)
#endif /* NO_OS */
#ifdef OSE
   #warning not defined
#endif /* OSE */
#ifdef XAPI
#define IFXOS_Wait(time)                     \
   xtm_wkafter(time)
#endif /* XAPI */

#ifndef IFXOS_DELAYMS
/** Short active delay in milli seconds without schedule
\param time - wait time in ms
 */
#ifdef VXWORKS
#define IFXOS_DELAYMS(time)           \
   delayMsec (time)
#endif /* VXWORKS */
#ifdef LINUX
#define IFXOS_DELAYMS(time)           \
   mdelay(time)
#endif /* LINUX */
#ifdef WINDOWS
#define IFXOS_DELAYMS(time)           \
   Sleep(time)
#endif /* WINDOWS */
#ifdef NO_OS
#define IFXOS_DELAYMS(time)           \
   No_OS_Wait_ms(time)
#endif /* NO_OS */
#ifdef OSE
   #warning not defined
#endif /* OSE */
#ifdef XAPI
#define IFXOS_DELAYMS(time)           \
   xtm_wkafter (time)
#endif /* XAPI */
#endif /* IFXOS_DELAYMS */

#ifndef IFXOS_DELAYUS
/** Short active delay in micro seconds without schedule
\param time - wait time in us
 */
#ifdef VXWORKS
#define IFXOS_DELAYUS(time)      \
   delayUsec (time)
#endif /* VXWORKS */
#ifdef LINUX
#define IFXOS_DELAYUS(time)      \
   udelay(time)
#endif /* LINUX */
#ifdef WINDOWS
#define IFXOS_DELAYUS(time)      \
   Sleep(time / 1000)
#endif /* WINDOWS */
#ifdef NO_OS
#define IFXOS_DELAYUS(time)      \
   No_OS_Wait_us(time)
#endif /* NO_OS */
#ifdef OSE
   #warning not defined
#endif /* OSE */
#ifdef XAPI
#define IFXOS_DELAYUS(time)      \
   xtm_wkafter(time/1000) /* only msec supported */
#endif /* XAPI */
#endif /* IFXOS_DELAYUS */

/** Returns system tick in milliseconds
Maybe used to measure roughly times for testing
\return system tick in milliseconds  */
#ifdef VXWORKS
#define IFXOS_GET_TICK() \
   (IFX_uint32_t)(tickGet() * 1000 / sysClkRateGet())
#endif /* VXWORKS */
#ifdef LINUX
#define IFXOS_GET_TICK()      \
      (IFX_uint32_t)(jiffies * 1000 / HZ)
#endif /* LINUX */
#ifdef WINDOWS
#define IFXOS_GET_TICK()      \
   (IFX_uint32_t)Get_Tick())
#endif /* WINDOWS */
#ifdef NO_OS
#define IFXOS_GET_TICK()      \
   No_OS_GetTickms()
#endif /* NO_OS */
#ifdef OSE
   #warning not defined
#endif /* OSE */
#ifdef XAPI
#define IFXOS_GET_TICK() \
   (IFX_uint32_t)(xtm_gettime())
#endif /* XAPI */
/*@}*/


/******************************************************************************
                     Events for internal driver communication
******************************************************************************/
/** \defgroup SYS_DRV_IFXOS_SYNC Event Macros
Events are used for the communication between high priority tasks or
interrupts and other tasks (e.g: to signalize a task sleeping on an event
that the event has occurred).
 Event macros are used for driver internal commonication. For example
a driver call can wait on an interrupt.
If the event has occured before the macros does not wait. No reseting
function is available.
\code
   typedef struct {
      ...
      IFXOS_event_t           wqRead;
   }XXX_DEV;

   // function waiting on the event
   readData(XXX_DEV* pDev)
   {
      ...
      IFXOS_InitEvent (pDev->evtRead);
      // wait for the event
      if (IFXOS_WaitEvent_timeout(pDev->evtRead, MBX_RD_TIMEOUT) == IFX_ERROR)
      {
         // timeout has expired without arrival of data
         return 0;
      }
      ...
   }

   // interrupt handler which wakes up the event
   void intHandle(XXX_DEV* pDev)
   {
      ...
      // ok, data available, wake up waiting function
      IFXOS_WakeUpEvent (pDev->evtRead);
   }
 \endcode
   */
/**@{*/

#ifdef VXWORKS
/** waits forever */
#define IFXOS_WAIT_FOREVER WAIT_FOREVER
/** never wait */
#define IFXOS_NOWAIT       NO_WAIT
#endif /* VXWORKS */
#ifdef LINUX
/** waits forever */
#define IFXOS_WAIT_FOREVER 0xFFFF
/** never wait */
#define IFXOS_NOWAIT       0
#endif /* LINUX */
#ifdef WINDOWS
/** waits forever */
#define IFXOS_WAIT_FOREVER INFINITE
/** never wait */
#define IFXOS_NOWAIT       0
#endif /* WINDOWS */
#ifdef NO_OS
/** waits forever */
#define IFXOS_WAIT_FOREVER WAIT_FOREVER
/** never wait */
#define IFXOS_NOWAIT       NO_WAIT
#endif /* NO_OS */
#ifdef XAPI
/** waits forever */
#define IFXOS_WAIT_FOREVER (0)
/** never wait */
#define IFXOS_NOWAIT       (-1)
#endif /* XAPI */

/** Type of event */
#ifdef VXWORKS
typedef SEM_ID                         IFXOS_event_t;
#endif /* VXWORKS */
#ifdef WINDOWS
typedef HANDLE                         IFXOS_event_t;
#endif /* WINDOWS */
#ifdef LINUX
typedef wait_queue_head_t              IFXOS_event_t;
#endif /* LINUX */
#ifdef NO_OS
typedef NO_OS_EVENT                    IFXOS_event_t;
#endif /* NO_OS */
#ifdef OSE
   #warning not defined
#endif /* OSE */
#ifdef XAPI
typedef unsigned long                   IFXOS_event_t;
#endif /* XAPI */

/** Initialize an event. No return
\param ev - event of type IFXOS_event_t
*/
#ifdef IFXOS_NOTDEFINED
#define OS_InitEvent(ev)
#endif /* IFXOS_NOTDEFINED */
#ifdef VXWORKS
#define IFXOS_InitEvent(ev) \
   (ev) = semBCreate(SEM_Q_FIFO, SEM_EMPTY)
#endif /* VXWORKS */
#ifdef LINUX
#define IFXOS_InitEvent(ev) \
   init_waitqueue_head (&(ev))
#endif /* LINUX */
#ifdef WINDOWS
#define IFXOS_InitEvent(ev)      \
   (ev) = CreateEvent(NULL, FALSE, FALSE, NULL)
#endif /* WINDOWS */
#ifdef NO_OS
#define IFXOS_InitEvent(ev)      \
   No_OS_InitEvent (&ev, FALSE)
#endif /* NO_OS */
#ifdef OSE
   #warning not defined
#endif /* OSE */
#ifdef XAPI
#define IFXOS_InitEvent(ev) \
   xsm_create( "IFXS", 1L, SM_FIFO, &ev)
#endif /* XAPI */


/** Signal an event. After signaling the state is reset
\param ev - event
\return IFX_SUCCESS if success, otherwise IFX_ERROR
 */
#ifdef LINUX
#define IFXOS_WakeUpEvent(ev)    \
   wake_up_interruptible(&(ev))
#endif /* LINUX */
#ifdef VXWORKS
#define IFXOS_WakeUpEvent(ev)    \
   semGive(ev)
#endif /* VXWORKS */
#ifdef WINDOWS
#define IFXOS_WakeUpEvent(ev)    \
   SetEvent(ev)
#endif /* WINDOWS */
#ifdef NO_OS
#define IFXOS_WakeUpEvent(ev)    \
   No_OS_WakeupEvent(ev)
#endif /* NO_OS */
#ifdef XAPI
#define IFXOS_WakeUpEvent(ev)    \
   xsm_v(ev)
#endif /* XAPI */

/** Reset event to initial state
\param ev - event of type IFXOS_event_t
 */
#ifdef VXWORKS
#define IFXOS_ClearEvent(ev)     \
   semFlush (ev); \
   semTake ((ev), NO_WAIT)
#endif /* VXWORKS */
#ifdef LINUX
#define IFXOS_ClearEvent(ev)     \
   init_waitqueue_head (&(ev))
#endif /* LINUX */
#ifdef WINDOWS
#define IFXOS_ClearEvent(ev)     \
   ResetEvent(ev)
#endif /* WINDOWS */
#ifdef NO_OS
   /*not supported*/
#define IFXOS_ClearEvent(ev)
#endif /* NO_OS */
#ifdef XAPI
#define IFXOS_ClearEvent(ev)     \
   xsm_v(ev); /* release, if not already done*/ \
   xsm_p(ev, SM_NOWAIT, 0L) /* and take it */
#endif /* XAPI */

/** Waits for a specified event with a specified timeout
    to occur or timeout.
\param ev - event of type IFXOS_event_t
\param timeout    - time out in ms
\return  IFX_SUCCESS if event occured, otherwise IFX_ERROR
*/
#ifdef VXWORKS
#define  IFXOS_WaitEvent_timeout(ev, timeout)          \
   (semTake (ev, (((timeout * sysClkRateGet()) / 1000)+1)) == ERROR ? IFX_ERROR : IFX_SUCCESS)
#endif /* VXWORKS */
#ifdef WINDOWS
#define  IFXOS_WaitEvent_timeout(ev, timeout)          \
   (WaitForSingleObject(ev, timeout) == WAIT_OBJECT_0 ? IFX_SUCCESS : IFX_ERROR)
#endif /* WINDOWS */
#ifdef LINUX
#define  IFXOS_WaitEvent_timeout(ev, timeout)          \
   (interruptible_sleep_on_timeout(&ev, HZ * (timeout) / 1000) == IFX_SUCCESS ? IFX_ERROR : IFX_SUCCESS)
#endif /* LINUX */
#ifdef NO_OS
#define  IFXOS_WaitEvent_timeout(ev, timeout)          \
   No_OS_SleepOnEvent(ev, timeout)
#endif /* NO_OS */
#ifdef XAPI
#define  IFXOS_WaitEvent_timeout(ev, timeout)          \
   (xsm_p (ev, (timeout == IFXOS_NOWAIT) ? SM_NOWAIT: SM_WAIT, timeout) != ERR_NO_ERROR ? IFX_ERROR : IFX_SUCCESS)
#endif /* XAPI */


/** Waits for a specified event with a specified condition
    to occur or timeout.
   \param event        - wait event
   \return  none
*/
#ifdef VXWORKS
#define  IFXOS_WaitEvent(event)          \
   semTake (event, WAIT_FOREVER)
#endif /* VXWORKS */
#ifdef LINUX
#define  IFXOS_WaitEvent(event)          \
   interruptible_sleep_on(&event)
#endif /* LINUX */
#ifdef WINDOWS
#define  IFXOS_WaitEvent(event)          \
   WaitForSingleObject(event,INFINITE)
#endif /* WINDOWS */
#ifdef NO_OS
#define  IFXOS_WaitEvent(event)          \
   No_OS_SleepOnEvent(ev, WAIT_FOREVER)
#endif /* NO_OS */
#ifdef XAPI
#define  IFXOS_WaitEvent(event)          \
   xsm_p (event, SM_WAIT, 0L)
#endif /* XAPI */
/*@}*/

/******************************************************************************
               Mutural exclusion for internal driver communication
******************************************************************************/
/** \defgroup SYS_DRV_IFXOS_MUTEX Mutual Exclusion
Mutexes are used to protect critical sections against race conditions.
They have several names across operating systems, but are all considered as
mutexes by the IFXOS abstraction.

\code
   typedef struct {
      ...
      // protect mailbox access
      IFXOS_mutex_t    mbxAcc;
   }XXX_DEV;

   void main()
   {
      ...
      // initialize mailbox protection semaphore
      IFXOS_MutexInit (pDev->mbxAcc);
      ...
   }

   // function accessing the mailbox
   writeData(XXX_DEV* pDev)
   {
      ...
      // protect access against concurrent tasks
      IFXOS_MutexLock (pDev->mbxAcc);
      // protect vinetic access against interrupts
      Vinetic_IrqLock(pDev);
      err = pDev->write(pDev, &cmd, 1);
      // release interrupt lock
      Vinetic_IrqUnlockDevice(pDev);
      // release task lock
      IFXOS_MutexUnlock (pDev->mbxAcc);
      ...
   }
\endcode
*/

/**@{*/

#ifdef VXWORKS
/** Mutex type.
 */
typedef SEM_ID             IFXOS_mutex_t;
#endif /* VXWORKS */
#ifdef LINUX
typedef struct semaphore*  IFXOS_mutex_t;
#endif /* LINUX */
#ifdef WINDOWS
typedef HANDLE             IFXOS_mutex_t;
#endif /* WINDOWS */
#ifdef NO_OS
typedef NO_OS_EVENT        IFXOS_mutex_t;
#endif /* NO_OS */
#ifdef XAPI
/** Mutex type.
 */
typedef unsigned long      IFXOS_mutex_t;
#endif /* XAPI */

/** initialze mutex
\param mutex - mutex handle
*/
#ifdef VXWORKS
#define IFXOS_MutexInit(mutex)  \
  (mutex) = semMCreate(SEM_Q_PRIORITY | SEM_INVERSION_SAFE | SEM_DELETE_SAFE)
#endif /* VXWORKS */
#ifdef WINDOWS
#define IFXOS_MutexInit(mutex) \
   (mutex) = CreateMutex(NULL, FALSE, NULL)
#endif /* WINDOWS */
#ifdef LINUX
#define IFXOS_MutexInit(mutex) \
   if ( ((mutex)=kmalloc(sizeof(struct semaphore), GFP_KERNEL)) != NULL )\
      {sema_init((mutex), 1);}
#endif /* LINUX */
#ifdef NO_OS
#define IFXOS_MutexInit(mutex)  \
   No_OS_InitEvent     (&mutex, TRUE)
#endif /* NO_OS */
#ifdef XAPI
#define IFXOS_MutexInit(mutex)  \
   xsm_create( "IFXS", 1L, SM_FIFO, &mutex)
#endif /* XAPI */

/** lock/take the mutex
\param mutex - mutex handle
*/
#ifdef VXWORKS
#define IFXOS_MutexLock(mutex)     \
   semTake(mutex, WAIT_FOREVER)
#endif /* VXWORKS */
#ifdef LINUX
#define IFXOS_MutexLock(mutex)     \
   down(mutex)
#endif /* LINUX */
#ifdef WINDOWS
#define IFXOS_MutexLock(mutex)     \
   WaitForSingleObject(mutex, INFINITE)
#endif /* WINDOWS */
#ifdef NO_OS
#define IFXOS_MutexLock(mutex)     \
   No_OS_SleepOnEvent  (mutex, WAIT_FOREVER)
#endif /* NO_OS */
#ifdef XAPI
#define IFXOS_MutexLock(mutex)     \
   xsm_p (mutex, SM_WAIT, 0L)
#endif /* XAPI */

/** lock/take the mutex but be interruptable by signals.
    Different than IFXOS_MutexLock is that the sleep is also interrupted when 
    a signal is received by the sleeping thread. This may not be needed or
    implemented for all operating systems.
\param mutex - mutex handle
*/
#ifdef VXWORKS
#define IFXOS_MutexLockInterruptible(mutex)     \
   (semTake(mutex, WAIT_FOREVER), 0)
#endif /* VXWORKS */
#ifdef LINUX
#define IFXOS_MutexLockInterruptible(mutex)     \
   down_interruptible(mutex)
#endif /* LINUX */
#ifdef WINDOWS
#define IFXOS_MutexLockInterruptible(mutex)     \
   WaitForSingleObject(mutex, INFINITE)
#endif /* WINDOWS */
#ifdef NO_OS
#define IFXOS_MutexLockInterruptible(mutex)     \
   No_OS_SleepOnEvent  (mutex, WAIT_FOREVER)
#endif /* NO_OS */
#ifdef XAPI
#define IFXOS_MutexLockInterruptible(mutex)     \
   xsm_p (mutex, SM_WAIT, 0L)
#endif /* XAPI */


/** unlock/give the mutex
\param mutex - mutex handle
\return IFX_SUCCESS for success, otherwise IFX_ERROR
*/
#ifdef IFXOS_NOTDEFINED
#define IFXOS_MutexUnlock(mutex)
#endif /* IFXOS_NOTDEFINED */
#ifdef VXWORKS
#define IFXOS_MutexUnlock(mutex)   \
   semGive(mutex)
#endif /* VXWORKS */
#ifdef LINUX
#define IFXOS_MutexUnlock(mutex)   \
   up(mutex)
#endif /* LINUX */
#ifdef WINDOWS
#define IFXOS_MutexUnlock(mutex)   \
   ReleaseMutex(mutex)
#endif /* WINDOWS */
#ifdef NO_OS
#define IFXOS_MutexUnlock(mutex)   \
   No_OS_WakeupEvent(mutex)
#endif /* NO_OS */
#ifdef XAPI
#define IFXOS_MutexUnlock(mutex)   \
   xsm_v(mutex)
#endif /* XAPI */


/** Delete a mutex element
\param mutex - mutex handle
 */
#ifdef VXWORKS
#define IFXOS_MutexDelete(mutex)      \
   semDelete(mutex); mutex = 0;
#endif /* VXWORKS */
#ifdef LINUX
#define IFXOS_MutexDelete(mutex)      \
   kfree(mutex); mutex = 0;
#endif /* LINUX */
#ifdef WINDOWS
#define IFXOS_MutexDelete(mutex)      \
   CloseHandle(mutex); mutex = 0;
#endif /* WINDOWS */
#ifdef NO_OS
#define IFXOS_MutexDelete(mutex)      \
   No_OS_DeleteEvent(&mutex)
#endif /* NO_OS */
#ifdef XAPI
#define IFXOS_MutexDelete(mutex)      \

#endif /* VXWORKS */
/*@}*/


/******************************************************************************
              driver external communication for select/polling
******************************************************************************/

/** \defgroup SYS_DRV_IFXOS_SELECT  Select and Polling.
The poll/select mechanism is used for user application synchronization after
occurrence of particular events (e.g: signalization to application,
that data is ready for reading)
VxWorks distinguishes between semaphores, events, mutex and
also select wait queues. Linux does not, but anyway we need macros
to support VxWorks and other OS. */

/**@{*/

/** wakeup type for select wait queues */
#ifdef VXWORKS
typedef SEL_WAKEUP_LIST             	IFXOS_wakelist_t;
#endif /* VXWORKS */
#ifdef LINUX
typedef wait_queue_head_t              IFXOS_wakelist_t;
#endif /* LINUX */
#ifdef WINDOWS
typedef HANDLE                       	IFXOS_wakelist_t;
#endif /* WINDOWS */
#ifdef NO_OS
typedef NO_OS_EVENT                   	IFXOS_wakelist_t;
#endif /* NO_OS */
#ifdef XAPI
typedef unsigned long                  IFXOS_wakelist_t;
#endif /* XAPI */

/** Initialize a queue. */
#ifdef VXWORKS
#define IFXOS_Init_WakeList(queue)           \
   selWakeupListInit(&queue)
#endif /* VXWORKS */
#ifdef LINUX
#define IFXOS_Init_WakeList(queue)           \
   init_waitqueue_head (&(queue))
#endif /* LINUX */
#ifdef WINDOWS
#define IFXOS_Init_WakeList(queue) \
   queue = CreateEvent(NULL, TRUE, TRUE, NULL)
#endif /* WINDOWS */
#ifdef NO_OS
   /* wakeup / select() not supported */
#define IFXOS_Init_WakeList(queue)
#endif /* NO_OS */
#ifdef XAPI
   /* wakeup / select() not supported */
#define IFXOS_Init_WakeList(queue)
#endif /* XAPI */

/** if called initializes the sleep on the given queue.
    This macros returns immediately
\param queue - wait queue handle
\param opt - optional parameter for additional information.
             For VxWorks node parameter, for linux poll table
\param fp - optional file pointer (needed for linux)
     */
#ifdef VXWORKS
#define IFXOS_SleepQueue(queue, opt, fp)    \
   selNodeAdd(&queue, (SEL_WAKEUP_NODE*)opt)
#endif /* VXWORKS */
#ifdef LINUX
#define IFXOS_SleepQueue(queue, opt, fp)    \
   poll_wait((struct file *)fp, &queue, (poll_table *)opt)
#endif /* LINUX */
#ifdef WINDOWS
#define IFXOS_SleepQueue(queue, opt, fp)
#endif /* WINDOWS */
#ifdef NO_OS
   /* wakeup / select() not supported */
#define IFXOS_SleepQueue(queue, opt, fp)
#endif /* NO_OS */
#ifdef XAPI
   /* wakeup / select() not supported */
#define IFXOS_SleepQueue(queue, opt, fp)
#endif /* XAPI */

/** Wakes up a waiting queue in select
\param queue - wait queue handle
\param type - type of the queue: IFXOS_READQ or IFXOS_WRITEQ.
              Not needed for linux
*/
#ifdef VXWORKS
#define IFXOS_WakeUp(queue, type)   \
   selWakeupAll(&queue, type)
#endif /* VXWORKS */
#ifdef LINUX
#define IFXOS_WakeUp(queue, type)   \
   wake_up_interruptible(&queue)
#endif /* LINUX */
#ifdef WINDOWS
#define IFXOS_WakeUp(queue, type)   \
   SetEvent(queue)
#endif /* WINDOWS */
#ifdef NO_OS
   /* wakeup / select() not supported */
#define IFXOS_WakeUp(queue, type)
#endif /* NO_OS */
#ifdef XAPI
#define IFXOS_WakeUp(queue, type)
   /* wakeup / select() not supported */
#endif /* XAPI */

/** definitions for select queues */
#ifdef VXWORKS
/** defines the write queue */
#define IFXOS_WRITEQ           SELWRITE
/** defines the read queue */
#define IFXOS_READQ            SELREAD
/** system event for write ready. Is returned on select to determine
    the kind of event  */
#define  IFXOS_SYSWRITE  0x00000002
/** system event for read ready. Is returned on select to determine
    the kind of event */
#define  IFXOS_SYSREAD   0x00000001
/** system exception, avoid usage, since it is not supported in all operating
    systems */
#define  IFXOS_SYSEXCEPT IFXOS_SYSREAD
#endif /* VXWORKS */
#ifdef LINUX
/** defines the write queue */
#define IFXOS_WRITEQ           1
/** defines the read queue */
#define IFXOS_READQ            2
/** system event for write ready. Is returned on select to determine
    the kind of event  */
#define  IFXOS_SYSWRITE  POLLOUT
/** system event for read ready. Is returned on select to determine
    the kind of event */
#define  IFXOS_SYSREAD   POLLIN
/** system exception, avoid usage, since it is not supported in all operating
    systems */
#define  IFXOS_SYSEXCEPT POLLPRI
#endif /* LINUX */
#ifdef WINDOWS
/** defines the write queue */
#define IFXOS_WRITEQ           1
/** defines the read queue */
#define IFXOS_READQ            2
/** system event for write ready. Is returned on select to determine
    the kind of event  */
#define  IFXOS_SYSWRITE  0x00000002
/** system event for read ready. Is returned on select to determine
    the kind of event */
#define  IFXOS_SYSREAD   0x00000001
/** system exception, avoid usage, since it is not supported in all operating
    systems */
#define  IFXOS_SYSEXCEPT IFXOS_SYSREAD
#endif /* WINDOWS */
#ifdef NO_OS
/** defines the write queue */
#define IFXOS_WRITEQ           1
/** defines the read queue */
#define IFXOS_READQ            2
/** system event for write ready. Is returned on select to determine
    the kind of event  */
#define  IFXOS_SYSWRITE  0x00000002
/** system event for read ready. Is returned on select to determine
    the kind of event */
#define  IFXOS_SYSREAD   0x00000001
/** system exception, avoid usage, since it is not supported in all operating
    systems */
#define  IFXOS_SYSEXCEPT IFXOS_SYSREAD
#endif /* NO_OS */
#ifdef XAPI
/** defines the write queue */
#define IFXOS_WRITEQ           xapi_SELWRITE
/** defines the read queue */
#define IFXOS_READQ            xapi_SELREAD
/** system event for write ready. Is returned on select to determine
    the kind of event  */
#define  IFXOS_SYSWRITE  0x00000002
/** system event for read ready. Is returned on select to determine
    the kind of event */
#define  IFXOS_SYSREAD   0x00000001
/** system exception, avoid usage, since it is not supported in all operating
    systems */
#define  IFXOS_SYSEXCEPT IFXOS_SYSREAD
#endif /* XAPI */
/*@}*/


/** \defgroup SYS_DRV_IFXOS_MISC Misc Macros and Function Declarations
 */
/**@{*/

/** copy from user to kernel space */
#ifdef VXWORKS
#define IFXOS_CPY_USR2KERN(to, from, n)       \
   memcpy((void *)to, (const void *)from, n)
#endif /* VXWORKS */
#ifdef LINUX
#define IFXOS_CPY_USR2KERN(to, from, n)       \
   (copy_from_user((void *)to, (const void *)from, n)?NULL:(void*)to)
#endif /* LINUX */
#ifdef WINDOWS
#define IFXOS_CPY_USR2KERN(to, from, n)       \
   memcpy(to, from, n)
#endif /* WINDOWS */
#ifdef NO_OS
#define IFXOS_CPY_USR2KERN(to, from, n)       \
   memcpy((to), (from), (n))
#endif /* NO_OS */
#ifdef XAPI
#define IFXOS_CPY_USR2KERN(to, from, n)       \
   memcpy((void *)to, (const void *)from, n)
#endif /* XAPI */

/** copy from kernel to user space */
#ifdef VXWORKS
#define IFXOS_CPY_KERN2USR(to, from, n)       \
   memcpy((void *)to, (const void *)from, n)
#endif /* VXWORKS */
#ifdef LINUX
#define IFXOS_CPY_KERN2USR(to, from, n)       \
   (copy_to_user((void *)to, (const void *)from, n)?NULL:(void*)to)
#endif /* LINUX */
#ifdef WINDOWS
#define IFXOS_CPY_KERN2USR(to, from, n)       \
   memcpy((void *)to, (const void *)from, n)
#endif /* WINDOWS */
#ifdef NO_OS
#define IFXOS_CPY_KERN2USR(to, from, n)       \
   memcpy((to), (from), (n))
#endif /* NO_OS */
#ifdef XAPI
#define IFXOS_CPY_KERN2USR(to, from, n)       \
   memcpy((void *)to, (const void *)from, n)
#endif /* XAPI */


#ifdef LINUX_2_6
#define MOD_INC_USE_COUNT
#define MOD_DEC_USE_COUNT

#else /* not LINUX_2_6, i.e LINUX_2_4 */

#ifndef wait_event_interruptible_timeout
#define __wait_event_interruptible_timeout(wq, condition, ret)                 \
do                                                                             \
{                                                                              \
   wait_queue_t __we;                                                          \
   init_waitqueue_entry(&__we, current);                                       \
                                                                               \
   add_wait_queue(&wq, &__we);                                                 \
   for (;;)                                                                    \
   {                                                                           \
      /* go halfway to sleep */                                                \
      set_current_state(TASK_INTERRUPTIBLE);                                   \
      /* check condition again */                                              \
      if (condition)                                                           \
         break;                                                                \
      ret = schedule_timeout(ret);                                             \
      /* timeout? then return */                                               \
      if (!ret)                                                                \
         break;                                                                \
   }                                                                           \
   /* restore the state in case we never fall asleep */                        \
   set_current_state(TASK_RUNNING);                                            \
   remove_wait_queue(&wq, &__we);                                              \
} while (0)

/**
   Linux 2.6 equivalent race condition free implmentation for
   IFXOS_WaitEvent_timeout.
*/
#define wait_event_interruptible_timeout(wq, condition, timeout)               \
({                                                                             \
   IFX_uint32_t __to = timeout;                                                \
   if (!(condition))                                                           \
      __wait_event_interruptible_timeout(wq, condition, __to);                 \
   __to;                                                                       \
})
#endif /* wait_event_interruptible_timeout */

#endif /* LINUX_2_6 */

/** return value of the interrupt service routine telling that ... */
typedef enum {
   /** no interrupt has been handled for this device */
   IFX_IRQ_NONE    = 0,
   /** an interrupt has been handled for this device */
   IFX_IRQ_HANDLED = 1
} IFX_irqreturn_t;

/**@}*/

#endif /* _SYS_DRV_IFXOS_H */

