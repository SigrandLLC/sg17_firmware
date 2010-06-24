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

/*#include <linux/config.h>*/
#include <linux/version.h>

#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/signal.h>

#include <asm/semaphore.h>

#include <linux/smp_lock.h>

#include "sys_drv_kthread_linux.h"

/* private functions */
static void kthread_launcher(void *data)
{
   kthread_t *kthread               = data;
   IFX_int32_t retVal               = IFX_ERROR;

   if(kthread == IFX_NULL)
   {
      return;
   }

   /* do LINUX specific setup */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0))
   daemonize();
   reparent_to_init();
#else
   daemonize(kthread->name);
#endif

   kthread->running = 1;
   kthread->function((void*)kthread);
   kthread->running = 0;

   /* When kthread->function exits (upon a check
    * kthread->terminate == 1 is TRUE),
    * thread gets completed.
    */
   complete_and_exit(&kthread->thrCompletion, (long)retVal);
}

/* public functions */

/* create a new kernel thread. Called by the creator. */
void start_kthread(void (*func)(kthread_t *), kthread_t *kthread,  char *name)
{

   if (kthread == IFX_NULL)
   {
      return;
   }

   /* Set the name of the future thread */
   strncpy(kthread->name, name, KTHREAD_NAME_LEN - 1);
   kthread->name[KTHREAD_NAME_LEN - 1] = '\0';

   /* Initialise termination flag */
   kthread->terminate = 0;

   /* Initialise thread function pointer */
   kthread->function = func;

   /* Initialise the completion structure*/
	init_completion(&kthread->thrCompletion);

   /* start kernel thread via the wrapper function */
   kthread->pid = kernel_thread( (int (*)(void *))kthread_launcher,
                  (void *)kthread,
                  (CLONE_FS | CLONE_FILES));

}

/* stop a kernel thread. Called by the removing instance */
void stop_kthread(kthread_t *kthread)
{
   if (kthread == IFX_NULL)
   {
      return;
   }

   /* this function needs to be protected with the big
      kernel lock (lock_kernel()). The lock must be
      grabbed before changing the terminate
      flag and released after the down() call. */
   lock_kernel();

   mb();

   /* force thread to shutdown */
   kthread->terminate = 1;

   mb();

   kill_proc(kthread->pid, SIGKILL, 1);
   /* Wait until a thread is completed and exites by
    * complete_and_exit(...) call.
    */

   /* release the big kernel lock */
   unlock_kernel();

   wait_for_completion (&kthread->thrCompletion);

   if (kthread->running != 0)
   {
      printk("ERROR - Periodic Task <%s> not stopped\n\r",
            kthread->name);
   }

   /* now we are sure the thread is in zombie state. We
      notify keventd to clean the process up.
   */
   kill_proc(2, SIGCHLD, 1);
}

/* initialize new created thread. Called by the new thread. */
void init_kthread(kthread_t *kthread)
{
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0))
   /* lock the kernel. A new kernel thread starts without
      the big kernel lock, regardless of the lock state
      of the creator (the lock level is *not* inheritated)
   */
   lock_kernel();

   /* set signal mask to what we want to respond */
   siginitsetinv(&current->blocked, sigmask(SIGKILL)|sigmask(SIGINT)|sigmask(SIGTERM));

   /* set name of this process */
   strcpy(current->comm, kthread->name);

   /* let others run */
   unlock_kernel();
#else
   /* Enable signals in Kernel >= 2.6 */
   allow_signal(SIGKILL);
   allow_signal(SIGINT);
   allow_signal(SIGTERM);
#endif

   /* initialise termination flag */
   kthread->terminate = 0;

}

/* cleanup of thread. Called by the exiting thread. */
void exit_kthread(kthread_t *kthread)
{
   /* This function is empty and
    * remains only for backward compatibility
    * reason.
    */
}


