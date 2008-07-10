#ifndef __SVD_LOG_H__
#define __SVD_LOG_H__

/* TODO:
 * (-) DFS / DFE - move from configure.in to svd_err.h
 */
/*
 * The logging levels and macros to use are defined as follows:
 *  - SU_DEBUG_0()  fatal errors, panic 
 *  - SU_DEBUG_1()  critical errors, minimal progress at subsystem level
 *  - SU_DEBUG_2()  non-critical errors
 *  - SU_DEBUG_3()  warnings, progress messages
 *  - SU_DEBUG_5()  signaling protocol actions (incoming packets, etc.)
 *  - SU_DEBUG_7()  media protocol actions (incoming packets, etc.)
 *  - SU_DEBUG_9()  entering/exiting functions, very verbatim progress
*/

#define LOG_FNC_A(str)	"%s(): %s\n",__func__,str
#define LOG_NOMEM 	" not enough memory"
#define LOG_NOMEM_A(str) " not enough memory for \"" str "\""
#define LOG_NOFILE_A(str) " could not operate on file \"" str "\""

#define LOG_FILE_PATH_DF "/var/log/svd.log"

#endif /* __SVD_LOG_H__ */

