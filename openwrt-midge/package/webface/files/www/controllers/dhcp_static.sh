#!/usr/bin/haserl
	. lib/misc.sh
	. lib/widgets.sh

	case "$FORM_do" in
		del) $kdb lrm $FORM_item
			;;
	esac;
	
	eval `$kdb -qqc list sys_dhcp_lanhost`
	render_form_header dhcp

	render_list_btns(){
		local item=$2
		
		# edit
		echo "<a href='javascript:popupWin = window.open(\"index.cgi?controller=dhcp_static_edit&item=${item}&popup=1\", \"popup\", \"width=400,height=300,top=0,modal=1,dialog=1,centerscreen=1,scrollbars=0,menubar=0,location=0,toolbar=0,dependent=1,status=0\"); popupWin.focus();'>Edit</a>"
		
		# del
		echo "<a href='index.cgi?controller=dhcp_static&do=del&item=${item}&frame=1' target='_self' xonclick='if (confirm(\"Are you sure?\")) { var f = document.createElement(\"form\"); f.style.display = \"none\"; this.parentNode.appendChild(f); f.method = \"GET\"; f.action = this.href;f.submit(); };return false;'>Destroy</a>"
	
	}
	
	render_list_line(){
		local lineno=$1
		eval "val=\"\${sys_dhcp_lanhost_${lineno}}\""
		eval "$val"
		echo "<tr><td>$lineno</td><td>$name</td><td>$ipaddr</td><td>$hwaddr</td><td>"
		render_list_btns dhcp_static_edit sys_dhcp_lanhost_${lineno}
		echo "</td></tr>"
	}
	
	render_list_header(){
		local s1="<tr>"
		local s2="<td class='listtopic'>"
		local s3="</td>"
		local s4="</tr>"
		
		echo $s1
		for n in "$@"; do
			echo $s2 $n $s3
		done
		echo $s2 Action $s3
		echo $s4
	}
	
	render_list_header "No" "Name" "IP Address" "MAC Address"
	
	i=0
	while [ $i -lt $kdb_lines_count ]; do
		render_list_line $i
		i=$(($i+1))
	done
	
	render_form_tail
