#!/usr/bin/haserl
	. conf/conf.sh
	. lib/misc.sh
	. lib/widgets.sh

	eval `$kdb -qq list sys_`
	printFormBegin network network_save.asp
	printTableTitle "Network settings" 2 

	if [ "$sys_iface_eth0_type" = "static" ]; then
		# sys_iface_eth0_ipaddr
		tip="IP address for wan BLA BLA BLA"
		desc="This address provided by ISP"
		validator='tmt:filters="ltrim,rtrim,nohtml,nospaces,nocommas,nomagic" tmt:message="Please input correct ip address" tmt:pattern="ipaddr"'
		printInput text "WAN IP address " sys_iface_eth0_ipaddr

		# sys_iface_eth0_netmask
		tip="netmask used for BLA BLA BLA"
		desc="Netmask also provided by ISP"
		validator='tmt:filters="ltrim,rtrim,nohtml,nospaces,nocommas,nomagic" tmt:message="Please input correct ip address" tmt:pattern="netmask"'
		printInput text "WAN Netmask" sys_iface_eth0_netmask
	elif [ "$sys_iface_eth0_type" = "dhcp" ]; then
		# sys_iface_eth0_ipaddr
		tip="BLA BLA BLA"
		desc="BLA BLA BLA"
		validator='tmt:filters="ltrim,rtrim,nohtml,nospaces,nocommas,nomagic" tmt:message="Please input correct ip address" tmt:pattern="ipaddr"'
		printInput text "DHCP EXAMPLE ONLY" sys_iface_eth0_ipaddr
	fi

	printFormSubmit
	printFormEnd

