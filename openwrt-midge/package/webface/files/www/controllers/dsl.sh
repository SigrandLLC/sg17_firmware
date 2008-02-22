#!/usr/bin/haserl

eoc_config=/sbin/eoc-config
eoc_info=/sbin/eoc-info

. /www/oem.sh

require_js_file "prototype.js"
require_js_file "dsl.js"

_sg16_status(){

	cpath="$sg16_cfg_path/$iface"
	help_2="dsl.status"
	render_table_title "$iface (module $MR16H_MODNAME) status" 2
	# ONLINE status
	
	link=`cat $cpath/state`
	tip=""
	desc="Link state"
	render_input_field static "Link state" status "$link"
}


_sg16_settings(){
	local iface=$1
	local slot=$2
	local dev=$3

	kdb_vars="  int:sys_pcicfg_s${slot}_${dev}_rate	\
			str:sys_pcicfg_s${slot}_${dev}_mode	\
			str:sys_pcicfg_s${slot}_${dev}_code	\
			str:sys_pcicfg_s${slot}_${dev}_cfg	\
			str:sys_pcicfg_s${slot}_${dev}_annex	\
			str:sys_pcicfg_s${slot}_${dev}_crc	\
			str:sys_pcicfg_s${slot}_${dev}_fill	\
			str:sys_pcicfg_s${slot}_${dev}_inv"	
	subsys="dsl."$slot"."$dev

	render_save_stuff
	sleep 1

	render_form_header
	# refresh configuration
	eval `kdb -qq ls "sys_pcicfg_s${slot}_${dev}*" ` 
	render_table_title "$iface (module $MR16H_MODNAME) settings" 2

	# sys_dsl_${iface}_name
	render_input_field "hidden" "hidden" iface $iface
	render_input_field "hidden" "hidden" pcislot "$slot"
	render_input_field "hidden" "hidden" pcidev "$dev"
	render_input_field "hidden" "hidden" page settings


	# sys_dsl_${iface}_mode
	tip=""
	desc="Select DSL mode"
	id='mode'
	onchange="OnChangeSG16Code();"	
	render_input_field select "Mode" sys_pcicfg_s${slot}_${dev}_mode  master 'Master' slave 'Slave'

	unset crate
	eval "crate=\$sys_pcicfg_s${slot}_${dev}_rate"
	tip=""
	desc="Select DSL line rate"
	validator='tmt:message="'$desc'"'
	id='rate'
	onchange="OnChangeSG16Code();"	
	render_input_field select "Rate" sys_pcicfg_s${slot}_${dev}_rate $crate $crate

	# sys_pcicfg_s${slot}_${dev}_code
	tip=""
	desc="Select DSL line coding"
	id='code'
	onchange="OnChangeSG16Code();"
	render_input_field select "Coding" sys_pcicfg_s${slot}_${dev}_code tcpam32 TCPAM32 tcpam16 TCPAM16 tcpam8 TCPAM8 tcpam4 TCPAM4

	# sys_pcicfg_s${slot}_${dev}_cfg
	tip=""
	desc="Select DSL configuration mode"
	id='cfg'
	onchange="OnChangeSG16Code();"	
	render_input_field select "Config" sys_pcicfg_s${slot}_${dev}_cfg local local preact preact

	# sys_pcicfg_s${slot}_${dev}_annex
	tip=""
	desc="Select DSL Annex"
	id='annex'
	onchange="OnChangeSG16Code();"	
	render_input_field select "Annex" sys_pcicfg_s${slot}_${dev}_annex A "Annex A" B "Annex B" F "Annex F"

	# sys_pcicfg_s${slot}_${dev}_crc32
	tip=""
	desc="Select DSL CRC length"
	render_input_field select "CRC" sys_pcicfg_s${slot}_${dev}_crc crc32 CRC32 crc16 CRC16

	# sys_pcicfg_s${slot}_${dev}_fill
	tip=""
	desc="Select DSL fill byte value"
	render_input_field select "Fill" sys_pcicfg_s${slot}_${dev}_fill  fill_ff FF fill_7e 7E

	# sys_pcicfg_s${slot}_${dev}_inv
	tip=""
	desc="Select DSL inversion mode"
	render_input_field select "Inversion" sys_pcicfg_s${slot}_${dev}_inv  normal off invert on

	render_submit_field
	render_form_tail

	run_js_code "OnChangeSG16Code();"
}


