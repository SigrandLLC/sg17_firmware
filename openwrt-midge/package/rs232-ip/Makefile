include $(TOPDIR)/rules.mk

PKG_NAME:=rs232-ip
PKG_VERSION:=1
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)
PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/ipkg-install

include $(TOPDIR)/package/rules.mk

$(eval $(call PKG_template,RS232_IP,rs232-ip,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))

$(PKG_BUILD_DIR)/.prepared:
	mkdir -p $(PKG_BUILD_DIR)
	touch $@

$(IPKG_RS232_IP):
	install -d -m0755 $(IDIR_RS232_IP)/etc/init.d
	install -m0755 ./files/etc/init.d/rs232ip $(IDIR_RS232_IP)/etc/init.d/
	$(IPKG_BUILD) $(IDIR_RS232_IP) $(PACKAGE_DIR)

