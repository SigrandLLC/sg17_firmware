
L1(){
	[ -n "$2" ] && echo "<strong><a href='?controller=$2'>$1</a></strong><br>" \
			|| echo "<strong>$1</strong><br>"
}

L2(){
	echo "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<a href='?controller=$2' class='navlnk'>$1</a><br>"
}

L3(){
	echo "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<a href='?controller=$2' class='navlnk'>$1</a><br>"
}

L1 System welcome
	L2 General 	general
	L2 Time		time
	L2 SHDSL	dsl
	L2 DNS		dns
	
L1 Network
	L2 Interfaces
		L3	dsl0 	"iface&iface=dsl0"
		L3	eth0 	"iface&iface=eth0"
		L3	eth1 	"iface&iface=eth1"
		L3	br0 	"iface&iface=br0"
		L3	ipsec0 	"iface&iface=ipsec0"
		L3	bond0 	"iface&iface=bond0"
		L3	pppoe0 	"iface&iface=pppoe0"
		L3	pptp0 	"iface&iface=pptp0"
	L2 Firewall	fw
		L3 Filter	"fw&table=filter"
		L3 NAT		"fw&table=nat"
	L2 "Static routes"	staticroute

L1 Services
	L2 DHCP		dhcp
	L2 "DNS Server" dns_server

#	<a href="javascript:showhide('diag','tri_diag')">
#					<img src="img/tri_c.gif" id="tri_diag" width="14" height="10" border="0"></a><strong><a href="javascript:showhide('diag','tri_diag')" class="navlnk">Diagnostics</a></strong><br>
#					<span id="diag"></span>
