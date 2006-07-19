include $(TOPDIR)/rules.mk

KDIR:=$(BUILD_DIR)/linux-$(KERNEL)-$(BOARD)

REV:=$(shell LANG=C svn info | awk '/^Revision:/ { print$$2 }' )
ifeq ($(REV),)
REV:=0
endif


export PADFILE=$(STAGING_DIR)/bin/padfile
export MKSYSHDR=$(STAGING_DIR)/bin/mksyshdr

$(SCRIPT_DIR)/mksyshdr:
	$(HOSTCC) src/mksyshdr.c -o $(MKSYSHDR)

$(SCRIPT_DIR)/padfile:
	$(HOSTCC) src/padfile.c -o $(PADFILE)


ifeq ($(KERNEL),2.4)
FSNAME:=$(patsubst jffs2-%,jffs2,$(FS))

ifeq ($(FS),squashfs)
adm5120-fs-prepare:

$(BIN_DIR)/openwrt-adm5120-$(KERNEL)-$(FSNAME).bin: $(SCRIPT_DIR)/mksyshdr $(SCRIPT_DIR)/padfile
	rm -f $(KDIR)/root.squashfs
	mkdir -p $(KDIR)/root/ram
	( cd $(KDIR)/root/ && rm -rf var tmp sbin/mount_root && ln -s ram/tmp tmp )
	( cd $(KDIR)/root/etc && ( rm -rf firewall.user functions.sh inittab \
		init.d/rcS init.d/S10* init.d/S40network init.d/S50dnsmasq init.d/S45firewall init.d/S50httpd \
		init.d/S50telnet init.d/S60cron init.d/S99done \
		 nvram.sh preinit resolv.conf sysconf dnsmasq.conf ) )
	( cd base-files; tar cf - --exclude .svn . | tar xvf - -C $(KDIR)/root/ )
	( \
		MIDGE_TYPE=tmpfs \
	 	MIDGE_KERNEL_BLOCKS=$(MIDGE_KERNEL_BLOCKS) \
		MIDGE_SQUASHFS_BLOCKS=$(MIDGE_SQUASHFS_BLOCKS) \
		MIDGE_DATAFS_BLOCKS=$$((31-$(MIDGE_KERNEL_BLOCKS)-$(MIDGE_SQUASHFS_BLOCKS))) \
		/bin/sh ./mkconf $(KDIR)/root/etc/midge.conf \
	)
	$(STAGING_DIR)/bin/mksquashfs-lzma $(KDIR)/root $(KDIR)/root.squashfs -nopad -noappend -root-owned -le $(MAKE_TRACE)
	( 	MAKECSYS=yes \
		MIDGE_KERNEL_BLOCKS=$(MIDGE_KERNEL_BLOCKS) \
		MIDGE_SQUASHFS_BLOCKS=$(MIDGE_SQUASHFS_BLOCKS) \
		MIDGE_DATAFS_BLOCKS=$$((31-$(MIDGE_KERNEL_BLOCKS)-$(MIDGE_SQUASHFS_BLOCKS))) \
		MIDGE_TYPE=tmpfs \
		/bin/sh ./mkimage -k $(LINUX_DIR)/vmlinuz -s $(KDIR)/root.squashfs -o $(BIN_DIR)/openwrt-$(BOARD)-$(KERNEL)-$(FS).bin.csys )
	( 	MIDGE_KERNEL_BLOCKS=$(MIDGE_KERNEL_BLOCKS) \
		MIDGE_SQUASHFS_BLOCKS=$(MIDGE_SQUASHFS_BLOCKS) \
		MIDGE_DATAFS_BLOCKS=$$((31-$(MIDGE_KERNEL_BLOCKS)-$(MIDGE_SQUASHFS_BLOCKS))) \
		MIDGE_TYPE=tmpfs \
		/bin/sh ./mkimage -k $(LINUX_DIR)/vmlinuz -s $(KDIR)/root.squashfs -o $(BIN_DIR)/openwrt-$(BOARD)-$(KERNEL)-$(FS).bin )
	

install: $(BIN_DIR)/openwrt-adm5120-$(KERNEL)-$(FSNAME).bin
endif

#ifneq ($(FS),jffs2-4MB)
#$(BIN_DIR)/openwrt-br6104k-$(KERNEL)-$(FSNAME).bin: $(BIN_DIR)/openwrt-$(BOARD)-$(KERNEL)-$(FS).bin
#
#install: $(BIN_DIR)/openwrt-br6104k-$(KERNEL)-$(FSNAME).bin
#endif

endif


ifeq ($(KERNEL),2.6)

ifeq ($(FS),jffs2-32MB)

adm5120-fs-prepare:
	test -f delete-files-$(KERNEL)-$(FS) && \
			for f in `cat delete-files-$(KERNEL)-$(FS)`; do \
				rm -rf $(KDIR)/root/$$f;	\
			done; 
	( cd ../../package/base-files/files/$(BOARD)-$(KERNEL) && \
		tar cf - --exclude .svn . | tar xvf - -C $(KDIR)/root/; \
		 $(SED) 's,$$R,r$(REV),g' $(KDIR)/root/etc/banner \
	)

$(BIN_DIR)/openwrt-$(BOARD)-$(KERNEL)-$(FS).bin: $(SCRIPT_DIR)/padfile
	( 	SG_KERNEL_BLOCKS=$(SG_KERNEL_BLOCKS) \
		SG_FS_BLOCKS=$(SG_FS_BLOCKS) \
		PADFILE=$(PADFILE) \
		CROSS_COMPILE=$(TARGET_CROSS) \
		IMG_NAME=$(BIN_DIR)/openwrt-$(BOARD)-$(KERNEL)-$(FS).bin \
		/bin/sh ./mkimage-jffs2 $(KDIR)/root.jffs2-32MB $(BUILD_DIR)/linux/vmlinux )

endif



endif

clean:
prepare: adm5120-fs-prepare
compile:
install: $(BIN_DIR)/openwrt-$(BOARD)-$(KERNEL)-$(FS).bin
