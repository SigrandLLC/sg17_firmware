include $(TOPDIR)/rules.mk

PKG_NAME        = drv-vinetic
PKG_VERSION     = 1.3.1
PKG_RELEASE     = tapi-3.6.1

PKG_DIR         = drv_vinetic-$(PKG_VERSION)_$(PKG_RELEASE)

   PKG_CONF_DIR = $(TOPDIR)/package/$(PKG_NAME)
    PKG_SRC_DIR = $(PKG_CONF_DIR)/$(PKG_DIR)
  PKG_BUILD_DIR =    $(BUILD_DIR)/$(PKG_DIR)
PKG_INSTALL_DIR = $(PKG_BUILD_DIR)/install
PKG_DEPEND      = drv-tapi

include $(TOPDIR)/package/target-configure-staging-flags.mk

include $(TOPDIR)/package/rules.mk



$(eval $(call PKG_template,DRV_VINETIC,drv-vinetic,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))


$(PKG_BUILD_DIR)/.prepared:
	rm -rf               $(PKG_BUILD_DIR)
	cp -a $(PKG_SRC_DIR) $(PKG_BUILD_DIR)
	ln -snf $(BUILD_DIR)/$(PKG_DIR) $(BUILD_DIR)/vinetic
	patch -d $(PKG_BUILD_DIR) -p1 < $(PKG_CONF_DIR)/vinetic.patch
	mkdir -p $(PKG_INSTALL_DIR)
	touch $@

$(PKG_BUILD_DIR)/.configured:  $(PKG_BUILD_DIR)/.prepared
	@echo
	@echo "CONFIGURE AND BUILD VINETIC..."
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
		--enable-2cpe \
		--enable-lt \
		--enable-fax \
		--disable-v1 \
		--with-max-devices=32 \
		--with-access-mode=INTEL_MUX \
		--with-access-width=8 \
		--enable-tapiincl=$(BUILD_DIR)/tapi/include \
		--enable-kernelincl=$(BUILD_DIR)/linux/include \
		--prefix=$(PKG_INSTALL_DIR)
	touch $@

$(PKG_BUILD_DIR)/.built:  $(PKG_BUILD_DIR)/.configured
	cd $(PKG_BUILD_DIR) && $(TARGET_CONFIGURE_OPTS) make
	cd $(PKG_BUILD_DIR) && $(TARGET_CONFIGURE_OPTS) make install
	touch $@

$(IPKG_DRV_VINETIC):
	install -d -m0755 $(IDIR_DRV_VINETIC)/lib/modules/2.6.16/
	cp -rpf $(PKG_INSTALL_DIR)/bin/*.ko $(IDIR_DRV_VINETIC)/lib/modules/2.6.16/
	echo "Depends: $(PKG_DEPEND)" >> $(IDIR_DRV_VINETIC)/CONTROL/control
	$(IPKG_BUILD) $(IDIR_DRV_VINETIC) $(PACKAGE_DIR)

