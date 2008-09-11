#!/usr/bin/haserl
<?
	. /etc/templates/lib
	. /www/lib/service.sh
	
	KDB_PARAMS=""
	
	kdb_set_val() {
		KDB_PARAMS="${KDB_PARAMS} : set '$1' "
	}
	
	kdb_commit() {
		eval "/usr/bin/kdb ${KDB_PARAMS}"
		return $?
	}

	update_configs_and_service_reload(){
		local subsys="$1"
		local s
		local service
		for s in $subsys; do 
			update_configs $s
			[ "$ERROR_MESSAGE" ] && return
		done
		fail_str="Update config failed: $ERROR_DETAIL"
		[ "$ERROR_MESSAGE" ] && return
		for service in $subsys; do
			service_reload $service 2>&1 | $LOGGER
		done
	}
	
IFS='
'
	for variable in $(env |grep FORM); do
		[ $(echo $variable |grep -c "FORM_SESSIONID") -eq 1 ] && continue
		[ $(echo $variable |grep -c "FORM_subsystem") -eq 1 ] && continue
		variable=$(echo $variable |sed s/FORM_//)
		kdb_set_val "$variable"
	done
IFS=' '

	kdb_commit
	
	SUBSYSTEM=$FORM_subsystem
	
	update_configs_and_service_reload "$SUBSYSTEM"
	
	echo ""
?>
