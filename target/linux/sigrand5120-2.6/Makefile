include $(TOPDIR)/rules.mk

LINUX_VERSION:=2.6.16
LINUX_RELEASE:=1
LINUX_KERNEL_MD5SUM:=9a91b2719949ff0856b40bc467fd47be
CONFIG_PATH=$(BUILD_DIR)/linux-$(KERNEL)-$(BOARD)/config-tmp

export LINUX_VERSION

include ../rules.mk

ifeq "$(BR2_LINUX_2_6_SIGRAND5120_NOFW)" "y"
  lk_config = config-nofw
else
  lk_config = config
endif

include ./$(lk_config)

include ../generic-$(KERNEL)/modules.mk
include ../kernel.mk

$(LINUX_DIR)/.patched: $(LINUX_DIR)/.unpacked
	[ -d ../generic-$(KERNEL)/patches ] && $(PATCH) $(LINUX_DIR) ../generic-$(KERNEL)/patches $(MAKE_TRACE)
	[ -d ./patches ] && $(PATCH) $(LINUX_DIR) ./patches $(MAKE_TRACE)
	@touch $@

menuconfig: $(LINUX_DIR)/.patched
	cp $(lk_config) $(LINUX_DIR)/.config
	$(MAKE1) -C $(LINUX_DIR) CROSS_COMPILE="$(KERNEL_CROSS)" ARCH=$(LINUX_KARCH) menuconfig $(MAKE_TRACE)
	cp $(LINUX_DIR)/.config $(lk_config)

$(LINUX_DIR)/.configured: $(LINUX_DIR)/.configured-sigrand5120-2.6
$(LINUX_DIR)/.configured-sigrand5120-2.6: $(LINUX_DIR)/.patched
	@$(CP) $(lk_config) $(LINUX_DIR)/.config
	@touch $@

