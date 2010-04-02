# $Id: Makefile 3365 2006-03-14 00:26:47Z nico $

include $(TOPDIR)/rules.mk

PKG_NAME:=pciutils
PKG_VERSION:=3.1.7
PKG_RELEASE:=2
PKG_MD5SUM:=35222edb86b50a6641352c66fe829b2e

PKG_SOURCE_URL := http://debian.nsu.ru/debian/pool/main/p/pciutils/
PKG_SOURCE_URL += http://ftp.debian.org/debian/pool/main/p/pciutils/
PKG_SOURCE:=$(PKG_NAME)_$(PKG_VERSION).orig.tar.bz2
PKG_CAT:=bzcat

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/ipkg-install

include $(TOPDIR)/package/rules.mk

$(eval $(call PKG_template,PCIUTILS,pciutils,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))

$(PKG_BUILD_DIR)/.configured:
	touch $@

$(PKG_BUILD_DIR)/.built:
	$(MAKE) -C $(PKG_BUILD_DIR) \
		CC=$(TARGET_CC) CFLAGS="$(TARGET_CFLAGS)" IDSDIR="/usr/share"	\
		HOST=$(GNU_TARGET_NAME) ZLIB=no DNS=no SHARED=no		\
		lspci setpci
	touch $@

$(IPKG_PCIUTILS):
	install -d -m0755 $(IDIR_PCIUTILS)/usr/sbin
	install -d -m0755 $(IDIR_PCIUTILS)/usr/share
	$(CP) $(PKG_BUILD_DIR)/{lspci,setpci} $(IDIR_PCIUTILS)/usr/sbin/
	$(CP) $(PKG_BUILD_DIR)/pci.ids $(IDIR_PCIUTILS)/usr/share/
	$(RSTRIP) $(IDIR_PCIUTILS)/
	$(IPKG_BUILD) $(IDIR_PCIUTILS) $(PACKAGE_DIR)
