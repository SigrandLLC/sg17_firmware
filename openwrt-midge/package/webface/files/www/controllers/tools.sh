#!/usr/bin/haserl
	. conf/conf.sh
	. lib/misc.sh
	. lib/widgets.sh

	page=${FORM_page:-ping}
	host=${FORM_host:-localhost}
	lines=${FORM_lines:-40}
	
	render_page_selection "" syslog "syslog" dmesg "dmesg" ping "ping" mtr "mtr" #dig "dig"
	
	resolve="-n"
	count="-c 5"
	packetsize="-s 100"
	if [ $REQUEST_METHOD = POST ]; then
		case $page in
		ping)
			render_console_start
			render_console_command ping $count $packetsize $host
			render_console_end
		;;
		mtr)
			render_console_start
			render_console_command /usr/sbin/mtr -r $resolve $count $packetsize $host
			render_console_end
		;;
		dig);;
		esac
	else 
		case $page in
			ping|mtr)
				render_form_header time time_save
				render_table_title "$page" 2 
				render_input_field hidden page page "$page"

				validator='tmt:required="true" tmt:message="Please input destination host" tmt:filters="ltrim,rtrim,nohtml,nospaces,nocommas,nomagic"'
				render_input_field text "Host" host 
				render_submit_field Run
				render_form_tail
			;;
			dig)
			;;
			syslog)
				render_console_start
				render_console_command /sbin/logread | tail -$lines
				render_console_end
			;;
			dmesg)
				render_console_start
				render_console_command /bin/dmesg | tail -$lines
				render_console_end
			;;
		esac
	fi
