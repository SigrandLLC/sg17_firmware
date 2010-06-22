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

*******************************************************************************/
#ifdef DRV_DAA_BOARD_EASY50712

#include "drv_daa.h"
#include "drv_daa_common.h"

/* board specific includes */
#include "drv_duslic_io.h"
#include "drv_duslic_gpio.h"


extern DAA_FUNCTION_MAP_t        gDaaFctMap[];

int dusDevHandle = 0, dusDevIoHandle = 0;

/**
   gpio callback function for the ringQ pin,
   this function decodes the status and triggers the generic state machine
   \param nDev    - device number raising the interrupt
   \param nCh     - channel number raising the interrupt
   \param nStatus - the current status of the signal
   \return void
*/
static void daa_0_ring_cb (int nDev, int nCh, unsigned short nStatus)
{
   irq_DAA_COM_ring_cb ( 0 );
}

/**
   gpio callback function for the batQ pin,
   this function decodes the status and triggers the generic state machine
   \param nDev    - device number raising the interrupt
   \param nCh     - channel number raising the interrupt
   \param nStatus - the current status of the signal
   \return void
*/
static void daa_0_bat_cb (int nDev, int nCh, unsigned short nStatus)
{
   irq_DAA_COM_bat_cb ( 0, nStatus ? DAA_BAT_EVT_BATTERY_DROP_IRQ :
                                     DAA_BAT_EVT_BATTERY_IRQ );
}

/**
   gpio callback function for the pol pin,
   this function decodes the status and triggers the generic state machine
   \param nDev    - device number raising the interrupt
   \param nCh     - channel number raising the interrupt
   \param nStatus - the current status of the signal
   \return void
*/
static void daa_0_pol_cb (int nDev, int nCh, unsigned short nStatus)
{
   irq_DAA_COM_pol_cb (0, nStatus ? DAA_POL_EVT_NORMAL_IRQ :
                                    DAA_POL_EVT_REVERSE_IRQ);
}

#ifdef DAA_APOH
/**
   gpio callback function for the apoh pin,
   this function decodes the status and triggers the generic state machine
   \param nDev    - device number raising the interrupt
   \param nCh     - channel number raising the interrupt
   \param nStatus - the current status of the signal
   \return void
*/
static void daa_0_apoh_cb (int nDev, int nCh, unsigned short nStatus)
{
   irq_DAA_COM_apoh_cb ( 0, nStatus ? DAA_APOH_EVT_APOH_IRQ :
                                      DAA_APOH_EVT_NOPOH_IRQ);
}
#endif /* DAA_APOH */

