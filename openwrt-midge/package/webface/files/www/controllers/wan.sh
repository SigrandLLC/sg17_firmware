#!/usr/bin/haserl
	. conf/conf.sh
	. lib/misc.sh
	. lib/widgets.sh


	if [ $REQUEST_METHOD = POST ]; then
		eval `$kdb -qq list sys_`
		[ "$sys_iface_wan_type" = "static" ] && kdb_vars="str:sys_iface_wan_ipaddr str:sys_iface_wan_netmask"
		subsys="network"
		save "$subsys" "$kdb_vars" 
		showSaveMessage
	fi
	
	eval `$kdb -qq list sys_`
	printFormBegin wan
	printTableTitle "Network settings" 2 

	if [ "$sys_iface_wan_type" = "static" ]; then
		# sys_iface_wan_ipaddr
		tip="IP address for wan BLA BLA BLA"
		desc="This address provided by ISP"
		validator='tmt:filters="ltrim,rtrim,nohtml,nospaces,nocommas,nomagic" tmt:message="Please input correct ip address" tmt:pattern="ipaddr"'
		printInput text "WAN IP address " sys_iface_wan_ipaddr

		# sys_iface_wan_netmask
		tip="netmask used for BLA BLA BLA"
		desc="Netmask also provided by ISP"
		validator='tmt:filters="ltrim,rtrim,nohtml,nospaces,nocommas,nomagic" tmt:message="Please input correct ip address" tmt:pattern="netmask"'
		printInput text "WAN Netmask" sys_iface_wan_netmask
	elif [ "$sys_iface_wan_type" = "dhcp" ]; then
		# sys_iface_wan_ipaddr
		tip="BLA BLA BLA"
		desc="BLA BLA BLA"
		validator='tmt:filters="ltrim,rtrim,nohtml,nospaces,nocommas,nomagic" tmt:message="Please input correct ip address" tmt:pattern="ipaddr"'
		printInput text "DHCP - NO OPTIONS<br>EXAMPLE ONLY" sys_iface_wan_ipaddr
	fi

	printFormSubmit
	printFormEnd

