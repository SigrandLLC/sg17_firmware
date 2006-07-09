#!/usr/bin/haserl
<? 
	. ../conf/conf.sh
	title="Network settings"
	#refresh=$CONF_REFRESH refresh_url="$HTTP_REFERER"
	. ../common/header.sh
	. ../lib/misc.sh
	. ../lib/widgets.sh
	. ../lib/kdb.sh
	. ../lib/cfg.sh
	. ../lib/svc.sh
 ?>

<? printTitle ?>

<?


	eval `$kdb -qq list sys_`

	kdb_vars="sys_iface_eth1_ipaddr sys_iface_eth1_netmask"
	[ "$sys_iface_eth0_type" = "static" ] && kdb_vars="$kdb_vars sys_iface_eth0_ipaddr sys_iface_eth0_netmask"
	subsys="network"
	save "$kdb_vars" "$subsys"


?>


<? . ../common/footer.sh ?>
