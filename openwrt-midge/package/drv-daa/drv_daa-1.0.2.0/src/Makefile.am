#                              Copyright (c) 2006
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


bin_PROGRAMS = drv_daa
drv_daa_SOURCES= \
      drv_daa_api.c \
      drv_daa_common.c \
      drv_daa_common_sm_ring.c \
      drv_daa_common_sm_apoh.c \
      drv_daa_common_sm_pol.c \
      drv_daa_common_sm_bat.c \
      drv_daa_board_template.c \
      drv_daa_board_easy3332.c \
      drv_daa_board_easy50712.c \
      drv_daa_board_easy3201.c \
      drv_daa_linux.c


EXTRA_DIST = \
      drv_daa.h \
      drv_daa_api.h \
      drv_daa_boards.h \
      drv_daa_linux.h \
      drv_daa_common.h \
      drv_daa_common_priv.h \
      drv_daa_board_template.h \
      drv_daa_board_easy3332.h \
      drv_daa_board_easy50712.h \
      drv_daa_board_easy3201.h


drv_daa_LDFLAGS = -r -nostdlib
drv_daa_CFLAGS  = -D@BOARD_NAME@ -D__KERNEL__ \
                  -DLINUX -DMODULE -DMODVERSIONS -DEXPORT_SYMTAB \
				  -Wall -Wstrict-prototypes -Wno-trigraphs -Wimplicit \
				  -Wreturn-type -Wunused -Wswitch -Wcomment -Wparentheses \
				  $(AM_CFLAGS)

INCLUDES = -I@srcdir@ \
           -I@top_srcdir@ \
           -I@KERNEL_INCL_PATH@ \
           -I@TAPI_INCL_PATH@ -I@TAPI_INCL_PATH@/../src \
           -I@TAPI_INCL_PATH@/../src/common/src \
           -I@DUSLIC_INCL_PATH@ -I@DUSLIC_INCL_PATH@/../src \
           -I@VINETIC_INCL_PATH@ -I@VINETIC_INCL_PATH@/../src \
           -I@DXT_INCL_PATH@ -I@DXT_INCL_PATH@/../src

lint:
	@flint ../std_lx.lnt $(INCLUDES) -u\
		-I$(shell dirname `$(CC) -print-file-name=include`)/include \
		$(drv_tapi_SOURCES)

install-exec-hook: $(bin_PROGRAMS)
	@for p in $(bin_PROGRAMS); do \
		$(STRIP) --strip-debug $(DESTDIR)$(bindir)/$$p; \
		echo "  $(STRIP) --strip-debug $(DESTDIR)$(bindir)/$$p"; \
	done

