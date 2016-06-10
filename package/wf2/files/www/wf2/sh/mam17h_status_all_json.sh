#!/bin/sh
RESULT=""
ifaces=`kdb get sys_dslam_ifaces`
path="/sys/class/net/"

let "num_ifaces=0"
for iface in $ifaces; do
	let "num_ifaces=$num_ifaces+1"
done
RESULT="{\n"
let "num=0"
for iface in $ifaces; do
	RESULT="$RESULT\"$iface\" : "
	pwr_presence=`cat /sys/class/net/$iface/ms_private/pwr_source`
	if [ "$pwr_presence" = "1" ]; then
		pwron=`cat /sys/class/net/$iface/ms_private/pwron`
		tmp="{\n\t\"pwr\" : {\n\t\t\"presence\" : \"${pwr_presence}\",\n \t\t\"pwron\" : \"$pwron\"\n\t},\n "
	else
		tmp="{\n\t\"pwr\" : {\n\t\t\"presence\" : \"${pwr_presence}\"\n\t}, "
	fi
	link_state=`cat /sys/class/net/$iface/ms_private/link_state`
	if [ "$link_state" = "1" ]; then
		up_time=`cat /sys/class/net/$iface/ms_private/uptime`
		tcpam=`cat /sys/class/net/$iface/ms_private/tcpam`
		rate=`cat /sys/class/net/$iface/ms_private/rate`
		tmp="$tmp\n\t\"link\" : {\n\t\t\"link_state\" : \"${link_state}\",\n\t\t\"uptime\" : \"${up_time}\",\n\t\t\"tcpam\" : \"${tcpam}\",\n\t\t\"rate\" : \"${rate}\"\n\t}\n}"
	else
		tmp="$tmp\n\t\"link\" : {\n\t\t\"link_state\" : \"${link_state}\"\n\t}\n}"
	fi

	RESULT="$RESULT $tmp"

	let "num=$num+1"
	if [ "$num" != "$num_ifaces" ]; then
		RESULT="$RESULT,\n"
	fi
done
RESULT="$RESULT\n}\n"
echo -e $RESULT
