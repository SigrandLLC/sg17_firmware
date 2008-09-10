Controllers['dsl'] = function(iface, pcislot, pcidev) {
	var page = this.Page();
	page.setHelp("dsl");
	page.setSubsystem($.sprintf("dsl.%s.%s", pcislot, pcidev));

	/* fill select with rates */
	var rateList = function(select, first, last, step, cur) {
		var rates = "";
		for (var i = first; i <= last; i += step) {
			rates += i + " ";
		}
		rates = $.trim(rates);
		$(select).setOptionsForSelect(rates, cur);
	};

	/* show status for 16 series */
	var sg16Status = function() {
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
	};

	/* show settings for 16 series */
	var sg16Settings = function() {
		var c, field, id;
		c = page.addContainer("settings");
		c.addTitle($.sprintf("%s (module %s) settings", iface, config.getOEM("MR16H_MODNAME")));
		
		/* available TCPAM values */
		var TCPAM = {
			"tcpam32": "TCPAM32",
			"tcpam16": "TCPAM16",
			"tcpam8": "TCPAM8",
			"tcpam4": "TCPAM4"
		};
		
		/* updating parameters */
		var onChangeSG16Code = function() {
			var cfg = $("#cfg").val();
			var annex = $("#annex").val();
			var mode = $("#mode").val();
			var code = $("#code").val();
			var rate = $("#rate").val();
			
			if (cfg == "preact" && annex == "F") {
				if (mode == "slave") {
					$("#code").setOptionsForSelect({"tcpam32": TCPAM["tcpam32"]});
					$("#code").attr("readonly", true);
					
					$("#rate").setOptionsForSelect("automatic");
					$("#rate").attr("readonly", true);
				} else {					
					$("#code").removeAttr("readonly");
					$("#rate").removeAttr("readonly");
					
					$("#code").setOptionsForSelect({
						"tcpam16": TCPAM["tcpam16"],
						"tcpam32": TCPAM["tcpam32"]
					}, code);
					
					/* update varibale's value */
					code = $("#code").val();
					
					if (code == "tcpam16") {
						rateList("#rate", 192, 2304, 64, rate);
					} else {
						rateList("#rate", 192, 5696, 64, rate);
					}
				}
			} else if (cfg == "preact") {
				$("#code").setOptionsForSelect({"tcpam16": TCPAM["tcpam16"]});
				$("#code").attr("readonly", true);
				
				if (mode == "slave") {
					$("#rate").setOptionsForSelect("automatic");
					$("#rate").attr("readonly", true);
				} else {
					$("#rate").removeAttr("readonly");
					rateList("#rate", 192, 2304, 64, rate);
				}
			} else {
				$("#code").removeAttr("readonly");
				$("#rate").removeAttr("readonly");
				
				$("#code").setOptionsForSelect(TCPAM, code);
				
				/* update varibale's value */
				code = $("#code").val();
				
				switch (code) {
					case "tcpam4":
						rateList("#rate", 64, 704, 64, rate);
						break;
					case "tcpam8":
						rateList("#rate", 192, 1216, 64, rate);
						break;
					case "tcpam16":
						rateList("#rate", 192, 3840, 64, rate);
						break;
					case "tcpam32":
						rateList("#rate", 256, 6016, 64, rate);
						break;
				}
			}
		};
		
		field = { 
			"type": "select",
			"name": $.sprintf("sys_pcicfg_s%s_%s_mode", pcislot, pcidev),
			"id": "mode",
			"text": "Mode",
			"descr": "Select DSL mode",
			"options": {"master": "Master", "slave": "Slave"},
			"onChange": onChangeSG16Code
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
			"options": rate
		};
		c.addWidget(field);
		
		field = { 
			"type": "select",
			"name": $.sprintf("sys_pcicfg_s%s_%s_code", pcislot, pcidev),
			"id": "code",
			"text": "Coding",
			"descr": "Select DSL line coding",
			"options": TCPAM,
			"onChange": onChangeSG16Code
		};
		c.addWidget(field);
		
		field = { 
			"type": "select",
			"name": $.sprintf("sys_pcicfg_s%s_%s_cfg", pcislot, pcidev),
			"id": "cfg",
			"text": "Config",
			"descr": "Select DSL configuration mode",
			"options": {"local": "local", "preact": "preact"},
			"onChange": onChangeSG16Code
		};
		c.addWidget(field);
		
		field = { 
			"type": "select",
			"name": $.sprintf("sys_pcicfg_s%s_%s_annex", pcislot, pcidev),
			"id": "annex",
			"text": "Annex",
			"descr": "Select DSL Annex",
			"options": {"A": "Annex A", "B": "Annex B", "F": "Annex F"},
			"onChange": onChangeSG16Code
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
		
		onChangeSG16Code();
	};
	
	/* get driver name */
	var type = config.get($.sprintf("sys_pcitbl_s%s_iftype", pcislot));
	
	page.addTab({
		"id": "status",
		"name": "Status",
		"func": function() {
			if (type == config.getOEM("MR16H_DRVNAME")) {
				sg16Status();
			}
		}
	});
	
	page.addTab({
		"id": "settings",
		"name": "Settings",
		"func": function() {
			if (type == config.getOEM("MR16H_DRVNAME")) {
				sg16Settings();
			}
		}
	});
	
	page.generateTabs();
}