include $(TOPDIR)/rules.mk

PKG_OS:=$(shell uname -s)
PKG_CPU:=$(shell uname -m)

IB_NAME:=OpenWrt-ImageBuilder-$(PKG_OS)-$(PKG_CPU)
IB_DIR:=$(BUILD_DIR)/$(IB_NAME)

define kernel_template

$(1)/$(2)-clean:
	@$(TRACE) target/linux/$(2)-$(1)-clean
	$(MAKE) -C $(2)-$(1) BOARD="$(2)" clean

$(1)/$(2)-prepare:
	@$(TRACE) target/linux/$(2)-$(1)-prepare
	$(MAKE) -C $(2)-$(1) BOARD="$(2)" prepare

$(1)/$(2)-compile:
	@$(TRACE) target/linux/$(2)-$(1)-compile
	$(MAKE) -C $(2)-$(1) BOARD="$(2)" compile

$(1)/$(2)-rebuild:
	@$(TRACE) target/linux/$(2)-$(1)-rebuild
	$(MAKE) -C $(2)-$(1) BOARD="$(2)" rebuild

$(1)/$(2)-install:
	@$(TRACE) target/linux/$(2)-$(1)-install
	$(MAKE) -C $(2)-$(1) BOARD="$(2)" install
	@echo "INSTALL: $(BUILD_DIR)/linux-$(1)-$(2)/root/lib/modules/$(1)*"
	rm -f $(BUILD_DIR)/linux-$(1)-$(2)/root/lib/modules/$(1)*/build
	rm -f $(BUILD_DIR)/linux-$(1)-$(2)/root/lib/modules/$(1)*/source

$(1)/$(2)-image: $(1)/$(2)-install
	@$(TRACE) target/linux/image-install
	$(MAKE) -C image KERNEL="$(1)" BOARD="$(2)" install

$(1)/$(2)-install-ib:
	@$(TRACE) target/linux/image-install-ib
	$(MAKE) -C image KERNEL="$(1)" BOARD="$(2)" IB_DIR="$(IB_DIR)" install-ib
	echo '$$$$(eval $$$$(call image_template,$(2),$(1)))' >> $(IB_DIR)/kernel.mk


ifeq ($(BR2_LINUX_$(3)),y)
clean: $(1)/$(2)-clean
prepare: $(1)/$(2)-prepare
compile: $(1)/$(2)-compile
rebuild: $(1)/$(2)-rebuild
$(BIN_DIR)/$(IB_NAME).tar.bz2 install: $(1)/$(2)-image
install-ib: $(1)/$(2)-install-ib
package/%:
	$(MAKE) -C $(2)-$(1) BOARD="$(2)" $$@
endif

.PHONY: $(1)/$(2)-clean $(1)/$(2)-prepare $(1)/$(2)-compile $(1)/$(2)-rebuild $(1)/$(2)-install $(1)/$(2)-image $(1)/$(2)-install-ib
endef

.PHONY: clean prepare compile rebuild install install-ib

menuconfig:
	@$(TRACE) target/linux-menuconfig
	$(MAKE) -C sigrand5120-2.6 menuconfig
prepare:
	@$(TRACE) target/linux/image-prepare
	$(MAKE) -C image prepare
compile:
	@$(TRACE) target/linux/image-compile
	$(MAKE) -C image compile
install:
rebuild:
clean:

$(eval $(call kernel_template,2.6,sigrand5120,2_6_SIGRAND5120))
#$(eval $(call kernel_template,2.6,adm5120,2_6_ADM5120))
