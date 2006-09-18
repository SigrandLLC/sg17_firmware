#!/usr/bin/haserl
	. lib/misc.sh
	. lib/widgets.sh
	
	frame=1

	case "$FORM_do" in
		del) $kdb lrm "$FORM_item";;
	esac;
	
	eval `$kdb -qqc list svc_dhcp_lanhost`
	render_form_header dhcp

	render_list_line(){
		local lineno=$1
		eval "val=\"\${svc_dhcp_lanhost_${lineno}}\""
		eval "$val"
		echo "<tr><td>$lineno</td><td>$name</td><td>$ipaddr</td><td>$hwaddr</td><td>"
		render_list_btns dhcp_static "svc_dhcp_lanhost_${lineno}"
		echo "</td></tr>"
	}
	
	
	render_list_header dhcp_static svc_dhcp_lanhost_ "" "No" "Name" "IP Address" "MAC Address"
	
	render_list_cycle_stuff

	render_form_tail
