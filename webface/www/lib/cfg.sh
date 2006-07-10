#!/bin/sh
# (c) Vladislav Moskovets 2005
# Sigrand webface project
# 

. /etc/templates/lib

cfg_flash(){
	# for original midge with 2M flash
	[ -x /sbin/flash ] && /sbin/flash save >/dev/null 2>&1
}

save(){
	local kdb_vars="$1"
	local sybsys="$2"

	while true; do 
		ok_str="Settings saved"

		for v in $kdb_vars; do
			kdb_set_string $v
		done
		kdb_commit
		fail_str="Savings failed: $ERROR_DETAIL"
		[ "$ERROR_MESSAGE" ] && break

		for s in $subsys; do 
			update_configs $s
			[ "$ERROR_MESSAGE" ] && break
		done
		fail_str="Update config failed: $ERROR_DETAIL"
		[ "$ERROR_MESSAGE" ] && break

		cfg_flash
		svc_reload $subsys
		break;
	done;
}
