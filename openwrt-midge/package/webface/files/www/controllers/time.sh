#!/usr/bin/haserl
	. conf/conf.sh
	. lib/misc.sh
	. lib/widgets.sh

	
	if [ $REQUEST_METHOD = POST ]; then
		# TODO: migrate to save()?
		kdb_vars="bool:sys_rdate_enable str:sys_rdate_host str:sys_timezone"
		subsys="time"
		save "$subsys" "$kdb_vars" 
		showSaveMessage
	fi

	eval `$kdb -qq list sys_`
	printFormBegin time time_save.asp
	printTableTitle "Time settings" 2 

	# sys_rdate_enable
	tip="Time synchronization on boot and each 3 hours"
	desc="Check this item if you want use time synchronizing"
	validator='tmt:required="true" tmt:message="Please input timeserver" tmt:filters="ltrim,rtrim"'
	printInput checkbox "Use time synchronizing" sys_rdate_enable 

	# sys_rdate_host
	tip="Input hostname of time server <br>Example: <b>clock-1.cs.cmu.edu</b>"
	desc="Please input hostname or ip address time server"
	validator='tmt:required="true" tmt:message="Please input timeserver" tmt:filters="ltrim,rtrim,nohtml,nospaces,nocommas,nomagic"'
	printInput text "Time server" sys_rdate_host 

	# sys_timezone
	tip="Select time zone<br>Example: <b>MST-2MDT</b>"
	desc="Please select timezone"
	validator='tmt:invalidindex=0 tmt:message="Please select timezone"'
	printInput select "Time zone" sys_timezone bad "Please select timezone" MSK Moscow EAST East

	printFormSubmit
	printFormEnd
