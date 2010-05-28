include $(TOPDIR)/rules.mk

PKG_NAME:=lockdev
PKG_VERSION:=1.0.3
PKG_RELEASE:=1.4
PKG_MD5SUM:=64b9c1b87b125fc348e892e24625524a

PKG_SOURCE_URL := http://debian.nsu.ru/debian/pool/main/l/lockdev/
PKG_SOURCE_URL += http://ftp.debian.org/debian/pool/main/l/lockdev/
PKG_SOURCE:=$(PKG_NAME)_$(PKG_VERSION).orig.tar.gz
PKG_CAT:=zcat

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/ipkg-install

include $(TOPDIR)/package/rules.mk

$(eval $(call PKG_template,LIBLOCKDEV,liblockdev,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))

$(PKG_BUILD_DIR)/.configured:
	touch $@

$(PKG_BUILD_DIR)/.built:
	$(MAKE) -C $(PKG_BUILD_DIR)			\
		basedir=$(PKG_INSTALL_DIR)/usr		\
		CC="$(TARGET_CC)" AR="$(TARGET_CROSS)ar"	\
		CFLAGS="$(TARGET_CFLAGS)"		\
		static install_dev
	touch $@

$(IPKG_LIBLOCKDEV):
	install -d -m0755                  $(IDIR_LIBLOCKDEV)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/* $(IDIR_LIBLOCKDEV)/usr/lib
	$(RSTRIP)                          $(IDIR_LIBLOCKDEV)
	$(IPKG_BUILD)                      $(IDIR_LIBLOCKDEV) $(PACKAGE_DIR)


$(PKG_BUILD_DIR)/.install-dev : $(PKG_BUILD_DIR)/.built
	mkdir -p $(STAGING_DIR)/usr/include
	$(CP)	$(PKG_BUILD_DIR)/src/ttylock.h	\
		$(PKG_BUILD_DIR)/src/lockdev.h	\
						$(STAGING_DIR)/usr/include/
	mkdir -p $(STAGING_DIR)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/liblockdev.a	$(STAGING_DIR)/usr/lib/
	touch $@

install-dev: $(PKG_BUILD_DIR)/.install-dev

uninstall-dev:
	rm -f	\
		$(STAGING_DIR)/usr/include/ttylock.h	\
		$(STAGING_DIR)/usr/include/lockdev.h	\
		$(STAGING_DIR)/usr/lib/liblockdev.a

compile-targets:   install-dev
  clean-targets: uninstall-dev

