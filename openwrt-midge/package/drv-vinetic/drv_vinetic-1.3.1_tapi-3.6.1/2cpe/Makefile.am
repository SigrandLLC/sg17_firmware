#                              Copyright (c) 2007
#                           Infineon Technologies AG
#                    Am Campeon 1-12; 81726 Munich, Germany
#
# THE DELIVERY OF THIS SOFTWARE AS WELL AS THE HEREBY GRANTED NON-EXCLUSIVE,
# WORLDWIDE LICENSE TO USE, COPY, MODIFY, DISTRIBUTE AND SUBLICENSE THIS
# SOFTWARE IS FREE OF CHARGE.
#
# THE LICENSED SOFTWARE IS PROVIDED "AS IS" AND INFINEON EXPRESSLY DISCLAIMS
# ALL REPRESENTATIONS AND WARRANTIES, WHETHER EXPRESS OR IMPLIED, INCLUDING
# WITHOUT LIMITATION, WARRANTIES OR REPRESENTATIONS OF WORKMANSHIP,
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, DURABILITY, THAT THE
# OPERATING OF THE LICENSED SOFTWARE WILL BE ERROR FREE OR FREE OF ANY THIRD
# PARTY CLAIMS, INCLUDING WITHOUT LIMITATION CLAIMS OF THIRD PARTY INTELLECTUAL
# PROPERTY INFRINGEMENT.
#
# EXCEPT FOR ANY LIABILITY DUE TO WILFUL ACTS OR GROSS NEGLIGENCE AND EXCEPT
# FOR ANY PERSONAL INJURY INFINEON SHALL IN NO EVENT BE LIABLE FOR ANY CLAIM
# OR DAMAGES OF ANY KIND, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
# ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
# DEALINGS IN THE SOFTWARE.


if VINETIC_2CPE
if KERNEL_2_6
bin_PROGRAMS = drv_vinetic.ko
# dummy file required for distcheck target
drv_vinetic_ko_SOURCES = @top_srcdir@/LICENSE
else
bin_PROGRAMS = drv_vinetic
endif

## VINETIC 2CPE driver related sources.
TYPE_SPEC_SOURCES = \
   drv_vinetic_api.c \
   drv_vinetic_int.c \
   drv_vinetic_host.c \
   drv_vinetic_access.c \
   drv_vinetic_bbd.c \
   drv_vinetic_gpio.c \
   drv_vinetic_gr909.c \
   drv_vinetic_alm_cpe.c \
   drv_vinetic_hosttapi.c \
   drv_vinetic_polling.c


## Common VINETIC driver sources
drv_vinetic_SOURCES = \
   ../src/drv_vinetic_ioctl.c \
   ../src/drv_vinetic_basic.c \
   ../src/drv_vinetic_dwnld.c \
   ../src/drv_vinetic_main.c \
   ../src/drv_vinetic_misc.c \
   ../src/drv_vinetic_stream.c \
   ../src/drv_vinetic_linux.c \
   ../src/drv_vinetic_dspconf.c \
   ../src/drv_vinetic_sig.c \
   ../src/drv_vinetic_sig_cptd.c \
   ../src/drv_vinetic_sig_dtmfg.c \
   ../src/drv_vinetic_sig_mftd.c \
   ../src/drv_vinetic_sig_utd.c \
   ../src/drv_vinetic_sig_utg.c \
   ../src/drv_vinetic_sig_cid.c \
   ../src/drv_vinetic_init.c \
   ../src/drv_vinetic_init_cap.c \
   ../src/drv_vinetic_pcm.c \
   ../src/drv_vinetic_alm.c \
   ../src/drv_vinetic_alm_tone.c \
   ../src/drv_vinetic_con.c \
   ../src/drv_vinetic_cod.c \
   ../src/drv_vinetic_cod_aal.c \
   ../src/drv_vinetic_qos.c \
   ../src/drv_vinetic_event.c \
   ../src/lib/lib_bbd/src/lib_bbd.c \
   ../src/lib/lib_fifo/lib_fifo.c \
   ../src/common/src/sys_drv_fifo.c \
   ../src/common/src/sys_drv_debug.c \
   $(TYPE_SPEC_SOURCES)

if EVALUATION
nodist_drv_vinetic_SOURCES = \
   ../src/evaluation/drv_vinetic_mbxtest.c \
   ../src/evaluation/drv_vinetic_streameval.c \
   ../src/evaluation/drv_vinetic_eval.c \
   ../src/evaluation/mpc862_streameval.c \
   ../src/evaluation/drv_vinetic_evalint.c \
   ../src/evaluation/drv_vinetic_lec.c
