#!/usr/bin/haserl
	. conf/conf.sh
	. lib/misc.sh
	. lib/widgets.sh

	if [ $REQUEST_METHOD = POST ]; then
		kdb_vars="str:sys_iface_lan_ipaddr str:sys_iface_lan_netmask"
		subsys="network"
		save "$subsys" "$kdb_vars" 
		showSaveMessage
	fi

	eval `$kdb -qq list sys_`
	printFormBegin lan
	printTableTitle "Network settings" 2 

	# sys_iface_lan_ipaddr
	tip="IP address for lan<br>192.168.1.1 etc BLA BLA BLA"
	desc="This address provided by ISP BLA BLA BLA"
	validator='tmt:filters="ltrim,rtrim,nohtml,nospaces,nocommas,nomagic" tmt:message="Please input correct ip address" tmt:pattern="ipaddr"'
	printInput text "LAN IP address " sys_iface_lan_ipaddr

	# sys_iface_lan_netmask
	tip="<h2>Examples:</h2><b>255.255.255.0</b> - /24<br><b>255.255.255.252</b> - /30<br>BLA BLA BLA"
	desc="Netmask"
	validator='tmt:filters="ltrim,rtrim,nohtml,nospaces,nocommas,nomagic" tmt:message="Please input correct ip address" tmt:pattern="netmask"'
	printInput text "LAN Netmask" sys_iface_lan_netmask

	printFormSubmit
	printFormEnd

