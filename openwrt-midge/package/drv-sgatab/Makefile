include $(TOPDIR)/rules.mk

PKG_NAME        = drv-sgatab
PKG_VERSION     = 1
PKG_RELEASE     = 1

        PKG_DIR = drv_sgatab
   PKG_CONF_DIR = $(TOPDIR)/package/$(PKG_NAME)
    PKG_SRC_DIR = $(PKG_CONF_DIR)/$(PKG_DIR)
  PKG_BUILD_DIR =    $(BUILD_DIR)/$(PKG_DIR)
PKG_INSTALL_DIR = $(PKG_BUILD_DIR)/install

include $(TOPDIR)/package/target-configure-staging-flags.mk

include $(TOPDIR)/package/rules.mk


$(eval $(call PKG_template,DRV_SGATAB,drv-sgatab,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))


$(PKG_BUILD_DIR)/.prepared:
	rm -rf               $(PKG_BUILD_DIR)
	cp -a $(PKG_SRC_DIR) $(PKG_BUILD_DIR)
	ln -snf $(BUILD_DIR)/$(PKG_DIR) $(BUILD_DIR)/sgatab
	mkdir -p $(PKG_INSTALL_DIR)/bin
	touch $@

$(PKG_BUILD_DIR)/.configured:  $(PKG_BUILD_DIR)/.prepared
	touch $@

$(PKG_BUILD_DIR)/.built:  $(PKG_BUILD_DIR)/.configured
	@echo
	@echo "BUILD AND INSTALL SGATAB..."
	cd $(PKG_BUILD_DIR) && $(TARGET_CONFIGURE_OPTS) \
		CC="$(TARGET_CC)" make  -w -C $(BUILD_DIR)/linux \
		M=$(PKG_BUILD_DIR) modules
	$(CP) $(PKG_BUILD_DIR)/drv_sgatab.ko $(PKG_INSTALL_DIR)/bin/
	touch $@

