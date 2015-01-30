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

******************************************************************************/

/*
   This file serves as a template to extent the support of the TAPI DAA plugin
   to support new systems which map the DAA functions to different IO pins.
   This TAPI DAA plugin is designed to be easily adapted to new systems and
   therefore layered in three layers API, COMMON and BOARD. Only the board
   specific part consisting of
   drv_daa_boards.h, drv_daa_board_template.h, drv_daa_board_template.c
   The following steps are required:

   1) Create a board specific copy of drv_daa_board_template.c|.h

   2) Add the include of drv_daa_board_phantastic.h to drv_daa_boards,
      add drv_daa_board_phantastic.c to Makefile.am

   3) Adapt DRV_DAA_MAX_DAA_CHANNELS in drv_daa_board_phantastic.h to reflect
      the number of DAA channels on your system.

   4) Adapt the implementation in drv_daa_board_phantastic.c to reflect your
      system specific DAA handling.
      The first hook point to be implemented is daa_board_OnInsmod(). This
      function is called by the TAPI once the DAA plugin is registered. At
      this point you might not be able to access the GPIOs already, but at
      least the board specific part must register it's functions to the two
      higher layer daa levels (API and COMMON).

      daa_<nDaaCh>_init() is getting called on IFX_TAPI_LINE_TYPE_SET ioctl
      and serves as a hook to initialize the GPIOs and register the callback
      functions (and further system specific configurations). This function
      expects that the system is initialized already such far, that the
      IOs used for the DAA functionality can be accessed already. In case
      of IFX TAPI low level device drivers, a IFX_TAPI_CH_INIT must be
      executed on the specific device before.

      The adaption to a new system further requires the implementation of
      certain callbackfunctions for DAA events Ringing, Battery, Polarity
      and APHO, a function to configure the hook state. In addition functions
      to read back the current state of all DAA functions are required.

      The three examples provided (drv_daa_board_easy3332, _easy50712 and
      _easy3201 serve as reference and show how the adaptaion could look like
      in case the DAA is connected to a VINETIC-CPE, DuSLIC or DuSLIC-xT.
*/

#ifdef DRV_DAA_BOARD_TEMPLATE

#include "drv_daa.h"
#include "drv_daa_common.h"

/* board specific includes */
#include "vinetic_io.h"
#include "drv_vinetic_gpio.h"

extern DAA_FUNCTION_MAP_t        gDaaFctMap[];

int vinDevHandle, vinDevIoHandle = 0;

/**
   gpio callback function for the ringQ pin,
   this function decodes the status and triggers the generic state machine
*/
static void daa_0_ring_cb (int nDev, int nCh, unsigned short nStatus)
{
   irq_DAA_COM_ring_cb ( 0 );
}

/**
   gpio callback function for the batQ pin,
   this function decodes the status and triggers the generic state machine
*/
static void daa_0_bat_cb (int nDev, int nCh, unsigned short nStatus)
{
   irq_DAA_COM_bat_cb ( 0, nStatus ? DAA_BAT_EVT_BATTERY_DROP_IRQ :
                                     DAA_BAT_EVT_BATTERY_IRQ );
}

