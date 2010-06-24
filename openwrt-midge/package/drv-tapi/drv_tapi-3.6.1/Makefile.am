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


AUTOMAKE_OPTIONS = foreign 1.6 nostdinc

SUBDIRS = src

DISTCHECK_CONFIGURE_FLAGS=@CONFIGURE_OPTIONS@

drv_tapi_docdir = ${prefix}/doc/drv_tapi

EXTRA_DIST = $(drv_tapi_doc_DATA)\
	doc/doxyconfig\
	doc/footer.html\
	doc/header.html\
	doc/logo.gif\
	doc/stylesheet.css

# Make subdirectories inside tar.gz writable for everyone
# (required for 2.6 kbuild)
dist-hook:
	find $(distdir) -type d -exec chmod 777 {} \; -print

# Create self extracting linux distribution
distcheck-hook:
	makeself.sh --gzip --notemp $(distdir) drv_tapi-$(PACKAGE_VERSION).sh \
	"$(PACKAGE_NAME)"

lint:
	@target=lint; \
    list='$(SUBDIRS)'; \
    for subdir in $$list; do \
       echo "Making $$target in $$subdir"; \
       if test "$$subdir" = "."; then \
          dot_seen=yes; \
          local_target="$$target-am"; \
       else \
          local_target="$$target"; \
       fi; \
       (cd $$subdir && $(MAKE) $(AM_MAKEFLAGS) $$local_target) \
       || eval $$failcom; \
     done;

errno:
	@top_srcdir@/script/updateErrno.pl @top_srcdir@/src/drv_tapi_strerrno.h @top_srcdir@/src

style:
	find @top_srcdir@/src -type f -name '*.[hc]' -exec @top_srcdir@/script/checkStyle.pl {} \;
