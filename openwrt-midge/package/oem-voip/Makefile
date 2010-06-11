#
# Copyright (C) 2006 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
# $Id$

include $(TOPDIR)/rules.mk

PKG_NAME:=oem-voip
PKG_VERSION:=0.1
PKG_RELEASE:=1

PKG_TAPI_DISTR:=drv_tapi-3.6.1.tar.gz
PKG_DAA_DISTR:=drv_daa-1.0.2.0.tar.gz
PKG_VINETIC_DISTR:=drv_vinetic-1.3.1_tapi-3.6.1.tar.gz
PKG_SGATAB_DISTR:=drv_sgatab
PKG_SVD_DISTR:=svd
PKG_SVI_DISTR:=svi
PKG_SVC_DISTR:=svc
PKG_LIBAB_DISTR:=libab
PKG_TST_DISTR:=tst

PKG_CONF_DIR:=$(TOPDIR)/package/$(PKG_NAME)
PKG_SRC_DIR:=$(PKG_CONF_DIR)/files/src/
PKG_PATCH_DIR:=$(PKG_CONF_DIR)/files/patches/
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/build

include $(TOPDIR)/package/target-configure-staging-flags.mk

include $(TOPDIR)/package/rules.mk


TAPI_UNPACK+=tar -vxpf $(PKG_SRC_DIR)/$(PKG_TAPI_DISTR) -C $(PKG_BUILD_DIR)
DAA_UNPACK+=tar -vxpf $(PKG_SRC_DIR)/$(PKG_DAA_DISTR) -C $(PKG_BUILD_DIR)
VINETIC_UNPACK+=tar -vxpf $(PKG_SRC_DIR)/$(PKG_VINETIC_DISTR) -C $(PKG_BUILD_DIR)
SGATAB_UNPACK+=cp -rfp $(PKG_SRC_DIR)/$(PKG_SGATAB_DISTR) $(PKG_BUILD_DIR)
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

	$(TAPI_UNPACK)
	$(DAA_UNPACK)
	$(VINETIC_UNPACK)
	$(SGATAB_UNPACK)
	$(SVD_UNPACK)
	$(SVI_UNPACK)
	$(SVC_UNPACK)
	$(LIBAB_UNPACK)
	$(TST_UNPACK)

	#---------======================================---------
	# 			prepare sources
	#---------======================================---------

	mkdir -p $(PKG_INSTALL_DIR)

	ln -snf $(PKG_BUILD_DIR)/drv_daa-1.0.2.0 $(PKG_BUILD_DIR)/daa
	ln -snf $(PKG_BUILD_DIR)/drv_tapi-3.6.1 $(PKG_BUILD_DIR)/tapi
	ln -snf $(PKG_BUILD_DIR)/drv_vinetic-1.3.1_tapi-3.6.1 $(PKG_BUILD_DIR)/vinetic
	ln -snf $(PKG_BUILD_DIR)/drv_sgatab $(PKG_BUILD_DIR)/sgatab
	ln -snf $(LINUX_DIR) $(PKG_BUILD_DIR)/linux

	cd $(pbd)/daa		&& patch -p1 < $(PKG_PATCH_DIR)/daa.patch
	cd $(pbd)/tapi		&& patch -p1 < $(PKG_PATCH_DIR)/tapi.patch
	cd $(pbd)/vinetic	&& patch -p1 < $(PKG_PATCH_DIR)/vinetic.patch

	touch $@



