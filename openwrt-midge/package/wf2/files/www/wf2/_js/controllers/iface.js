Controllers['iface'] = function(iface) {
	var page = this.Page();
	page.setSubsystem("network");
	page.setHelpPage("iface");
	
	/* STATUS tab */
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
	
	/* GENERAL tab */
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
	
	/* METHOD tab */
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
	
	/* OPTIONS tab */
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
	
	/* SPECIFIC tab */
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
	
	/* ROUTES tab */
	
	/* route item */
	var routeItem = $.sprintf("sys_iface_%s_route_", iface);
	
	/* generate page with fields for adding new route */
	var addRoute = function(item) {
		var c, field;
		page.clearTab("routes");
		c = page.addContainer("routes");
		c.setHelpPage("traffic");
		c.setHelpSection("routes.list");

		if (!item) {
			c.addTitle("Add route");
			values = config.getParsed(routeItem + "*");
			item = routeItem + $.len(values);
		} else c.addTitle("Edit route");

		field = { 
			"type": "text",
			"item": item,
			"name": "net",
			"text": "Network",
			"descr": "Network (without mask) or host",
			"tip": "E.g., 192.168.0.0 or 10.0.0.1",
			"validator": {"required": true, "ipAddr": true}
		};
		c.addWidget(field);
		
		field = { 
			"type": "text",
			"item": item,
			"name": "netmask",
			"text": "Netmask",
			"descr": "Netmask for network or host (in xxx.xxx.xxx.xxx format)",
			"tip": "E.g., 255.255.255.0 — /24 — Class C network<br>255.255.255.252 — /30" +
				"<br>255.255.255.255 — /32 — for a single host",
			"validator": {"required": true, "netmask": true}
		};
		c.addWidget(field);
		
		field = { 
			"type": "text",
			"item": item,
			"name": "gw",
			"text": "Gateway",
			"descr": "Gateway for route",
			"validator": {"required": true, "ipAddr": true}
		};
		c.addWidget(field);
		
		c.addSubmit({
			"complexValue": item,
			"submitName": "Add/Update",
			"extraButton": {
				"name": "Back",
				"func": showRoutes
			},
			"onSubmit": showRoutes
		});
	};
	
	var showRoutes = function() {
		var c;
		page.clearTab("routes");
		c = page.addContainer("routes");
		c.setHelpPage("traffic");
		c.setHelpSection("routes");
		c.addTitle("Routes", 5);
	
		c.addTableHeader("Network|Mask|Gateway", addRoute);
		c.generateList(routeItem + "*", "net netmask gw", addRoute, showRoutes);
	};
	
	page.addTab({
		"id": "routes",
		"name": "Routes",
		"func": showRoutes
	});
	

	/* QoS tab */
	var showQos = function() {
		var c, field;
		c = page.addContainer("qos");
		c.addTitle("QoS settings");
		c.setHelpPage("qos");
		
		/* IDs of widgets */
		var widgetsIDs = new Array();
		
		/* remove widgets */
		var removeWidgets = function() {
			if (widgetsIDs.length != 0) {
				$.each(widgetsIDs, function(num, value) {
					$("#" + value).parents("tr").remove();
				});
				widgetsIDs = new Array();
			}
		};
		
		/* add widgets for pfifo and bfifo */
		var addFifoWidgets = function(type) {
			var id;
			
			id = $.sprintf("sys_iface_%s_qos_fifo_limit", iface);
			widgetsIDs.push(id);
			field = { 
				"type": "text",
				"name": id,
				"id": id,
				"text": "Buffer size",
				"validator": {"required": true, "min": 1, "max": 65535},
				"defaultValue": type == "pfifo" ? "128" : "10240"
			};
			c.addWidget(field);
		};
		
		/* add widgets for esfq */
		var addEsfqWidgets = function() {
			var id;
			
			id = $.sprintf("sys_iface_%s_qos_esfq_limit", iface);
			widgetsIDs.push(id);
			field = { 
				"type": "text",
				"name": id,
				"id": id,
				"text": "Limit",
				"descr": "Maximum packets in buffer",
				"validator": {"required": true, "min": 10, "max": 65535},
				"defaultValue": "128"
			};
			c.addWidget(field);
			
			id = $.sprintf("sys_iface_%s_qos_esfq_depth", iface);
			widgetsIDs.push(id);
			field = { 
				"type": "text",
				"name": id,
				"id": id,
				"text": "Depth",
				"validator": {"required": true, "min": 10, "max": 65535},
				"defaultValue": "128"
			};
			c.addWidget(field);
			
			id = $.sprintf("sys_iface_%s_qos_esfq_hash", iface);
			widgetsIDs.push(id);
			field = { 
				"type": "select",
				"name": id,
				"id": id,
				"text": "Hash",
				"options": {"classic": "Classic", "src": "Source address",
					"dst": "Destination address"},
				"defaultValue": "128"
			};
			c.addWidget(field);
		};
		
		/* add widgets for tbf */
		var addTbfWidgets = function() {
			var id;
			
			id = $.sprintf("sys_iface_%s_qos_tbf_rate", iface);
			widgetsIDs.push(id);
			field = { 
				"type": "text",
				"name": id,
				"id": id,
				"text": "Rate",
				"descr": "Maximum rate for interface",
				"validator": {"qosBandwith": true},
				"defaultValue": "512kbit",
				"tip": "Unit can be: <br><i>kbit</i>, <i>Mbit</i> — for bit per second<br>" +
					"and <i>kbps</i>, <i>Mbps</i> — for bytes per second"
			};
			c.addWidget(field);
		};
		
		field = { 
			"type": "select",
			"name": $.sprintf("sys_iface_%s_qos_sch", iface),
			"id": $.sprintf("sys_iface_%s_qos_sch", iface),
			"text": "Scheduler",
			"descr": "Scheduler for the interface",
			"options": {"pfifo_fast": "Default discipline pfifo_fast", "bfifo": "FIFO with bytes buffer",
				"pfifo": "FIFO with packets buffer", "tbf": "Token Bucket Filter",
				"sfq": "Stochastic Fairness Queueing", "esfq": "Enhanced Stochastic Fairness Queueing",
				"htb": "Hierarchical Token Bucket"},
			"onChange": function() {
				/* remove previous widgets */
				removeWidgets();
				
				var sch = $($.sprintf("#sys_iface_%s_qos_sch", iface)).val();
				switch (sch) {
					case "bfifo":
					case "pfifo":
						addFifoWidgets(sch);
						break;
					
					case "esfq":
						addEsfqWidgets();
						break;
					
					case "tbf":
						addTbfWidgets();
						break;
				}
				
				/* set nice tooltips for new fields */
				$("input").tooltip({"track": true});
				$("select").tooltip({"track": true});
			}
		};
		c.addWidget(field);
	
		c.addSubmit();
	}
	
	page.addTab({
		"id": "qos",
		"name": "QoS",
		"func": showQos
	});
	
	page.generateTabs();
};
