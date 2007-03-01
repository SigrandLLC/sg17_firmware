#!/usr/bin/haserl

cfg_path="/sys/bus/pci/drivers/sg16lan"

iface="${FORM_iface}"
	
eval `kdb -qq ls "sys_dsl_*" `
	

if [ -z "$iface" ]; then
	render_table_title "Modem status" 2
	for iface in $sys_dsl_ifaces; do
		link=`cat ${cfg_path}/${iface}/state`
		tip=""
		desc="SHDSL Link state"
		render_input_field static "SHDSL Status" status "$link"
	done
else

	kdb_vars="  int:sys_dsl_${iface}_rate	\
			str:sys_dsl_${iface}_mode	\
			str:sys_dsl_${iface}_code	\
			str:sys_dsl_${iface}_cfg	\
			str:sys_dsl_${iface}_annex	\
			str:sys_dsl_${iface}_crc	\
			str:sys_dsl_${iface}_fill	\
			bool:sys_dsl_${iface}_inv"	
	subsys="dsl."$iface

	render_save_stuff

	render_form_header
	render_table_title "$iface modem settings" 2

	# sys_dsl_${iface}_name
	render_input_field "hidden" "hidden" iface $iface

	# TODO: static list?
	# sys_dsl_${iface}_rate
	tip=""
	desc="Select DSL line rate"
	validator='tmt:message="'$desc'"'
	render_input_field select "Rate" sys_dsl_${iface}_rate $(for i in `seq 1 64`; do n=$(($i*64)); echo $n $n; done)

	# sys_dsl_${iface}_mode
	tip=""
	desc="Select DSL mode"
	render_input_field select "Mode" sys_dsl_${iface}_mode  master 'Master' slave 'Slave'

	# sys_dsl_${iface}_code
	tip=""
	desc="Select DSL line coding"
	render_input_field select "Coding" sys_dsl_${iface}_code tcpam32 TCPAM32 tcpam16 TCPAM16 tcpam8 TCPAM8 tcpam4 TCPAM4

	# sys_dsl_${iface}_cfg
	tip=""
	desc="Select DSL configuration mode"
	render_input_field select "Config" sys_dsl_${iface}_cfg local local preact preact

	# sys_dsl_${iface}_annex
	tip=""
	desc="Select DSL Annex"
	render_input_field select "Annex" sys_dsl_${iface}_annex A "Annex A" B "Annex B" F "Annex F"

	# sys_dsl_${iface}_crc32
	tip=""
	desc="Select DSL CRC length"
	render_input_field select "CRC" sys_dsl_${iface}_crc crc32 CRC32 crc16 CRC16

	# sys_dsl_${iface}_fill
	tip=""
	desc="Select DSL fill byte value"
	render_input_field select "Fill" sys_dsl_${iface}_fill  fill_ff FF fill_7e 7E

	# sys_dsl_${iface}_inv
	default=0
	tip=""
	desc="Select DSL data inversion"
	render_input_field checkbox "Inversion" sys_dsl_${iface}_inv

	render_submit_field
	render_form_tail

fi