_sg17_status(){

	#-------------- STATUS table --------------------
	help_2="dsl.status"
	render_table_title "$iface (module $MR17H_MODNAME) status" 2	
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
		render_input_field static "SNR margin" status "$1 dB"
		# Loop Attenuation
		tip=""
		desc="Loop attenuation"
		render_input_field static "Loop attenuation" status "$2 dB"

	fi
	unset conf_path link link_state pwrovl pwrunb actrate actcpam actclkmode
}

_sg17_settings(){
	local iface=$1
	local slot=$2
	local dev=$3

	kdb_vars="\
			str:sys_pcicfg_s${slot}_${dev}_ctrl	\
			str:sys_pcicfg_s${slot}_${dev}_mode	\
			str:sys_pcicfg_s${slot}_${dev}_clkmode	\
			int:sys_pcicfg_s${slot}_${dev}_ratetype	\
			int:sys_pcicfg_s${slot}_${dev}_rate	\
			int:sys_pcicfg_s${slot}_${dev}_mrate	\
			str:sys_pcicfg_s${slot}_${dev}_code	\
			str:sys_pcicfg_s${slot}_${dev}_annex	\
			str:sys_pcicfg_s${slot}_${dev}_crc	\
			str:sys_pcicfg_s${slot}_${dev}_fill	\
			str:sys_pcicfg_s${slot}_${dev}_inv	\
			str:sys_pcicfg_s${slot}_${dev}_pwron"	
	subsys="dsl."$slot"."$dev

	eval `kdb -qq sls "sys_pcicfg_s${slot}_${dev}_" ` 
	eval "new_ctrl=\$FORM_sys_pcicfg_s${slot}_${dev}_ctrl"
	eval "new_mode=\$FORM_sys_pcicfg_s${slot}_${dev}_mode"

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
			kdb set sys_pcicfg_s${slot}_${dev}_mode=$new_mode
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
	eval `kdb -qq ls "sys_pcicfg_s${slot}_${dev}_*" ` 
	eval `kdb -qq sls "sys_pcicfg_s${slot}_${dev}_" ` 


	render_form_header
	
	#-------------- SETTINGS table ---------------
	render_table_title "$iface (module $MR17H_MODNAME) settings" 2
	
	# get device type
	tmp=`cat /sys/class/net/$iface/sg17_private/chipver`
	
	# sys_dsl_${iface}_name
	render_input_field "hidden" "hidden" iface $iface
	render_input_field "hidden" "hidden" pcislot "$slot"
	render_input_field "hidden" "hidden" pcidev "$dev"
	render_input_field "hidden" "hidden" page settings
	id='chipver'
	render_input_field "hidden" "hidden" chipver "$tmp"

	# Control from eocd
	tip=""
	desc="Control type (manual or by eoc daemon)"
	render_input_field select "Control type" sys_pcicfg_s${slot}_${dev}_ctrl  manual 'Manual' eocd 'EOCd'


	if [ "$ctrl" = "eocd" ]; then
		render_submit_field
		render_form_tail
		return
	fi
	
	# sys_pcicfg_s${slot}_${dev}_mode
	tip=""
	desc="Select DSL mode"
	id='mode'
	onchange="OnChangeSG17Code();"	
	render_input_field select "Mode" sys_pcicfg_s${slot}_${dev}_mode  master 'Master' slave 'Slave'

	# sys_pcicfg_s${slot}_${dev}_clkmode
	tip=""
	desc="Select DSL clock mode"
	id='clkmode'
	onchange="OnChangeSG17Code();"	
	render_input_field select "Clock mode" sys_pcicfg_s${slot}_${dev}_clkmode  'plesio' 'plesio' 'sync' 'sync'

	# sys_pcicfg_s${slot}_${dev}_ratetype
	eval "ratetype=\$sys_pcicfg_s${slot}_${dev}_ratetype"
	tip=""
	desc="Select DSL line rate"
	validator='tmt:message="'$desc'"'
	id='ratetype'
	onchange="OnChangeSG17Code();"	
	render_input_field checkbox "Manual rate" sys_pcicfg_s${slot}_${dev}_ratetype $ratetype


	# sys_pcicfg_s${slot}_${dev}_rate
	eval "crate=\$sys_pcicfg_s${slot}_${dev}_rate"
	tip=""
	desc="Select DSL line rate"
	validator='tmt:message="'$desc'"'
	id='rate'
	onchange="OnChangeSG17Code();"	
	render_input_field select "Rate" sys_pcicfg_s${slot}_${dev}_rate $crate $crate

	# sys_pcicfg_s${slot}_${dev}_rate
	eval "mrate=\$sys_pcicfg_s${slot}_${dev}_mrate"
	tip=""
	desc="Select DSL line rate"
	validator='tmt:message="'$desc'"'
	id='mrate'
	onchange="OnChangeSG17Code();"	
	render_input_field text "Manual rate" sys_pcicfg_s${slot}_${dev}_mrate $mrate $mrate
	

	# sys_pcicfg_s${slot}_${dev}_code
	eval "ctcpam=\$sys_pcicfg_s${slot}_${dev}_code"
	[ -z "$ctcpam" ] && ctcpam=tcpam32
	tip=""
	desc="Select DSL line coding"
	id='code'
	onchange="OnChangeSG17Code();"
	render_input_field select "Coding" sys_pcicfg_s${slot}_${dev}_code "$ctcpam" "$ctcpam"

	# sys_pcicfg_s${slot}_${dev}_annex
	tip=""
	desc="Select DSL Annex"
	id='annex'
	render_input_field select "Annex" sys_pcicfg_s${slot}_${dev}_annex A "Annex A" B "Annex B"

	# sys_pcicfg_s${slot}_${dev}_crc32
	tip=""
	desc="Select DSL CRC length"
	render_input_field select "CRC" sys_pcicfg_s${slot}_${dev}_crc crc32 CRC32 crc16 CRC16

	# sys_pcicfg_s${slot}_${dev}_fill
	tip=""
	desc="Select DSL fill byte value"
	render_input_field select "Fill" sys_pcicfg_s${slot}_${dev}_fill  fill_ff FF fill_7e 7E

	# sys_pcicfg_s${slot}_${dev}_pwron
	tip=""
	desc="Select DSL Power feeding mode"
	render_input_field select "Power" sys_pcicfg_s${slot}_${dev}_pwron  pwroff off pwron on

	render_submit_field
	render_form_tail

	run_js_code "OnChangeSG17Code();"
}


