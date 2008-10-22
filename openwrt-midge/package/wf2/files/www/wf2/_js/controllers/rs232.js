Controllers['rs232'] = function(node, pcislot, pcidev) {
	var page = this.Page();
	page.setSubsystem($.sprintf("rs232.%s.%s", pcislot, pcidev));
	
	/* settings tab */
	page.addTab({
		"id": "settings",
		"name": "RS232 settings",
		"func": function() {
			var c, field;
			c = page.addContainer("settings");
			c.addTitle($.sprintf("%s (module %s%s%s, slot %s) settings", node,
				config.getOEM("MR17S_MODNAME"), config.getOEM("OEM_IFPFX"),
				config.get($.sprintf("sys_pcitbl_s%s_ifnum", pcislot)),	pcislot - 2));
		
			field = { 
				"type": "select",
				"name": $.sprintf("sys_pcicfg_s%s_%s_baudrate", pcislot, pcidev),
				"text": "Baud rate",
				"options": [230400,115200,57600,38400,28800,19200,14400,9600,7200,4800,3600,2400,1800,1200,600,300]
			};
			c.addWidget(field);
			
			field = { 
				"type": "select",
				"name": $.sprintf("sys_pcicfg_s%s_%s_cs", pcislot, pcidev),
				"text": "Character size (bits)",
				"options": {"cs7": "7", "cs8": "8"}
			};
			c.addWidget(field);
			
			field = { 
				"type": "select",
				"name": $.sprintf("sys_pcicfg_s%s_%s_stopb", pcislot, pcidev),
				"text": "Stop bits",
				"options": {"-cstopb": "1", "cstopb": "2"}
			};
			c.addWidget(field);
			
			field = { 
				"type": "select",
				"name": $.sprintf("sys_pcicfg_s%s_%s_parity", pcislot, pcidev),
				"text": "Parity",
				"options": "none even odd"
			};
			c.addWidget(field);
			
			field = { 
				"type": "select",
				"name": $.sprintf("sys_pcicfg_s%s_%s_fctrl", pcislot, pcidev),
				"text": "Hardware Flow control",
				"options": {"0": "off", "1": "on"}
			};
			c.addWidget(field);
			
			field = { 
				"type": "select",
				"name": $.sprintf("sys_pcicfg_s%s_%s_sigfwd", pcislot, pcidev),
				"text": "Forward Modem Signals",
				"options": {"0": "off", "1": "on"}
			};
			c.addWidget(field);
			
			c.addSubmit();
		}
	});
	
	page.generateTabs();
}
