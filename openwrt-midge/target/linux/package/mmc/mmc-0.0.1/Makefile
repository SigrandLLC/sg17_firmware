TARGET  := mmc
WARN    := -W -Wall -Wstrict-prototypes -Wmissing-prototypes -fno-strict-aliasing -mips32
INCLUDE := -isystem /home/rohde/buildroot/build_mipsel/linux/include 
CFLAGS  := -O2 -DMODULE -D__KERNEL__ ${WARN} ${INCLUDE} -fno-pic -mno-abicalls -mlong-calls
CC      := /home/rohde/develop/buildroot/build_mipsel/staging_dir/bin/mipsel-linux-uclibc-gcc

all:
	$(CC) ${CFLAGS} -c -o ${TARGET}.o ${TARGET}.c

clean:
	rm -f mmc.o
