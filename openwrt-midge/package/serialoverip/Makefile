include $(TOPDIR)/rules.mk

PKG_NAME:=serialoverip
PKG_VERSION:=1.0
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME).tgz
PKG_SOURCE_URL:=@SF/serialoverip/serialoverip
PKG_MD5SUM:=0931cca65b8f9d32079fa3e55130ee1e
PKG_CAT:=zcat


PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/ipkg-install

include $(TOPDIR)/package/rules.mk

$(eval $(call PKG_template,SERIALOVERIP,serialoverip,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))

$(PKG_BUILD_DIR)/.configured: $(PKG_BUILD_DIR)/.prepared
	rm -f $(PKG_BUILD_DIR)/serialoverip
	touch $@

$(PKG_BUILD_DIR)/.built:
	$(TARGET_CC) $(TARGET_CFLAGS) \
		-o $(PKG_BUILD_DIR)/serialoverip $(PKG_BUILD_DIR)/serialoverip.c
	touch $@

$(IPKG_SERIALOVERIP):
	install -d -m0755 $(IDIR_SERIALOVERIP)/usr/sbin
	install -m0755 $(PKG_BUILD_DIR)/serialoverip $(IDIR_SERIALOVERIP)/usr/sbin/
	$(RSTRIP) $(IDIR_SERIALOVERIP)
	$(IPKG_BUILD) $(IDIR_SERIALOVERIP) $(PACKAGE_DIR)

