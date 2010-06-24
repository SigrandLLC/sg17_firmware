#ifndef _DRV_TAPI_VXWORKS_H
#define _DRV_TAPI_VXWORKS_H
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
   Module      : $RCSfile: drv_tapi_vxworks.h,v $
   Date        : $Date: 2004/07/15 11:32:33 $
   Description : This file contains the includes and the defines
                 specific to the vxWorks OS
   Remarks     : Please use the compiler switches here if you have
                 more than one OS.
*******************************************************************************/

/* ============================= */
/* Global Includes               */
/* ============================= */

/* VxWorks Includes*/

#include <vxworks.h>

#include <drv/intrCtl/ppc860Intr.h>
/*#include <drv/multi/ppc860Siu.h>
#include <drv/multi/ppc860Cpm.h>
#include <drv/sio/ppc860Sio.h>*/
#include <drv/timer/timerDev.h>

#include <selectLib.h>
/*#include <arch/ppc/ivPpc.h>*/
#include <iosLib.h>
#include <stdio.h>
#include <time.h>
#include <taskLib.h>
#include <cacheLib.h>
#include <logLib.h>
#include <intLib.h>
#include <string.h>
#include <sys/ioctl.h>
/* include message queues support of vxworks. */
#include <msgQLib.h>

/* functions macros */

#ifndef GetImmr
   #define GetImmr()                         vxImmrGet()
#endif
#define IMAP_ADDR                            vxImmrGet()


/* ============================= */
/* Global Structures             */
/* ============================= */

/*
    Device header passed to the IOS at the device driver creation during
    the system startup.
*/
typedef struct
{
    DEV_HDR           DevHdr;   /* VxWorks specific: IOS header               */
    /** \todo Which one to use Major or Driver number? */
    IFX_int32_t       nMajorNum; /* Major number */
    IFX_int32_t       nDrvNum;    /* Device Driver Number                       */
    IFX_uint32_t      nDevNum;     /* Device number                              */
    IFX_uint32_t      nChNum;     /* Channel Number                             */
    IFX_int32_t       nInUse;    /* In Use counter                             */
    IFX_void_t*       pCtx;     /* context pointer: device or channel context */
} Y_S_dev_header;

/* ============================= */
/* Global function declaration   */
/* ============================= */


/*IMPORT IFX_uint32_t vxImmrGet(void);*/


#endif /* _DRV_TAPI_VXWORKS_H */

