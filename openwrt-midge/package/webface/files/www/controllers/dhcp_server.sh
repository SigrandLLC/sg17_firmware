#!/usr/bin/haserl
	
	subsys="dhcp"
	page=${FORM_page}
	iface=$page

	kdb_vars="bool:sys_iface_${iface}_dhcp_enabled int:sys_iface_${iface}_dhcp_lease_time"
	kdb_vars="$kdb_vars str:sys_iface_${iface}_dhcp_router str:sys_iface_${iface}_dhcp_nameserver"
	kdb_vars="$kdb_vars str:sys_iface_${iface}_dhcp_domain_name "
	kdb_vars="$kdb_vars str:sys_iface_${iface}_dhcp_ntpserver str:sys_iface_${iface}_dhcp_winsserver "
	kdb_vars="$kdb_vars str:sys_iface_${iface}_dhcp_startip str:sys_iface_${iface}_dhcp_endip str:sys_iface_${iface}_dhcp_netmask "

	if [ "x${FORM_iface_select}x" = "xx" ]; then
		render_save_stuff
	fi

	eval `kdb -qq ls sys_iface*`

	ifaces=`kdb sskls 'sys*valid=1' sys_iface_ _valid`
	fifaces='';
	for i in $ifaces; do
		eval "proto=\$sys_iface_${i}_proto"
		case "$proto" in 
			ether|bridge|vlan|bonding) fifaces="$fifaces $i $i";;
		esac
	done
	
	render_page_selection "" $fifaces

	## select example
	## interface select
	#form_method='GET'
	#render_form_header dhcp_server
	#render_table_title "DHCP Server"
	#fifaces="'' --select--" 
	#autosubmit="y"
	#render_input_field select "Interface" iface $fifaces
	#render_form_tail

	# ----------------

	if [ "x${iface}x" != "xx" ]; then
		. common/dhcp_server.sh
		render_form_header dhcp_server_common
		render_dhcp_server_common
		render_form_tail
		render_dhcp_server_static
	fi
	
# vim:foldmethod=indent:foldlevel=1
