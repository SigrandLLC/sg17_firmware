Controllers['dsl'] = function(iface, pcislot, pcidev) {
	var eocInfoCmd = "/sbin/eoc-info";
	
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
		var link = config.cmdExecute({
			"cmd": $.sprintf("/bin/cat %s/link_state", confPath),
			"async": false
		});
		
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
			
			field = {
				"type": "html",
				"name": "snrMargin",
				"text": "SNR margin",
				"descr": "Signal/Noise ratio margin",
				"cmd": $.sprintf("/bin/cat %s/statistics_row", confPath),
				"dataFilter": function(data) {
					return data.split(" ")[0]
				}
			};
			c.addWidget(field);
			
			field = {
				"type": "html",
				"name": "loopAttn",
				"text": "Loop attenuation",
				"cmd": $.sprintf("/bin/cat %s/statistics_row", confPath),
				"dataFilter": function(data) {
					return data.split(" ")[1]
				}
			};
			c.addWidget(field);
		}
		
		/* PBO */
		var pboMode = config.cmdExecute({
			"cmd": $.sprintf("/bin/cat %s/pbo_mode", confPath),
			"async": false
		});
		
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
					c.addSubWidget(field, {"type": "insertAfter", "anchor": "#rate"});
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
					c.addSubWidget(field, {"type": "insertAfter", "anchor": "#pbomode"});
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
				c.addWidget(field, {"type": "insertAfter", "anchor": parentWidget});
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
			c.addWidget(field, {"type": "insertAfter", "anchor": parentWidget});
			
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
			c.addWidget(field, {"type": "insertAfter", "anchor": parentWidget});
			
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
			c.addWidget(field, {"type": "insertAfter", "anchor": parentWidget});
			
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
			c.addWidget(field, {"type": "insertAfter", "anchor": parentWidget});
			
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
			c.addWidget(field, {"type": "insertAfter", "anchor": parentWidget});
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
			"type": "checkbox",
			"name": $.sprintf("sys_mux_%s_mxen", iface),
			"text": "Enable multiplexing",
			"descr": "Enable multiplexing on this interface",
			"tip": "This option is equivalent to MXEN on a multiplexing page."
		};
		c.addWidget(field);
		
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
	
	/* show main statistics page with unit selection */
	var showStatistics = function(eocInfo) {
		var c = page.addContainer("statistics");
		c.setHelpPage("eoc");
		
		/* statistics is available only for master */
		if (eocInfo.type == "slave") {
			c.addTitle("Statistics is available only for MASTER interfaces.");
			return;
		}
		
		c.addTitle("Select channel unit");
		
		var onUnitChange = function() {
			/* save selected unit in cookie */
			$.cookie("unit", $("#unit").val());
			
			/* remove br and all forms, except first */
			$("form:eq(1) ~ br, form:eq(0) ~ form").remove();
			
			if ($("#unit").val() == "general") {
				config.cmdExecute({
					"cmd": $.sprintf("%s -ri%s", eocInfoCmd, iface),
					"callback": showGeneral,
					"filter": config.parseData
				});
			} else {
				config.cmdExecute({
					"cmd": $.sprintf("%s -ri%s", eocInfoCmd, iface),
					"callback": function(eocInfo) {
						showStatUnit(eocInfo, $("#unit").val());
					},
					"filter": config.parseData
				});
			}
		};
		
		/* create hash with channel units */
		var units = {"general": "General"};
		if (eocInfo.unit_num > 0) {
			units['stu-c'] = "STU-C";
			if (eocInfo.link == "1") {
				units['stu-r'] = "STU-R";
				for (var i = 1; i <= eocInfo.reg_num; i++) units['sru' + i] = "SRU" + i;
			} else
				/* do not show statistics for last regenerator if EOC is offline */
				for (var i = 1; i < eocInfo.reg_num; i++) units['sru' + i] = "SRU" + i;
		}
		
		/* value of this widget is saved in cookie, because we not need it in KDB */
		var field = { 
			"type": "select",
			"name": "unit",
			"cookie": true,
			"text": "Channel unit",
			"descr": "Select channel unit to view statistics.",
			"options": units,
			"onChange": onUnitChange
		};
		c.addWidget(field);
		
		page.addBr("statistics");
		
		onUnitChange();
	};
	
	/* show general interface statistics */
	var showGeneral = function(eocInfo) {
		var c = page.addContainer("statistics");
		c.setHelpPage("eoc");
		c.addTitle(iface + " state");
	
		var field = {
			"type": "html",
			"name": "status",
			"text": "Channel link",
			"descr": "STU-C connected to STU-R.",
			"str": eocInfo.link == "1" ? "online": "offline"
		};
		c.addWidget(field);
		
		var field = {
			"type": "html",
			"name": "regs",
			"text": "Regenerators",
			"descr": "Regenerators in channel (actual number).",
			"str": eocInfo.reg_num
		};
		c.addWidget(field);
		
		var field = {
			"type": "html",
			"name": "pairs",
			"text": "Wire pairs",
			"descr": "Number of wire pairs in channel.",
			"str": eocInfo.loop_num
		};
		c.addWidget(field);
		
		var field = {
			"type": "html",
			"name": "rate",
			"text": "Rate",
			"descr": "Channel rate value.",
			"str": eocInfo.rate
		};
		c.addWidget(field);
		
		var field = {
			"type": "html",
			"name": "annex",
			"text": "Annex",
			"descr": "Channel annex value.",
			"str": eocInfo.annex
		};
		c.addWidget(field);
		
		var field = {
			"type": "html",
			"name": "encoding",
			"text": "Encoding",
			"descr": "Channel tcpam value.",
			"str": eocInfo.tcpam
		};
		c.addWidget(field);
	};
	
	/* show unit statistics */
	var showStatUnit = function(eocInfo, unit) {
		/* Show current state of interface */
		var showState = function(eocInfo, side, loop, c, row) {
			var field;
			
			field = {
				"type": "html",
				"name": "state_side_" + side + loop,
				"str": side
			};
			c.addTableWidget(field, row);
			
			field = {
				"type": "html",
				"name": "state_loop_" + side + loop,
				"str": $.sprintf("Pair%s", loop + 1)
			};
			c.addTableWidget(field, row);
			
			field = {
				"type": "html",
				"name": "snr_" + side + loop,
				"str": eocInfo.snr
			};
			c.addTableWidget(field, row);
			
			field = {
				"type": "html",
				"name": "lattn_" + side + loop,
				"str": eocInfo.lattn
			};
			c.addTableWidget(field, row);
			
			field = {
				"type": "html",
				"name": "es_" + side + loop,
				"str": eocInfo.es
			};
			c.addTableWidget(field, row);
			
			field = {
				"type": "html",
				"name": "ses_" + side + loop,
				"str": eocInfo.ses
			};
			c.addTableWidget(field, row);
			
			field = {
				"type": "html",
				"name": "crc_" + side + loop,
				"str": eocInfo.crc
			};
			c.addTableWidget(field, row);
			
			field = {
				"type": "html",
				"name": "losws_" + side + loop,
				"str": eocInfo.losws
			};
			c.addTableWidget(field, row);
			
			field = {
				"type": "html",
				"name": "uas_" + side + loop,
				"str": eocInfo.uas
			};
			c.addTableWidget(field, row);
		};
		
		/* show relative counters for interface */
		var showRelativeCounters = function(eocInfo, side, loop, c, row, reload) {
			var field;
			
			if (reload) row.empty();
			
			field = {
				"type": "html",
				"name": "tdate_" + side + loop,
				"str": eocInfo.tdate
			};
			c.addTableWidget(field, row);
			
			field = {
				"type": "html",
				"name": "ttime_" + side + loop,
				"str": eocInfo.ttime
			};
			c.addTableWidget(field, row);
			
			field = {
				"type": "html",
				"name": "relative_side_" + side + loop,
				"str": side
			};
			c.addTableWidget(field, row);
			
			field = {
				"type": "html",
				"name": "relative_loop_" + side + loop,
				"str": $.sprintf("Pair%s", loop + 1)
			};
			c.addTableWidget(field, row);
			
			field = {
				"type": "html",
				"name": "tes_" + side + loop,
				"str": eocInfo.tes
			};
			c.addTableWidget(field, row);
			
			field = {
				"type": "html",
				"name": "tses_" + side + loop,
				"str": eocInfo.tses
			};
			c.addTableWidget(field, row);
			
			field = {
				"type": "html",
				"name": "tcrc_" + side + loop,
				"str": eocInfo.tcrc
			};
			c.addTableWidget(field, row);
			
			field = {
				"type": "html",
				"name": "tlosws_" + side + loop,
				"str": eocInfo.tlosws
			};
			c.addTableWidget(field, row);
			
			field = {
				"type": "html",
				"name": "tuas_" + side + loop,
				"str": eocInfo.tuas
			};
			c.addTableWidget(field, row);
			
			field = {
				"type": "button",
				"name": "reset_" + side + loop,
				"text": "reset",
				"func": function() {
					config.cmdExecute({
						"cmd": $.sprintf("%s -r -i%s -u%s -e%s -l%s -v", eocInfoCmd, iface, unit, side, loop),
						"callback": function() {
							config.cmdExecute({
								"cmd": $.sprintf("%s -r -i%s -u%s -e%s -l%s", eocInfoCmd, iface, unit, side, loop),
								"callback": function(eocInfo) {
									showRelativeCounters(eocInfo, side, loop, cRelative, row, true);
								},
								"filter": config.parseData
							});
						}
					});
				}
			};
			c.addTableWidget(field, row);
		};
		
		/* show current 15 minutes and 1 day intervals */
		var showCurrentIntervals = function(eocInfo, side, loop, c, row1, row2) {
			var field;
			
			var showInterval = function(name, varPrefix, row) {
				field = {
					"type": "html",
					"name": "name_" + side + loop + varPrefix,
					"str": name
				};
				c.addTableWidget(field, row);
				
				field = {
					"type": "html",
					"name": "side_" + side + loop + varPrefix,
					"str": side
				};
				c.addTableWidget(field, row);
				
				field = {
					"type": "html",
					"name": "loop_" + side + loop + varPrefix,
					"str": $.sprintf("Pair%s", loop + 1)
				};
				c.addTableWidget(field, row);
				
				field = {
					"type": "html",
					"name": "es_" + side + loop + varPrefix,
					"str": eocInfo[varPrefix + 'es']
				};
				c.addTableWidget(field, row);
				
				field = {
					"type": "html",
					"name": "ses_" + side + loop + varPrefix,
					"str": eocInfo[varPrefix + 'ses']
				};
				c.addTableWidget(field, row);
				
				field = {
					"type": "html",
					"name": "crc_" + side + loop + varPrefix,
					"str": eocInfo[varPrefix + 'crc']
				};
				c.addTableWidget(field, row);
				
				field = {
					"type": "html",
					"name": "losws_" + side + loop + varPrefix,
					"str": eocInfo[varPrefix + 'losws']
				};
				c.addTableWidget(field, row);
				
				field = {
					"type": "html",
					"name": "uas_" + side + loop + varPrefix,
					"str": eocInfo[varPrefix + 'uas']
				};
				c.addTableWidget(field, row);
				
				field = {
					"type": "html",
					"name": "elaps_" + side + loop + varPrefix,
					"str": eocInfo[varPrefix + 'elaps']
				};
				c.addTableWidget(field, row);
			};
			
			showInterval("Curr 15 minutes", "m15", row1);
			showInterval("Curr 1 day", "d1", row2);
		};
		
		/* show all 15 minutes intervals */
		var show15MinIntervals = function(eocInfo, side, loop, c, row) {
			if (eocInfo.eoc_error == 1) return;
			
			var field;
			
			field = {
				"type": "html",
				"name": "int_day_" + side + loop + eocInfo['int'],
				"str": eocInfo.int_day
			};
			c.addTableWidget(field, row);
			
			field = {
				"type": "html",
				"name": "time_start_" + side + loop + eocInfo['int'],
				"str": eocInfo.time_start
			};
			c.addTableWidget(field, row);
			
			field = {
				"type": "html",
				"name": "time_end_" + side + loop + eocInfo['int'],
				"str": eocInfo.time_end
			};
			c.addTableWidget(field, row);
			
			field = {
				"type": "html",
				"name": "es_" + side + loop + eocInfo['int'],
				"str": eocInfo.es
			};
			c.addTableWidget(field, row);
			
			field = {
				"type": "html",
				"name": "ses_" + side + loop + eocInfo['int'],
				"str": eocInfo.ses
			};
			c.addTableWidget(field, row);
			
			field = {
				"type": "html",
				"name": "crc_" + side + loop + eocInfo['int'],
				"str": eocInfo.crc
			};
			c.addTableWidget(field, row);
			
			field = {
				"type": "html",
				"name": "losws_" + side + loop + eocInfo['int'],
				"str": eocInfo.losws
			};
			c.addTableWidget(field, row);
			
			field = {
				"type": "html",
				"name": "uas_" + side + loop + eocInfo['int'],
				"str": eocInfo.uas
			};
			c.addTableWidget(field, row);
			
			field = {
				"type": "html",
				"name": "mon_pers_" + side + loop + eocInfo['int'],
				"str": eocInfo.mon_pers
			};
			c.addTableWidget(field, row);
			
			/* we have only 96 intervals */
			if (parseInt(eocInfo['int']) + 1 > 96) return;
			
			var rowNext = c.addTableRow();
			
			/* call this function with next interval */
			config.cmdExecute({
				"cmd": $.sprintf("%s -r -i%s -u%s -e%s -l%s -m%s", eocInfoCmd, iface, unit, side,
					loop, parseInt(eocInfo['int']) + 1),
				"callback": function(eocInfo) {
					show15MinIntervals(eocInfo, side, loop, c, rowNext);
				},
				"filter": config.parseData
			});
		};
		
		/* show all 1 days intervals */
		var show1DayIntervals = function(eocInfo, side, loop, c, row) {
			if (eocInfo.eoc_error == 1) return;
			
			var field;
			
			field = {
				"type": "html",
				"name": "int_day_" + side + loop + eocInfo['int'],
				"str": eocInfo.int_day
			};
			c.addTableWidget(field, row);
			
			field = {
				"type": "html",
				"name": "es_" + side + loop + eocInfo['int'],
				"str": eocInfo.es
			};
			c.addTableWidget(field, row);
			
			field = {
				"type": "html",
				"name": "ses_" + side + loop + eocInfo['int'],
				"str": eocInfo.ses
			};
			c.addTableWidget(field, row);
			
			field = {
				"type": "html",
				"name": "crc_" + side + loop + eocInfo['int'],
				"str": eocInfo.crc
			};
			c.addTableWidget(field, row);
			
			field = {
				"type": "html",
				"name": "losws_" + side + loop + eocInfo['int'],
				"str": eocInfo.losws
			};
			c.addTableWidget(field, row);
			
			field = {
				"type": "html",
				"name": "uas_" + side + loop + eocInfo['int'],
				"str": eocInfo.uas
			};
			c.addTableWidget(field, row);
			
			field = {
				"type": "html",
				"name": "mon_pers_" + side + loop + eocInfo['int'],
				"str": eocInfo.mon_pers
			};
			c.addTableWidget(field, row);
			
			/* we have only 30 intervals */
			if (parseInt(eocInfo['int']) + 1 > 30) return;
			
			var rowNext = c.addTableRow();
			
			/* call this function with next interval */
			config.cmdExecute({
				"cmd": $.sprintf("%s -r -i%s -u%s -e%s -l%s -d%s", eocInfoCmd, iface, unit, side,
					loop, parseInt(eocInfo['int']) + 1),
				"callback": function(eocInfo) {
					show1DayIntervals(eocInfo, side, loop, c, rowNext);
				},
				"filter": config.parseData
			});
		};
		
		/* show sensors status */
		var showSensors = function(eocInfo, c) {
			if (eocInfo.eoc_error == 1) return;
			
			var field;
			for (var sensor = 1; sensor <= 3; sensor++) {
				var row = c.addTableRow();
				
				field = {
					"type": "html",
					"name": "sensor_num" + sensor,
					"str": sensor
				};
				c.addTableWidget(field, row);
				
				field = {
					"type": "html",
					"name": "sensor_state" + sensor,
					"str": eocInfo[$.sprintf("sens%s_cur", sensor)]
				};
				c.addTableWidget(field, row);
				
				field = {
					"type": "html",
					"name": "sensor_counter" + sensor,
					"str": eocInfo[$.sprintf("sens%s_cnt", sensor)]
				};
				c.addTableWidget(field, row);
			}
		};
		
		/* type of sides */
		var sides = [];
		if (unit == "stu-c") sides.push("CustSide");
		else if (unit == "stu-r") sides.push("NetSide");
		else {
			sides.push("NetSide");
			sides.push("CustSide");
		}
		
		/* add State table */
		var cState = page.addContainer("statistics");
		cState.setHelpPage("eoc");
		cState.addTitle(iface + " state", 9);
		cState.addTableHeader("Side|Pair|SNR|LoopAttn|ES|SES|CV|LOSWS|UAS");
		
		$.each(sides, function(num, side) {
			for (var loop = 0; loop < eocInfo.loop_num; loop++) {
				var row = cState.addTableRow();
				
				config.cmdExecute({
					"cmd": $.sprintf("%s -r -i%s -u%s -e%s -l%s", eocInfoCmd, iface, unit, side, loop),
					/* 
					 * because loop is common variable, we use double-closure to ensure that every callback
					 * has it's own copy of this variable.
					 */
					"callback": function(loop2) {
						return function(eocInfo) {
							showState(eocInfo, side, loop2, cState, row);
						}
					}(loop),
					"filter": config.parseData
				});
			}
		});
		
		/* for regenerators add Sensors table */
		if (unit.search("sru") != -1) {
			page.addBr("statistics");
			var cSensors = page.addContainer("statistics");
			cSensors.setHelpPage("eoc");
			cSensors.addTitle(iface + " sensors", 9);
			cSensors.addTableHeader("Sensor #|Current state|Event Counter");
			
			config.cmdExecute({
				"cmd": $.sprintf("%s -r -i%s -u%s --sensors", eocInfoCmd, iface, unit),
				"callback": function(eocInfo) {
					showSensors(eocInfo, cSensors);
				},
				"filter": config.parseData
			});
		}
		
		/* add Relative counters table */
		page.addBr("statistics");
		var cRelative = page.addContainer("statistics");
		cRelative.setHelpPage("eoc");
		cRelative.addTitle(iface + " relative counters", 10);
		cRelative.addTableHeader("Start date|Start time|Side|Pair|ES|SES|CV|LOSWS|UAS|Reset");
		
		$.each(sides, function(num, side) {
			for (var loop = 0; loop < eocInfo.loop_num; loop++) {
				var row = cRelative.addTableRow();
				
				config.cmdExecute({
					"cmd": $.sprintf("%s -r -i%s -u%s -e%s -l%s", eocInfoCmd, iface, unit, side, loop),
					"callback": function(loop2) {
						return function(eocInfo) {
							showRelativeCounters(eocInfo, side, loop2, cRelative, row);
						}
					}(loop),
					"filter": config.parseData
				});
			}
		});
		
		/* add current intervals */
		page.addBr("statistics");
		var cCurrIntervals = page.addContainer("statistics");
		cCurrIntervals.setHelpPage("eoc");
		cCurrIntervals.addTitle(iface + " current intervals", 9);
		cCurrIntervals.addTableHeader("Interval|Side|Pair|ES|SES|CV|LOSWS|UAS|Time elapsed");
		
		$.each(sides, function(num, side) {
			for (var loop = 0; loop < eocInfo.loop_num; loop++) {
				var row1 = cCurrIntervals.addTableRow();
				var row2 = cCurrIntervals.addTableRow();
				
				config.cmdExecute({
					"cmd": $.sprintf("%s -r -i%s -u%s -e%s -l%s", eocInfoCmd, iface, unit, side, loop),
					"callback": function(loop2) {
						return function(eocInfo) {
							showCurrentIntervals(eocInfo, side, loop2, cCurrIntervals, row1, row2);
						}
					}(loop),
					"filter": config.parseData
				});
			}
		});
		
		/* add 15 minutes intervals */
		$.each(sides, function(num, side) {
			for (var loop = 0; loop < eocInfo.loop_num; loop++) {
				/* create table for each side and pair */
				page.addBr("statistics");
				var c15MinIntervals = page.addContainer("statistics");
				c15MinIntervals.setHelpPage("eoc");
				c15MinIntervals.addTitle(
					$.sprintf("%s %s Pair%s 15 Minutes error intervals", iface, side, loop + 1), 9);
				c15MinIntervals.addTableHeader("Date|Start time|End time|ES|SES|CV|LOSWS|UAS|Monitoring (%)");

				var row = c15MinIntervals.addTableRow();
				
				/* call function with initial interval number (1) */
				config.cmdExecute({
					"cmd": $.sprintf("%s -r -i%s -u%s -e%s -l%s -m%s", eocInfoCmd, iface, unit, side, loop, "1"),
					"callback": function(loop2) {
						return function(eocInfo) {
							show15MinIntervals(eocInfo, side, loop2, c15MinIntervals, row);
						}
					}(loop),
					"filter": config.parseData
				});
			}
		});
		
		/* add 1 days intervals */
		$.each(sides, function(num, side) {
			for (var loop = 0; loop < eocInfo.loop_num; loop++) {
				/* create table for each side and pair */
				page.addBr("statistics");
				var c1DayIntervals = page.addContainer("statistics");
				c1DayIntervals.setHelpPage("eoc");
				c1DayIntervals.addTitle(
					$.sprintf("%s %s Pair%s 1 Day error intervals", iface, side, loop + 1), 7);
				c1DayIntervals.addTableHeader("Date|ES|SES|CV|LOSWS|UAS|Monitoring (%)");

				var row = c1DayIntervals.addTableRow();
				
				/* call function with initial interval number (1) */
				config.cmdExecute({
					"cmd": $.sprintf("%s -r -i%s -u%s -e%s -l%s -d%s", eocInfoCmd, iface, unit, side, loop, "1"),
					"callback": function(loop2) {
						return function(eocInfo) {
							show1DayIntervals(eocInfo, side, loop2, c1DayIntervals, row);
						}
					}(loop),
					"filter": config.parseData
				});
			}
		});
	};
	
	/* add statistics tab for MR17H */
	if (type == config.getOEM("MR17H_DRVNAME")) {
		page.addTab({
			"id": "statistics",
			"name": "Statistics",
			"func": function() {
				/* do not show statistics for manual-controlled interfaces */
				if (config.get($.sprintf("sys_pcicfg_s%s_%s_ctrl", pcislot, pcidev)) == "manual") {
					var c = page.addContainer("statistics");
					c.setHelpPage("eoc");
					c.addTitle("Statistics is available only for interfaces with EOCd control.");
					return;
				}
				
				config.cmdExecute({
					"cmd": $.sprintf("%s -ri%s", eocInfoCmd, iface),
					"callback": function(eocInfo) {
						showStatistics(eocInfo);
					},
					"filter": config.parseData
				});
			}
		});
	}
	
	page.generateTabs();
};
