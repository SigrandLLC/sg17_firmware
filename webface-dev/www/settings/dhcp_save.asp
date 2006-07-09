#!/usr/bin/haserl
<? 
	. ../conf/conf.sh
	title="DHCP settings"
	refresh=$CONF_REFRESH refresh_url="$HTTP_REFERER"
	. ../common/header.sh
	. ../lib/misc.sh
	. ../lib/widgets.sh
	. ../lib/kdb.sh
	. ../lib/cfg.sh
	. ../lib/svc.sh
 ?>

<? printTitle ?>

<?

	kdb_set_bool sys_dhcp_enable && \
	kdb_set_string sys_dhcp_lease_time && \
	kdb_set_string sys_dhcp_nameserver && \
	kdb_set_string sys_dhcp_domain_name && \
	kdb_set_string sys_dhcp_ntpserver && \
	kdb_set_string sys_dhcp_winsserver && \
	kdb_set_string sys_dhcp_startip && \
	kdb_set_string sys_dhcp_endip && \
	kdb_set_string sys_dhcp_netmask && \
	kdb_commit

	if [ "$KDB_ERROR" ]; then
		displayMessageBox "Error" "Savings failed: $KDB_ERROR"
	else
		displayMessageBox "Done" "Settings saved"

		update_configs dhcp
		if [ "$ERROR_MESSAGE" ]; then
			displayMessageBox "Error" " Configration failed: $ERROR_DETAIL"
		else
			displayMessageBox "Done" "Configuration updated"
			cfg_flash
			svc_reload dhcp
		fi
	fi


?>


<? . ../common/footer.sh ?>
