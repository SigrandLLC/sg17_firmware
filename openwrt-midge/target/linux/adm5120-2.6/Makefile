include $(TOPDIR)/rules.mk

LINUX_VERSION:=2.6.16
LINUX_RELEASE:=1
LINUX_KERNEL_MD5SUM:=9a91b2719949ff0856b40bc467fd47be
CONFIG_PATH=$(BUILD_DIR)/linux-$(KERNEL)-$(BOARD)/config-tmp


include ../rules.mk
include ./config
include ../generic-$(KERNEL)/modules.mk
include ../kernel.mk

$(LINUX_DIR)/.internal_kern:
	rm -Rf $(LINUX_BUILD_DIR)
	$(MAKE) $(LINUX_DIR)/.unpacked
	touch $@

remove_old: $(LINUX_BUILD_DIR)
	rm -Rf $(LINUX_BUILD_DIR)


ifeq ($(BR2_EXTERNAL_KERN_PATH),)
$(LINUX_DIR)/.patched: $(LINUX_DIR)/.internal_kern
	[ -d ../generic-$(KERNEL)/patches ] && $(PATCH) $(LINUX_DIR) ../generic-$(KERNEL)/patches $(MAKE_TRACE)
	[ -d ./patches ] && $(PATCH) $(LINUX_DIR) ./patches $(MAKE_TRACE)
	touch $@
else
$(LINUX_DIR)/.patched: remove_old
	mkdir -p $(LINUX_BUILD_DIR)
	ln -s $(BR2_EXTERNAL_KERN_PATH) $(LINUX_BUILD_DIR)/linux-$(LINUX_VERSION)
	rm -f $(BUILD_DIR)/linux
	touch $(LINUX_DIR)/.modules_done
	touch $(STAMP_DIR)/.linux-compile
	echo
	echo "*****************************************************" > /dev/null
	echo "Check that vmlinux is present" > /dev/null
	echo "Full path $(LINUX_DIR)/vmlinux" >  /dev/null
	echo
	if test -f "$(LINUX_DIR)/vmlinux" ; then touch "$(LINUX_DIR)/vmlinux"; fi
	ln -s $(LINUX_BUILD_DIR)/linux-$(LINUX_VERSION) $(BUILD_DIR)/linux
	if test ! -f "$(LINUX_DIR)/.config" ; then make -C $(LINUX_DIR) menuconfig; fi
	touch $@
endif


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
