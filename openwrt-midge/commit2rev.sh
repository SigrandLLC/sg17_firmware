#!/bin/sh

if test "$#" -ne 1 -o -z "$1"; then
	echo 1>&2 "Usage: $0 commit"
	exit 1
fi

commit=$1

line=`git rev-list --reverse HEAD | fgrep -n $commit | cut -d: -f1`
rev=$(( $line + 9 ))
echo "$rev"
