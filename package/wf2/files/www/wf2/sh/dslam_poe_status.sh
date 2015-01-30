#!/bin/sh

RESULT="poe_status = {\n"
iface=$1
slot=$2
port=$3
type=`kdb get sys_dslam_${iface}_type`
if [ "${type}" = "1" ]; then
	cat /sys/class/net/${iface}/ms_private/status
else
	echo "${port} 3" > /proc/sys/net/ethernet/fe${slot}/poe
	poe="`cat /proc/sys/net/ethernet/fe${slot}/poe`"
	status="${poe##*: }"
	status=${status%% U=*}
	if [ "${status}" = "Enabled ON" ]; then
		u="${poe##*U=}"
		u="${u%%V*}"
		i="${poe##*I=}"
		i="${i%%mA*}"
		p="${poe##*P=}"
		p="${p%%W*}"
	else
		u="${poe##*U=}"
		u="${u%%V*}"
		i=0
		p=0
		tmp=${poe##*T=*V }
		tmp=${tmp%%:*}
		status="$status $tmp"
	fi
	RESULT="${RESULT} \t\"status\":\"${status}\",\n\t\"voltage\":\"${u} V\",\n\t\"current\":\"${i} mA\",\n\t\"power\":\"${p} W\"\n};\n"
	echo -en $RESULT
fi
