#!/usr/bin/haserl
	. lib/misc.sh
	. lib/widgets.sh

	subsys="dhcp"

	item=$FORM_item
	eval_string="export FORM_$item=\"name=$FORM_name ipaddr=$FORM_ipaddr hwaddr=$FORM_hwaddr\""

	render_popup_save_stuff
	
	render_form_header dhcp_static_edit
	render_table_title "DHCP Host settings" 2
	render_popup_form_stuff
	
	# name
	desc="Host name"
	validator='tmt:filters="ltrim,rtrim,nohtml,nospaces,nocommas,nomagic" tmt:message="Please input correct name" tmt:pattern="ltrim,rtrim,nohtml,nospaces,nocommas,nomagic,nobackquotes"'
	render_input_field text "Host name" name
	
	# ipaddr
	desc="IP Address for host"
	validator='tmt:filters="ltrim,rtrim,nohtml,nospaces,nocommas,nomagic" tmt:message="Please input correct ip address" tmt:pattern="ipaddr"'
	render_input_field text "IP Address" ipaddr

	# hwaddr
	desc="MAC Address for host"
	validator='tmt:filters="ltrim,rtrim,nohtml,nospaces,nocommas,nomagic" tmt:message="Please input correct mac address" tmt:pattern="macaddr"'
	render_input_field text "MAC Address" hwaddr

	render_submit_field

	render_form_tail
