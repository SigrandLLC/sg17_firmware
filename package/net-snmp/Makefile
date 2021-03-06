# $Id: Makefile 3112 2006-02-01 23:53:19Z mbm $

include $(TOPDIR)/rules.mk

PKG_NAME:=net-snmp
PKG_VERSION:=5.1.2
PKG_RELEASE:=2.1
PKG_MD5SUM:=8080555ab3f90011f25d5122042d9a8d

PKG_SOURCE_URL:=@SF/net-snmp
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_CAT:=zcat

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/ipkg-install

SNMP_MIB_MODULES_INCLUDED = \
	disman/event-mib \
	host/hr_device \
	host/hr_disk \
	host/hr_filesys \
	host/hr_network \
	host/hr_partition \
	host/hr_proc \
	host/hr_system \
	mibII/at \
	mibII/icmp \
	mibII/interfaces \
	mibII/ip \
	mibII/snmp_mib \
	mibII/sysORTable \
	mibII/system_mib \
	mibII/tcp \
	mibII/udp \
	mibII/vacm_context \
	mibII/vacm_vars \
	mibII/shdsl/shdsl \
	snmpv3/snmpEngine \
	snmpv3/snmpMPDStats \
	snmpv3/usmStats \
	snmpv3/usmUser \
	snmpv3mibs \
	tunnel \
	ucd-snmp/disk \
	ucd-snmp/dlmod \
	ucd-snmp/extensible \
	ucd-snmp/loadave \
	ucd-snmp/memory \
	ucd-snmp/pass \
	ucd-snmp/proc \
	ucd-snmp/vmstat \
	util_funcs \
	utilities/execute \
	host/hr_storage \
	agent_mibs \
	agentx \
	host \
	ieee802dot11 \
	mibII \
	notification \
	snmpv3mibs \
	target \
	ucd_snmp \
	utilities \

#SNMP_MIB_MODULES_EXCLUDED = \
#	agent_mibs \
#	agentx \
#	host \
#	ieee802dot11 \
#	mibII \
#	notification \
#	snmpv3mibs \
#	target \
#	ucd_snmp \
#	utilities \
#
SNMP_TRANSPORTS_INCLUDED = Callback UDP

SNMP_TRANSPORTS_EXCLUDED = TCP TCPv6 UDPv6 Unix

#	--disable-mib-loading \
#	--disable-mibs \
#	--without-opaque-special-types \


PKG_CONFIGURE_OPTIONS = \
	--enable-shared \
	--enable-static \
	--with-endianness=little \
	--with-logfile=/var/log/snmpd.log \
	--with-persistent-directory=/usr/lib/snmp/ \
	--with-default-snmp-version=1 \
	--with-sys-contact=root@localhost \
	--with-sys-location=Unknown \
	--enable-applications \
	--disable-ipv6 \
	--disable-manuals \
	--disable-scripts \
	--disable-debugging \
	--with-out-mib-modules="$(SNMP_MIB_MODULES_EXCLUDED)" \
	--with-mib-modules="$(SNMP_MIB_MODULES_INCLUDED)" \
	--with-out-transports="$(SNMP_TRANSPORTS_EXCLUDED)" \
	--with-transports="$(SNMP_TRANSPORTS_INCLUDED)" \
	--with-openssl="" \
	--without-libwrap \
	--without-rpm \
	--without-zlib \

include $(TOPDIR)/package/rules.mk

$(eval $(call PKG_template,LIBNETSNMP,libnetsnmp,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))
$(eval $(call PKG_template,SNMPD,snmpd,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))
$(eval $(call PKG_template,SNMPD_STATIC,snmpd-static,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))
$(eval $(call PKG_template,SNMP_UTILS,snmp-utils,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))

$(PKG_BUILD_DIR)/.configured:
	( cd $(PKG_BUILD_DIR); rm -rf config.{cache,status}; \
		$(TARGET_CONFIGURE_OPTS) \
		CFLAGS="$(TARGET_CFLAGS)" \
		CPPFLAGS="-I$(STAGING_DIR)/usr/include -I$(STAGING_DIR)/include" \
		LDFLAGS="-L$(STAGING_DIR)/usr/lib -L$(STAGING_DIR)/lib" \
		ac_cv_header_pcap_h=no \
		./configure \
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
			$(DISABLE_NLS) \
			$(PKG_CONFIGURE_OPTIONS) \
	);
	touch $@

$(PKG_BUILD_DIR)/.built: $(PKG_BUILD_DIR)/.configured
	rm -rf $(PKG_INSTALL_DIR)
	mkdir -p $(PKG_INSTALL_DIR)
	$(MAKE1) -C $(PKG_BUILD_DIR) \
		INSTALL_PREFIX="$(PKG_INSTALL_DIR)" \
		all install
	( cd $(PKG_INSTALL_DIR); mv ./usr/sbin/snmpd ./usr/sbin/snmpd-shared; )
ifneq ($(BR2_PACKAGE_SNMPD_STATIC),)
	( cd $(PKG_BUILD_DIR); rm -f agent/snmpd; )
	$(MAKE1) -C $(PKG_BUILD_DIR) \
		LDFLAGS="-L$(STAGING_DIR)/usr/lib -L$(STAGING_DIR)/lib -static" \
		INSTALL_PREFIX="$(PKG_INSTALL_DIR)" \
		all install
	( cd $(PKG_INSTALL_DIR); mv ./usr/sbin/snmpd ./usr/sbin/snmpd-static; )
