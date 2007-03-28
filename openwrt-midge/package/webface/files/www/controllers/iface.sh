#!/usr/bin/haserl

	export iface=${FORM_iface:-eth0}
	page=${FORM_page:-status}
	subsys="network"

	handle_list_del_item
		
	eval `kdb -qq ls "sys_iface_${iface}_*" : sls "sys_iface_${iface}_" `
	
	
	# enable DHCP page only on "ether" interfaces
	dhcp_page=""
	case $proto in
		ether|bridge|bonding|vlan) dhcp_page="dhcp DHCP";;
	esac
		
	render_page_selection "iface=$iface" status Status general "General" method "Method" options "Options" spec "Specific" $dhcp_page qos "QoS" routes "Routes"
	
	case $page in
		'general')	
			kdb_vars="bool:sys_iface_${iface}_enabled bool:sys_iface_${iface}_auto str:sys_iface_${iface}_method " ;;
		'method')
			[ "$method" = "static" ] && kdb_vars="str:sys_iface_${iface}_ipaddr str:sys_iface_${iface}_netmask str:sys_iface_${iface}_gateway str:sys_iface_${iface}_broadcast"
			[ "$method" = "dynamic" ] && kdb_vars="str:sys_iface_${iface}_dynhostname"
			[ "$proto" = "hdlc" ] && kdb_vars="$kdb_vars str:sys_iface_${iface}_pointopoint"
			;;
		'options')	
			kdb_vars="bool:sys_iface_${iface}_opt_accept_redirects bool:sys_iface_${iface}_opt_forwarding bool:sys_iface_${iface}_opt_proxy_arp bool:sys_iface_${iface}_opt_rp_filter"
			;;
		'dhcp')
			kdb_vars="bool:sys_iface_${iface}_dhcp_enabled int:sys_iface_${iface}_dhcp_lease_time"
			kdb_vars="$kdb_vars str:sys_iface_${iface}_dhcp_router str:sys_iface_${iface}_dhcp_nameserver"
			kdb_vars="$kdb_vars str:sys_iface_${iface}_dhcp_domain_name "
			kdb_vars="$kdb_vars str:sys_iface_${iface}_dhcp_ntpserver str:sys_iface_${iface}_dhcp_winsserver "
			kdb_vars="$kdb_vars str:sys_iface_${iface}_dhcp_startip str:sys_iface_${iface}_dhcp_endip str:sys_iface_${iface}_dhcp_netmask "
			subsys="dhcp"
		;;
		'qos')		
			if [ "$FORM_form" != form2 ]; then
				kdb_vars="str:sys_iface_${iface}_qos_sch "
			else
				case "$qos_sch" in
					bfifo|pfifo)
						kdb_vars="int:sys_iface_${iface}_qos_fifo_limit";;
					sqf)
						kdb_vars="";;
					esqf)
						kdb_vars="int:sys_iface_${iface}_qos_esfq_limit int:sys_iface_${iface}_qos_esfq_depth int:sys_iface_${iface}_qos_esfq_hash";;
					tbf)
						kdb_vars="str:sys_iface_${iface}_qos_tbf_rate int:sys_iface_${iface}_qos_tbf_limit str:sys_iface_${iface}_qos_tbf_latency int:sys_iface_${iface}_qos_tbf_buffer";;
				esac
			fi
			;;
		'spec')
			case "$proto" in
				bridge)
					kdb_vars="bool:sys_iface_${iface}_br_stp str:sys_iface_${iface}_br_ifaces str:sys_iface_${iface}_br_prio str:sys_iface_${iface}_br_fd str:sys_iface_${iface}_br_hello str:sys_iface_${iface}_br_maxage";;
				bonding)
					kdb_vars="str:sys_iface_${iface}_bond_ifaces";;
				pptp)
					kdb_vars="str:sys_iface_${iface}_pptp_server str:sys_iface_${iface}_pptp_username str:sys_iface_${iface}_pptp_password bool:sys_iface_${iface}_pptp_defaultroute str:sys_iface_${iface}_pptp_pppdopt";;
				pppoe)
					kdb_vars="str:sys_iface_${iface}_pppoe_iface str:sys_iface_${iface}_pppoe_ac str:sys_iface_${iface}_pppoe_service bool:sys_iface_${iface}_pptp_defaultroute  str:sys_iface_${iface}_pppoe_username str:sys_iface_${iface}_pppoe_password str:sys_iface_${iface}_pppoe_pppdopt" ;;
				ether)
					kdb_vars="str:sys_iface_${iface}_mac";;
			esac
			
	esac

	render_save_stuff
	
	eval `kdb -qq ls sys_iface_${iface}_* : sls sys_iface_${iface}_ `

	render_form_header

	render_input_field hidden iface iface "$iface"
	render_input_field hidden page page "$page"

	# first form
	case $page in 
	'status')
		ip=`which ip`
		ip=${ip:-/sbin/ip}
		realiface=$iface
		[ -n "$real" -a -d /proc/sys/net/ipv4/conf/$real ] && realiface=$real
		
		if $ip link ls dev $realiface >/dev/null 2>&1; then

			render_console_start "Interface status" 2 
			render_console_command /sbin/ifconfig $realiface
			render_console_command $ip link show dev $realiface
			render_console_command $ip addr show dev $realiface
			render_console_end

			render_console_start "Routes" 2 
			render_console_command $ip route show dev $realiface
			render_console_end

			render_console_start "ARP" 2 
			render_console_command $ip neigh show dev $realiface
			render_console_end
			case "$proto" in 
			ether)
				render_table_title "Internal switch status" 2 
				render_console_start
				render_console_command cat /proc/sys/net/adm5120sw/status
				render_console_end
				;;
			bridge)
				render_table_title "Bridge status" 2 
				render_console_start
				render_console_command /usr/sbin/brctl show $realiface
				render_console_command /usr/sbin/brctl showmacs $realiface
				render_console_end
				;;
			bonding)
				render_console_start "Bonding status" 2 
				render_console_command cat /proc/net/bonding/$realiface
				render_console_end
				;;
			pppoe|pptp)
				render_console_start "PPP stats" 2 
				[ -x /usr/sbin/pppstats ] && render_console_command /usr/sbin/pppstats $realiface
				render_console_end
				;;
			esac
			
			if [ "$dhcp_enabled" = 1 -a -x /usr/bin/dumpleases -a -r /var/lib/misc/udhcpd.${realiface}.leases ]; then
				render_console_start "DHCP Leases" 2 
				render_console_command /usr/bin/dumpleases -f /var/lib/misc/udhcpd.${realiface}.leases
				render_console_end
			fi
			
			if [ -n "$qos_sch" ]; then
				render_console_start "Traffic Control" 2 
				render_console_command /usr/sbin/tc -s qdisc ls dev $realiface
				#render_console_command /usr/sbin/tc class ls dev $realiface
				#render_console_command /usr/sbin/tc filter ls dev $realiface
				render_console_end
			fi
		else
			render_table_title "Interface status" 2 
			render_console_start
			echo "Interface <b>$realiface</b> currently does not exist"
			render_console_end
		fi
		;;
	'general')
		tip=""
		desc=""
		validator='tmt:required="true"'
		render_input_field checkbox "Enabled" sys_iface_${iface}_enabled

		# sys_iface_${iface}_auto
		tip=""
		desc=""
		validator='tmt:required="true"'
		render_input_field checkbox "Auto" sys_iface_${iface}_auto

		# sys_iface_${iface}_method
		tip="Select address method:<br><b>Static:</b> IP address is static<br><b>Dynamic:</b> DHCP/PPPoE/PPTP/etc"
		desc="Please select method of the interface"
		validator='tmt:required="true" tmt:message="Please select method"'
		render_input_field select "Method" sys_iface_${iface}_method none 'None' static 'Static address' zeroconf 'Zero Configuration' dynamic 'Dynamic address'
		;;
	'method')
		if [ "$method" = "static" ]; then

			# sys_iface_${iface}_ipaddr
			#tip="IP address for interface"
			desc="Address (dotted quad) <b>required</b>"
			validator="$tmtreq $validator_ipaddr"
			render_input_field text "Static address " sys_iface_${iface}_ipaddr

			# sys_iface_${iface}_netmask
			#tip="Netmask used for BLA BLA BLA"
			desc="Netmask (dotted quad) <b>required</b>"
			validator="$tmtreq $validator_netmask"
			render_input_field text "Netmask" sys_iface_${iface}_netmask

			# sys_iface_${iface}_broadcast
			#tip="IP address for interface"
			desc="Broadcast (dotted quad)"
			validator=$validator_ipaddr
			render_input_field text "Broadcast" sys_iface_${iface}_broadcast
			
			# sys_iface_${iface}_gateway
			#tip="Gateway used for default routing"
			desc="Default gateway (dotted quad)"
			validator=$validator_ipaddr
			render_input_field text "Gateway" sys_iface_${iface}_gateway
		fi

		if [ "$proto" = "hdlc" ]; then
			# sys_iface_${iface}_pointopoint
			desc="Point-to-Point address (dotted quad)"
			validator="$tmtreq $validator_ipaddr"
			render_input_field text "Point to Point" sys_iface_${iface}_pointopoint
		fi


		#if [ "$method" = "dynamic" ]; then
		#	desc="Hostname to be requested"
		#	validator='tmt:filters="ltrim,rtrim,nohtml,nospaces,nocommas,nomagic"'
		#	render_input_field text "DHCP Client hostname" sys_iface_${iface}_dynhostname 
		#fi
		;;
	'options')
		default=1
		render_input_field checkbox "Accept redirects" sys_iface_${iface}_opt_accept_redirects
		default=1
		render_input_field checkbox "Forwarding" sys_iface_${iface}_opt_forwarding
		default=0
		render_input_field checkbox "Proxy ARP" sys_iface_${iface}_opt_proxy_arp
		default=1
		render_input_field checkbox "RP Filter" sys_iface_${iface}_opt_rp_filter
		;;
	'dhcp')
		# sys_iface_${iface}_dhcp_enabled
		tip=""
		desc="Check this item if you want use DHCP server on your LAN"
		validator='tmt:required="true"'
		render_input_field checkbox "Enable DHCP server" sys_iface_${iface}_dhcp_enabled

		desc="Start of dynamic ip range address for your LAN (dotted quad) <b>required</b>"
		validator="$tmtreq $validator_ipaddr"
		render_input_field text "Start IP" sys_iface_${iface}_dhcp_startip

		desc="End of dynaic ip range address for your LAN (dotted quad) <b>required</b>"
		validator="$tmtreq $validator_ipaddr"
		render_input_field text "End IP" sys_iface_${iface}_dhcp_endip

		tip="<b>Example:</b> 255.255.255.0"
		desc="Netmask for your LAN (dotted quad) <b>required</b>"
		validator="$tmtreq $validator_netmask"
		render_input_field text "Netmask" sys_iface_${iface}_dhcp_netmask

		# sys_iface_${iface}_dhcp_router
		tip="Router for subnet"
		desc="Default router for your LAN hosts (dotted quad) "
		validator=$validator_ipaddr
		render_input_field text "Default router" sys_iface_${iface}_dhcp_router

		# sys_iface_${iface}_dhcp_lease_time
		tip="Select default lease time"
		desc="Please select lease time"
		validator='tmt:message="Please select lease time"'
		render_input_field select "Default lease time" sys_iface_${iface}_dhcp_lease_time 600 "10 minutes" 1800 "30 minutes" 3600 "1 hour" \
				10800 "3 hours" 36000 "10 hours" 86400 "24 hours" #infinite "Infinite"

		# sys_iface_${iface}_dhcp_nameserver
		tip="DNS server for your LAN hosts<br>You can use this device as DNS server"
		desc="DNS server for your LAN hosts (dotted quad)"
		validator=$validator_ipaddr
		render_input_field text "DNS server" sys_iface_${iface}_dhcp_nameserver

		# sys_iface_${iface}_dhcp_domain_name
		tip="Most queries for names within this domain can use short names relative to the local domain"
		desc="Allows DHCP hosts to have fully qualified domain names"
		validator='tmt:filters="ltrim,rtrim,nohtml,nospaces,nocommas,nomagic"'
		render_input_field text "Domain" sys_iface_${iface}_dhcp_domain_name

		# sys_iface_${iface}_dhcp_ntpserver
		tip="NTP server for your LAN hosts"
		desc="NTP server for your LAN hosts (dotted quad)"
		validator=$validator_ipaddr
		render_input_field text "NTP server" sys_iface_${iface}_dhcp_ntpserver

		tip="WINS server for your LAN hosts"
		desc="WINS server for your LAN hosts (dotted quad)"
		validator=$validator_ipaddr
		render_input_field text "WINS server" sys_iface_${iface}_dhcp_winsserver
		;;
	'spec')
		case $proto in
		'ether')
			render_table_title "Ethernet Specific parameters" 2 

			desc="MAC Address for interface"
			validator=$validator_macaddr
			render_input_field text "MAC Address" sys_iface_${iface}_mac
			;;
		'pppoe')
			render_table_title "PPPoE Specific parameters" 2 
			
			desc="Parent interface name <b>required</b>"
			validator='tmt:required="true" tmt:message="Please input parent interface name" tmt:filters="ltrim,rtrim,nohtml,nospaces,nocommas,nomagic"'
			render_input_field text "Interface" sys_iface_${iface}_pppoe_iface
			
			tip="Router will only initiate sessions with access concentrators which can provide the specified service.<br>  In most cases, you should <b>not</b> specify this option."
			desc="Desired service name"
			validator='tmt:message="Please input desired service name" tmt:filters="nohtml,nomagic"'
			render_input_field text "Service" sys_iface_${iface}_pppoe_service
			validator='tmt:filters="nomagic"'
			
			tip="Router will only initiate sessions with the specified access concentrator. In most cases, you should <b>not</b> specify this option. Use it only if you know that there are multiple access concentrators."
			desc="Desired access concentrator name"
			render_input_field text "Access Concentrator" sys_iface_${iface}_pppoe_ac
			
			default=0
			desc="Add a default route to the system routing tables, using the peer as the gateway"
			render_input_field checkbox "Default route" sys_iface_${iface}_pppoe_defaultroute
			
			validator='tmt:required="true" tmt:filters="nomagic"'
			render_input_field text "Username" sys_iface_${iface}_pppoe_username
			
			validator='tmt:required="true" tmt:filters="nomagic"'
			render_input_field text "Password" sys_iface_${iface}_pppoe_password
			# TODO: about name, remotename options
			default="noauth nobsdcomp nodeflate"
			render_input_field text "PPPD Options" sys_iface_${iface}_pppoe_pppdopt
			;;
		'pptp')
			render_table_title "PPtP Specific parameters" 2 
			desc="PPtP Server <b>required</b>"

			
			validator=$validator_dnsdomainoripaddr
			render_input_field text "Server" sys_iface_${iface}_pptp_server
			
			validator='tmt:required="true" tmt:filters="ltrim,rtrim,nohtml,nospaces,nocommas,nomagic"'
			render_input_field text "Username" sys_iface_${iface}_pptp_username
			
			validator='tmt:required="true" tmt:filters="nomagic"'
			render_input_field text "Password" sys_iface_${iface}_pptp_password
			
			default=0
			desc="Add a default route to the system routing tables, using the peer as the gateway"
			render_input_field checkbox "Default route" sys_iface_${iface}_pptp_defaultroute
			
			# TODO: replace route?
			# TODO: add static route to pptp server?
			#render_input_field select "Encryption" sys_iface_${iface}_pptp_enc nomppe nomppe mppe-40 mppe-40 mppe-56 mppe-56 mppe-128 mppe-128
			
			default="noauth nobsdcomp nodeflate nomppe"	#		require-mppe-128"
			render_input_field text "PPPD Options" sys_iface_${iface}_pptp_pppdopt
			;;
		'bonding')
			render_table_title "Bonding Specific parameters" 2 

			desc="MAC Address for interface"
			validator=$validator_macaddr
			render_input_field text "MAC Address" sys_iface_${iface}_mac

			tip="<b>Example:</b>eth0 eth1 dsl0<br><b>Note:</b>You can use only Ethernet-like interfaces, like ethX, dslX, bondX<br><b>Note:</b> Interfaces should be enabled, but <b>auto</b> should be switched <b>off</b>"
			desc="Interfaces for bonding separated by space"
			validator=$validator_ifacelist
			render_input_field text "Interfaces" sys_iface_${iface}_bond_ifaces
			
			# cleans interface _auto param
			if [ $REQUEST_METHOD = POST ]; then
				eval 'ifaces=$sys_iface_'${iface}'_bond_ifaces'
				for i in $ifaces; do
					kdb set sys_iface_${i}_auto=0;
				done
			fi
			;;
		'bridge')
			render_table_title "Bridge Specific parameters" 2 
			tip="Multiple ethernet bridges can work together to create even larger networks of ethernets using the IEEE 802.1d spanning tree protocol.This protocol  is used for finding the shortest path between two ethernets, and for eliminating loops from the topology."
			desc="Enable Spanning Tree Protocol"
			render_input_field checkbox "STP Enabled" sys_iface_${iface}_br_stp
			
			tip="<b>Example:</b> eth0 eth1 dsl0<br><b>Note:</b> You can use only Ethernet-like interfaces, like ethX, dslX<br><b>Note:</b> Interfaces should be enabled, but <b>auto</b> should be switched <b>off</b>"
			desc="Interfaces for bridge separated by space"
			validator=$validator_ifacelist
			render_input_field text "Interfaces" sys_iface_${iface}_br_ifaces
			
			tip="The priority value is  an  unsigned  16-bit  quantity  (a  number between  0 and 65535), and has no dimension. Lower priority values are better. The bridge with the lowest priority will be elected <b>root bridge</b>."
			desc="Bridge priority"
			validator='tmt:filters="ltrim,rtrim,nohtml,nocommas,nomagic" tmt:pattern="positiveinteger" tmt:minnumber=1 tmt:maxnumber=65535 tmt:message="Please input number" '
			render_input_field text "Priority" sys_iface_${iface}_br_prio
			tip="Sets the bridges <b>bridge forward delay</b> to <time> seconds."
			validator='tmt:filters="ltrim,rtrim,nohtml,nocommas,nomagic" tmt:pattern="positiveinteger" tmt:minnumber=0 tmt:maxnumber=60  tmt:message="Please input number"'
			render_input_field text "Forward delay" sys_iface_${iface}_br_fd
			validator='tmt:filters="ltrim,rtrim,nohtml,nocommas,nomagic" tmt:pattern="positiveinteger" tmt:minnumber=0 tmt:maxnumber=60  tmt:message="Please input number"'
			render_input_field text "Hello time" sys_iface_${iface}_br_hello
			validator='tmt:filters="ltrim,rtrim,nohtml,nocommas,nomagic" tmt:pattern="positiveinteger" tmt:minnumber=0 tmt:maxnumber=600  tmt:message="Please input number"'
			render_input_field text "Max age" sys_iface_${iface}_br_maxage

			# cleans interface _auto param
			if [ $REQUEST_METHOD = POST ]; then
				eval 'ifaces=$sys_iface_'${iface}'_br_ifaces'
				for i in $ifaces; do
					kdb set sys_iface_${i}_auto=0;
				done
			fi
			;;
		esac
		;;
	'qos')
		# sys_iface_${iface}_qos_sch
		desc="Please select scheduler for the interface"
		validator='tmt:required="true" tmt:message="Please select method"'
		render_input_field select "Scheduler" sys_iface_${iface}_qos_sch pfifo_fast "Default discipline pfifo_fast" bfifo 'FIFO with bytes buffer' pfifo 'FIFO with packets buffer ' 'tbf' 'Token Bucket Filter' 'sfq' 'Stochastic Fairness Queueing' 'esfq' 'Enhanced <b>Stochastic Fairness Queueing'  #'htb' 'Hierarchical Token Bucket' 
		;;
		
	'routes')
		;;
	esac
	
	[ "$page" = 'status' -o "$page" = 'routes' ] || render_submit_field
	render_form_tail

	# second form
	case $page in
	'dhcp')
		# static dhcp list
		#render_form_header dhcp_leases
		#render_table_title "DHCP Static leases" 2 
		#render_iframe_list "dhcp_static" "iface=$iface"
		#render_form_tail

		render_form_header dhcp

		render_list_line(){
			local lineno=$1
			eval "val=\"\${sys_iface_${iface}_dhcp_host_${lineno}}\""
			unset name ipaddr hwaddr
			eval "$val"
			echo "<tr><td>$lineno</td><td>$name</td><td>$ipaddr</td><td>$hwaddr</td><td>"
			render_list_btns dhcp_static_edit "sys_iface_${iface}_dhcp_host_${lineno}" "page=${page}&iface=${iface}"
			echo "</td></tr>"
		}
		
		
		render_list_header dhcp_static sys_iface_${iface}_dhcp_host_ "iface=${iface}" "No" "Name" "IP Address" "MAC Address"
		
		eval `kdb -qqc ls sys_iface_${iface}_dhcp_host*`
		render_list_cycle_stuff

		render_form_tail


		;;
	'qos')
		# static dhcp list
		if [ "$qos_sch" != sfq -a "$qos_sch" != pfifo_fast ]; then
			render_form_header qos
			render_input_field hidden iface iface "$iface"
			render_input_field hidden page page "$page"
			render_input_field hidden form form "form2"
			render_table_title "QoS Settings" 2 
		fi
		case $qos_sch in
			bfifo|pfifo)
				[ $qos_sch = pfifo ] && default=128 || default=10240
				desc="Buffer size"
				validator='tmt:filters="ltrim,rtrim,nohtml,nocommas,nomagic" tmt:pattern="positiveinteger" tmt:minnumber=1 tmt:maxnumber=65535 tmt:message="Please input number" '
				render_input_field text "Buffer" sys_iface_${iface}_qos_fifo_limit
			;;
			sfq)
				# nothing to setup
			;;
			esfq)
				default=128
				desc="Maximum packets in buffer"
				validator='tmt:filters="ltrim,rtrim,nohtml,nocommas,nomagic" tmt:pattern="positiveinteger" tmt:minnumber=10 tmt:maxnumber=65535 tmt:message="Please input number" '
				render_input_field text "Limit" sys_iface_${iface}_qos_esfq_limit

				default=128
				desc="Depth"
				validator='tmt:filters="ltrim,rtrim,nohtml,nocommas,nomagic" tmt:pattern="positiveinteger" tmt:minnumber=10 tmt:maxnumber=65535 tmt:message="Please input number" '
				render_input_field text "Depth" sys_iface_${iface}_qos_esfq_depth
				render_input_field select "Hash" sys_iface_${iface}_qos_esfq_hash classic 'Classic' src 'Source address' dst 'Destination address'
				;;
			tbf)
				tip="Unit can be: <br/><b>kbit</b>, <b>Mbit</b> - for bit per second<br/> and <b>kbps</b>, <b>Mbps</b> - for bytes per second"
				default='512kbit'
				desc="Maximum rate for interface"
				validator='tmt:filters="ltrim,rtrim,nohtml,nocommas,nomagic" tmt:pattern="qosbandw"'
				render_input_field text "Rate" sys_iface_${iface}_qos_tbf_rate

				default="4096"
				desc="Maximum size of tokens buffer in bytes"
				validator='tmt:required="true" tmt:filters="ltrim,rtrim,nohtml,nocommas,nomagic" tmt:pattern="positiveinteger" tmt:minnumber=10 tmt:maxnumber=65535 tmt:message="Please input number" '
				render_input_field text "Tocken buffer" sys_iface_${iface}_qos_tbf_buffer

				default=""
				desc="Maximum size of buffer in bytes <br/><b>Note:</b> You should use <i>limit</i> <b>OR</b> <i>latency</i>"
				validator='tmt:filters="ltrim,rtrim,nohtml,nocommas,nomagic" tmt:pattern="positiveinteger" tmt:minnumber=10 tmt:maxnumber=65535 tmt:message="Please input number" '
				render_input_field text "Limit" sys_iface_${iface}_qos_tbf_limit

				default='20ms'
				desc="Buffer size for packet latency<br/><b>Note:</b> You should use <i>limit</i> <b>OR</b> <i>latency</i>"
				validator='tmt:filters="ltrim,rtrim,nohtml,nocommas,nomagic" tmt:pattern="qoslatency"'
				render_input_field text "Latency" sys_iface_${iface}_qos_tbf_latency
			;;
		esac
		if [ "$qos_sch" != sfq -a "$qos_sch" != pfifo_fast ]; then
			render_submit_field
			render_form_tail
		fi
		;;

	'routes')
		if [ $method != dynamic ]; then
			render_form_header routes
			render_list_line(){
				local lineno=$1
				eval "val=\"\${sys_iface_${iface}_route_${lineno}}\""
				unset net netmask gw
				eval "$val"
				echo "<tr><td>$lineno</td><td>$net</td><td>$netmask</td><td>$gw</td><td>"
				render_list_btns staticroute_edit "sys_iface_${iface}_route_${lineno}" "page=${page}&iface=${iface}"
				echo "</td></tr>"
			}
			
			render_list_header staticroute sys_iface_${iface}_route_ "iface=${iface}" "No" "Network" "Mask" "Gateway"
			eval `$kdb -qqc list sys_iface_${iface}_route*`

			render_list_cycle_stuff

			render_form_note "You should restart interface to apply settings"
			render_form_tail
		fi
		;;

	esac
