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
			if [ -n "$iface" ]; then
				{ ifdown $iface; \
					ifup $iface } 2>&1 | tee $logfile | $LOGGER ;
			else
				/etc/init.d/network restart 2>&1 | tee $logfile | $LOGGER ;
			fi
			#displayFile $logfile
		;;

		dhcp)
			/etc/init.d/dhcpd restart 2>&1 | tee $logfile | $LOGGER
			#displayFile $logfile
		;;
		dns)
			#/etc/init.d/S50webface-dnsmasq restart 2>&1 | tee $logfile | $LOGGER
			#displayFile $logfile
		;;
		dns_server)
			/etc/init.d/bind restart 2>&1 | tee $logfile | $LOGGER
			#displayFile $logfile
		;;
		dsl*)
			iface=${sybsys##*.}
			/etc/init.d/dsl restart $iface 2>&1 | tee $logfile | $LOGGER
			#displayFile $logfile		
		;;
		fw)
			/etc/init.d/fw restart 2>&1 | tee $logfile | $LOGGER
			#displayFile $logfile		
		;;
		esac
	done
}

