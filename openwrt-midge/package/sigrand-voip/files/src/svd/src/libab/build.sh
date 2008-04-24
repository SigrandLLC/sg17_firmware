#!/bin/sh

# $1 - target configure opts

src_files="\
	ab_basic.c \
	ab_line.c \
	ab_events.c \
	ab_media.c \
	"

echo MAKING LIBAB ...
	rm *.o
	rm *.a
	$1 mipsel-linux-uclibc-gcc -Wall -I./vinetic/include/ -I./tapi/include/ -I./sgatab/ -I.. -I../.. $src_files -c
	ar cr libab.a *.o
echo OK

