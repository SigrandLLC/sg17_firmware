#!/bin/bash

echo off
make clean
make
scp mam17h.ko root@192.168.2.234:/lib/modules/2.6.16/mam17h.ko
#scp mam17h_cfg root@192.168.2.234:/sbin/mam17h_cfg
make clean
