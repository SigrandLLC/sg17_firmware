#!/usr/bin/haserl
	. conf/conf.sh
	. lib/misc.sh
	. lib/widgets.sh

	
	
	eval `$kdb -qq list sys_iface_$iface`
	[ "$sys_iface_$iface_type" = "static" ] && kdb_vars="str:sys_iface_wan_ipaddr str:sys_iface_wan_netmask str:sys_iface_wan_gateway"
	subsys="network"

	render_save_stuff

	eval `$kdb -qq list sys_iface`
	render_form_header iface
	render_table_title "General interface settings" 2

	# sys_iface_${iface}_enabled
	tip=""
	desc=""
	validator='tmt:required="true"'
	render_input_field checkbox "Enabled" sys_iface_${iface}_enabled

	# sys_iface_${iface}_auto
	tip=""
	desc=""
	validator='tmt:required="true"'
	render_input_field checkbox "Auto" sys_iface_${iface}_auto

	# sys_iface_wan_type
	tip="Select connection type:<br><b>Static:</b> IP address is static<br><b>Dynamic:</b> DHCP"
	desc="Please select connection type of your ISP"
	validator='tmt:required="true" tmt:message="Please select connection type"'
	render_input_field radio "Connection type" sys_iface_wan_type 'static' 'Static address' 'dhcp' 'Dynamic address' \
						#	 'pppoe' 'PPPoE connection'  'pptp' 'PPTP connection'
	render_submit_field
	render_form_tail

	render_form_header iface
	render_table_title "Address settings" 2

	if [ "$sys_iface_${iface}_type" = "static" ]; then
		# sys_iface_${iface}_ipaddr
		#tip="IP address for interface"
		desc="Netmask (dotted quad) <b>required</b>"
		validator='tmt:filters="ltrim,rtrim,nohtml,nospaces,nocommas,nomagic" tmt:message="Please input correct ip address" tmt:pattern="ipaddr"'
		render_input_field text "Static address " sys_iface_${iface}_ipaddr

		# sys_iface_${iface}_netmask
		#tip="Netmask used for BLA BLA BLA"
		desc="Netmask for interface"
		validator='tmt:filters="ltrim,rtrim,nohtml,nospaces,nocommas,nomagic" tmt:message="Please input correct netmask" tmt:pattern="netmask"'
		render_input_field text "Netmask" sys_iface_${iface}_netmask

		# sys_iface_${iface}_gateway
		tip="Gateway used for default routing"
		desc="Gateway "
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

