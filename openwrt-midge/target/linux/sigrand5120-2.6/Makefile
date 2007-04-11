include $(TOPDIR)/rules.mk

LINUX_VERSION:=2.6.16
LINUX_RELEASE:=1
LINUX_KERNEL_MD5SUM:=9a91b2719949ff0856b40bc467fd47be
CONFIG_PATH=$(BUILD_DIR)/linux-$(KERNEL)-$(BOARD)/config-tmp


include ../rules.mk
include ./config

include ../generic-$(KERNEL)/modules.mk
include ../kernel.mk

$(eval $(call KMOD_template,ADM5120_SW,adm5120-sw,\
	$(MODULES_DIR)/kernel/drivers/net/adm5120sw.o \
))

$(eval $(call KMOD_template,SIGRAND_SG16LAN,sg16lan,\
	$(MODULES_DIR)/kernel/drivers/net/sg16lan/sg16lan.ko \
))

$(eval $(call KMOD_template,SIGRAND_SG17LAN,sg17lan,\
	$(MODULES_DIR)/kernel/drivers/net/sg17lan/sg17lan.ko \
))

$(eval $(call KMOD_template,SIGRAND_MR16G,mr16g-hdlc,\
	$(MODULES_DIR)/kernel/drivers/net/wan/mr16g/mr16g_hdlc.ko \
))

$(LINUX_DIR)/.patched: $(LINUX_DIR)/.unpacked
	[ -d ../generic-$(KERNEL)/patches ] && $(PATCH) $(LINUX_DIR) ../generic-$(KERNEL)/patches $(MAKE_TRACE)
	[ -d ./patches ] && $(PATCH) $(LINUX_DIR) ./patches $(MAKE_TRACE)
	@$(CP) config $(LINUX_DIR)/.config
	touch $@


