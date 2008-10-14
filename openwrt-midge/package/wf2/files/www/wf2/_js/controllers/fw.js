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
	 * options['showFunc'] — func to call on submit and on click on Back buton (to show list of items).
	 */
	var addFwRule = function(options) {
		var c, field, item;
		page.clearTab(options['tab']);
		c = page.addContainer(options['tab']);
		options['helpPage'] && c.setHelpPage(options['helpPage']);
		options['helpSection'] && c.setHelpSection(options['helpSection']);

		/* add new item */
		if (!options['item']) {
			c.addTitle("Add rule");
			values = config.getParsed(options['itemTpl'] + "*");
			item = options['itemTpl'] + $.len(values);
		/* edit selected item */
		} else {
			c.addTitle("Edit rule");
			item = options['item'];
		}

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
			"text": "Destination port",
			"descr": "Destination port or port range",
			"validator": {"required": true, "ipPortRange": true},
			"defaultValue": "any",
			"tip": tip
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
	};
	
	/* show filter page */
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
		
		/* forward chain */
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
				"showFunc": showFilter
			};
			
			item && (options['item'] = item);
			addFwRule(options);
		};
		
		c.addTableHeader("Rule name|Src|Dst|Proto|Src port|Dst port|Action", addForwardRule);
		c.generateList("sys_fw_filter_forward_*", "name src dst proto sport dport target", addForwardRule, showFilter);
		
		/* input chain */
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
				"showFunc": showFilter
			};
			
			item && (options['item'] = item);
			addFwRule(options);
		};
		
		c.addTableHeader("Rule name|Src|Dst|Proto|Src port|Dst port|Action", addInputRule);
		c.generateList("sys_fw_filter_input_*", "name src dst proto sport dport target", addInputRule, showFilter);
	};
	
	/* filter tab */
	page.addTab({
		"id": "filter",
		"name": "Filter",
		"func": showFilter
	});
	
	page.generateTabs();
}
