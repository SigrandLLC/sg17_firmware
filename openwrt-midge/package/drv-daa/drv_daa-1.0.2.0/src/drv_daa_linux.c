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

/*   Description : DAA Device Driver, Linux part */


#include "drv_daa_api.h"
#include "drv_daa_common.h"

#ifdef LINUX

/* ============================= */
/* Global Declarations           */
/* ============================= */

static IFX_int8_t                debug_level       = DBG_LEVEL_HIGH;

MODULE_PARM(debug_level, "b");
MODULE_PARM_DESC(debug_level, "set to get more (1) or fewer (4) debug outputs");


#if CONFIG_PROC_FS
/**
   Read the version information from the driver.
   \return
   length
*/
static int daa_get_version_proc (char *buf)
{
    int len;

    len = sprintf(buf, "%s\n", &DAA_WHATVERSION[4]);

    len += sprintf(buf + len, "Compiled on %s, %s for Linux kernel %s\n",
                   __DATE__, __TIME__, UTS_RELEASE);

    return len;
}


/**
   Read the status information from the driver.

   \return
   length
*/
static int daa_get_status_proc (char *buf)
{
    int len=0;
#if 0
    int i, j;

    len += sprintf(buf+len, "Interrupt-Count %d\n", cpc5621_intcount);

    for (i=0; i<MAX_DEVICE; i++)
    {
        for (j=0; j<MAX_CHANNEL; j++)
        {
            len += sprintf(buf+len, "********************************\n");
            len += sprintf(buf+len, "pDev(%d, %d) = 0x%08X\n", i, j, (int)CPC5621_Devices[i][j]);
            len += sprintf(buf+len, "--------------------------------\n");
            if (CPC5621_Devices[i][j] != NULL)
            {
                len += sprintf(buf+len, "bInit   = %s\n", CPC5621_Devices[i][j]->bInit?"TRUE":"FALSE");
                len += sprintf(buf+len, "bOpen   = %s\n", CPC5621_Devices[i][j]->bOpen?"TRUE":"FALSE");
            }
        }
    }
#endif
    return len;
}


/**
   The proc filesystem: function to read and entry.

   \return
   length
*/
static int daa_read_proc (char *page, char **start, off_t off,
                          int count, int *eof, void *data)
{
    int len;

    int (*fn)(char *buf);

    if (data != NULL)
    {
        fn = data;
        len = fn(page);
    }
    else
        return 0;

    if (len <= off+count)
        *eof = 1;
    *start = page + off;
    len -= off;
    if (len>count)
        len = count;
    if (len<0)
        len = 0;
    return len;
}
#endif


/**
   Initialize the module

   \return
   Error code or 0 on success
   \remark
   Called by the kernel.
*/
int init_module(void)
{
   int err;
#if CONFIG_PROC_FS
   struct proc_dir_entry *driver_proc_node;
#endif

   printk("%s, (c) 2007, Infineon Technologies AG\n\r", DAA_WHATVERSION);

   SetTraceLevel(DAA_DRV, debug_level);

   err = DAA_COM_OnInsmod();
   if (err != IFX_SUCCESS)
      printk("DAA ERR DAA_COM_OnInsmod failed\n\r");

   IFX_TAPI_Register_DAA_Drv(DAA_CtxGet());

#if CONFIG_PROC_FS
   /* install the proc entry */
   TRACE(DAA_DRV,DBG_LEVEL_LOW,("DAA_DRV: using proc fs\n"));

   driver_proc_node = proc_mkdir("driver/" DRV_DAA_NAME, NULL);
   if (driver_proc_node != NULL)
   {
      create_proc_read_entry("version", S_IFREG|S_IRUGO,
                             driver_proc_node, daa_read_proc, (void *)daa_get_version_proc );
      create_proc_read_entry("status", S_IFREG|S_IRUGO,
                             driver_proc_node, daa_read_proc, (void *)daa_get_status_proc );
   }
   else
   {
      TRACE(DAA_DRV,DBG_LEVEL_HIGH,("init_module: cannot create proc entry\n"));
   }
#endif

    return 0;
}


/**
   Clean up the module if unloaded.

   \remark
   Called by the kernel.
*/
void cleanup_module(void)
{
   int err;
#if CONFIG_PROC_FS
   remove_proc_entry("driver/" DRV_DAA_NAME "/version", 0);
   remove_proc_entry("driver/" DRV_DAA_NAME "/status", 0);
   remove_proc_entry("driver/" DRV_DAA_NAME, 0);
#endif

   /* unregister DAA references from TAPI */
   IFX_TAPI_Register_DAA_Drv(IFX_NULL);

   /* uninitialize device specific settings */
   err = DAA_COM_OnRmmod();
   if (err != IFX_SUCCESS)
      printk("DAA ERR DAA_COM_OnRmmod failed\n\r");

   TRACE(DAA_DRV, DBG_LEVEL_NORMAL, ("DAA_DRV: cleanup successful\n"));
}


/****************************************************************************/

MODULE_AUTHOR("Infineon Technologies AG");
MODULE_DESCRIPTION("IFX TAPI DAA abstraction module");
MODULE_SUPPORTED_DEVICE("IFX supported DAAs");

#endif /* LINUX */
