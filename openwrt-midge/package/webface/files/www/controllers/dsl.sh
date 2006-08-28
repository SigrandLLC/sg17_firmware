#!/usr/bin/haserl
	. conf/conf.sh
	. lib/misc.sh
	. lib/widgets.sh
	
print_dsl_settings(){
	
	local iface
	local var1
	local var2
	
	iface="$1"
        render_table_title $iface" dsl interface sttings" 2

	# hidden mark dsl_iface_name
	tip=""
	desc=""
	validator='tmt:message="'$desc'"'
	var2="dsl_iface_name"	
	render_input_field "hidden" "hidden" $var2 $iface

	
	# sys_dsl_ethX_rate
	tip=""
	desc="Select DSL line rate"
	validator='tmt:message="'$desc'"'
	var1=$iface" rate"
	var2="sys_dsl_"$iface"_rate"	
	render_input_field text "$var1" $var2  

	# sys_dsl_ethX_mode
	tip=""
	desc="Select DSL mode"
	validator='tmt:message="'$desc'"'
	var1=$iface" mode"	
	var2="sys_dsl_"$iface"_mode"	
	render_input_field select "$var1" $var2 master master slave slave


	# sys_dsl_ethX_code
	tip=""
	desc="Select DSL line coding"
	validator='tmt:message="'$desc'"'
	var1=$iface" coding"	
	var2="sys_dsl_"$iface"_code"	
	render_input_field select "$var1" $var2 tcpam32 TCPAM32 tcpam16 TCPAM16 tcpam8 TCPAM8 tcpam4 TCPAM4

	# sys_dsl_ethX_cfg
	tip=""
	desc="Select DSL configuration mode"
	validator='tmt:message="'$desc'"'
	var1=$iface" config"	
	var2="sys_dsl_"$iface"_cfg"	
	render_input_field select "$var1" $var2 local local preact preact

	# sys_dsl_ethX_annex
	tip=""
	desc="Select DSL Annex"
	validator='tmt:message="'$desc'"'
	var1=$iface" annex"	
	var2="sys_dsl_"$iface"_annex"	
	render_input_field select "$var1" $var2 A "Annex A" B "Annex B" F "Annex F"

	# sys_dsl_ethX_mac
	tip=""
	desc="Select Ethernet MAC address(last 3 numbers)"
	validator='tmt:message="'$desc'"'	
	var1=$iface" MAC"	
	var2="sys_dsl_"$iface"_mac"	
	render_input_field text "$var1" $var2 


	# sys_dsl_ethX_crc32
	tip=""
	desc="Select DSL CRC length"
	validator='tmt:message="'$desc'"'
	var1=$iface" CRC"	
	var2="sys_dsl_"$iface"_crc32"	
	render_input_field select "$var1" $var2 crc32 CRC32 crc16 CRC16

	# sys_dsl_ethX_fill
	tip=""
	desc="Select DSL fill byte value"
	validator='tmt:message="'$desc'"'
	var1=$iface" fill"	
	var2="sys_dsl_"$iface"_fill"	
	render_input_field select "$var1" $var2 fill_ff FF fill_7e 7E

	# sys_dsl_ethX_inv
	tip=""
	desc="Select DSL data inversion"
	validator='tmt:message="'$desc'"'	
	var1=$iface" inverse"	
	var2="sys_dsl_"$iface"_inv"	
	render_input_field select "$var1" $var2 normal Normal invert Inverse

}	
	
	if [ $REQUEST_METHOD = POST ]; then
		#TODO: Check save
		kdb_vars=""
		var="\$FORM_dsl_iface_name"
		iface="`eval echo $var`"
		echo "<br>iface="$iface
					
		kdb_vars="$kdb_vars sys_dsl_${iface}_rate sys_dsl_${iface}_mode sys_dsl_${iface}_code \
				sys_dsl_${iface}_cfg sys_dsl_${iface}_mac\
			sys_dsl_${iface}_annex sys_dsl_${iface}_crc32 \ 
			sys_dsl_${iface}_fill sys_dsl_${iface}_inv"
		
		if [ "$KDB_ERROR" ]; then
			render_message_box "Error" "Savings failed: $KDB_ERROR"
		else
			render_message_box "Done" "Settings saved"
		fi
			subsys="dsl."$iface
		save "$kdb_vars" "$subsys"
	fi

	render_save_message

	local iface
	eval `$kdb -qq list sys_`
	for iface in `kdb get sys_dsl_ifaces`; do	
	    render_form_header dsl dsl_save.asp
	    print_dsl_settings $iface
	    render_submit_field
	    render_form_tail
	done	

