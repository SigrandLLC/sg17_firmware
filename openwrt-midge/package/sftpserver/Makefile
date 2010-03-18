include $(TOPDIR)/rules.mk

PKG_NAME:=sftpserver
PKG_VERSION:=0.0+
PKG_RELEASE:=1
PKG_MD5SUM:=257e7ed0a9d6c8ece6b3dbcfa0edcbd6

DROPBEAR_SFTPSERVER_PATH = /usr/libexec/sftp-server

# bzr clone http://www.greenend.org.uk/rjk/bzr/sftpserver.dev sftpserver
#PKG_SOURCE_URL := http://www.greenend.org.uk/rjk/sftpserver/
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_CAT:=bzcat

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

include $(TOPDIR)/package/rules.mk

$(eval $(call PKG_template,SFTPSERVER,$(PKG_NAME),$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))

$(PKG_BUILD_DIR)/.configured:
	(cd $(PKG_BUILD_DIR); rm -rf config.cache; \
		$(TARGET_CONFIGURE_OPTS) \
		CFLAGS="$(TARGET_CFLAGS) \
			-I$(STAGING_DIR)/usr/include \
			-I$(STAGING_DIR)/usr/include/iconv" \
		LDFLAGS="-L$(STAGING_DIR)/usr/lib -L$(STAGING_DIR)/usr/lib/iconv -lpthread" \
		./configure -C	\
		--target=$(GNU_TARGET_NAME) \
		--host=$(GNU_TARGET_NAME) \
		--build=$(GNU_HOST_NAME) \
		--program-prefix="" \
		--program-suffix="" \
		--prefix=/usr \
		--exec-prefix=/usr \
		--bindir=/usr/bin \
		--datadir=/usr/share \
		--includedir=/usr/include \
		--infodir=/usr/share/info \
		--libdir=/usr/lib \
		--libexecdir=/usr/lib \
		--localstatedir=/var \
		--mandir=/usr/share/man \
		--sbindir=/usr/sbin \
		--sysconfdir=/etc \
		$(DISABLE_LARGEFILE) \
		--enable-daemon \
	);
	touch $@

$(PKG_BUILD_DIR)/.built:
	$(MAKE) -C $(PKG_BUILD_DIR)
	touch $@

$(IPKG_SFTPSERVER):
	mkdir -p $(dir $(IDIR_SFTPSERVER)$(DROPBEAR_SFTPSERVER_PATH))
	install -m 755 $(PKG_BUILD_DIR)/gesftpserver \
		$(IDIR_SFTPSERVER)$(DROPBEAR_SFTPSERVER_PATH)
	$(STRIP) $(IDIR_SFTPSERVER)$(DROPBEAR_SFTPSERVER_PATH)
	$(IPKG_BUILD) $(IDIR_SFTPSERVER) $(PACKAGE_DIR)
