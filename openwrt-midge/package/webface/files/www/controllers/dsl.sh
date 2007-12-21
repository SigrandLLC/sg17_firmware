#!/usr/bin/haserl

eoc_config=/sbin/eoc-config
eoc_info=/sbin/eoc-info

. /www/controllers/oem.sh



require_js_file "prototype.js"
require_js_file "dsl.js"

_sg16_status(){
	cpath="$sg16_cfg_path/$iface"
	help_2="dsl.status"
	render_table_title "$iface "$MR16H_MODNAME" modem status" 2
	# ONLINE status
	
	link=`cat $cpath/state`
	tip=""
	desc="Link state"
	render_input_field static "Link state" status "$link"
}


_sg16_settings(){
	kdb_vars="  int:sys_dsl_${iface}_rate	\
			str:sys_dsl_${iface}_mode	\
			str:sys_dsl_${iface}_code	\
			str:sys_dsl_${iface}_cfg	\
			str:sys_dsl_${iface}_annex	\
			str:sys_dsl_${iface}_crc	\
			str:sys_dsl_${iface}_fill	\
			str:sys_dsl_${iface}_inv"	
	subsys="dsl."$iface

	render_save_stuff
	sleep 1

	render_form_header
	# refresh configuration
	eval `kdb -qq ls "sys_dsl_${iface}_*" ` 
	render_table_title "$iface "$MR16H_MODNAME" modem settings" 2

	# sys_dsl_${iface}_name
	render_input_field "hidden" "hidden" iface $iface
	render_input_field "hidden" "hidden" page settings


	# sys_dsl_${iface}_mode
	tip=""
	desc="Select DSL mode"
	id='mode'
	onchange="OnChangeSG16Code();"	
	render_input_field select "Mode" sys_dsl_${iface}_mode  master 'Master' slave 'Slave'

	unset crate
	eval "crate=\$sys_dsl_${iface}_rate"
#	echo "<br>CRATE = $crate"
	tip=""
	desc="Select DSL line rate"
	validator='tmt:message="'$desc'"'
	id='rate'
	onchange="OnChangeSG16Code();"	
	render_input_field select "Rate" sys_dsl_${iface}_rate $crate $crate

	# sys_dsl_${iface}_code
	tip=""
	desc="Select DSL line coding"
	id='code'
	onchange="OnChangeSG16Code();"
	render_input_field select "Coding" sys_dsl_${iface}_code tcpam32 TCPAM32 tcpam16 TCPAM16 tcpam8 TCPAM8 tcpam4 TCPAM4

	# sys_dsl_${iface}_cfg
	tip=""
	desc="Select DSL configuration mode"
	id='cfg'
	onchange="OnChangeSG16Code();"	
	render_input_field select "Config" sys_dsl_${iface}_cfg local local preact preact

	# sys_dsl_${iface}_annex
	tip=""
	desc="Select DSL Annex"
	id='annex'
	onchange="OnChangeSG16Code();"	
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
	tip=""
	desc="Select DSL inversion mode"
	render_input_field select "Inversion" sys_dsl_${iface}_inv  normal off invert on

	render_submit_field
	render_form_tail

	run_js_code "OnChangeSG16Code();"
}


_sg17_status(){

	#-------------- STATUS table --------------------
	help_2="dsl.status"
	render_table_title "$iface "$MR17H_MODNAME" modem status" 2
	conf_path="$sg17_cfg_path/$iface/sg17_private"
	# ONLINE status
	link_state=`cat $conf_path/link_state`	
	if [ "$link_state" -eq "1" ]; then
	  link="online"
	else
	  link="offline"
	fi
	tip=""
	desc="Link state"
	render_input_field static "Link state" status "$link"
	# power status
	pwrovl=`cat $conf_path/pwrovl`
	pwrunb=`cat $conf_path/pwrunb`
	if [ "$pwrovl" -eq "0" ]; then
		pwrovl="no overload"
	else
		pwrovl="overload"
	fi
	if [ "$pwrunb" -eq "0" ]; then
		pwrunb="balanced"
	else
		pwrunb="unbalanced"
	fi
 	tip=""
	desc="Power balance"
	render_input_field static "Power balance" status "$pwrunb"
	tip=""
	desc="Power overload"
	render_input_field static "Power overload" status "$pwrovl"

	if [ "$link_state" -eq "1" ]; then
		# rate
		actrate=`cat $conf_path/rate`
	 	tip=""
		desc="Actual rate"
		render_input_field static "Actual Rate" status "$actrate"
		#tcpam
		actcpam=`cat $conf_path/tcpam`
		tip=""
		desc="Actual Line coding"
		render_input_field static "Actual Line code" status "$actcpam"
		#clkmode
		actclkmode=`cat $conf_path/clkmode`
		tip=""
		desc="Actual Clock mode"
		render_input_field static "Actual Clock mode" status "$actclkmode"
		# Statistics
		statistic=`cat $conf_path/statistics_row`
		set $statistic
		# SNR
		tip=""
		desc="Signal/Noise ratio margin"
		render_input_field static "SNR margin" status "$1, dB"
		# Loop Attenuation
		tip=""
		desc="Loop attenuation"
		render_input_field static "Loop attenuation" status "$2, dB"

	fi
	unset conf_path link link_state pwrovl pwrunb actrate actcpam actclkmode
}

