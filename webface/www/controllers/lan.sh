#!/usr/bin/haserl
	. conf/conf.sh
	. lib/misc.sh
	. lib/widgets.sh

	eval `$kdb -qq list sys_`
	printFormBegin network network_save.asp
	printTableTitle "Network settings" 2 

	# sys_iface_eth1_ipaddr
	tip="IP address for lan<br>192.168.1.1 etcBLA BLA BLA"
	desc="This address provided by ISP BLA BLA BLA"
	validator='tmt:filters="ltrim,rtrim,nohtml,nospaces,nocommas,nomagic" tmt:message="Please input correct ip address" tmt:pattern="ipaddr"'
	printInput text "LAN IP address " sys_iface_eth1_ipaddr

	# sys_iface_eth1_netmask
	tip="<h2>Examples:</h2><b>255.255.255.0</b> - /24<br><b>255.255.255.252</b> - /30<br>BLA BLA BLA"
	desc="Netmask"
	validator='tmt:filters="ltrim,rtrim,nohtml,nospaces,nocommas,nomagic" tmt:message="Please input correct ip address" tmt:pattern="netmask"'
	printInput text "LAN Netmask" sys_iface_eth1_netmask

	printFormSubmit
	printFormEnd

