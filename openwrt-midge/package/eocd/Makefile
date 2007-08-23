# $Id:  $

include $(TOPDIR)/rules.mk

PKG_NAME:=eocd
PKG_RELEASE:=1
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)
CFLAGS:=
DEBUG := no

LIBCONF_VER=1.1.3

include $(TOPDIR)/package/rules.mk

$(eval $(call PKG_template,SIGRAND_EOCD,eocd,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))

$(PKG_BUILD_DIR)/.prepared:
	@echo "PKG_DIR="$(PKG_BUILD_DIR)" CURRENT="`pwd`
	mkdir -p $(PKG_BUILD_DIR)
	mkdir -p $(PKG_BUILD_DIR)/eocd-src
	tar -xjvf eocd.tar.bz2 -C $(PKG_BUILD_DIR)/eocd-src
	tar -xzvf libconfig-$(LIBCONF_VER).tar.gz -C $(PKG_BUILD_DIR)
	ln -s $(PKG_BUILD_DIR)/libconfig-$(LIBCONF_VER)  $(PKG_BUILD_DIR)/eocd-src/libconfig
	cp mips-compile.tmpl $(PKG_BUILD_DIR)/libconfig-$(LIBCONF_VER)
	touch $@

$(PKG_BUILD_DIR)/.configured:
	rm --preserve-root -f $(PKG_BUILD_DIR)/mr17h-drv/Makefile
	@echo "CFLAGS=$(CFLAGS)"
	sed <$(PKG_BUILD_DIR)/eocd-src/Makefile.tmpl \
	> $(PKG_BUILD_DIR)/eocd-src/Makefile \
	-e "s%@CROSS_PREFIX@%$(KERNEL_CROSS)%g"
	
	sed <$(PKG_BUILD_DIR)/libconfig-$(LIBCONF_VER)/mips-compile.tmpl \
	> $(PKG_BUILD_DIR)/libconfig-$(LIBCONF_VER)/mips-compile.sh \
	-e "s%@CROSS_PREFIX@%$(KERNEL_CROSS)%g"
	chmod +x $(PKG_BUILD_DIR)/libconfig-$(LIBCONF_VER)/mips-compile.sh
	touch $@

$(PKG_BUILD_DIR)/.built: $(PKG_BUILD_DIR)/.configured
	top=`pwd`
	cd $(PKG_BUILD_DIR)/libconfig-$(LIBCONF_VER)/ && ./mips-compile.sh
	cd $(PKG_BUILD_DIR)/eocd-src && make all-mips
	touch $@

$(IPKG_SIGRAND_EOCD):
	install -d -m0755 $(IDIR_SIGRAND_EOCD)/sbin
	install -d -m0755 $(IDIR_SIGRAND_EOCD)/etc/eocd
	install -d -m0755 $(IDIR_SIGRAND_EOCD)/lib
	cp $(PKG_BUILD_DIR)/eocd-src/eocd  $(IDIR_SIGRAND_EOCD)/sbin
	cp $(PKG_BUILD_DIR)/eocd-src/eocd.conf $(IDIR_SIGRAND_EOCD)/etc/eocd
	cp $(STAGING_DIR)/lib/libstdc++.so.6 $(IDIR_SIGRAND_EOCD)/lib
	cp $(PKG_BUILD_DIR)/eocd-src/app-if/applications/eoc-info  $(IDIR_SIGRAND_EOCD)/sbin
	echo "Depends: $(PKG_DEPEND)" >> $(IDIR_SIGRAND_EOCD)/CONTROL/control
	$(IPKG_BUILD) $(IDIR_SIGRAND_EOCD) $(PACKAGE_DIR)	
	@echo "INSTALLLLLLLLLLL"

local_build: $(PKG_BUILD_DIR)/.prepared
	cd $(PKG_BUILD_DIR)/mr17h-drv && make KERN_PATH=$(LINUX_DIR) CROSS_PREFIX=$(STAGING_DIR)/bin/mipsel-linux-
