include $(TOPDIR)/rules.mk

PKG_NAME      = wf2
PKG_RELEASE   = 2
PKG_BUILD_DIR = $(BUILD_DIR)/$(PKG_NAME)

include $(TOPDIR)/package/rules.mk

$(eval $(call PKG_template,WF2,wf2,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))

$(PKG_BUILD_DIR)/.prepared:
	mkdir -p $(PKG_BUILD_DIR)
	cp -a templates $(PKG_BUILD_DIR)
	touch $@

$(PKG_BUILD_DIR)/.configured:
#------------------- OEM -------------------------#
# Sigrand modules
	sed <$(PKG_BUILD_DIR)/templates/etc/templates/oem-vars.tmpl	\
	> $(PKG_BUILD_DIR)/templates/etc/templates/oem-vars		\
	-e "s%@OEM_IFPFX@%$(BR2_SIGRAND_OEM_IFPFX)%g"			\
	-e "s%@MR16H_TOOL@%$(BR2_SIGRAND_OEM_MR16H_TOOL)%g"		\
	-e "s%@MR16H_MODNAME@%$(BR2_SIGRAND_OEM_MR16H)%g"		\
	-e "s%@MR16H_DRVNAME@%$(BR2_SIGRAND_OEM_MR16H_DRV)%g"		\
	-e "s%@MR16G_MODNAME@%$(BR2_SIGRAND_OEM_MR16G)%g"		\
	-e "s%@MR16G_DRVNAME@%$(BR2_SIGRAND_OEM_MR16G_DRV)%g"		\
	-e "s%@MR17G_MODNAME@%$(BR2_SIGRAND_OEM_MR17G)%g"		\
	-e "s%@MR17G_DRVNAME@%$(BR2_SIGRAND_OEM_MR17G_DRV)%g"		\
	-e "s%@MR17S_MODNAME@%$(BR2_SIGRAND_OEM_MR17S)%g"		\
	-e "s%@MR17S_DRVNAME@%$(BR2_SIGRAND_OEM_MR17S_DRV)%g"		\
	-e "s%@MR17S_TTYNAME@%$(BR2_SIGRAND_OEM_MR17S_TTYNAME)%g"	\
	-e "s%@MR17S_TOOL@%$(BR2_SIGRAND_OEM_MR17S_TOOL)%g"		\
	-e "s%@MR17H_TOOL@%$(BR2_SIGRAND_OEM_MR17H_TOOL)%g"		\
	-e "s%@MR17H_MODNAME@%$(BR2_SIGRAND_OEM_MR17H)%g"		\
	-e "s%@MR17H_V1SFX@%$(BR2_SIGRAND_OEM_MR17H_V1SFX)%g"		\
	-e "s%@MR17H_V2SFX@%$(BR2_SIGRAND_OEM_MR17H_V2SFX)%g"		\
	-e "s%@MR17H_PWRSFX@%$(BR2_SIGRAND_OEM_MR17H_PWRSFX)%g"		\
	-e "s%@MR17H_DRVNAME@%$(BR2_SIGRAND_OEM_MR17H_DRV)%g"		\
	-e "s%@MS17H_MODNAME@%$(BR2_SIGRAND_OEM_MS17H)%g"		\
	-e "s%@MS17E_MODNAME@%$(BR2_SIGRAND_OEM_MS17E)%g"		\
	-e "s%@MS17E_V2_MODNAME@%$(BR2_SIGRAND_OEM_MS17E_V2)%g"
	rm --force $(PKG_BUILD_DIR)/templates/etc/templates/oem-vars.tmpl
#  E1 for controllers
	sed <$(PKG_BUILD_DIR)/templates/www/oem.sh.tmpl			\
	> $(PKG_BUILD_DIR)/templates/www/oem.sh				\
	-e "s%@OEM_IFPFX@%$(BR2_SIGRAND_OEM_IFPFX)%g"			\
	-e "s%@MR17H_MODNAME@%$(BR2_SIGRAND_OEM_MR17H)%g"		\
	-e "s%@MR17H_DRVNAME@%$(BR2_SIGRAND_OEM_MR17H_DRV)%g"		\
	-e "s%@MR17H_V1SFX@%$(BR2_SIGRAND_OEM_MR17H_V1SFX)%g"		\
	-e "s%@MR17H_V2SFX@%$(BR2_SIGRAND_OEM_MR17H_V2SFX)%g"		\
	-e "s%@MR17H_PWRSFX@%$(BR2_SIGRAND_OEM_MR17H_PWRSFX)%g"		\
	-e "s%@MR16G_MODNAME@%$(BR2_SIGRAND_OEM_MR16G)%g"		\
	-e "s%@MR16G_DRVNAME@%$(BR2_SIGRAND_OEM_MR16G_DRV)%g"		\
	-e "s%@MR17G_MODNAME@%$(BR2_SIGRAND_OEM_MR17G)%g"		\
	-e "s%@MR17G_DRVNAME@%$(BR2_SIGRAND_OEM_MR17G_DRV)%g"		\
	-e "s%@MR17S_MODNAME@%$(BR2_SIGRAND_OEM_MR17S)%g"		\
	-e "s%@MR17S_DRVNAME@%$(BR2_SIGRAND_OEM_MR17S_DRV)%g"		\
	-e "s%@MR16H_MODNAME@%$(BR2_SIGRAND_OEM_MR16H)%g"		\
	-e "s%@MR16H_DRVNAME@%$(BR2_SIGRAND_OEM_MR16H_DRV)%g"		\
	-e "s%@MS17H_MODNAME@%$(BR2_SIGRAND_OEM_MS17H)%g"		\
	-e "s%@MS17E_MODNAME@%$(BR2_SIGRAND_OEM_MS17E)%g"		\
	-e "s%@MS17E_V2_MODNAME@%$(BR2_SIGRAND_OEM_MS17E_V2)%g"
	rm --force $(PKG_BUILD_DIR)/templates/www/oem.sh.tmpl
	chmod +x $(PKG_BUILD_DIR)/templates/www/oem.sh
	touch $@

$(PKG_BUILD_DIR)/.built:
	touch $@

$(IPKG_WF2):
	tar cf - -C files . | tar xvf - -C $(IDIR_WF2)
	install -d -m0755 $(IDIR_WF2)/usr/bin
	tar cf - -C $(PKG_BUILD_DIR)/templates . | tar xvf - -C $(IDIR_WF2)

	echo $(BR2_SIGRAND_OEM_HOSTNAME) > $(IDIR_WF2)/etc/hostname

	sed -e "s/Sigrand router/$(BR2_SIGRAND_OEM_SYSNAME)/g" \
		< files/etc/lighttpd.conf > $(IDIR_WF2)/etc/lighttpd.conf

ifneq ($(BR2_SIGRAND_OEM_WF2_HEADER_LOGO),"default")
	cp logos/$(BR2_SIGRAND_OEM_WF2_HEADER_LOGO) $(IDIR_WF2)/www/wf2/ui/img/header.jpg;
endif

ifneq ($(BR2_SIGRAND_OEM_WF2_BOTTOM_LOGO),"default")
	cp logos/$(BR2_SIGRAND_OEM_WF2_BOTTOM_LOGO) $(IDIR_WF2)/www/wf2/ui/img/footer_logo.gif;
endif

	$(IPKG_BUILD) $(IDIR_WF2) $(PACKAGE_DIR)
