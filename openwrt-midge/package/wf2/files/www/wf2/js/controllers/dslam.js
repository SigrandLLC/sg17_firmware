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
		c.addTitle(iface + " status");

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

			field = {
				"type": "html",
				"name": "actualClockMode",
				"text": "Actual clock mode",
				"str": status.link.clkmode
			};
			c.addWidget(field);

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
/*
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
*/
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
		c.addTitle(iface + " settings");
		c.setSubsystem($.sprintf("dslam_dsl.%s.%s", pcislot, pcidev));


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
/*
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
*/
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



		field = {
			"type": "select",
			"name": $.sprintf("sys_pcicfg_s%s_%s_pwron", pcislot, pcidev),
			"id": "power",
			"text": "Power",
			"descr": "DSL power feeding mode.",
			"options": {"pwroff": "off", "pwron": "on"},
		};
		c.addWidget(field);
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
		field = {
			"type": "select",
			"name": $.sprintf("sys_pcicfg_s%s_%s_fill", pcislot, pcidev),
			"text": "Fill",
			"descr": "DSL fill byte value.",
			"options": {"fill_ff": "FF", "fill_7e": "7E"}
		};
		c.addWidget(field);

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

