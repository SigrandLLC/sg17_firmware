Controllers['iface'] = function(iface) {
	var field;
	var c;
	var tabs = this.pageTabs({status: "Status", general: "General", method: "Method",
		options: "Options", specific: "Specific", qos: "QoS", routes: "Routes"},
		{subsystem: "network", help: "iface"});
	
	/* status tab */
	c = tabs.tabs['status'].addContainer("status");
	c.addTitle("Interface status");
	c.addConsole(["/sbin/ifconfig " + iface, "/usr/sbin/ip addr show dev " + iface,
		"/usr/sbin/ip link show dev " + iface]);
	
	c = tabs.tabs['status'].addBr();
	c = tabs.tabs['status'].addContainer("status");
	c.addTitle("Routes");
	c.addConsole("/usr/sbin/ip route show dev " + iface);
	
	c = tabs.tabs['status'].addBr();
	c = tabs.tabs['status'].addContainer("status");
	c.addTitle("ARP");
	c.addConsole("/usr/sbin/ip neigh show dev " + iface);
	
	/* general tab */
	c = tabs.tabs['general'].addContainer("general");
	c.addTitle("Interface general settings");

	field = { 
		type: "text",
		name: "sys_iface_" + iface + "_desc",
		text: "Description"
	}
	c.addWidget(field);

	field = { 
		type: "checkbox",
		name: "sys_iface_" + iface + "_enabled",
		text: "Enabled"
	}
	c.addWidget(field);
	
	field = { 
		type: "checkbox",
		name: "sys_iface_" + iface + "_auto",
		text: "Auto"
	}
	c.addWidget(field);
	
	field = { 
		type: "select",
		name: "sys_iface_" + iface + "_method",
		text: "Method",
		descr: "Select method of setting IP address",
		options: {"none": "None", "static": "Static address", "zeroconf": "Zero Configuration", "dynamic": "Dynamic address"}
	}
	c.addWidget(field);
	
	field = { 
		type: "select",
		name: "sys_iface_" + iface + "_depend_on",
		text: "Depended on",
		options: config.get("sys_ifaces")
	}
	c.addWidget(field);

	c.addSubmit();
	
	/* method tab */
	var c = tabs.tabs['method'].addContainer("method");
	if (config.get("sys_iface_" + iface + "_proto") == "hdlc")
	{
		c.addTitle("Point-to-Point address settings");
	
		field = { 
			type: "text",
			name: "sys_iface_" + iface + "_pointopoint_local",
			text: "Point to Point local",
			descr: "Point-to-Point local address (dotted quad)",
			validator: {required: true},
			message: "Please enter correct IP address"
		}
		c.addWidget(field);
		
		field = { 
			type: "text",
			name: "sys_iface_" + iface + "_pointopoint_remote",
			text: "Point to Point remote",
			descr: "Point-to-Point remote address (dotted quad)",
			validator: {required: true},
			message: "Please enter correct IP address"
		}
		c.addWidget(field);
		
		c.addSubmit();
		
	}
	else if (config.get("sys_iface_" + iface + "_method") == "static")
	{
		c.addTitle("Static address settings");
	
		field = { 
			type: "text",
			name: "sys_iface_" + iface + "_ipaddr",
			text: "Static address",
			descr: "Address (dotted quad)",
			validator: {required: true},
			message: "Please enter correct IP address"
		}
		c.addWidget(field);
		
		field = { 
			type: "text",
			name: "sys_iface_" + iface + "_netmask",
			text: "Netmask",
			descr: "Netmask (dotted quad)",
			validator: {required: true},
			message: "Please enter correct IP netmask"
		}
		c.addWidget(field);
		
		field = { 
			type: "text",
			name: "sys_iface_" + iface + "_broadcast",
			text: "Broadcast",
			descr: "Broadcast (dotted quad)"
		}
		c.addWidget(field);
		
		field = { 
			type: "text",
			name: "sys_iface_" + iface + "_gateway",
			text: "Gateway",
			descr: "Default gateway (dotted quad)"
		}
		c.addWidget(field);
		
		c.addSubmit();
	}
	
	/* options tab */
	c = tabs.tabs['options'].addContainer("options");
	c.addTitle("Interface options");

	field = { 
		type: "checkbox",
		name: "sys_iface_" + iface + "_opt_accept_redirects",
		text: "Accept redirects"
	}
	c.addWidget(field);
	
	field = { 
		type: "checkbox",
		name: "sys_iface_" + iface + "_opt_forwarding",
		text: "Forwarding"
	}
	c.addWidget(field);
	
	field = { 
		type: "checkbox",
		name: "sys_iface_" + iface + "_opt_proxy_arp",
		text: "Proxy ARP"
	}
	c.addWidget(field);
	
	field = { 
		type: "checkbox",
		name: "sys_iface_" + iface + "_opt_rp_filter",
		text: "RP Filter"
	}
	c.addWidget(field);
	
	c.addSubmit();
	
	/* specific tab */
	c = tabs.tabs['specific'].addContainer("spec");
	
	switch (config.get("sys_iface_" + iface + "_proto"))
	{
		case "ether":
			c.addTitle("Ethernet Specific parameters");
			
			field = { 
				type: "text",
				name: "sys_iface_" + iface + "_mac",
				text: "MAC address",
				descr: "MAC address for the interface"
			}
			c.addWidget(field);
			
			c.addSubmit();
			
			break;
			
		case "pppoe":
			c.addTitle("PPPoE Specific parameters");
			
			field = { 
				type: "text",
				name: "sys_iface_" + iface + "_pppoe_iface",
				text: "Interface",
				descr: "Parent interface name"
			}
			c.addWidget(field);
			
			field = { 
				type: "text",
				name: "sys_iface_" + iface + "_pppoe_service",
				text: "Service",
				descr: "Desired service name",
				tip: "Router will only initiate sessions with access concentrators which" + 
					" can provide the specified service.<br>  In most cases, you should <b>not</b>" + 
					" specify this option."
			}
			c.addWidget(field);
			
			field = { 
				type: "text",
				name: "sys_iface_" + iface + "_pppoe_ac",
				text: "Access Concentrator",
				descr: "Desired access concentrator name",
				tip: "Router will only initiate sessions with the specified access concentrator." + 
					" In most cases, you should <b>not</b> specify this option. Use it only if you" + 
					" know that there are multiple access concentrators."
			}
			c.addWidget(field);
			
			field = { 
				type: "checkbox",
				name: "sys_iface_" + iface + "_pppoe_defaultroute",
				text: "Default route",
				descr: "Add a default route to the system routing tables, using the peer as the gateway"
			}
			c.addWidget(field);
			
			field = { 
				type: "text",
				name: "sys_iface_" + iface + "_pppoe_username",
				text: "Username"
			}
			c.addWidget(field);
			
			field = { 
				type: "text",
				name: "sys_iface_" + iface + "_pppoe_password",
				text: "Password"
			}
			c.addWidget(field);
			
			field = { 
				type: "text",
				name: "sys_iface_" + iface + "_pppoe_pppdopt",
				text: "PPPD options"
			}
			c.addWidget(field);
			
			c.addSubmit();
			
			break;
			
		case "pptp":
			c.addTitle("PPtP Specific parameters");
			
			field = { 
				type: "text",
				name: "sys_iface_" + iface + "_pptp_server",
				text: "Server",
				descr: "PPtP server"
			}
			c.addWidget(field);
			
			field = { 
				type: "text",
				name: "sys_iface_" + iface + "_pptp_username",
				text: "Username"
			}
			c.addWidget(field);
			
			field = { 
				type: "text",
				name: "sys_iface_" + iface + "_pptp_password",
				text: "Password"
			}
			c.addWidget(field);
			
			field = { 
				type: "checkbox",
				name: "sys_iface_" + iface + "_pptp_defaultroute",
				text: "Default route",
				descr: "Add a default route to the system routing tables, using the peer as the gateway"
			}
			c.addWidget(field);
			
			field = { 
				type: "text",
				name: "sys_iface_" + iface + "_pptp_pppdopt",
				text: "PPPD options"
			}
			c.addWidget(field);
			
			c.addSubmit();
			
			break;
		
		case "bonding":
			/* TODO: we need to unset 'auto' option for interfaces this bonding consist of */
			
			c.addTitle("Bonding Specific parameters");
			
			field = { 
				type: "text",
				name: "sys_iface_" + iface + "_bond_ifaces",
				text: "Interfaces",
				descr: "Interfaces for bonding separated by space",
				tip: "<b>Example:</b>eth0 eth1 dsl0<br><b>Note:</b>You can use only Ethernet-like" + 
					" interfaces, like ethX, dslX, bondX<br><b>Note:</b> Interfaces should be" + 
					" enabled, but <b>auto</b> should be switched <b>off</b>"
			}
			c.addWidget(field);
			
			c.addSubmit();
			
			break;
			
		case "bridge":
			/* TODO: we need to unset 'auto' option for interfaces this bonding consist of */
	
			c.addTitle("Bridge Specific parameters");
			
			field = { 
				type: "checkbox",
				name: "sys_iface_" + iface + "_br_stp",
				text: "STP enabled",
				descr: "Enable Spanning Tree Protocol",
				tip: "Multiple ethernet bridges can work together to create even larger networks" + 
					" of ethernets using the IEEE 802.1d spanning tree protocol.This protocol is" + 
					" used for finding the shortest path between two ethernets, and for eliminating" + 
					" loops from the topology."
			}
			c.addWidget(field);
			
			field = { 
				type: "text",
				name: "sys_iface_" + iface + "_br_ifaces",
				text: "Interfaces",
				descr: "Interfaces for bridge separated by space",
				tip: "<b>Example:</b> eth0 eth1 dsl0<br><b>Note:</b> You can use only" + 
				" Ethernet-like interfaces, like ethX, dslX<br><b>Note:</b> Interfaces should" + 
				" be enabled, but <b>auto</b> should be switched <b>off</b>."
			}
			c.addWidget(field);
			
			field = { 
				type: "text",
				name: "sys_iface_" + iface + "_br_prio",
				text: "Priority",
				descr: "Bridge priority",
				tip: "The priority value is an unsigned 16-bit quantity (a number between 0" + 
					" and 65535), and has no dimension. Lower priority values are better. The bridge" +
					" with the lowest priority will be elected <b>root bridge</b>."
			}
			c.addWidget(field);
			
			field = { 
				type: "text",
				name: "sys_iface_" + iface + "_br_fd",
				text: "Forward delay",
				descr: "Sets the bridges <b>bridge forward delay</b> to <time> seconds."
			}
			c.addWidget(field);
			
			field = { 
				type: "text",
				name: "sys_iface_" + iface + "_br_hello",
				text: "Hello time"
			}
			c.addWidget(field);
			
			field = { 
				type: "text",
				name: "sys_iface_" + iface + "_br_maxage",
				text: "Max age"
			}
			c.addWidget(field);
			
			c.addSubmit();
			
			break;
	}
}
