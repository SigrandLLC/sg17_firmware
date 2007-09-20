CFLAGS=-g

ifneq ($(ARCH),)
GCC=./mips-gcc
else
GCC=gcc
endif

all: dev_interface.o mxconfig.o mxobjects.o
	$(GCC) $(CFLAGS) -o mxconfig mxconfig.o dev_interface.o mxobjects.o

clean:
	rm --force *.o mxconfig