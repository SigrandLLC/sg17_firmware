include $(TOPDIR)/rules.mk

PKG_NAME:=sercd
PKG_VERSION:=3.0.0
PKG_RELEASE:=1
PKG_MD5SUM:=3ad41d9e8fa28e12f96652fdd4db30b0

PKG_SOURCE_URL:=@SF/sercd/
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_CAT:=zcat

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/ipkg-install

include $(TOPDIR)/package/rules.mk

$(eval $(call PKG_template,SERCD,sercd,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))

$(PKG_BUILD_DIR)/.configured: $(PKG_BUILD_DIR)/.prepared
	cd $(PKG_BUILD_DIR) &&		\
	$(TARGET_CONFIGURE_OPTS)	\
	CFLAGS="$(TARGET_CFLAGS) -DNO_TELNET"	\
	./configure			\
	--target=$(GNU_TARGET_NAME)	\
	--host=$(GNU_TARGET_NAME)	\
	--build=$(GNU_HOST_NAME)	\
	--prefix=/usr			\
	--sysconfdir=/etc
	touch $@

$(PKG_BUILD_DIR)/.built:
	rm -rf $(PKG_INSTALL_DIR)
	mkdir -p $(PKG_INSTALL_DIR)
	$(MAKE) -C $(PKG_BUILD_DIR) \
	$(TARGET_CONFIGURE_OPTS) \
	DESTDIR="$(PKG_INSTALL_DIR)" \
	all install
	touch $@

$(IPKG_SERCD):
	install -d -m0755 $(IDIR_SERCD)/usr/sbin
	cp -fpR $(PKG_INSTALL_DIR)/usr/sbin/sercd $(IDIR_SERCD)/usr/sbin
	#+ for test only
	#install -d -m0755 $(IDIR_SERCD)/etc
	##cp -fpR $(PKG_BUILD_DIR)/sercd.conf     $(IDIR_SERCD)/etc
	#install -d -m0755                       $(IDIR_SERCD)/etc/init.d
	#install -m0755 ./files/etc/init.d/sercd $(IDIR_SERCD)/etc/init.d/sercd
	#install -m0755 ./files/sercat           $(IDIR_SERCD)/usr/sbin/sercat
	#- for test only
	$(RSTRIP) $(IDIR_SERCD)
	$(IPKG_BUILD) $(IDIR_SERCD) $(PACKAGE_DIR)

