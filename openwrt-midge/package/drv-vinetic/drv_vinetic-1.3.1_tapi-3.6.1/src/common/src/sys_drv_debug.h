#ifndef _sys_drv_debug_h
#define _sys_drv_debug_h
/******************************************************************************

                               Copyright (c) 2006
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

******************************************************************************/

/**   
   \file 
   System debug interface
*/

/* ============================= */
/* Global Macro Definitions      */
/* ============================= */

/* ============================= */
/* Group=VOS - debug interface   */
/* ============================= */

#define DBG_LEVEL_OFF 		4
#define DBG_LEVEL_HIGH		3
#define DBG_LEVEL_NORMAL	2
#define DBG_LEVEL_LOW 		1

/* FIXME: test for GNU compiler */
/*lint -emacro(2,__TRACE) -emacro(10,__TRACE)*/
/* This inhibits PC-Lint from reporting errors 2 and 10 for the __TRACE macro */
/* since PC-Lints preprocessor is not able to deal with macros with variable  */
/* number of arguments */
#ifdef VXWORKS
/* enable logMsg with variable number of arguments */
   #ifndef __PROTOTYPE_5_0
   #define __PROTOTYPE_5_0
   #endif

   #include "vxWorks.h"
   #include "logLib.h"

   #ifdef __GNUC__
	   #define __TRACE(fmt, args...) logMsg(fmt, ##args)
   #endif

   #define PRINTF	printf
   #ifdef __GNUC__
      #define PRINTK __TRACE
   #else
      #define PRINTK logMsg
   #endif

   #ifdef __GNUC__
      #define DRV_LOG __TRACE
   #else
      #define DRV_LOG logMsg
   #endif
#endif /* VXWORKS */

#ifdef LINUX

/*
to see the debug output on console
use following line:
   echo 8 > /proc/sys/kernel/printk
*/
#ifdef __KERNEL__
#ifndef MY_TRACE
   #define __TRACE(fmt,args...) printk(KERN_DEBUG fmt "\r", ##args)
#else
   extern void __TRACE (char* sFormat, ...);
#endif
   #define PRINTF __TRACE
   #define DRV_LOG __TRACE
#else
   #define PRINTF printf
   #define DRV_LOG printf
#endif /* __KERNEL__ */
#endif /* LINUX */

#ifdef OSE
   #ifdef USE_DEBUG_PRINTF
   #include "dbgprintf.h"
   #define PRINTF dbgprintf
   #define DRV_LOG(x) dbgprintf( "%s", x)
   #else
   #define PRINTF printf
   #define DRV_LOG(x) printf( "%s", x)
   #endif/* USE_DEBUG_PRINTF*/
#endif /* OSE */

#ifdef PSOS
   #define PRINTF	printf
   #define DRV_LOG(x)
#endif /* PSOS */

#ifdef NO_OS
   #define PRINTF	printf
   #define PRINTK printf
   #define DRV_LOG printf
#endif /* NO_OS */

#if (defined WIN32 || defined WINDOWS)
   #ifdef TRACE
      #undef TRACE
   #endif /* TRACE */
   #define PRINTF print
   #define DRV_LOG print
   extern void (*print)(char * const pszFormat, ...);
#endif /* WIN32 */

#ifdef XAPI
   extern int trc_printf(char* format, ...);
   #define PRINTF	   trc_printf
   #define PRINTK    trc_printf
   #define DRV_LOG   trc_printf
#endif /* XAPI */

#ifdef ENABLE_TRACE
/**
   Prototype for a trace group.

   \param name Name of the trace group

   \code
   // declares a logical trace group named LLC
   DECLARE_TRACE_GROUP(LLC) 
   \endcode
*/
#define DECLARE_TRACE_GROUP(name) extern unsigned int G_nTraceGroup##name


