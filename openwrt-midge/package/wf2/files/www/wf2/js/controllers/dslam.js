var vlan_number = 0;

Controllers.dslam_dsl_all = function() {
	var page = this.Page();
	page.setHelpPage("");
	page.addTab({
		"id": "dsl_all",
		"name": "SHDSL status",
		"func": function() {
			var c, field, row;
			c = page.addContainer("dsl_all");
			c.addTitle("SHDSL status", {"colspan":6});
			c.addTableHeader("Port|Power|Status|Uptime|Tcpam|Rate");

			var ifaces = config.getParsed("sys_dslam_ifaces");
			var status;
			config.cmdExecute({"cmd": "./mam17h_status_all_json.sh", "async" : false, "dataType": "json", "callback": 
				function(reply) {
					status = reply;
				}
			});
			$.each(ifaces, function(n, iface) {
				var slot = config.get($.sprintf("sys_iface_%s_slot", iface));
				var port = config.get($.sprintf("sys_iface_%s_port", iface));
				var sw_num = config.get($.sprintf("sys_iface_%s_sw", iface));
				var sw_port = config.get($.sprintf("sys_iface_%s_sw_port", iface));

				var state = "<font color='#FF0000'>offline</font>";
				var rate="", tcpam="", uptime="", pwron="off", pwr="";
				
				pwron = (status[iface].pwr.pwron==1)?"on":"off";
				pwr = status[iface].pwr.presence==1?"p":"";
				if (pwr == "") pwron = "none";
				if (status[iface].link.link_state == 1) {
					state = "<font color='#00FF00'>online</font>";
					rate = status[iface].link.rate;
					tcpam = status[iface].link.tcpam;
					uptime = status[iface].link.uptime;
//					var d = (uptime / 86400) - ((uptime / 86400)%1);
//					uptime = uptime - d * 86400;
//					var h = (uptime / 3600) - ((uptime / 3600)%1);
//					uptime = uptime - h * 3600;
//					var m = (uptime / 60) - ((uptime / 60)%1)
//					uptime = uptime - m * 60;
//					var s = uptime;
//					uptime = $.sprintf("%s:%s:%s", h, m, s);
				}
				
				row = c.addTableRow();
				field = {
					"type" : "html",
					"name" : $.sprintf("p%s%s", slot, port),
					"str" : $.sprintf("%s (port %s.%s, module %s%s%s)", iface, slot, port, config.getOEM("MAM17H_MODNAME"), 4, pwr)
				};
				c.addTableWidget(field, row);
				field = {
					"type" : "html",
					"name" : $.sprintf("p%s%s_pwr", slot, port),
					"str" : pwron
				};
				c.addTableWidget(field, row);
				field = {
					"type" : "html",
					"name" : $.sprintf("p%s%s_state", slot, port),
					"str" : state
				};
				c.addTableWidget(field, row);
				field = {
					"type" : "html",
					"name" : $.sprintf("p%s%s_uptime", slot, port),
					"str" : uptime
				};
				c.addTableWidget(field, row);
				field = {
					"type" : "html",
					"name" : $.sprintf("p%s%s_tcpam", slot, port),
					"str" : tcpam
				};
				c.addTableWidget(field, row);
				field = {
					"type" : "html",
					"name" : $.sprintf("p%s%s_rate", slot, port),
					"str" : rate
				};
				c.addTableWidget(field, row);
			});
		}
	});
	page.generateTabs();

}

