include $(TOPDIR)/rules.mk

PKG_NAME    = hdb
PKG_VERSION = 1
PKG_RELEASE = 1


PKG_BUILD_DIR = $(BUILD_DIR)/$(PKG_NAME)

include $(TOPDIR)/package/rules.mk

$(eval $(call PKG_template,HDB,hdb,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))
BR2_PACKAGE_HDB = y

$(PKG_BUILD_DIR)/.prepared:
	mkdir -p $(PKG_BUILD_DIR)
	touch $@

$(PKG_BUILD_DIR)/.built:
	$(TARGET_CC) $(TARGET_CFLAGS) -o $(PKG_BUILD_DIR)/hdb -luClibc++ hdb.cpp
	touch $@

$(IPKG_HDB):
	install -m0755 -d $(IDIR_HDB)/usr/bin
	install -m0755 $(PKG_BUILD_DIR)/hdb $(IDIR_HDB)/usr/bin
	$(RSTRIP)         $(IDIR_HDB)/usr/bin/*
	$(IPKG_BUILD) $(IDIR_HDB) $(PACKAGE_DIR)

