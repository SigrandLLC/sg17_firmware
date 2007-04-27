# $Id: netfilter.mk 2411 2005-11-11 03:41:43Z nico $

ifeq ($(BR2_LINUX_2_6),y)
P_V4:=ipv4/netfilter/
P_XT:=netfilter/
else
P_V4:=
P_XT:=
endif

IPT_CONNTRACK-m :=
IPT_CONNTRACK-$(CONFIG_IP_NF_MATCH_CONNMARK) += $(P_V4)ipt_connmark
IPT_CONNTRACK-$(CONFIG_IP_NF_TARGET_CONNMARK) += $(P_V4)ipt_CONNMARK
IPT_CONNTRACK-$(CONFIG_IP_NF_MATCH_CONNTRACK) += $(P_V4)ipt_conntrack
IPT_CONNTRACK-$(CONFIG_IP_NF_MATCH_HELPER) += $(P_V4)ipt_helper
IPT_CONNTRACK-$(CONFIG_IP_NF_MATCH_STATE) += $(P_V4)ipt_state
IPT_CONNTRACK-$(CONFIG_NETFILTER_XT_MATCH_CONNTRACK) += $(P_XT)xt_conntrack
IPT_CONNTRACK-$(CONFIG_NETFILTER_XT_MATCH_HELPER) += $(P_XT)xt_helper
IPT_CONNTRACK-$(CONFIG_NETFILTER_XT_MATCH_STATE) += $(P_XT)xt_state


IPT_EXTRA-m :=
ifneq ($(KERNEL),2.6)
IPT_EXTRA-$(CONFIG_IP_NF_MATCH_CONDITION) += $(P_V4)ipt_condition
IPT_EXTRA-$(CONFIG_IP_NF_MATCH_CONNBYTES) += $(P_V4)ipt_connbytes
IPT_EXTRA-$(CONFIG_NETFILTER_XT_MATCH_CONNBYTES) += $(P_XT)xt_connbytes
IPT_EXTRA-$(CONFIG_IP_NF_MATCH_LIMIT) += $(P_V4)ipt_limit
IPT_EXTRA-$(CONFIG_IP_NF_MATCH_PHYSDEV) += $(P_V4)ipt_physdev
IPT_EXTRA-$(CONFIG_IP_NF_MATCH_PKTTYPE) += $(P_V4)ipt_pkttype
IPT_EXTRA-$(CONFIG_IP_NF_MATCH_QUOTA) += $(P_V4)ipt_quota
IPT_EXTRA-$(CONFIG_IP_NF_MATCH_STRING) += $(P_V4)ipt_string
endif
IPT_EXTRA-$(CONFIG_NETFILTER_XT_MATCH_LIMIT) += $(P_XT)xt_limit
IPT_EXTRA-$(CONFIG_IP_NF_TARGET_LOG) += $(P_V4)ipt_LOG
IPT_EXTRA-$(CONFIG_IP_NF_MATCH_MULTIPORT) += $(P_V4)ipt_multiport
IPT_EXTRA-$(CONFIG_IP_NF_MATCH_OWNER) += $(P_V4)ipt_owner
IPT_EXTRA-$(CONFIG_NETFILTER_XT_MATCH_PKTTYPE) += $(P_XT)xt_pkttype
IPT_EXTRA-$(CONFIG_IP_NF_MATCH_RECENT) += $(P_V4)ipt_recent
IPT_EXTRA-$(CONFIG_IP_NF_TARGET_REJECT) += $(P_V4)ipt_REJECT
IPT_EXTRA-$(CONFIG_NETFILTER_XT_MATCH_STRING) += $(P_XT)xt_string

IPT_FILTER-m :=
IPT_FILTER-$(CONFIG_IP_NF_MATCH_IPP2P) += $(P_V4)ipt_ipp2p
IPT_FILTER-$(CONFIG_IP_NF_MATCH_LAYER7) += $(P_V4)ipt_layer7

IPT_IMQ-m :=
IPT_IMQ-$(CONFIG_IP_NF_TARGET_IMQ) += $(P_V4)ipt_IMQ

IPT_IPOPT-m :=
ifneq ($(KERNEL),2.6)
IPT_IPOPT-$(CONFIG_IP_NF_MATCH_LENGTH) += $(P_V4)ipt_length
IPT_IPOPT-$(CONFIG_IP_NF_MATCH_MAC) += $(P_V4)ipt_mac
IPT_IPOPT-$(CONFIG_IP_NF_MATCH_MARK) += $(P_V4)ipt_mark
IPT_IPOPT-$(CONFIG_IP_NF_TARGET_MARK) += $(P_V4)ipt_MARK
IPT_IPOPT-$(CONFIG_IP_NF_MATCH_TCPMSS) += $(P_V4)ipt_tcpmss
IPT_IPOPT-$(CONFIG_IP_NF_MATCH_UNCLEAN) += $(P_V4)ipt_unclean
endif
IPT_IPOPT-$(CONFIG_IP_NF_MATCH_DSCP) += $(P_V4)ipt_dscp
IPT_IPOPT-$(CONFIG_IP_NF_TARGET_DSCP) += $(P_V4)ipt_DSCP
IPT_IPOPT-$(CONFIG_IP_NF_MATCH_ECN) += $(P_V4)ipt_ecn
IPT_IPOPT-$(CONFIG_IP_NF_TARGET_ECN) += $(P_V4)ipt_ECN
IPT_IPOPT-$(CONFIG_NETFILTER_XT_MATCH_LENGTH) += $(P_XT)xt_length
IPT_IPOPT-$(CONFIG_NETFILTER_XT_MATCH_MAC) += $(P_XT)xt_mac
IPT_IPOPT-$(CONFIG_NETFILTER_XT_MATCH_MARK) += $(P_XT)xt_mark
IPT_IPOPT-$(CONFIG_NETFILTER_XT_TARGET_MARK) += $(P_XT)xt_MARK
IPT_IPOPT-$(CONFIG_NETFILTER_XT_MATCH_TCPMSS) += $(P_XT)xt_tcpmss
IPT_IPOPT-$(CONFIG_IP_NF_TARGET_TCPMSS) += $(P_V4)ipt_TCPMSS
IPT_IPOPT-$(CONFIG_IP_NF_MATCH_TOS) += $(P_V4)ipt_tos
IPT_IPOPT-$(CONFIG_IP_NF_MATCH_TIME) += $(P_V4)ipt_time
IPT_IPOPT-$(CONFIG_IP_NF_TARGET_TOS) += $(P_V4)ipt_TOS
IPT_IPOPT-$(CONFIG_IP_NF_MATCH_TTL) += $(P_V4)ipt_ttl
IPT_IPOPT-$(CONFIG_IP_NF_TARGET_TTL) += $(P_V4)ipt_TTL

