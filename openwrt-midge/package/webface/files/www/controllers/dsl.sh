#!/usr/bin/haserl
	. conf/conf.sh
	. lib/misc.sh
	. lib/widgets.sh

cfg_path="/sys/bus/pci/drivers/sg16lan"
	
print_dsl_settings(){
	
	local iface
	local var1
	local var2
	
	iface="$1"

	
        render_table_title $iface" interface sttings" 2

	# hidden mark dsl_iface_name
	tip=""
	desc=""
	validator='tmt:message="'$desc'"'
	var2="iface"	
	render_input_field "hidden" "hidden" $var2 $iface


	# hidden mark dsl_iface_name
	link=`cat ${cfg_path}/${iface}/state`
	tip=""
	desc="Link state"
	validator='tmt:message="'$desc'"'
	var1="link state"		
	var2="link"	
#	render_input_field select "$var1" $var2 online online offline offline
	render_2table_field "$var1" "$link"

	
	# sys_dsl_ethX_rate
	tip=""
	desc="Select DSL line rate"
	validator='tmt:message="'$desc'"'
	var1="rate"
	var2="sys_dsl_${iface}_rate"	
	render_input_field text "$var1" $var2  

	# sys_dsl_ethX_mode
	tip=""
	desc="Select DSL mode"
	validator='tmt:message="'$desc'"'
	var1="mode"	
	var2="sys_dsl_${iface}_mode"		
	render_input_field select "$var1" $var2 master master slave slave


	# sys_dsl_ethX_code
	tip=""
	desc="Select DSL line coding"
	validator='tmt:message="'$desc'"'
	var1="coding"	
	var2="sys_dsl_${iface}_coding"
	render_input_field select "$var1" $var2 tcpam32 TCPAM32 tcpam16 TCPAM16 tcpam8 TCPAM8 tcpam4 TCPAM4

	# sys_dsl_ethX_cfg
	tip=""
	desc="Select DSL configuration mode"
	validator='tmt:message="'$desc'"'
	var1="config"	
	var2="sys_dsl_${iface}_cfg"	
	render_input_field select "$var1" $var2 local local preact preact

	# sys_dsl_ethX_annex
	tip=""
	desc="Select DSL Annex"
	validator='tmt:message="'$desc'"'
	var1="annex"	
	var2="sys_dsl_${iface}_annex"	
	render_input_field select "$var1" $var2 A "Annex A" B "Annex B" F "Annex F"

	# sys_dsl_ethX_mac
	tip=""
	desc="Select Ethernet MAC address(last 3 numbers)"
	validator='tmt:message="'$desc'"'	
	var1="MAC"	
	var2="sys_dsl_${iface}_mac"	
	render_input_field text "$var1" $var2 


	# sys_dsl_ethX_crc32
	tip=""
	desc="Select DSL CRC length"
	validator='tmt:message="'$desc'"'
	var1="CRC"	
	var2="sys_dsl_${iface}_crc32"	
	render_input_field select "$var1" $var2 crc32 CRC32 crc16 CRC16

	# sys_dsl_ethX_fill
	tip=""
	desc="Select DSL fill byte value"
	validator='tmt:message="'$desc'"'
	var1="fill"	
	var2="sys_dsl_${iface}_fill"	
	render_input_field select "$var1" $var2 fill_ff FF fill_7e 7E

	# sys_dsl_ethX_inv
	tip=""
	desc="Select DSL data inversion"
	validator='tmt:message="'$desc'"'	
	var1="inverse"	
	var2="sys_dsl_${iface}_inv"	
	render_input_field select "$var1" $var2 normal Normal invert Inverse

}	

 
#	render_title "SHDSL settings"	
	[ -d "$cfg_path" ] || die "Service stopped"
	
	if [ $REQUEST_METHOD = POST ]; then
		#TODO: Check save
		var="\$FORM_iface"
		iface="`eval echo $var`"
		
		kdb_vars="  int:sys_dsl_${iface}_rate	\
			    str:sys_dsl_${iface}_mode	\
			    str:sys_dsl_${iface}_cfg	\
			    str:sys_dsl_${iface}_mac	\
		            str:sys_dsl_${iface}_annex	\
			    str:sys_dsl_${iface}_crc32	\
		            str:sys_dsl_${iface}_fill	\
			    str:sys_dsl_${iface}_inv"	
		subsys="dsl."$iface
		save  "$subsys" "$kdb_vars"
		render_save_message		
	fi

	eval `$kdb -qq list sys_dsl_`
	for iface in `kdb get sys_dsl_ifaces`; do	
	    render_form_header dsl dsl_save.asp
	    print_dsl_settings $iface
	    render_submit_field
	    render_form_tail
	done	

