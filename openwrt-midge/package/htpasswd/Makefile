include $(TOPDIR)/rules.mk

PKG_NAME    = htpasswd
PKG_VERSION = 1
PKG_RELEASE = 1


PKG_BUILD_DIR = $(BUILD_DIR)/$(PKG_NAME)

include $(TOPDIR)/package/rules.mk

$(eval $(call PKG_template,HTPASSWD,htpasswd,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))
BR2_PACKAGE_HTPASSWD = y

$(PKG_BUILD_DIR)/.prepared:
	mkdir -p $(PKG_BUILD_DIR)
	touch $@

$(PKG_BUILD_DIR)/.built:
	$(TARGET_CC) -lcrypt $(TARGET_CFLAGS) -o $(PKG_BUILD_DIR)/htpasswd htpasswd.c
	touch $@

$(IPKG_HTPASSWD):
	install -m0755 -d  $(IDIR_HTPASSWD)/usr/bin
	install -m0755     $(PKG_BUILD_DIR)/htpasswd $(IDIR_HTPASSWD)/usr/bin
	$(RSTRIP)          $(IDIR_HTPASSWD)/usr/bin/*
	$(IPKG_BUILD)      $(IDIR_HTPASSWD) $(PACKAGE_DIR)
