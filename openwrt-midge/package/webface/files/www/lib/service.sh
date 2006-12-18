#!/bin/sh
# (c) Vladislav Moskovets 2005
# Sigrand webface project
# 

service_reload(){
	local service="$1"
	
	case "$service" in
	network)
		if [ -n "$iface" ]; then
			/sbin/ifdown $iface;
			/sbin/ifup $iface  
		else
			/etc/init.d/network restart
		fi
	;;
	dhcp)
		/etc/init.d/udhcpd restart $iface
		#displayFile $logfile
	;;
	dns)
		#/etc/init.d/S50webface-dnsmasq restart 
		#displayFile $logfile
	;;
	dns_server)
		/etc/init.d/bind restart 
		#displayFile $logfile
	;;
	dsl*)
		iface=${sybsys##*.}
		/etc/init.d/dsl restart $iface 
		#displayFile $logfile		
	;;
	fw)
		/etc/init.d/fw restart 
		#displayFile $logfile		
	;;
	esac
}

