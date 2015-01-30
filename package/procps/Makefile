include $(TOPDIR)/rules.mk

PKG_NAME:=procps
PKG_VERSION:=3.2.7
PKG_RELEASE:=11
#PKG_VERSION:=3.2.8
#PKG_RELEASE:=8
PKG_MD5SUM:=

PKG_SOURCE_URL := http://debian.nsu.ru/debian/pool/main/p/procps/
PKG_SOURCE_URL += http://ftp.debian.org/debian/pool/main/p/procps/
PKG_SOURCE:=$(PKG_NAME)_$(PKG_VERSION).orig.tar.gz
PKG_CAT:=zcat

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/ipkg-install

include $(TOPDIR)/package/rules.mk

$(eval $(call PKG_template,PROCPS,procps,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))

$(PKG_BUILD_DIR)/.configured:
	touch $@

$(PKG_BUILD_DIR)/.built:
	$(MAKE) -C $(PKG_BUILD_DIR)				\
		CC="$(TARGET_CC)" CFLAGS="$(TARGET_CFLAGS)"	\
		SHARED=0 lib64=lib CURSES=			\
		proc/libproc.a ps/ps
	touch $@

$(IPKG_PROCPS):
	install -d -m0755 $(IDIR_PROCPS)/usr/bin
	$(CP) $(PKG_BUILD_DIR)/ps/ps $(IDIR_PROCPS)/usr/bin/
	$(RSTRIP) $(IDIR_PROCPS)/
	$(IPKG_BUILD) $(IDIR_PROCPS) $(PACKAGE_DIR)
