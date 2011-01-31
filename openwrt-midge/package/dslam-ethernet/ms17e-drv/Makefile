MS17E_MODNAME=ms17e
CFLAGS+=
obj-m := $(MS17E_MODNAME).o
$(MS17E_MODNAME)-objs := ms17e_main.o ms17e_debug.o ms17e_net.o ms17e_sysfs.o

$(MS17E_MODNAME).ko: ms17e_main.c ms17e_debug.c ms17e_net.c ms17e_sysfs.c
	make -C /home/imbizo/midge/openwrt-midge/build_mipsel/linux SUBDIRS=$(PWD) modules CROSS_COMPILE="/home/imbizo/midge/openwrt-midge/staging_dir_mipsel/bin/mipsel-linux-uclibc-"

all: $(MS17E_MODNAME).ko

clean:
	rm --preserve-root -f .*.o*
	rm --preserve-root -f .*.ko*
	rm --preserve-root -f *.o*
	rm --preserve-root -f *.ko*
	rm --preserve-root -f *.mod.*
	rm --preserve-root -f -R .tmp*
