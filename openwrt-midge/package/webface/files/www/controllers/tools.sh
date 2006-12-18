#!/usr/bin/haserl
	. conf/conf.sh
	. lib/misc.sh
	. lib/widgets.sh

	page=${FORM_page:-ping}
	host=${FORM_host:-localhost}
	lines=${FORM_lines:-40}
	count=${FORM_count:-5}
	
	render_page_selection "" syslog "syslog" dmesg "dmesg" ping "ping" mtr "mtr" #dig "dig"
	

	render_form_header 
	if [ $REQUEST_METHOD = POST ]; then
		#render_table_title "$page" 2 
		resolve="-n"
		count="-c $count"
		packetsize="-s 100"
		case $page in
		ping)
			render_console_start "$page"
			render_console_command ping $count $packetsize $host
			render_console_end
		;;
		mtr)
			render_console_start "$page"
			render_console_command /usr/sbin/mtr -r $resolve $count $packetsize $host
			render_console_end
		;;
		dig)
			# TODO
		;;
		esac

	else 
		render_table_title "$page" 2 
		render_input_field hidden page page "$page"
		case $page in
			ping|mtr)
				validator='tmt:required="true" tmt:message="Please input destination host" tmt:filters="ltrim,rtrim,nohtml,nospaces,nocommas,nomagic"'
				render_input_field text "Host" host 
				default="5"
				validator='tmt:filters="ltrim,rtrim,nohtml,nocommas,nomagic" tmt:pattern="positiveinteger" tmt:minnumber=1 tmt:maxnumber=50  tmt:message="Please input number"'
				render_input_field text "Count" count

				render_submit_field "Run"
			;;
			dig)
				# TODO
			;;
			syslog)
				render_console_start
				render_console_command /sbin/logread | tail -$lines | colorizelog
				render_console_end
			;;
			dmesg)
				render_console_start
				render_console_command /bin/dmesg | tail -$lines
				render_console_end
			;;
		esac
	fi
	render_form_tail

