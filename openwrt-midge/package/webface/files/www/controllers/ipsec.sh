#!/usr/bin/haserl
	. conf/conf.sh
	. lib/misc.sh
	. lib/widgets.sh


	page=${FORM_page:-status}
	subsys="ipsec"
	setkey=/usr/sbin/setkey

	case "$FORM_do" in
		del) 
				$kdb lrm "$FORM_item"
				debug $kdb lrm "$FORM_item"
			;;
	esac;

		
	eval `$kdb -qq ls sys_ipsec `
	
	
	render_page_selection "" status "Status" ah "AH" esp "ESP" policy "Policy" random "Random keys" #"Authentication header" esp "Encapsulated Security Payload" 
	
	render_form_header

	case $page in 
	'status')
		render_console_start "Security Association Database" 2 
		render_console_command $setkey -D
		render_console_end
		render_console_start "Security Policy Database" 2 
		render_console_command $setkey -DP
		render_console_end
		;;
	'ah')
		render_table_title "Authentication header" 2 
		render_iframe_list "ipsec_table" "table=ah"
		;;
	'esp')
		render_table_title "Encapsulated Security Payload" 2 
		render_iframe_list "ipsec_table" "table=esp"
		;;
	'policy')
		render_table_title "Security Policy Database" 2 
		render_iframe_list "ipsec_table" "table=policy"
		;;
	'random')
		render_console_start "Random 64 bit key" 2 
		dd if=/dev/urandom bs=1 count=8 2>/dev/null | hexdump -e "16/2 \"%04x\"\"\\n\""
		render_console_end

		render_console_start "Random 96 bit key" 2 
		dd if=/dev/urandom bs=1 count=12 2>/dev/null | hexdump -e "16/2 \"%04x\"\"\\n\""
		render_console_end

		render_console_start "Random 128 bit key" 2 
		dd if=/dev/urandom bs=1 count=16 2>/dev/null | hexdump -e "16/2 \"%04x\"\"\\n\""
		render_console_end

		render_console_start "Random 192 bit key" 2 
		dd if=/dev/urandom bs=1 count=24 2>/dev/null | hexdump -e "16/2 \"%04x\"\"\\n\""
		render_console_end

		render_console_start "Random 256 bit key" 2 
		dd if=/dev/urandom bs=1 count=32 2>/dev/null | hexdump -e "16/2 \"%04x\"\"\\n\""
		render_console_end
		;;
	esac
	
	render_form_tail
