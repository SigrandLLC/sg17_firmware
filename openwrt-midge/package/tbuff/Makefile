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
	$(TARGET_CC) $(TARGET_CFLAGS) -o $(PKG_BUILD_DIR)/tbuffd demon.c kdb.c md5.c
	$(TARGET_CC) $(TARGET_CFLAGS) -o $(PKG_BUILD_DIR)/tbuffctl prog.c kdb.c md5.c
	touch $@

$(IPKG_SIGRAND_TBUFF):
	install -d -m0755 $(IDIR_SIGRAND_TBUFF)/sbin
	install -m0755 $(PKG_BUILD_DIR)/tbuffd $(IDIR_SIGRAND_TBUFF)/sbin/
	install -m0755 $(PKG_BUILD_DIR)/tbuffctl $(IDIR_SIGRAND_TBUFF)/sbin/
	cp S80tbuff $(PKG_BUILD_DIR)/
	chmod 700 $(PKG_BUILD_DIR)/S80tbuff
	$(RSTRIP) $(IDIR_SIGRAND_TBUFF)/sbin/*
	echo "Depends: $(PKG_DEPEND)" >> $(IDIR_SIGRAND_TBUFF)/CONTROL/control
	$(IPKG_BUILD) $(IDIR_SIGRAND_TBUFF) $(PACKAGE_DIR)

