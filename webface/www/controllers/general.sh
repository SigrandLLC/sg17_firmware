#!/usr/bin/haserl
	. conf/conf.sh
	. lib/misc.sh
	. lib/widgets.sh

	flashMessage
	
	if [ $REQUEST_METHOD = POST ]; then
		kdb_vars="sys_hostname sys_iface_eth0_type"
		subsys="network"
		save "$kdb_vars" "$subsys"
	fi
	
	eval `$kdb -qq list sys_`
	printFormBegin general
	printTableTitle "General settings" 2 
		
	# sys_hostname
	tip="This is tip for hostname <b>Bold tip</b>"
	desc="This is description for hostname"
	validator='tmt:required="true" tmt:message="Please input name of host" tmt:filters="ltrim,rtrim,nohtml,nospaces,nocommas,nomagic"'
	printInput text "Hostname" sys_hostname 
	
	# sys_iface_eth0_type
	tip="Select connection type:<br><b>Static:</b> IP address is static<br><b>Dynamic:</b> DHCP"
	desc="Please select connection type of your ISP"
	validator='tmt:required="true" tmt:message="Please select connection type"'
	printInput radio "Connection type" sys_iface_eth0_type 'static' 'Static address' 'dhcp' 'Dynamic address' \
						#	 'pppoe' 'PPPoE connection'  'pptp' 'PPTP connection'
	printFormSubmit
	printFormEnd

