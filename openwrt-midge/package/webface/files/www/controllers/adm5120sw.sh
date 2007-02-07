#!/usr/bin/haserl
	
	for port in $ADM5120SW_PORTS; do
		kdb_vars="$kdb_vars str:sys_switch_port${port}_iface str:sys_switch_port${port}_speed str:sys_switch_port${port}_duplex"
	done

	render_save_stuff
	eval `kdb -qq ls sys_switch_* `
	
	render_form_header 
	#render_table_title "Internal switch port configuration" 2 
	
	for port in $ADM5120SW_PORTS; do 
		render_table_title "Port $port" 2
		render_input_field select "Attach port $port to" sys_switch_port${port}_iface `for i in $ADM5120SW_PORTS; do echo "eth${i} eth${i}"; done`
		render_input_field -d select "Speed" sys_switch_port${port}_speed auto 'Auto' 10 '10M' 100 '100M'
		render_input_field -d select "Duplex" sys_switch_port${port}_duplex auto 'Auto' full 'Full' half 'Half'
		
	done
	render_submit_field
	render_form_tail

