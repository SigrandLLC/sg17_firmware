# $Id:  $

include $(TOPDIR)/rules.mk

MS17E_MODNAME=$(BR2_SIGRAND_OEM_MS17E)
PKG_NAME:=dslam-ethernet
PKG_VERSION=1.0
PKG_RELEASE:=1
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)
CFLAGS:=
DEBUG := no

include $(TOPDIR)/package/rules.mk

$(eval $(call PKG_template,SIGRAND_MS17E,dslam-ethernet,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))

$(PKG_BUILD_DIR)/.prepared:
	@echo "PKG_DIR="$(PKG_BUILD_DIR)
	mkdir -p $(PKG_BUILD_DIR)
	cp -R ms17e-drv $(PKG_BUILD_DIR)
	touch $@

$(PKG_BUILD_DIR)/.configured:
	rm --preserve-root -f $(PKG_BUILD_DIR)/ms17e-drv/Makefile
	@echo "CFLAGS=$(CFLAGS)"
	sed <$(PKG_BUILD_DIR)/ms17e-drv/Makefile.tmpl \
	> $(PKG_BUILD_DIR)/ms17e-drv/Makefile \
	-e "s%@KERNSRC@%$(LINUX_DIR)%g" \
	-e "s%@CROSS_PREFIX@%$(KERNEL_CROSS)%g" \
	-e "s%@CFLAGS@%$(CFLAGS)%g" \
	-e "s%@KERNVER@%2.6.16%g" \
	-e "s%@INST_PATH@%$(IDIR_SIGRAND_DSLAM)%g" \
	-e "s%@MS17E_MODNAME@%$(MS17E_MODNAME)%g" \
	-e "s%@DEBUG@%$(DEBUG)%g"

	sed <$(PKG_BUILD_DIR)/ms17e-drv/ms17e_main.h.tmpl \
	> $(PKG_BUILD_DIR)/ms17e-drv/ms17e_main.h \
	-e "s%@MS17E_MODNAME@%$(MS17E_MODNAME)%g"

$(PKG_BUILD_DIR)/.built: $(PKG_BUILD_DIR)/.configured
	cd $(PKG_BUILD_DIR)/ms17e-drv && make
	touch $@

$(IPKG_SIGRAND_MS17E):
	install -d -m0755 $(IDIR_SIGRAND_MS17E)/lib/modules/2.6.16
	install -m0755 $(PKG_BUILD_DIR)/ms17e-drv/$(MS17E_MODNAME).ko $(IDIR_SIGRAND_MS17E)/lib/modules/2.6.16/

	install -d -m0755 $(IDIR_SIGRAND_MS17E)/usr/sbin
	install -m0755 $(PKG_BUILD_DIR)/ms17e-drv/$(MS17E_MODNAME)_debug $(IDIR_SIGRAND_MS17E)/usr/sbin
	install -m0755 $(PKG_BUILD_DIR)/ms17e-drv/poe_debug $(IDIR_SIGRAND_MS17E)/usr/sbin

	$(IPKG_BUILD) $(IDIR_SIGRAND_MS17E) $(PACKAGE_DIR)
