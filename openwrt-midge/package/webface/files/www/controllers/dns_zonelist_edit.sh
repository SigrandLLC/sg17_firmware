#!/usr/bin/haserl
	. lib/misc.sh
	. lib/widgets.sh

	subsys=""
	
	[ -n "$FORM_serial" ] && serial=$(get_new_serial)

	item=$FORM_item
	eval_string="export FORM_$item=\"zone=$FORM_zone zoneid=$FORM_zoneid admin=$FORM_admin serial=$FORM_serial  refresh=$FORM_refresh retry=$FORM_retry expire=$FORM_expire\""
	render_popup_save_stuff
	
	render_form_header dns_zonelist_edit
	render_table_title "DNS Zone options" 2
	render_popup_form_stuff
	
	if [ -n "$FORM_additem" ]; then
		# zoneid
		desc="Identifier of zone - internal zone name"
		validator='tmt:required=true tmt:filters="ltrim,rtrim,nohtml,nospaces,nocommas,nomagic" tmt:message="Please input correct dns zone name" tmt:pattern="dnszone"'
		render_input_field text "Zone id" zoneid
	else
		render_input_field hidden zoneid zoneid $zoneid
	fi

	render_input_field hidden serial serial "$serial"

	# zone
	desc="Name of zone"
	validator='tmt:required=true tmt:filters="ltrim,rtrim,nohtml,nospaces,nocommas,nomagic" tmt:message="Please input correct dns zone name" tmt:pattern="dnszone"'
	render_input_field text "Zone" zone
	
	# admin
	desc="Email of zone admin"
	validator='tmt:required=true tmt:filters="ltrim,rtrim,nohtml,nospaces,nocommas,nomagic" tmt:message="Please input correct email" tmt:pattern="email"'
	render_input_field text "Admin" admin

	# refresh
	tip="Indicates the time when the slave will try to refresh the zone from the master.<br>RFC 1912 recommends 1200 to 43200 seconds"
	desc="Time when the slave will try to refresh the zone from the master."
	validator='tmt:required=true tmt:filters="ltrim,rtrim,nohtml,nospaces,nocommas,nomagic" tmt:message="Please input refresh time" tmt:pattern="positiveinteger" tmt:minnumber=1200 tmt:maxnumber=500000'
	render_input_field text "Refresh" refresh

	# retry
	tip="Typical values would be 180 (3 minutes) to 900 (15 minutes) or higher"
	desc="Defines the time between retries if the slave (secondary) fails to contact the master when refresh (above) has expired"
	validator='tmt:required=true tmt:filters="ltrim,rtrim,nohtml,nospaces,nocommas,nomagic" tmt:message="Please input retry time" tmt:pattern="positiveinteger" tmt:minnumber=180 tmt:maxnumber=20000'
	render_input_field text "retry" retry

	# expire
	tip="Slave servers stop responding to queries for the zone when this time has expired and no contact has been made with the master<br>RFC 1912 recommends 1209600 to 2419200 seconds (2-4 weeks) to allow for major outages of the master."
	desc="Indicates when the zone data is no longer authoritative."
	validator='tmt:required=true tmt:filters="ltrim,rtrim,nohtml,nospaces,nocommas,nomagic" tmt:message="Please input expire time" tmt:pattern="positiveinteger" tmt:minnumber=10000 tmt:maxnumber=90000000'
	render_input_field text "expire" expire
	render_submit_field

	render_form_tail
