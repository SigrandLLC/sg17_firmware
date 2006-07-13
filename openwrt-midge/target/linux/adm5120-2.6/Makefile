include $(TOPDIR)/rules.mk

LINUX_VERSION:=2.6.16
LINUX_RELEASE:=1
LINUX_KERNEL_MD5SUM:=9a91b2719949ff0856b40bc467fd47be
CONFIG_PATH=$(BUILD_DIR)/linux-$(KERNEL)-$(BOARD)/config-tmp


include ../rules.mk
include ./config
include ../generic-$(KERNEL)/modules.mk
include ../kernel.mk

$(LINUX_DIR)/.patched: $(LINUX_DIR)/.unpacked
	[ -d ../generic-$(KERNEL)/patches ] && $(PATCH) $(LINUX_DIR) ../generic-$(KERNEL)/patches $(MAKE_TRACE)
	[ -d ./patches ] && $(PATCH) $(LINUX_DIR) ./patches $(MAKE_TRACE)
	touch $@


adm5120-kern-configured:
	/bin/sh ./scripts/add_kcmdline config $(BR2_KERNEL_CMDLINE) $(CONFIG_PATH)
	@cp  $(CONFIG_PATH) $(LINUX_DIR)/.config
	rm --force  $(CONFIG_PATH)



$(LINUX_DIR)/.unpacked: 
$(LINUX_DIR)/.depend_done: 
$(LINUX_DIR)/.modules_done: 
$(STAMP_DIR)/.linux-compile: $(LINUX_DIR)/.modules_done


#$(LINUX_BUILD_DIR)/vmlinuz: $(LINUX_DIR)/vmlinux
#	cp  $(LINUX_DIR)/vmlinuz $@
#	touch $@
#
#compile: $(LINUX_BUILD_DIR)/vmlinuz