endif
	touch $@

$(IPKG_LIBNETSNMP):
	install -d -m0755 $(IDIR_LIBNETSNMP)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libnetsnmp{,agent,helpers,mibs}.so.* $(IDIR_LIBNETSNMP)/usr/lib/
	$(RSTRIP) $(IDIR_LIBNETSNMP)
	$(IPKG_BUILD) $(IDIR_LIBNETSNMP) $(PACKAGE_DIR)

$(IPKG_SNMPD):
	install -d -m0755 $(IDIR_SNMPD)/etc/snmp
	install -m0644 ./files/snmpd.conf $(IDIR_SNMPD)/etc/snmp/snmpd.conf
	install -m0644 ./files/dslam_ethernet_status.sh $(IDIR_SNMPD)/etc/snmp/dslam_ethernet_status.sh
	install -m0644 ./files/dslam_dsl_status.sh $(IDIR_SNMPD)/etc/snmp/dslam_dsl_status.sh
	install -d -m0755 $(IDIR_SNMPD)/etc/default
	install -m0644 ./files/snmpd.default $(IDIR_SNMPD)/etc/default/snmpd
	install -d -m0755 $(IDIR_SNMPD)/etc/init.d
	install -m0755 ./files/snmpd.init $(IDIR_SNMPD)/etc/init.d/snmpd
	rm -f $(IDIR_SNMPD)/etc/init.d/S80snmpd
	ln -s snmpd $(IDIR_SNMPD)/etc/init.d/S80snmpd
	install -d -m0755 $(IDIR_SNMPD)/usr/sbin
	install -m0755 $(PKG_INSTALL_DIR)/usr/sbin/snmpd-shared $(IDIR_SNMPD)/usr/sbin/snmpd
	$(RSTRIP) $(IDIR_SNMPD)
	$(IPKG_BUILD) $(IDIR_SNMPD) $(PACKAGE_DIR)

$(IPKG_SNMPD_STATIC):
	install -d -m0755 $(IDIR_SNMPD_STATIC)/etc/snmp
	install -m0644 ./files/snmpd.conf $(IDIR_SNMPD_STATIC)/etc/snmp/snmpd.conf
	install -m0644 ./files/dslam_ethernet_status.sh $(IDIR_SNMPD_STATIC)/etc/snmp/dslam_ethernet_status.sh
	install -m0644 ./files/dslam_dsl_status.sh $(IDIR_SNMPD_STATIC)/etc/snmp/dslam_dsl_status.sh
	install -d -m0755 $(IDIR_SNMPD_STATIC)/etc/default
	install -m0644 ./files/snmpd.default $(IDIR_SNMPD_STATIC)/etc/default/snmpd
	install -d -m0755 $(IDIR_SNMPD_STATIC)/etc/init.d
	install -m0755 ./files/snmpd.init $(IDIR_SNMPD_STATIC)/etc/init.d/snmpd
	rm -f $(IDIR_SNMPD_STATIC)/etc/init.d/S80snmpd
	ln -s snmpd $(IDIR_SNMPD_STATIC)/etc/init.d/S80snmpd
	install -d -m0755 $(IDIR_SNMPD_STATIC)/usr/sbin
	install -m0755 $(PKG_INSTALL_DIR)/usr/sbin/snmpd-static $(IDIR_SNMPD_STATIC)/usr/sbin/snmpd
	$(RSTRIP) $(IDIR_SNMPD_STATIC)
	$(IPKG_BUILD) $(IDIR_SNMPD_STATIC) $(PACKAGE_DIR)
	install -d -m0755 $(IDIR_SNMPD_STATIC)/usr/share
	install -d -m0755 $(IDIR_SNMPD_STATIC)/usr/share/snmp
	install -d -m0755 $(IDIR_SNMPD_STATIC)/usr/share/snmp/mibs
	$(CP) $(PKG_BUILD_DIR)/mibs/*.txt $(IDIR_SNMPD_STATIC)/usr/share/snmp/mibs

$(IPKG_SNMP_UTILS):
	install -d -m0755 $(IDIR_SNMP_UTILS)/usr/bin
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/snmp{get,set,status,test,trap,walk} $(IDIR_SNMP_UTILS)/usr/bin/
	$(RSTRIP) $(IDIR_SNMP_UTILS)
	$(IPKG_BUILD) $(IDIR_SNMP_UTILS) $(PACKAGE_DIR)

$(STAGING_DIR)/usr/lib/libnetsnmp.so: $(PKG_BUILD_DIR)/.built
	mkdir -p $(STAGING_DIR)/usr/bin
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/net-snmp-config $(STAGING_DIR)/usr/bin/
	mkdir -p $(STAGING_DIR)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/net-snmp $(STAGING_DIR)/usr/include/
	mkdir -p $(STAGING_DIR)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libnetsnmp{,agent,helpers,mibs}.{a,so*} $(STAGING_DIR)/usr/lib/
	touch $@

install-dev: $(STAGING_DIR)/usr/lib/libnetsnmp.so

uninstall-dev:
	rm -rf \
		$(STAGING_DIR)/usr/bin/net-snmp-config \
		$(STAGING_DIR)/usr/include/net-snmp \
		$(STAGING_DIR)/usr/lib/libnetsnmp{,agent,helpers,mibs}.{a,so*} \

compile-targets: install-dev
clean-targets: uninstall-dev