endif


EXTRA_DIST = \
   @top_srcdir@/include/vinetic_io.h \
   @top_srcdir@/include/drv_vinetic_errno.h \
   @srcdir@/drv_vinetic_hostapi.h \
   @srcdir@/drv_vinetic_dcctl.h \
   @srcdir@/drv_vinetic_int.h \
   @srcdir@/drv_vinetic_host.h \
   @srcdir@/drv_vinetic_access.h \
   @srcdir@/drv_vinetic_bbd.h \
   @srcdir@/drv_vinetic_alm_cpe.h \
   @top_srcdir@/src/lib/lib_bbd/src/lib_bbd.h \
   @top_srcdir@/src/drv_vinetic_basic.h \
   @top_srcdir@/src/drv_vinetic_dwnld.h \
   @top_srcdir@/src/drv_vinetic_main.h \
   @top_srcdir@/src/drv_vinetic_misc.h \
   @top_srcdir@/src/drv_vinetic_sig.h \
   @top_srcdir@/src/drv_vinetic_sig_priv.h \
   @top_srcdir@/src/drv_vinetic_gpio.h \
   @top_srcdir@/src/drv_vinetic_gpio_user.h \
   @top_srcdir@/src/drv_vinetic_stream.h \
   @top_srcdir@/src/drv_vinetic_con.h \
   @top_srcdir@/src/drv_vinetic_tone.h \
   @top_srcdir@/src/drv_vinetic_dspconf.h \
   @top_srcdir@/src/drv_vinetic_pcm.h \
   @top_srcdir@/src/drv_vinetic_alm.h \
   @top_srcdir@/src/drv_vinetic_cod.h \
   @top_srcdir@/src/drv_vinetic_cod_aal.h \
   @top_srcdir@/src/drv_vinetic_sig_cid.h \
   @top_srcdir@/src/drv_vinetic_init.h \
   @top_srcdir@/src/drv_vinetic_pcm_priv.h \
   @top_srcdir@/src/drv_vinetic_alm_priv.h \
   @top_srcdir@/src/drv_vinetic_cod_priv.h \
   @top_srcdir@/src/drv_vinetic_con_priv.h \
   @top_srcdir@/src/drv_vinetic_qos.c \
   @top_srcdir@/src/drv_vinetic_qos.h \
   @top_srcdir@/src/drv_vinetic_ll_if_version.h\
   @top_srcdir@/drv_version.h \
   @top_srcdir@/src/drv_api.h \
   @top_srcdir@/src/drv_vinetic.h \
   @top_srcdir@/src/drv_vinetic_api.h \
   @top_srcdir@/src/drv_vinetic_vxworks.c \
   @top_srcdir@/src/drv_config_user.default.h \
   @top_srcdir@/src/drv_config_user.danube.h \
   @top_srcdir@/src/drv_vinetic_vxworks.h \
   @top_srcdir@/src/common/src/ifx_types.h \
   @top_srcdir@/src/common/src/sys_drv_delaylib_vxworks.c \
   @top_srcdir@/src/common/src/sys_drv_delaylib_vxworks.h \
   @top_srcdir@/src/common/src/sys_drv_ifxos.h \
   @top_srcdir@/src/common/src/sys_drv_kthread_linux.c \
   @top_srcdir@/src/common/src/sys_drv_kthread_linux.h \
   @top_srcdir@/src/common/src/sys_drv_timerlib_vxworks.c \
   @top_srcdir@/src/common/src/sys_drv_timerlib_vxworks.h \
   @top_srcdir@/src/common/src/sys_drv_fifo.h \
   @top_srcdir@/src/common/src/sys_drv_debug.h \
   @top_srcdir@/src/common/src/sys_drv_defs.h \
   @top_srcdir@/src/mpc/src/drv_mpc.h \
   @top_srcdir@/src/lib/lib_fifo/lib_fifo.c \
   @top_srcdir@/src/lib/lib_fifo/lib_fifo.h \
   @top_srcdir@/lib_tapi/lib_tapi_lt_gr909.h \
   @top_srcdir@/lib_tapi/lib_tapi_lt_vincpe.c \
   @top_srcdir@/src/lib/lib_bufferpool/lib_bufferpool.h \
   @top_srcdir@/src/lib/lib_bufferpool/lib_bufferpool.c \
   @srcdir@/drv_vinetic.wpj


