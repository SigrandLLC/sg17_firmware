Controllers['dhcp'] = function() {
	var page = this.Page();
	
	/* DHCP tab */
	page.addTab({
		"id": "dhcp",
		"name": "DHCP server",
		"func": function() {
			var c, field;
			c = page.addContainer("dhcp");
			c.addTitle("DHCP server interface select");
			
			var onInterfaceChange = function() {
				/* save selected interface in cookie */
				$.cookie("dhcpInterface", $("#dhcpInterface").val());
				
				/* remove br and all forms (except first) with DHCP settings */
				$("form:eq(1) ~ br, form:eq(0) ~ form").remove();
				
				/* add DHCP settings for selected interface */
				serviceDHCP(page, $("#dhcpInterface").val());
			};
			
			/* Generate list of interfaces which can use DHCP */
			var ifaces = "";
			$.each(config.getParsed("sys_ifaces"), function(num, iface) {
				var proto = config.get($.sprintf("sys_iface_%s_proto", iface));
				switch (proto) {
					case "ether":
					case "bridge":
					case "bonding":
					case "vlan":
						ifaces += iface + " ";
						break;
				}
			});
			ifaces = $.trim(ifaces);
			
			/* value of this widget is saved in cookie, because we not need it in KDB */
			field = { 
				"type": "select",
				"name": "dhcpInterface",
				"id": "dhcpInterface",
				"cookie": true,
				"text": "Interface",
				"descr": "Select network interface",
				"options": ifaces,
				"onChange": onInterfaceChange
			};
			c.addWidget(field);
			
			page.addBr("dhcp");
			
			onInterfaceChange();
		}
	});
	
	page.generateTabs();
};

/*
 * Adds DHCP settings for specified interface.
 * 
 * page — destination page;
 * iface — interface.
 */
function serviceDHCP(page, iface) {
	var c, field;
	
	c = page.addContainer("dhcp");
	c.setHelpSection("dhcp_server");
	c.addTitle("DHCP server on interface " + iface);
	c.setSubsystem("dhcp." + iface);
	
	field = { 
		"type": "checkbox",
		"name": $.sprintf("sys_iface_%s_dhcp_enabled", iface),
		"id": "dhcpEnabled",
		"text": "Enable DHCP server",
		"descr": "Run DHCP server on interface " + iface
	};
	c.addWidget(field);
	
	field = { 
		"type": "text",
		"name": $.sprintf("sys_iface_%s_dhcp_startip", iface),
		"text": "Start IP",
		"descr": "Start of dynamic IP address range for your LAN",
		"validator": {"required": true, "ipAddr": true}
	};
	c.addWidget(field);
	
	field = { 
		"type": "text",
		"name": $.sprintf("sys_iface_%s_dhcp_endip", iface),
		"text": "End IP",
		"descr": "End of dynamic IP address range for your LAN",
		"validator": {"required": true, "ipAddr": true}
	};
	c.addWidget(field);
	
	field = { 
		"type": "text",
		"name": $.sprintf("sys_iface_%s_dhcp_netmask", iface),
		"text": "Netmask",
		"descr": "Netmask for your LAN",
		"tip": "E.g., <i>255.255.255.0</i>",
		"validator": {"required": true, "netmask": true}
	};
	c.addWidget(field);
	
	field = { 
		"type": "text",
		"name": $.sprintf("sys_iface_%s_dhcp_router", iface),
		"text": "Default router",
		"descr": "Default router for your LAN hosts",
		"validator": {"ipAddr": true}
	};
	c.addWidget(field);
	
	field = { 
		"type": "select",
		"name": $.sprintf("sys_iface_%s_dhcp_lease_time", iface),
		"text": "Default lease time",
		"options": {"600": "10 minutes", "1800": "30 minutes", "3600": "1 hour", "10800": "3 hours",
			"36000": "10 hours", "86400": "24 hours"}
	};
	c.addWidget(field);
	
	field = { 
		"type": "text",
		"name": $.sprintf("sys_iface_%s_dhcp_nameserver", iface),
		"text": "DNS server",
		"descr": "DNS server for your LAN hosts",
		"validator": {"ipAddr": true}
	};
	c.addWidget(field);
	
	field = { 
		"type": "text",
		"name": $.sprintf("sys_iface_%s_dhcp_domain_name", iface),
		"text": "Domain",
		"descr": "Allows DHCP hosts to have fully qualified domain names",
		"tip": "Most queries for names within this domain can use short names relative to the local domain",
		"validator": {"domainName": true}
	};
	c.addWidget(field);
	
	field = { 
		"type": "text",
		"name": $.sprintf("sys_iface_%s_dhcp_ntpserver", iface),
		"text": "NTP server",
		"descr": "NTP server for your LAN hosts",
		"validator": {"ipAddr": true}
	};
	c.addWidget(field);
	
	field = { 
		"type": "text",
		"name": $.sprintf("sys_iface_%s_dhcp_winsserver", iface),
		"text": "WINS server",
		"descr": "WINS server for your LAN hosts",
		"validator": {"ipAddr": true}
	};
	c.addWidget(field);
	
	c.addSubmit();
	
	page.addBr("dhcp");
	serviceDHCPStatic(page, iface)
}

/*
 * Adds DHCP static lease table.
 * 
 * page — destination page;
 * iface — interface.
 */
function serviceDHCPStatic(page, iface) {
	/*
	 * DHCP page can be accessed from interface page and from DHCP service page,
	 * which are different. But both pages have the same tab id, so we just imitate
	 * the click on the tab name to generate the contents of the page.
	 */
	var showDHCP = function() {
		$("#tab_dhcp_link").click();
	};
	
	var dhcpItem = $.sprintf("sys_iface_%s_dhcp_host_", iface);
	
	/* generate page with fields for adding new static lease */
	var addStatic = function(item) {
		var c, field;
		page.clearTab("dhcp");
		c = page.addContainer("dhcp");
		c.setHelpPage("dhcp_server");
		c.setHelpSection("dhcp_server.static_add");
		c.setSubsystem("dhcp." + iface);

		if (!item) {
			c.addTitle("Add static lease");
			values = config.getParsed(dhcpItem + "*");
			item = dhcpItem + $.len(values);
		} else c.addTitle("Edit static lease");
		
		field = { 
			"type": "text",
			"item": item,
			"name": "name",
			"text": "Host name",
			"validator": {"required": true, "alphanum": true}
		};
		c.addWidget(field);
		
		field = { 
			"type": "text",
			"item": item,
			"name": "ipaddr",
			"text": "IP Address",
			"descr": "IP address for host",
			"validator": {"required": true, "ipAddr": true}
		};
		c.addWidget(field);
		
		field = { 
			"type": "text",
			"item": item,
			"name": "hwaddr",
			"text": "MAC Address",
			"descr": "MAC Address of host",
			"validator": {"required": true, "macAddr": true}
		};
		c.addWidget(field);
		
		c.addSubmit({
			"complexValue": item,
			"submitName": "Add/Update",
			"extraButton": {
				"name": "Back",
				"func": showDHCP
			},
			"onSubmit": showDHCP
		});
	};
	
	c = page.addContainer("dhcp");
	c.setSubsystem("dhcp." + iface);
	c.addTitle("DHCP static addresses on interface " + iface, 5);
	c.addTableHeader("Name|IP address|MAC address", addStatic);
	c.generateList(dhcpItem + "*", "name ipaddr hwaddr", addStatic, showDHCP);
}
