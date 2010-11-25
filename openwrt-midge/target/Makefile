include $(TOPDIR)/rules.mk

all: install

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

linux-compile: utils-install lzma-install
linux-install: $(BIN_DIR)

prepare: linux-prepare
compile: linux-compile
menuconfig: linux-menuconfig
install: image_clean linux-install
clean: linux-clean utils-clean lzma-clean image_clean

image_clean:
	rm -f $(BIN_DIR)/openwrt-*

linux/package/%:
	$(MAKE) -C $(TOPDIR)/target/linux $(patsubst linux/%,%,$@)

%-clean:
	@$(TRACE) target/$(patsubst %-clean,%,$@)-clean
	$(MAKE) -C $(patsubst %-clean,%,$@) clean
%-prepare:
	@$(TRACE) target/$(patsubst %-prepare,%,$@)-prepare
	$(MAKE) -C $(patsubst %-prepare,%,$@) prepare
%-menuconfig: %-prepare
	@$(TRACE) target/$(patsubst %-menuconfig,%,$@)-menuconfig
	$(MAKE) -C $(patsubst %-menuconfig,%,$@) menuconfig
%-compile: %-prepare
	@$(TRACE) target/$(patsubst %-compile,%,$@)-compile
	$(MAKE) -C $(patsubst %-compile,%,$@) compile
%-rebuild:
	@$(TRACE) target/$(patsubst %-rebuild,%,$@)-rebuild
	$(MAKE) -C $(patsubst %-rebuild,%,$@) rebuild
%-install: %-compile
	@$(TRACE) target/$(patsubst %-install,%,$@)-install
	$(MAKE) -C $(patsubst %-install,%,$@) install

