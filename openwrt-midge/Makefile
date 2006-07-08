# Makefile for OpenWrt
#
# Copyright (C) 2005 by Felix Fietkau <openwrt@nbd.name>
# Copyright (C) 1999-2004 by Erik Andersen <andersen@codepoet.org>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
#

#--------------------------------------------------------------
# Just run 'make menuconfig', configure stuff, then run 'make'.
# You shouldn't need to mess with anything beyond this point...
#--------------------------------------------------------------
TOPDIR=${shell pwd}
export TOPDIR

ifneq ($(DEVELOPER),)
CONFIG_CONFIG_IN = Config.in.devel
else
CONFIG_CONFIG_IN = Config.in
endif
CONFIG_DEFCONFIG = .defconfig
CONFIG = package/config

noconfig_targets := menuconfig config oldconfig randconfig \
	defconfig allyesconfig allnoconfig tags

# Pull in the user's configuration file
ifeq ($(filter $(noconfig_targets),$(MAKECMDGOALS)),)
-include $(TOPDIR)/.config
endif

ifeq ($(strip $(BR2_HAVE_DOT_CONFIG)),y)
include $(TOPDIR)/rules.mk

all: world

.NOTPARALLEL:
.PHONY: all world clean dirclean distclean image_clean target_clean source configtest

#############################################################
#
# You should probably leave this stuff alone unless you know
# what you are doing.
#
#############################################################

# In this section, we need .config
include .config.cmd

world: $(DL_DIR) $(BUILD_DIR) configtest 
	$(MAKE) toolchain/install target/compile package/compile root_clean package/install target/install package_index
	@$(TRACE) Build complete.

configtest:
	-cp .config .config.test
	-scripts/configtest.pl

package_index:
	(cd $(PACKAGE_DIR); $(STAGING_DIR)/usr/bin/ipkg-make-index . > Packages)

$(DL_DIR):
	@mkdir -p $(DL_DIR)

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

source: toolchain/source package/source target/source

package/%:
	@$(TRACE) $@
	$(MAKE) -C package $(patsubst package/%,%,$@)

target/%:
	@$(TRACE) $@
	$(MAKE) -C target $(patsubst target/%,%,$@)

toolchain/%:
	@$(TRACE) $@
	$(MAKE) -C toolchain $(patsubst toolchain/%,%,$@)

#############################################################
#
# Cleanup and misc junk
#
#############################################################
root_clean:
	@$(TRACE) root_clean
	rm -rf $(BUILD_DIR)/linux-*/root $(BUILD_DIR)/root

target_clean: root_clean
	rm -f $(STAMP_DIR)/.*-compile
	rm -f $(STAMP_DIR)/.*-install
	rm -rf $(BIN_DIR)

clean: dirclean

dirclean:
	@$(TRACE) dirclean
	@$(MAKE) -C $(CONFIG) clean
	rm -rf $(BUILD_DIR)

distclean: dirclean
	rm -rf $(STAMP_DIR) $(DL_DIR) $(TOOL_BUILD_DIR) $(STAGING_DIR)
	rm -f .config* .tmpconfig.h

else # ifeq ($(strip $(BR2_HAVE_DOT_CONFIG)),y)

all: menuconfig

# configuration
# ---------------------------------------------------------------------------

$(CONFIG)/conf:
	$(MAKE) -C $(CONFIG) conf
	-@if [ ! -f .config ] ; then \
		cp $(CONFIG_DEFCONFIG) .config; \
	fi
$(CONFIG)/mconf:
	$(MAKE) -C $(CONFIG) 
	-@if [ ! -f .config ] ; then \
		cp $(CONFIG_DEFCONFIG) .config; \
	fi

menuconfig: $(CONFIG)/mconf
	-touch .config
	-cp .config .config.test
	@$(CONFIG)/mconf $(CONFIG_CONFIG_IN)
	-./scripts/configtest.pl

config: $(CONFIG)/conf
	-touch .config
	-cp .config .config.test
	@$(CONFIG)/conf $(CONFIG_CONFIG_IN)
	-./scripts/configtest.pl

oldconfig: $(CONFIG)/conf
	-touch .config
	-cp .config .config.test
	@$(CONFIG)/conf -o $(CONFIG_CONFIG_IN)
	-./scripts/configtest.pl

randconfig: $(CONFIG)/conf
	-touch .config
	-cp .config .config.test
	@$(CONFIG)/conf -r $(CONFIG_CONFIG_IN)
	-./scripts/configtest.pl

allyesconfig: $(CONFIG)/conf
	-touch .config
	-cp .config .config.test
	@$(CONFIG)/conf -o $(CONFIG_CONFIG_IN)
	-./scripts/configtest.pl

allnoconfig: $(CONFIG)/conf
	-touch .config
	-cp .config .config.test
	@$(CONFIG)/conf -n $(CONFIG_CONFIG_IN)
	-./scripts/configtest.pl

defconfig: $(CONFIG)/conf
	-touch .config
	-cp .config .config.test
	@$(CONFIG)/conf -d $(CONFIG_CONFIG_IN)
	-./scripts/configtest.pl

endif # ifeq ($(strip $(BR2_HAVE_DOT_CONFIG)),y)
