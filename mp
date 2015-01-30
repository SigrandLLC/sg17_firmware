#!/bin/sh

pn=`basename $0`
usage()
{
   echo 1>&2 "Usage: $pn [-v] target package [package...]"
   exit 1
}

if test "$#" -lt 2; then
   usage
fi


case $1 in
   -v) V=99; shift;;
esac

if test -n "$V"; then
   _v_="V=${V}"
else
   _v_=
fi


target=$1; shift

for p in "$@"; do
	nice -19 make ${_v_} package/${p}-${target} || exit 1
done
