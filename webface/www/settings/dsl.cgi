#!/usr/bin/haserl
<? 
	. ../conf/conf.sh
	title="DSL settings"
	. ../common/header.sh
	. ../lib/misc.sh
	. ../lib/widgets.sh
	
print_dsl_settings(){
	
	local iface
	local var1
	local var2
	
	iface="$1"
        printTableTitle $iface" dsl interface sttings" 2

	# hidden mark dsl_iface_name
	tip=""
	desc=""
	validator='tmt:message="'$desc'"'
	var2="dsl_iface_name"	
	printInput "hidden" "hidden" $var2 $iface

	
	# sys_dsl_ethX_rate
	tip=""
	desc="Select DSL line rate"
	validator='tmt:message="'$desc'"'
	var1=$iface" rate"
	var2="sys_dsl_"$iface"_rate"	
	printInput text "$var1" $var2  

	# sys_dsl_ethX_mode
	tip=""
	desc="Select DSL mode"
	validator='tmt:message="'$desc'"'
	var1=$iface" mode"	
	var2="sys_dsl_"$iface"_mode"	
	printInput select "$var1" $var2 master master slave slave


	# sys_dsl_ethX_code
	tip=""
	desc="Select DSL line coding"
	validator='tmt:message="'$desc'"'
	var1=$iface" coding"	
	var2="sys_dsl_"$iface"_code"	
	printInput select "$var1" $var2 tcpam32 TCPAM32 tcpam16 TCPAM16 tcpam8 TCPAM8 tcpam4 TCPAM4

	# sys_dsl_ethX_cfg
	tip=""
	desc="Select DSL configuration mode"
	validator='tmt:message="'$desc'"'
	var1=$iface" config"	
	var2="sys_dsl_"$iface"_cfg"	
	printInput select "$var1" $var2 local local preact preact

	# sys_dsl_ethX_annex
	tip=""
	desc="Select DSL Annex"
	validator='tmt:message="'$desc'"'
	var1=$iface" annex"	
	var2="sys_dsl_"$iface"_annex"	
	printInput select "$var1" $var2 A "Annex A" B "Annex B" F "Annex F"

	# sys_dsl_ethX_mac
	tip=""
	desc="Select Ethernet MAC address(last 3 numbers)"
	validator='tmt:message="'$desc'"'	
	var1=$iface" MAC"	
	var2="sys_dsl_"$iface"_mac"	
	printInput text "$var1" $var2 


	# sys_dsl_ethX_crc32
	tip=""
	desc="Select DSL CRC length"
	validator='tmt:message="'$desc'"'
	var1=$iface" CRC"	
	var2="sys_dsl_"$iface"_crc32"	
	printInput select "$var1" $var2 crc32 CRC32 crc16 CRC16

	# sys_dsl_ethX_fill
	tip=""
	desc="Select DSL fill byte value"
	validator='tmt:message="'$desc'"'
	var1=$iface" fill"	
	var2="sys_dsl_"$iface"_fill"	
	printInput select "$var1" $var2 fill_ff FF fill_7e 7E

	# sys_dsl_ethX_inv
	tip=""
	desc="Select DSL data inversion"
	validator='tmt:message="'$desc'"'	
	var1=$iface" inverse"	
	var2="sys_dsl_"$iface"_inv"	
	printInput select "$var1" $var2 normal Normal invert Inverse

}	
	
 ?>

<? printTitle ?>

<?
	local iface
	eval `$kdb -qq list sys_`
	for iface in `kdb get sys_dsl_ifaces`; do	
	    printFormBegin dsl dsl_save.asp
	    print_dsl_settings $iface
	    printFormSubmit
	    printFormEnd
	done	

?>

<? . ../common/footer.sh ?>