INCLUDES = \
   -I@abs_top_builddir@ \
   -I@abs_top_srcdir@ \
   -I@abs_top_srcdir@/include \
   -I@abs_srcdir@ \
   -I@KERNEL_INCL_PATH@ \
   -I@KERNEL_INCL_PATH@/asm/mach-generic \
   -I@KERNEL_BUILD_PATH@/include \
   -I@KERNEL_BUILD_PATH@/include2 \
   -I@abs_top_srcdir@/src \
   -I@abs_top_srcdir@/2cpe \
   -I@abs_top_srcdir@/src/common/src \
   -I@abs_top_srcdir@/src/lib/lib_fifo \
   -I@abs_top_srcdir@/src/lib/lib_bufferpool \
   -I@abs_top_srcdir@/src/lib/lib_bbd/src \
   -I@abs_top_srcdir@/src/bin_images \
   -I@abs_top_srcdir@/src/mpc/src \
   -I@abs_top_srcdir@/src/evaluation \
   -I@abs_top_srcdir@/src/testing \
   -I@abs_top_srcdir@/src/../../drv_spi/src

if HL_TAPI_INCL_PATH_SET
INCLUDES += -I@HL_TAPI_INCL_PATH@
else
INCLUDES += -I@top_srcdir@/src/drv_tapi/include
endif

if EVENT_LOGGER_DEBUG
INCLUDES += -I@abs_top_srcdir@/../event_logger/src
endif
if EVALUATION
INCLUDES += -I@top_srcdir@/src/mpc/src \
            -I@top_srcdir@/../../comacsd_bootloader/u-boot/include
endif

if WARNINGS
AM_CFLAGS = -Wall -Wimplicit -Wreturn-type -Wunused -Wswitch -Wcomment -Wparentheses
#-Wpointer-arith
else
AM_CFLAGS = -Wall
endif

drv_vinetic_CFLAGS =-D__LINUX__ -DLINUX -D__KERNEL__ -DMODULE -DEXPORT_SYMTAB $(AM_CFLAGS)
if KERNEL_2_6
drv_vinetic_CFLAGS += -fno-common
endif

drv_vinetic_LDFLAGS = -r -nostdlib

lint:
	@flint @top_srcdir@/std_lx.lnt $(INCLUDES)\
	-I$(shell dirname `$(CC) -print-file-name=include`)/include \
	$(drv_vinetic_SOURCES)

rsm:
	rsm -fa -n -s -Ta $(drv_vinetic_SOURCES) $(EXTRA_DIST)

install-exec-hook: $(bin_PROGRAMS)
	@for p in $(bin_PROGRAMS); do \
		$(STRIP) --strip-debug $(DESTDIR)$(bindir)/$$p; \
		echo "  $(STRIP) --strip-debug $(DESTDIR)$(bindir)/$$p"; \
	done

# Extra rule for linux-2.6 kernel object
if KERNEL_2_6
drv_vinetic_OBJS = "$(subst .c,.o, $(notdir $(drv_vinetic_SOURCES)))"

drv_vinetic.ko:
	@echo -e "Making Linux 2.6.x kernel object"
	@if test ! -e drv_vinetic_int.c ; then \
		echo "create symlinks"; \
		cp -s $(filter %.c,$(addprefix @abs_srcdir@/,$(drv_vinetic_SOURCES))) $(PWD)/ ; \
	fi
	@echo -e "# drv_vinetic: Generated to build Linux 2.6.x kernel object" > $(PWD)/Kbuild
	@echo -e "obj-m := $(subst .ko,.o,$@)"			>> $(PWD)/Kbuild
	@echo -e "$(subst .ko,,$@)-y := $(drv_vinetic_OBJS)"	>> $(PWD)/Kbuild
	@echo -e "EXTRA_CFLAGS := -DHAVE_CONFIG_H  $(CFLAGS) $(drv_vinetic_CFLAGS) $(INCLUDES)"	>> $(PWD)/Kbuild
	$(MAKE) ARCH=@KERNEL_ARCH@ -C @KERNEL_BUILD_PATH@ O=@KERNEL_BUILD_PATH@ M=$(PWD) modules

clean-generic:
	@echo "drv_vinetic: Cleanup Linux 2.6.x kernel object build"
	@- find . -name ".*.cmd" | xargs rm -f
	@- rm -f Module.symvers Kbuild
	@- rm -rf .tmp_versions drv_vinetic.mod.c
endif

endif
