#!/usr/bin/haserl

	page=${FORM_page:-settings}
	subsys="voip"
	PORTSINFO="/proc/driver/sgatab/channels"
	PORTS_INFO_FULL=`cat $PORTSINFO`
	PORTS_INFO_FULL="SIP:SIP $PORTS_INFO_FULL"

	case $page in
		'settings')
			kdb_vars="str:sys_voip_settings_codec_ext_quality str:sys_voip_settings_codec_int_quality int:sys_voip_settings_selfnumber"
			;;
		'sip')
			kdb_vars="str:sys_voip_sip_server str:sys_voip_sip_username str:sys_voip_sip_password"
			;;
		'hotline')
			for port in $PORTS_INFO_FULL; do
				portnum=`echo $port | awk -F ':' '{print $1}'`
				kdb_vars="$kdb_vars bool:sys_voip_hotline_${portnum}_hotline"
				kdb_vars="$kdb_vars str:sys_voip_hotline_${portnum}_number"
				kdb_vars="$kdb_vars str:sys_voip_hotline_${portnum}_comment"
			done
			;;
	esac
	
	render_save_stuff

	eval `kdb -qq ls sys_voip_*`
	
	render_page_selection "" settings "Settings" sip "SIP settings" route "Route table" address "Address book" hotline "Hotline" 
	
	render_form_header
	render_input_field hidden page page "$page"

	case $page in 
		'settings')
			render_table_title "VoIP Settings"
			
			# sys_voip_selfnumber
			tip=""
			desc="Router ID"
			validator="$tmtreq $validator_voip_router_id"
			render_input_field text "Router ID" sys_voip_settings_selfnumber			
			
			# sys_voip_codec_ext_quality
			tip="Quality of calls through SIP-server"
			desc="External call quality"
			render_input_field select "External quality" sys_voip_settings_codec_ext_quality speed "Speed" medium "Medium" quality "Quality"
			
			# sys_voip_codec_int_quality
			tip="Quality of calls between routers"
			desc="Internal call quality"
			render_input_field select "Internal quality" sys_voip_settings_codec_int_quality speed "Speed" medium "Medium" quality "Quality"

			render_submit_field
			;;
		'sip')
			render_table_title "SIP settings"
			
			# sys_voip_sip_server
			tip=""
			desc="SIP server to register on"
			validator="$validator_dnsdomainoripaddr"
			render_input_field text "SIP server" sys_voip_sip_server
			
			# sys_voip_sip_username
			tip=""
			desc="Username on SIP server"
			validator=""
			render_input_field text "Username" sys_voip_sip_username
			
			# sys_voip_sip_password
			tip=""
			desc="Password on SIP server"
			validator=""
			render_input_field password "Password" sys_voip_sip_password
			
			render_submit_field
			;;
		'route')
			render_table_title "Route table"
			render_iframe_list "voip_route_table"
			;;
		'address')
			render_table_title "Address book"
			render_iframe_list "voip_address_book"
			;;
		'hotline')
			render_table_title "Hotline settings"
			echo "
				<tr><td colspan=\"2\">
				<table width=\"600px\" border=\"1\" style=\"border: solid 1px; 1px; 1px; 1px;\">
					<tr align='center'>
						<td>Channel</td><td>Type</td><td>Hotline</td>
						<td>Complete number</td><td>Comment</td>
					</tr>"

			for port in $PORTS_INFO_FULL; do
				portnum=`echo $port | awk -F ':' '{print $1}'`
				porttype=`echo $port | awk -F ':' '{print $2}'`
				
				echo "<tr><td>$portnum</td><td>$porttype</td>"

				# sys_voip_hotline_${portnum}_hotline
				tip="Enable hotline for this channel"
				validator=""
				id="sys_voip_hotline_${portnum}_hotline"
				render_input_td_field checkbox sys_voip_hotline_${portnum}_hotline

				# sys_voip_hotline_${portnum}_number
				tip=""
				validator="tmt:required='conditional' tmt:dependonbox='sys_voip_hotline_${portnum}_hotline' $validator_voip_complete_number"
				render_input_td_field $disabled text sys_voip_hotline_${portnum}_number
				
				# sys_voip_hotline_${portnum}_comment
				tip=""
				validator="$validator_comment"
				render_input_td_field $disabled text sys_voip_hotline_${portnum}_comment				
				
				echo "</tr>"
			done
			
			echo "</table></td></tr>"
			render_submit_field
			;;
	esac
	
	render_form_tail
