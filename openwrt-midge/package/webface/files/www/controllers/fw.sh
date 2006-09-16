#!/usr/bin/haserl
	. conf/conf.sh
	. lib/misc.sh
	. lib/widgets.sh

	if [ -z "$FORM_table" ]; then
		kdb_vars="int:svc_dns_tcpclients str:svc_dns_enable"
		#kdb_vars="$kdb_vars "
		subsys=""

		render_title "Firewall settings"

		render_save_stuff

		eval `$kdb -qq ls sys_fw`

		render_form_header fw 

		# sys_fw_enable
		tip=""
		desc="Check this item if you want use firewall on your router"
		validator='tmt:required="true"'
		render_input_field checkbox "Enable Firewall" sys_fw_enable

		render_submit_field
		render_form_tail
	elif [ "$FORM_table" = "filter" ]; then
		# fw forward list
		render_form_header fw_filter_forward
		render_table_title "FORWARD" 2 
		render_iframe_list "fw_chain" "table=filter&chain=forward"
		render_form_tail

		# fw input list
		render_form_header fw_filter_input
		render_table_title "INPUT" 2 
		render_iframe_list "fw_chain" "table=filter&chain=input"
		render_form_tail

		# fw output list
		render_form_header fw_filter_output
		render_table_title "OUTPUT" 2 
		render_iframe_list "fw_chain" "table=filter&chain=output"
		render_form_tail
	
	elif [ "$FORM_table" = "nat" ]; then
		# fw prerouting list
		render_form_header fw_nat_prerouting
		render_table_title "PREROUTING" 2 
		render_iframe_list "fw_chain" "table=nat&chain=prerouting"
		render_form_tail
		
		# fw postrouting list
		render_form_header fw_nat_postrouting
		render_table_title "POSTROUTING" 2 
		render_iframe_list "fw_chain" "table=nat&chain=postrouting"
		render_form_tail
		
	fi



