aprsdigi-compile: libax25-compile
ax25-apps-compile: libax25-compile
arpd-compile: libpcap-compile
arpwatch-compile: libpcap-compile
atftp-compile: readline-compile ncurses-compile
bind-compile: openssl-compile
bitchx-compile: ncurses-compile
bitlbee-compile: libiconv-compile openssl-compile glib-compile
cbtt-compile: mysql-compile zlib-compile
curl-compile: openssl-compile zlib-compile
cyrus-sasl-compile: openssl-compile
deco-compile: ncurses-compile
dhcp6-compile: ncurses-compile
dsniff-compile: openssl-compile
freetype-compile: zlib-compile
fprobe-compile: libpcap-compile
gdbserver-compile: ncurses-compile
gpsd-compile: uclibc++-compile
iftop-compile: libpcap-compile libpthread-compile ncurses-compile
ipcad-compile: libpcap-compile
irssi-compile: glib-compile ncurses-compile
iperf-compile: uclibc++-compile
iptables-snmp-compile: net-snmp-compile
iptraf-compile: ncurses-compile
ipsec-tools-compile: openssl-compile
jamvm-compile: libffi-sable-compile zlib-compile sablevm-classpath-compile
httping-compile: openssl-compile
kismet-compile: uclibc++-compile libpcap-compile ncurses-compile
less-compile: ncurses-compile
lcd4linux-compile: ncurses-compile
lighttpd-compile: openssl-compile pcre-compile
logrotate-compile: popt-compile
miax-compile: bluez-libs-compile
miredo-compile: uclibc++-compile
monit-compile: openssl-compile
mtr-compile: ncurses-compile
mutt-compile: ncurses-compile openssl-compile
mysql-compile: ncurses-compile zlib-compile readline-compile
nano-compile: ncurses-compile
net-snmp-compile: libelf-compile
nfs-server-compile: portmap-compile
nmap-compile: uclibc++-compile pcre-compile libpcap-compile
nocatsplash-compile: glib-compile
voip-compile: sofia-sip-install libconfig-install
voip-compile: drv-vinetic-compile drv-tapi-compile drv-daa-compile
voip-compile: drv-sgatab-compile libab-compile svd-compile
drv-vinetic-compile: drv-tapi-compile
drv-daa-compile: drv-tapi-compile drv-vinetic-compile
drv-sgatab-compile: drv-tapi-compile drv-vinetic-compile
libab-compile: drv-tapi-compile drv-vinetic-compile drv-sgatab-compile
svd-compile: drv-tapi-compile libab-compile libconfig-install
openh323-compile: pwlib-compile
openldap-compile: cyrus-sasl-compile openssl-compile
openssh-compile: zlib-compile openssl-compile
openssl-compile: zlib-compile
osiris-compile: openssl-compile
peercast-compile: uclibc++-compile
peerguardian-compile: libpthread-compile
portmap-compile: tcp_wrappers-compile
postgresql-compile: zlib-compile
ppp-compile: libpcap-compile
privoxy-compile: pcre-compile
ptunnel-compile: libpcap-compile
quagga-compile: readline-compile ncurses-compile
rsync-compile: popt-compile
screen-compile: ncurses-compile
sftpserver-compile: libiconv-install
rs232-tcpext-compile: liblockdev-install
sipp-compile: ncurses-compile uclibc++-compile libpthread-compile
siproxd-compile:
sipsak-compile: openssl-compile
socat-compile: openssl-compile
sqlite-compile: ncurses-compile readline-compile
sqlite2-compile: ncurses-compile readline-compile
squid-compile: openssl-compile
ssltunnel-compile: openssl-compile ppp-compile
tcpdump-compile: libpcap-compile
tinc-compile: zlib-compile openssl-compile liblzo-compile
vncrepeater-compile: uclibc++-compile
vtun-compile: zlib-compile openssl-compile liblzo-compile
wificonf-compile: wireless-tools-compile nvram-compile
wiviz-compile: libpcap-compile
wknock-compile: libpcap-compile
wpa_supplicant-compile: openssl-compile
wx200d-compile: postgresql-compile
xsupplicant-compile: openssl-compile

asterisk-compile: ncurses-compile openssl-compile
ifneq ($(BR2_PACKAGE_ASTERISK_CHAN_BLUETOOTH),)
asterisk-compile: bluez-libs-compile
endif
ifneq ($(BR2_PACKAGE_ASTERISK_CHAN_H323),)
asterisk-compile: openh323-compile uclibc++-compile
endif
ifneq ($(BR2_PACKAGE_ASTERISK_CODEC_SPEEX),)
asterisk-compile: speex-compile
endif
ifneq ($(BR2_PACKAGE_ASTERISK_PGSQL),)
asterisk-compile: postgresql-compile
endif
ifneq ($(BR2_PACKAGE_ASTERISK_MYSQL),)
asterisk-compile: mysql-compile
endif
ifneq ($(BR2_PACKAGE_ASTERISK_SQLITE),)
asterisk-compile: sqlite2-compile
endif

hostapd-compile: wireless-tools-compile
ifneq ($(BR2_PACKAGE_HOSTAPD),)
hostapd-compile: openssl-compile
endif

ifeq ($(BR2_PACKAGE_LIBOPENSSL),y)
openvpn-compile: openssl-compile
endif
ifeq ($(BR2_PACKAGE_OPENVPN_LZO),y)
openvpn-compile: liblzo-compile
endif

pmacct-compile: libpcap-compile
ifneq ($(BR2_COMPILE_PMACCT_MYSQL),)
pmacct-compile: mysql-compile
endif
ifneq ($(BR2_COMPILE_PMACCT_PGSQL),)
pmacct-compile: postgresql-compile
endif
ifneq ($(BR2_COMPILE_PMACCT_SQLITE),)
pmacct-compile: sqlite-compile
endif

rrs-compile: uclibc++-compile
ifneq ($(BR2_PACKAGE_RRS),)
rrs-compile: openssl-compile
endif

ulogd-compile: iptables-compile
ifneq ($(BR2_PACKAGE_ULOGD_MOD_MYSQL),)
ulogd-compile: mysql-compile
endif
ifneq ($(BR2_PACKAGE_ULOGD_MOD_PCAP),)
ulogd-compile: libpcap-compile
endif
ifneq ($(BR2_PACKAGE_ULOGD_MOD_PGSQL),)
ulogd-compile: postgresql-compile
endif
ifneq ($(BR2_PACKAGE_ULOGD_MOD_SQLITE),)
ulogd-compile: sqlite-compile
endif
