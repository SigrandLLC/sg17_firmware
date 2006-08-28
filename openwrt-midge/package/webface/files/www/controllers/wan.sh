#!/usr/bin/haserl
	. conf/conf.sh
	. lib/misc.sh
	. lib/widgets.sh


	if [ $REQUEST_METHOD = POST ]; then
		eval `$kdb -qq list sys_`
		[ "$sys_iface_wan_type" = "static" ] && kdb_vars="str:sys_iface_wan_ipaddr str:sys_iface_wan_netmask str:sys_iface_wan_gateway"
		subsys="network"
		save "$subsys" "$kdb_vars" 
		render_save_message
	fi
	
	eval `$kdb -qq list sys_`
	render_form_header wan
	render_table_title "Network settings" 2 

	if [ "$sys_iface_wan_type" = "static" ]; then
		# sys_iface_wan_ipaddr
		tip="IP address for wan BLA BLA BLA"
		desc="This address provided by ISP"
		validator='tmt:filters="ltrim,rtrim,nohtml,nospaces,nocommas,nomagic" tmt:message="Please input correct ip address" tmt:pattern="ipaddr"'
		render_input_field text "WAN IP address " sys_iface_wan_ipaddr

		# sys_iface_wan_netmask
		tip="Netmask used for BLA BLA BLA"
		desc="Netmask also provided by ISP"
		validator='tmt:filters="ltrim,rtrim,nohtml,nospaces,nocommas,nomagic" tmt:message="Please input correct netmask" tmt:pattern="netmask"'
		render_input_field text "WAN Netmask" sys_iface_wan_netmask

		# sys_iface_wan_gateway
		tip="Gateway used for routing BLA BLA BLA"
		desc="Gateway also provided by ISP"
		validator='tmt:filters="ltrim,rtrim,nohtml,nospaces,nocommas,nomagic" tmt:message="Please input correct ip address" tmt:pattern="ipaddr"'
		render_input_field text "WAN Gateway" sys_iface_wan_gateway
	elif [ "$sys_iface_wan_type" = "dhcp" ]; then
		# sys_iface_wan_ipaddr
		tip="BLA BLA BLA"
		desc="BLA BLA BLA"
		validator='tmt:filters="ltrim,rtrim,nohtml,nospaces,nocommas,nomagic" tmt:message="Please input correct ip address" tmt:pattern="ipaddr"'
		render_input_field text "DHCP - NO OPTIONS<br>EXAMPLE ONLY" sys_iface_wan_ipaddr
	fi

	render_submit_field
	render_form_tail

