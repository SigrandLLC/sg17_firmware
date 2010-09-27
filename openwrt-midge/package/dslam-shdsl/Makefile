# $Id:  $

include $(TOPDIR)/rules.mk

MAM17H_MODNAME="ms17h"
PKG_NAME:=dslam-shdl
PKG_VERSION=1.0
PKG_RELEASE:=1
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)
CFLAGS:=
DEBUG := no
ifeq ($(BR2_PACKAGE_SIGRAND_MAM17H_DEBUG),y)
#    CFLAGS +=-DDEBUG_SYSFS -DDEBUG_ON
    DEBUG := yes
endif

include $(TOPDIR)/package/rules.mk

$(eval $(call PKG_template,SIGRAND_MAM17H,dslam-shdsl,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))

$(PKG_BUILD_DIR)/.prepared:
	@echo "PKG_DIR="$(PKG_BUILD_DIR)
	mkdir -p $(PKG_BUILD_DIR)
	cp -R mam17h-drv $(PKG_BUILD_DIR)
	touch $@

$(PKG_BUILD_DIR)/.configured:
	rm --preserve-root -f $(PKG_BUILD_DIR)/mam17h-drv/Makefile
	@echo "CFLAGS=$(CFLAGS)"
	sed <$(PKG_BUILD_DIR)/mam17h-drv/Makefile.tmpl \
	> $(PKG_BUILD_DIR)/mam17h-drv/Makefile \
	-e "s%@KERNSRC@%$(LINUX_DIR)%g" \
	-e "s%@CROSS_PREFIX@%$(KERNEL_CROSS)%g" \
	-e "s%@CFLAGS@%$(CFLAGS)%g" \
	-e "s%@KERNVER@%2.6.16%g" \
	-e "s%@INST_PATH@%$(IDIR_SIGRAND_DSLAM)%g" \
	-e "s%@MAM17H_MODNAME@%$(MAM17H_MODNAME)%g" \
	-e "s%@DEBUG@%$(DEBUG)%g"

	sed <$(PKG_BUILD_DIR)/mam17h-drv/tools/mam17h_cfg.tmpl \
	> $(PKG_BUILD_DIR)/mam17h-drv/tools/mam17h_cfg \
	-e "s%@MAM17H_MODNAME@%$(MAM17H_MODNAME)%g"
	touch $@

$(PKG_BUILD_DIR)/.built: $(PKG_BUILD_DIR)/.configured
	cd $(PKG_BUILD_DIR)/mam17h-drv && make
	touch $@

$(IPKG_SIGRAND_MAM17H):
	install -d -m0755 $(IDIR_SIGRAND_MAM17H)/sbin
	mkdir -p $(PKG_BUILD_DIR)/mam17h-drv/tmp
	cp $(PKG_BUILD_DIR)/mam17h-drv/tools/mam17h_cfg $(PKG_BUILD_DIR)/mam17h-drv/tmp/$(MAM17H_MODNAME)_cfg
	install -m0755 $(PKG_BUILD_DIR)/mam17h-drv/tmp/$(MAM17H_MODNAME)_cfg $(IDIR_SIGRAND_MAM17H)/sbin/
	install -d -m0755 $(IDIR_SIGRAND_MAM17H)/lib/modules/2.6.16
	install -m0755 $(PKG_BUILD_DIR)/mam17h-drv/$(MAM17H_MODNAME).ko $(IDIR_SIGRAND_MAM17H)/lib/modules/2.6.16/
	install -d -m0755 $(IDIR_SIGRAND_MAM17H)/lib/firmware
	install -m0755 $(PKG_BUILD_DIR)/mam17h-drv/firmware/dfe.bin $(IDIR_SIGRAND_MAM17H)/lib/firmware/
	install -m0755 $(PKG_BUILD_DIR)/mam17h-drv/firmware/idc.bin $(IDIR_SIGRAND_MAM17H)/lib/firmware/
	$(IPKG_BUILD) $(IDIR_SIGRAND_MAM17H) $(PACKAGE_DIR)
