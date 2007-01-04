
L1(){
	[ -n "$2" ] && echo "<strong><a href='?controller=$2'>$1</a></strong><br>" \
			|| echo "<strong>$1</strong><br>"
}

L2(){
	local class=navlnk
	[ -n "$2" ] && echo "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<a href='?controller=$2' class='$class'>$1</a><br>" \
		|| echo "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<span class='$class'>$1</span><br>"
}

L3(){
	local class=navlnk
	[ -n "$3" ] && class="$3"
	echo "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<a href='?controller=$2' class='$class'>$1</a><br>"
}

L1 System welcome
	L2 General 	general
	L2 Time		time
	L2 SHDSL	dsl
	L2 DNS		dns
	
L1 Network
	L2 Interfaces ifaces
	for i in `kdb get sys_ifaces`; do
		class=""
		[ "$FORM_iface" = "$i" ] && class="navlnk_a"
		L3	$i "iface&iface=$i" $class
	done
	L2 Firewall	fw
		L3 Filter	"fw&table=filter"
		L3 NAT		"fw&table=nat"
	L2 IPSec ipsec
	
L1 Services
	L2 "DNS Server" dns_server
L1 Tools
	L2 syslog	"tools&page=syslog"
	L2 dmesg	"tools&page=dmesg"
	L2 ping	"tools&page=ping"
	L2 mtr	"tools&page=mtr"
#	L2 dig	"tools&page=dig"
	L2 password	"passwd"

#	<a href="javascript:showhide('diag','tri_diag')">
#					<img src="img/tri_c.gif" id="tri_diag" width="14" height="10" border="0"></a><strong><a href="javascript:showhide('diag','tri_diag')" class="navlnk">Diagnostics</a></strong><br>
#					<span id="diag"></span>
