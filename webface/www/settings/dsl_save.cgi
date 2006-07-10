#!/usr/bin/haserl
<? 
        . ../conf/conf.sh
        title="DSL settings"
	. ../common/header.sh
	. ../lib/misc.sh
	. ../lib/widgets.sh
	. ../lib/kdb.sh
	. ../lib/cfg.sh
	. ../lib/svc.sh
 ?>

<? printTitle ?>


<?
	kdb_vars=""
	var="\$FORM_dsl_iface_name"
	iface="`eval echo $var`"
	echo "<br>iface="$iface
				
	kdb_vars="$kdb_vars sys_dsl_${iface}_rate sys_dsl_${iface}_mode sys_dsl_${iface}_code \
	        sys_dsl_${iface}_cfg sys_dsl_${iface}_mac\
		sys_dsl_${iface}_annex sys_dsl_${iface}_crc32 \ 
		sys_dsl_${iface}_fill sys_dsl_${iface}_inv"
	
	if [ "$KDB_ERROR" ]; then
		displayMessageBox "Error" "Savings failed: $KDB_ERROR"
	else
		displayMessageBox "Done" "Settings saved"
	fi
        subsys="dsl."$iface
	save "$kdb_vars" "$subsys"
?>

<? . ../common/footer.sh ?>
