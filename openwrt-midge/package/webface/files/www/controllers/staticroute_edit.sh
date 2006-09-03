#!/usr/bin/haserl
	. lib/misc.sh
	. lib/widgets.sh

	subsys=""

	item=$FORM_item
	eval_string="export FORM_$item=\"net=$FORM_net netmask=$FORM_netmask gw=$FORM_gw\""

	render_popup_save_stuff
	
	render_form_header dhcp_static_edit
	render_table_title "DHCP Host settings" 2
	render_popup_form_stuff
	
	# net
	desc="Network or host"
	validator='tmt:filters="ltrim,rtrim,nohtml,nospaces,nocommas,nomagic" tmt:message="Please input correct ip address" tmt:pattern="ipaddr"'
	render_input_field text "Network" net
	
	# netmask
	tip="<h2>Examples:</h2><b>255.255.255.0</b> - /24 - Class <b>C</b> network<br><b>255.255.255.252</b> - /30<br><b>255.255.255.255</b> - /32 for a single host<br>BLA BLA BLA"
	desc="Netmask"
	validator='tmt:filters="ltrim,rtrim,nohtml,nospaces,nocommas,nomagic" tmt:message="Please input correct ip netmask" tmt:pattern="netmask"'
	render_input_field text "Netmask" netmask

	# gw
	desc="Gateway for route"
	validator='tmt:filters="ltrim,rtrim,nohtml,nospaces,nocommas,nomagic" tmt:message="Please input correct ip address" tmt:pattern="ipaddr"'
	render_input_field text "Gateway" gw

	render_submit_field

	render_form_tail
