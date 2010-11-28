# Main makefile for the packages
include $(TOPDIR)/rules.mk
include $(TOPDIR)/package/depend.mk

package-$(BR2_COMPILE_BIND) += bind
package-$(BR2_PACKAGE_BRIDGE) += bridge
package-$(BR2_PACKAGE_BUSYBOX) += busybox
package-$(BR2_PACKAGE_CBTT) += cbtt
package-$(BR2_PACKAGE_COLLECTD) += collectd
package-$(BR2_COMPILE_CYRUS_SASL) += cyrus-sasl
package-$(BR2_PACKAGE_DNSMASQ) += dnsmasq
package-$(BR2_PACKAGE_DROPBEAR) += dropbear
package-$(BR2_PACKAGE_SFTPSERVER) += sftpserver
package-$(BR2_COMPILE_GLIB) += glib
package-$(BR2_PACKAGE_HASERL) += haserl
package-$(BR2_PACKAGE_IFENSLAVE) += ifenslave
package-$(BR2_PACKAGE_IPKG) += ipkg
package-$(BR2_COMPILE_IPROUTE2) += iproute2
package-$(BR2_PACKAGE_IPSEC_TOOLS) += ipsec-tools
package-$(BR2_COMPILE_IPTABLES) += iptables
package-$(BR2_COMPILE_IPUTILS) += iputils
package-$(BR2_PACKAGE_LIBELF) += libelf
package-$(BR2_PACKAGE_LIBICONV) += libiconv
package-$(BR2_PACKAGE_LIBLOCKDEV) += liblockdev
package-$(BR2_PACKAGE_LIBLZO) += liblzo
package-$(BR2_PACKAGE_LIBPCAP) += libpcap
package-$(BR2_PACKAGE_LIBPTHREAD) += libpthread
package-$(BR2_PACKAGE_LIBWRAP) += tcp_wrappers
package-$(BR2_PACKAGE_LIGHTTPD) += lighttpd
package-$(BR2_PACKAGE_LOGROTATE) += logrotate
package-$(BR2_COMPILE_LUA) += lua
package-$(BR2_PACKAGE_MGETTY) += mgetty
package-$(BR2_PACKAGE_MICROCOM) += microcom
package-$(BR2_PACKAGE_MTR) += mtr
package-$(BR2_PACKAGE_MODULEINITTOOLS) += module-init-tools
package-$(BR2_PACKAGE_NANO) += nano
package-$(BR2_COMPILE_NCURSES) += ncurses
package-$(BR2_COMPILE_NET_SNMP) += net-snmp
package-$(BR2_PACKAGE_NTPCLIENT) += ntpclient
package-$(BR2_PACKAGE_NVRAM) += nvram
package-$(BR2_COMPILE_OPENSSL) += openssl
package-$(BR2_PACKAGE_OPENVPN) += openvpn
package-$(BR2_PACKAGE_PCIUTILS) += pciutils
package-$(BR2_COMPILE_PCMCIA_CS) += pcmcia-cs
package-$(BR2_COMPILE_PCRE) += pcre
package-$(BR2_PACKAGE_PEERGUARDIAN) += peerguardian
package-$(BR2_PACKAGE_PPP) += ppp
package-$(BR2_PACKAGE_PPTP) += pptp
package-$(BR2_PACKAGE_PROCPS) += procps
package-$(BR2_PACKAGE_QUAGGA) += quagga
package-$(BR2_COMPILE_READLINE) += readline
package-$(BR2_COMPILE_RP_PPPOE) += rp-pppoe
package-$(BR2_PACKAGE_RS232_TCPEXT) += rs232-tcpext
package-$(BR2_PACKAGE_SETSERIAL) += setserial
package-$(BR2_COMPILE_SPEEX) += speex
package-$(BR2_PACKAGE_STRACE) += strace
package-$(BR2_PACKAGE_TCPDUMP) += tcpdump
package-$(BR2_PACKAGE_UCLIBCXX) += uclibc++
package-$(BR2_PACKAGE_WRT_RADAUTH) += wrt-radauth
package-$(BR2_PACKAGE_ZLIB) += zlib
#
#
package-$(BR2_PACKAGE_WEBFACE) += webface
package-y += kdb
#package-y += hdb
package-y += htpasswd
package-$(BR2_PACKAGE_WF2) += wf2
package-$(BR2_PACKAGE_SIGRAND_SETHDLC) += sethdlc
package-$(BR2_PACKAGE_SIGRAND_MUX) += oem-mux
package-$(BR2_PACKAGE_SIGRAND_EOCD) += eocd
package-$(BR2_PACKAGE_SIGRAND_TBUFF) += tbuff
package-$(BR2_PACKAGE_SIGRAND_LINKDEPS) += linkdeps
package-$(BR2_PACKAGE_SIGRAND_PPPFT_MP) += pppft-mp
package-$(BR2_PACKAGE_SIGRAND_MR16H) += oem-shdsl.g1
package-$(BR2_PACKAGE_SIGRAND_MR17H) += oem-shdsl.g2
package-$(BR2_PACKAGE_SIGRAND_MR16G) += oem-e1.g1
package-$(BR2_PACKAGE_SIGRAND_MR17G) += oem-e1.g2
package-$(BR2_PACKAGE_SIGRAND_MR17S) += oem-rs232.g1
package-$(BR2_PACKAGE_SIGRAND_ADM5120SW) += adm5120sw
package-$(BR2_PACKAGE_SIGRAND_DSLAM) += dslam
package-$(BR2_PACKAGE_SIGRAND_MAM17H) += dslam-shdsl
package-$(BR2_PACKAGE_HOTPLUG) += hotplug
package-$(BR2_PACKAGE_CONTEXT_HELP) += context-help
package-$(BR2_PACKAGE_CONTEXT_HELP_OEM) += context-help-oem
package-$(BR2_PACKAGE_VOIP) += voip
#
#package-$(BR2_PACKAGE_APRSD) += aprsd


