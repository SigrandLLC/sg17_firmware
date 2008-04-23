# 
# Copyright (C) 2006 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
# $Id$

include $(TOPDIR)/rules.mk

PKG_NAME:=sigrand-voip
PKG_VERSION:=0.1
PKG_RELEASE:=1

PKG_TAPI_DISTR:=drv_tapi-3.6.1.tar.gz
PKG_DAA_DISTR:=drv_daa-1.0.2.0.tar.gz
PKG_VINETIC_DISTR:=drv_vinetic-1.3.1_tapi-3.6.1.tar.gz
PKG_SGATAB_DISTR:=drv_sgatab-1.0.1.tar.gz
PKG_SVD_DISTR:=svd-0.1.tar.gz
PKG_SVINIT_DISTR:=svinit-0.1.tar.gz

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/build
PKG_SRC_DIR:=$(PKG_CONF_DIR)/files/src/

include $(TOPDIR)/package/rules.mk

PKG_CONF_DIR:=$(TOPDIR)/package/$(PKG_NAME)

TAPI_UNPACK+=tar -vxpf $(PKG_SRC_DIR)/$(PKG_TAPI_DISTR) -C $(PKG_BUILD_DIR)
DAA_UNPACK+=tar -vxpf $(PKG_SRC_DIR)/$(PKG_DAA_DISTR) -C $(PKG_BUILD_DIR)
VINETIC_UNPACK+=tar -vxpf $(PKG_SRC_DIR)/$(PKG_VINETIC_DISTR) -C $(PKG_BUILD_DIR)
SGATAB_UNPACK+=tar -vxpf $(PKG_SRC_DIR)/$(PKG_SGATAB_DISTR) -C $(PKG_BUILD_DIR)
SVD_UNPACK+=tar -vxpf $(PKG_SRC_DIR)/$(PKG_SVD_DISTR) -C $(PKG_BUILD_DIR)
SVINIT_UNPACK+=tar -vxpf $(PKG_SRC_DIR)/$(PKG_SVINIT_DISTR) -C $(PKG_BUILD_DIR)


$(eval $(call PKG_template,SIGRAND_VOIP,sigrand-voip,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))



$(PKG_BUILD_DIR)/.prepared:
	mkdir -p $(PKG_BUILD_DIR);
	
	#---------======================================---------
	# 			unpack sources
	#---------======================================---------
	
	$(TAPI_UNPACK);
	$(DAA_UNPACK);
	$(VINETIC_UNPACK);
	$(SGATAB_UNPACK);
	$(SVD_UNPACK);
	$(SVINIT_UNPACK);
	
	#---------======================================---------
	# 			prepare sources
	#---------======================================---------
	
	mkdir -p $(PKG_INSTALL_DIR)
	cp $(PKG_CONF_DIR)/files/patches/* $(PKG_BUILD_DIR)
	cp $(PKG_CONF_DIR)/files/prepare_all $(PKG_BUILD_DIR)
	$(PKG_BUILD_DIR)/prepare_all $(PKG_BUILD_DIR) $(LINUX_DIR) 
	touch $@




$(PKG_BUILD_DIR)/.configured:  $(PKG_BUILD_DIR)/.prepared
	#---------======================================---------
	# 		configure and build drivers stack
	#---------======================================---------
	cp $(PKG_CONF_DIR)/files/cnb_stack $(PKG_BUILD_DIR)
	$(PKG_BUILD_DIR)/cnb_stack $(PKG_BUILD_DIR) $(LINUX_DIR) $(PKG_INSTALL_DIR)
	touch $@




$(PKG_BUILD_DIR)/.built:  $(PKG_BUILD_DIR)/.configured
	#---------======================================---------
	#		configure and build routines
	#---------======================================---------
	cp $(PKG_CONF_DIR)/files/cnb_routines $(PKG_BUILD_DIR)
	$(PKG_BUILD_DIR)/cnb_routines $(PKG_BUILD_DIR) $(PKG_INSTALL_DIR)
	touch $@

$(IPKG_SIGRAND_VOIP): 
	install -d -m0755 $(IDIR_SIGRAND_VOIP)/lib/modules/2.6.16/
	install -d -m0755 $(IDIR_SIGRAND_VOIP)/bin/
	install -d -m0755 $(IDIR_SIGRAND_VOIP)/lib/firmware/
	install -d -m0755 $(IDIR_SIGRAND_VOIP)/etc/init.d/
	cp -rpf $(PKG_INSTALL_DIR)/bin/*.ko $(IDIR_SIGRAND_VOIP)/lib/modules/2.6.16/
	cp -rpf $(PKG_INSTALL_DIR)/bin/sv* $(IDIR_SIGRAND_VOIP)/bin/
	cp -rpf $(PKG_CONF_DIR)/files/fw/* $(IDIR_SIGRAND_VOIP)/lib/firmware/
	cp -rpf $(PKG_CONF_DIR)/files/rc/* $(IDIR_SIGRAND_VOIP)/etc/init.d/
	cp -rpf $(PKG_CONF_DIR)/files/*.conf $(IDIR_SIGRAND_VOIP)/etc/
	echo "Depends: $(PKG_DEPEND)" >> $(IDIR_SIGRAND_VOIP)/CONTROL/control
	$(RSTRIP) $(IDIR_SIGRAND_VOIP)
	$(IPKG_BUILD) $(IDIR_SIGRAND_VOIP) $(PACKAGE_DIR)
	#---------============== IPKG_SIGRAN_VOIP ===========---------
	#---------- make ipkg
	#---------======================================---------
