# $Id: Makefile 3112 2006-02-01 23:53:19Z mbm $

include $(TOPDIR)/rules.mk

PKG_NAME:=socat

PKG_VERSION:=1.7.1.2
PKG_RELEASE:=3
PKG_MD5SUM:=2a5b7b0355421a039fb68b7541181b38

PKG_SOURCE_URL := http://debian.nsu.ru/debian/pool/main/s/socat/
PKG_SOURCE_URL += http://ftp.debian.org/debian/pool/main/s/socat/
PKG_SOURCE:=$(PKG_NAME)_$(PKG_VERSION).orig.tar.gz
PKG_CAT:=zcat

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/ipkg-install

include $(TOPDIR)/package/rules.mk

$(eval $(call PKG_template,SOCAT,socat,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))

ifeq "$(GNU_TARGET_NAME)" "mipsel-linux"
   shifts =	sc_cv_sys_crdly_shift=9 \
		sc_cv_sys_tabdly_shift=11 \
		sc_cv_sys_csize_shift=4
endif

$(PKG_BUILD_DIR)/.configured:
	(cd $(PKG_BUILD_DIR); rm -rf config.{cache,status}; \
		$(TARGET_CONFIGURE_OPTS) \
		CFLAGS="$(TARGET_CFLAGS)" \
		CPPFLAGS="-I$(STAGING_DIR)/usr/include -I$(STAGING_DIR)/include" \
		LDFLAGS="-L$(STAGING_DIR)/usr/lib -L$(STAGING_DIR)/lib" \
		ac_cv_func_memcmp_working=yes \
		sc_cv_termios_ispeed=no \
		$(shifts) \
		./configure \
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
			$(DISABLE_NLS) \
			--disable-libwrap \
			--disable-readline \
			--disable-ext2 \
			--disable-filan \
	)
	touch $@

$(PKG_BUILD_DIR)/.built:
	rm -rf $(PKG_INSTALL_DIR)
	mkdir -p $(PKG_INSTALL_DIR)
	$(MAKE) -C $(PKG_BUILD_DIR) \
		DESTDIR="$(PKG_INSTALL_DIR)" \
		all install
	touch $@

$(IPKG_SOCAT):
	install -d -m0755 $(IDIR_SOCAT)/usr/bin
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/socat $(IDIR_SOCAT)/usr/bin/
	$(RSTRIP) $(IDIR_SOCAT)
	$(IPKG_BUILD) $(IDIR_SOCAT) $(PACKAGE_DIR)

