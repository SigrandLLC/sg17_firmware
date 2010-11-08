include $(TOPDIR)/rules.mk

PKG_NAME    = kdb
PKG_VERSION = 1
PKG_RELEASE = 1


PKG_BUILD_DIR = $(BUILD_DIR)/$(PKG_NAME)

include $(TOPDIR)/package/rules.mk

$(eval $(call PKG_template,KDB,kdb,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))
BR2_PACKAGE_KDB = y

$(PKG_BUILD_DIR)/.prepared:
	mkdir -p $(PKG_BUILD_DIR)
	touch $@

$(PKG_BUILD_DIR)/.built:
	$(TARGET_CC) $(TARGET_CFLAGS) -o $(PKG_BUILD_DIR)/kdb kdb.c md5.c
	touch $@


$(IPKG_KDB):
	cd files && cp -a .  $(IDIR_KDB)
	install -m0755 -d    $(IDIR_KDB)/usr/bin
	install -m0755 $(PKG_BUILD_DIR)/kdb $(IDIR_KDB)/usr/bin
ifeq ($(BR2_SIGRAND_OEM_MANUFACTURER),"Zelax")
	cp kdb.default.zelax $(IDIR_KDB)/etc/kdb.default
endif

	sed -i -e "s/@OEM_HOSTNAME@/$(BR2_SIGRAND_OEM_HOSTNAME)/g" \
		$(IDIR_KDB)/etc/kdb.default

	sed -i -e "s/@OEM_ETH0_IP_ADDR@/$(BR2_SIGRAND_OEM_ETH0_IP_ADDR)/g" \
			$(IDIR_KDB)/etc/kdb.default
	sed -i -e "s/@OEM_ETH0_IP_NETMASK@/$(BR2_SIGRAND_OEM_ETH0_IP_NETMASK)/g" \
			$(IDIR_KDB)/etc/kdb.default

	sed -i -e "s/@OEM_ETH1_IP_ADDR@/$(BR2_SIGRAND_OEM_ETH1_IP_ADDR)/g" \
			$(IDIR_KDB)/etc/kdb.default
	sed -i -e "s/@OEM_ETH1_IP_NETMASK@/$(BR2_SIGRAND_OEM_ETH1_IP_NETMASK)/g" \
			$(IDIR_KDB)/etc/kdb.default

	sed -i -e "s/@OEM_ETH2_IP_ADDR@/$(BR2_SIGRAND_OEM_ETH2_IP_ADDR)/g" \
			$(IDIR_KDB)/etc/kdb.default
	sed -i -e "s/@OEM_ETH2_IP_NETMASK@/$(BR2_SIGRAND_OEM_ETH2_IP_NETMASK)/g" \
			$(IDIR_KDB)/etc/kdb.default

	sed -i -e "s/@OEM_ETH3_IP_ADDR@/$(BR2_SIGRAND_OEM_ETH3_IP_ADDR)/g" \
			$(IDIR_KDB)/etc/kdb.default
	sed -i -e "s/@OEM_ETH3_IP_NETMASK@/$(BR2_SIGRAND_OEM_ETH3_IP_NETMASK)/g" \
			$(IDIR_KDB)/etc/kdb.default

	$(RSTRIP) $(IDIR_KDB)/usr/bin/*
	$(IPKG_BUILD) $(IDIR_KDB) $(PACKAGE_DIR)
