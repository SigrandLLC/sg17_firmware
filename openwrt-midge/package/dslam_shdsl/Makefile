# $Id:  $

include $(TOPDIR)/rules.mk

PKG_NAME:=dslam_shdl
PKG_VERSION=0.1
PKG_RELEASE:=0.1
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)
CFLAGS:=
DEBUG := no
ifeq ($(BR2_PACKAGE_SIGRAND_MAM17H_DEBUG),y)
#    CFLAGS +=-DDEBUG_SYSFS -DDEBUG_ON
    DEBUG := yes
endif

include $(TOPDIR)/package/rules.mk

$(eval $(call PKG_template,SIGRAND_MAM17H,dslam_shdsl,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))

$(PKG_BUILD_DIR)/.prepared:
	@echo "PKG_DIR="$(PKG_BUILD_DIR)
	mkdir -p $(PKG_BUILD_DIR)
	cp -R mam17h-drv $(PKG_BUILD_DIR)
	touch $@

$(PKG_BUILD_DIR)/.configured:
	touch $@

$(PKG_BUILD_DIR)/.built: $(PKG_BUILD_DIR)/.configured
	cd $(PKG_BUILD_DIR)/mam17h-drv && make
	touch $@

$(IPKG_SIGRAND_MAM17H):
	install -d -m0755 $(IDIR_SIGRAND_MAM17H)/sbin
	install -d -m0755 $(IDIR_SIGRAND_MAM17H)/lib/modules/2.6.16
	install -d -m0755 $(IDIR_SIGRAND_MAM17H)/lib/firmware
#	cd $(PKG_BUILD_DIR)/mam17h-drv && make install
#	$(IPKG_BUILD) $(IDIR_SIGRAND_MAM17H) $(PACKAGE_DIR)

local_build: $(PKG_BUILD_DIR)/.prepared
	cd $(PKG_BUILD_DIR)/mam17h-drv && make KERN_PATH=$(LINUX_DIR) CROSS_PREFIX=$(STAGING_DIR)/bin/mipsel-linux-
