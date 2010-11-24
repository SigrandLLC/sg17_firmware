# $Id: Makefile 3221 2006-02-12 06:00:38Z nbd $

include $(TOPDIR)/rules.mk

PKG_NAME := base-files-arch
PKG_RELEASE := 2
PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

include $(TOPDIR)/package/rules.mk

IDIR_OPENWRT:=$(PKG_BUILD_DIR)/ipkg
IPKG_OPENWRT:=$(PACKAGE_DIR)/base-files-$(BOARD)-$(KERNEL)_$(PKG_RELEASE)_$(ARCH).ipk

$(PKG_BUILD_DIR)/.prepared:
	mkdir -p $(PKG_BUILD_DIR)
	touch $@

$(PKG_BUILD_DIR)/.built:
	touch $@

$(IDIR_OPENWRT):
	$(SCRIPT_DIR)/make-ipkg-dir.sh $(IDIR_OPENWRT) ipkg/base-files-arch.control $(PKG_RELEASE) $(ARCH)
	$(SED) s,base-files-arch,base-files-$(BOARD)-$(KERNEL),g $(IDIR_OPENWRT)/CONTROL/control

$(IPKG_OPENWRT): $(IDIR_OPENWRT)
	[ -d files/$(BOARD)-$(KERNEL) ] && $(CP) -fpR files/$(BOARD)-$(KERNEL)/* $(IDIR_OPENWRT)/
	$(RSTRIP) $(IDIR_OPENWRT)
	$(IPKG_BUILD) $(IDIR_OPENWRT) $(PACKAGE_DIR)

install-targets: compile openwrt-install

openwrt-install:
	$(IPKG) install $(IPKG_OPENWRT)

compile-targets: install-dev $(IPKG_OPENWRT)
install-dev:
	mkdir -p $(STAGING_DIR)/usr/include