/**
   initialisation routine of daa 0, implementation is system dependant.
   This function is getting called on IFX_TAPI_LINE_TYPE_SET ioctl, if
   the channel is configured as FXO. This is the place to reserve and
   configure GPIOs for later use.
   \param none
   \return IFX_SUCCESS or IFX_ERROR in case of failure
*/
static IFX_return_t daa_0_init (void)
{
   DUS_IO_Config_t ioCfg;
   /*VINETIC_GPIO_CONFIG ioCfg;*/
   int ret;

   /* open first device (0), first channel (counting starts at 1) */
   dusDevHandle = DUS_OpenKernel(0, 1);
   if (dusDevHandle == IFX_ERROR)
   {
      printk("drv_daa, failed to open drv_duslic\n\r");
      return IFX_ERROR;
   }

   dusDevIoHandle = DUS_IoReserve  (dusDevHandle, DUS_IO_CH_IO_1 |
                                                  DUS_IO_CH_IO_2 |
                                                  DUS_IO_CH_IO_3 |
                                                  DUS_IO_CH_IO_4 );

   if (dusDevIoHandle == 0)
   {
      printk("reservation failed\n\r");
      return IFX_ERROR;
   }

   /* HOOKQ */
   ioCfg.nMode = DUS_IO_MODE_OUTPUT;
   ioCfg.nGpio = DUS_IO_CH_IO_1;
   ret = DUS_IoConfig(dusDevIoHandle, &ioCfg);
   if (ret != IFX_SUCCESS)
   {
      printk("cfg output failed\n\r");
      return IFX_ERROR;
   }

   /* RINGQ */
   ioCfg.nMode = DUS_IO_MODE_INPUT | DUS_IO_MODE_INT /*| DUS_IO_MODE_INT_FALLING*/;
   ioCfg.nGpio = DUS_IO_CH_IO_2;
   ioCfg.callback = daa_0_ring_cb;
   ret = DUS_IoConfig(dusDevIoHandle, &ioCfg);
   if (ret != IFX_SUCCESS)
   {
      printk("cfg input 3 failed\n\r");
      return IFX_ERROR;
   }

   /* BATQ */
   ioCfg.nMode = DUS_IO_MODE_INPUT | DUS_IO_MODE_INT /*| GPIO_INT_RISING | GPIO_INT_FALLING*/;
   ioCfg.nGpio = DUS_IO_CH_IO_3;
   ioCfg.callback = daa_0_bat_cb;
   ret = DUS_IoConfig(dusDevIoHandle, &ioCfg);
   if (ret != IFX_SUCCESS)
   {
      printk("cfg input 4 failed\n\r");
      return IFX_ERROR;
   }

   /* POLQ */
   ioCfg.nMode = DUS_IO_MODE_INPUT | DUS_IO_MODE_INT /*| GPIO_INT_RISING | GPIO_INT_FALLING*/;
   ioCfg.nGpio = DUS_IO_CH_IO_4;
   ioCfg.callback = daa_0_pol_cb;
   ret = DUS_IoConfig(dusDevIoHandle, &ioCfg);
   if (ret != IFX_SUCCESS)
   {
      printk("cfg input 5 failed\n\r");
      return IFX_ERROR;
   }

#ifdef DAA_APOH
   /* APOH */
   /* connected to some gpio */
#endif /* DAA_APOH */

   return ret;
}

/** **************************************************************************
   daa 0 hook set implementation (board specific)
   \param offhook - desired hook state to be set
   \return IFX_SUCCESS or IFX_ERROR in case of failure
*/
static IFX_return_t daa_0_hookSet (IFX_TAPI_FXO_HOOK_t offhook)
{
   int ret = IFX_SUCCESS;

   /* HOOKQ control GPIO1
      offhook => pin locic low
      onhook  => pin logic high */
   if (dusDevIoHandle != 0)
   {
      ret = DUS_IoSet(dusDevIoHandle,
                      (offhook == IFX_TAPI_FXO_HOOK_OFFHOOK) ?
                          0x00 : DUS_IO_CH_IO_1,
                      DUS_IO_CH_IO_1);
   }
   else
   {
      ret = IFX_ERROR;
   }

   return ret;
}

/** **************************************************************************
   daa 0 hook get implementation (board specific)
   \param offhook - current hook state (read back)
   \return IFX_SUCCESS or IFX_ERROR in case of failure
*/
static IFX_return_t daa_0_hookGet (IFX_TAPI_FXO_HOOK_t *offhook)
{
   unsigned char get;
   int ret = IFX_SUCCESS;

   /* HOOKQ control GPIO1
      offhook => pin locic low
      onhook  => pin logic high */
   if (dusDevIoHandle != 0)
   {
      ret = DUS_IoGet(dusDevIoHandle, &get, DUS_IO_CH_IO_1);
      *offhook = (! get) ? IFX_TAPI_FXO_HOOK_OFFHOOK : IFX_TAPI_FXO_HOOK_ONHOOK;
   }
   else
   {
      ret = IFX_ERROR;
   }
   return ret;
}