$(PKG_BUILD_DIR)/.configured:  $(PKG_BUILD_DIR)/.prepared
	#---------======================================---------
	# 		configure and build drivers stack
	#---------======================================---------

	@echo
	@echo "CONFIGURE AND BUILD DAA..."
	cd $(pbd)/daa && aclocal
	cd $(pbd)/daa && autoheader
	cd $(pbd)/daa && automake -a --foreign
	cd $(pbd)/daa && autoconf
	cd $(pbd)/daa && \
		$(TARGET_CONFIGURE_OPTS) \
		./configure \
		--host=$(GNU_TARGET_NAME) \
		--build=$(GNU_HOST_NAME) \
		--disable-apoh \
		--disable-duslicincl \
		--disable-dxtincl \
		--enable-vineticincl=$(PKG_BUILD_DIR)/vinetic/include \
		--enable-tapiincl=$(PKG_BUILD_DIR)/tapi/include \
		--enable-kernelincl=$(PKG_BUILD_DIR)/linux/include \
		--enable-boardname=SG \
		--prefix=$(PKG_INSTALL_DIR)
	cd $(pbd)/daa && $(TARGET_CONFIGURE_OPTS) make
	cd $(pbd)/daa && $(TARGET_CONFIGURE_OPTS) make install

	@echo
	@echo "CONFIGURE AND BUILD VINETIC..."
	cd $(pbd)/vinetic && aclocal
	cd $(pbd)/vinetic && autoheader
	cd $(pbd)/vinetic && automake -a --foreign
	cd $(pbd)/vinetic && autoconf
	cd $(pbd)/vinetic && \
		$(TARGET_CONFIGURE_OPTS) \
		./configure \
		--host=$(GNU_TARGET_NAME) \
		--build=$(GNU_HOST_NAME) \
		--enable-linux-26 \
		--enable-trace \
		--enable-debug \
		--enable-2cpe \
		--enable-lt \
		--enable-fax \
		--disable-v1 \
		--with-max-devices=32 \
		--with-access-mode=INTEL_MUX \
		--with-access-width=8 \
		--enable-tapiincl=$(PKG_BUILD_DIR)/tapi/include \
		--enable-kernelincl=$(PKG_BUILD_DIR)/linux/include \
		--prefix=$(PKG_INSTALL_DIR)
	cd $(pbd)/vinetic && $(TARGET_CONFIGURE_OPTS) make
	cd $(pbd)/vinetic && $(TARGET_CONFIGURE_OPTS) make install

	@echo
	@echo "CONFIGURE AND BUILD TAPI..."
	cd $(pbd)/tapi && aclocal
	cd $(pbd)/tapi && autoheader
	cd $(pbd)/tapi && automake -a --foreign
	cd $(pbd)/tapi && autoconf
	cd $(pbd)/tapi && \
		$(TARGET_CONFIGURE_OPTS) \
		./configure \
		--host=$(GNU_TARGET_NAME) \
		--build=$(GNU_HOST_NAME) \
		--enable-linux-26 \
		--enable-trace \
		--enable-debug \
		--enable-voice \
		--enable-dtmf \
		--enable-cid \
		--enable-lt \
		--enable-dect \
		--enable-fax \
		--enable-extkeypad \
		--enable-kernelincl=$(PKG_BUILD_DIR)/linux/include \
		--prefix=$(PKG_INSTALL_DIR)
	cd $(pbd)/tapi && $(TARGET_CONFIGURE_OPTS) make
	cd $(pbd)/tapi && $(TARGET_CONFIGURE_OPTS) make install

	@echo
	@echo "CONFIGURE AND BUILD SGATAB..."
	cd $(pbd)/sgatab && $(TARGET_CONFIGURE_OPTS) \
		CC="$(TARGET_CC)" make  -w -C $(pbd)/linux M=$(pbd)/sgatab modules
	$(CP) $(pbd)/sgatab/drv_sgatab.ko $(PKG_INSTALL_DIR)/bin

	touch $@



$(PKG_BUILD_DIR)/.built:  $(PKG_BUILD_DIR)/.configured
	#---------======================================---------
	#		configure and build routines
	#---------======================================---------

	@echo
	@echo "CONFIGURE AND BUILD SVI..."
	cd $(pbd)/svi && $(TARGET_CONFIGURE_OPTS) \
		$(TARGET_CC) $(TARGET_CFLAGS) \
		-I$(PKG_BUILD_DIR)/vinetic/include/ \
		-I$(PKG_BUILD_DIR)/tapi/include/ \
		-I$(PKG_BUILD_DIR)/sgatab/ \
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
		-I./vinetic/include/ \
		-I./tapi/include/ \
		-I./sgatab/ \
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
		-I../vinetic/include/ \
		-I../tapi/include/ \
		-I../drv_sgatab/ \
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
		-I../vinetic/include/ \
		-I../tapi/include/ \
		-I../drv_sgatab/ \
		-I../libab \
		-L../libab \
		etst.c \
		-o etst \
		-lab
	cd $(pbd)/tst && $(TARGET_CONFIGURE_OPTS) \
		$(TARGET_CC) $(TARGET_CFLAGS) \
		-Wall \
		-I../vinetic/include/ \
		-I../tapi/include/ \
		-I../drv_sgatab/ \
		-I../libab \
		-L../libab \
		vtst.c \
		-o vtst \
		-lab
	cd $(pbd)/tst && $(TARGET_CONFIGURE_OPTS) \
		$(TARGET_CC) $(TARGET_CFLAGS) \
		-Wall \
		-I../vinetic/include/ \
		-I../tapi/include/ \
		-I../drv_sgatab/ \
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
	cp -rpf $(PKG_INSTALL_DIR)/bin/*.ko $(IDIR_OEM_VOIP)/lib/modules/2.6.16/
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
