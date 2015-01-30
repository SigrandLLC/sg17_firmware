include $(TOPDIR)/rules.mk

PKG_NAME:=statserial

PKG_VERSION:=1.1
PKG_RELEASE:=22
PKG_MD5SUM:=8b412d438026fdcc2848919b6b3fe322

PKG_SOURCE_URL := http://debian.nsu.ru/debian/pool/main/s/statserial
PKG_SOURCE_URL += http://ftp.debian.org/debian/pool/main/s/statserial
PKG_SOURCE:=$(PKG_NAME)_$(PKG_VERSION).orig.tar.gz
PKG_CAT:=zcat

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION).orig
PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/ipkg-install

include $(TOPDIR)/package/rules.mk

$(eval $(call PKG_template,STATSERIAL,statserial,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))

$(PKG_BUILD_DIR)/.configured:
	touch $@

$(PKG_BUILD_DIR)/.built:
	$(MAKE) -C $(PKG_BUILD_DIR) statserial \
		$(TARGET_CONFIGURE_OPTS) \
		CC="$(TARGET_CC)" LD="$(TARGET_CROSS)ld" \
		CFLAGS="$(TARGET_CFLAGS) -I$(STAGING_DIR)/usr/include -I$(STAGING_DIR)/include" \
		LDFLAGS="-L$(STAGING_DIR)/usr/lib -L$(STAGING_DIR)/lib"
	touch $@


$(IPKG_STATSERIAL):
	install -d -m0755 $(IDIR_STATSERIAL)/usr/bin
	$(CP) $(PKG_BUILD_DIR)/statserial $(IDIR_STATSERIAL)/usr/bin/
	$(RSTRIP)     $(IDIR_STATSERIAL)
	$(IPKG_BUILD) $(IDIR_STATSERIAL) $(PACKAGE_DIR)