## ------------------------- MAIN -------------------------##


iface="${FORM_iface}"
slot="${FORM_pcislot}"
dev="${FORM_pcidev}"
if [ -z "$iface" ]; then
	for s in `kdb get sys_pcitbl_slots`; do
		type=`kdb get sys_pcitbl_s${s}_iftype`
		if [ "$type" != "$MR16H_DRVNAME" ] && [ "$type" != "$MR17H_DRVNAME" ];
		then
			continue
		fi
		slot=$s
		dev=0
		iface=`kdb get sys_pcitbl_s${s}_ifaces | awk '{print $(1)}'`
		break
	done
fi
page=${FORM_page:-status} 
unset mode rate code annex cfg crc fill inv pwron

mtype=`kdb get sys_pcitbl_s${slot}_iftype"`

render_page_selection "iface=$iface&pcislot=$slot&pcidev=$dev" \
		status "Status" settings "Settings"



case $page in
'status')	
	case $mtype in
	$MR16H_DRVNAME)
		_sg16_status $iface
		;;
	$MR17H_DRVNAME)
		_sg17_status $iface
		;;
	esac
	;;
'settings')
	case $mtype in
	$MR16H_DRVNAME)
		_sg16_settings $iface $slot $dev
		;;
	$MR17H_DRVNAME)
		_sg17_settings $iface $slot $dev
		;;
	esac
	;;
esac


