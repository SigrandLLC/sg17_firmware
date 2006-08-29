#!/usr/bin/haserl
	. lib/misc.sh
	. lib/widgets.sh

	if [ $REQUEST_METHOD = POST ]; then
		kdb_vars="bool:sys_dhcp_enable int:sys_dhcp_lease_time str:sys_dhcp_router str:sys_dhcp_nameserver str:sys_dhcp_domain_name "
		kdb_vars="$kdb_vars str:sys_dhcp_ntpserver str:sys_dhcp_winsserver str:sys_dhcp_startip str:sys_dhcp_endip "
		kdb_vars="$kdb_vars str:sys_dhcp_netmask "
		subsys="dhcp"
		save "$subsys" "$kdb_vars" 
		render_save_message
	fi

	eval `$kdb -qq list sys_`
	render_form_header dhcp
	render_table_title "DHCP Settings" 2 

	# sys_dhcp_enable
	tip=""
	desc="Check this item if you want use DHCP server on your LAN"
	validator='tmt:required="true"'
	render_input_field checkbox "Enable DHCP server" sys_dhcp_enable

	# sys_dhcp_router
	tip="Router for subnet"
	desc="Default router for your LAN hosts"
	validator='tmt:filters="ltrim,rtrim,nohtml,nospaces,nocommas,nomagic" tmt:message="Please input correct ip for router" tmt:pattern="ipaddr"'
	render_input_field text "Default router" sys_dhcp_router

	# sys_dhcp_lease_time
	tip="Select default lease time"
	desc="Please select lease time"
	validator='tmt:message="Please select lease time"'
	render_input_field select "Default lease time" sys_dhcp_lease_time 600 "10 minutes" 1800 "30 minutes" 3600 "1 hour" \
			10800 "3 hours" 36000 "10 hours" 86400 "24 hours" #infinite "Infinite"

	# sys_dhcp_nameserver
	tip="DNS server for your LAN hosts<br>You can use this device as DNS server"
	desc="DNS server for your LAN hosts"
	validator='tmt:filters="ltrim,rtrim,nohtml,nospaces,nocommas,nomagic" tmt:message="Please input correct ip for DNS server" tmt:pattern="ipaddr"'
	render_input_field text "DNS server" sys_dhcp_nameserver

	# sys_dhcp_domain_name
	tip="Most queries for names within this domain can use short names relative to the local domain"
	desc="Allows DHCP hosts to have fully qualified domain names"
	validator='tmt:filters="ltrim,rtrim,nohtml,nospaces,nocommas,nomagic"'
	render_input_field text "Domain" sys_dhcp_domain_name

	# sys_dhcp_ntpserver
	tip="NTP server for your LAN hosts"
	desc="NTP server for your LAN hosts"
	validator='tmt:filters="ltrim,rtrim,nohtml,nospaces,nocommas,nomagic" tmt:message="Please input correct ip for NTP server" tmt:pattern="ipaddr"'
	render_input_field text "NTP server" sys_dhcp_ntpserver

	# sys_dhcp_winsserver
	tip="WINS server for your LAN hosts"
	desc="WINS server for your LAN hosts"
	validator='tmt:filters="ltrim,rtrim,nohtml,nospaces,nocommas,nomagic" tmt:message="Please input correct ip for WINS server" tmt:pattern="ipaddr"'
	render_input_field text "WINS server" sys_dhcp_winsserver

	# sys_dhcp_startip
	tip=""
	desc="Start of dynamic ip range address for your LAN"
	validator='tmt:required="true" tmt:message="Please input correct start IP" tmt:filters="ltrim,rtrim,nohtml,nospaces,nocommas,nomagic" tmt:pattern="ipaddr"'
	render_input_field text "Start IP" sys_dhcp_startip

	# sys_dhcp_endip
	tip=""
	desc="End of dynaic ip range address for your LAN"
	validator='tmt:required="true" tmt:message="Please input correct end IP" tmt:filters="ltrim,rtrim,nohtml,nospaces,nocommas,nomagic" tmt:pattern="ipaddr"'
	render_input_field text "End IP" sys_dhcp_endip

	# sys_dhcp_netmask
	tip="<b>Example:</b> 255.255.255.0"
	desc="Netmask for your LAN"
	validator='tmt:required="true" tmt:message="Please input correct netmask" tmt:filters="ltrim,rtrim,nohtml,nospaces,nocommas,nomagic" tmt:pattern="netmask"'
	render_input_field text "Netmask" sys_dhcp_netmask

	render_submit_field
	render_form_tail

	render_form_header dhcp_leases dhcp_save
	render_table_title "DHCP Static leases" 2 

	render_iframe_list "dhcp_static"
	#render_submit_field
	render_form_tail

	render_form_header dhcp_add_lease dhcp_save.asp
	render_table_title "Add static lease" 2 

	render_submit_field
	render_form_tail
