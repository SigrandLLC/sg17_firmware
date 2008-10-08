Controllers['iface'] = function(iface) {
	var page = this.Page();
	page.setSubsystem("network");
	page.setHelpPage("iface");
	
	/* status tab */
	page.addTab({
		"id": "status",
		"name": "Status",
		"func": function() {
			var c;
			c = page.addContainer("status");
			c.addTitle("Interface status");
			c.addConsole(["/sbin/ifconfig " + iface, "/usr/sbin/ip addr show dev " + iface,
				"/usr/sbin/ip link show dev " + iface]);
			
			page.addBr("status");
			c = page.addContainer("status");
			c.addTitle("Routes");
			c.addConsole("/usr/sbin/ip route show dev " + iface);
			
			page.addBr("status");
			c = page.addContainer("status");
			c.addTitle("ARP");
			c.addConsole("/usr/sbin/ip neigh show dev " + iface);
		}
	});
	
	/* general tab */
	page.addTab({
		"id": "general",
		"name": "General",
		"func": function() {
			var c, field;
			c = page.addContainer("general");
			c.addTitle("Interface general settings");
		
			field = { 
				"type": "text",
				"name": "sys_iface_" + iface + "_desc",
				"text": "Description"
			};
			c.addWidget(field);
		
			field = { 
				"type": "checkbox",
				"name": "sys_iface_" + iface + "_enabled",
				"text": "Enabled"
			};
			c.addWidget(field);
			
			field = { 
				"type": "checkbox",
				"name": "sys_iface_" + iface + "_auto",
				"text": "Auto"
			};
			c.addWidget(field);
			
			field = { 
				"type": "select",
				"name": "sys_iface_" + iface + "_method",
				"text": "Method",
				"descr": "Select method of setting IP address",
				"options": {"none": "None", "static": "Static address", "zeroconf": "Zero Configuration", "dynamic": "Dynamic address"}
			};
			c.addWidget(field);
			
			var dependList = new Object();
			$.each(config.getParsed("sys_ifaces"), function(name, value) {
				dependList[value] = value;
			});
			dependList['none'] = "None";
			field = { 
				"type": "select",
				"name": "sys_iface_" + iface + "_depend_on",
				"text": "Depended on",
				"options": dependList,
				"defaultValue": "none"
			};
			c.addWidget(field);
		
			c.addSubmit();
		}
	});
	
	/* method tab */
	page.addTab({
		"id": "method",
		"name": "Method",
		"func": function() {
			var c, field;
			c = page.addContainer("method");
			if (config.get("sys_iface_" + iface + "_proto") == "hdlc")
			{
				c.addTitle("Point-to-Point address settings");
			
				field = { 
					"type": "text",
					"name": "sys_iface_" + iface + "_pointopoint_local",
					"text": "Point to Point local",
					"descr": "Point-to-Point local address",
					"tip": "e.g., 10.0.0.1",
					"validator": {"required": true, "ipAddr": true}
				};
				c.addWidget(field);
				
				field = { 
					"type": "text",
					"name": "sys_iface_" + iface + "_pointopoint_remote",
					"text": "Point to Point remote",
					"descr": "Point-to-Point remote address",
					"tip": "e.g., 10.0.0.2",
					"validator": {"required": true, "ipAddr": true}
				};
				c.addWidget(field);
				
				c.addSubmit();
				
			}
			else if (config.get("sys_iface_" + iface + "_method") == "static")
			{
				c.addTitle("Static address settings");
			
				field = { 
					"type": "text",
					"name": "sys_iface_" + iface + "_ipaddr",
					"text": "Static address",
					"descr": "Address",
					"tip": "e.g., 192.168.2.100",
					"validator": {"required": true, "ipAddr": true}
				};
				c.addWidget(field);
				
				field = { 
					"type": "text",
					"name": "sys_iface_" + iface + "_netmask",
					"text": "Netmask",
					"descr": "Network mask",
					"tip": "e.g., 255.255.255.0",
					"validator": {"required": true, "netmask": true}
				};
				c.addWidget(field);
				
				field = { 
					"type": "text",
					"name": "sys_iface_" + iface + "_broadcast",
					"text": "Broadcast",
					"descr": "Broadcast address",
					"tip": "e.g., 192.168.2.255",
					"validator": {"ipAddr": true}
				};
				c.addWidget(field);
				
				field = { 
					"type": "text",
					"name": "sys_iface_" + iface + "_gateway",
					"text": "Gateway",
					"descr": "Default gateway",
					"tip": "e.g., 192.168.2.1",
					"validator": {"ipAddr": true}
				};
				c.addWidget(field);
				
				c.addSubmit();
			}
		}
	});
	
	/* options tab */
	page.addTab({
		"id": "options",
		"name": "Options",
		"func": function() {
			var c, field;
			c = page.addContainer("options");
			c.addTitle("Interface options");
		
			field = { 
				"type": "checkbox",
				"name": "sys_iface_" + iface + "_opt_accept_redirects",
				"text": "Accept redirects"
			};
			c.addWidget(field);
			
			field = { 
				"type": "checkbox",
				"name": "sys_iface_" + iface + "_opt_forwarding",
				"text": "Forwarding"
			};
			c.addWidget(field);
			
			field = { 
				"type": "checkbox",
				"name": "sys_iface_" + iface + "_opt_proxy_arp",
				"text": "Proxy ARP"
			};
			c.addWidget(field);
			
			field = { 
				"type": "checkbox",
				"name": "sys_iface_" + iface + "_opt_rp_filter",
				"text": "RP Filter"
			};
			c.addWidget(field);
			
			c.addSubmit();
		}
	});
	
	/* specific tab */
	page.addTab({
		"id": "specific",
		"name": "Specific",
		"func": function() {
			var c, field;
			c = page.addContainer("specific");
			
			switch (config.get("sys_iface_" + iface + "_proto"))
			{
				case "ether":
					c.addTitle("Ethernet Specific parameters");
					
					field = { 
						"type": "text",
						"name": "sys_iface_" + iface + "_mac",
						"text": "MAC address",
						"descr": "MAC address for the interface",
						"tip": "e.g., 00:ff:1f:00:75:99",
						"validator": {"macAddr": true}
					};
					c.addWidget(field);
					
					c.addSubmit();
					
					break;
					
				case "pppoe":
					c.addTitle("PPPoE Specific parameters");
					
					field = { 
						"type": "text",
						"name": "sys_iface_" + iface + "_pppoe_iface",
						"text": "Interface",
						"descr": "Parent interface name",
						"validator": {"required": true}
					};
					c.addWidget(field);
					
					field = { 
						"type": "text",
						"name": "sys_iface_" + iface + "_pppoe_service",
						"text": "Service",
						"descr": "Desired service name",
						"tip": "Router will only initiate sessions with access concentrators which" + 
							" can provide the specified service.<br>  In most cases, you should <b>not</b>" + 
							" specify this option."
					};
					c.addWidget(field);
					
					field = { 
						"type": "text",
						"name": "sys_iface_" + iface + "_pppoe_ac",
						"text": "Access Concentrator",
						"descr": "Desired access concentrator name",
						"tip": "Router will only initiate sessions with the specified access concentrator." + 
							" In most cases, you should <b>not</b> specify this option. Use it only if you" + 
							" know that there are multiple access concentrators."
					};
					c.addWidget(field);
					
					field = { 
						"type": "checkbox",
						"name": "sys_iface_" + iface + "_pppoe_defaultroute",
						"text": "Default route",
						"descr": "Add a default route to the system routing tables, using the peer as the gateway"
					};
					c.addWidget(field);
					
					field = { 
						"type": "text",
						"name": "sys_iface_" + iface + "_pppoe_username",
						"text": "Username",
						"validator": {"required": true}
					};
					c.addWidget(field);
					
					field = { 
						"type": "text",
						"name": "sys_iface_" + iface + "_pppoe_password",
						"text": "Password",
						"validator": {"required": true}
					};
					c.addWidget(field);
					
					field = { 
						"type": "text",
						"name": "sys_iface_" + iface + "_pppoe_pppdopt",
						"text": "PPPD options",
						"defaultValue": "noauth nobsdcomp nodeflate"
					};
					c.addWidget(field);
					
					c.addSubmit();
					
					break;
					
				case "pptp":
					c.addTitle("PPtP Specific parameters");
					
					field = { 
						"type": "text",
						"name": "sys_iface_" + iface + "_pptp_server",
						"text": "Server",
						"descr": "PPtP server",
						"validator": {"required": true, "domainNameOrIpAddr": true}
					};
					c.addWidget(field);
					
					field = { 
						"type": "text",
						"name": "sys_iface_" + iface + "_pptp_username",
						"text": "Username",
						"validator": {"required": true}
					};
					c.addWidget(field);
					
					field = { 
						"type": "text",
						"name": "sys_iface_" + iface + "_pptp_password",
						"text": "Password",
						"validator": {"required": true}
					};
					c.addWidget(field);
					
					field = { 
						"type": "checkbox",
						"name": "sys_iface_" + iface + "_pptp_defaultroute",
						"text": "Default route",
						"descr": "Add a default route to the system routing tables, using the peer as the gateway"
					};
					c.addWidget(field);
					
					field = { 
						"type": "text",
						"name": "sys_iface_" + iface + "_pptp_pppdopt",
						"text": "PPPD options",
						"defaultValue": "noauth nobsdcomp nodeflate nomppe"
					};
					c.addWidget(field);
					
					c.addSubmit();
					
					break;
				
				case "bonding":
					c.addTitle("Bonding Specific parameters");
					
					field = { 
						"type": "text",
						"name": "sys_iface_" + iface + "_bond_ifaces",
						"text": "Interfaces",
						"descr": "Interfaces for bonding separated by space",
						"tip": "<b>Example:</b>eth0 eth1 dsl0<br><b>Note:</b>You can use only Ethernet-like" + 
							" interfaces, like ethX, dslX, bondX<br><b>Note:</b> Interfaces should be" + 
							" enabled, but <b>auto</b> should be switched <b>off</b>",
						"validator": {"required": true}
					};
					c.addWidget(field);
					
					c.addSubmit();
					
					break;
					
				case "bridge":
					c.addTitle("Bridge Specific parameters");
					
					field = { 
						"type": "checkbox",
						"name": "sys_iface_" + iface + "_br_stp",
						"text": "STP enabled",
						"descr": "Enable Spanning Tree Protocol",
						"tip": "Multiple ethernet bridges can work together to create even larger networks" + 
							" of ethernets using the IEEE 802.1d spanning tree protocol.This protocol is" + 
							" used for finding the shortest path between two ethernets, and for eliminating" + 
							" loops from the topology."
					};
					c.addWidget(field);
					
					field = { 
						"type": "text",
						"name": "sys_iface_" + iface + "_br_ifaces",
						"text": "Interfaces",
						"descr": "Interfaces for bridge separated by space",
						"tip": "<b>Example:</b> eth0 eth1 dsl0<br><b>Note:</b> You can use only" + 
						" Ethernet-like interfaces, like ethX, dslX<br><b>Note:</b> Interfaces should" + 
						" be enabled, but <b>auto</b> should be switched <b>off</b>.",
						"validator": {"required": true}
					};
					c.addWidget(field);
					
					field = { 
						"type": "text",
						"name": "sys_iface_" + iface + "_br_prio",
						"text": "Priority",
						"descr": "Bridge priority",
						"tip": "The priority value is an unsigned 16-bit quantity (a number between 0" + 
							" and 65535), and has no dimension. Lower priority values are better. The bridge" +
							" with the lowest priority will be elected <b>root bridge</b>.",
						"validator": {"min": 1, "max": 65535}
					};
					c.addWidget(field);
					
					field = { 
						"type": "text",
						"name": "sys_iface_" + iface + "_br_fd",
						"text": "Forward delay",
						"descr": "Forward delay in seconds.",
						"validator": {"min": 0, "max": 60}
					};
					c.addWidget(field);
					
					field = { 
						"type": "text",
						"name": "sys_iface_" + iface + "_br_hello",
						"text": "Hello time",
						"descr": "Hello time in seconds",
						"validator": {"min": 0, "max": 60}
					};
					c.addWidget(field);
					
					field = { 
						"type": "text",
						"name": "sys_iface_" + iface + "_br_maxage",
						"text": "Max age",
						"descr": "Max age in seconds",
						"validator": {"min": 0, "max": 600}
					};
					c.addWidget(field);
					
					c.addSubmit();
					
					break;
			}
		}
	});
	
	page.generateTabs();
};
