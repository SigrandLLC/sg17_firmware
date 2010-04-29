include $(TOPDIR)/rules.mk

PKG_NAME = rs232-tcpext
PKG_RELEASE = 1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

include $(TOPDIR)/package/rules.mk

$(eval $(call PKG_template,RS232_TCPEXT,rs232-tcpext,$(PKG_RELEASE),$(ARCH)))

$(PKG_BUILD_DIR)/.prepared:
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)
	touch $@

cflags = "$(TARGET_CFLAGS) -I $(STAGING_DIR)/usr/include"
ldflags = "-L $(STAGING_DIR)/usr/lib"

$(PKG_BUILD_DIR)/.configured:
	@echo " CFLAGS =  $(cflags)"	 > $(PKG_BUILD_DIR)/local.mk
	@echo "LDFLAGS = $(ldflags)"	>> $(PKG_BUILD_DIR)/local.mk
	@for i in $(TARGET_CONFIGURE_OPTS); do \
		echo "$$i"		>> $(PKG_BUILD_DIR)/local.mk; \
	done
	touch $@

$(PKG_BUILD_DIR)/.built:
	$(MAKE) -C $(PKG_BUILD_DIR)
	touch $@

$(IPKG_RS232_TCPEXT):
	@#mkdir -p $(IDIR_RS232_TCPEXT)/usr/bin
	@#install -m0755 $(PKG_BUILD_DIR)/rs232-tcpext $(IDIR_RS232_TCPEXT)/usr/bin
	@#$(RSTRIP) $(IDIR_RS232_TCPEXT)
	@#$(IPKG_BUILD) $(IDIR_RS232_TCPEXT) $(PACKAGE_DIR)
