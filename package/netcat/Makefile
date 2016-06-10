include $(TOPDIR)/rules.mk

PKG_NAME    := netcat
PKG_VERSION := 1.10
PKG_RELEASE := 41
PKG_MD5SUM  :=


PKG_SOURCE_URL  := http://mirror.yandex.ru/debian/pool/main/n/netcat/
PKG_SOURCE      := $(PKG_NAME)_$(PKG_VERSION).orig.tar.bz2
PKG_CAT         := bzcat

PKG_BUILD_DIR   :=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_INSTALL_DIR :=$(PKG_BUILD_DIR)/ipkg-install

include $(TOPDIR)/package/rules.mk

$(eval $(call PKG_template,NETCAT,netcat,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))


$(PKG_BUILD_DIR)/.prepared:
	mkdir -p $(PKG_BUILD_DIR)
	tar -xf $(DL_DIR)/$(PKG_SOURCE) -C $(PKG_BUILD_DIR) --strip-components=1
	list="`sed 's/\#.*$$//' < ./patches/series | grep -v '^$$'`"; \
		if test -n "$$list" ; then \
			for patch in `echo $$list` ; do \
				patch -d $(PKG_BUILD_DIR) -p1 < ./patches/$$patch; \
			done ; \
		fi
	touch $@

$(PKG_BUILD_DIR)/.built: $(PKG_BUILD_DIR)/.prepared
	$(MAKE) -C $(PKG_BUILD_DIR) linux \
	  CC="$(TARGET_CC)" \
	  XFLAGS="$(TARGET_CFLAGS) -I$(STAGING_DIR)/usr/include -I$(STAGING_DIR)/include" \
	  LDFLAGS="-L$(STAGING_DIR)/usr/lib -L$(STAGING_DIR)/lib" \
	  STATIC='' \
	  DFLAGS='-DLINUX -DTELNET -DGAPING_SECURITY_HOLE -DIP_TOS -DDEBIAN_VERSION=\"$(PKG_VERSION)-$(PKG_RELEASE)\"'
	touch $@

$(IPKG_NETCAT):
	install -m0755 -d             $(IDIR_NETCAT)/usr/bin
	mkdir -p                  $(PKG_INSTALL_DIR)/usr/bin
	$(CP) $(PKG_BUILD_DIR)/nc $(PKG_INSTALL_DIR)/usr/bin/
	$(CP) $(PKG_BUILD_DIR)/nc     $(IDIR_NETCAT)/usr/bin/
	ln -sf                 nc $(PKG_INSTALL_DIR)/usr/bin/netcat
	ln -sf                 nc     $(IDIR_NETCAT)/usr/bin/netcat
	$(RSTRIP) $(IDIR_NETCAT)
	$(IPKG_BUILD) $(IDIR_NETCAT) $(PACKAGE_DIR)
