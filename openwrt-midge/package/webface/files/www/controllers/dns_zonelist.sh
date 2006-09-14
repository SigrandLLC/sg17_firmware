#!/usr/bin/haserl
	. lib/misc.sh
	. lib/widgets.sh
	
	frame=1

	case "$FORM_do" in
		del) $kdb lrm "$FORM_item";;
	esac;
	
	eval `$kdb -qqc list svc_dns_zonelist`
	render_form_header dns_zonelist

	render_list_line(){
		local lineno=$1
		local item="svc_dns_zonelist_${lineno}"
		eval "val=\"\${svc_dns_zonelist_${lineno}}\""
		eval "$val"
		#local href="javascript:openPopup(window, \"dns_zone\", \"$item\");"
		local href="/?controller=dns_zone&zoneid=$zoneid"
		echo "<tr><td><a href='$href' target=_parent>$zoneid</a></td><td><a href='$href' target=_parent>$zone</a></td><td>$admin</td><td>$serial</td><td>"
		render_list_btns dns_zonelist "$item"
		echo "</td></tr>"
	}
	
	
	render_list_header "Zone id" "Zone name" "Admin" "Serial"
	
	render_list_cycle_stuff

	render_button_list_add dns_zonelist svc_dns_zonelist_
	
	render_form_tail