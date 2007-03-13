include $(TOPDIR)/rules.mk

LINUX_VERSION:=2.4.32
LINUX_RELEASE:=1
LINUX_KERNEL_MD5SUM:=38f4d0830e95a20f4bfed17622d5557c

include ../rules.mk
include ./config

include ../generic-$(KERNEL)/modules.mk
include ../kernel.mk

$(LINUX_DIR)/.patched: $(LINUX_DIR)/.unpacked
	mkdir -p $(LINUX_DIR)
	touch $@

$(LINUX_DIR)/.unpacked: 
	touch $@

$(LINUX_DIR)/.depend_done:
	touch $@

$(LINUX_DIR)/.modules_done: 
	touch $@

$(STAMP_DIR)/.linux-compile:
	touch $@

