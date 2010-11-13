include $(TOPDIR)/rules.mk

PKG_NAME:=webface
PKG_VERSION:=1
PKG_RELEASE:=2


PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

include $(TOPDIR)/package/rules.mk

$(eval $(call PKG_template,WEBFACE,webface,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))

$(PKG_BUILD_DIR)/.prepared:
	mkdir -p $(PKG_BUILD_DIR)
	touch $@

OEM_URL_INFO=[ <a href=\"$(BR2_SIGRAND_OEM_URL)\" class=\"tblnk\">$(BR2_SIGRAND_OEM_MANUFACTURER)</a> ]

$(PKG_BUILD_DIR)/.configured:
	touch $@

$(IPKG_WEBFACE):
	tar cf - -C files . | tar xvf - -C $(IDIR_WEBFACE)

	sed < $(IDIR_WEBFACE)/www/common/footer.sh.tmpl > $(IDIR_WEBFACE)/www/common/footer.sh \
	    -e "s%@OEM_URL_INFO@%$(OEM_URL_INFO)%g"
	rm -f $(IDIR_WEBFACE)/www/common/footer.sh.tmpl
	chmod +x $(IDIR_WEBFACE)/www/common/footer.sh

	sed < $(IDIR_WEBFACE)/www/common/header.sh.tmpl > $(IDIR_WEBFACE)/www/common/header.sh \
	    -e "s%@OEM_URL@%$(BR2_SIGRAND_OEM_URL)%g" \
	    -e "s%@OEM_MANUFACTURER@%$(BR2_SIGRAND_OEM_MANUFACTURER)%g"
	rm -f $(IDIR_WEBFACE)/www/common/header.sh.tmpl
	chmod +x $(IDIR_WEBFACE)/www/common/header.sh

ifneq ($(BR2_SIGRAND_OEM_BGROUND),"default")
	cp logos/$(BR2_SIGRAND_OEM_BGROUND) $(IDIR_WEBFACE)/www/img/SG.jpg;
endif

ifneq ($(BR2_SIGRAND_OEM_HEADERBG),"default")
	cp logos/$(BR2_SIGRAND_OEM_HEADERBG) $(IDIR_WEBFACE)/www/img/header_bg.png;
	cp logos/header.sh $(IDIR_WEBFACE)/www/common/header.sh
endif

ifneq ($(BR2_SIGRAND_OEM_LOGO),"default")
	cp logos/$(BR2_SIGRAND_OEM_LOGO) $(IDIR_WEBFACE)/www/img/logo.jpg;
	rm -f $(IDIR_WEBFACE)/www/img/logo.gif;
#	sed "s/href=\"http:\/\/www\.sigrand\.ru\/\"//g"  $(IDIR_WEBFACE)/www/common/footer.sh >  $(IDIR_WEBFACE)/www/common/footer.sh-tmp;
#	mv  $(IDIR_WEBFACE)/www/common/footer.sh-tmp  $(IDIR_WEBFACE)/www/common/footer.sh;
	cp logos/footer.sh $(IDIR_WEBFACE)/www/common/footer.sh
	cat $(IDIR_WEBFACE)/www/common/header.sh | sed "s/sigrand\.ru//g" | \
			sed "s/href=\"http:\/\/\"//g" > $(IDIR_WEBFACE)/www/common/header.sh-tmp;
	mv $(IDIR_WEBFACE)/www/common/header.sh-tmp $(IDIR_WEBFACE)/www/common/header.sh;
	chmod +x $(IDIR_WEBFACE)/www/common/header.sh $(IDIR_WEBFACE)/www/common/footer.sh;
endif

	$(IPKG_BUILD) $(IDIR_WEBFACE) $(PACKAGE_DIR)
