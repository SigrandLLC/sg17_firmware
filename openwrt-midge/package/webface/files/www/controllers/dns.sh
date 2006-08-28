#!/usr/bin/haserl
	. conf/conf.sh
	. lib/misc.sh
	. lib/widgets.sh

	
	if [ $REQUEST_METHOD = POST ]; then
		kdb_set_string sys_dns_nameserver && \
		kdb_set_string sys_dns_domain && \
		kdb_commit

		if [ "$KDB_ERROR" ]; then
			render_message_box "Error" "Savings failed: $KDB_ERROR"
		else
			render_message_box "Done" "Settings saved"

			update_configs dns

			if [ "$ERROR_MESSAGE" ]; then
				render_message_box "Error" " Configration failed: $ERROR_DETAIL"
			else
				render_message_box "Done" "Configuration updated"
				cfg_flash
			fi
		fi
	fi

	eval `$kdb -qq list sys_`
	render_form_header dns 
	render_table_title "DNS Settings" 2 

	# sys_dns_nameserver
	tip="Dns server for your router BLA BLA BLA"
	desc="Please enter ip address of upstream dns server"
	validator='tmt:filters="ltrim,rtrim,nohtml,nospaces,nocommas,nomagic"  tmt:message="Please input correct ip address" tmt:pattern="ipaddr"'
	render_input_field text "Upstream server" sys_dns_nameserver

	# sys_dns_domain
	tip="Domain for your net BLA BLA BLA"
	desc="Please domain"
	validator=''
	render_input_field text "Domain" sys_dns_domain

	render_submit_field
	render_form_tail

