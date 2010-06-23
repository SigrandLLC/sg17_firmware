# $Id:  $

include $(TOPDIR)/rules.mk

PKG_NAME:=oem-pppft-mp
PKG_VERSION=1
PKG_RELEASE=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

include $(TOPDIR)/package/rules.mk

$(eval $(call PKG_template,SIGRAND_PPPFT_MP,oem-pppft-mp,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))

$(PKG_BUILD_DIR)/.prepared:
	mkdir -p $(PKG_BUILD_DIR)
	touch $@

$(PKG_BUILD_DIR)/.configured:
	touch $@

$(IPKG_SIGRAND_PPPFT_MP):
	tar cf - -C files --exclude .svn . | tar xvf - -C $(IDIR_SIGRAND_PPPFT_MP)
	$(IPKG_BUILD) $(IDIR_SIGRAND_PPPFT_MP) $(PACKAGE_DIR)
