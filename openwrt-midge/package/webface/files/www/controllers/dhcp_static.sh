#!/usr/bin/haserl
	. lib/misc.sh
	. lib/widgets.sh
	
	frame=1

	export iface=${FORM_iface:-eth0}

	case "$FORM_do" in
		del) $kdb lrm "$FORM_item";;
	esac;
	
	eval `$kdb -qqc list sys_iface_${iface}_dhcp_host`
	render_form_header dhcp

	render_list_line(){
		local lineno=$1
		eval "val=\"\${sys_iface_${iface}_dhcp_host_${lineno}}\""
		eval "$val"
		echo "<tr><td>$lineno</td><td>$name</td><td>$ipaddr</td><td>$hwaddr</td><td>"
		render_list_btns dhcp_static "sys_iface_${iface}_dhcp_host_${lineno}"
		echo "</td></tr>"
	}
	
	
	render_list_header dhcp_static sys_iface_${iface}_dhcp_host_ "" "No" "Name" "IP Address" "MAC Address"
	
	render_list_cycle_stuff

	render_form_tail
