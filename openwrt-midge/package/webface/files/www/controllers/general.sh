#!/usr/bin/haserl
	. conf/conf.sh
	. lib/misc.sh
	. lib/widgets.sh

	render_title "General settings"

	
	if [ $REQUEST_METHOD = POST ]; then
		kdb_vars="str:sys_hostname str:sys_iface_wan_type"
		subsys="network"
		save "$subsys" "$kdb_vars" 
		render_save_message
	fi
	
	eval `$kdb -qq list sys_`
	render_form_header general

	# sys_hostname
	tip="This is tip for hostname <b>Bold tip</b>"
	desc="This is description for hostname"
	validator='tmt:required="true" tmt:message="Please input name of host" tmt:filters="ltrim,rtrim,nohtml,nospaces,nocommas,nomagic"'
	render_input_field text "Hostname" sys_hostname 
	
	# sys_iface_wan_type
	tip="Select connection type:<br><b>Static:</b> IP address is static<br><b>Dynamic:</b> DHCP"
	desc="Please select connection type of your ISP"
	validator='tmt:required="true" tmt:message="Please select connection type"'
	render_input_field radio "Connection type" sys_iface_wan_type 'static' 'Static address' 'dhcp' 'Dynamic address' \
						#	 'pppoe' 'PPPoE connection'  'pptp' 'PPTP connection'
	render_submit_field
	render_form_tail

