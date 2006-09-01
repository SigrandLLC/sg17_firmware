#!/bin/sh
# (c) Vladislav Moskovets 2005
# Sigrand webface project
# 

#local s="set last_update=`date +%Y%m%d-%H:%M:%S`";
#KDB_PARAMS="set last_update=`date +%Y%m%d-%H:%M:%S` : "
KDB_PARAMS="set q=q "		#TODO : remove this hack

kdb_ladd_string(){
	local param="$1";
	local value;

	var="\$FORM_${param}"
	value="`eval echo $var`"
	debug "kdb_ladd_string(): var=$var value=$value <br>"
	if [ ! -z "$value" ]; then
		KDB_PARAMS="${KDB_PARAMS} : ladd $param=\"$value\" "
	else
		warn "kdb_set_string(): var=$var value=$value <br>"
	fi
}

kdb_set_string(){
	local param="$1";
	local value;

	var="\$FORM_${param}"
	value="`eval echo $var`"
	debug "kdb_set_string(): var=$var value=$value <br>"
	if [ ! -z "$value" ]; then
		KDB_PARAMS="${KDB_PARAMS} : set $param=\"$value\" "
	else
		# clears key
		KDB_PARAMS="${KDB_PARAMS} : set $param= "
	fi
}

kdb_set_int(){
	kdb_set_string $1
}

kdb_set_bool(){
	local param="$1";
	local value;

	var="\$FORM_${param}"
	value="`eval echo $var`"
	debug "kdb_set_bool(): var=$var value=$value <br>"
	if [ "$value" = "on" ]; then
		KDB_PARAMS="${KDB_PARAMS} : set $param=1 "
	else
		KDB_PARAMS="${KDB_PARAMS} : set $param=0 "
	fi
}

kdb_commit(){
	debug "$kdb $KDB_PARAMS"
	eval "$kdb ${KDB_PARAMS}"
}
