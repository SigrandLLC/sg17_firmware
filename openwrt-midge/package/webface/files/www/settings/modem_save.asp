#!/usr/bin/haserl
<? 
	. ../conf/conf.sh
	title="Modem settings"
	refresh=$CONF_REFRESH refresh_url="$HTTP_REFERER"
	. ../common/header.sh
	. ../lib/misc.sh
	. ../lib/widgets.sh
	. ../lib/kdb.sh
 ?>

<? printTitle ?>


<?

	kdb_set_string sys_modem_speed && \
	kdb_set_string sys_modem_mode && \
	kdb_commit

	if [ "$KDB_ERROR" ]; then
		displayMessageBox "Error" "Savings failed: $KDB_ERROR"
	else
		displayMessageBox "Done" "Settings saved"
	fi


?>


<? . ../common/footer.sh ?>
