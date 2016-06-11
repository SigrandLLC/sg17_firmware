#!/bin/sh

pn=`basename $0`
usage()
{
   echo 1>&2 "Usage: $pn target package [package...]"
   exit 1
}

if test "$#" -lt 2; then
   usage
fi


target=$1; shift

for p in "$@"; do
	nice -19 make package/${p}-${target} || exit 1
done
