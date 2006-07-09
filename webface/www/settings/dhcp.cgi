#!/usr/bin/haserl
<? 
	. ../conf/conf.sh
	title="DHCP settings"
	. ../common/header.sh
	. ../lib/misc.sh
	. ../lib/widgets.sh
 ?>

<? printTitle ?>


<?

	eval `$kdb -qq list sys_`
	printFormBegin dhcp dhcp_save.asp
	printTableTitle "$title" 2 

	# sys_dhcp_enable
	tip=""
	desc="Check this item if you want use DHCP server on your LAN"
	validator='tmt:required="true"'
	printInput checkbox "Enable DHCP server" sys_dhcp_enable

	# sys_dhcp_lease_time
	tip="Select default lease time"
	desc="Please select lease time"
	validator='tmt:message="Please select lease time"'
	printInput select "Default lease time" sys_dhcp_lease_time 10m "10 minutes" 30m "30 minutes" 1h "1 hour" \
			3h "3 hours" 10h "10 hours" 24h "24 hours" infinite "Infinite"

	# sys_dhcp_nameserver
	tip="DNS server for your LAN hosts<br>You can use this device as DNS server"
	desc="DNS server for your LAN hosts"
	validator='tmt:filters="ltrim,rtrim,nohtml,nospaces,nocommas,nomagic" tmt:message="Please input correct ip for DNS server" tmt:pattern="ipaddr"'
	printInput text "DNS server" sys_dhcp_nameserver

	# sys_dhcp_domain_name
	tip="Most queries for names within this domain can use short names relative to the local domain"
	desc="Allows DHCP hosts to have fully qualified domain names"
	validator='tmt:filters="ltrim,rtrim,nohtml,nospaces,nocommas,nomagic"'
	printInput text "Domain" sys_dhcp_domain_name

	# sys_dhcp_ntpserver
	tip="NTP server for your LAN hosts"
	desc="NTP server for your LAN hosts"
	validator='tmt:filters="ltrim,rtrim,nohtml,nospaces,nocommas,nomagic" tmt:message="Please input correct ip for NTP server" tmt:pattern="ipaddr"'
	printInput text "NTP server" sys_dhcp_ntpserver

	# sys_dhcp_winsserver
	tip="WINS server for your LAN hosts"
	desc="WINS server for your LAN hosts"
	validator='tmt:filters="ltrim,rtrim,nohtml,nospaces,nocommas,nomagic" tmt:message="Please input correct ip for WINS server" tmt:pattern="ipaddr"'
	printInput text "WINS server" sys_dhcp_winsserver

	# sys_dhcp_startip
	tip=""
	desc="Start of dynamic ip range address for your LAN"
	validator='tmt:required="true" tmt:message="Please input correct start IP" tmt:filters="ltrim,rtrim,nohtml,nospaces,nocommas,nomagic" tmt:pattern="ipaddr"'
	printInput text "Start IP" sys_dhcp_startip

	# sys_dhcp_endip
	tip=""
	desc="End of dynaic ip range address for your LAN"
	validator='tmt:required="true" tmt:message="Please input correct end IP" tmt:filters="ltrim,rtrim,nohtml,nospaces,nocommas,nomagic" tmt:pattern="ipaddr"'
	printInput text "End IP" sys_dhcp_endip

	# sys_dhcp_netmask
	tip="<b>Example:</b> 255.255.255.0"
	desc="Netmask for your LAN"
	validator='tmt:required="true" tmt:message="Please input correct netmask" tmt:filters="ltrim,rtrim,nohtml,nospaces,nocommas,nomagic" tmt:pattern="netmask"'
	printInput text "Netmask" sys_dhcp_netmask

	printFormSumbit
	printFormEnd

	printFormBegin dhcp_leases dhcp_save.asp
	printTableTitle "DHCP Static leases" 2 

	printFormSumbit
	printFormEnd

	printFormBegin dhcp_add_lease dhcp_save.asp
	printTableTitle "Add static lease" 2 

	printFormSumbit
	printFormEnd
?>

<? . ../common/footer.sh ?>
