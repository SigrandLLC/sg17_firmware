#!/usr/bin/haserl
	. lib/misc.sh
	. lib/widgets.sh

	subsys="ipsec"

	table=$FORM_table
	item=$FORM_item

	eval_string="export FORM_$item=\"name=$FORM_name enabled=$FORM_enabled proto=$FORM_proto src=$FORM_src dst=$FORM_dst sport=$FORM_sport dport=$FORM_dport natto=$FORM_natto target=$FORM_target\""
	render_popup_save_stuff
	
	render_form_header ipsec_edit
	render_table_title "IPSec $table edit" 2
	render_popup_form_stuff
	
	render_input_field hidden table table "$table"
	
	# enabled
	desc="Check this item to enable rule"
	validator='tmt:required="true"'
	render_input_field checkbox "Enable" enabled

	# src
	desc="Source address specification"
	validator='tmt:required="true" tmt:filters="ltrim,rtrim,nohtml,nospaces,nocommas,nomagic,noquotes,nodoublequotes" tmt:message="Please input correct address" tmt:pattern="ipaddr"'
	render_input_field text "Source" src

	# dst
	desc="Destination address specification"
	validator='tmt:required="true" tmt:filters="ltrim,rtrim,nohtml,nospaces,nocommas,nomagic,noquotes,nodoublequotes" tmt:message="Please input correct address" tmt:pattern="ipaddr"'
	render_input_field text "Destination" dst

	# algo
	tip=""
	desc="The protocol of the rule or of the packet to check"
	validator='tmt:message="Please select lease time"'
	render_input_field select "Protocol" proto all "ALL" tcp "TCP" udp "UDP" icmp "ICMP"

	render_submit_field

	render_form_tail
	
	
