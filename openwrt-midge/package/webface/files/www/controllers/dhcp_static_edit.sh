#!/usr/bin/haserl
	. lib/misc.sh
	. lib/widgets.sh

	subsys="dhcp"

	item=$FORM_item
	eval `$kdb -qqc list "$item"`

	if [ "$REQUEST_METHOD" = POST ]; then
		eval "export FORM_$item=\"name=$FORM_name ipaddr=$FORM_ipaddr hwaddr=$FORM_hwaddr\""
		if [ -z "$FORM_additem" ]; then
			save "$subsys" "str:$item" 
		else
			ok_str="Item added"
			kdb_ladd_string $item
			kdb_commit
		fi
		render_save_message
		render_js_close_popup
		render_js_refresh_parent
	fi

	eval `$kdb -qqc list "$item"`
	[ -z "$FORM_additem" ] && eval "export \$${item}"

	render_form_header dhcp_static_edit
	render_table_title "DHCP Host settings" 2
	render_input_field hidden item item $item
	render_input_field hidden popup popup 1
	[ -n "$FORM_additem" ] && render_input_field hidden additem additem 1

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
