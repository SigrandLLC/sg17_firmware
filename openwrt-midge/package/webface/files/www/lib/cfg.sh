#!/bin/sh
# (c) Vladislav Moskovets 2005
# Sigrand webface project
# 

. /etc/templates/lib

cfg_flash(){
	/sbin/flash save >/dev/null 2>&1
}

save(){
	local kdb_vars="$1"
	local sybsys="$2"

	for v in $kdb_vars; do
		kdb_set_string $v
	done
	kdb_commit
		
	if [ "$ERROR_MESSAGE" ]; then
		displayMessageBox "Error" "Savings failed: $ERROR_DETAIL"
	else
		displayMessageBox "Done" "Settings saved"

		for s in $subsys; do 
			update_configs $s
			[ "$ERROR_MESSAGE" ] && break
		done

		if [ "$ERROR_MESSAGE" ]; then
			displayMessageBox "Error" " Configration failed: $ERROR_DETAIL"
		else
			displayMessageBox "Done" "Configuration updated"
			cfg_flash
			svc_reload $subsys
		fi
	fi
}

