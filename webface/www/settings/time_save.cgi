#!/usr/bin/haserl

	kdb_set_bool   sys_rdate_enable && \
	kdb_set_string sys_rdate_host && \
	kdb_set_string sys_timezone && \
	kdb_commit

	if [ "$KDB_ERROR" ]; then
		displayMessageBox "Error" "Savings failed: $KDB_ERROR"
	else
		displayMessageBox "Done" "Settings saved"

		update_configs time

		if [ "$ERROR_MESSAGE" ]; then
			displayMessageBox "Error" " Configration failed: $ERROR_DETAIL"
		else
			displayMessageBox "Done" "Configuration updated"
			cfg_flash
		fi
	fi

?>


<? . ../common/footer.sh ?>
