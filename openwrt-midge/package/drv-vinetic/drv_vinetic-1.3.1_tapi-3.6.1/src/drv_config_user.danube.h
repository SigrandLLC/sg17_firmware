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

/** \file drv_config_user.h
   This file is intended to customize some optional settings of the driver.

   In case of optional features (like SPI interface) it may also contain
   some required definitions where no default values are making sense.

   This file will only be used if the compiler switch ENABLE_USER_CONFIG
   is defined (e.g. by "configure --enable-user-config")

   \remark
   This file (drv_config_user.danube.h) is already prepared for systems
   with the danube chipset.

   To use this file, make a copy or link with the name "drv_config_user.h"
   in your build-directory.
*/

#ifndef _DRV_CONFIG_USER_H
#define _DRV_CONFIG_USER_H

/* ============================= */
/* Includes                      */
/* ============================= */
#include <asm/danube/ifx_ssc.h>
#include <asm/danube/irq.h>
#include <asm/danube/port.h>
#include <asm/danube/danube.h>

extern int ifx_ssc_cs_low(u32 pin);
extern int ifx_ssc_cs_high(u32 pin);
extern int ifx_ssc_txrx(char * tx_buf, u32 tx_len, char * rx_buf, u32 rx_len);


/* ============================= */
/* Defines                       */
/* ============================= */

/** activate the new callback mechanism to provide control of the CS handling
    to the SPI bus driver. drv_vinetic will register a callback (CB) function
    (v2cpe_spi_cs_handler()) to the SPI bus driver which allows to define
    exactly which pins to use for CS handling of VINETIC */
#if 0
#define VIN_USE_SPI_CS_CALLBACK
#endif /* 0 */


/** SPI initialisation
    This macro is called during chip initialisation (if SPI is used) and
    can be extended to do board specific initialisation related to the
    VINETIC-CPE.
 */
#ifdef VIN_USE_SPI_CS_CALLBACK
#define SPI_INIT(pDev) do {                                        \
   /* NOTICE, GPIO2 was meant for RELAY, which switch ch0 */       \
   /* between FXS (high) or FX0 (low) */                           \
   /* Prepare GPIO pin 2 as input and interrupt line */            \
   *DANUBE_GPIO_P0_DIR &= ~(1<<2);                                 \
   *DANUBE_GPIO_P0_ALTSEL0 &= ~(1<<2);                             \
   *DANUBE_GPIO_P0_ALTSEL1 |= 1<<2;                                \
   *DANUBE_ICU_EIU_EXIN_C =                                        \
            (*DANUBE_ICU_EIU_EXIN_C & 0xff) | 0x600;               \
   *DANUBE_ICU_EIU_INEN |= 1<<2;                                   \
   /* portdriver - configure GPIO12 as output - used for CS */     \
   danube_port_reserve_pin (0,12,1);                               \
   danube_port_clear_altsel0 (0,12,1);                             \
   danube_port_set_dir_out (0,12,1);                               \
   printk("VINETIC-CPE device driver - init GPIO2,12\n\r");        \
   pDev->spi_dev_id =  ifx_ssc_register_spi_dev("vinetic-cpe");    \
   ifx_ssc_register_dev_interrupt(pDev->spi_dev_id,                \
                                  pDev->pIrq->nIrq);               \
   ifx_ssc_register_dev_spi_bus_mode(pDev->spi_dev_id, 3);         \
   ifx_ssc_register_dev_spi_bus_speed(pDev->spi_dev_id, 5000000);  \
   ifx_ssc_register_dev_cs_handler(pDev->spi_dev_id,               \
                                   v2cpe_spi_cs_handler,           \
                                   pDev->nDevNr);                  \
} while(0)
#else
#define SPI_INIT(pDev) do {                                        \
   /* NOTICE, GPIO2 was meant for RELAY, which switch ch0 */       \
   /* between FXS (high) or FX0 (low) */                           \
   /* Prepare GPIO pin 2 as input and interrupt line */            \
   *DANUBE_GPIO_P0_DIR &= ~(1<<2);                                 \
   *DANUBE_GPIO_P0_ALTSEL0 &= ~(1<<2);                             \
   *DANUBE_GPIO_P0_ALTSEL1 |= 1<<2;                                \
   *DANUBE_ICU_EIU_EXIN_C =                                        \
            (*DANUBE_ICU_EIU_EXIN_C & 0xff) | 0x600;               \
   *DANUBE_ICU_EIU_INEN |= 1<<2;                                   \
   /* portdriver - configure GPIO12 as output - used for CS */     \
   danube_port_reserve_pin (0,12,1);                               \
   danube_port_clear_altsel0 (0,12,1);                             \
   danube_port_set_dir_out (0,12,1);                               \
   printk("VINETIC-CPE device driver - init GPIO2,12\n\r");        \
   /*configure the spi to mode 3*/                                 \
   *(u32*)(0xbe100818)|=1;                                         \
   *(u32*)(0xbe100810)=(*(u32*)(0xbe100810) & ~(3<<5))|(1<<6);     \
   *(u32*)(0xbe100818)|=2;                                         \
   printk("switch to SPI mode 3\n\r");                             \
} while(0)
#endif /* VIN_USE_SPI_CS_CALLBACK */

/* SPI buffer size configured */
#define SPI_MAXBYTES_SIZE            32  /*SPI_BUFFER_SIZE*/


/** SPI chipselect set/unset */
#define SPI_CS_SET(nDevNr, high_low)     do  {        \
            if ((high_low) == 0)                      \
            {                                         \
               danube_port_clear_output(0,12,1);      \
            }                                         \
            else                                      \
            {                                         \
               danube_port_set_output(0,12,1);        \
            }                                         \
         } while(0)

/* spi low level access function */
#define spi_ll_read_write(txptr,txsize,rxptr,rxsize)  \
        ifx_ssc_txrx ((txptr),(txsize),(rxptr),(rxsize))

#endif /* _DRV_CONFIG_USER_H */

