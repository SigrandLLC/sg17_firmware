#!/usr/bin/haserl
	. lib/misc.sh
	. lib/widgets.sh
	
	frame=1

	table=$FORM_table
	
	case "$FORM_do" in
		del) $kdb lrm "$FORM_item";;
	esac;
	
	eval `$kdb -qqc list sys_ipsec_${table}_`
	render_form_header ipsec_${table}

	case $table in
	ah|esp)
		render_list_header ipsec_table sys_ipsec_${table}_ "table=$table" "SPI" "Src" "Dst" "Algorithm" "Key"

		render_list_line(){
			local lineno=$1
			local item="sys_ipsec_${table}_${lineno}"
			local target_img="<img src=img/blank.gif>"
			local style
			eval "var=\$$item"
			eval "$var"
			[  "x${enabled}x" = "xx"  ] && style="class='lineDisabled'"
			
			echo "<tr $style><td>$spi</td><td>$src</td><td>$dst</td><td>$algo</td><td>$key</td><td>"
			render_list_btns ipsec_table_edit "$item" "table=$table"
			echo '</td></tr>'
		}
		;;
	policy)
		;;
	esac
	
	
	render_list_cycle_stuff

	render_form_tail
