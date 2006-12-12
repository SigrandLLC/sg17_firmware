#!/usr/bin/haserl
	. conf/conf.sh
	. lib/misc.sh
	. lib/widgets.sh

	page=${FORM_page:-ping}
	host=${FORM_host:-localhost}
	
	render_page_selection "" ping "ping" mtr "mtr" #dig "dig"
	
	resolve="-n"
	count="-c 5"
	packetsize="-s 100"
	if [ $REQUEST_METHOD = POST ]; then
		case $page in
		ping)
				render_console_start
				render_console_command ping $resolve $count $packetsize $host
				render_console_end
			;;
		mtr)
				render_console_start
				render_console_command /usr/bin/mtr -r $resolve $count $packetsize $host
				render_console_end
			;;
		dig);;
		esac
	fi
	
	case $page in
	ping|mtr)
	
	;;
	dig)
	;;
	esac
