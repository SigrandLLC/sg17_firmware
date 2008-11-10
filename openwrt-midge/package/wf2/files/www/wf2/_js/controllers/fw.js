Controllers['fw'] = function() {
	var page = this.Page();
	page.setSubsystem("fw");
	page.setHelpPage("fw");
	
	/* settings tab */
	page.addTab({
		"id": "settings",
		"name": "Firewall settings",
		"func": function() {
			var c, field;
			c = page.addContainer("settings");
			c.addTitle("Firewall settings");
		
			field = { 
				"type": "checkbox",
				"name": "sys_fw_enabled",
				"text": "Enable firewall",
				"descr": "Check this item if you want use firewall"
			};
			c.addWidget(field);
			
			c.addSubmit();
		}
	});
	
	/*
	 * Event handler.
	 * When adding or editing FW rule:
	 * when selected targets DNAT or SNAT, add NatTo widget.
	 * 
	 * list — current list to add new widget to (passed automatically by framework).
	 */
	var onChangeTarget = function(list) {
		/* get type of target */
		var target = $("#target").val();
		
		/* if target is DNAT or SNAT and there is no NatTo field — add it */
		if ((target == "DNAT" || target == "SNAT") && $("#natto").length == 0) {
			/* add new field */
			var field = { 
				"type": "text",
				"name": "natto",
				"id": "natto",
				"text": "Nat to address",
				"descr": target == "DNAT" ? "Do Destination NAT to address" : "Do Source NAT to address",
				"validator": {"required": true, "ipAddrPort": true},
				"tip": "You can add port number after ip address<br><i>Example: 192.168.0.1:80</i>"
			};
			list.addDynamicWidget(field, $("#dport").parents("tr"));
		/* remove field */
		} else {
			$("#natto").parents("tr").remove();
		}
	};
	
	/*
	 * Adds widgets for adding or editing FW rules.
	 * 
	 * options — array of options:
	 *  - list — list object to add widgets to;
	 *  - chain — name of chain for which widgets is added.
	 */
	var addFwAddingRuleWidgets = function(options) {
		var field;
		
		field = { 
			"type": "checkbox",
			"name": "enabled",
			"text": "Enabled",
			"descr": "Check this item to enable rule"
		};
		options['list'].addWidget(field);
		
		field = { 
			"type": "text",
			"name": "name",
			"text": "Short name",
			"descr": "Name of rule",
			"validator": {"required": true, "alphanumU": true}
		};
		options['list'].addWidget(field);
		
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
		options['list'].addWidget(field);
		
		field = { 
			"type": "text",
			"name": "dst",
			"text": "Destination IP",
			"descr": "Destination address",
			"validator": {"required": true, "ipNetMaskIptables": true},
			"defaultValue": "0.0.0.0/0",
			"tip": tip
		};
		options['list'].addWidget(field);
		
		field = { 
			"type": "select",
			"name": "proto",
			"text": "Protocol",
			"descr": "A protocol of the packet to check",
			"defaultValue": "all",
			"options": "all tcp udp icmp"
		};
		options['list'].addWidget(field);
		
		tip = "An inclusive range can also be specified, using the format <b>port:port</b>. " +
				"If the first port is omitted, 0 is assumed; if the last is omitted, 65535 is assumed.";

		field = { 
			"type": "text",
			"name": "sport",
			"text": "Source port",
			"descr": "Source port or port range",
			"validator": {"required": true, "ipPortRange": true},
			"defaultValue": "any",
			"tip": tip
		};
		options['list'].addWidget(field);
		
		field = { 
			"type": "text",
			"name": "dport",
			"id": "dport",
			"text": "Destination port",
			"descr": "Destination port or port range",
			"validator": {"required": true, "ipPortRange": true},
			"defaultValue": "any",
			"tip": tip
		};
		options['list'].addWidget(field);
		
		var targets = "ACCEPT DROP REJECT";
		
		/* depending on chain add additional targets */
		switch (options['chain']) {
			case "PREROUTING":
				targets += " DNAT";
				break;
			
			case "POSTROUTING":
				targets += " SNAT MASQUERADE";
				break;
		}
		
		field = { 
			"type": "select",
			"name": "target",
			"id": "target",
			"text": "Action",
			"descr": "What to do with packet",
			"defaultValue": "ACCEPT",
			"options": targets,
			"onChange": onChangeTarget
		};
		options['list'].addWidget(field);
	};
	
	/* FILTER tab */
	page.addTab({
		"id": "filter",
		"name": "Filter",
		"func": function() {
			var c, field, list;
			
			/* policies */
			c = page.addContainer("filter");
			c.setHelpSection("filter_policy");
			c.addTitle("Default policies");
		
			field = { 
				"type": "select",
				"name": "sys_fw_filter_policy_forward",
				"text": "Default policy for FORWARD",
				"options": "ACCEPT DROP"
			};
			c.addWidget(field);
			
			field = { 
				"type": "select",
				"name": "sys_fw_filter_policy_input",
				"text": "Default policy for INPUT",
				"options": "ACCEPT DROP"
			};
			c.addWidget(field);
			
			field = { 
				"type": "select",
				"name": "sys_fw_filter_policy_output",
				"text": "Default policy for OUTPUT",
				"options": "ACCEPT DROP"
			};
			c.addWidget(field);
			
			c.addSubmit();
			
			/* FORWARD chain */
			page.addBr("filter");
			c = page.addContainer("filter");
			c.setHelpSection("filter_forward");
			
			/* create list of forward rules */
			list = c.createList({
				"tabId": "filter",
				"header": ["Name", "Src", "Dst", "Proto", "Src port", "Dst port", "Action"],
				"varList": ["name", "src", "dst", "proto", "sport", "dport", "target"],
				"listItem": "sys_fw_filter_forward_",
				"onAddOrEditItemRender": onChangeTarget,
				"addMessage": "Add rule to FORWARD chain",
				"editMessage": "Edit rule in FORWARD chain",
				"listTitle": "Filter, FORWARD chain",
				"helpPage": "filter",
				"helpSection": "filter_add"
			});
			
			addFwAddingRuleWidgets({
				"list": list,
				"chain": "FORWARD"
			});
			
			list.generateList();
			
			/* INPUT chain */
			page.addBr("filter");
			c = page.addContainer("filter");
			c.setHelpSection("filter_input");
			
			/* create list of input rules */
			list = c.createList({
				"tabId": "filter",
				"header": ["Name", "Src", "Dst", "Proto", "Src port", "Dst port", "Action"],
				"varList": ["name", "src", "dst", "proto", "sport", "dport", "target"],
				"listItem": "sys_fw_filter_input_",
				"onAddOrEditItemRender": onChangeTarget,
				"addMessage": "Add rule to INPUT chain",
				"editMessage": "Edit rule in INPUT chain",
				"listTitle": "Filter, INPUT chain",
				"helpPage": "filter",
				"helpSection": "filter_add"
			});
			
			addFwAddingRuleWidgets({
				"list": list,
				"chain": "INPUT"
			});
			
			list.generateList();
			
			/* OUTPUT chain */
			page.addBr("filter");
			c = page.addContainer("filter");
			c.setHelpSection("filter_output");
			
			/* create list of output rules */
			list = c.createList({
				"tabId": "filter",
				"header": ["Name", "Src", "Dst", "Proto", "Src port", "Dst port", "Action"],
				"varList": ["name", "src", "dst", "proto", "sport", "dport", "target"],
				"listItem": "sys_fw_filter_output_",
				"onAddOrEditItemRender": onChangeTarget,
				"addMessage": "Add rule to OUTPUT chain",
				"editMessage": "Edit rule in OUTPUT chain",
				"listTitle": "Filter, OUTPUT chain",
				"helpPage": "filter",
				"helpSection": "filter_add"
			});
			
			addFwAddingRuleWidgets({
				"list": list,
				"chain": "OUTPUT"
			});
			
			list.generateList();
		}
	});
	
	/* NAT tab */
	page.addTab({
		"id": "nat",
		"name": "NAT",
		"func": function() {
			var c, field, list;
		
			/* policies */
			c = page.addContainer("nat");
			c.setHelpPage("nat");
			c.setHelpSection("nat_policy");
			c.addTitle("Default policies");
		
			field = { 
				"type": "select",
				"name": "sys_fw_nat_policy_prerouting",
				"text": "Default policy for PREROUTING",
				"options": "ACCEPT DROP"
			};
			c.addWidget(field);
			
			field = { 
				"type": "select",
				"name": "sys_fw_nat_policy_postrouting",
				"text": "Default policy for POSTROUTING",
				"options": "ACCEPT DROP"
			};
			c.addWidget(field);
			
			c.addSubmit();
			
			/* PREROUTING chain */
			page.addBr("nat");
			c = page.addContainer("nat");
			c.setHelpPage("nat");
			c.setHelpSection("nat_prerouting");
			
			/* create list of prerouting rules */
			list = c.createList({
				"tabId": "nat",
				"header": ["Name", "Src", "Dst", "Proto", "Src port", "Dst port", "Action"],
				"varList": ["name", "src", "dst", "proto", "sport", "dport", "target"],
				"listItem": "sys_fw_nat_prerouting_",
				"onAddOrEditItemRender": onChangeTarget,
				"addMessage": "Add rule to PREROUTING chain",
				"editMessage": "Edit rule in PREROUTING chain",
				"listTitle": "NAT, PREROUTING chain",
				"helpPage": "nat",
				"helpSection": "nat_add"
			});
			
			addFwAddingRuleWidgets({
				"list": list,
				"chain": "PREROUTING"
			});
			
			list.generateList();
			
			/* POSTROUTING chain */
			page.addBr("nat");
			c = page.addContainer("nat");
			c.setHelpPage("nat");
			c.setHelpSection("nat_postrouting");
			
			/* create list of postrouting rules */
			list = c.createList({
				"tabId": "nat",
				"header": ["Name", "Src", "Dst", "Proto", "Src port", "Dst port", "Action"],
				"varList": ["name", "src", "dst", "proto", "sport", "dport", "target"],
				"listItem": "sys_fw_nat_postrouting_",
				"onAddOrEditItemRender": onChangeTarget,
				"addMessage": "Add rule to POSTROUTING chain",
				"editMessage": "Edit rule in POSTROUTING chain",
				"listTitle": "NAT, POSTROUTING chain",
				"helpPage": "nat",
				"helpSection": "nat_add"
			});
			
			addFwAddingRuleWidgets({
				"list": list,
				"chain": "POSTROUTING"
			});
			
			list.generateList();
		}
	});
	
	page.generateTabs();
};
