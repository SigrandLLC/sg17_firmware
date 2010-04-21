include $(TOPDIR)/rules.mk

PKG_NAME = rs232-ip-extender
PKG_RELEASE = 1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

include $(TOPDIR)/package/rules.mk

$(eval $(call PKG_template,RS232_IP_EXTENDER,rs232-ip-extender,$(PKG_RELEASE),$(ARCH)))

$(PKG_BUILD_DIR)/.prepared:
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)
	touch $@

cflags = "$(TARGET_CFLAGS) -I $(STAGING_DIR)/usr/include"

$(PKG_BUILD_DIR)/.configured:
	@echo "CFLAGS = $(cflags)"	 > $(PKG_BUILD_DIR)/local.mk
	@for i in $(TARGET_CONFIGURE_OPTS); do \
		echo "$$i"		>> $(PKG_BUILD_DIR)/local.mk; \
	done
	touch $@

$(PKG_BUILD_DIR)/.built:
	$(MAKE) -C $(PKG_BUILD_DIR)
	touch $@

$(IPKG_RS232_IP_EXTENDER):
	@#mkdir -p $(IDIR_RS232_IP_EXTENDER)/usr/bin
	@#install -m0755 $(PKG_BUILD_DIR)/rs232-ip-extender $(IDIR_RS232_IP_EXTENDER)/usr/bin
	@#$(RSTRIP) $(IDIR_RS232_IP_EXTENDER)
	@#$(IPKG_BUILD) $(IDIR_RS232_IP_EXTENDER) $(PACKAGE_DIR)
