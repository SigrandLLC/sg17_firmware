#!/usr/bin/haserl

###### this file is obsolete

	. lib/misc.sh
	. lib/widgets.sh

	if [ $REQUEST_METHOD = POST ]; then
		kdb_vars="bool:svc_dhcp_enabled int:svc_dhcp_lease_time str:svc_dhcp_router str:svc_dhcp_nameserver str:svc_dhcp_domain_name "
		kdb_vars="$kdb_vars str:svc_dhcp_ntpserver str:svc_dhcp_winsserver str:svc_dhcp_startip str:svc_dhcp_endip "
		kdb_vars="$kdb_vars str:svc_dhcp_netmask "
		subsys="dhcp"
		save "$subsys" "$kdb_vars" 
		render_save_message
	fi

	eval `$kdb -qq list svc_dhcp`
	render_form_header dhcp
	render_table_title "DHCP Settings" 2 

	# svc_dhcp_enabled
	tip=""
	desc="Check this item if you want use DHCP server on your LAN"
	validator='tmt:required="true"'
	render_input_field checkbox "Enable DHCP server" svc_dhcp_enabled

	# svc_dhcp_router
	tip="Router for subnet"
	desc="Default router for your LAN hosts"
	validator='tmt:filters="ltrim,rtrim,nohtml,nospaces,nocommas,nomagic" tmt:message="Please input correct ip for router" tmt:pattern="ipaddr"'
	render_input_field text "Default router" svc_dhcp_router

	# svc_dhcp_lease_time
	tip="Select default lease time"
	desc="Please select lease time"
	validator='tmt:message="Please select lease time"'
	render_input_field select "Default lease time" svc_dhcp_lease_time 600 "10 minutes" 1800 "30 minutes" 3600 "1 hour" \
			10800 "3 hours" 36000 "10 hours" 86400 "24 hours" #infinite "Infinite"

	# svc_dhcp_nameserver
	tip="DNS server for your LAN hosts<br>You can use this device as DNS server"
	desc="DNS server for your LAN hosts"
	validator='tmt:filters="ltrim,rtrim,nohtml,nospaces,nocommas,nomagic" tmt:message="Please input correct ip for DNS server" tmt:pattern="ipaddr"'
	render_input_field text "DNS server" svc_dhcp_nameserver

	# svc_dhcp_domain_name
	tip="Most queries for names within this domain can use short names relative to the local domain"
	desc="Allows DHCP hosts to have fully qualified domain names"
	validator='tmt:filters="ltrim,rtrim,nohtml,nospaces,nocommas,nomagic"'
	render_input_field text "Domain" svc_dhcp_domain_name

	# svc_dhcp_ntpserver
	tip="NTP server for your LAN hosts"
	desc="NTP server for your LAN hosts"
	validator='tmt:filters="ltrim,rtrim,nohtml,nospaces,nocommas,nomagic" tmt:message="Please input correct ip for NTP server" tmt:pattern="ipaddr"'
	render_input_field text "NTP server" svc_dhcp_ntpserver

	# svc_dhcp_winsserver
	tip="WINS server for your LAN hosts"
	desc="WINS server for your LAN hosts"
	validator='tmt:filters="ltrim,rtrim,nohtml,nospaces,nocommas,nomagic" tmt:message="Please input correct ip for WINS server" tmt:pattern="ipaddr"'
	render_input_field text "WINS server" svc_dhcp_winsserver

	# svc_dhcp_startip
	tip=""
	desc="Start of dynamic ip range address for your LAN"
	validator='tmt:required="true" tmt:message="Please input correct start IP" tmt:filters="ltrim,rtrim,nohtml,nospaces,nocommas,nomagic" tmt:pattern="ipaddr"'
	render_input_field text "Start IP" svc_dhcp_startip

	# svc_dhcp_endip
	tip=""
	desc="End of dynaic ip range address for your LAN"
	validator='tmt:required="true" tmt:message="Please input correct end IP" tmt:filters="ltrim,rtrim,nohtml,nospaces,nocommas,nomagic" tmt:pattern="ipaddr"'
	render_input_field text "End IP" svc_dhcp_endip

	# svc_dhcp_netmask
	tip="<b>Example:</b> 255.255.255.0"
	desc="Netmask for your LAN"
	validator='tmt:required="true" tmt:message="Please input correct netmask" tmt:filters="ltrim,rtrim,nohtml,nospaces,nocommas,nomagic" tmt:pattern="netmask"'
	render_input_field text "Netmask" svc_dhcp_netmask

	render_submit_field
	render_form_tail

	# static dhcp list
	render_form_header dhcp_leases dhcp_save
	render_table_title "DHCP Static leases" 2 
	render_iframe_list "dhcp_static"
	render_form_tail

