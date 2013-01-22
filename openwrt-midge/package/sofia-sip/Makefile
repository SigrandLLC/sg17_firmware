include $(TOPDIR)/rules.mk

PKG_NAME:=sofia-sip
 version = 1.12
PKG_VERSION:=$(version).10
PKG_RELEASE:=4
PKG_MD5SUM:=9e07fde3ad2009e44d1100ca3950d02b

PKG_SOURCE_URL := http://debian.nsu.ru/debian/pool/main/s/sofia-sip/
PKG_SOURCE_URL += http://ftp.debian.org/debian/pool/main/s/sofia-sip/
PKG_SOURCE:=$(PKG_NAME)_$(PKG_VERSION).orig.tar.gz
PKG_CAT:=zcat

PKG_BUILD_DIR:=$(BUILD_DIR)/sofia-sip-$(PKG_VERSION)
PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/install

include $(TOPDIR)/package/target-configure-staging-flags.mk

include $(TOPDIR)/package/rules.mk

$(eval $(call PKG_template,SOFIA_SIP,sofia-sip,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))


ifeq "0" "1"
  --enable-expensive-checks
                          run also expensive checks [disabled]
  --enable-64             build with 64 bit support
  --enable-experimental   enable experimental features [disabled]
  --disable-tag-cast      cast tag values with inlined functions [enabled]
  --disable-size-compat   use compatibility size_t types [enabled]
  --disable-ip6          disable IPv6 functionality [enabled]
  --disable-poll-port     disable su_poll_port [enabled]
                          Use this option in systems emulating poll with select
  --enable-sctp           use SCTP [disabled]
  --disable-stun          disable stun module (enabled)
  --enable-ntlm           enable NTLM support [disabled]
  --with-rt               use POSIX realtime library [used by default]
endif

$(PKG_BUILD_DIR)/.configured:
	rm -rf $(PKG_BUILD_DIR)/config.{cache,status}
	cd $(PKG_BUILD_DIR) &&			\
		$(TARGET_CONFIGURE_OPTS)	\
		./configure			\
			--target=$(GNU_TARGET_NAME)	\
			--host=$(GNU_TARGET_NAME)	\
			--build=$(GNU_HOST_NAME)	\
			--prefix=/usr		\
			$(DISABLE_LARGEFILE)	\
			$(DISABLE_NLS)		\
			--disable-shared	\
			--enable-static		\
			--with-gnu-ld		\
			--without-doxygen	\
			--without-glib		\
			--without-openssl	\
			--disable-nth		\
			--disable-ip6		\
			--enable-ndebug
	touch $@


$(PKG_BUILD_DIR)/.built:
	$(MAKE) -C $(PKG_BUILD_DIR)/libsofia-sip-ua install DESTDIR=$(PKG_INSTALL_DIR)
	touch $@

$(PKG_BUILD_DIR)/.install-dev : $(PKG_BUILD_DIR)/.built
	mkdir -p $(STAGING_DIR)/usr/include
	$(CP) -a $(PKG_INSTALL_DIR)/usr/include/sofia-sip-$(version)/sofia*	\
		 $(STAGING_DIR)/usr/include/
	mkdir -p $(STAGING_DIR)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libsofia*.a	\
		 $(STAGING_DIR)/usr/lib/
	touch $@

install-dev: $(PKG_BUILD_DIR)/.install-dev

uninstall-dev:
	rm -rf	$(STAGING_DIR)/usr/lib/libsofia*.a \
		$(STAGING_DIR)/usr/include/sofia*

compile-targets:   install-dev
  clean-targets: uninstall-dev

