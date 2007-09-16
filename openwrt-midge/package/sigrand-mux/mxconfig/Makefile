CFLAGS=-g

all: dev_interface.o mxconfig.c
	$(CC) $(CFLAGS) -o mxconfig mxconfig.c dev_interface.o

dev_interface.o: dev_interface.c dev_interface.h
	$(CC) $(CFLAGS) -c dev_interface.c

clean:
	rm --force *.o mxconfig