include $(TOPDIR)/rules.mk

PKG_NAME:=voip
PKG_VERSION:=0.1
PKG_RELEASE:=1

PKG_SVI_DISTR:=svi
PKG_SVC_DISTR:=svc
PKG_TST_DISTR:=tst

PKG_CONF_DIR:=$(TOPDIR)/package/$(PKG_NAME)
PKG_SRC_DIR:=$(PKG_CONF_DIR)/files/src/
PKG_PATCH_DIR:=$(PKG_CONF_DIR)/files/patches/
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_INSTALL_DIR = $(PKG_BUILD_DIR)/install

include $(TOPDIR)/package/target-configure-staging-flags.mk
TARGET_CFLAGS += -I$(BUILD_DIR)/tapi/include
TARGET_CFLAGS += -I$(BUILD_DIR)/vinetic/include
TARGET_CFLAGS += -I$(BUILD_DIR)/sgatab
TARGET_CFLAGS += -I$(BUILD_DIR)/libab
TARGET_CFLAGS += -L$(BUILD_DIR)/libab

include $(TOPDIR)/package/rules.mk


SVI_UNPACK+=cp -rfp $(PKG_SRC_DIR)/$(PKG_SVI_DISTR) $(PKG_BUILD_DIR)
SVC_UNPACK+=cp -rfp $(PKG_SRC_DIR)/$(PKG_SVC_DISTR) $(PKG_BUILD_DIR)
TST_UNPACK+=cp -rfp $(PKG_SRC_DIR)/$(PKG_TST_DISTR) $(PKG_BUILD_DIR)


$(eval $(call PKG_template,VOIP,voip,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))

pbd = $(PKG_BUILD_DIR)



$(PKG_BUILD_DIR)/.prepared:

	mkdir -p $(PKG_BUILD_DIR);

	#---------======================================---------
	# 			unpack sources
	#---------======================================---------

	$(SVI_UNPACK)
	$(SVC_UNPACK)
	$(TST_UNPACK)

	#---------======================================---------
	# 			prepare sources
	#---------======================================---------

	mkdir -p $(PKG_INSTALL_DIR)/bin

	ln -snf $(LINUX_DIR) $(PKG_BUILD_DIR)/linux

	touch $@



$(PKG_BUILD_DIR)/.configured:  $(PKG_BUILD_DIR)/.prepared
	#---------======================================---------
	# 		configure and build drivers stack
	#---------======================================---------

	touch $@



$(PKG_BUILD_DIR)/.built:  $(PKG_BUILD_DIR)/.configured
	#---------======================================---------
	#		configure and build routines
	#---------======================================---------

	@echo
	@echo "CONFIGURE AND BUILD SVI..."
	cd $(pbd)/svi && $(TARGET_CONFIGURE_OPTS) \
		$(TARGET_CC) $(TARGET_CFLAGS) \
		svi.c -o svi \
		-lconfig -static
	cd $(pbd)/svi && cp svi $(PKG_INSTALL_DIR)/bin/

	@echo
	@echo "CONFIGURE AND BUILD SVC..."
	cd $(pbd)/svc && $(TARGET_CONFIGURE_OPTS) \
		$(TARGET_CC) $(TARGET_CFLAGS) \
		-Wall \
		svc.c \
		-o svc \
		-lab
	cd $(pbd)/svc && cp svc $(PKG_INSTALL_DIR)/bin/

ifeq "0" "1"
	@echo
	@echo "BUILD ?TST..."
	cd $(pbd)/tst && $(TARGET_CONFIGURE_OPTS) \
		$(TARGET_CC) $(TARGET_CFLAGS) \
		-Wall \
		etst.c \
		-o etst \
		-lab
	cd $(pbd)/tst && $(TARGET_CONFIGURE_OPTS) \
		$(TARGET_CC) $(TARGET_CFLAGS) \
		-Wall \
		vtst.c \
		-o vtst \
		-lab
	cd $(pbd)/tst && $(TARGET_CONFIGURE_OPTS) \
		$(TARGET_CC) $(TARGET_CFLAGS) \
		-Wall \
		iotst.c \
		-o iotst \
		-lab
	cd $(pbd)/tst && ${CP} *tst $(PKG_INSTALL_DIR)/bin/
endif # TST

	touch $@


$(IPKG_VOIP):
	install -d -m0755 $(IDIR_VOIP)/lib/modules/2.6.16/
	install -d -m0755 $(IDIR_VOIP)/bin/
	install -d -m0755 $(IDIR_VOIP)/sbin/
	install -d -m0755 $(IDIR_VOIP)/lib/firmware/
	install -d -m0755 $(IDIR_VOIP)/etc/init.d/

	#FIXME: these *.ko must be installed in appropriate drv-* packages
	cp -rpf $(BUILD_DIR)/daa/install/bin/*.ko     $(IDIR_VOIP)/lib/modules/2.6.16/
	cp -rpf $(BUILD_DIR)/tapi/install/bin/*.ko    $(IDIR_VOIP)/lib/modules/2.6.16/
	cp -rpf $(BUILD_DIR)/vinetic/install/bin/*.ko $(IDIR_VOIP)/lib/modules/2.6.16/
	cp -rpf $(BUILD_DIR)/sgatab/install/bin/*.ko  $(IDIR_VOIP)/lib/modules/2.6.16/
	cp -rpf $(BUILD_DIR)/svd/ipkg-install/bin/sv* $(IDIR_VOIP)/bin/
	cp -rpf $(PKG_INSTALL_DIR)/bin/sv* $(IDIR_VOIP)/bin/

	#cp -rpf $(PKG_INSTALL_DIR)/bin/*tst $(IDIR_VOIP)/bin/
	cp -rpf $(PKG_CONF_DIR)/files/fw/* $(IDIR_VOIP)/lib/firmware/
	#FIXME: these sbin/* must be installed in svd package
	cp -rpf $(PKG_CONF_DIR)/files/sbin/* $(IDIR_VOIP)/sbin/
	cp -rpf $(PKG_CONF_DIR)/files/rc/* $(IDIR_VOIP)/etc/init.d/
	$(RSTRIP) $(IDIR_VOIP)/bin/sv*
	#$(RSTRIP) $(IDIR_VOIP)/bin/*tst
	$(IPKG_BUILD) $(IDIR_VOIP) $(PACKAGE_DIR)
	#---------============ IPKG_VOIP ===========---------
	#---------- make ipkg
	#---------======================================---------
