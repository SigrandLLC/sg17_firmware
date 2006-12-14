include ../netfilter.mk

# Networking

$(eval $(call KMOD_template,GRE,gre,\
	$(MODULES_DIR)/kernel/net/ipv4/ip_gre.ko \
,CONFIG_NET_IPGRE,,54,ip_gre))

$(eval $(call KMOD_template,BONDING,bonding,\
	$(MODULES_DIR)/kernel/drivers/net/bonding/bonding.ko \
,CONFIG_BONDING,,20,bonding))




