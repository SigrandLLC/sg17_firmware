# $Id:  $

include $(TOPDIR)/rules.mk

PKG_NAME:=sigrand-mux
PKG_RELEASE:=1
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)
CFLAGS:=

include $(TOPDIR)/package/rules.mk

$(eval $(call PKG_template,SIGRAND_MR17H,sigrand-mr17h,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))

$(PKG_BUILD_DIR)/.prepared:
	@echo "PKG_DIR="$(PKG_BUILD_DIR)
	mkdir -p $(PKG_BUILD_DIR)
	cp -R mxconfig $(PKG_BUILD_DIR)
	touch $@

$(PKG_BUILD_DIR)/.configured:
	touch $@

$(PKG_BUILD_DIR)/.built: $(PKG_BUILD_DIR)/.configured
	cd $(PKG_BUILD_DIR)/mxconfig && make CC=$(TARGET_CC)
	touch $@

$(IPKG_SIGRAND_MR17H):
	install -d -m0755 $(IDIR_SIGRAND_MR17H)/sbin
	cp $(PKG_BUILD_DIR)/mxconfig/mxconfig $(IDIR_SIGRAND_MR17H)/sbin