/** **************************************************************************
   daa 0 battery get implementation (board specific)
   \param bat - current battery (read back)
   \return IFX_SUCCESS or IFX_ERROR in case of failure
*/
static IFX_return_t daa_0_batGet (IFX_enDis_t *bat)
{
   unsigned char get;
   int ret = IFX_SUCCESS;

   /* BATQ control GPIO3
      offhook => pin locic low
      onhook  => pin logic high */
   if (dusDevIoHandle != 0)
   {
      ret = DUS_IoGet(dusDevIoHandle, &get, DUS_IO_CH_IO_3);
      *bat = (! get) ? IFX_ENABLE : IFX_DISABLE;
   }
   else
   {
      ret = IFX_ERROR;
   }
   return ret;
}

/** **************************************************************************
   daa 0 polarity get implementation (board specific)
   \param pol - current polarity (read back)
   \return IFX_SUCCESS or IFX_ERROR in case of failure
   \note direction cannot be clearly defined for all boards / FXO ports
   Clare Litelink reports low, if the ring line is negative to tip which is
   normally considered as normal polarity.
*/
static IFX_return_t daa_0_polGet (IFX_enDis_t *pol)
{
   unsigned char get;
   int ret = IFX_SUCCESS;

   /* POL control GPIO4
      normal  => pin locic low
      reverse => pin logic high */
   if (dusDevIoHandle != 0)
   {
      ret = DUS_IoGet(dusDevIoHandle, &get, DUS_IO_CH_IO_4);
      *pol = (!get) ? IFX_ENABLE : IFX_DISABLE;
   }
   else
   {
      ret = IFX_ERROR;
   }
   return ret;
}

#ifdef DAA_APOH
/** **************************************************************************
   daa 0 apoh get implementation (board specific)
   \param apoh - current apoh state (read back)
   \return IFX_SUCCESS or IFX_ERROR in case of failure
*/
static IFX_return_t daa_0_apohGet (IFX_enDis_t *apoh)
{
   unsigned char get;
   int ret = IFX_SUCCESS;

   /* APOH (active high) control Danube IO xxx
      nopoh => pin locic low
      apoh  => pin logic high */

   /* tbd */

   return ret;
}
#endif /* DAA_APOH */

/**
   registration of system specific functions.
   This function is getting called on insmod/loading of drv_daa.
   Note that at this time you cannot access other device's GPIOs as
   they aren't yet initialized.
   \param void
   \return IFX_SUCCESS or IFX_ERROR in case of failure
*/
IFX_return_t daa_board_OnInsmod (void)
{
   /* add functions for DAA channel 0 here */
   gDaaFctMap[0].init      = daa_0_init;
   gDaaFctMap[0].hookSet   = daa_0_hookSet;
   gDaaFctMap[0].hookGet   = daa_0_hookGet;
   gDaaFctMap[0].batGet    = daa_0_batGet;
   gDaaFctMap[0].polGet    = daa_0_polGet;
#ifdef DAA_APOH
   gDaaFctMap[0].apohGet   = daa_0_apohGet;
#endif /* DAA_APOH */

   /* add further DAA channels here, remember to configure
      DRV_DAA_MAX_DAA_CHANNELS in drv_daa_board_<xx>.h as well */

   return IFX_SUCCESS;
}

/**
   deregistration of system specific functions
   This function is getting called on rmmod/unload of drv_daa and must undo the
   board specific settings done in daa_N_xxx to leave the systems in stable
   state.
   \param void
   \return IFX_SUCCESS or IFX_ERROR in case of failure
*/
IFX_return_t daa_board_OnRmmod (void)
{
   int err1 = IFX_SUCCESS, err2 = IFX_SUCCESS;
   if (dusDevIoHandle != 0)
   {
      err1 = DUS_IoRelease (dusDevIoHandle);
      if (err1 != IFX_SUCCESS)
         printk("drv_daa, failed to release GPIO reservations\n\r");
   }
   if (dusDevHandle != 0)
   {
      /* close DuSLIC device */
      err2 = DUS_ReleaseKernel(dusDevHandle);
      if (err2 != IFX_SUCCESS)
         printk("drv_daa, failed to release dev handle to drv_duslic\n\r");
   }
   return (err1|err2);
}

#endif /* DRV_DAA_BOARD_EASY50712 */

