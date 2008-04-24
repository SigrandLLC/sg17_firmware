#!/bin/sh

# $1 - target configure opts
# $2 - host
# $3 - build 

curr_path=`pwd`
libab_path=${curr_path}/src/libab
dest_path=${curr_path}/src
asrc_path=${curr_path}/../../

make clean
 
aclocal
autoheader
automake -a --foreign
autoconf

$1 ./configure --host=$2 --build=$3

cd $libab_path
./build.sh $1

cd ${curr_path}
make

