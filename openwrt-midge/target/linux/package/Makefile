# Main makefile for the packages
include $(TOPDIR)/rules.mk
include ../rules.mk

package-y += base-files

all: compile install
clean: $(patsubst %,%-clean,$(package-) $(package-y) $(package-m))
prepare: $(patsubst %,%-prepare,$(package-y) $(package-m))
compile: $(patsubst %,%-compile,$(package-y) $(package-m))
install: $(patsubst %,%-install,$(package-y))

KPKG_MAKEOPTS += BUILD_DIR="$(LINUX_BUILD_DIR)"

%-prepare:
	@$(START_TRACE) "target/linux/package/$(patsubst %-prepare,%,$@)/prepare: "
	$(MAKE) -C $(patsubst %-prepare,%,$@) \
		$(KPKG_MAKEOPTS) \
		prepare
	@$(CMD_TRACE) " done"
	@$(END_TRACE)

%-compile:
	@$(START_TRACE) "target/linux/package/$(patsubst %-compile,%,$@)-compile: "
	$(MAKE) -C $(patsubst %-compile,%,$@) \
		$(KPKG_MAKEOPTS) \
		compile
	@$(CMD_TRACE) " done"
	@$(END_TRACE)

%-install:
	@$(START_TRACE) "target/linux/package/$(patsubst %-install,%,$@)-install: "
	$(MAKE) -C $(patsubst %-install,%,$@) \
		$(KPKG_MAKEOPTS) \
		install
	@$(CMD_TRACE) " done"
	@$(END_TRACE)

%-clean:
	@$(START_TRACE) "target/linux/package/$(patsubst %-clean,%,$@)-clean: "
	@$(MAKE) -C $(patsubst %-clean,%,$@) \
		$(KPKG_MAKEOPTS) \
		clean
	@$(CMD_TRACE) " done"
	@$(END_TRACE)

