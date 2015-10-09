#!/bin/sh

. /etc/templates/oem-vars

iface="$1"
param="$2"

if [ "$1" = "-h" -o "$1" = "--help" -o "$1" = "" ]; then
	echo -e "Usage: $0 PORT on_off|state\n\tPORT:\tdsl00|dsl01|dsl02|dsl03|dsl10|dsl11|dsl12|dsl13|dsl20|dsl21|dsl22|dsl23|dsl30|dsl31|dsl32|dsl33"
	exit 0
fi

if [ `kdb get sys_dslam_card` != 1 ]; then
	echo "error! where is no dslam module!"
	exit 1
fi
slot=`expr substr $iface 4 1`
pnum=`expr substr $iface 5 1`
case $slot in
	0)
		sw=sw0;
		sw_port=$(($pnum));
		if [ "`kdb get sys_pcitbl_s0002_iftype`" != "$mam17h_modname" ]; then
			echo "error! in the upper left slot no ms17h module!"
			exit 1
		fi
	;;
	1)
		sw=sw0;
		sw_port=$(($(($pnum))+8));
		if [ "`kdb get sys_pcitbl_s0003_iftype`" != "$mam17h_modname" ]; then
			echo "error! in the lower left slot no ms17h module!"
			exit 1
		fi
	;;
	2)
		sw=sw1;
		sw_port=$(($pnum));
		if [ "`kdb get sys_pcitbl_s0004_iftype`" != "$mam17h_modname" ]; then
			echo "error! in the upper right slot no ms17h module!"
			exit 1
		fi
	;;
	3)
		sw=sw1;
		sw_port=$(($(($pnum))+8));
		if [ "`kdb get sys_pcitbl_s0005_iftype`" != "$mam17h_modname" ]; then
			echo "error! in the lower right slot no ms17h module!"
			exit 1
		fi
	;;
esac

echo "$sw_port" > /proc/sys/net/dslam_sw/$sw/status
status=`cat /proc/sys/net/dslam_sw/$sw/status`
let "tmp=0"
for i in $status; do
		if [ "$tmp" = "1" ]; then
				flow=$i
		fi
		if [ "$tmp" = "2" ]; then
				duplex=$i
		fi
		if [ "$tmp" = "3" ]; then
				speed=$i
		fi
		if [ "$tmp" = "4" ]; then
				state=$i
				if [ "$state" = "up" ]; then
						state="up"
				else
						state="down"
				fi
		fi
		if [ "$tmp" = "5" ]; then
				auto=$i
				if [ "$auto" = "auto" ]; then
						auto="on"
				else
						auto="off"
				fi
		fi
		if [ "$tmp" = "6" ]; then
				on_off=$i
		fi
		let "tmp=$tmp+1"
done

if [ "$param" = "on_off" ]; then
		echo $on_off
fi
if [ "$param" = "state" ]; then
		echo $state
fi
