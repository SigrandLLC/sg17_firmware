#!/usr/bin/haserl
	. lib/misc.sh
	. lib/widgets.sh

	item=$FORM_item
	eval `$kdb -qqc list "$item"`
	eval "export \$${item}"
	render_form_header dhcp_static_edit
	render_table_title "DHCP Host settings" 2
	render_input_field hidden item item $item
	render_input_field hidden popup popup 1

	# name
	tip=""
	desc="Host name"
	validator='tmt:filters="ltrim,rtrim,nohtml,nospaces,nocommas,nomagic" tmt:message="Please input correct name" tmt:pattern="ltrim,rtrim,nohtml,nospaces,nocommas,nomagic,nobackquotes"'
	render_input_field text "Host name" name
	
	# ipaddr
	tip=""
	desc="IP Address for host"
	validator='tmt:filters="ltrim,rtrim,nohtml,nospaces,nocommas,nomagic" tmt:message="Please input correct ip address" tmt:pattern="ipaddr"'
	render_input_field text "IP Address" ipaddr

	# ipaddr
	tip=""
	desc="MAC Address for host"
	validator='tmt:filters="ltrim,rtrim,nohtml,nospaces,nocommas,nomagic" tmt:message="Please input correct mac address" tmt:pattern="macaddr"'
	render_input_field text "MAC Address" hwaddr

	render_submit_field

	render_form_tail
