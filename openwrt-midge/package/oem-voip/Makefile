include $(TOPDIR)/rules.mk

PKG_NAME:=oem-voip
PKG_VERSION:=0.1
PKG_RELEASE:=1

PKG_SVD_DISTR:=svd
PKG_SVI_DISTR:=svi
PKG_SVC_DISTR:=svc
PKG_LIBAB_DISTR:=libab
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

include $(TOPDIR)/package/rules.mk


SVD_UNPACK+=cp -rfp $(PKG_SRC_DIR)/$(PKG_SVD_DISTR) $(PKG_BUILD_DIR)
SVI_UNPACK+=cp -rfp $(PKG_SRC_DIR)/$(PKG_SVI_DISTR) $(PKG_BUILD_DIR)
SVC_UNPACK+=cp -rfp $(PKG_SRC_DIR)/$(PKG_SVC_DISTR) $(PKG_BUILD_DIR)
LIBAB_UNPACK+=cp -rfp $(PKG_SRC_DIR)/$(PKG_LIBAB_DISTR) $(PKG_BUILD_DIR)
TST_UNPACK+=cp -rfp $(PKG_SRC_DIR)/$(PKG_TST_DISTR) $(PKG_BUILD_DIR)


$(eval $(call PKG_template,OEM_VOIP,oem-voip,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))

pbd = $(PKG_BUILD_DIR)



$(PKG_BUILD_DIR)/.prepared:

	mkdir -p $(PKG_BUILD_DIR);

	#---------======================================---------
	# 			unpack sources
	#---------======================================---------

	$(SVD_UNPACK)
	$(SVI_UNPACK)
	$(SVC_UNPACK)
	$(LIBAB_UNPACK)
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
	@echo "CONFIGURE SVD..."
	cd $(pbd)/svd && aclocal
	cd $(pbd)/svd && autoheader
	cd $(pbd)/svd && automake -a --foreign
	cd $(pbd)/svd && autoconf
	cd $(pbd)/svd && $(TARGET_CONFIGURE_OPTS) \
		./configure \
		--host=$(GNU_TARGET_NAME) \
		--build=$(GNU_HOST_NAME) \
		--prefix=$(PKG_INSTALL_DIR)

	@echo
	@echo "BUILD LIBAB..."
	cd $(pbd)/libab && rm -f *.o *.a
	cd $(pbd)/libab && $(TARGET_CONFIGURE_OPTS) \
		$(TARGET_CC) $(TARGET_CFLAGS) \
		-Wall \
		ab_hwinit.c \
		ab_basic.c \
		ab_line.c \
		ab_events.c \
		ab_media.c \
		-c
	cd $(pbd)/libab && $(AR) cr libab.a *.o

	@echo
	@echo "BUILD SVD..."
	cd $(pbd)/svd && $(TARGET_CONFIGURE_OPTS) make
	cd $(pbd)/svd && $(TARGET_CONFIGURE_OPTS) make install

	@echo
	@echo "CONFIGURE AND BUILD SVC..."
	cd $(pbd)/svc && $(TARGET_CONFIGURE_OPTS) \
		$(TARGET_CC) $(TARGET_CFLAGS) \
		-Wall \
		-I../libab \
		-L../libab \
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
		-I../libab \
		-L../libab \
		etst.c \
		-o etst \
		-lab
	cd $(pbd)/tst && $(TARGET_CONFIGURE_OPTS) \
		$(TARGET_CC) $(TARGET_CFLAGS) \
		-Wall \
		-I../libab \
		-L../libab \
		vtst.c \
		-o vtst \
		-lab
	cd $(pbd)/tst && $(TARGET_CONFIGURE_OPTS) \
		$(TARGET_CC) $(TARGET_CFLAGS) \
		-Wall \
		-I../libab \
		-L../libab \
		iotst.c \
		-o iotst \
		-lab
	cd $(pbd)/tst && ${CP} *tst $(PKG_INSTALL_DIR)/bin/
endif # TST

	touch $@


$(IPKG_OEM_VOIP):
	install -d -m0755 $(IDIR_OEM_VOIP)/lib/modules/2.6.16/
	install -d -m0755 $(IDIR_OEM_VOIP)/bin/
	install -d -m0755 $(IDIR_OEM_VOIP)/sbin/
	install -d -m0755 $(IDIR_OEM_VOIP)/lib/firmware/
	install -d -m0755 $(IDIR_OEM_VOIP)/etc/init.d/

	#FIXME: these *.ko must be installed in appropriate drv-* packages
	cp -rpf $(BUILD_DIR)/daa/install/bin/*.ko     $(IDIR_OEM_VOIP)/lib/modules/2.6.16/
	cp -rpf $(BUILD_DIR)/tapi/install/bin/*.ko    $(IDIR_OEM_VOIP)/lib/modules/2.6.16/
	cp -rpf $(BUILD_DIR)/vinetic/install/bin/*.ko $(IDIR_OEM_VOIP)/lib/modules/2.6.16/
	cp -rpf $(BUILD_DIR)/sgatab/install/bin/*.ko  $(IDIR_OEM_VOIP)/lib/modules/2.6.16/
	cp -rpf $(PKG_INSTALL_DIR)/bin/sv* $(IDIR_OEM_VOIP)/bin/

	#cp -rpf $(PKG_INSTALL_DIR)/bin/*tst $(IDIR_OEM_VOIP)/bin/
	cp -rpf $(PKG_CONF_DIR)/files/fw/* $(IDIR_OEM_VOIP)/lib/firmware/
	cp -rpf $(PKG_CONF_DIR)/files/sbin/* $(IDIR_OEM_VOIP)/sbin/
	cp -rpf $(PKG_CONF_DIR)/files/rc/* $(IDIR_OEM_VOIP)/etc/init.d/
	echo "Depends: $(PKG_DEPEND)" >> $(IDIR_OEM_VOIP)/CONTROL/control
	$(RSTRIP) $(IDIR_OEM_VOIP)/bin/sv*
	#$(RSTRIP) $(IDIR_OEM_VOIP)/bin/*tst
	$(IPKG_BUILD) $(IDIR_OEM_VOIP) $(PACKAGE_DIR)
	#---------============ IPKG_OEM_VOIP ===========---------
	#---------- make ipkg
	#---------======================================---------
