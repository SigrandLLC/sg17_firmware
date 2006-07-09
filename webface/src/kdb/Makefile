
#CC=gcc

kdb: main.c
	$(CC) -g main.c -o kdb
install: kdb
	cp kdb ../../router/root/usr/bin/
clean:
	rm -f kdb *.o