Controllers.dslam_dsl = function(iface, pcislot, pcidev) {
	/* create page and set common settings */
	var page = this.Page();
	page.setHelpPage("");

	/* fill select with rates */
	var rateList = function(select, first, last, step, cur) {
		var rates = "";
		for (var i = first; i <= last; i += step) {
			rates += i + " ";
		}
		rates = $.trim(rates);
		$(select).setOptionsForSelect({"options": rates, "curValue": cur});
	};


	/* show status */
	var mam17hStatus = function(status) {
		var c, field;
		c = page.addContainer("status", {"clear": true});
		num_chan = config.get($.sprintf("sys_pcitbl_s%s_ifnum", pcislot));
		pwr = config.get($.sprintf("sys_pcicfg_s%s_pwr_source", pcislot));
		if (pwr == "1") pwr="p"; else pwr="";

		c.addTitle($.sprintf("%s (port %s.%s, module %s%s%s) status", iface, pcislot-2, pcidev, config.getOEM("MAM17H_MODNAME"), num_chan, pwr));

		field = {
			"type": "html",
			"name": "link_state",
			"text": "Link state",
			"str": status.link.link_state == "1" ? "online" : "offline"
		};
		c.addWidget(field);

		/* power present */
		if (status.pwr.presence == "1") {
			field = {
				"type": "html",
				"name": "pwrUnb",
				"text": "Power balance",
				"str": status.pwr.unb == "0" ? "balanced" : "unbalanced"
			};
			c.addWidget(field);

			field = {
				"type": "html",
				"name": "pwrOvl",
				"text": "Power overload",
				"str": status.pwr.ovl == "0" ? "no overload" : "overload"
			};
			c.addWidget(field);
		}

		/* online */
		if (status.link.link_state == "1") {
			field = {
				"type": "html",
				"name": "actualRate",
				"text": "Actual rate",
				"str": status.link.rate
			};
			c.addWidget(field);

			field = {
				"type": "html",
				"name": "actualLineCode",
				"text": "Actual line code",
				"str": status.link.tcpam
			};
			c.addWidget(field);
/*
			field = {
				"type": "html",
				"name": "actualClockMode",
				"text": "Actual clock mode",
				"str": status.link.clkmode
			};
			c.addWidget(field);
*/
			/* statistics */
			field = {
				"type": "html",
				"name": "snrMargin",
				"text": "SNR margin",
				"descr": "Signal/Noise ratio margin",
				"str": status.link.statistics_row.split(" ")[0]
			};
			c.addWidget(field);

			field = {
				"type": "html",
				"name": "loopAttn",
				"text": "Loop attenuation",
				"str": status.link.statistics_row.split(" ")[1]
			};
			c.addWidget(field);
		}

		// PBO
		field = {
			"type": "html",
			"name": "pboMode",
			"text": "PBO",
			"descr": "Power backoff",
			"str": status.pbo.mode
		};
		c.addWidget(field);

		if (status.pbo.mode == "Forced") {
			field = {
				"type": "html",
				"name": "pboVal",
				"text": "PBO values",
				"descr": "Power backoff values",
				"str": status.pbo.val + " dB"
			};
			c.addWidget(field);
		}

	};



	var rateList8 = [192,256,320,384,512,640,768,1024,1280,1536,1792,2048,2304,2560,3072,3584,3840];
	var rateList16 = [192,256,320,384,512,640,768,1024,1280,1536,1792,2048,2304,2560,3072,3584,3840];
	var rateList32 = [768,1024,1280,1536,1792,2048,2304,2560,3072,3584,3840,4096,4608,5120,5696,6144,7168,8192,9216,10176];
	var rateList64 = [768,1024,1280,1536,1792,2048,2304,2560,3072,3584,3840,4096,4608,5120,5696,6144,7168,8192,9216,10240,11520,12736];
	var rateList128 = [768,1024,1280,1536,1792,2048,2304,2560,3072,3584,3840,4096,4608,5120,5696,6144,7168,8192,9216,10240,11520,12800,14080,15296];

	var getRateList = function(tcpam) {
		switch (tcpam) {
		case "tcpam8":
			return rateList8;
		case "tcpam16":
			return rateList16;
		case "tcpam32":
			return rateList32;
		case "tcpam64":
			return rateList64;
		case "tcpam128":
			return rateList128;
		}
	};
	/* return minimum rate for passed rateList */
	var getMinRate = function(rateList) {
		return rateList[0];
	};

	/* return maximum rate for passed rateList */
	var getMaxRate = function(rateList) {
		return rateList[rateList.length - 1];
	};

	/* get rate list depending on TCPAM and chipVer value, with added "other" value */
	var getRates = function(tcpam) {
		var rates = getRateList(tcpam);
		var ratesList = {};

		$.each(rates, function(num, rate) {
			ratesList[rate] = rate;
		});
		ratesList['-1'] = "other";

		return ratesList;
	};

	/* returns correct rate for passed chipVer and tcpam */
	var getNearestCorrectRate = function(tcpam, rate) {
		var rateList = getRateList(tcpam);
		var availableRate = parseInt(rate, 10);
		var min = getMinRate(rateList);
		var max = getMaxRate(rateList);

		if (availableRate < min || availableRate < 64) {
			return min;
		} else if (availableRate > max) {
			return max;
		} else if ((availableRate % 64) != 0) {
			var remainder = availableRate % 64;
			if (remainder < 32) {
				availableRate -= remainder;
				return availableRate;
			} else {
				availableRate += (64 - remainder);
				return availableRate;
			}
		} else {
			return availableRate;
		}
	};

	/* return description for Rate field */
	var getDslRateDescr = function(tcpam) {
		var rateList = getRateList(tcpam);
		return $.sprintf("%s, %s %s %s %s.", _("DSL line rate in kbit/s"), _("from"),
				getMinRate(rateList), _("to"), getMaxRate(rateList));
	};

	var mam17hSettings = function() {
		var c, field;
		c = page.addContainer("settings");
		c.setSubsystem($.sprintf("dslam_dsl.%s.%s", pcislot, pcidev));

		num_chan = config.get($.sprintf("sys_pcitbl_s%s_ifnum", pcislot));
		pwr = config.get($.sprintf("sys_pcicfg_s%s_pwr_source", pcislot));
		if (pwr == "1") pwr="p"; else pwr="";

		c.addTitle($.sprintf("%s (port %s.%s, module %s%s%s) status", iface, pcislot-2, pcidev, config.getOEM("MAM17H_MODNAME"), num_chan, pwr));


		// dsl mode change
		var onModeChange = function() {
			if ($("#mode").val() == "slave") {
				$(".widgetMaster").parents("tr").remove();
			} else {
				addMasterWidgets();
			}
		};

		// add or remove pboval field depending on pbomode value
		var setPboval = function() {
			// If PBO is active, add text widget to enter it's value
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
		};

		// add or remove mrate field depending on rate value
		var setMrate = function() {
			// if rate is "other", add text widget to enter rate value
			if ($("#rate").val() == "-1" && $("#mrate").length == 0) {
				var field = {
					"type": "text",
					"name": $.sprintf("sys_pcicfg_s%s_%s_mrate", pcislot, pcidev),
					"id": "mrate",
					"validator": {"required": true, "min": 0},
					"cssClass": "widgetManualMaster",
					"onChange": function() {
						$("#mrate").val(getNearestCorrectRate($("#tcpam").val(),
								$("#mrate").val()))
					},
					"valueFilter": function(value) {
						return getNearestCorrectRate($("#tcpam").val(), value);
					}
				};
				c.addSubWidget(field, {"type": "insertAfter", "anchor": "#rate"});
			// otherwise, remove it
			} else if ($("#rate").val() != "-1") {
				$("#mrate").remove();
			}
		};

		// add widgets for master
		var addMasterWidgets = function() {

			// tcpam
			field = {
				"type": "select",
				"name": $.sprintf("sys_pcicfg_s%s_%s_code", pcislot, pcidev),
				"id": "tcpam",
				"text": "Coding",
				"descr": "DSL line coding.",
				"options": "tcpam8 tcpam16 tcpam32 tcpam64 tcpam128",
				"defaultValue": "tcpam16",
				"cssClass": "widgetMaster",
				"onChange": function() {
					// update rate list
					$("#rate").setOptionsForSelect({
							"options": getRates($("#tcpam").val()),
							"curValue": getNearestCorrectRate($("#tcpam").val(), $("#rate").val())
					});
					setMrate();

					// update description
					$("#rate").nextAll("p").html(getDslRateDescr($("#tcpam").val()));
				}
			};
			c.addWidget(field);

			// rate
			field = {
				"type": "select",
				"name": $.sprintf("sys_pcicfg_s%s_%s_rate", pcislot, pcidev),
				"id": "rate",
				"text": "Rate",
				"descr": getDslRateDescr($("#tcpam").val()),
				"onChange": setMrate,
				"cssClass": "widgetMaster",
				"options": getRates($("#tcpam").val()),
				"defaultValue": "2304"
			};
			c.addWidget(field);
			setMrate();

			// annex
			field = {
				"type": "select",
				"name": $.sprintf("sys_pcicfg_s%s_%s_annex", pcislot, pcidev),
				"text": "Annex",
				"descr": "DSL Annex.",
				"options": {"A": "Annex A", "B": "Annex B"},
				"cssClass": "widgetMaster"
			};
			c.addWidget(field);

			// pbomode
			field = {
				"type": "checkbox",
				"name": $.sprintf("sys_pcicfg_s%s_%s_pbomode", pcislot, pcidev),
				"id": "pbomode",
				"text": "PBO forced",
				"descr": "Example: 21:13:15, STU-C-SRU1=21,SRU1-SRU2=13,...",
				"onClick": setPboval,
				"cssClass": "widgetMaster"
			};
			c.addWidget(field);
			setPboval();
/*
			// clock mode
			field = {
				"type": "select",
				"name": $.sprintf("sys_pcicfg_s%s_%s_clkmode", pcislot, pcidev),
				"id": "clockMode",
				"text": "Clock mode",
				"descr": "DSL clock mode.",
				"options": "sync plesio",
				"cssClass": "widgetMaster"
			};
			c.addWidget(field);
*/
		};


		// mode
		field = {
			"type": "select",
			"name": $.sprintf("sys_pcicfg_s%s_%s_mode", pcislot, pcidev),
			"id": "mode",
			"text": "Mode",
			"descr": "DSL mode.",
			"options": {"master": "Master", "slave": "Slave"},
			"defaultValue" : "slave",
			"onChange": onModeChange
		};
		c.addWidget(field);

		var pwr_source = config.get($.sprintf("sys_pcicfg_s%s_pwr_source", pcislot));
		if (pwr_source == "1") {
		if (pcidev == 0) {
			field = {
				"type": "select",
				"name": $.sprintf("sys_pcicfg_s%s_pwr_0_on", pcislot),
				"id": "power",
				"text": "Power",
				"descr": "DSL power feeding mode.",
				"options": {"pwroff": "off", "pwron": "on"},
			};
			c.addWidget(field);		
		}
		if (pcidev == 2) {
			field = {
				"type": "select",
				"name": $.sprintf("sys_pcicfg_s%s_pwr_1_on", pcislot),
				"id": "power",
				"text": "Power",
				"descr": "DSL power feeding mode.",
				"options": {"pwroff": "off", "pwron": "on"},
			};
			c.addWidget(field);
		}

		if (pcidev == 1) {
			str = config.get($.sprintf("sys_pcicfg_s%s_pwr_0_on", pcislot));
			if (str != "pwron") str = "off"; else str = "on";
			field = {
				"type": "html",
				"name": $.sprintf("sys_pcicfg_s%s_pwr_0_on", pcislot),
				"id": "power",
				"text": "Power",
				"descr": "DSL power feeding mode. (combined with dsl0)",
				"str": str
			};
			c.addWidget(field);		
		}
		if (pcidev == 3) {
			str = config.get($.sprintf("sys_pcicfg_s%s_pwr_1_on", pcislot));
			if (str != "pwron") str = "off"; else str = "on";
			field = {
				"type": "html",
				"name": $.sprintf("sys_pcicfg_s%s_pwr_1_on", pcislot),
				"id": "power",
				"text": "Power",
				"descr": "DSL power feeding mode. (combined with dsl2)",
				"str": str
			};
			c.addWidget(field);		
		}
		}
/*
		field = {
			"type": "select",
			"name": $.sprintf("sys_pcicfg_s%s_%s_advlink", pcislot, pcidev),
			"id": "advlink",
			"text": "AdvLink",
			"descr": "DSL Advanced link detection.",
			"options": "off on"
		};
		c.addWidget(field);
*/
/*
		field = {
			"type": "select",
			"name": $.sprintf("sys_pcicfg_s%s_%s_fill", pcislot, pcidev),
			"text": "Fill",
			"descr": "DSL fill byte value.",
			"options": {"fill_ff": "FF", "fill_7e": "7E"}
		};
		c.addWidget(field);
*/
		// add widgets for manual & master mode
		if ($("#mode").val() == "master") {
			addMasterWidgets();
		}


		// on submit, add or remove current channel from EOCd interfaces
		var additionalKeys = [];

		c.addSubmit({
			"onSuccess": function() {
				updateFields($.sprintf("sys_pcicfg_s%s_%s_mrate", pcislot, pcidev), true);
				
			}
		});

	};


	/* get driver name to determine interface version (MR16H/MR17H) */
	var type = config.get($.sprintf("sys_pcitbl_s%s_iftype", pcislot));

	page.addTab({
		"id": "status",
		"name": "Status",
		"func": function() {
			var c = page.addContainer("status");

			/* check that router is online */
			if (!config.isOnline()) {
				c.addTitle("Router is offline");
				return;
			}

			c.addTitle("Loading data...");
			config.cmdExecute({
				"cmd": "./mam17h_status_json.sh " + iface,
				"callback": mam17hStatus,
				"dataType": "json"
			});
		}
	});

	page.addTab({
		"id": "settings",
		"name": "Settings",
		"func": function () {mam17hSettings();}
	});

	page.generateTabs();
};

var timer = 0;

