include $(TOPDIR)/rules.mk

PKG_NAME = rs232-mstate
PKG_VERSION = 1
PKG_RELEASE = 1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)
#PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/ipkg-install

include $(TOPDIR)/package/rules.mk

$(eval $(call PKG_template,RS232_MSTATE,rs232-mstate,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))

$(PKG_BUILD_DIR)/.prepared :
	mkdir -p      $(PKG_BUILD_DIR)
	$(CP) -L ./src/* $(PKG_BUILD_DIR)
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

$(PKG_BUILD_DIR)/.built :
	$(MAKE) -C $(PKG_BUILD_DIR)
	touch $@

instx = install -m0755
$(IPKG_RS232_MSTATE) :
	@mkdir -p                               $(IDIR_RS232_MSTATE)/usr/bin
	@$(instx) $(PKG_BUILD_DIR)/rs232-mstate $(IDIR_RS232_MSTATE)/usr/bin
	@$(RSTRIP)                              $(IDIR_RS232_MSTATE)
	@$(IPKG_BUILD)                          $(IDIR_RS232_MSTATE) $(PACKAGE_DIR)

