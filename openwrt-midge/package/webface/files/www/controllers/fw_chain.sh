#!/usr/bin/haserl
	. lib/misc.sh
	. lib/widgets.sh
	
	frame=1

	table=$FORM_table
	chain=$FORM_chain
	
	case "$FORM_do" in
		del) $kdb lrm "$FORM_item";;
	esac;
	
	eval `$kdb -qqc list sys_fw_`
	render_form_header fw_filter

	render_list_line(){
		local lineno=$1
		local item="sys_fw_filter_${lineno}"
		eval "val=\"\${item}\""
		eval "$val"
		enabled_img="<img src=img/disabled.gif>"
		[ -n "$enabled" ] && enabled_img="<img src=img/enabled.gif>"
		echo "<tr><td>$lineno</td><td>$enabled_img </a></td><td><a href='$href' target=_parent>$zone</a></td><td>$admin</td><td>$serial</td><td>"
		render_list_btns dns_zonelist "$item" "table=$table&chain=$chain"
		echo '</td></tr>'
	}
	
	
	render_list_header "No" "Rule name" "Src" "Dst" "Proto" "Src port" "Dst port" "Action"
	
	render_list_cycle_stuff

	render_button_list_add fw_chain sys_fw_${table}_${chain}_ "table=$table&chain=$chain"
	
	render_form_tail
