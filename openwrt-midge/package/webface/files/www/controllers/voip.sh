#!/usr/bin/haserl

	page=${FORM_page:-settings}
	subsys="voip"
	PORTSINFO="/proc/driver/sgatab/channels"
	PORTS_INFO_FULL=`cat $PORTSINFO`
	PORTS_INFO_FULL="SIP:SIP $PORTS_INFO_FULL"

	case $page in
		'settings')
			kdb_vars="str:sys_voip_settings_codec_ext_quality str:sys_voip_settings_codec_int_quality int:sys_voip_settings_selfnumber str:sys_voip_settings_selfip str:sys_voip_settings_log"
			;;
		'sip')
			kdb_vars="str:sys_voip_sip_registrar str:sys_voip_sip_username str:sys_voip_sip_password str:sys_voip_sip_user_sip_uri int:sys_voip_sip_expires"
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
			help_1="voip.settings"
			help_2=""
			render_table_title "VoIP Settings"
			
			# sys_voip_settings_selfnumber
			tip=""
			desc="Router ID"
			validator="$tmtreq $validator_voip_router_id"
			render_input_field text "Router ID" sys_voip_settings_selfnumber			
			
			# sys_voip_settings_selfip
			tip=""
			desc="Router IP"
			validator="$tmtreq $validator_ipaddr"
			render_input_field text "Router IP" sys_voip_settings_selfip						
			
			# sys_voip_settings_codec_ext_quality
			tip="Quality of calls through SIP-server"
			desc="External call quality"
			render_input_field select "External quality" sys_voip_settings_codec_ext_quality speed "Speed" medium "Medium" quality "Quality"
			
			# sys_voip_settings_codec_int_quality
			tip="Quality of calls between routers"
			desc="Internal call quality"
			render_input_field select "Internal quality" sys_voip_settings_codec_int_quality speed "Speed" medium "Medium" quality "Quality"

			# sys_voip_settings_log
			tip=""
			desc="Level of logging"
			render_input_field select "Logging level" sys_voip_settings_log 0 "0" 1 "1" 2 "2" 3 "3" 4 "4" 5 "5" 6 "6" 7 "7" 8 "8" 9 "9"

			render_submit_field
			;;
		'sip')
			help_1="voip.sip"
			help_2=""
			render_table_title "SIP settings"
			
			# sys_voip_sip_registrar
			tip="f.e. <b>sip:server</b>"
			desc="SIP registrar to register on"
			validator="$tmtreq $validator_voip_registrar"
			render_input_field text "Registrar" sys_voip_sip_registrar
			
			# sys_voip_sip_username
			tip="f.e <b>user</b>"
			desc="Username on SIP registrar"
			validator="$tmtreq tmt:message='Please enter username'"
			render_input_field text "Username" sys_voip_sip_username
			
			# sys_voip_sip_password
			tip=""
			desc="Password on SIP registrar"
			validator="$tmtreq tmt:message='Please enter password'"
			render_input_field password "Password" sys_voip_sip_password
			
			# sys_voip_sip_user_sip_uri
			tip="f.e. <b>sip:user@server</b>"
			desc="User SIP URI"
			validator="$tmtreq $validator_voip_sip_uri"
			render_input_field text "User SIP URI" sys_voip_sip_user_sip_uri
			
			# sys_voip_sip_expires
			tip=""
			desc="Registration expiration"
			validator="$validator_voip_sip_expires"
			render_input_field text "Expires" sys_voip_sip_expires
			
			render_submit_field
			;;
		'route')
			help_1="voip.route"
			help_2=""
			render_table_title "Route table"
			render_iframe_list "voip_route_table"
			;;
		'address')
			help_1="voip.address"
			help_2=""
			render_table_title "Address book"
			render_iframe_list "voip_address_book"
			;;
		'hotline')
			help_1="voip.hotline"
			help_2=""
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
