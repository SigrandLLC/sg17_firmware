#!/usr/bin/haserl
	. lib/misc.sh
	. lib/widgets.sh
	
	frame=1

	table=$FORM_table
	chain=$FORM_chain
	
	case "$FORM_do" in
		del) $kdb lrm "$FORM_item";;
	esac;
	
	eval `$kdb -qqc list sys_fw_${table}_${chain}_`
	render_form_header fw_${table}_${chain}

	render_list_line(){
		local lineno=$1
		local item="sys_fw_${table}_${chain}_${lineno}"
		local enabled_img="<img src=img/disabled.gif>"
		local target_img="<img src=img/disabled.gif>"
		eval "var=\$$item"
		eval "$var"
		[ -n "$enabled" ] && enabled_img="<img src=img/enabled.gif>"
		
		echo "<tr><td>$lineno</td><td>$enabled_img </a>$name</td><td>$src</td><td>$dst</td><td>$proto</td><td>$sport</td><td>$dport</td><td>$target</td><td>"
		render_list_btns fw_chain "$item" "table=$table&chain=$chain"
		echo '</td></tr>'
	}
	
	
	render_list_header fw_chain sys_fw_${table}_${chain}_ "table=$table&chain=$chain" "No" "Rule name" "Src" "Dst" "Proto" "Src port" "Dst port" "Action"
	
	render_list_cycle_stuff

	render_form_tail
