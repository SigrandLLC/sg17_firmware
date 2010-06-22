#ifndef _DRV_TAPI_API_H
#define _DRV_TAPI_API_H
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
   \file drv_tapi_api.h
   Internal functional API of the driver.
*/

/* ============================= */
/* includes                      */
/* ============================= */
/*#include "sys_drv_defs.h"*/
#include "drv_tapi_version.h"

/* ============================= */
/* Global defs                   */
/* ============================= */
#ifndef DRV_TAPI_NAME
   #ifdef LINUX
      /** device name */
      #define DRV_TAPI_NAME          "tapi"
   #else
      /** device name */
      #define DRV_TAPI_NAME          "/dev/tapi"
   #endif
#else
   #error module name already specified
#endif

#ifndef _MKSTR_1
#define _MKSTR_1(x)    #x
#define _MKSTR(x)      _MKSTR_1(x)
#endif

/** driver version as string */
#define DRV_TAPI_VER_STR        _MKSTR(DRV_TAPI_VER_MAJOR)"."_MKSTR(DRV_TAPI_VER_MINOR)"."_MKSTR(DRV_TAPI_VER_STEP)"."_MKSTR(DRV_TAPI_VER_TYPE)

/** driver version as string */
#define DRV_TAPI_LL_IF_VER_STR        _MKSTR(DRV_TAPI_LL_IF_VER_MAJOR)"."_MKSTR(DRV_TAPI_LL_IF_VER_MINOR)"."_MKSTR(DRV_TAPI_LL_IF_VER_STEP)"."_MKSTR(DRV_TAPI_LL_IF_VER_TYPE)

/** driver version, what string */
#define DRV_TAPI_WHAT_STR "@(#)IFX TAPI, version " DRV_TAPI_VER_STR

/** maximum instances of this driver */
#define MAX_TAPI_INSTANCES      1

#define DEFAULT_EVTPT         0x62

/* what string */
extern const char TAPI_WHATVERSION[];

#endif

