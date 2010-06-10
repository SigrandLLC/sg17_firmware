include $(TOPDIR)/rules.mk

PKG_NAME:=libconfig
 version = 1.3.2
PKG_VERSION:=$(version)
PKG_RELEASE:=2
PKG_MD5SUM:=094a82afd382aa2305c6cc3c06025c2d

PKG_SOURCE_URL := http://debian.nsu.ru/debian/pool/main/libc/$(PKG_NAME)
PKG_SOURCE_URL += http://ftp.debian.org/debian/pool/main/libc/$(PKG_NAME)
PKG_SOURCE:=$(PKG_NAME)_$(PKG_VERSION).orig.tar.gz
PKG_CAT:=zcat

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/ipkg-install

include $(TOPDIR)/package/target-configure-staging-flags.mk

include $(TOPDIR)/package/rules.mk

$(eval $(call PKG_template,LIBCONFIG,libconfig,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))


$(PKG_BUILD_DIR)/.configured:
	rm -rf $(PKG_BUILD_DIR)/config.{cache,status}
	cd $(PKG_BUILD_DIR) &&			\
		$(TARGET_CONFIGURE_OPTS)	\
		./configure			\
			--target=$(GNU_TARGET_NAME)	\
			--host=$(GNU_TARGET_NAME)	\
			--build=$(GNU_HOST_NAME)	\
			--prefix=/usr		\
			$(DISABLE_LARGEFILE)	\
			$(DISABLE_NLS)		\
			--disable-shared	\
			--enable-static		\
			--with-gnu-ld		\
			--disable-cxx
	touch $@


$(PKG_BUILD_DIR)/.built:
	$(MAKE) -C $(PKG_BUILD_DIR) install DESTDIR=$(PKG_INSTALL_DIR)
	touch $@

$(PKG_BUILD_DIR)/.install-dev : $(PKG_BUILD_DIR)/.built
	mkdir -p $(STAGING_DIR)/usr/include
	$(CP) -a $(PKG_INSTALL_DIR)/usr/include/*	\
		 $(STAGING_DIR)/usr/include/
	mkdir -p $(STAGING_DIR)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libconfig.a	\
		 $(STAGING_DIR)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/libconfig.pc	\
		 $(STAGING_DIR)/usr/lib/pkgconfig/
	touch $@

install-dev: $(PKG_BUILD_DIR)/.install-dev

uninstall-dev:
	rm -rf	$(STAGING_DIR)/usr/lib/libconfig.a \
		$(STAGING_DIR)/usr/lib/pkgconfig/libconfig.pc \
		$(STAGING_DIR)/usr/include/libconfig.h

compile-targets:   install-dev
  clean-targets: uninstall-dev
