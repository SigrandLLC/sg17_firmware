include $(TOPDIR)/rules.mk

PKG_NAME      = context-help-oem
PKG_RELEASE   = 1
PKG_VERSION   = 1.1.2
pkg_dir       = $(PKG_NAME)-$(PKG_VERSION)
PKG_BUILD_DIR = $(BUILD_DIR)/$(PKG_NAME)
#http://sigrand.ru/temp/doc/$(PKG_NAME)-$(PKG_VERSION).tar.bz2

include $(TOPDIR)/package/rules.mk

$(eval $(call PKG_template,CONTEXT_HELP_OEM,context-help-oem,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))

$(PKG_BUILD_DIR)/.prepared:
	mkdir -p $(PKG_BUILD_DIR)
	touch $@

$(IPKG_CONTEXT_HELP_OEM):
	install -d -m0755 $(IDIR_CONTEXT_HELP_OEM)/www/help
	install -d -m0755 $(IDIR_CONTEXT_HELP_OEM)/www/help/img
	install -m0644 $(pkg_dir)/*.html $(IDIR_CONTEXT_HELP_OEM)/www/help
	install -m0644 $(pkg_dir)/img/*  $(IDIR_CONTEXT_HELP_OEM)/www/help/img
	$(IPKG_BUILD) $(IDIR_CONTEXT_HELP_OEM) $(PACKAGE_DIR)
