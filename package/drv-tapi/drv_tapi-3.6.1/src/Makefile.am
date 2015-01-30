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

if KERNEL_2_6
bin_PROGRAMS = drv_tapi.ko
# dummy file required for distcheck target
drv_tapi_ko_SOURCES = @top_srcdir@/LICENSE
else
bin_PROGRAMS = drv_tapi
endif

## the driver itself
drv_tapi_SOURCES = \
    drv_tapi_linux.c\
    drv_tapi_common.c\
    drv_tapi_dial.c\
    drv_tapi_event.c\
    drv_tapi_ioctl.c\
    drv_tapi_meter.c\
    drv_tapi_misc.c\
    drv_tapi_opcontrol.c\
    drv_tapi_pcm.c\
    drv_tapi_ring.c\
    drv_tapi_tone.c\
    drv_tapi_cid.c\
    drv_tapi_voice.c\
    drv_tapi_init.c\
    drv_tapi_stream.c\
    drv_tapi_polling.c\
    drv_tapi_fxo.c\
    lib/lib_fifo/lib_fifo.c\
    lib/lib_bufferpool/lib_bufferpool.c\
    common/src/sys_drv_fifo.c\
    common/src/sys_drv_debug.c

if UDP_REDIRECT
drv_tapi_SOURCES +=\
   drv_tapi_qos.c
endif

if KPI_SUPPORT
drv_tapi_SOURCES +=\
   drv_tapi_kpi.c \
   common/src/sys_drv_kthread_linux.c
endif

EXTRA_DIST = \
	@top_srcdir@/drv_tapi_version.h\
	@top_srcdir@/include/drv_tapi_io.h\
    @top_srcdir@/include/drv_tapi_event_io.h\
    @top_srcdir@/include/drv_tapi_kpi_io.h\
    @top_srcdir@/include/ifx_types.h\
    @top_srcdir@/include/ifx_udp_redirect.h\
    @top_srcdir@/include/drv_tapi_ll_interface.h\
    @top_srcdir@/include/drv_tapi_fxo_ll_interface.h\
    @top_srcdir@/include/lib_fifo.h\
    @top_srcdir@/include/lib_bufferpool.h\
    @top_srcdir@/include/drv_tapi_qos.h\
    @top_srcdir@/include/drv_tapi_event.h\
    @srcdir@/drv_api.h\
    @srcdir@/drv_tapi.h\
    @srcdir@/drv_tapi_ioctl.h\
    @srcdir@/drv_tapi_api.h\
    @srcdir@/drv_tapi_cid.h\
    @srcdir@/drv_tapi_errno.h\
    @srcdir@/drv_tapi_linux.h\
    @srcdir@/drv_tapi_stream.h\
    @srcdir@/drv_tapi_polling.h\
    @srcdir@/drv_tapi_vxworks.h\
    @srcdir@/drv_tapi_vxworks.c\
    @srcdir@/drv_tapi_vxworks_bsp.c\
    @srcdir@/drv_tapi_ll_if_version.h\
    @srcdir@/drv_tapi_kpi.h\
    @srcdir@/drv_tapi_kpi.c\
    @srcdir@/drv_tapi_qos.c\
    @srcdir@/common/src/sys_drv_defs.h\
    @srcdir@/common/src/sys_drv_fifo.h\
    @srcdir@/common/src/sys_drv_debug.h\
    @srcdir@/common/src/sys_drv_ifxos.h\
    @srcdir@/common/src/sys_drv_delaylib_vxworks.h\
    @srcdir@/common/src/sys_drv_delaylib_vxworks.c\
    @srcdir@/common/src/sys_drv_timerlib_vxworks.c\
    @srcdir@/common/src/sys_drv_timerlib_vxworks.h\
    @srcdir@/common/src/sys_drv_kthread_linux.h\
    @srcdir@/common/src/sys_drv_kthread_linux.c\
    @srcdir@/drv_tapi.wpj

INCLUDES  = -I@abs_srcdir@ \
            -I@abs_top_srcdir@ \
			-I@abs_top_builddir@ \
            -I@abs_top_srcdir@/include \
            -I@abs_srcdir@/common/src

if KERNEL_2_6
INCLUDES += -I@KERNEL_INCL_PATH@ \
		    -I@KERNEL_INCL_PATH@/asm/mach-generic \
            -I@KERNEL_BUILD_PATH@/include \
            -I@KERNEL_BUILD_PATH@/include2
endif

if ! KERNEL_2_6
INCLUDES += -I@KERNEL_INCL_PATH@
endif

if EVENT_LOGGER_DEBUG
INCLUDES += -I@top_srcdir@/../event_logger/src
endif


if WARNINGS
AM_CFLAGS = -Wall -Wimplicit -Wreturn-type -Wunused -Wswitch -Wcomment -Wparentheses
# -Wpointer-arith
else
AM_CFLAGS = -Wall
endif

## flags for the driver
if USE_MODULE
drv_tapi_CFLAGS = -DLINUX -D__KERNEL__ -DMODULE -Wno-format -DEXPORT_SYMTAB $(AM_CFLAGS)
else
drv_tapi_CFLAGS = -DLINUX -D__KERNEL__ -Wno-format -DEXPORT_SYMTAB $(AM_CFLAGS)
endif

if KERNEL_2_6
drv_tapi_CFLAGS += -fno-common
endif


drv_tapi_LDFLAGS = -r -nostdlib

lint:
	@flint ../std_lx.lnt $(INCLUDES) -u\
	-I$(shell dirname `$(CC) -print-file-name=include`)/include \
	$(drv_tapi_SOURCES)

install-exec-hook: $(bin_PROGRAMS)
	@for p in $(bin_PROGRAMS); do \
		$(STRIP) --strip-debug $(DESTDIR)$(bindir)/$$p; \
		echo "  $(STRIP) --strip-debug $(DESTDIR)$(bindir)/$$p"; \
	done

# Extra rule for linux-2.6 kernel object
if KERNEL_2_6
drv_tapi_OBJS = "$(subst .c,.o,$(notdir $(drv_tapi_SOURCES)))"

drv_tapi.ko: $(drv_tapi_SOURCES) $(EXTRA_DIST)
	@echo -e "Making Linux 2.6.x kernel object"
	@if test ! -e drv_tapi_event.c ; then \
		echo "create symlinks"; \
		cp -s $(filter %.c,$(addprefix @abs_srcdir@/,$(drv_tapi_SOURCES))) $(PWD)/ ; \
	fi
	@echo -e "# drv_tapi: Generated to build Linux 2.6.x kernel object" > $(PWD)/Kbuild
	@echo -e "obj-m := $(subst .ko,.o,$@)"			>> $(PWD)/Kbuild
	@echo -e "$(subst .ko,,$@)-y := $(drv_tapi_OBJS)"	>> $(PWD)/Kbuild
	@echo -e "EXTRA_CFLAGS := -DHAVE_CONFIG_H  $(CFLAGS) $(drv_tapi_CFLAGS) $(INCLUDES)"	>> $(PWD)/Kbuild
	$(MAKE) ARCH=@KERNEL_ARCH@ -C @KERNEL_BUILD_PATH@ O=@KERNEL_BUILD_PATH@ M=$(PWD) modules

clean-generic:
	@echo "drv_tapi: Cleanup Linux 2.6.x kernel object build"
	@- find . -name ".*.cmd" | xargs rm -f
	@- rm -f Module.symvers Kbuild
	@- rm -rf .tmp_versions drv_tapi.mod.c

endif