/**
   gpio callback function for the pol pin,
   this function decodes the status and triggers the generic state machine
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
*/
static IFX_return_t daa_0_init (void)
{
   VINETIC_GPIO_CONFIG ioCfg;
   int ret;


   vinDevHandle = VINETIC_OpenKernel(0, 0);
   if (vinDevHandle == IFX_ERROR)
   {
      printk("open failed\n\r");
      return IFX_ERROR;
   }

   vinDevIoHandle = VINETIC_GpioReserve(vinDevHandle, VINETIC_IO_DEV_GPIO_2 |
                                                      VINETIC_IO_DEV_GPIO_3 |
                                                      VINETIC_IO_DEV_GPIO_4 |
                                                      VINETIC_IO_DEV_GPIO_5 |
                                                      VINETIC_IO_DEV_GPIO_7);
   if (vinDevIoHandle == 0)
   {
      printk("reservation failed\n\r");
      return IFX_ERROR;
   }

   /* HOOKQ */
   ioCfg.nMode = GPIO_MODE_OUTPUT;;
   ioCfg.nGpio = VINETIC_IO_DEV_GPIO_2;
   ret = VINETIC_GpioConfig(vinDevIoHandle, &ioCfg);
   if (ret != IFX_SUCCESS)
   {
      printk("cfg output failed\n\r");
      return IFX_ERROR;
   }

   /* RINGQ */
   ioCfg.nMode = GPIO_MODE_INPUT | GPIO_MODE_INT | GPIO_INT_FALLING;
   ioCfg.nGpio = VINETIC_IO_DEV_GPIO_3;
   ioCfg.callback = daa_0_ring_cb;
   ret = VINETIC_GpioConfig(vinDevIoHandle, &ioCfg);
   if (ret != IFX_SUCCESS)
   {
      printk("cfg input 3 failed\n\r");
      return IFX_ERROR;
   }

   /* BATQ */
   ioCfg.nMode = GPIO_MODE_INPUT | GPIO_MODE_INT | GPIO_INT_RISING | GPIO_INT_FALLING;
   ioCfg.nGpio = VINETIC_IO_DEV_GPIO_4;
   ioCfg.callback = daa_0_bat_cb;
   ret = VINETIC_GpioConfig(vinDevIoHandle, &ioCfg);
   if (ret != IFX_SUCCESS)
   {
      printk("cfg input 4 failed\n\r");
      return IFX_ERROR;
   }

   /* POLQ */
   ioCfg.nMode = GPIO_MODE_INPUT | GPIO_MODE_INT | GPIO_INT_RISING | GPIO_INT_FALLING;
   ioCfg.nGpio = VINETIC_IO_DEV_GPIO_5;
   ioCfg.callback = daa_0_pol_cb;
   ret = VINETIC_GpioConfig(vinDevIoHandle, &ioCfg);
   if (ret != IFX_SUCCESS)
   {
      printk("cfg input 5 failed\n\r");
      return IFX_ERROR;
   }

#ifdef DAA_APOH
   /* APOH */
   ioCfg.nMode = GPIO_MODE_INPUT | GPIO_MODE_INT | GPIO_INT_RISING | GPIO_INT_FALLING;
   ioCfg.nGpio = VINETIC_IO_DEV_GPIO_7;
   ioCfg.callback = daa_0_apoh_cb;
   ret = VINETIC_GpioConfig(vinDevIoHandle, &ioCfg);
   if (ret != IFX_SUCCESS)
   {
      printk("cfg input 7 failed\n\r");
      return IFX_ERROR;
   }
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

   /* HOOKQ control GPIO2
      offhook => pin locic low
      onhook  => pin logic high */
   if (vinDevIoHandle != 0)
   {
      ret = VINETIC_GpioSet(vinDevIoHandle,
                            (offhook == IFX_TAPI_FXO_HOOK_OFFHOOK) ?
                                 0x00 : VINETIC_IO_DEV_GPIO_2,
                            VINETIC_IO_DEV_GPIO_2);
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
   unsigned short get;
   int ret = IFX_SUCCESS;

   /* HOOKQ control GPIO2 bit 0x04
      offhook => pin locic low
      onhook  => pin logic high */
   if (vinDevIoHandle != 0)
   {
      ret = VINETIC_GpioGet(vinDevIoHandle, &get, VINETIC_IO_DEV_GPIO_2);
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
   unsigned short get;
   int ret = IFX_SUCCESS;

   /* BATQ control GPIO4
      offhook => pin locic low
      onhook  => pin logic high */
   if (vinDevIoHandle != 0)
   {
      ret = VINETIC_GpioGet(vinDevIoHandle, &get, VINETIC_IO_DEV_GPIO_4);
      *bat = (! get) ? IFX_ENABLE : IFX_DISABLE;
   }
   else
   {
      ret = IFX_ERROR;
   }
   return ret;
}

/** **************************************************************************
   daa 0 pol get implementation (board specific)
   \param pol - current pol state (read back)
   \return IFX_SUCCESS or IFX_ERROR in case of failure
   \note direction cannot be clearly defined for all boards / FXO ports
   Clare Litelink reports low, if the ring line is negative to tip which is
   normally considered as normal polarity.
*/
static IFX_return_t daa_0_polGet (IFX_enDis_t *pol)
{
   unsigned short get;
   int ret = IFX_SUCCESS;

   /* POL (active high) control GPIO5
      normal   => pin locic low
      reverse  => pin logic high */
   if (vinDevIoHandle != 0)
   {
      ret = VINETIC_GpioGet(vinDevIoHandle, &get, VINETIC_IO_DEV_GPIO_5);
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
   unsigned short get;
   int ret = IFX_SUCCESS;

   /* APOH (active high) control GPIO7
      nopoh => pin locic low
      apoh  => pin logic high */
   if (vinDevIoHandle != 0)
   {
      ret = VINETIC_GpioGet(vinDevIoHandle, &get, VINETIC_IO_DEV_GPIO_7);
      *apoh = (get) ? IFX_ENABLE : IFX_DISABLE;
   }
   else
   {
      ret = IFX_ERROR;
   }
   return ret;
}
#endif /* DAA_APOH */

/**
   initialisation routine of daa 1, implementation is system dependant.
   This function is getting called on IFX_TAPI_LINE_TYPE_SET ioctl, if
   the channel is configured as FXO. This is the place to reserve and
   configure GPIOs for later use.
*/
static IFX_return_t daa_1_init (void)
{
   printk("init 1\n\r");
   return IFX_SUCCESS;
}


/**
   registration of system specific functions.
   This function is getting called on insmod of drv_daa in Linux sytems.
   Note that at this time you cannot access other device's GPIOs as
   they aren't yet initialized.
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
      DRV_DAA_MAX_DAA_CHANNELS in drv_daa_board_<xx>.h as well.

   gDaaFctMap[1].init      = daa_1_init;
      etc.
   */

   return IFX_SUCCESS;
}


#endif /* DRV_DAA_BOARD_TEMPLATE */
