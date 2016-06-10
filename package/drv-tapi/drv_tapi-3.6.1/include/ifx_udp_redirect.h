/****************************************************************************
  Copyright (c) 2002-2004, Infineon Technologies.  All rights reserved.

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
Module      : udp_loopback.h
Date        : 2006-10-05
Description : This file contains definitions of udp loopback structures
              and function prototypes.
 *******************************************************************************/

#ifndef __UDP_LOOPBACK_H__
#define __UDP_LOOPBACK_H__

/**
 * \file udp_loopback.h
 *
 * Contains structures, function prototypes.
 */

#ifndef __KERNEL__

#endif /* __KERNEL__ */

#ifdef __KERNEL__

#define MAX_CHANNEL		8

/** Udp filter table status definition */
#define FILTER_NO_ERROR 	0x00
#define FILTER_NULL 	    	0x01
#define FILTER_NO_CHANNEL 	0x02
#define FILTER_NO_SRCPORT 	0x04
#define FILTER_NO_SRCIP 	0x08
#define FILTER_NO_DSTPORT 	0x10
#define FILTER_NO_DSTIP 	0x20
#define FILTER_NO_CALLBACK 0x40

/** Return code definition */
#define NO_ERROR		           0x0000
#define CHANNEL_NUM_ERR		     0x0001
#define REG_CALLBACK_ERR	     0x0002
#define NO_CALLBACK		        0x0003
#define CHANNEL_NO_ERR		     0x0004
#define WRONG_PKT		           0x0005
#define CALL_MK_SESSION_ERR     0x0006
#define CALL_DEL_SESSION_ERR    0x0007
#define CALLBACK_ERR            0x0008


/*typedef int (*v_callback)(const int chanDev,
                          const void* const data,
                          const size_t len);*/

typedef int (*v_callback_ingress)(int chanDev,
                                  void* data,
                                  size_t len);

typedef void* (*v_callback_bufferpool_get)(const void* const buff_pool);


/*extern int reg_callback(v_callback callback, int channel_num);*/

extern int reg_buffer_pool_get(v_callback_bufferpool_get callback,
                               const void* const pool_id);
extern int reg_ingress(v_callback_ingress callback,
                       int channel_num);

extern int mk_session(int channel, u16 sport, u32 saddr, u16 dport, u32 daddr);
extern int del_session(int channel, u16 sport);
extern int active_session(int channel, u16 sport);
extern int close_redirect(void);
extern int vtou_redirectrtp(int channel,
                            void* buff,
                            size_t len);

#endif /* __KERNEL__ */
#endif /* __UDP_LOOPBACK_H__ */

