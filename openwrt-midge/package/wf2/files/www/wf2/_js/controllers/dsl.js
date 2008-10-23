Controllers['dsl'] = function(iface, pcislot, pcidev) {
	var page = this.Page();
	page.setHelpPage("dsl");
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

	/* fill select with fixed rates */
	var fixedRateList = function(select, cur, rates) {
		var ratesList = new Object();
		$.each(rates, function(num, rate){
			ratesList[rate] = "" + rate;
		});
		ratesList["-1"] = "other";
		$(select).setOptionsForSelect(ratesList, cur);
	};

	/* title of sg16 */
	var getSg16Title = function() {
		return $.sprintf("%s (module %s%s%s) ", iface,
			config.getOEM("MR16H_MODNAME"), config.getOEM("OEM_IFPFX"),
			config.get($.sprintf("sys_pcitbl_s%s_ifnum", pcislot)));
	};

	/* show status for 16 series */
	var sg16Status = function() {
		var c, field;
		c = page.addContainer("status");
		c.addTitle(getSg16Title() + "status");
		
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
		c.addTitle(getSg16Title() + "settings");
		
		/* available TCPAM values */
		var TCPAM = {
			"tcpam32": "TCPAM32",
			"tcpam16": "TCPAM16",
			"tcpam8": "TCPAM8",
			"tcpam4": "TCPAM4"
		};
		
		/* updates parameters */
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
		
		/* add parameters */
		
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
	
	/* title of sg17 */
	var getSg17Title = function(pwr) {
		var sfx;
		var ver = config.getCachedOutput($.sprintf("/bin/cat %s/%s/sg17_private/chipver",
			config.getOEM("sg17_cfg_path"), iface));
		
		if (ver == "v1") {
			sfx = config.getOEM("MR17H_V1SFX");
		} else if (ver == "v2") {
			sfx = config.getOEM("MR17H_V2SFX");
		}
		
		if (pwr == 1) {
			sfx += config.getOEM("MR17H_PWRSFX");
		}
		
		return $.sprintf("%s (module %s%s%s%s) ", iface, config.getOEM("MR17H_MODNAME"),
			config.getOEM("OEM_IFPFX"), config.get($.sprintf("sys_pcitbl_s%s_ifnum", pcislot)),
			sfx);
	};
	
	/* show status for 17 series */
	var sg17Status = function() {
		var c, field;
		c = page.addContainer("status");
		
		var confPath = $.sprintf("%s/%s/sg17_private", config.getOEM("sg17_cfg_path"), iface);
		
		/* power status */
		var pwrPresence = config.getCachedOutput($.sprintf("/bin/cat %s/pwr_source", confPath));
		
		c.addTitle(getSg17Title(pwrPresence) + "status");
		
		/* get link state */
		var link = cmdExecute($.sprintf("/bin/cat %s/link_state", confPath), {"sync": true});
		
		field = {
			"type": "html",
			"name": "link_state",
			"text": "Link state",
			"str": link == 1 ? "online" : "offline"
		};
		c.addWidget(field);
		
		if (pwrPresence == 1) {
			field = {
				"type": "html",
				"name": "pwrUnb",
				"text": "Power balance",
				"cmd": $.sprintf("/bin/cat %s/pwrunb", confPath),
				"dataFilter": function(data) {
					return data == 0 ? "balanced" : "unbalanced"
				}
			};
			c.addWidget(field);
			
			field = {
				"type": "html",
				"name": "pwrOvl",
				"text": "Power overload",
				"cmd": $.sprintf("/bin/cat %s/pwrovl", confPath),
				"dataFilter": function(data) {
					return data == 0 ? "no overload" : "overload"
				}
			};
			c.addWidget(field);
		}
		
		/* online */
		if (link == 1) {
			field = {
				"type": "html",
				"name": "actualRate",
				"text": "Actual rate",
				"cmd": $.sprintf("/bin/cat %s/rate", confPath)
			};
			c.addWidget(field);
			
			field = {
				"type": "html",
				"name": "actualLineCode",
				"text": "Actual line code",
				"cmd": $.sprintf("/bin/cat %s/tcpam", confPath)
			};
			c.addWidget(field);
			
			field = {
				"type": "html",
				"name": "actualClockMode",
				"text": "Actual clock mode",
				"cmd": $.sprintf("/bin/cat %s/clkmode", confPath)
			};
			c.addWidget(field);
			
			/* statistics */
			var stat = cmdExecute($.sprintf("/bin/cat %s/statistics_row", confPath), {"sync": true}).split(" ");
			field = {
				"type": "html",
				"name": "snrMargin",
				"text": "SNR margin",
				"descr": "Signal/Noise ratio margin",
				"str": $.sprintf("%s dB", stat[0])
			};
			c.addWidget(field);
			
			field = {
				"type": "html",
				"name": "loopAttn",
				"text": "Loop attenuation",
				"str": $.sprintf("%s dB", stat[1])
			};
			c.addWidget(field);
		}
		
		/* PBO */
		var pboMode = cmdExecute($.sprintf("/bin/cat %s/pbo_mode", confPath), {"sync": true});
		if (pboMode == "Forced") {
			field = {
				"type": "html",
				"name": "pboVal",
				"text": "PBO values",
				"descr": "Power backoff values",
				"cmd": $.sprintf("/bin/cat %s/pbo_val", confPath),
				"dataFilter": function(data) {
					return data + " dB"
				}
			};
			c.addWidget(field);
		}
	};
	
	/* show settings for 17 series */
	var sg17Settings = function() {
		var c, field, id;
		c = page.addContainer("settings");
		var confPath = $.sprintf("%s/%s/sg17_private", config.getOEM("sg17_cfg_path"), iface);
		
		/* power status */
		var pwrPresence = config.getCachedOutput($.sprintf("/bin/cat %s/pwr_source", confPath));
		
		c.addTitle(getSg17Title(pwrPresence) + "settings");
		
		/* available TCPAM values */
		var TCPAM = {
			"tcpam128": "TCPAM128",
			"tcpam64": "TCPAM64",
			"tcpam32": "TCPAM32",
			"tcpam16": "TCPAM16",
			"tcpam8": "TCPAM8"
		};
		
		/* list of rates */
		var rateList8 = new Array(192,256,320,384,512,640,768,1024,1280,1536,1792,2048,2304,2560,3072,3584,3840);
		var rateList16 = new Array(192,256,320,384,512,640,768,1024,1280,1536,1792,2048,2304,2560,3072,3584,3840);
		var rateList32_v1 = new Array(768,1024,1280,1536,1792,2048,2304,2560,3072,3584,3840,4096,4608,5120,5696);
		var rateList32_v2 = new Array(768,1024,1280,1536,1792,2048,2304,2560,3072,3584,3840,4096,4608,5120,5696,6144,7168,8192,9216,10176);
		var rateList64 = new Array(768,1024,1280,1536,1792,2048,2304,2560,3072,3584,3840,4096,4608,5120,5696,6144,7168,8192,9216,10240,11520,12736);
		var rateList128 = new Array(768,1024,1280,1536,1792,2048,2304,2560,3072,3584,3840,4096,4608,5120,5696,6144,7168,8192,9216,10240,11520,12800,14080);
		
		/* name and id of mrate field (used for manual speed setting, can be updated by subsystem) */
		var mrateId = $.sprintf("sys_pcicfg_s%s_%s_mrate", pcislot, pcidev);
		
		/* updates parameters */
		var onChangeSG17Code = function() {
			if ($("#ctrl").val() == "manual") {
				addManualWidgets();
			} else {
				removeManualWidgets();
			}
				
			var mode = $("#mode").val();
			var code = $("#code").val();
			var rate = $("#rate").val();
			var annex = $("#annex").val();
			var clkmode = $("#clkmode").val();
			
			if (mode == "slave") {
				$("#rate").setOptionsForSelect("automatic");
				$("#rate").attr("readonly", true);
				
				$("#code").setOptionsForSelect("automatic");
				$("#code").attr("readonly", true);
				
				$("#clkmode").setOptionsForSelect("automatic");
				$("#clkmode").attr("readonly", true);
				
				$("#annex").setOptionsForSelect("automatic");
				$("#annex").attr("readonly", true);
				
				$("#pbomode").attr("readonly", true);
				
				$("#" + mrateId).remove();
				$("#pboval").remove();
			} else {
				$("#code").removeAttr("readonly");
				var chipVer = config.getCachedOutput($.sprintf("/bin/cat %s/chipver", confPath));
				if (chipVer == "v1") {
					$("#code").setOptionsForSelect({
						"tcpam8": TCPAM["tcpam8"],
						"tcpam16": TCPAM["tcpam16"],
						"tcpam32": TCPAM["tcpam32"]
					}, code);
				} else {
					$("#code").setOptionsForSelect(TCPAM, code);
				}
				
				/* update varibale's value */
				code = $("#code").val();
				
				$("#rate").removeAttr("readonly");
				if (chipVer == "v1") {
					switch (code) {
						case "tcpam8":
							fixedRateList("#rate", rate, rateList8);
							break;
						case "tcpam16":
							fixedRateList("#rate", rate, rateList16);
							break;
						case "tcpam32":
							fixedRateList("#rate", rate, rateList32_v1);
							break;
					}
				} else {
					switch (code) {
						case "tcpam8":
							fixedRateList("#rate", rate, rateList8);
							break;
						case "tcpam16":
							fixedRateList("#rate", rate, rateList16);
							break;
						case "tcpam32":
							fixedRateList("#rate", rate, rateList32_v2);
							break;
						case "tcpam64":
							fixedRateList("#rate", rate, rateList64);
							break;
						case "tcpam128":
							fixedRateList("#rate", rate, rateList128);
							break;
					}
				}
				
				/* if rate is "other", add text widget to enter rate value */
				if (rate == -1 && $("#" + mrateId).length == 0) {
					field = { 
						"type": "text",
						"name": mrateId,
						"id": mrateId,
						"validator": {"required": true, "min": 0}
					};
					c.addSubWidget(field, "#rate");
				/* otherwise, remove it */
				} else if (rate > 0) {
					$("#" + mrateId).remove();
				}
				
				$("#clkmode").removeAttr("readonly");
				$("#clkmode").setOptionsForSelect("plesio plesio-ref sync", clkmode);
				
				$("#pbomode").removeAttr("readonly");
				/* If PBO is active, add text field for it's value */
				if ($("#pbomode").attr("checked") == true && $("#pboval").length == 0) {
					field = { 
						"type": "text",
						"name": $.sprintf("sys_pcicfg_s%s_%s_pboval", pcislot, pcidev),
						"id": "pboval",
						"validator": {"required": true, "pbo": true}
					};
					c.addSubWidget(field, "#pbomode");
				} else if ($("#pbomode").attr("checked") == false) {
					$("#pboval").remove();
				}
				
				$("#annex").removeAttr("readonly");
				$("#annex").setOptionsForSelect({"A": "Annex A", "B": "Annex B"}, annex);
			}
		}
		
		/* IDs of manual widgets */
		var manualWidgetsIDs = new Array();
		
		/* add, if not exist, widgets for manual mode */
		var addManualWidgets = function() {
			if (manualWidgetsIDs.length != 0) return;
			
			/* widget after which insert this element */
			var parentWidget = $("#ctrl").parents("tr");
						
			if (pwrPresence == 1) {
				/* pwron */
				manualWidgetsIDs.push("pwron");
				field = { 
					"type": "select",
					"name": $.sprintf("sys_pcicfg_s%s_%s_pwron", pcislot, pcidev),
					"id": "pwron",
					"text": "Power",
					"descr": "Select DSL Power feeding mode",
					"options": {"pwroff": "off", "pwron": "on"}
				};
				c.addWidget(field, parentWidget);
			}
			
			/* pbo */
			manualWidgetsIDs.push("pbomode");
			field = { 
				"type": "checkbox",
				"name": $.sprintf("sys_pcicfg_s%s_%s_pbomode", pcislot, pcidev),
				"id": "pbomode",
				"text": "PBO forced",
				"descr": "Example: 21:13:15, STU-C-SRU1=21,SRU1-SRU2=13,...",
				"onChange": onChangeSG17Code
			};
			c.addWidget(field, parentWidget);
			
			/* annex */
			manualWidgetsIDs.push("annex");
			field = { 
				"type": "select",
				"name": $.sprintf("sys_pcicfg_s%s_%s_annex", pcislot, pcidev),
				"id": "annex",
				"text": "Annex",
				"descr": "Select DSL Annex",
				"options": {"A": "Annex A", "B": "Annex B"}
			};
			c.addWidget(field, parentWidget);
			
			/* code */
			manualWidgetsIDs.push("code");
			var name = $.sprintf("sys_pcicfg_s%s_%s_code", pcislot, pcidev);
			var value = config.get(name).length == 0 ? "tcpam32" : config.get(name);
			field = { 
				"type": "select",
				"name": name,
				"id": "code",
				"text": "Coding",
				"descr": "Select DSL line coding",
				"options": value,
				"onChange": onChangeSG17Code
			};
			c.addWidget(field, parentWidget);
			
			/* rate */
			manualWidgetsIDs.push("rate");
			var name = $.sprintf("sys_pcicfg_s%s_%s_rate", pcislot, pcidev);
			var value = config.get(name) == "-1" ? "other" : config.get(name);
			var rateOptions = new Object();
			rateOptions[config.get(name)] = value;
			field = { 
				"type": "select",
				"name": name,
				"id": "rate",
				"text": "Rate",
				"descr": "Select DSL line rate",
				"options": rateOptions,
				"onChange": onChangeSG17Code
			};
			c.addWidget(field, parentWidget);
			
			/* mode */
			manualWidgetsIDs.push("mode");
			field = { 
				"type": "select",
				"name": $.sprintf("sys_pcicfg_s%s_%s_mode", pcislot, pcidev),
				"id": "mode",
				"text": "Mode",
				"descr": "Select DSL mode",
				"options": {"master": "Master", "slave": "Slave"},
				"onChange": onChangeSG17Code
			};
			c.addWidget(field, parentWidget);
		};
		
		/* remove, if exist, widgets for manual mode */
		var removeManualWidgets = function() {
			if (manualWidgetsIDs.length != 0) {
				$.each(manualWidgetsIDs, function(num, value) {
					$("#" + value).parents("tr").remove();
				});
				manualWidgetsIDs = new Array();
			}
		};
		
		/* add parameters */
		
		field = { 
			"type": "select",
			"name": $.sprintf("sys_pcicfg_s%s_%s_ctrl", pcislot, pcidev),
			"id": "ctrl",
			"text": "Control type",
			"descr": "Control type (manual or by EOC daemon)",
			"options": {"manual": "Manual", "eocd": "EOCd"},
			"onChange": onChangeSG17Code
		};
		c.addWidget(field);
		
		field = { 
			"type": "select",
			"name": $.sprintf("sys_pcicfg_s%s_%s_clkmode", pcislot, pcidev),
			"id": "clkmode",
			"text": "Clock mode",
			"descr": "Select DSL clock mode",
			"options": "plesio plesio-ref sync"
		};
		c.addWidget(field);
		
		field = { 
			"type": "select",
			"name": $.sprintf("sys_pcicfg_s%s_%s_advlink", pcislot, pcidev),
			"text": "AdvLink",
			"descr": "Select DSL Advanced link detection",
			"options": "off on"
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
		
		c.addSubmit({
			"onSuccess": function() {
				updateFields(mrateId, true);
			}
		});
		
		onChangeSG17Code();
	}
	
	/* get driver name */
	var type = config.get($.sprintf("sys_pcitbl_s%s_iftype", pcislot));
	
	page.addTab({
		"id": "status",
		"name": "Status",
		"func": function() {
			if (type == config.getOEM("MR16H_DRVNAME")) {
				sg16Status();
			} else if (type == config.getOEM("MR17H_DRVNAME")) {
				sg17Status();
			}
		}
	});
	
	page.addTab({
		"id": "settings",
		"name": "Settings",
		"func": function() {
			if (type == config.getOEM("MR16H_DRVNAME")) {
				sg16Settings();
			} else if (type == config.getOEM("MR17H_DRVNAME")) {
				sg17Settings();
			}
		}
	});
	
	page.generateTabs();
}
