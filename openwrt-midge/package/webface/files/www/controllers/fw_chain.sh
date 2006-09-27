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
		local target_img="<img src=img/blank.gif>"
		local style
		eval "var=\$$item"
		eval "$var"
		[  "x${enabled}x" = "xx"  ] && style="class='lineDisabled'"
		[ "$target" = "ACCEPT" ] && target_img="<img src=img/accept.gif>"
		[ "$target" = "DROP" ] && target_img="<img src=img/drop.gif>"
		[ "$target" = "REJECT" ] && target_img="<img src=img/reject.gif>"
		#[ "$target" = "SNAT" ] && target_img="<img src=img/snat.gif>"
		#[ "$target" = "DNAT" ] && target_img="<img src=img/dnat.gif>"
		
		echo "<tr $style><td>$lineno</td><td>$name</td><td>$src</td><td>$dst</td><td>$proto</td><td>$sport</td><td>$dport</td><td>$target_img $target</td><td>"
		render_list_btns fw_chain "$item" "table=$table&chain=$chain"
		echo '</td></tr>'
	}
	
	
	render_list_header fw_chain sys_fw_${table}_${chain}_ "table=$table&chain=$chain" "No" "Rule name" "Src" "Dst" "Proto" "Src port" "Dst port" "Action"
	
	render_list_cycle_stuff

	render_form_tail
