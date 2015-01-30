include $(TOPDIR)/rules.mk

# bzr clone http://www.greenend.org.uk/rjk/bzr/sftpserver.dev sftpserver
PKG_NAME        = sftpserver
PKG_VERSION     = 0.0
PKG_RELEASE     = 1

PKG_DIR         = sftpserver

DROPBEAR_SFTPSERVER_PATH = /usr/libexec/sftp-server

   PKG_CONF_DIR = $(TOPDIR)/package/$(PKG_NAME)
    PKG_SRC_DIR = $(PKG_CONF_DIR)/$(PKG_DIR)
  PKG_BUILD_DIR =    $(BUILD_DIR)/$(PKG_DIR)
PKG_INSTALL_DIR = $(PKG_BUILD_DIR)/install

include $(TOPDIR)/package/target-configure-staging-flags.mk
TARGET_CFLAGS += -I$(STAGING_DIR)/usr/include/iconv
TARGET_CFLAGS += -L$(STAGING_DIR)/usr/lib/iconv -lpthread

include $(TOPDIR)/package/rules.mk

$(eval $(call PKG_template,SFTPSERVER,$(PKG_NAME),$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))

$(PKG_BUILD_DIR)/.prepared:
	rm -rf               $(PKG_BUILD_DIR)
	cp -a $(PKG_SRC_DIR) $(PKG_BUILD_DIR)
	mkdir -p $(PKG_INSTALL_DIR)
	touch $@

$(PKG_BUILD_DIR)/.configured:
	cd $(PKG_BUILD_DIR) && aclocal
	cd $(PKG_BUILD_DIR) && if grep ^AC_PROG_LIBTOOL configure.ac; then libtoolize; fi
	cd $(PKG_BUILD_DIR) && mkdir -p config.aux
	cd $(PKG_BUILD_DIR) && autoconf
	cd $(PKG_BUILD_DIR) && autoheader
	cd $(PKG_BUILD_DIR) && automake -a || true
	cd $(PKG_BUILD_DIR) && automake --foreign -a
	cd $(PKG_BUILD_DIR) && rm -rf config.cache
	cd $(PKG_BUILD_DIR) && \
		$(TARGET_CONFIGURE_OPTS) \
		./configure -C	\
		--target=$(GNU_TARGET_NAME) \
		--host=$(GNU_TARGET_NAME) \
		--build=$(GNU_HOST_NAME) \
		--program-prefix="" \
		--program-suffix="" \
		--prefix=/usr \
		--exec-prefix=/usr \
		--bindir=/usr/bin \
		--datadir=/usr/share \
		--includedir=/usr/include \
		--infodir=/usr/share/info \
		--libdir=/usr/lib \
		--libexecdir=/usr/lib \
		--localstatedir=/var \
		--mandir=/usr/share/man \
		--sbindir=/usr/sbin \
		--sysconfdir=/etc \
		$(DISABLE_LARGEFILE) \
		--enable-daemon
	touch $@

$(PKG_BUILD_DIR)/.built:
	$(MAKE) -C $(PKG_BUILD_DIR)
	touch $@

$(IPKG_SFTPSERVER):
	mkdir -p $(dir $(IDIR_SFTPSERVER)$(DROPBEAR_SFTPSERVER_PATH))
	install -m 755 $(PKG_BUILD_DIR)/gesftpserver \
		$(IDIR_SFTPSERVER)$(DROPBEAR_SFTPSERVER_PATH)
	$(STRIP) $(IDIR_SFTPSERVER)$(DROPBEAR_SFTPSERVER_PATH)
	$(IPKG_BUILD) $(IDIR_SFTPSERVER) $(PACKAGE_DIR)