DEV_LIBS:=tcp_wrappers glib ncurses openssl pcre zlib libpcap iptables liblzo fuse portmap libelf uclibc++ speex wireless-tools nvram libffi-sable libpcap radiusclient-ng net-snmp
DEV_LIBS_COMPILE:=$(patsubst %,%-compile,$(DEV_LIBS))
SDK_DEFAULT_PACKAGES:=busybox dnsmasq iptables wireless-tools dropbear bridge ipkg ppp
SDK_DEFAULT_COMPILE:=$(patsubst %,%-compile,$(SDK_DEFAULT_PACKAGES))
COMPILE_PACKAGES:=$(patsubst %,%-compile,$(package-y) $(package-m))
INSTALL_PACKAGES:=$(patsubst %,%-install,$(package-y))

all: compile
clean: $(patsubst %,%-clean,$(package-) $(package-y) $(package-m))
compile: $(COMPILE_PACKAGES)
install: base-files-install $(INSTALL_PACKAGES)

$(COMPILE_PACKAGES): base-files-compile
$(INSTALL_PACKAGES): base-files-install

sdk-compile: $(DEV_LIBS_COMPILE) $(SDK_DEFAULT_COMPILE)

$(STAMP_DIR):
	mkdir -p $@

$(TARGET_DIR):
	mkdir -p $(TARGET_DIR)

%-prepare: $(STAMP_DIR) $(TARGET_DIR)
	@[ -f $(STAMP_DIR)/.$@ ] || { \
		$(START_TRACE) "package/$(patsubst %-prepare,%,$@)-prepare: "; \
		$(MAKE) -C $(patsubst %-prepare,%,$@) prepare && { \
			touch $(STAMP_DIR)/.$@; \
			$(CMD_TRACE) " done"; \
			$(END_TRACE); \
		} \
	}

%-menuconfig: %-prepare
	@$(START_TRACE) "package/$(patsubst %-menuconfig,%,$@)-menuconfig: "
	@$(MAKE) -C $(patsubst %-menuconfig,%,$@) menuconfig
	@$(CMD_TRACE) " done"
	@$(END_TRACE)

%-compile:
	@[ -f $(STAMP_DIR)/.$@ ] || { \
		$(START_TRACE) "package/$(patsubst %-compile,%,$@)-compile: "; \
		$(MAKE) -C $(patsubst %-compile,%,$@) compile && { \
			touch $(STAMP_DIR)/.$(patsubst %-compile,%,$@)-prepare; \
			touch $(STAMP_DIR)/.$@; \
			$(CMD_TRACE) " done"; \
			$(END_TRACE); \
		} \
	}

%-install: %-compile
	@$(START_TRACE) "package/$(patsubst %-install,%,$@)-install: "
	@$(MAKE) -C $(patsubst %-install,%,$@) install
	@$(CMD_TRACE) " done"
	@$(END_TRACE)

%-rebuild:
	@$(START_TRACE) "package/$(patsubst %-rebuild,%,$@)-rebuild: "
	@rm -f $(STAMP_DIR)/.$(patsubst %-rebuild,%,$@)-*
	$(MAKE) -C $(patsubst %-rebuild,%,$@) rebuild
	@$(CMD_TRACE) " done"
	@$(END_TRACE)

%-clean:
	@$(START_TRACE) "package/$(patsubst %-clean,%,$@)-clean: "
	@$(MAKE) -C $(patsubst %-clean,%,$@) clean
	@rm -f $(STAMP_DIR)/.$(patsubst %-clean,%,$@)-*
	@$(CMD_TRACE) " done"
	@$(END_TRACE)

