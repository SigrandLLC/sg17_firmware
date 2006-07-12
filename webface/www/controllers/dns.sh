#!/usr/bin/haserl
	. conf/conf.sh
	. lib/misc.sh
	. lib/widgets.sh

	
	if [ $REQUEST_METHOD = POST ]; then
		kdb_set_string sys_dns_nameserver && \
		kdb_set_string sys_dns_domain && \
		kdb_commit

		if [ "$KDB_ERROR" ]; then
			displayMessageBox "Error" "Savings failed: $KDB_ERROR"
		else
			displayMessageBox "Done" "Settings saved"

			update_configs dns

			if [ "$ERROR_MESSAGE" ]; then
				displayMessageBox "Error" " Configration failed: $ERROR_DETAIL"
			else
				displayMessageBox "Done" "Configuration updated"
				cfg_flash
			fi
		fi
	fi

	eval `$kdb -qq list sys_`
	printFormBegin dns 
	printTableTitle "$title" 2 

	# sys_dns_nameserver
	tip="Dns server for your router BLA BLA BLA"
	desc="Please enter ip address of upstream dns server"
	validator='tmt:filters="ltrim,rtrim,nohtml,nospaces,nocommas,nomagic"  tmt:message="Please input correct ip address" tmt:pattern="ipaddr"'
	printInput text "Upstream server" sys_dns_nameserver

	# sys_dns_domain
	tip="Domain for your net BLA BLA BLA"
	desc="Please domain"
	validator=''
	printInput text "Domain" sys_dns_domain

	printFormSubmit
	printFormEnd