Controllers.dslamsw = function() {
	var sw_port2port = new Object();
	var port_name = new Object();
	var port2sw_port = new Object();

	var ifaces = config.getParsed("sys_dslam_ifaces");
	$.each(ifaces, function(n, iface) {
		var slot = config.get($.sprintf("sys_iface_%s_slot", iface));
		var port = config.get($.sprintf("sys_iface_%s_port", iface));
		var sw = config.get($.sprintf("sys_iface_%s_sw", iface));
		var sw_port = config.get($.sprintf("sys_iface_%s_sw_port", iface));
		sw_port2port[$.sprintf("sw%sp%s", sw, sw_port)] = $.sprintf("%s.%s", slot, port);
		port_name[$.sprintf("%s.%s", slot, port)] = iface;
		port2sw_port[$.sprintf("%s.%s", slot, port)] = $.sprintf("sw%sp%s", sw, sw_port);
	});
	sw_port2port["sw0p24"] = "4.0";
	port2sw_port["4.0"] = "sw0p24";
	port_name["4.0"] = "gigabit0";
	sw_port2port["sw1p24"] = "4.1";
	port2sw_port["4.1"] = "sw1p24";
	port_name["4.1"] = "gigabit1";
	sw_port2port["sw0p26"] = "4.2";
	port2sw_port["4.2"] = "sw0p26";
	port_name["4.2"] = "CPU";

	var page = this.Page();
	page.setHelpPage("");
/*
	page.addTab({
		"id": "statistics",
		"name": "Ports statistics",
		"func": function() {
			var load_stat = function() {
				page.clearTab("statistics");
				var c, field;
				c = page.addContainer("statistics");
				c.addTitle("Ports statistics", {"colspan":3});
				c.addTableHeader("Port|TX packages|RX packages");
				var sw0_stat, sw1_stat;
				var ifaces = config.getParsed("sys_dslam_ifaces");
				config.cmdExecute({"cmd": "cat /proc/sys/net/dslam_sw/sw0/statistics_json", "async" : false, "dataType": "json", "callback": 
					function(reply) {
						sw0_stat = reply;
					}
				});
				config.cmdExecute({"cmd": "cat /proc/sys/net/dslam_sw/sw1/statistics_json", "async" : false, "dataType": "json", "callback": 
					function(reply) {
						sw1_stat = reply;
					}
				});
				$.each(ifaces, function(n, iface) {
					var slot = config.get($.sprintf("sys_iface_%s_slot", iface));
					var port = config.get($.sprintf("sys_iface_%s_port", iface));
					var sw_num = config.get($.sprintf("sys_iface_%s_sw", iface));
					var sw_port = config.get($.sprintf("sys_iface_%s_sw_port", iface));
					row = c.addTableRow();
					field = {
						"type" : "html",
						"name" : $.sprintf("sw%sp%s", sw_num, sw_port),
						"str" : $.sprintf("Port %s.%s (%s)", slot, port, iface)
					};
					c.addTableWidget(field, row);
					field = {
						"type" : "html",
						"name" : $.sprintf("sw%sp%s_tx", sw_num, sw_port),
						"id" : $.sprintf("sw%sp%s_tx", sw_num, sw_port),
						"str" : $.sprintf("%s", (sw_num == "1")?sw1_stat[sw_port].tx:sw0_stat[sw_port].tx)
					};
					c.addTableWidget(field, row);
					field = {
						"type" : "html",
						"name" : $.sprintf("sw%sp%s_rx", sw_num, sw_port),
						"id" : $.sprintf("sw%sp%s_rx", sw_num, sw_port),
						"str" : $.sprintf("%s", (sw_num == "1")?sw1_stat[sw_port].rx:sw0_stat[sw_port].rx)
					};
					c.addTableWidget(field, row);
				});
				for (i = 0; i <=2; i++) {
					var port_name;
					var sw_num, sw_port;
					row = c.addTableRow();
					switch (i) {
						case 0:
							port_name = "gigabit0";
							sw_num=0;
							sw_port=24;
						break;
						case 1:
							port_name = "gigabit1";
							sw_num=1;
							sw_port=24;
						break;
						case 2:
							port_name = "CPU";
							sw_num=0;
							sw_port=26;
						break;
					}
					field = {
						"type" : "html",
						"name" : $.sprintf("sw%sp%s", sw_num, sw_port),
						"str" : $.sprintf("Port 4.%s (%s)", i, port_name)
					};
					c.addTableWidget(field, row);
					field = {
						"type" : "html",
						"name" : $.sprintf("sw%sp%s_tx", sw_num, sw_port),
						"id" : $.sprintf("sw%sp%s_tx", sw_num, sw_port),
						"str" : $.sprintf("%s", sw_num?sw1_stat[sw_port].tx:sw0_stat[sw_port].tx)
					};
					c.addTableWidget(field, row);
					field = {
						"type" : "html",
						"name" : $.sprintf("sw%sp%s_rx", sw_num, sw_port),
						"id" : $.sprintf("sw%sp%s_rx", sw_num, sw_port),
						"str" : $.sprintf("%s", sw_num?sw1_stat[sw_port].rx:sw0_stat[sw_port].rx)
					};
					c.addTableWidget(field, row);
				}
				c.addSubmit({ "submitName" : "Reset statistics counters", "noSubmit" : true,
					"onSubmit": function() {
						config.cmdExecute({"cmd": "echo \"27\" > /proc/sys/net/dslam_sw/sw0/statistics"});
					}
				});
				setTimeout(load_stat, 1000);
			}
			if (timer == 1) return true;
			load_stat();
			timer = 1;
		}
	});
*/

	page.addTab({
		"id": "vlan",
		"name": "VLAN",
		"func": function() {
			var c, field;
			c = page.addContainer("vlan");
			c.addTitle("Ports settings");
			
			var ifaces = config.getParsed("sys_dslam_ifaces");

			var addWidgets = function() {
				var str = new String($("#port").val());
				var sw_num, sw_port;
				if ((str.charCodeAt(0) - 48) == 4) {
					if ((str.charCodeAt(2) - 48) == 0) {
						sw_num = 0;
						sw_port = 24;
					}
					if ((str.charCodeAt(2) - 48) == 1) {
						sw_num = 1;
						sw_port = 24;
					}
					if ((str.charCodeAt(2) - 48) == 2) {
						sw_num = 0;
						sw_port = 26;
					}
				} else {
					if ((str.charCodeAt(0) - 48) == 0) {
						sw_num = 0;
						sw_port = str.charCodeAt(2) - 48;
					} else {
						if ((str.charCodeAt(0) - 48) == 1) {
							sw_num = 0;
							sw_port = str.charCodeAt(2) - 48 + 8;
						} else {
							if ((str.charCodeAt(0) - 48) == 2) {
								sw_num = 1;
								sw_port = str.charCodeAt(2) - 48;
							} else {
								if ((str.charCodeAt(0) - 48) == 3) {
									sw_num = 1;
									sw_port = str.charCodeAt(2) - 48 + 8;
								}
							}
						}
					}
				}
//				alert("sw_num = "+sw_num+" sw_port = "+sw_port);
				var field = {
					"type" : "text",
					"name" : $.sprintf("sys_dslam_sw%s_vlan_%s_pvid", sw_num, sw_port),
					"text": "PVID",
//					"options": {"0":"none", "1":"tagging", "2":"untagging"},
					"cssClass": "widgetsport",
					"id": $.sprintf("sys_dslam_sw%s_vlan_%s_pvid", sw_num, sw_port)
				}
				c.addWidget(field);
				field = {
					"type" : "select",
					"name" : $.sprintf("sys_dslam_sw%s_vlan_%s_tag", sw_num, sw_port),
					"text": "Tagging/Untagging",
					"options": {"0":"none", "1":"tagging", "2":"untagging"},
					"cssClass": "widgetsport",
					"id": $.sprintf("sys_dslam_sw%s_vlan_%s_tag", sw_num, sw_port)
				}
				c.addWidget(field);
			
				updateFields($.sprintf("sys_dslam_sw%s_vlan_%s_pvid", sw_num, sw_port), false);
				updateFields($.sprintf("sys_dslam_sw%s_vlan_%s_tag", sw_num, sw_port), false);
				var pvid = config.get($.sprintf("sys_dslam_sw%s_vlan_%s_pvid", sw_num, sw_port));
				var tag = config.get($.sprintf("sys_dslam_sw%s_vlan_%s_tag", sw_num, sw_port));
				if (pvid == null) pvid = "";
				if (tag == null) tag = 0;
				$($.sprintf("#sys_dslam_sw%s_vlan_%s_pvid", sw_num, sw_port)).val(pvid);
				$($.sprintf("#sys_dslam_sw%s_vlan_%s_tag", sw_num, sw_port)).val(tag);
			}

			var onChange = function() {
				$(".widgetsport").parents("tr").remove();
				addWidgets();
			}

			page.addBr("vlan");
			var c2 = page.addContainer("vlan");

			list = c2.createList({
				"tabId": "vlan",
				"header": ["Number", "Name", "VID", "Ports"],
				"varList": ["number", "name", "vid", "ports"],
				"listItem": $.sprintf("sys_dslam_vlan_table_"),
				"addMessage": _("Add VLAN"),
				"editMessage": _("Edit VLAN"),
				"listTitle": _("VLANs list"),
				"processValueFunc" : function(options) {
					if (options.varName == "ports") {
						var left=-1, right=-1;
						var ret = "";

						$.each(ifaces, function(n, iface) {
							var slot = config.get($.sprintf("sys_iface_%s_slot", iface));
							var port = config.get($.sprintf("sys_iface_%s_port", iface));
							if (options.keyValues[$.sprintf("p%s%s", slot, port)] == "1") {
								if (left == -1)	{
									left = iface;
									right = iface;
								} else {
									right = iface;
								}
							} else {
								if (left != -1) {
									if (left == right) {
										ret += left+", ";
										left = -1;
									} else {
										ret += left+"-"+right+", ";
										left = -1;
									}
								}
							}
						});
						if (left != -1) {
							if (left == right) {
								ret += left+", ";
							} else {
								ret += left+"-"+right+", ";
							}
						}
						if (options.keyValues["p40"] == "1")
							ret += "gigabit0, ";
						if (options.keyValues["p41"] == "1")
							ret += "gigabit1, ";
						if (options.keyValues["p42"] == "1")
							ret += "CPU, ";

						var str = new String(ret);
						str = str.substring(0, str.length - 2);
						return str;
					} else {
						if (options.varName == "number") {
							vlan_number++;
							return $.sprintf("%s", vlan_number-1);
						} else {
							return options.varValue;
						}
					}
				},
				"checkOnDelete": function(list) {
					return {
						"deleteAllowed": true,
						"actionOnDelete": function() {
							config.cmdExecute({"cmd": "/etc/init.d/dslam_sw update_cfg vlan"});
						}
					};
				},
				"checkOnSubmit": function(list) {
//					alert("checkOnSubmit");
					sw0ports = 0;
					sw1ports = 0;
					if (document.getElementById("p40").checked) sw0ports += Math.pow(2, 24);
					if (document.getElementById("p41").checked) sw1ports += Math.pow(2, 24);
					if (document.getElementById("p42").checked) sw0ports += Math.pow(2, 26);
					for (i = 0; i <= 3; i++)
					{
						for (j = 0; j <= 7; j++)
						{
//							alert (document.getElementById($.sprintf("p%s%s", i, j)));
							if (document.getElementById($.sprintf("p%s%s", i, j)) != null)
							if (document.getElementById($.sprintf("p%s%s", i, j)).checked)
							{
								switch (i) {
									case 0:
										sw0ports += Math.pow(2, j);
									break;
									case 1:
										sw0ports += Math.pow(2, j+8);
									break;
									case 2:
										sw1ports += Math.pow(2, j);
									break;
									case 3:
										sw1ports += Math.pow(2, j+8);
									break;
								}
							}
						}
					}
					if (sw0ports && sw1ports) {
						sw0ports += Math.pow(2, 25);
						sw1ports += Math.pow(2, 25);
					}
					$("#sw0ports").val($.sprintf("%x", sw0ports));
					$("#sw1ports").val($.sprintf("%x", sw1ports));
					var items = config.getParsed("sys_dslam_vlan_table_*");
					var num_vlans=0;
					$.each(items, function(key, value) {
						num_vlans++;
					});
					if (num_vlans >= 31)
					{
						return {"addAllowed": false, "message": "Too much VLANs, remove one"};
					} else {
						config.cmdExecute({"cmd": "/etc/init.d/dslam_sw update_cfg vlan"});
//						alert("sw0ports = "+sw0ports+" sw1ports = "+sw1ports);
						return {"addAllowed": true};
					}
				}
			});

			field = {
				"type": "text",
				"name": "name",
				"text": "VLAN name",
				"validator": {"required": true, "alphanumU": true}
			};
			list.addWidget(field);
                        
       	                field = {
				"type": "text",
				"name": "vid",
				"text": "VLAN ID (VID)",
				"validator": {"required": true, "min": 0}
			};
			list.addWidget(field);

       	                field = {
				"type": "hidden",
				"name": "sw0ports",
				"text": "sw1Ports",
				"id"  : "sw0ports"
			};
			list.addWidget(field);

       	                field = {
				"type": "hidden",
				"name": "sw1ports",
				"text": "sw1Ports",
				"id"  : "sw1ports"
			};
			list.addWidget(field);
			
			$.each(ifaces, function(n, iface) {
				var slot = config.get($.sprintf("sys_iface_%s_slot", iface));
				var port = config.get($.sprintf("sys_iface_%s_port", iface));
				field = {
					"type": "checkbox",
					"name": $.sprintf("p%s%s", slot, port),
					"text": $.sprintf("Port %s.%s (%s)", slot, port, iface),
					"id"  : $.sprintf("p%s%s", slot, port),
				};
				list.addWidget(field);
			});
			field = {
				"type": "checkbox",
				"name": "p40",
				"text": "Port 4.0 (gigabit0)",
				"id"  : "p40",
			};
			list.addWidget(field);
			field = {
				"type": "checkbox",
				"name": "p41",
				"text": "Port 4.1 (gigabit1)",
				"id"  : "p41",
			};
			list.addWidget(field);
			field = {
				"type": "checkbox",
				"name": "p42",
				"text": "Port 4.2 (CPU)",
				"id"  : "p42",
			};
			list.addWidget(field);


			vlan_number = 0;
			list.generateList();

			var options = new Object();

			$.each(ifaces, function(n, iface) {
				var slot = config.get($.sprintf("sys_iface_%s_slot", iface));
				var port = config.get($.sprintf("sys_iface_%s_port", iface));
				options[$.sprintf("%s.%s", slot, port)] = $.sprintf("%s.%s (%s)", slot, port, iface);
			});
			options["4.0"] = "4.0 (gigabit0)";
			options["4.1"] = "4.1 (gigabit1)";
			options["4.2"] = "4.2 (CPU)";

			field = {
				"type": "select",
				"text": "Port",
				"id": "port",
				"name": "sys_dslam_tmp",
				"options": options,
				"onChange": onChange
			};
			c.addWidget(field);

			c.addSubmit({"noSubmit" : true,"preSubmit" : function()
				{
					var str = new String($("#port").val());
					var sw_num, sw_port;
					if ((str.charCodeAt(0) - 48) == 4) {
						if ((str.charCodeAt(2) - 48) == 0) {
							sw_num = 0;
							sw_port = 24;
						}
						if ((str.charCodeAt(2) - 48) == 1) {
							sw_num = 1;
							sw_port = 24;
						}
						if ((str.charCodeAt(2) - 48) == 2) {
							sw_num = 0;
							sw_port = 26;
						}
					} else {
						if ((str.charCodeAt(0) - 48) == 0) {
							sw_num = 0;
							sw_port = str.charCodeAt(2) - 48;
						} else {
							if ((str.charCodeAt(0) - 48) == 1) {
								sw_num = 0;
								sw_port = str.charCodeAt(2) - 48 + 8;
							} else {
								if ((str.charCodeAt(0) - 48) == 2) {
									sw_num = 1;
									sw_port = str.charCodeAt(2) - 48;
								} else {
									if ((str.charCodeAt(0) - 48) == 3) {
										sw_num = 1;
										sw_port = str.charCodeAt(2) - 48 + 8;
									}
								}
							}
						}
					}
//					alert("sw_num = "+sw_num+" sw_port = "+sw_port);
					config.cmdExecute({"cmd": $.sprintf("kdb set sys_dslam_sw%s_vlan_%s_pvid=%s", sw_num, sw_port, 
					$($.sprintf("#sys_dslam_sw%s_vlan_%s_pvid", sw_num, sw_port)).val())});
					config.cmdExecute({"cmd": $.sprintf("kdb set sys_dslam_sw%s_vlan_%s_tag=%s", sw_num, sw_port, 
					$($.sprintf("#sys_dslam_sw%s_vlan_%s_tag", sw_num, sw_port)).val())});
							
					config.cmdExecute({"cmd": $.sprintf("/etc/init.d/dslam_sw update_cfg vlan")});
				},
				 "reload" : false});

			addWidgets();
		}

	});
	
	page.addTab({
		"id": "cos",
		"name": "CoS",
		"func": function() {
			var c, field, row;
			c = page.addContainer("cos");
			c.addTitle("CoS settings");
			
			field = {
				"type": "select",
				"name": "sys_dslam_cos_mode",
				"options": {0:"First-in First-out", 1:"Strict Priority", 2:"Weight-and-roundrobin"},
				"text": "CoS mode"
			};
			c.addWidget(field);
			field = {
				"type": "text",
				"name": "sys_dslam_cos_low_priority_queue",
				"text": "Low priority queue weight number",
				"validator": {"alphanumU": true, "min": 0, "max": 7}
			};
			c.addWidget(field);
			field = {
				"type": "text",
				"name": "sys_dslam_cos_high_priority_queue",
				"text": "High priority queue weight number",
				"validator": {"alphanumU": true, "min": 0, "max": 7}
			};
			c.addWidget(field);
			field = {
				"type": "text",
				"name": "sys_dslam_cos_aging_time",
				"text": "Out queue aging time",
				"validator": {"alphanumU": true, "min": 0, "max": 63},
				"descr": "If empty out queue aging is disable."
			};
			c.addWidget(field);
			field = {
				"type": "checkbox",
				"name": "sys_dslam_cos_pause_flow_control",
				"text": "Pause flow control"
			};
			c.addWidget(field);
			c.addSubmit({
				"onSuccess": function() {
					config.cmdExecute({"cmd": "/etc/init.d/dslam_sw update_cfg cos 1"});
				}

			});
			
			page.addBr("cos");
			c = page.addContainer("cos");
			c.addTitle("Ports CoS settings", {"colspan":4});
			c.addTableHeader("Port|Port based CoS|802.1Q priority tag based CoS|IP TOS based CoS");

			var ifaces = config.getParsed("sys_dslam_ifaces");
			$.each(ifaces, function(n, iface) {
				var slot = config.get($.sprintf("sys_iface_%s_slot", iface));
				var port = config.get($.sprintf("sys_iface_%s_port", iface));
				var sw_num = config.get($.sprintf("sys_iface_%s_sw", iface));
				var sw_port = config.get($.sprintf("sys_iface_%s_sw_port", iface));

				row = c.addTableRow();
				field = {
					"type" : "html",
					"name" : $.sprintf("p%s%s", slot, port),
					"str" : $.sprintf("Port %s.%s (%s)", slot, port, iface)
				};
				c.addTableWidget(field, row);
				field = {
					"type" : "checkbox",
					"name" : $.sprintf("sys_dslam_cos_port_p%s%s", slot, port),
					"id"   : $.sprintf("port_sw%s_p%s", sw_num, sw_port)
				};
				c.addTableWidget(field, row);
				field = {
					"type" : "checkbox",
					"name" : $.sprintf("sys_dslam_cos_tag_p%s%s", slot, port),
					"id"   : $.sprintf("tag_sw%s_p%s", sw_num, sw_port)
				};
				c.addTableWidget(field, row);
				field = {
					"type" : "checkbox",
					"name" : $.sprintf("sys_dslam_cos_ip_p%s%s", slot, port),
					"id"   : $.sprintf("ip_sw%s_p%s", sw_num, sw_port)
				};
				c.addTableWidget(field, row);
			});

			for (i = 0; i <=2; i++) {
				var port_name;
				var sw_num, sw_port;
				row = c.addTableRow();
				switch (i) {
					case 0:
						port_name = "gigabit0";
						sw_num=0;
						sw_port=24;
					break;
					case 1:
						port_name = "gigabit1";
						sw_num=1;
						sw_port=24;
					break;
					case 2:
						port_name = "CPU";
						sw_num=0;
						sw_port=26;
					break;
				}
				field = {
					"type" : "html",
					"name" : $.sprintf("somename5%s", i),
					"str" : $.sprintf("Port 4.%s (%s)", i, port_name)
				};
				c.addTableWidget(field, row);
				field = {
					"type" : "checkbox",
					"name" : $.sprintf("sys_dslam_cos_port_p4%s", i),
					"id"   : $.sprintf("port_sw%s_p%s", sw_num, sw_port)
				};
				c.addTableWidget(field, row);
				field = {
					"type" : "checkbox",
					"name" : $.sprintf("sys_dslam_cos_tag_p4%s", i),
					"id"   : $.sprintf("tag_sw%s_p%s", sw_num, sw_port)
				};
				c.addTableWidget(field, row);
				field = {
					"type" : "checkbox",
					"name" : $.sprintf("sys_dslam_cos_ip_p4%s", i),
					"id"   : $.sprintf("ip_sw%s_p%s", sw_num, sw_port)
				};
				c.addTableWidget(field, row);
			}


			var types = {"port":"port", "tag":"tag", "ip":"ip"};
			$.each(types, function(n, type) {
				for (sw = 0; sw <=1; sw++) {
					var low=config.get($.sprintf("sys_dslam_sw%s_cos_%s_low", sw, type));
					var high=config.get($.sprintf("sys_dslam_sw%s_cos_%s_high", sw, type));
					for (port = 0; port <= 26; port++) {
						if (port <= 15) {
							if ((1 << port) & low) {
								document.getElementById($.sprintf("%s_sw%s_p%s", type, sw, port)).checked = true;
							}
						} else {
							if (high & (1 << (port-16))) {
								document.getElementById($.sprintf("%s_sw%s_p%s", type, sw, port)).checked = true;
							}
						}
					}
				}
			});

			c.addSubmit({
				"noSubmit" : true,
				"onSubmit" : function() {
					var types = {"port":"port", "tag":"tag", "ip":"ip"};
					var request="";
					var fields= new Array();
					$.each(types, function(n, type) {
						for (sw = 0; sw <=1; sw++) {
							var low=0, high=0;
							for (port = 0; port <= 26; port++) {
								if (port <= 15) {
									if (document.getElementById($.sprintf("%s_sw%s_p%s", type, sw, port)) &&
									document.getElementById($.sprintf("%s_sw%s_p%s", type, sw, port)).checked)
										low += 1 << port;
								} else {
									if (document.getElementById($.sprintf("%s_sw%s_p%s", type, sw, port)) &&
									document.getElementById($.sprintf("%s_sw%s_p%s", type, sw, port)).checked)
										high += 1 << (port - 16);
								}
							}
//							alert("type = "+type+" sw = "+sw+" low = "+low+" high = "+high);
							request += $.sprintf("kdb set sys_dslam_sw%s_cos_%s_low=%s; ", sw, type, low);
							request += $.sprintf("kdb set sys_dslam_sw%s_cos_%s_high=%s; ", sw, type, high);

							fields.push($.sprintf("sys_dslam_sw%s_cos_%s_low", sw, type, low));
							fields.push($.sprintf("sys_dslam_sw%s_cos_%s_high", sw, type, high));
						}
					});
					config.cmdExecute({"cmd": request, "callback": 
						function() {
							config.cmdExecute({"cmd": "/etc/init.d/dslam_sw update_cfg cos 2"});
							updateFields(fields, false);
						}
					});
				}
			});

			page.addBr("cos");
			c = page.addContainer("cos");
			c.addTitle("TCP/UDP port number based CoS", {"colspan":4});
			
			row = c.addTableRow();
			field = {
				"type" : "html",
				"name" : "FTP",
				"str" : "FTP (20,21)"
			};
			c.addTableWidget(field, row);
			field = {
				"type" : "select",
				"name" : "sys_dslam_cos_ftp",
				"options": {0:"Disable", 1:"Drop/Forward to CPU", 2:"Low priority", 3:"High priority"},
				"defaultValue":0
				
			};
			c.addTableWidget(field, row);
			field = {
				"type" : "html",
				"name" : "SSH",
				"str" : "SSH (22)"
			};
			c.addTableWidget(field, row);
			field = {
				"type" : "select",
				"name" : "sys_dslam_cos_ssh",
				"options": {0:"Disable", 1:"Drop/Forward to CPU", 2:"Low priority", 3:"High priority"},
				"defaultValue":0				
			};
			c.addTableWidget(field, row);

			row = c.addTableRow();
			field = {
				"type" : "html",
				"name" : "TELNET",
				"str" : "TELNET (23)"
			};
			c.addTableWidget(field, row);
			field = {
				"type" : "select",
				"name" : "sys_dslam_cos_telnet",
				"options": {0:"Disable", 1:"Drop/Forward to CPU", 2:"Low priority", 3:"High priority"},
				"defaultValue":0
				
			};
			c.addTableWidget(field, row);
			field = {
				"type" : "html",
				"name" : "SMTP",
				"str" : "SMTP (25)"
			};
			c.addTableWidget(field, row);
			field = {
				"type" : "select",
				"name" : "sys_dslam_cos_smtp",
				"options": {0:"Disable", 1:"Drop/Forward to CPU", 2:"Low priority", 3:"High priority"},
				"defaultValue":0				
			};
			c.addTableWidget(field, row);

			row = c.addTableRow();
			field = {
				"type" : "html",
				"name" : "DNS",
				"str" : "DNS (53)"
			};
			c.addTableWidget(field, row);
			field = {
				"type" : "select",
				"name" : "sys_dslam_cos_dns",
				"options": {0:"Disable", 1:"Drop/Forward to CPU", 2:"Low priority", 3:"High priority"},
				"defaultValue":0
				
			};
			c.addTableWidget(field, row);
			field = {
				"type" : "html",
				"name" : "TFTP",
				"str" : "TFTP (69)"
			};
			c.addTableWidget(field, row);
			field = {
				"type" : "select",
				"name" : "sys_dslam_cos_tftp",
				"options": {0:"Disable", 1:"Drop/Forward to CPU", 2:"Low priority", 3:"High priority"},
				"defaultValue":0				
			};
			c.addTableWidget(field, row);

			row = c.addTableRow();
			field = {
				"type" : "html",
				"name" : "HTTP",
				"str" : "HTTP (80,8080)"
			};
			c.addTableWidget(field, row);
			field = {
				"type" : "select",
				"name" : "sys_dslam_cos_http",
				"options": {0:"Disable", 1:"Drop/Forward to CPU", 2:"Low priority", 3:"High priority"},
				"defaultValue":0
				
			};
			c.addTableWidget(field, row);
			field = {
				"type" : "html",
				"name" : "POP3",
				"str" : "POP3 (110)"
			};
			c.addTableWidget(field, row);
			field = {
				"type" : "select",
				"name" : "sys_dslam_cos_pop3",
				"options": {0:"Disable", 1:"Drop/Forward to CPU", 2:"Low priority", 3:"High priority"},
				"defaultValue":0				
			};
			c.addTableWidget(field, row);

			row = c.addTableRow();
			field = {
				"type" : "html",
				"name" : "NEWS",
				"str" : "NEWS (119)"
			};
			c.addTableWidget(field, row);
			field = {
				"type" : "select",
				"name" : "sys_dslam_cos_news",
				"options": {0:"Disable", 1:"Drop/Forward to CPU", 2:"Low priority", 3:"High priority"},
				"defaultValue":0
				
			};
			c.addTableWidget(field, row);
			field = {
				"type" : "html",
				"name" : "SNTP",
				"str" : "SNTP (22)"
			};
			c.addTableWidget(field, row);
			field = {
				"type" : "select",
				"name" : "sys_dslam_cos_sntp",
				"options": {0:"Disable", 1:"Drop/Forward to CPU", 2:"Low priority", 3:"High priority"},
				"defaultValue":0				
			};
			c.addTableWidget(field, row);

			row = c.addTableRow();
			field = {
				"type" : "html",
				"name" : "NETBIOS",
				"str" : "NETBIOS (137,138,139)"
			};
			c.addTableWidget(field, row);
			field = {
				"type" : "select",
				"name" : "sys_dslam_cos_netbios",
				"options": {0:"Disable", 1:"Drop/Forward to CPU", 2:"Low priority", 3:"High priority"},
				"defaultValue":0
				
			};
			c.addTableWidget(field, row);
			field = {
				"type" : "html",
				"name" : "IMAP",
				"str" : "IMAP (143,220)"
			};
			c.addTableWidget(field, row);
			field = {
				"type" : "select",
				"name" : "sys_dslam_cos_imap",
				"options": {0:"Disable", 1:"Drop/Forward to CPU", 2:"Low priority", 3:"High priority"},
				"defaultValue":0				
			};
			c.addTableWidget(field, row);
			
			row = c.addTableRow();
			field = {
				"type" : "html",
				"name" : "SNMP",
				"str" : "SNMP (161,162)"
			};
			c.addTableWidget(field, row);
			field = {
				"type" : "select",
				"name" : "sys_dslam_cos_snmp",
				"options": {0:"Disable", 1:"Drop/Forward to CPU", 2:"Low priority", 3:"High priority"},
				"defaultValue":0				
			};
			c.addTableWidget(field, row);
			field = {
				"type" : "html",
				"name" : "HTTPS",
				"str" : "HTTPS (443)"
			};
			c.addTableWidget(field, row);
			field = {
				"type" : "select",
				"name" : "sys_dslam_cos_https",
				"options": {0:"Disable", 1:"Drop/Forward to CPU", 2:"Low priority", 3:"High priority"},
				"defaultValue":0				
			};
			c.addTableWidget(field, row);
			
			row = c.addTableRow();
			field = {
				"type" : "html",
				"name" : "1863",
				"str" : "Port number (1863)"
			};
			c.addTableWidget(field, row);
			field = {
				"type" : "select",
				"name" : "sys_dslam_cos_1863",
				"options": {0:"Disable", 1:"Drop/Forward to CPU", 2:"Low priority", 3:"High priority"},
				"defaultValue":0				
			};
			c.addTableWidget(field, row);
			field = {
				"type" : "html",
				"name" : "XP_RDP",
				"str" : "XP_RDP (3389)"
			};
			c.addTableWidget(field, row);
			field = {
				"type" : "select",
				"name" : "sys_dslam_cos_xp_rdp",
				"options": {0:"Disable", 1:"Drop/Forward to CPU", 2:"Low priority", 3:"High priority"},
				"defaultValue":0				
			};
			c.addTableWidget(field, row);

			row = c.addTableRow();
        		field = {
				"type" : "html",
				"name" : "4000",
				"str" : "Port number (4000,8000)"
			};
			c.addTableWidget(field, row);
			field = {
				"type" : "select",
				"name" : "sys_dslam_cos_4000",
				"options": {0:"Disable", 1:"Drop/Forward to CPU", 2:"Low priority", 3:"High priority"},
				"defaultValue":0				
			};
			c.addTableWidget(field, row);
			field = {
				"type" : "html",
				"name" : "5190",
				"str" : "Port number (5190)"
			};
			c.addTableWidget(field, row);
			field = {
				"type" : "select",
				"name" : "sys_dslam_cos_5190",
				"options": {0:"Disable", 1:"Drop/Forward to CPU", 2:"Low priority", 3:"High priority"},
				"defaultValue":0				
			};
			c.addTableWidget(field, row);
			row = c.addTableRow();
			field = {
				"type" : "html",
				"name" : "5050",
				"str" : "Port number (5050)"
			};
			c.addTableWidget(field, row);
			field = {
				"type" : "select",
				"name" : "sys_dslam_cos_5050",
				"options": {0:"Disable", 1:"Drop/Forward to CPU", 2:"Low priority", 3:"High priority"},
				"defaultValue":0				
			};
			c.addTableWidget(field, row);
			field = {
				"type" : "html",
				"name" : "BOOTP",
				"str" : "BOOTP/DHCP (67,68)"
			};
			c.addTableWidget(field, row);
			field = {
				"type" : "select",
				"name" : "sys_dslam_cos_bootp",
				"options": {0:"Disable", 1:"Drop/Forward to CPU", 2:"Low priority", 3:"High priority"},
				"defaultValue":0				
			};
			c.addTableWidget(field, row);


			row = c.addTableRow();
			field = {
				"type" : "text",
				"name" : "sys_dslam_cos_porta",
				"validator": {"alphanumU": true, "min": 0, "max": 65535}
			};
			c.addTableWidget(field, row);
			field = {
				"type" : "select",
				"name" : "sys_dslam_cos_port_a",
				"options": {0:"Disable", 1:"Drop/Forward to CPU", 2:"Low priority", 3:"High priority"},
				"defaultValue":0				
			};
			c.addTableWidget(field, row);
			field = {
				"type" : "text",
				"name" : "sys_dslam_cos_portb",
				"validator": {"alphanumU": true, "min": 0, "max": 65535}
			};
			c.addTableWidget(field, row);
			field = {
				"type" : "select",
				"name" : "sys_dslam_cos_port_b",
				"options": {0:"Disable", 1:"Drop/Forward to CPU", 2:"Low priority", 3:"High priority"},
				"defaultValue":0				
			};
			c.addTableWidget(field, row);

			row = c.addTableRow();
			field = {
				"type" : "text",
				"name" : "sys_dslam_cos_portc",
				"validator": {"alphanumU": true, "min": 0, "max": 65535}
			};
			c.addTableWidget(field, row);
			field = {
				"type" : "select",
				"name" : "sys_dslam_cos_port_c",
				"options": {0:"Disable", 1:"Drop/Forward to CPU", 2:"Low priority", 3:"High priority"},
				"defaultValue":0				
			};
			c.addTableWidget(field, row);
			field = {
				"type" : "text",
				"name" : "sys_dslam_cos_portd",
				"validator": {"alphanumU": true, "min": 0, "max": 65535}
			};
			c.addTableWidget(field, row);
			field = {
				"type" : "select",
				"name" : "sys_dslam_cos_port_d",
				"options": {0:"Disable", 1:"Drop/Forward to CPU", 2:"Low priority", 3:"High priority"},
				"defaultValue":0				
			};
			c.addTableWidget(field, row);

			var ports={0:"ftp",1:"ssh",2:"telnet",3:"smtp",4:"dns",5:"tftp",6:"http",7:"pop3",
				8:"news",9:"sntp",10:"netbios",11:"imap",12:"snmp",13:"https",14:"1863",15:"xp_rdp",
				16:"4000",17:"5190",18:"5050",19:"bootp", 20:"port_a",21:"port_b",22:"port_c",23:"port_d"
			};
			var request="";
			var reg33=config.get("sys_dslam_cos_reg33");
			var reg34=config.get("sys_dslam_cos_reg34");
			var reg35=config.get("sys_dslam_cos_reg35");

//			alert("reg33 = "+reg33+" reg34 = "+reg34+" reg35 = "+reg35);

			$.each(ports, function(n, port) {
				if (n <= 7) {
					$($.sprintf("#sys_dslam_cos_%s", port)).val((reg33 >> (2*n)) & 3)
				} else {
					if (n <= 15) {
						$($.sprintf("#sys_dslam_cos_%s", port)).val((reg34 >> (2*(n-8))) & 3)
					} else {
						$($.sprintf("#sys_dslam_cos_%s", port)).val((reg35 >> (2*(n-16))) & 3)
					}
				}
			});
			$("#sys_dslam_cos_porta").val(config.get("sys_dslam_cos_porta"));
			$("#sys_dslam_cos_portb").val(config.get("sys_dslam_cos_portb"));
			$("#sys_dslam_cos_portc").val(config.get("sys_dslam_cos_portc"));
			$("#sys_dslam_cos_portd").val(config.get("sys_dslam_cos_portd"));

			c.addSubmit({
				"noSubmit" : true,
				"onSubmit" : function() {
					var ports={0:"ftp",1:"ssh",2:"telnet",3:"smtp",4:"dns",5:"tftp",6:"http",7:"pop3",
					8:"news",9:"sntp",10:"netbios",11:"imap",12:"snmp",13:"https",14:"1863",15:"xp_rdp",
					16:"4000",17:"5190",18:"5050",19:"bootp", 20:"port_a",21:"port_b",22:"port_c",23:"port_d"
					};
					var request="";
					var reg33=0, reg34=0, reg35=0;
					$.each(ports, function(n, port) {
						if (n <= 7) {	
							reg33 += ($($.sprintf("#sys_dslam_cos_%s", port)).val() << (2*n));
						} else {
							if (n <= 15) {
								reg34 += ($($.sprintf("#sys_dslam_cos_%s", port)).val() << (2*(n - 8)));
							} else {
								reg35 += ($($.sprintf("#sys_dslam_cos_%s", port)).val() << (2*(n - 16)));
							}
						}
					});
//					alert("reg33 = "+reg33+" reg34 = "+reg34+" reg35 = "+reg35);
					request += $.sprintf("kdb set sys_dslam_cos_reg33=%s; ", reg33);
					request += $.sprintf("kdb set sys_dslam_cos_reg34=%s; ", reg34);
					request += $.sprintf("kdb set sys_dslam_cos_reg35=%s; ", reg35);
					
					request += $.sprintf("kdb set sys_dslam_cos_porta=%s; ", $("#sys_dslam_cos_porta").val());
					request += $.sprintf("kdb set sys_dslam_cos_portb=%s; ", $("#sys_dslam_cos_portb").val());
					request += $.sprintf("kdb set sys_dslam_cos_portc=%s; ", $("#sys_dslam_cos_portc").val());
					request += $.sprintf("kdb set sys_dslam_cos_portd=%s; ", $("#sys_dslam_cos_portd").val());
					
					var fields;
					config.cmdExecute({"cmd": request, "callback": 
						function() {
							config.cmdExecute({"cmd": "/etc/init.d/dslam_sw update_cfg cos 3"});
							updateFields(new Array("sys_dslam_cos_reg33", "sys_dslam_cos_reg34", "sys_dslam_cos_reg35",
							"sys_dslam_cos_porta", "sys_dslam_cos_portb", 
							"sys_dslam_cos_portc", "sys_dslam_cos_portd"), false);
						}
					});
				}
			});

		}
	});

	page.addTab({
		"id": "bandwidth",
		"name": "Bandwidth Control",
		"func": function() {
			var c, field, row;
			c = page.addContainer("bandwidth");
			c.addTitle("Bandwidth Control", {"colspan":3});
			c.addTableHeader("Port|Transmit rate|Receive rate");


			var ifaces = config.getParsed("sys_dslam_ifaces");
			$.each(ifaces, function(n, iface) {
				var slot = config.get($.sprintf("sys_iface_%s_slot", iface));
				var port = config.get($.sprintf("sys_iface_%s_port", iface));
				var sw_num = config.get($.sprintf("sys_iface_%s_sw", iface));
				var sw_port = config.get($.sprintf("sys_iface_%s_sw_port", iface));

				row = c.addTableRow();
				field = {
					"type" : "html",
					"name" : $.sprintf("p%s%s", slot, port),
					"str" : $.sprintf("Port %s.%s (%s)", slot, port, iface)
				};
				c.addTableWidget(field, row);
				field = {
					"type" : "text",
					"name" : $.sprintf("sys_dslam_bandwidth_transmit_sw%s_p%s", sw_num, sw_port),
					"id" : $.sprintf("sys_dslam_bandwidth_transmit_sw%s_p%s", sw_num, sw_port),
					"validator": {"alphanumU": true, "min": 0, "max": 65280},
					"tip" : "From 0 to 65280, modulo 256 Kbyte/s<br>0 or empty - maximum speed",
					"defaultValue":0
				};
				c.addTableWidget(field, row);
				field = {
					"type" : "text",
					"name" : $.sprintf("sys_dslam_bandwidth_receive_sw%s_p%s", sw_num, sw_port),
					"id" : $.sprintf("sys_dslam_bandwidth_receive_sw%s_p%s", sw_num, sw_port),
					"validator": {"alphanumU": true, "min": 0, "max": 65280},
					"tip" : "From 0 to 65280, modulo 256 Kbyte/s<br>0 or empty - maximum speed",
					"defaultValue":0
				};
				c.addTableWidget(field, row);
			});
			for (i = 0; i <=2; i++) {
				var port_name;
				var sw_num, sw_port;
				row = c.addTableRow();
				switch (i) {
					case 0:
						port_name = "gigabit0";
						sw_num=0;
						sw_port=24;
					break;
					case 1:
						port_name = "gigabit1";
						sw_num=1;
						sw_port=24;
					break;
					case 2:
						port_name = "CPU";
						sw_num=0;
						sw_port=26;
					break;
				}
				field = {
					"type" : "html",
					"name" : $.sprintf("somename5%s", i),
					"str" : $.sprintf("Port 4.%s (%s)", i, port_name)
				};
				c.addTableWidget(field, row);
				field = {
					"type" : "text",
					"name" : $.sprintf("sys_dslam_bandwidth_transmit_sw%s_p%s", sw_num, sw_port),
					"id" : $.sprintf("sys_dslam_bandwidth_transmit_sw%s_p%s", sw_num, sw_port),
					"validator": {"alphanumU": true, "min": 0, "max": (i == 2)?65280:522240},
					"tip" : $.sprintf("From 0 to %s Kbyte/s<br>0 or empty - maximum speed", (i == 2)?"65280, modulo 256":"522240, modulo 2048"),
					"defaultValue":0
				};
				c.addTableWidget(field, row);
				field = {
					"type" : "text",
					"name" : $.sprintf("sys_dslam_bandwidth_receive_sw%s_p%s", sw_num, sw_port),
					"id" : $.sprintf("sys_dslam_bandwidth_receive_sw%s_p%s", sw_num, sw_port),
					"validator": {"alphanumU": true, "min": 0, "max": (i == 2)?65280:522240},
					"tip" : $.sprintf("From 0 to %s Kbyte/s<br>0 or empty - maximum speed", (i == 2)?"65280, modulo 256":"522240, modulo 2048"),
					"defaultValue":0
				};
				c.addTableWidget(field, row);
			}
			c.addSubmit({
				"onSubmit": function() {
					for (sw = 0; sw <= 1; sw++) {
						for (port = 0; port <= 26; port++) {
							var modulo = 256;
							if (port == 24) modulo = 2048;
							var receive = parseInt($($.sprintf("#sys_dslam_bandwidth_receive_sw%s_p%s", sw, port)).val());
							var transmit = parseInt($($.sprintf("#sys_dslam_bandwidth_transmit_sw%s_p%s", sw, port)).val());
							if ((receive % modulo) != 0) {
								$($.sprintf("#sys_dslam_bandwidth_receive_sw%s_p%s", sw, port)).val(modulo * Math.round(receive / modulo));
							}
							if ((transmit % modulo) != 0) {
								$($.sprintf("#sys_dslam_bandwidth_transmit_sw%s_p%s", sw, port)).val(modulo * Math.round(transmit / modulo));
							}
						}
					}
				},
				"onSuccess": function() {
					config.cmdExecute({"cmd": "/etc/init.d/dslam_sw update_cfg bandwidth"});
				}
			});
		}
	});
			
	page.addTab({
		"id": "bcast",
		"name": "Broadcast storm control",
		"func": function() {
			var c, field;
			c = page.addContainer("bcast");
			c.addTitle("Broadcast strom control");
			
			field = {
				"type" : "text",
				"name" : "sys_dslam_bcast_threshold",
				"id"   : "sys_dslam_bcast_threshold",
				"text" : "Threshold",
				"validator": {"alphanumU": true, "min": 0, "max": 63},
				"defaultValue":63
			};
			c.addWidget(field);
			$("#sys_dslam_bcast_threshold").val(config.get("sys_dslam_bcast_threshold"));

			var ifaces = config.getParsed("sys_dslam_ifaces");
			$.each(ifaces, function(n, iface) {
				var slot = config.get($.sprintf("sys_iface_%s_slot", iface));
				var port = config.get($.sprintf("sys_iface_%s_port", iface));
				var sw_num = config.get($.sprintf("sys_iface_%s_sw", iface));
				var sw_port = config.get($.sprintf("sys_iface_%s_sw_port", iface));

				field = {
					"type" : "checkbox",
					"name" : $.sprintf("sys_dslam_bcast_sw%s_p%s", sw_num, sw_port),
					"id"   : $.sprintf("sys_dslam_bcast_sw%s_p%s", sw_num, sw_port),
					"text" : $.sprintf("Port %s.%s (%s)", slot, port, iface)
				};
				c.addWidget(field);
			});
			field = {
				"type" : "checkbox",
				"name" : "sys_dslam_bcast_sw0_p24",
				"id"   : "sys_dslam_bcast_sw0_p24",
				"text" : "Port 4.0 (gigabit0)"
			};
			c.addWidget(field);
			field = {
				"type" : "checkbox",
				"name" : "sys_dslam_bcast_sw1_p24",
				"id"   : "sys_dslam_bcast_sw1_p24",
				"text" : "Port 4.1 (gigabit1)"
			};
			c.addWidget(field);
			field = {
				"type" : "checkbox",
				"name" : "sys_dslam_bcast_sw0_p26",
				"id"   : "sys_dslam_bcast_sw0_p26",
				"text" : "Port 4.2 (CPU)"
			};
			c.addWidget(field);
			
			var sw0_reg42=config.get("sys_dslam_sw0_reg42");
			var sw1_reg42=config.get("sys_dslam_sw1_reg42");
			var sw0_reg43=config.get("sys_dslam_sw0_reg43");
			var sw1_reg43=config.get("sys_dslam_sw1_reg43");
			
			for (port = 0; port <= 26; port++) {
				if (port <= 15) {
					if ((sw0_reg42 >> port) & 1)
						document.getElementById($.sprintf("sys_dslam_bcast_sw0_p%s", port)).checked=true;
					if ((sw1_reg42 >> port) & 1)
						document.getElementById($.sprintf("sys_dslam_bcast_sw1_p%s", port)).checked=true;
				} else {
					if ((sw0_reg43 >> (port-16)) & 1)
						document.getElementById($.sprintf("sys_dslam_bcast_sw0_p%s", port)).checked=true;
					if ((sw1_reg43 >> (port-16)) & 1)
						document.getElementById($.sprintf("sys_dslam_bcast_sw1_p%s", port)).checked=true;
				}
			}

			c.addSubmit({
				"noSubmit" : true,
				"onSubmit" : function() {
				        var sw0_reg42=0, sw0_reg43=0;
				        var sw1_reg42=0, sw1_reg43=0;
				        var request = "";
					
					for (port = 0; port <= 26; port++) {
						if (port <= 15) {
							if (document.getElementById($.sprintf("sys_dslam_bcast_sw0_p%s", port)))
							if (document.getElementById($.sprintf("sys_dslam_bcast_sw0_p%s", port)).checked)
								sw0_reg42 += Math.pow(2, port);
							if (document.getElementById($.sprintf("sys_dslam_bcast_sw1_p%s", port)))
							if (document.getElementById($.sprintf("sys_dslam_bcast_sw1_p%s", port)).checked)
								sw1_reg42 += Math.pow(2, port);
						} else {
							if (document.getElementById($.sprintf("sys_dslam_bcast_sw0_p%s", port)))
							if (document.getElementById($.sprintf("sys_dslam_bcast_sw0_p%s", port)).checked)
								sw0_reg43 += Math.pow(2, port-16);
							if (document.getElementById($.sprintf("sys_dslam_bcast_sw1_p%s", port)))
							if (document.getElementById($.sprintf("sys_dslam_bcast_sw1_p%s", port)).checked)
								sw1_reg43 += Math.pow(2, port-16);
						}
					}

//					alert("sw0_reg42 = "+sw0_reg42+" sw0_reg43 = "+sw0_reg43);
//					alert("sw1_reg42 = "+sw1_reg42+" sw1_reg43 = "+sw1_reg43);

					request += $.sprintf("kdb set sys_dslam_sw0_reg42=%s; ", sw0_reg42);
					request += $.sprintf("kdb set sys_dslam_sw1_reg42=%s; ", sw1_reg42);
					request += $.sprintf("kdb set sys_dslam_sw0_reg43=%s; ", sw0_reg43);
					request += $.sprintf("kdb set sys_dslam_sw1_reg43=%s; ", sw1_reg43);
					request += $.sprintf("kdb set sys_dslam_bcast_threshold=%s; ", $("#sys_dslam_bcast_threshold").val());
					
					config.cmdExecute({"cmd": request, "callback": 
						function() {
							config.cmdExecute({"cmd": "/etc/init.d/dslam_sw update_cfg bcast"});
							updateFields(new Array("sys_dslam_sw0_reg42", "sys_dslam_sw1_reg42", "sys_dslam_sw0_reg43",
							"sys_dslam_sw1_reg43", "sys_dslam_bcast_threshold"), false);
						}
					});
				}
			});
		}
	});
/*
	page.addTab({
		"id": "mac_security",
		"name": "MAC security",
		"func": function() {
			var c, field, row;
			c = page.addContainer("mac_security");
			c.addTitle("MAC security");

			var onChange = function() {
				$(".widgetsport").parents("tr").remove();
				addWidgets();
			}

			var options = new Object();

			var ifaces = config.getParsed("sys_dslam_ifaces");
			$.each(ifaces, function(n, iface) {
				var slot = config.get($.sprintf("sys_iface_%s_slot", iface));
				var port = config.get($.sprintf("sys_iface_%s_port", iface));
				options[$.sprintf("%s.%s", slot, port)] = $.sprintf("%s.%s (%s)", slot, port, iface);
			});
			options["4.0"] = "4.0 (gigabit0)";
			options["4.1"] = "4.1 (gigabit1)";
			options["4.2"] = "4.2 (CPU)";

			field = {
				"type" : "select",
				"name" : "port",
				"id"   : "port",
				"text" : "Port",
				"onChange": onChange,
				"options" : options
			}
			c.addWidget(field);

			var sw_num, sw_port;
			var addWidgets = function() {
				var str = new String($("#port").val());
				if ((str.charCodeAt(0) - 48) == 4) {
					if ((str.charCodeAt(2) - 48) == 0) {
						sw_num = 0;
						sw_port = 24;
					}
					if ((str.charCodeAt(2) - 48) == 1) {
						sw_num = 1;
						sw_port = 24;
					}
					if ((str.charCodeAt(2) - 48) == 2) {
						sw_num = 0;
						sw_port = 26;
					}
				} else {
					if ((str.charCodeAt(0) - 48) == 0) {
						sw_num = 0;
						sw_port = str.charCodeAt(2) - 48;
					} else {
						if ((str.charCodeAt(0) - 48) == 1) {
							sw_num = 0;
							sw_port = str.charCodeAt(2) - 48 + 8;
						} else {
							if ((str.charCodeAt(0) - 48) == 2) {
								sw_num = 1;
								sw_port = str.charCodeAt(2) - 48;
							} else {
								if ((str.charCodeAt(0) - 48) == 3) {
									sw_num = 1;
									sw_port = str.charCodeAt(2) - 48 + 8;
								}
							}
						}
					}
				}
//				alert("sw_num = "+sw_num+" sw_port = "+sw_port);
				var field = {
					"type" : "checkbox",
					"name" : $.sprintf("sys_dslam_sw%s_mac_sec_%s", sw_num, sw_port),
					"text": "Block packages from unknown MAC",
					"cssClass": "widgetsport",
					"id": "mac_sec"
				}
				c.addWidget(field);
				
				var macs = "";
				config.cmdExecute({
					"cmd": "/etc/init.d/dslam_sw get_mac_table",
					"dataType" : "json",
//					"async" : false,
					"callback": function(reply) {
						$.each(reply["sw0"], function(mac, port) {
							if ($("#port").val() == $.sprintf("%s", sw_port2port[$.sprintf("sw0p%s", port)])) {
								macs += mac+"\n";
							}
						});
						$.each(reply["sw1"], function(mac, port) {
							if ($("#port").val() == $.sprintf("%s (%s)", sw_port2port[$.sprintf("sw0p%s", port)])) {
								macs += mac+"\n";
							}
						});
//						if (macs == "") macs = "none";
						$($.sprintf("#td_sys_dslam_sw%s_mac_sec_%s_macs", sw_num, sw_port)).html($.sprintf("<span class=\"htmlWidget widgetsport\"><pre>%s</pre></span>", macs));
					}
				});

				field = {
					"type" : "html",
					"name" : $.sprintf("sys_dslam_sw%s_mac_sec_%s_macs", sw_num, sw_port),
					"text": "MACs behind this port",
					"cssClass": "widgetsport",
					"id": $.sprintf("sys_dslam_sw%s_mac_sec_%s_macs", sw_num, sw_port),
					"str" : "wait a sec..."
				}
				c.addWidget(field);
				

//				updateFields($.sprintf("sys_dslam_sw%s_vlan_%s_pvid", sw_num, sw_port), false);
//				updateFields($.sprintf("sys_dslam_sw%s_vlan_%s_tag", sw_num, sw_port), false);
//				var pvid = config.get($.sprintf("sys_dslam_sw%s_vlan_%s_pvid", sw_num, sw_port));
//				var tag = config.get($.sprintf("sys_dslam_sw%s_vlan_%s_tag", sw_num, sw_port));
//				if (pvid == null) pvid = "";
//				if (tag == null) tag = 0;
//				$($.sprintf("#sys_dslam_sw%s_vlan_%s_pvid", sw_num, sw_port)).val(pvid);
//				$($.sprintf("#sys_dslam_sw%s_vlan_%s_tag", sw_num, sw_port)).val(tag);
		
			}

			addWidgets();
			c.addSubmit({"onSubmit" : function() {
				var cmd = "";
				if (document.getElementById("mac_sec").checked) {
					cmd += $.sprintf("echo \"%s\" > /proc/sys/net/dslam_sw/sw%s/disable_learning; ", sw_port, sw_num)
				} else {
					cmd += $.sprintf("echo \"%s\" > /proc/sys/net/dslam_sw/sw%s/enable_learning; ", sw_port, sw_num)
				}
				cmd += $.sprintf("kdb set sys_dslam_sw%s_macs_p%s=\"%s\"", sw_num, sw_port, $($.sprintf("#td_sys_dslam_sw%s_mac_sec_%s_macs", sw_num, sw_port)).text());
				config.cmdExecute({"cmd" : cmd});
			}
			});





/*			
			config.cmdExecute({
				"cmd": "/etc/init.d/dslam_sw get_mac_table",
				"dataType" : "json",
				"callback": function(reply) {
					if (typeof(reply) != "object") {
						page.clearTab("mac_security");
						c = page.addContainer("mac_security");
						c.addTitle("Error read LUT!!!");
					} else {
//						page.clearTab("mac_security");
						page.addBr("mac_security");
						c = page.addContainer("mac_security");
						c.addTitle("MAC table");
						var count = 0;
						for (cur_port = 0; cur_port <= 26; cur_port++)
						$.each(reply["sw0"], function(mac, port) {
							if (port != "25")
							if (parseInt(port) == cur_port) {
								field = {
									"type" : "html",
									"str" : $.sprintf("<pre>%s</pre>", mac),
									"name" : count,
									"text"  : $.sprintf("Port %s (%s)", sw_port2port[$.sprintf("sw0p%s", port)],
									port_name[sw_port2port[$.sprintf("sw0p%s", port)]])
								}
								c.addWidget(field);
								count++;
							}
						});


						for (cur_port = 0; cur_port <= 26; cur_port++)
						$.each(reply["sw1"], function(mac, port) {
							if (port != "25")
							if (parseInt(port) == cur_port) {
								field = {
									"type" : "html",
									"str" : $.sprintf("<pre>%s</pre>", mac),
									"name" : count,
									"text"  : $.sprintf("Port %s (%s)", sw_port2port[$.sprintf("sw0p%s", port)],
									port_name[sw_port2port[$.sprintf("sw0p%s", port)]])
								}
								c.addWidget(field);
								count++;
							}
						});
					}
				}
			});

		}
	});
*/
/*
	page.addTab({
		"id": "IGMP_snooping",
		"name": "IGMP snooping",
		"func": function() {
			var c, field, row;
			c = page.addContainer("IGMP_snooping");
			c.addTitle("IGMP snooping");
			
		}
	});
*/
	page.generateTabs();
}
		