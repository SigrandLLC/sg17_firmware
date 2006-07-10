#!/bin/sh
# (c) Vladislav Moskovets 2005
# Sigrand webface project
# 

svc_reload(){
	local svc="$1"
	local logfile=/tmp/$svc.svc.log

	case "$svc" in
	network)
		/etc/init.d/S40network restart >$logfile 2>&1
		displayFile $logfile
	;;

	dhcp|dns)
		/etc/init.d/S50webface-dnsmasq restart >$logfile 2>&1
		displayFile $logfile
	;;
	dsl*)
		iface=${sybsys##*.}
		/etc/init.d/S50dsl restart $iface >$logfile 2>&1
		displayFile $logfile		

	esac
}

