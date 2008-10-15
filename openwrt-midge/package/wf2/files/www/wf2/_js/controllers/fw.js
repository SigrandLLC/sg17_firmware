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
	 * Generate page with fields for adding new fw rule.
	 * 
	 * options['tab'] — tab ID;
	 * options['helpPage'] — help page;
	 * options['helpSection'] — help section;
	 * options['item'] — name of rule item in KDB (e.g., sys_fw_filter_forward_2);
	 * options['itemTpl'] — name of rule item template (e.g., sys_fw_filter_forward_);
	 * options['showFunc'] — func to call on submit and on click on Back buton (to show list of items);
	 * options['chain'] — netfilter chain (input, output, postrouting, ...).
	 */
	var addFwRule = function(options) {
		var c, field, item;
		page.clearTab(options['tab']);
		c = page.addContainer(options['tab']);
		options['helpPage'] && c.setHelpPage(options['helpPage']);
		options['helpSection'] && c.setHelpSection(options['helpSection']);

		/* add new item */
		if (!options['item']) {
			c.addTitle($.sprintf("Add rule to %s chain", options['chain']));
			values = config.getParsed(options['itemTpl'] + "*");
			item = options['itemTpl'] + $.len(values);
		/* edit selected item */
		} else {
			c.addTitle($.sprintf("Edit rule in %s chain", options['chain']));
			item = options['item'];
		}
		
		/* when selected targets DNAT or SNAT, add NatTo field */
		var onChangeTarget = function() {
			/* get type of target */
			var target = $("#target").val();
			
			/* if target is DNAT or SNAT and there is no NatTo field — add it */
			if ((target == "DNAT" || target == "SNAT") && $("#natto").length == 0) {
				/* add new field */
				field = { 
					"type": "text",
					"item": item,
					"name": "natto",
					"id": "natto",
					"text": "Nat to address",
					"descr": "Do Source NAT or Destination NAT to address",
					"validator": {"required": true, "ipAddrPort": true},
					"tip": "You can add port number after ip address<br><i>Example: 192.168.0.1:80</i>"
				};
				c.addWidget(field, $("#dport").parents("tr"));
				
				/* set nice tooltips for new field */
				$("#natto").tooltip({"track": true});
				
			/* remove field */
			} else {
				$("#natto").parents("tr").remove();
			}
		};

		field = { 
			"type": "checkbox",
			"item": item,
			"name": "enabled",
			"text": "Enabled",
			"descr": "Check this item to enable rule"
		};
		c.addWidget(field);
		
		field = { 
			"type": "text",
			"item": item,
			"name": "name",
			"text": "Short name",
			"descr": "Name of rule",
			"validator": {"required": true, "alphanum": true}
		};
		c.addWidget(field);
		
		var tip = "Address can be either a network IP address (with /mask), or a plain IP address, " +
				"A ! argument before the address specification inverts the sense of the address." +
				"<br><b>Examples:</b> 192.168.1.0/24, 192.168.1.5<br> Use 0.0.0.0/0 for <b>any</b>";
		
		field = { 
			"type": "text",
			"item": item,
			"name": "src",
			"text": "Source",
			"descr": "Source address",
			"validator": {"required": true, "ipNetMaskIptables": true},
			"defaultValue": "0.0.0.0/0",
			"tip": tip
		};
		c.addWidget(field);
		
		field = { 
			"type": "text",
			"item": item,
			"name": "dst",
			"text": "Destination",
			"descr": "Destination address",
			"validator": {"required": true, "ipNetMaskIptables": true},
			"defaultValue": "0.0.0.0/0",
			"tip": tip
		};
		c.addWidget(field);
		
		field = { 
			"type": "select",
			"item": item,
			"name": "proto",
			"text": "Protocol",
			"descr": "A protocol of the packet to check",
			"defaultValue": "all",
			"options": "all tcp udp icmp"
		};
		c.addWidget(field);
		
		tip = "An inclusive range can also be specified, using the format <b>port:port</b>. " +
				"If the first port is omitted, 0 is assumed; if the last is omitted, 65535 is assumed.";
		field = { 
			"type": "text",
			"item": item,
			"name": "sport",
			"text": "Source port",
			"descr": "Source port or port range",
			"validator": {"required": true, "ipPortRange": true},
			"defaultValue": "any",
			"tip": tip
		};
		c.addWidget(field);
		
		field = { 
			"type": "text",
			"item": item,
			"name": "dport",
			"id": "dport",
			"text": "Destination port",
			"descr": "Destination port or port range",
			"validator": {"required": true, "ipPortRange": true},
			"defaultValue": "any",
			"tip": tip
		};
		c.addWidget(field);
		
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
			"item": item,
			"name": "target",
			"id": "target",
			"text": "Action",
			"descr": "What to do with packet",
			"defaultValue": "ACCEPT",
			"options": targets,
			"onChange": onChangeTarget
		};
		c.addWidget(field);
		
		c.addSubmit({
			"complexValue": item,
			"submitName": "Add/Update",
			"extraButton": {
				"name": "Back",
				"func": options['showFunc']
			},
			"onSubmit": options['showFunc']
		});
		
		onChangeTarget();
	};
	
	/* show FILTER page */
	var showFilter = function() {
		var c, field;
		page.clearTab("filter");
		
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
		c.addTitle("Filter, FORWARD chain", 9);

		/* calls addFwRule with parameters for forward chain */
		var addForwardRule = function(item) {
			var options = {
				"tab": "filter",
				"helpPage": "filter",
				"helpSection": "filter_add",
				"itemTpl": "sys_fw_filter_forward_",
				"showFunc": showFilter,
				"chain": "FORWARD"
			};
			
			item && (options['item'] = item);
			addFwRule(options);
		};
		
		c.addTableHeader("Rule name|Src|Dst|Proto|Src port|Dst port|Action", addForwardRule);
		c.generateList("sys_fw_filter_forward_*", "name src dst proto sport dport target", addForwardRule, showFilter);
		
		/* INPUT chain */
		page.addBr("filter");
		c = page.addContainer("filter");
		c.setHelpSection("filter_input");
		c.addTitle("Filter, INPUT chain", 9);
		
		/* calls addFwRule with parameters for input chain */
		var addInputRule = function(item) {
			var options = {
				"tab": "filter",
				"helpPage": "filter",
				"helpSection": "filter_add",
				"itemTpl": "sys_fw_filter_input_",
				"showFunc": showFilter,
				"chain": "INPUT"
			};
			
			item && (options['item'] = item);
			addFwRule(options);
		};
		
		c.addTableHeader("Rule name|Src|Dst|Proto|Src port|Dst port|Action", addInputRule);
		c.generateList("sys_fw_filter_input_*", "name src dst proto sport dport target", addInputRule, showFilter);
		
		/* OUTPUT chain */
		page.addBr("filter");
		c = page.addContainer("filter");
		c.setHelpSection("filter_output");
		c.addTitle("Filter, OUTPUT chain", 9);
		
		/* calls addFwRule with parameters for output chain */
		var addOutputRule = function(item) {
			var options = {
				"tab": "filter",
				"helpPage": "filter",
				"helpSection": "filter_add",
				"itemTpl": "sys_fw_filter_output_",
				"showFunc": showFilter,
				"chain": "OUTPUT"
			};
			
			item && (options['item'] = item);
			addFwRule(options);
		};
		
		c.addTableHeader("Rule name|Src|Dst|Proto|Src port|Dst port|Action", addOutputRule);
		c.generateList("sys_fw_filter_output_*", "name src dst proto sport dport target", addOutputRule, showFilter);
	};
	
	/* FILTER tab */
	page.addTab({
		"id": "filter",
		"name": "Filter",
		"func": showFilter
	});
	
	/* show NAT page */
	var showNat = function() {
		var c, field;
		page.clearTab("nat");
		
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
		c.addTitle("NAT, PREROUTING chain", 9);

		/* calls addFwRule with parameters for prerouting chain */
		var addPreroutingRule = function(item) {
			var options = {
				"tab": "nat",
				"helpPage": "nat",
				"helpSection": "nat_add",
				"itemTpl": "sys_fw_nat_prerouting_",
				"showFunc": showNat,
				"chain": "PREROUTING"
			};
			
			item && (options['item'] = item);
			addFwRule(options);
		};
		
		c.addTableHeader("Rule name|Src|Dst|Proto|Src port|Dst port|Action", addPreroutingRule);
		c.generateList("sys_fw_nat_prerouting_*", "name src dst proto sport dport target", addPreroutingRule, showNat);
		
		/* POSTROUTING chain */
		page.addBr("nat");
		c = page.addContainer("nat");
		c.setHelpPage("nat");
		c.setHelpSection("nat_postrouting");
		c.addTitle("NAT, POSTROUTING chain", 9);
		
		/* calls addFwRule with parameters for postrouting chain */
		var addPostroutingRule = function(item) {
			var options = {
				"tab": "nat",
				"helpPage": "nat",
				"helpSection": "nat_add",
				"itemTpl": "sys_fw_nat_postrouting_",
				"showFunc": showNat,
				"chain": "POSTROUTING"
			};
			
			item && (options['item'] = item);
			addFwRule(options);
		};
		
		c.addTableHeader("Rule name|Src|Dst|Proto|Src port|Dst port|Action", addPostroutingRule);
		c.generateList("sys_fw_nat_postrouting_*", "name src dst proto sport dport target", addPostroutingRule, showNat);
	};
	
	/* NAT tab */
	page.addTab({
		"id": "nat",
		"name": "NAT",
		"func": showNat
	});
	
	page.generateTabs();
}
