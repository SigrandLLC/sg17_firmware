#!/bin/sh

TMP_PATH=/tmp
TMP_DIR=vendor-sysinfo-reader
STDIR=$TMP_PATH/$TMP_DIR
IFNAME=$1
ARCH_PATH=/root/
ARCN_NAME=$ARCH_PATH/vendor-sysinfo.tar.bz2

rm -Rf $STDIR
mkdir $STDIR

# OS information
dmesg > $STDIR/dmesg.`date -I`.log
logread > $STDIR/logread.`date -I`.log
cat /proc/meminfo > $STDIR/meminfo.`date -I`.log
ps ax > $STDIR/ps.`date -I`.log

# Read SHDSL registers
dsl_ifs=`ls /sys/class/net/ | grep dsl`
for i in $dsl_ifs; do
if [ -d /sys/class/net/$i/sg_private ]; then
	cat  /sys/class/net/$i/sg_private/regs > $STDIR/$i-regs.`date -I `.log
fi
done

tar -C $TMP_PATH -cf /root/vendor-sysinfo.tar $TMP_DIR
echo "/root/vendor-sysinfo.tar created. Send it to Sigrand"

