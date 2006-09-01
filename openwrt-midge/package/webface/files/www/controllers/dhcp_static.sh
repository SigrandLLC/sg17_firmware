#!/usr/bin/haserl
	. lib/misc.sh
	. lib/widgets.sh

	case "$FORM_do" in
		del) $kdb lrm "$FORM_item"
			;;
	esac;
	
	eval `$kdb -qqc list sys_dhcp_lanhost`
	render_form_header dhcp

	render_list_line(){
		local lineno=$1
		eval "val=\"\${sys_dhcp_lanhost_${lineno}}\""
		eval "$val"
		echo "<tr><td>$lineno</td><td>$name</td><td>$ipaddr</td><td>$hwaddr</td><td>"
		render_list_btns dhcp_static "sys_dhcp_lanhost_${lineno}"
		echo "</td></tr>"
	}
	
	
	render_list_header "No" "Name" "IP Address" "MAC Address"
	
	i=0
	while [ $i -lt $kdb_lines_count ]; do
		render_list_line $i
		i=$(($i+1))
	done
	render_button_list_add dhcp_static sys_dhcp_lanhost_
	
	render_form_tail
