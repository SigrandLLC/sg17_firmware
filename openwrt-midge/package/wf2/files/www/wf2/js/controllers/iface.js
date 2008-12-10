Controllers.iface = function(iface) {
	var page = this.Page();
	page.setSubsystem("network." + iface);
	page.setHelpPage("iface");
	
	/* STATUS tab */
	page.addTab({
		"id": "status",
		"name": "Status",
		"func": function() {
			var c, field;
			var realIface = config.get($.sprintf("sys_iface_%s_real", iface)) ?
				config.get($.sprintf("sys_iface_%s_real", iface)) : iface;
			
			c = page.addContainer("status");
			c.addTitle("Interface status");
			
			/* add general widget, which will contain three buttons */
			field = {
				"type": "general",
				"name": "interface_ctrl",
				"text": "Interface control"
			};
			c.addWidget(field);
			
			/* restart button */
			field = {
				"type": "button",
				"name": "btn_restart",
				"text": "Restart",
				"func": function(thisContainer, src) {
					$(src.currentTarget).attr("disabled", true);
					config.cmdExecute({
						"cmd": $.sprintf("/etc/init.d/network restart %s", realIface),
						"callback": function() {
							$(src.currentTarget).removeAttr("disabled");
							c.containerRedraw();
						}
					});
				}
			};
			c.addSubWidget(field, {
					"type": "prependToAnchor",
					"anchor": "#td_interface_ctrl"
				}
			);
			
			/* stop button */
			field = {
				"type": "button",
				"name": "btn_stop",
				"text": "Stop",
				"func": function(thisContainer, src) {
					$(src.currentTarget).attr("disabled", true);
					config.cmdExecute({
						"cmd": $.sprintf("/sbin/ifdown %s", realIface),
						"callback": function() {
							$(src.currentTarget).removeAttr("disabled");
							c.containerRedraw();
						}
					});
				}
			};
			c.addSubWidget(field, {
					"type": "prependToAnchor",
					"anchor": "#td_interface_ctrl"
				}
			);
			
			/* start button */
			field = {
				"type": "button",
				"name": "btn_start",
				"text": "Start",
				"func": function(thisContainer, src) {
					$(src.currentTarget).attr("disabled", true);
					config.cmdExecute({
						"cmd": $.sprintf("/sbin/ifup %s", realIface),
						"callback": function() {
							$(src.currentTarget).removeAttr("disabled");
							c.containerRedraw();
						}
					});
				}
			};
			c.addSubWidget(field, {
					"type": "prependToAnchor",
					"anchor": "#td_interface_ctrl"
				}
			);
			
			page.addBr("status");
			c = page.addContainer("status");
			c.addTitle("Interface status");
			c.addConsole(["/sbin/ifconfig " + realIface, "/usr/sbin/ip addr show dev " + realIface,
				"/usr/sbin/ip link show dev " + realIface]);
			
			page.addBr("status");
			c = page.addContainer("status");
			c.addTitle("Routes");
			c.addConsole("/usr/sbin/ip route show dev " + realIface);
			
			page.addBr("status");
			c = page.addContainer("status");
			c.addTitle("ARP");
			c.addConsole("/usr/sbin/ip neigh show dev " + realIface);
			
			/* add additional info for bridge interface */
			if (realIface.search(/^br/) != -1) {
				page.addBr("status");
				c = page.addContainer("status");
				c.addTitle("System bridges");
				c.addConsole("/usr/sbin/brctl show");
				
				page.addBr("status");
				c = page.addContainer("status");
				c.addTitle($.sprintf("Bridge %s info", realIface));
				c.addConsole("/usr/sbin/brctl showmacs " + realIface);
				
				page.addBr("status");
				c = page.addContainer("status");
				c.addTitle($.sprintf("STP bridge %s info", realIface));
				c.addConsole("/usr/sbin/brctl showstp " + realIface);
			}
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
			
			if (config.get($.sprintf("sys_iface_%s_proto", iface)) != "vlan") {
				var dependList = new Object();
				$.each(config.getParsed("sys_ifaces"), function(name, value) {
					dependList[value] = value;
				});
				dependList['none'] = "None";
				field = { 
					"type": "select",
					"name": "sys_iface_" + iface + "_depend_on",
					"text": "Depends on",
					"options": dependList,
					"defaultValue": "none"
				};
				c.addWidget(field);
			}
		
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
			if (config.get($.sprintf("sys_iface_%s_proto", iface)) == "hdlc")
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
						"name": $.sprintf("sys_iface_%s_bond_ifaces", iface),
						"text": "Interfaces",
						"descr": "Interfaces for bonding separated by space",
						"tip": "<b>Example:</b>eth0 eth1 dsl0<br><b>Note:</b>You can use only Ethernet-like" + 
							" interfaces, like ethX, dslX, bondX<br><b>Note:</b> Interfaces should be" + 
							" enabled, but <b>auto</b> should be switched <b>off</b>",
						"validator": {"required": true}
					};
					c.addWidget(field);
					
					/* set auto=0 enabled=1 for depending interfaces */
					var additionalKeys = [];
					c.addSubmit({
						"additionalKeys": additionalKeys,
						"preSubmit": function() {
							$.each($($.sprintf("#sys_iface_%s_bond_ifaces", iface)).val().split(" "),
								function(num, value) {
									$.addObjectWithProperty(additionalKeys, $.sprintf("sys_iface_%s_auto", value), "0");
									$.addObjectWithProperty(additionalKeys, $.sprintf("sys_iface_%s_enabled", value), "1");
								});
						}
					});
					
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
					
					/* set auto=0 enabled=1 for depending interfaces */
					var additionalKeys = [];
					c.addSubmit({
						"additionalKeys": additionalKeys,
						"preSubmit": function() {
							$.each($($.sprintf("#sys_iface_%s_br_ifaces", iface)).val().split(" "),
								function(num, value) {
									$.addObjectWithProperty(additionalKeys, $.sprintf("sys_iface_%s_auto", value), "0");
									$.addObjectWithProperty(additionalKeys, $.sprintf("sys_iface_%s_enabled", value), "1");
								});
						}
					});
					
					break;
			}
		}
	});
	
	/* ROUTES tab */	
	page.addTab({
		"id": "routes",
		"name": "Routes",
		"func": function() {
			var c = page.addContainer("routes");
			c.setHelpPage("traffic");
			c.setHelpSection("routes");
			
			/* create list of routes */
			var list = c.createList({
				"tabId": "routes",
				"header": ["Network", "Mask", "Gateway"],
				"varList": ["net", "netmask", "gw"],
				"listItem": $.sprintf("sys_iface_%s_route_", iface),
				"addMessage": "Add route",
				"editMessage": "Edit route",
				"listTitle": "Routes",
				"helpPage": "traffic",
				"helpSection": "routes.list"
			});
			
			field = { 
				"type": "text",
				"name": "net",
				"text": "Network",
				"descr": "Network (without mask) or host",
				"tip": "E.g., 192.168.0.0 or 10.0.0.1",
				"validator": {"required": true, "ipAddr": true}
			};
			list.addWidget(field);
			
			field = { 
				"type": "text",
				"name": "netmask",
				"text": "Netmask",
				"descr": "Netmask for network or host (in xxx.xxx.xxx.xxx format)",
				"tip": "E.g., 255.255.255.0 — /24 — Class C network<br>255.255.255.252 — /30" +
					"<br>255.255.255.255 — /32 — for a single host",
				"validator": {"required": true, "netmask": true}
			};
			list.addWidget(field);
			
			field = { 
				"type": "text",
				"name": "gw",
				"text": "Gateway",
				"descr": "Gateway for route",
				"validator": {"required": true, "ipAddr": true}
			};
			list.addWidget(field);
			
			list.generateList();
		}
	});

	/* QoS tab */
	var showQos = function() {
		/* add widgets for PFIFO and BFIFO */
		var addFifoWidgets = function(type) {
			var field;
			field = { 
				"type": "text",
				"name": $.sprintf("sys_iface_%s_qos_fifo_limit", iface),
				"text": "Buffer size",
				"descr": $.sprintf("Queue length in %s", type == "pfifo" ? "packets" : "bytes"),
				"validator": {"required": true, "min": 1, "max": 65535},
				"defaultValue": type == "pfifo" ? "128" : "10240"
			};
			c.addWidget(field);
		};
		
		/* add widgets for ESFQ */
		var addEsfqWidgets = function() {
			var field;
			field = { 
				"type": "text",
				"name": $.sprintf("sys_iface_%s_qos_esfq_limit", iface),
				"text": "Limit",
				"descr": "Maximum packets in buffer",
				"validator": {"required": true, "min": 10, "max": 65535},
				"defaultValue": "128"
			};
			c.addWidget(field);
			
			field = { 
				"type": "text",
				"name": $.sprintf("sys_iface_%s_qos_esfq_depth", iface),
				"text": "Depth",
				"validator": {"required": true, "min": 10, "max": 65535},
				"defaultValue": "128"
			};
			c.addWidget(field);
			
			field = { 
				"type": "select",
				"name": $.sprintf("sys_iface_%s_qos_esfq_hash", iface),
				"text": "Hash",
				"options": {"classic": "Classic", "src": "Source address",
					"dst": "Destination address"},
				"defaultValue": "128"
			};
			c.addWidget(field);
		};
		
		/* add widgets for TBF */
		var addTbfWidgets = function() {
			var field;
	
			field = { 
				"type": "text",
				"name": $.sprintf("sys_iface_%s_qos_tbf_rate", iface),
				"text": "Rate",
				"descr": "Maximum rate for interface",
				"validator": {"required": true, "qosBandwith": true},
				"defaultValue": "512kbit",
				"tip": "Unit can be: <br><i>kbit</i>, <i>Mbit</i> — for bit per second<br>" +
					"and <i>kbps</i>, <i>Mbps</i> — for bytes per second"
			};
			c.addWidget(field);
		};
		
		/* add widgets for HTB */
		var addHtbWidgets = function() {
			var field;
			
			/* generate list of available default classes */
			var defaultClasses = {"0": "none"};
			$.each(config.getParsed($.sprintf("sys_iface_%s_qos_htb_class_*", iface)),
				function(key, value) {
					var classId = value['classid'].split(":")[1];
					defaultClasses[classId] = value['name'];
				}
			);
	
			field = { 
				"type": "select",
				"name": $.sprintf("sys_iface_%s_qos_htb_default", iface),
				"text": "Default class",
				"descr": "Name of default class",
				"options": defaultClasses,
				"defaultValue": "0"
			};
			c.addWidget(field);			
			
			/*
			 * Callback for generateList(), returns name of parent class.
			 * 
			 * varName — item's variable name;
			 * varValue — item's variable value.
			 */
			var getParentName = function(varName, varValue) {
				/* if current variable is not "parent" — return without modification */
				if (varName != "parent" && varName != "flowid") return varValue;
				
				/* if value of variable is "1:0" — class name is root */
				if (varValue == "1:0") return "root";
				
				/* search class with classid varValue and saves it's name to parentName */
				var parentName = "ERROR";
				var classes = config.getParsed($.sprintf("sys_iface_%s_qos_htb_class_*", iface));
				$.each(classes, function(classKey, classValues) {
					if (classValues['classid'] == varValue) {
						parentName = classValues['name'];
						return false;
					}
				});
				
				return parentName;
			};
			
			/* add Classes table */
			page.addBr("qos");

			/* we use different variable because "c" is used to add elements to main form */
			var c2 = page.addContainer("qos");
			c2.setHelpPage("htb");
			c2.setHelpSection("htb");
			
			/* create list of classes */
			var list = c2.createList({
				"tabId": "qos",
				"header": ["Name", "Parent", "Rate", "Ceil", "Qdisc"],
				"varList": ["name", "parent", "rate", "ceil", "qdisc"],
				"listItem": $.sprintf("sys_iface_%s_qos_htb_class_", iface),
				"processValueFunc": getParentName,
				"addMessage": "Add QoS HTB class",
				"editMessage": "Edit QoS HTB class",
				"listTitle": $.sprintf("Classes on %s", iface),
				"helpPage": "htb",
				"helpSection": "htb_class_add"
			});
			
			field = {
				"type": "checkbox",
				"name": "enabled",
				"text": "Enabled",
				"descr": "Check this item to enable class"
			};
			list.addWidget(field);
			
			field = {
				"type": "text",
				"name": "name",
				"text": "Name",
				"descr": "Name of class",
				"validator": {"required": true, "alphanumU": true}
			};
			list.addWidget(field);
			
			/* generate list of available parent classes and find next classid */
			var max = 0;
			var parentClasses = {"1:0": "root"};
			$.each(config.getParsed($.sprintf("sys_iface_%s_qos_htb_class_*", iface)),
				function(key, value) {
					parentClasses[value['classid']] = value['name'];
					var cur = parseInt(value['classid'].split(":")[1]);
					if (cur > max) max = cur;
				}
			);
			var classid = "1:" + (max + 1);
			
			/*
			 * if we are editing class — it's classid will be get from KDB,
			 * if we are adding new class — we generate new classid and set it as default value.
			 */
			field = {
				"type": "hidden",
				"name": "classid",
				"defaultValue": classid
			};
			list.addWidget(field);
			
			field = { 
				"type": "select",
				"name": "parent",
				"text": "Parent class",
				"descr": "Name of parent class",
				"options": parentClasses
			};
			list.addWidget(field);
			
			field = {
				"type": "text",
				"name": "rate",
				"text": "Rate",
				"descr": "Class rate",
				"tip": "Unit can be: <br><i>kbit</i>, <i>Mbit</i> — for bit per second<br>" +
					"and <i>kbps</i>, <i>Mbps</i> — for bytes per second.",
				"validator": {"required": true, "qosBandwith": true}
			};
			list.addWidget(field);
			
			field = {
				"type": "text",
				"name": "ceil",
				"text": "Ceil",
				"descr": "Max rate",
				"tip": "Unit can be: <br><i>kbit</i>, <i>Mbit</i> — for bit per second<br>" +
					"and <i>kbps</i>, <i>Mbps</i> — for bytes per second.",
				"validator": {"qosBandwith": true}
			};
			list.addWidget(field);
			
			field = {
				"type": "text",
				"name": "qdisc",
				"text": "Qdisc",
				"descr": "Qdisc for this class",
				"tip": "Optional qdisc for this class.<br><i>ATTENTION! At this moment, " +
					"you have to use character '#' for spaces.</i><br>For example, you can enter " +
					"<i>esfq#limit#128#depth#128#divisor#10#hash#classic#perturb#15</i> " +
					"or <i>sfq#perturb#10</i>, etc."
			};
			list.addWidget(field);
			
			list.generateList();
			
			/* add Filter table */
			page.addBr("qos");

			/* we use different variable because "c" is used to add elements to main form */
			c2 = page.addContainer("qos");
			c2.setHelpPage("htb");
			c2.setHelpSection("htb");
			
			/* create list of filter */
			var list = c2.createList({
				"tabId": "qos",
				"header": ["Name", "Prio", "Proto", "Src addr", "Dst addr", "Src port", "Dst port", "Class"],
				"varList": ["name", "prio", "proto", "src", "dst", "src_port", "dst_port", "flowid"],
				"listItem": $.sprintf("sys_iface_%s_qos_htb_filter_", iface),
				"processValueFunc": getParentName,
				"addMessage": "Add QoS HTB filter",
				"editMessage": "Edit QoS HTB filter",
				"listTitle": $.sprintf("Filters on %s", iface),
				"helpPage": "htb",
				"helpSection": "htb_filter_add"
			});
			
			field = {
				"type": "checkbox",
				"name": "enabled",
				"text": "Enabled",
				"descr": "Check this item to enable class"
			};
			list.addWidget(field);
			
			field = {
				"type": "text",
				"name": "name",
				"text": "Name",
				"descr": "Name of filter",
				"validator": {"required": true, "alphanumU": true}
			};
			list.addWidget(field);
			
			field = {
				"type": "text",
				"name": "prio",
				"text": "Prio",
				"descr": "Rule priority",
				"tip": "Prio can be any positive integer value.<br><i>Examples:</i> 1, 10, 17.",
				"validator": {"required": true, "min": 1, "max": 65535},
				"defaultValue": "1"
			};
			list.addWidget(field);
			
			field = { 
				"type": "select",
				"name": "proto",
				"text": "Protocol",
				"descr": "A protocol of the packet to check",
				"defaultValue": "any",
				"options": "any tcp udp icmp"
			};
			list.addWidget(field);
			
			var tip = "Address can be either a network IP address (with /mask), or a plain IP address, " +
				"A ! argument before the address specification inverts the sense of the address." +
				"<br><b>Examples:</b> 192.168.1.0/24, 192.168.1.5<br> Use 0.0.0.0/0 for <b>any</b>";
		
			field = { 
				"type": "text",
				"name": "src",
				"text": "Source IP",
				"descr": "Source address",
				"validator": {"required": true, "ipNetMaskIptables": true},
				"defaultValue": "0.0.0.0/0",
				"tip": tip
			};
			list.addWidget(field);
			
			field = { 
				"type": "text",
				"name": "dst",
				"text": "Destination IP",
				"descr": "Destination address",
				"validator": {"required": true, "ipNetMaskIptables": true},
				"defaultValue": "0.0.0.0/0",
				"tip": tip
			};
			list.addWidget(field);
			
			field = { 
				"type": "text",
				"name": "src_port",
				"text": "Source port",
				"descr": "Source port",
				"validator": {"required": true, "ipPort": true},
				"defaultValue": "any"
			};
			list.addWidget(field);
			
			field = { 
				"type": "text",
				"name": "dst_port",
				"text": "Destination port",
				"descr": "Destination port",
				"validator": {"required": true, "ipPort": true},
				"defaultValue": "any"
			};
			list.addWidget(field);
			
			/* generate list of available classes */
			var classes = new Object();
			$.each(config.getParsed($.sprintf("sys_iface_%s_qos_htb_class_*", iface)),
				function(key, value) {
					classes[value['classid']] = value['name'];
				}
			);
			
			field = { 
				"type": "select",
				"name": "flowid",
				"text": "Class",
				"descr": "Put matching packets in this class",
				"options": classes
			};
			list.addWidget(field);
			
			list.generateList();
		};
		
		/* add widgets specific for selected scheduler */
		var addSchedulerWidgets = function() {
			var sch = $("#sch").val();
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
				
				case "htb":
					addHtbWidgets();
					break;
			}
		};
		
		/*
		 * on scheduler change remove unnecessary widgets (all except scheduler select)
		 * and add new widgets.
		 */
		var onSchedulerUpdate = function() {
			/* get tab object */
			var tab = page.getTab("qos");
			
			/* remove all widgets except scheduler select */
			$("tbody > tr", tab).not("tr:has(#sch)").remove();
			
			/* remove all forms except first one (with scheduler select) */
			$("form:not(:first)", tab).remove();
			
			/* remove all br between forms */
			$("form:first ~ br", tab).remove();
			
			/* add new widgets */
			addSchedulerWidgets();
		};
		
		/* add main form */
		var c, field;
		page.clearTab("qos");
		c = page.addContainer("qos");
		c.setHelpPage("qos");
		c.addTitle("QoS settings");
		
		/* Scheduler */
		field = { 
			"type": "select",
			"name": $.sprintf("sys_iface_%s_qos_sch", iface),
			"id": "sch",
			"text": "Scheduler",
			"descr": "Scheduler for the interface",
			"options": {
				"pfifo_fast": "Default discipline pfifo_fast",
				"bfifo": "FIFO with bytes buffer",
				"pfifo": "FIFO with packets buffer",
				"tbf": "Token Bucket Filter",
				"sfq": "Stochastic Fairness Queueing",
				"esfq": "Enhanced Stochastic Fairness Queueing",
				"htb": "Hierarchical Token Bucket"
			},
			"onChange": onSchedulerUpdate,
			"defaultValue": "pfifo_fast"
		};
		c.addWidget(field);
		
		addSchedulerWidgets();
		
		c.addSubmit();
	}
	
	page.addTab({
		"id": "qos",
		"name": "QoS",
		"func": showQos
	});
	
	/* DHCP tab */
	page.addTab({
		"id": "dhcp",
		"name": "DHCP",
		"func": function() {
			serviceDHCP(page, iface);
		}
	});
	
	page.generateTabs();
};
