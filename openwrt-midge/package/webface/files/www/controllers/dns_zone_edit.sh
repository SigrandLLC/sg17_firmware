#!/usr/bin/haserl
	. lib/misc.sh
	. lib/widgets.sh

	subsys=""

	if [ -n "$FORM_datatype" ]; then
		[ "$FORM_datatype" = "MX" -a -z "$FORM_prio" ]  && FORM_prio="10"
		[ "$FORM_datatype" != "MX" ]  && FORM_prio=""
	fi
	
	item=$FORM_item
	eval_string="export FORM_$item=\"domain=$FORM_domain datatype=$FORM_datatype prio=$FORM_prio data=$FORM_data\""
	render_popup_save_stuff
	
	render_form_header dns_zonelist_edit
	render_table_title "DNS Zone options" 2
	render_popup_form_stuff
	
	# domain
	tip="<b>Tip:</b> Use @ for current zone"
	desc="Domain"
	validator='tmt:required=true tmt:filters="ltrim,rtrim,nohtml,nospaces,nocommas,nomagic" tmt:message="Please input correct dns domain name" tmt:pattern="dnsdomain"'
	render_input_field text "Domain" domain
	
	# datatype
	desc="Select type of record: A, NS,CNAME, MX" # TXT, PTR currently not supported
	validator='tmt:message="Please select type of record"'
	render_input_field select "Type of record" datatype A "A" CNAME "CNAME" MX "MX" NS "NS"

	# prio
	desc="Priority used only on MX records"
	validator='tmt:filters="ltrim,rtrim,nohtml,nospaces,nocommas,nomagic" tmt:message="Please input priority" tmt:pattern="positiveinteger" tmt:minnumber=1 tmt:maxnumber=999'
	render_input_field text "Priority" prio

	# data
	desc="Please input data for this record"
	validator='tmt:required=true tmt:filters="ltrim,rtrim,nohtml,nospaces,nocommas,nomagic" tmt:message="Please input correct dns domain name" tmt:pattern="dnsdomainoripaddr"'
	render_input_field text "Data" data

	render_submit_field

	render_form_tail