IPT_IPSEC-m :=
IPT_IPSEC-$(CONFIG_IP_NF_MATCH_AH_ESP) += $(P_V4)ipt_ah $(P_V4)ipt_esp

IPT_NAT-m :=
# IPT_NAT-$(CONFIG_IP_NF_NAT) += $(P_V4)ipt_SNAT $(P_V4)ipt_DNAT
IPT_NAT-$(CONFIG_IP_NF_TARGET_MASQUERADE) += $(P_V4)ipt_MASQUERADE
IPT_NAT-$(CONFIG_IP_NF_TARGET_MIRROR) += $(P_V4)ipt_MIRROR
IPT_NAT-$(CONFIG_IP_NF_TARGET_REDIRECT) += $(P_V4)ipt_REDIRECT
ifeq ($(BR2_LINUX_2_6),y)
IPT_NAT-m += $(P_V4)ip_nat
endif


IPT_NAT_EXTRA-m := 
IPT_NAT_EXTRA-$(CONFIG_IP_NF_AMANDA) += $(P_V4)ip_conntrack_amanda
IPT_NAT_EXTRA-$(CONFIG_IP_NF_CT_PROTO_GRE) += $(P_V4)ip_conntrack_proto_gre
IPT_NAT_EXTRA-$(CONFIG_IP_NF_NAT_PROTO_GRE) += $(P_V4)ip_nat_proto_gre
IPT_NAT_EXTRA-$(CONFIG_IP_NF_H323) += $(P_V4)ip_conntrack_h323
IPT_NAT_EXTRA-$(CONFIG_IP_NF_NAT_H323) += $(P_V4)ip_nat_h323
IPT_NAT_EXTRA-$(CONFIG_IP_NF_MMS) += $(P_V4)ip_conntrack_mms
IPT_NAT_EXTRA-$(CONFIG_IP_NF_NAT_MMS) += $(P_V4)ip_nat_mms
IPT_NAT_EXTRA-$(CONFIG_IP_NF_RTSP) += $(P_V4)ip_conntrack_rtsp
IPT_NAT_EXTRA-$(CONFIG_IP_NF_NAT_RTSP) += $(P_V4)ip_nat_rtsp
IPT_NAT_EXTRA-$(CONFIG_IP_NF_PPTP) += $(P_V4)ip_conntrack_pptp
IPT_NAT_EXTRA-$(CONFIG_IP_NF_FTP) += $(P_V4)ip_conntrack_ftp
IPT_NAT_EXTRA-$(CONFIG_IP_NF_SIP) += $(P_V4)ip_conntrack_sip
IPT_NAT_EXTRA-$(CONFIG_IP_NF_NAT_PPTP) += $(P_V4)ip_nat_pptp
IPT_NAT_EXTRA-$(CONFIG_IP_NF_NAT_FTP) += $(P_V4)ip_nat_ftp
ifeq ($(BR2_LINUX_2_6),y)
IPT_NAT_EXTRA-$(CONFIG_IP_NF_NAT_SIP) += $(P_V4)ip_nat_sip
IPT_NAT_EXTRA-$(CONFIG_IP_NF_NAT_SNMP_BASIC) += $(P_V4)ip_nat_snmp_basic
IPT_NAT_EXTRA-$(CONFIG_IP_NF_SIP) += $(P_V4)ip_conntrack_sip
IPT_NAT_EXTRA-$(CONFIG_IP_NF_NAT_SIP) += $(P_V4)ip_nat_sip
IPT_NAT_EXTRA-$(CONFIG_IP_NF_TFTP) += $(P_V4)ip_conntrack_tftp
endif

IPT_QUEUE-m :=
IPT_QUEUE-$(CONFIG_IP_NF_QUEUE) += $(P_V4)ip_queue

IPT_ULOG-m :=
IPT_ULOG-$(CONFIG_IP_NF_TARGET_ULOG) += $(P_V4)ipt_ULOG

IPT_BUILTIN := ipt_standard
IPT_BUILTIN += ipt_icmp ipt_tcp ipt_udp
IPT_BUILTIN += $(IPT_CONNTRACK-y)
IPT_BUILTIN += $(IPT_EXTRA-y)
IPT_BUILTIN += $(IPT_FILTER-y)
IPT_BUILTIN += $(IPT_IMQ-y)
IPT_BUILTIN += $(IPT_IPOPT-y)
IPT_BUILTIN += $(IPT_IPSEC-y)
IPT_BUILTIN += $(IPT_NAT-y)
IPT_BUILTIN += $(IPT_ULOG-y)
