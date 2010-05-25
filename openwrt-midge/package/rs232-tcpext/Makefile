include $(TOPDIR)/rules.mk

PKG_NAME = rs232-tcpext
PKG_VERSION = 1
PKG_RELEASE = 1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)
#PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/ipkg-install

include $(TOPDIR)/package/rules.mk

$(eval $(call PKG_template,RS232_TCPEXT,rs232-tcpext,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))

$(PKG_BUILD_DIR)/.prepared :
	mkdir -p      $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)
	touch $@

cflags = "$(TARGET_CFLAGS) -I $(STAGING_DIR)/usr/include"
ldflags = "-L $(STAGING_DIR)/usr/lib"

$(PKG_BUILD_DIR)/.configured :
	@echo " CFLAGS =  $(cflags)"	 > $(PKG_BUILD_DIR)/local.mk
	@echo "LDFLAGS = $(ldflags)"	>> $(PKG_BUILD_DIR)/local.mk
	@for i in $(TARGET_CONFIGURE_OPTS); do \
		echo "$$i"		>> $(PKG_BUILD_DIR)/local.mk; \
	done
	touch $@

$(PKG_BUILD_DIR)/.built : $(PKG_BUILD_DIR)/.configured
	$(MAKE) -C $(PKG_BUILD_DIR)
	touch $@

instx = install -m0755
$(IPKG_RS232_TCPEXT) :
	@mkdir -p                               $(IDIR_RS232_TCPEXT)/usr/sbin
	@$(instx) $(PKG_BUILD_DIR)/rs232-tcpext $(IDIR_RS232_TCPEXT)/usr/sbin
	@mkdir -p                               $(IDIR_RS232_TCPEXT)/etc/init.d
	@$(instx)     ./etc/init.d/rs232_tcpext $(IDIR_RS232_TCPEXT)/etc/init.d/
	@ln -sf                    rs232_tcpext $(IDIR_RS232_TCPEXT)/etc/init.d/S70rs232_tcpext
	@ln -sf                    rs232_tcpext $(IDIR_RS232_TCPEXT)/etc/init.d/K70rs232_tcpext
	@$(RSTRIP)                              $(IDIR_RS232_TCPEXT)
	@$(IPKG_BUILD)                          $(IDIR_RS232_TCPEXT) $(PACKAGE_DIR)

