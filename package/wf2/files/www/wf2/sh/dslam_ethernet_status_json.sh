#!/bin/sh

RESULT="{\n"
iface="$1"
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
				0) sw=sw0; sw_port=$((7-$pnum));;
				1) sw=sw0; sw_port=$(($((7-$pnum))+8));;
				2) sw=sw1; sw_port=$((7-$pnum));;
				3) sw=sw1; sw_port=$(($((7-$pnum))+8));;
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
	pwr_src=`cat /sys/class/net/${iface}/ms_private/pwr_source`
	if [ "$pwr_src" = "1" ]; then
		pwr_en=`cat /sys/class/net/${iface}/ms_private/pwr_enable`
		if [ "$pwr_en" = "1" ]; then
			RESULT="$RESULT\t\"poe\" : \"on\",\n"
		else
			RESULT="$RESULT\t\"poe\" : \"off\",\n"
		fi
	fi
fi
RESULT="$RESULT\t\"on_off\":\"$on_off\",\n\t\"state\":\"$state\","
RESULT="$RESULT\n\t\"autoneg\":\"$auto\",\n\t\"flow\":\"$flow\",\n"
RESULT="$RESULT\t\"rate\":\"$speed\",\n\t\"duplex\":\"$duplex\"\n}\n"
echo -en $RESULT
