
L1(){
	[ -n "$2" ] && echo "<strong><a href='?controller=$2'>$1</a></strong><br>" \
			|| echo "<strong>$1</strong><br>"
}

L2(){
	echo "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<a href='?controller=$2&action=$3' class='navlnk'>$1</a><br>"
}

L1 System welcome
	L2 General 	general show
	L2 Time		time	show
	L2 Modem	dsl		show
	
L1 Network
	L2 WAN		wan		show
	L2 LAN		lan		show
	L2 Firewall	firewall show
	L2 "Static routes"	staticroute show

L1 Services
	L2 DHCP		dhcp	show
	L2 DNS		dns 	show

#	<a href="javascript:showhide('diag','tri_diag')">
#					<img src="img/tri_c.gif" id="tri_diag" width="14" height="10" border="0"></a><strong><a href="javascript:showhide('diag','tri_diag')" class="navlnk">Diagnostics</a></strong><br>
#					<span id="diag"></span>
