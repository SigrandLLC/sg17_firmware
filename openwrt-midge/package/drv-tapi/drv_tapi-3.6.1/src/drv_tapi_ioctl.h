#ifndef DRV_TAPI_IOCTL_H
#define DRV_TAPI_IOCTL_H
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

 *****************************************************************************
   \file
   \remarks
 *******************************************************************/

/* ============================= */
/* Includes                      */
/* ============================= */

/* ============================= */
/* Global structure definition   */
/* ============================= */

/** Generic structure for ioctl addressing. */
typedef struct
{
   /** Device index */
   IFX_uint16_t dev;
   /** Channel "module" index */
   IFX_uint16_t ch;
   /** Any parameter used by ioctls */
   IFX_uint32_t param;
} IFX_TAPI_IOCTL_t;

typedef struct
{
   union
   {
      TAPI_CHANNEL* pTapiCh;
      TAPI_DEV*     pTapiDev;
   }p;
   /** Parameter size if available, otherwise 0. The parameter size is given by the
      ioctl definition */
   IFX_uint32_t nParamSize;
   /** Flags a driver global ioctl */
   IFX_uint32_t bDrv : 1;
   /** Flags a device global ioctl */
   IFX_uint32_t bDev : 1;
   /** Flags a channel ioctl */
   IFX_uint32_t bCh : 1;
   /** if IFX_TRUE it was copied from user space */
   IFX_uint32_t bUsrCpy : 1;
   /** the file descriptor is a single dev node without information
      about the channe and the device. The context is queried from the
      given ioctl structure */
   IFX_uint32_t bSingleFd : 1;
}IFX_TAPI_ioctlCtx_t;

/* ============================= */
/* Global function declaration   */
/* ============================= */

extern int TAPI_Dev_Spec_Ioctl (IFX_TAPI_DRV_CTX_t* pDrvCtx,
                                IFX_TAPI_ioctlCtx_t* pCtx,
                                IFX_uint32_t nCmd, IFX_uint32_t nArgument);
extern IFX_int32_t  TAPI_Spec_Ioctl (IFX_TAPI_DRV_CTX_t* pDrvCtx,
                                     IFX_TAPI_ioctlCtx_t* pCtx,
                                     IFX_uint32_t iocmd,
                                     IFX_uint32_t ioarg);

extern IFX_int32_t TAPI_Ioctl (IFX_TAPI_DRV_CTX_t* pDrvCtx,
                               TAPI_CHANNEL *pChannel,
                               IFX_uint32_t cmd, IFX_uint32_t arg);

extern void IFX_TAPI_ioctlContextGet (IFX_TAPI_DRV_CTX_t *pDrvCtx,
                               IFX_int32_t nMinor,
                               IFX_TAPI_ioctlCtx_t *pCtx);


extern IFX_int32_t  TAPI_OS_IoctlCh (IFX_TAPI_DRV_CTX_t* pDrvCtx,
                              IFX_TAPI_ioctlCtx_t* pCtx,
                              TAPI_CHANNEL* pChannel,
                              IFX_uint32_t iocmd, IFX_uint32_t ioarg);

#endif /* DRV_TAPI_IOCTL_H */

