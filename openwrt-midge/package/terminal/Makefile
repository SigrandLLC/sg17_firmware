# $Id:  $

include $(TOPDIR)/rules.mk

PKG_NAME:=terminal
PKG_VERSION:=1.0
PKG_RELEASE:=0


PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

include $(TOPDIR)/package/rules.mk

$(eval $(call PKG_template,SIGRAND_TERMINAL,terminal,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))

$(PKG_BUILD_DIR)/.prepared:
	mkdir -p $(PKG_BUILD_DIR)
	touch $@

$(PKG_BUILD_DIR)/.configured:
	touch $@

$(PKG_BUILD_DIR)/.built:
	$(TARGET_CC) $(TARGET_CFLAGS) -o $(PKG_BUILD_DIR)/demon demon.c kdb.c md5.c
	$(TARGET_CC) $(TARGET_CFLAGS) -o $(PKG_BUILD_DIR)/prog prog.c kdb.c md5.c
	touch $@

$(IPKG_SIGRAND_TERMINAL):
	install -d -m0755 $(IDIR_SIGRAND_TERMINAL)/sbin
	install -m0755 $(PKG_BUILD_DIR)/demon $(IDIR_SIGRAND_TERMINAL)/sbin
	install -m0755 $(PKG_BUILD_DIR)/prog $(IDIR_SIGRAND_TERMINAL)/sbin
	$(RSTRIP) $(IDIR_SIGRAND_TERMINAL)/sbin/*
	echo "Depends: $(PKG_DEPEND)" >> $(IDIR_SIGRAND_TERMINAL)/CONTROL/control
	$(IPKG_BUILD) $(IDIR_SIGRAND_TERMINAL) $(PACKAGE_DIR)

