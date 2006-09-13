#!/usr/bin/haserl
	. conf/conf.sh
	. lib/misc.sh
	. lib/widgets.sh

	kdb_vars="int:svc_dns_tcpclients str:svc_dns_defzone_name str:svc_dns_defzone_admin int:svc_dns_defzone_refresh"
	kdb_vars="$kdb_vars int:svc_dns_defzone_retry int:svc_dns_defzone_expire int:svc_dns_defzone_ttl"
	subsys="dns"

	render_save_stuff

	eval `$kdb -qq list sys_`
	render_form_header dns 
	render_table_title "DNS Settings" 2 

#	# svc_dns_tcpclients
#	desc="Please enter number TCP clients"
#	validator='tmt:filters="ltrim,rtrim,nohtml,nospaces,nocommas,nomagic"  tmt:message="Please input correct ip address" tmt:pattern="ipaddr"'
#	render_input_field text "Max TCP Clients" svc_dns_tcpclients

	# svc_dns_enable
	tip=""
	desc="Check this item if you want use DNS server on your router"
	validator='tmt:required="true"'
	render_input_field checkbox "Enable DNS server" svc_dns_enable

	# svc_dns_options_tmp
	tip=""
	desc=""
	validator='tmt:required="true"'
	render_input_field checkbox "TMP" svc_dns_options_tmp

	render_submit_field
	render_form_tail

	# dns zone list
	render_form_header dns_zonelist 
	render_table_title "Zones" 2 
	render_iframe_list "dns_zonelist"
	render_form_tail

	
