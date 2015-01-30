include $(TOPDIR)/rules.mk

PKG_NAME = rs232-tcpdmx
PKG_VERSION = 1
PKG_RELEASE = 1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)
#PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/ipkg-install

include $(TOPDIR)/package/rules.mk

$(eval $(call PKG_template,RS232_TCPDMX,rs232-tcpdmx,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))

$(PKG_BUILD_DIR)/.prepared :
	mkdir -p      $(PKG_BUILD_DIR)
	#$(CP) ./src/* $(PKG_BUILD_DIR)
	touch $@


cflags = "$(TARGET_CFLAGS) -I $(STAGING_DIR)/usr/include"
ldflags = "-L $(STAGING_DIR)/usr/lib"

local_mk = $(PKG_BUILD_DIR)/local.mk

define CONFIGURE_CMD
  echo "TOP_DIR      = $(PWD)/src"                 > $(local_mk)
  echo "SRC_DIR      = $(PWD)/src"                >> $(local_mk)
  echo "BLD_DIR      = $(PKG_BUILD_DIR)"          >> $(local_mk)
  echo " CFLAGS      = $(cflags)"                 >> $(local_mk)
  echo "LDFLAGS      = $(ldflags)"                >> $(local_mk)

  ln -s $(PWD)/src/Makefile   $(PKG_BUILD_DIR)
  ln -s $(PWD)/src/defs.make  $(PKG_BUILD_DIR)
  ln -s $(PWD)/src/rules.make $(PKG_BUILD_DIR)
endef

$(PKG_BUILD_DIR)/.configured :
	$(CONFIGURE_CMD)
	@for i in $(TARGET_CONFIGURE_OPTS); do \
		echo "$$i"		>> $(local_mk); \
	done
	touch $@

$(PKG_BUILD_DIR)/.built : $(PKG_BUILD_DIR)/.configured
	$(MAKE) -C $(PKG_BUILD_DIR)
	touch $@

instx = install -m0755
$(IPKG_RS232_TCPDMX) :
	@mkdir -p                               $(IDIR_RS232_TCPDMX)/usr/sbin
	@$(instx) $(PKG_BUILD_DIR)/rs232-tcpdmx $(IDIR_RS232_TCPDMX)/usr/sbin
	@mkdir -p                               $(IDIR_RS232_TCPDMX)/etc/init.d
	@$(instx)     ./etc/init.d/rs232_tcpdmx $(IDIR_RS232_TCPDMX)/etc/init.d/
	@ln -sf                    rs232_tcpdmx $(IDIR_RS232_TCPDMX)/etc/init.d/S70rs232_tcpdmx
	@ln -sf                    rs232_tcpdmx $(IDIR_RS232_TCPDMX)/etc/init.d/K70rs232_tcpdmx
	@$(RSTRIP)                              $(IDIR_RS232_TCPDMX)
	@$(IPKG_BUILD)                          $(IDIR_RS232_TCPDMX) $(PACKAGE_DIR)
