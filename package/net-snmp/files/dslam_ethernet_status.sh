#!/bin/sh

. /etc/templates/oem-vars

iface="$1"
param="$2"

if [ "$1" = "-h" -o "$1" = "--help" -o "$1" = "" ]; then
	echo -e "Usage: $0 PORT poe|on_off|state|autoneg|flow|rate|duplex\n\tPORT:\tge0|ge1|fe00|fe01|...|fe07|fe10|fe11|...|fe17|...|fe20|...|fe27|...|fe30|...|fe37"
	exit 0
fi

if [ `kdb get sys_dslam_card` != 1 ]; then
	echo "error! where is no dslam module!"
	exit 1
fi
		if [ "$iface" = "ge0" ]; then
				sw=sw0
				sw_port=24
		else if [ "$iface" = "ge1" ]; then
						sw=sw1
						sw_port=24
				else
						slot=`expr substr $iface 3 1`
						pnum=`expr substr $iface 4 1`
						case $slot in
								0)
									sw=sw0;
									sw_port=$((7-$pnum));
									if [ "`kdb get sys_pcitbl_s0002_iftype`" != "$ms17e_v2_modname" ]; then
										echo "error! in the upper left slot no ms17e module!"
										exit 1
									fi
								;;
								1)
									sw=sw0;
									sw_port=$(($((7-$pnum))+8));
									if [ "`kdb get sys_pcitbl_s0003_iftype`" != "$ms17e_v2_modname" ]; then
										echo "error! in the lower left slot no ms17e module!"
										exit 1
									fi
								;;
								2)
									sw=sw1;
									sw_port=$((7-$pnum));
									if [ "`kdb get sys_pcitbl_s0004_iftype`" != "$ms17e_v2_modname" ]; then
										echo "error! in the upper right slot no ms17e module!"
										exit 1
									fi
								;;
								3)
									sw=sw1;
									sw_port=$(($((7-$pnum))+8));
									if [ "`kdb get sys_pcitbl_s0005_iftype`" != "$ms17e_v2_modname" ]; then
										echo "error! in the lower right slot no ms17e module!"
										exit 1
									fi
								;;
						esac
				fi
		fi
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
if [ "$iface" != "ge0" -a "$iface" != "ge1" ]; then
		pwr_src=`if [ -e /sys/class/net/${iface}/ms_private/pwe_source ]; then cat /sys/class/net/${iface}/ms_private/pwr_source; fi`
		if [ "$pwr_src" = "1" ]; then
				pwr_en=`cat /sys/class/net/${iface}/ms_private/pwr_enable`
				if [ "$pwr_en" = "1" ]; then
						RESULT="$RESULT\t\"poe\" : \"on\",\n"
				else
						RESULT="$RESULT\t\"poe\" : \"off\",\n"
				fi
		fi
fi

if [ "$param" = "poe" ]; then
		if [ "$pwr_src" = "1" ]; then
				pwr_en=`cat /sys/class/net/${iface}/ms_private/pwr_enable`
				if [ "$pwr_en" = "1" ]; then
						echo on
						exit 1
				else
						echo off
						exit 0
				fi
		else
				echo none
		fi
fi
if [ "$param" = "on_off" ]; then
		echo $on_off
fi
if [ "$param" = "state" ]; then
		echo $state
fi
if [ "$param" = "autoneg" ]; then
		echo $auto
fi
if [ "$param" = "flow" ]; then
		echo $flow
fi
if [ "$param" = "rate" ]; then
		echo $speed
fi
if [ "$param" = "duplex" ]; then
		echo $duplex
fi