include $(TOPDIR)/rules.mk

PKG_NAME        = drv-tapi
PKG_VERSION     = 3.6.1
PKG_RELEASE     = 1

        PKG_DIR = drv_tapi-$(PKG_VERSION)
   PKG_CONF_DIR = $(TOPDIR)/package/$(PKG_NAME)
    PKG_SRC_DIR = $(PKG_CONF_DIR)/$(PKG_DIR)
  PKG_BUILD_DIR =    $(BUILD_DIR)/$(PKG_DIR)
PKG_INSTALL_DIR = $(PKG_BUILD_DIR)/install

include $(TOPDIR)/package/target-configure-staging-flags.mk

include $(TOPDIR)/package/rules.mk


$(eval $(call PKG_template,DRV_TAPI,drv-tapi,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))


$(PKG_BUILD_DIR)/.prepared:
	rm -rf               $(PKG_BUILD_DIR)
	cp -a $(PKG_SRC_DIR) $(PKG_BUILD_DIR)
	ln -snf $(BUILD_DIR)/$(PKG_DIR) $(BUILD_DIR)/tapi
	patch -d $(PKG_BUILD_DIR) -p1 < $(PKG_CONF_DIR)/tapi.patch
	mkdir -p $(PKG_INSTALL_DIR)
	touch $@

$(PKG_BUILD_DIR)/.configured:  $(PKG_BUILD_DIR)/.prepared
	@echo
	@echo "CONFIGURE TAPI..."
	cd $(PKG_BUILD_DIR) && aclocal
	cd $(PKG_BUILD_DIR) && autoheader
	cd $(PKG_BUILD_DIR) && automake -a --foreign
	cd $(PKG_BUILD_DIR) && autoconf
	cd $(PKG_BUILD_DIR) && \
		$(TARGET_CONFIGURE_OPTS) \
		./configure \
		--host=$(GNU_TARGET_NAME) \
		--build=$(GNU_HOST_NAME) \
		--enable-linux-26 \
		--enable-trace \
		--enable-debug \
		--enable-voice \
		--enable-dtmf \
		--enable-cid \
		--enable-lt \
		--enable-dect \
		--enable-fax \
		--enable-extkeypad \
		--enable-kernelincl=$(BUILD_DIR)/linux/include \
		--prefix=$(PKG_INSTALL_DIR)
	touch $@

$(PKG_BUILD_DIR)/.built:  $(PKG_BUILD_DIR)/.configured
	@echo
	@echo "BUILD AND INSTALL TAPI..."
	cd $(PKG_BUILD_DIR) && $(TARGET_CONFIGURE_OPTS) make
	cd $(PKG_BUILD_DIR) && $(TARGET_CONFIGURE_OPTS) make install
	touch $@

