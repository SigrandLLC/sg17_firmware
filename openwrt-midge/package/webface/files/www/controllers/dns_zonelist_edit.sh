#!/usr/bin/haserl
	. lib/misc.sh
	. lib/widgets.sh

	subsys="dns_server"
	
	FORM_serial=$(get_new_serial $FORM_serial)

	item=$FORM_item
	eval_string="export FORM_$item=\"zone=$FORM_zone enabled=$FORM_enabled zoneid=$FORM_zoneid nameserver=$FORM_nameserver admin=$FORM_admin serial=$FORM_serial  refresh=$FORM_refresh ttl=$FORM_ttl retry=$FORM_retry expire=$FORM_expire zonetype=$FORM_zonetype \""
	render_popup_save_stuff
	
	render_form_header dns_zonelist_edit
	render_table_title "DNS Zone options" 2
	render_popup_form_stuff
	
	if [ -n "$FORM_additem" ]; then
		# zoneid
		tip="<b>Example:</b> domain1"
		desc="Identifier of zone - just a simple name"
		validator='tmt:required=true tmt:filters="ltrim,rtrim,nohtml,nospaces,nodots,noquotes,nodoublequotes,nocommas,nomagic" tmt:message="Please input zone identifier" tmt:pattern="dnszone"'
		render_input_field text "Zone id" zoneid
	else
		render_input_field hidden zoneid zoneid $zoneid
	fi

	render_input_field hidden serial serial "$serial"
	render_input_field hidden zonetype zonetype "master"
	
	# zone
	desc="Name of zone"
	validator='tmt:required=true tmt:filters="ltrim,rtrim,nohtml,nospaces,nocommas,nomagic" tmt:message="Please input correct dns zone name" tmt:pattern="dnszone"'
	render_input_field text "Zone" zone
	
	# enabled
	tip=""
	desc="Check this item to enable zone"
	validator='tmt:required="true"'
	render_input_field checkbox "Enable" enabled

	# server
	tip="This is most commonly written as a Fully-qualified Domain Name (FQDN and ends with a dot). If the record points to an EXTERNAL server (not defined in this zone) it MUST end with a . (dot) e.g. ns1.example.net."
	desc="A name server that will respond authoritatively for the domain"
	validator='tmt:required=true tmt:filters="ltrim,rtrim,nohtml,nospaces,nocommas,nomagic" tmt:message="Please input correct nameserver for zone" tmt:pattern="dnszone"'
	render_input_field text "Name server" nameserver

	# admin
	desc="Email of zone admin"
	validator='tmt:required=true tmt:filters="ltrim,rtrim,nohtml,nospaces,nocommas,nomagic" tmt:message="Please input correct email" tmt:pattern="email"'
	render_input_field text "Admin" admin

	# refresh
	default="28800"
	tip="Indicates the time when the slave will try to refresh the zone from the master.<br>RFC 1912 recommends 1200 to 43200 seconds"
	desc="Time (seconds) when the slave will try to refresh the zone from the master."
	validator='tmt:required=true tmt:filters="ltrim,rtrim,nohtml,nospaces,nocommas,nomagic" tmt:message="Please input refresh time" tmt:pattern="positiveinteger" tmt:minnumber=1200 tmt:maxnumber=500000'
	render_input_field text "Refresh" refresh

	# ttl
	default="86400"
	tip="TTL in the DNS context defines the duration in seconds that the record may be cached. Zero indicates the record should not be cached"
	desc="Time (seconds) to live"
	validator='tmt:required=true tmt:filters="ltrim,rtrim,nohtml,nospaces,nocommas,nomagic" tmt:message="Please input ttl time" tmt:pattern="positiveinteger" tmt:minnumber=1 tmt:maxnumber=500000'
	render_input_field text "TTL" ttl

	# retry
	default="7200"
	tip="Typical values would be 180 (3 minutes) to 900 (15 minutes) or higher"
	desc="Defines the time (seconds) between retries if the slave (secondary) fails to contact the master when refresh (above) has expired"
	validator='tmt:required=true tmt:filters="ltrim,rtrim,nohtml,nospaces,nocommas,nomagic" tmt:message="Please input retry time" tmt:pattern="positiveinteger" tmt:minnumber=180 tmt:maxnumber=20000'
	render_input_field text "retry" retry

	# expire
	default="1209600"
	tip="Slave servers stop responding to queries for the zone when this time has expired and no contact has been made with the master<br>RFC 1912 recommends 1209600 to 2419200 seconds (2-4 weeks) to allow for major outages of the master."
	desc="Indicates when (seconds) the zone data is no longer authoritative."
	validator='tmt:required=true tmt:filters="ltrim,rtrim,nohtml,nospaces,nocommas,nomagic" tmt:message="Please input expire time" tmt:pattern="positiveinteger" tmt:minnumber=10000 tmt:maxnumber=90000000'
	render_input_field text "expire" expire
	render_submit_field

	render_form_tail
	
	
