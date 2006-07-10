#!/usr/bin/haserl
<? 
	. ../conf/conf.sh
	title="Network settings"
	. ../common/header.sh
	. ../lib/misc.sh
	. ../lib/widgets.sh
 ?>

<? printTitle ?>


<?

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

?>

<? . ../common/footer.sh ?>
