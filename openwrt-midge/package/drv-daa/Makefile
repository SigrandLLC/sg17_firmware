include $(TOPDIR)/rules.mk

PKG_NAME        = drv-daa
PKG_VERSION     = 1.0.2.0
PKG_RELEASE     = 1

        PKG_DIR = drv_daa-$(PKG_VERSION)
   PKG_CONF_DIR = $(TOPDIR)/package/$(PKG_NAME)
    PKG_SRC_DIR = $(PKG_CONF_DIR)/$(PKG_DIR)
  PKG_BUILD_DIR =    $(BUILD_DIR)/$(PKG_DIR)
PKG_INSTALL_DIR = $(PKG_BUILD_DIR)/install

include $(TOPDIR)/package/target-configure-staging-flags.mk

include $(TOPDIR)/package/rules.mk


$(eval $(call PKG_template,DRV_DAA,drv-daa,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))


$(PKG_BUILD_DIR)/.prepared:
	rm -rf               $(PKG_BUILD_DIR)
	cp -a $(PKG_SRC_DIR) $(PKG_BUILD_DIR)
	ln -snf $(BUILD_DIR)/$(PKG_DIR) $(BUILD_DIR)/daa
	patch -d $(PKG_BUILD_DIR) -p1 < $(PKG_CONF_DIR)/daa.patch
	mkdir -p $(PKG_INSTALL_DIR)
	touch $@

$(PKG_BUILD_DIR)/.configured:  $(PKG_BUILD_DIR)/.prepared
	@echo
	@echo "CONFIGURE DAA..."
	cd $(PKG_BUILD_DIR) && aclocal
	cd $(PKG_BUILD_DIR) && autoheader
	cd $(PKG_BUILD_DIR) && automake -a --foreign
	cd $(PKG_BUILD_DIR) && autoconf
	cd $(PKG_BUILD_DIR) && \
		$(TARGET_CONFIGURE_OPTS) \
		./configure \
		--host=$(GNU_TARGET_NAME) \
		--build=$(GNU_HOST_NAME) \
		--disable-apoh \
		--disable-duslicincl \
		--disable-dxtincl \
		--enable-kernelincl=$(BUILD_DIR)/linux/include \
		--enable-tapiincl=$(BUILD_DIR)/tapi/include \
		--enable-vineticincl=$(BUILD_DIR)/vinetic/include \
		--enable-boardname=SG \
		--prefix=$(PKG_INSTALL_DIR)
	touch $@

$(PKG_BUILD_DIR)/.built:  $(PKG_BUILD_DIR)/.configured
	@echo
	@echo "BUILD AND INSTALL DAA..."
	cd $(PKG_BUILD_DIR) && $(TARGET_CONFIGURE_OPTS) make
	cd $(PKG_BUILD_DIR) && $(TARGET_CONFIGURE_OPTS) make install
	touch $@

$(IPKG_DRV_DAA):
	install -d -m0755 $(IDIR_DRV_DAA)/lib/modules/2.6.16/
	cp -rpf $(PKG_INSTALL_DIR)/bin/*.ko $(IDIR_DRV_DAA)/lib/modules/2.6.16/
	echo "Depends: $(PKG_DEPEND)" >> $(IDIR_DRV_DAA)/CONTROL/control
	$(IPKG_BUILD) $(IDIR_DRV_DAA) $(PACKAGE_DIR)

