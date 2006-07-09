#!/usr/bin/haserl
<? 
	. ../conf/conf.sh
	title="DNS settings"
	refresh=$CONF_REFRESH refresh_url="$HTTP_REFERER"
	. ../common/header.sh
	. ../lib/misc.sh
	. ../lib/widgets.sh
	. ../lib/kdb.sh
 ?>

<? printTitle ?>

<?

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


?>


<? . ../common/footer.sh ?>
