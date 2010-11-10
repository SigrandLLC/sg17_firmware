# $Id:  $

include $(TOPDIR)/rules.mk

PKG_NAME:=tbuff
PKG_VERSION:=1.0
PKG_RELEASE:=0


PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

include $(TOPDIR)/package/rules.mk

$(eval $(call PKG_template,SIGRAND_TBUFF,tbuff,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))

$(PKG_BUILD_DIR)/.prepared:
	mkdir -p $(PKG_BUILD_DIR)
	touch $@

$(PKG_BUILD_DIR)/.configured:
	touch $@

$(PKG_BUILD_DIR)/.built:
	$(TARGET_CC) $(TARGET_CFLAGS) -o $(PKG_BUILD_DIR)/tbuffd tbuffd.c kdb.c md5.c remove_esc.c
	$(TARGET_CC) $(TARGET_CFLAGS) -o $(PKG_BUILD_DIR)/tbuffctl tbuffctl.c kdb.c md5.c
	touch $@

$(IPKG_SIGRAND_TBUFF):
	install -d -m0755 $(IDIR_SIGRAND_TBUFF)/sbin
	install -m0755 $(PKG_BUILD_DIR)/tbuffd $(IDIR_SIGRAND_TBUFF)/sbin/
	install -m0755 $(PKG_BUILD_DIR)/tbuffctl $(IDIR_SIGRAND_TBUFF)/sbin/
	install -d -m0755 $(IDIR_SIGRAND_TBUFF)/www
	install -d -m0755 $(IDIR_SIGRAND_TBUFF)/www/wf2
	install -d -m0755 $(IDIR_SIGRAND_TBUFF)/www/wf2/sh
	rm -f $(IDIR_SIGRAND_TBUFF)/www/wf2/sh/tbuffctl
	ln -s /sbin/tbuffctl $(IDIR_SIGRAND_TBUFF)/www/wf2/sh/tbuffctl
	install -d -m0755 $(IDIR_SIGRAND_TBUFF)/usr
	install -d -m0755 $(IDIR_SIGRAND_TBUFF)/usr/bin
	install -m0755 console $(IDIR_SIGRAND_TBUFF)/usr/bin/
	install -m0755 rs232-speed $(IDIR_SIGRAND_TBUFF)/usr/bin/
	install -d -m0755 $(IDIR_SIGRAND_TBUFF)/etc/init.d/
	install -m0700 tbuff $(IDIR_SIGRAND_TBUFF)/etc/init.d/
	rm -f       $(IDIR_SIGRAND_TBUFF)/etc/init.d/[SK]80tbuff
	ln -s tbuff $(IDIR_SIGRAND_TBUFF)/etc/init.d/S80tbuff
	ln -s tbuff $(IDIR_SIGRAND_TBUFF)/etc/init.d/K80tbuff
	$(RSTRIP) $(IDIR_SIGRAND_TBUFF)/sbin/*
	$(IPKG_BUILD) $(IDIR_SIGRAND_TBUFF) $(PACKAGE_DIR)

