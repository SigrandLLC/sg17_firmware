#!/bin/sh
# (c) Vladislav Moskovets 2005
#

service_reload(){
	local service="$1"
	local auto;
	
	case "$service" in
	network)
		if [ -n "$iface" ]; then
			/sbin/ifdown $iface;
			#eval 'auto=$'sys_iface_${iface}_auto
			auto=`/usr/bin/kdb get sys_iface_${iface}_auto`
			[ "$auto" = 1 ] && /sbin/ifup $iface
		else
			/etc/init.d/network restart
		fi
	;;
	dhcp*)
		[ -z "$iface" ] && iface=${service#*.}
		/etc/init.d/udhcpd restart $iface
	;;
	dns_server)
		/etc/init.d/bind restart
	;;
	# we can enable/disable multiplexing on module settings page, so restart
	# multiplexing on module settings change
	dsl*)
		tmp=${service#*.}
		slot=${tmp%.*}
		dev=${tmp#*.}
		/etc/init.d/dsl restart $slot $dev
		/etc/init.d/mux start
	;;
	e1*)
		tmp=${service#*.}
		slot=${tmp%.*}
		dev=${tmp#*.}
		/etc/init.d/e1 restart "$slot" "$dev"
		/etc/init.d/mux start
		/etc/init.d/network restart
	;;
	rs232*)
		tmp=${service#*.}
		slot=${tmp%.*}
		dev=${tmp#*.}
		/etc/init.d/rs232 restart "$slot" "$dev"
		/etc/init.d/mux start
	;;
	fw)
		/etc/init.d/fw restart
	;;
	logging)
		/etc/init.d/sysklog restart
	;;
	ipsec)
		/etc/templates/ipsec-tools.sh | /usr/sbin/setkey -c
	;;
	mux)
		/etc/init.d/mux start
	;;
	voip)
		/etc/init.d/rcvoip restart
	;;
	iface_del*)
		iface=${service#*.}
		/sbin/ifdown $iface 2>&1 | ${LOGGER}
	;;
	security*)
		tmp=${service#*.}
		form=${tmp%.*}
		passwd=${tmp#*.}

		case $form in
			htpasswd)
				echo $passwd | htpasswd /etc/htpasswd admin 2>&1 | $LOGGER
			;;
			passwd)
				(echo $passwd; sleep 1; echo $passwd) | passwd root 2>&1 | $LOGGER
			;;
		esac
	;;
	esac
}

