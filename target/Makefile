include $(TOPDIR)/rules.mk

all: install

$(BIN_DIR):
	@mkdir -p $(BIN_DIR)

linux-compile: utils-install lzma-install
linux-install: $(BIN_DIR)

prepare: linux-prepare
compile: linux-compile
menuconfig: linux-menuconfig
install: image_clean linux-install
clean: linux-clean utils-clean lzma-clean image_clean

image_clean:
	@rm -f $(BIN_DIR)/openwrt-*

linux/package/%:
	@$(MAKE) -C $(TOPDIR)/target/linux $(patsubst linux/%,%,$@) $(MAKE_TRACE)

%-clean:
	@$(TRACE) target/$(patsubst %-clean,%,$@)-clean
	@$(MAKE) -C $(patsubst %-clean,%,$@) clean $(MAKE_TRACE)
%-prepare:
	@$(TRACE) target/$(patsubst %-prepare,%,$@)-prepare
	@$(MAKE1) -C $(patsubst %-prepare,%,$@) prepare $(MAKE_TRACE)
%-menuconfig: %-prepare
	@$(TRACE) target/$(patsubst %-menuconfig,%,$@)-menuconfig
	@$(MAKE1) -C $(patsubst %-menuconfig,%,$@) menuconfig $(MAKE_TRACE)
%-compile: %-prepare
	@$(TRACE) target/$(patsubst %-compile,%,$@)-compile
	@$(MAKE) -C $(patsubst %-compile,%,$@) compile $(MAKE_TRACE)
%-rebuild:
	@$(TRACE) target/$(patsubst %-rebuild,%,$@)-rebuild
	@$(MAKE) -C $(patsubst %-rebuild,%,$@) rebuild $(MAKE_TRACE)
%-install: %-compile
	@$(TRACE) target/$(patsubst %-install,%,$@)-install
	@$(MAKE) -C $(patsubst %-install,%,$@) install $(MAKE_TRACE)
