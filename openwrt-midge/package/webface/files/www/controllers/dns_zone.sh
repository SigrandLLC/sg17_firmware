#!/usr/bin/haserl
	. lib/misc.sh
	. lib/widgets.sh
	
	zoneid=$FORM_zoneid
	
	case "$FORM_do" in
		del) $kdb lrm "$FORM_item";;
	esac;
	
	eval `$kdb -qqc list svc_dns_zone_$zoneid`
	render_form_header dns_zone

	render_list_line(){
		local lineno=$1
		local item="svc_dns_zone_${zoneid}_${lineno}"
		eval "val=\"\$${item}\""
		eval "$val"
		[ "$datatype" != "MX" ] && prio=""
		echo "<tr><td>$lineno</td><td>$domain</td><td>$datatype</td><td>$prio</td><td>$data</td><td>"
		render_list_btns dns_zone "$item" "zoneid=$zoneid"
		echo "</td></tr>"
	}
	
	
	render_list_header "No" "Domain" "Type" "Priority" "Data"
	
	render_list_cycle_stuff

	render_button_list_add dns_zone svc_dns_zone_${zoneid}_
	
	render_form_tail
