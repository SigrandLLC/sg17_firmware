#!/bin/sh

curr_path=`pwd`
vinetic_name=drv_vinetic-1.3.1_tapi-3.6.1
path_to_bin=${curr_path}/../../../../../staging_dir_mipsel/bin/
path_to_lin=${curr_path}/../../../../../build_mipsel/linux/
build_dir=${curr_path}/../../bin/

PATH=$PATH:${path_to_bin}

cd ${curr_path}/..
tar -xvpf ${vinetic_name}.tar.gz 
ln -snf ${vinetic_name}	vinetic

cd ${curr_path}

CC=mipsel-linux-gcc make -w -C ${path_to_lin} M=${curr_path} modules

cp  drv_sgatab.ko ${build_dir}
./clean_there

cd ${curr_path}/..
rm -rf vinetic ${vinetic_name}