/**
   Create a trace group.

   \param Name of the trace group

   \remark
   This has to be done once in the project. We do it in the file prj_debug.c.
   The default level of this trace group is DBG_LEVEL_HIGH.

   \code
   // creates a logical trace group named LLC
   CREATE_TRACE_GROUP(LLC)
   \endcode
*/
#define CREATE_TRACE_GROUP(name) unsigned int G_nTraceGroup##name = DBG_LEVEL_HIGH


/**
   Prints a trace message.

   \param name       Name of the trace group
   \param level      level of this message
   \param message    a printf compatible formated string + opt. arguments

   \remark
   The string will be redirected via printf if level is higher or equal to the
   actual level for this trace group ( please see SetTraceLevel ).

   \code
   TRACE(LLC,DBG_LEVEL_NORMAL,("LLC> State:%d\n", nState));
   \endcode
*/
#define TRACE(name,level,message) do {if(level >= G_nTraceGroup##name) \
      { PRINTF message ; } } while(0)


/**
   Set the actual level of a trace group.

   \param name       Name of the trace group
   \param new_level  new trace level
*/
#define SetTraceLevel(name, new_level) {G_nTraceGroup##name = new_level;}

/**
   Get the actual level of a trace group.

   \param name       Name of the trace group
   \param new_level  new trace level
*/
#define GetTraceLevel(name) G_nTraceGroup##name

#else /* ENABLE_TRACE */
   #define DECLARE_TRACE_GROUP(name)
   #define CREATE_TRACE_GROUP(name)
   #define TRACE(name,level,message) {}
   #define SetTraceLevel(name, new_level) {}
   #define GetTraceLevel(name) 0
#endif /* ENABLE_TRACE */


#ifdef ENABLE_LOG
/**
   Prototype for a log group.

   \param name       Name of the log group

   \code
   // declares a logical log group named LLC
   DECLARE_LOG_GROUP(LLC) 
   \endcode
*/
#define DECLARE_LOG_GROUP(diag_group) extern unsigned int G_nLogGroup##diag_group


/**
   Create a log group.

   \param name       Name of the log group

   \remark
   This has to be done once in the project. We do it in the file prj_debug.c.
   The default level of this log group is DBG_LEVEL_HIGH.

   \code
   // creates a logical log group named LLC
   CREATE_LOG_GROUP(LLC)
   \endcode
*/
#define CREATE_LOG_GROUP(diag_group) unsigned int G_nLogGroup##diag_group = DBG_LEVEL_HIGH


/**
   Prints a log message.

   \param name       Name of the log group
   \param level      level of this message
   \param message    a c-string

   \remark
   The string will be redirected via VOS_Log if level is higher or equal to the
   actual level for this log group ( please see SetLogLevel ).

   \code
   LOG(LLC,DBG_LEVEL_NORMAL,("LLC> State:%d\n", nState));
   \endcode
*/
/* lint -emacro(155,LOG)*/
/* lint -emacro(26,LOG)*/
/* PC Lint is not able to deal with structures like {commands}; */
#define LOG(diag_group,level,message) do {if(level >= G_nLogGroup##diag_group) \
   { DRV_LOG message; }else{}} while(0)

/**
   Set the actual level of a log group.

   \param name       Name of the log group
   \param new_level  new log level
*/
#define SetLogLevel(diag_group, new_level) {G_nLogGroup##diag_group = new_level;}

/**
   Get the actual level of a log group.

   \param name       Name of the log group
   \param new_level  new log level
*/
#define GetLogLevel(diag_group) G_nLogGroup##diag_group

#else /* ENABLE_LOG */
   #define DECLARE_LOG_GROUP(diag_group)
   #define CREATE_LOG_GROUP(diag_group)
   #define LOG(diag_group,level,message) {}
   #define SetLogLevel(diag_group, new_level) {}
   #define GetLogLevel(diag_group) 0
#endif /* ENABLE_LOG */

#ifdef DEBUG
extern void * sys_dbgMalloc(int size, int line, const char* sFile);
extern void sys_dbgFree (void *pBuf, int line, const char* sFile);
extern void dbgResult(void);
#endif /* DEBUG */

#endif /* _sys_drv_debug_h */

