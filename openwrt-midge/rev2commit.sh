#!/bin/sh

if test "$#" -ne 1 -o -z "$1"; then
	echo 1>&2 "Usage: $0 revision"
	exit 1
fi

rev=$1

set -e

line=$(( $rev - 9 ))

if test "$line" -lt 0; then
	echo 1>&2 "$0: $rev is not appropriate number"
	exit 1
fi

#echo "rev: $rev, line: $line"

# select line:
git rev-list --reverse HEAD | head -n${line} | tail -n1
