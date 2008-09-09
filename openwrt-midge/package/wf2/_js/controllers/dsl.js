Controllers['dsl'] = function(iface, pcislot, pcidev) {
	var page = this.Page();
	page.setHelp("dsl");
	page.setSubsystem($.sprintf("dsl.%s.%s", pcislot, pcidev));

	page.addTab({
		"id": "status",
		"name": "Status",
		"func": function() {
			var c, field;
			c = page.addContainer("status");
			c.addTitle($.sprintf("%s (module %s) status", iface, config.getOEM("MR16H_MODNAME")));
			
			field = {
				"type": "html",
				"name": "link_state",
				"text": "Link state",
				"cmd": $.sprintf("/bin/cat %s/%s/state", config.getOEM("sg16_cfg_path"), iface)
			};
			c.addWidget(field);
		}
	});
	
	page.addTab({
		"id": "settings",
		"name": "Settings",
		"func": function() {
			var c, field, id;
			c = page.addContainer("settings");
			c.addTitle($.sprintf("%s (module %s) settings", iface, config.getOEM("MR16H_MODNAME")));
			
			field = { 
				"type": "select",
				"name": $.sprintf("sys_pcicfg_s%s_%s_mode", pcislot, pcidev),
				"id": "mode",
				"text": "Mode",
				"descr": "Select DSL mode",
				"options": {"master": "Master", "slave": "Slave"},
				"onChange": OnChangeSG16Code
			};
			c.addWidget(field);
			
			var name = $.sprintf("sys_pcicfg_s%s_%s_rate", pcislot, pcidev);
			var rate = config.get(name);
			field = { 
				"type": "select",
				"name": name,
				"id": "rate",
				"text": "Rate",
				"descr": "Select DSL line rate",
				"options": {"rate": rate}
			};
			c.addWidget(field);
			
			field = { 
				"type": "select",
				"name": $.sprintf("sys_pcicfg_s%s_%s_code", pcislot, pcidev),
				"id": "code",
				"text": "Coding",
				"descr": "Select DSL line coding",
				"options": {"tcpam32": "TCPAM32", "tcpam16": "TCPAM16", "tcpam8": "TCPAM8", "tcpam4": "TCPAM4"}
			};
			c.addWidget(field);
			
			field = { 
				"type": "select",
				"name": $.sprintf("sys_pcicfg_s%s_%s_cfg", pcislot, pcidev),
				"id": "cfg",
				"text": "Config",
				"descr": "Select DSL configuration mode",
				"options": {"local": "local", "preact": "preact"}
			};
			c.addWidget(field);
			
			field = { 
				"type": "select",
				"name": $.sprintf("sys_pcicfg_s%s_%s_annex", pcislot, pcidev),
				"id": "annex",
				"text": "Annex",
				"descr": "Select DSL Annex",
				"options": {"A": "Annex A", "B": "Annex B", "F": "Annex F"}
			};
			c.addWidget(field);
			
			field = { 
				"type": "select",
				"name": $.sprintf("sys_pcicfg_s%s_%s_crc", pcislot, pcidev),
				"text": "CRC",
				"descr": "Select DSL CRC length",
				"options": {"crc32": "CRC32", "crc16": "CRC16"}
			};
			c.addWidget(field);
			
			field = { 
				"type": "select",
				"name": $.sprintf("sys_pcicfg_s%s_%s_fill", pcislot, pcidev),
				"text": "Fill",
				"descr": "Select DSL fill byte value",
				"options": {"fill_ff": "FF", "fill_7e": "7E"}
			};
			c.addWidget(field);
			
			field = { 
				"type": "select",
				"name": $.sprintf("sys_pcicfg_s%s_%s_inv", pcislot, pcidev),
				"text": "Inversion",
				"descr": "Select DSL inversion mode",
				"options": {"normal": "off", "invert": "on"}
			};
			c.addWidget(field);
			
			c.addSubmit();
		}
	});
	
	page.generateTabs();
}