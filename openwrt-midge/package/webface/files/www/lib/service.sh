#!/bin/sh
# (c) Vladislav Moskovets 2005
# Sigrand webface project
# 

service_reload(){
	local services="$1"
	
	for service in $services; do 
		local logfile=/tmp/$service.svc.log
		case "$service" in
		network)
			/etc/init.d/S40network restart >$logfile 2>&1
			#displayFile $logfile
		;;

		dhcp)
			/etc/init.d/dhcpd restart >$logfile 2>&1
			#displayFile $logfile
		;;
		dns)
			/etc/init.d/S50webface-dnsmasq restart >$logfile 2>&1
			#displayFile $logfile
		;;
		dns_server)
			/etc/init.d/bind restart >$logfile 2>&1
			#displayFile $logfile
		;;
		dsl*)
			iface=${sybsys##*.}
			/etc/init.d/dsl restart $iface >$logfile 2>&1
			#displayFile $logfile		
		;;
		fw)
			/etc/init.d/fw restart >$logfile 2>&1
			#displayFile $logfile		
		;;
		esac
	done
}

