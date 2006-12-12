#!/usr/bin/haserl
	. lib/misc.sh
	. lib/widgets.sh

#### this file is obsolete
	
	case "$FORM_do" in
		del) $kdb lrm "$FORM_item";;
	esac;
	
	eval `$kdb -qqc list sys_staticroute`
	render_form_header staticroutes
	render_title "Static routes"

	render_list_line(){
		local lineno=$1
		eval "val=\"\${sys_staticroute_${lineno}}\""
		eval "$val"
		echo "<tr><td>$lineno</td><td>$net</td><td>$netmask</td><td>$gw</td><td>"
		render_list_btns staticroute_edit "sys_staticroute_${lineno}"
		echo "</td></tr>"
	}
	
	render_list_header staticroute sys_staticroute_ "" "No" "Network" "Mask" "Gateway"
	
	i=0
	while [ $i -lt $kdb_lines_count ]; do
		render_list_line $i
		i=$(($i+1))
	done
	
	render_form_tail

