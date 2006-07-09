#!/usr/bin/haserl
<? 
	. ../conf/conf.sh
	title="General settings"
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
	kdb_vars="sys_hostname sys_iface_eth0_type"
	subsys="network"
	save "$kdb_vars" "$subsys"
?>


<? . ../common/footer.sh ?>
