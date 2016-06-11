#!/bin/sh

pn=`basename $0`
usage()
{
   echo 1>&2 "Usage: $pn package target [target...]"
   exit 1
}

if test "$#" -lt 2; then
   usage
fi


pkg=$1; shift

for t in "$@"; do
	if   test -e   package/${pkg}/Makefile; then
		  pdir=package/${pkg}
	elif test -e    target/${pkg}/Makefile; then
		   pdir=target/${pkg}
	elif test -e toolchain/${pkg}/Makefile; then
		pdir=toolchain/${pkg}
	else
		echo 1>&2 "$pn package dir ${pkg} is not found"
		exit 1
	fi

	nice -19 make ${pdir}-${t} || exit 1
done
