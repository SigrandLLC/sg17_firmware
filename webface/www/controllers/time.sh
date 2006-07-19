#!/usr/bin/haserl
	. conf/conf.sh
	. lib/misc.sh
	. lib/widgets.sh

	
	if [ $REQUEST_METHOD = POST ]; then
		# TODO: migrate to save()?
		kdb_vars="bool:sys_ntpclient_enable str:sys_ntpclient_server str:sys_timezone"
		subsys="time"
		save "$subsys" "$kdb_vars" 
		showSaveMessage
	fi

	eval `$kdb -qq list sys_`
	printFormBegin time time_save
	printTableTitle "Time settings" 2 

	# sys_ntpclient_enable
	tip="Time synchronization on boot and each 3 hours"
	desc="Check this item if you want use time synchronizing"
	validator='tmt:required="true" tmt:message="Please input timeserver" tmt:filters="ltrim,rtrim"'
	printInput checkbox "Use time synchronizing" sys_ntpclient_enable 

	# sys_ntpclient_server
	tip="Input hostname of time server <br>Example: <b>clock-1.cs.cmu.edu</b>"
	desc="Please input hostname or ip address time server"
	validator='tmt:required="true" tmt:message="Please input timeserver" tmt:filters="ltrim,rtrim,nohtml,nospaces,nocommas,nomagic"'
	printInput text "Time server" sys_ntpclient_server 

	# sys_timezone
	tip="Select time zone<br>Example: <b>GMT</b>, <b>GMT+1</b>, <b>GMT+2</b>"
	desc="Please select timezone"
	validator='tmt:invalidindex=0 tmt:message="Please select timezone"'
	printInput select "Time zone" sys_timezone bad "Please select timezone" -12 GMT-12 -11 GMT-11 -10 GMT-10 -9 GMT-9 -8 GMT-8 -7 GMT-7 -6 GMT-6 -5 GMT-5 -4 GMT-4 -3 GMT-3 -2 GMT-2 -1 GMT-1 0 GMT 1 GMT+1 2 GMT+2 3 GMT+3 4 GMT+4 5 GMT+5 6 GMT+6 7 GMT+7 8 GMT+8 9 GMT+9 10 GMT+10 11 GMT+11 12 GMT+12

	printFormSubmit
	printFormEnd

