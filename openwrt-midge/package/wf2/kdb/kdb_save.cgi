#!/usr/bin/haserl
<?
	KDB_PARAMS=""
	
	kdb_set_val() {
		KDB_PARAMS="${KDB_PARAMS} : set $1 "
	}
	
	kdb_commit() {
		eval `kdb ${KDB_PARAMS}`
		return $?
	}
	
	for variable in $(env |grep FORM); do
		[ $(echo $variable |grep -c "FORM_SESSIONID") -eq 1 ] && continue
		variable=$(echo $variable |sed s/FORM_//)
		kdb_set_val $variable
	done
	
	kdb_commit
	
	echo ""
?>