_sg17_settings(){

	kdb_vars="\
			str:sys_dsl_${iface}_ctrl	\
			str:sys_dsl_${iface}_mode	\
			str:sys_dsl_${iface}_clkmode	\
			int:sys_dsl_${iface}_rate	\
			str:sys_dsl_${iface}_code	\
			str:sys_dsl_${iface}_annex	\
			str:sys_dsl_${iface}_crc	\
			str:sys_dsl_${iface}_fill	\
			str:sys_dsl_${iface}_inv	\
			str:sys_dsl_${iface}_pwron"	
	subsys="dsl."$iface

	eval `kdb -qq sls "sys_dsl_${iface}_" ` 
	eval "new_ctrl=\$FORM_sys_dsl_${iface}_ctrl"
	eval "new_mode=\$FORM_sys_dsl_${iface}_mode"

	if [ "$new_ctrl" != "$ctrl" ]; then
		case "$new_ctrl" in
		eocd)
			case "$new_mode" in
			master)
				opt="-m1"
				;;
			slave)
				opt="-m0"
				;;
			esac
			$eoc_config -ochannel -a$iface $opt -s
			$eoc_config -us # Save settingso to disk
			kdb set sys_dsl_${iface}_mode=$new_mode
			render_js_refresh_window 300
			;;
		manual)
			$eoc_config -ochannel -d$iface -s
			$eoc_config -us # Save settingso to disk
			render_js_refresh_window 300
			;;
		esac
	fi
	render_save_stuff
	sleep 1

	# refresh configuration
	unset 
	eval `kdb -qq ls "sys_dsl_${iface}_*" ` 
	eval `kdb -qq sls "sys_dsl_${iface}_" ` 


	render_form_header
	
	#-------------- SETTINGS table ---------------
	render_table_title "$iface "$MR17H_MODNAME" modem settings" 2


	# sys_dsl_${iface}_name
	render_input_field "hidden" "hidden" iface $iface
	render_input_field "hidden" "hidden" page settings

	# Control from eocd
	tip=""
	desc="Control type (manual or by eoc daemon)"
	render_input_field select "Control type" sys_dsl_${iface}_ctrl  manual 'Manual' eocd 'EOCd'


	if [ "$ctrl" = "eocd" ]; then
		render_submit_field
		render_form_tail
		return
	fi
	
	# sys_dsl_${iface}_mode
	tip=""
	desc="Select DSL mode"
	id='mode'
	onchange="OnChangeSG17Code();"	
	render_input_field select "Mode" sys_dsl_${iface}_mode  master 'Master' slave 'Slave'

	# sys_dsl_${iface}_clkmode
	tip=""
	desc="Select DSL clock mode"
	id='clkmode'
	onchange="OnChangeSG17Code();"	
	render_input_field select "Clock mode" sys_dsl_${iface}_clkmode  'plesio' 'plesio' 'sync' 'sync'

	# sys_dsl_${iface}_rate
	eval "crate=\$sys_dsl_${iface}_rate"
#	echo "<br>CRATE = $crate"
	tip=""
	desc="Select DSL line rate"
	validator='tmt:message="'$desc'"'
	id='rate'
	onchange="OnChangeSG17Code();"	
	render_input_field select "Rate" sys_dsl_${iface}_rate $crate $crate
	

	# sys_dsl_${iface}_code
	tip=""
	desc="Select DSL line coding"
	id='code'
	onchange="OnChangeSG17Code();"
	render_input_field select "Coding" sys_dsl_${iface}_code tcpam32 TCPAM32 tcpam16 TCPAM16 tcpam8 TCPAM8 tcpam4 TCPAM4

	# sys_dsl_${iface}_annex
	tip=""
	desc="Select DSL Annex"
	render_input_field select "Annex" sys_dsl_${iface}_annex A "Annex A" B "Annex B"

	# sys_dsl_${iface}_crc32
	tip=""
	desc="Select DSL CRC length"
	render_input_field select "CRC" sys_dsl_${iface}_crc crc32 CRC32 crc16 CRC16

	# sys_dsl_${iface}_fill
	tip=""
	desc="Select DSL fill byte value"
	render_input_field select "Fill" sys_dsl_${iface}_fill  fill_ff FF fill_7e 7E

	# sys_dsl_${iface}_inv
	tip=""
	desc="Select DSL inversion mode"
	render_input_field select "Inversion" sys_dsl_${iface}_inv  normal off invert on

	# sys_dsl_${iface}_pwron
	tip=""
	desc="Select DSL Power feeding mode"
	render_input_field select "Power" sys_dsl_${iface}_pwron  pwroff off pwron on

	render_submit_field
	render_form_tail

	run_js_code "OnChangeSG17Code();"
}


## ------------------------- MAIN -------------------------##


iface="${FORM_iface}"
if [ -z "$iface" ]; then
	for i in `kdb get sys_dsl_ifaces`; do
		if [ -z "$iface" ]; then
			iface=$i
		else
			break
		fi
	done
fi
page=${FORM_page:-status} 
unset mode rate code annex cfg crc fill inv pwron



mtype=`kdb get sys_dsl_${iface}_mtype"`

render_page_selection "iface=$iface" status "Status" settings "Settings"


case $page in
'status')	
	case $mtype in
	mr16h)
		_sg16_status $iface
		;;
	mr17h)
		_sg17_status $iface
		;;
	esac
	;;
'settings')
	
	case $mtype in
	mr16h)
		_sg16_settings $iface
		;;
	mr17h)
		_sg17_settings $iface
		;;
	esac
	;;
esac


