Controllers['e1'] = function(iface, pcislot, pcidev) {
	var page = this.Page();
	page.setHelpPage("e1");
	page.setSubsystem($.sprintf("e1.%s.%s", pcislot, pcidev));
	
	page.addTab({
		"id": "e1",
		"name": "E1",
		"func": function() {
			var c, field, id;
			var options = {};
			
			c = page.addContainer("e1");
			
			/* set title */
			var moduleType = config.get($.sprintf("sys_pcitbl_s%s_iftype", pcislot));
			var moduleName = moduleType == config.getOEM("MR16G_DRVNAME") ?
				config.getOEM("MR16G_MODNAME") : config.getOEM("MR17G_MODNAME");
			c.addTitle($.sprintf("%s (%s%s%s, %s, %s %s) %s", iface, moduleName,
				config.getOEM("OEM_IFPFX"), config.get($.sprintf("sys_pcitbl_s%s_ifnum", pcislot)), 
				config.getCachedOutput($.sprintf("muxonly_%s", iface)) == 0
				? _("full capabilities") : _("multiplexing only"), _("slot"), pcislot - 2,
				_("settings")));
			
			/* IDs of *HDLC widgets */
			var hdlcWidgetsIDs = new Array();
			
			/* IDs of CISCO widgets */
			var ciscoWidgetsIDs = new Array();
			
			/* IDs of framed widgets */
			var framedWidgetsIDs = new Array();
			
			/* add, if not exist, widgets for HDLC and ETHER-HDLC protocol */
			var addHdlcWidgets = function() {
				if (hdlcWidgetsIDs.length != 0) return;
				
				/* widget after which insert this element */
				var protoWidget = $("#pcicfgProto").parents("tr");
				
				/* encoding */
				field = { 
					"type": "select",
					"name": $.sprintf("sys_pcicfg_s%s_%s_hdlc_enc", pcislot, pcidev),
					"id": "enc",
					"text": "Encoding",
					"options": "nrz nrzi fm-mark fm-space manchester"
				};
				c.addWidget(field, protoWidget);
				hdlcWidgetsIDs.push("enc");

				/* parity */
				field = { 
					"type": "select",
					"name": $.sprintf("sys_pcicfg_s%s_%s_hdlc_parity", pcislot, pcidev),
					"id": "parity",
					"text": "Parity",
					"options": "crc16-itu no-parity crc16 crc16-pr0 crc16-itu-pr0 crc32-itu"
				};
				c.addWidget(field, protoWidget);
				hdlcWidgetsIDs.push("parity");
			};
			
			/* add, if not exist, widgets for CISCO protocol */
			var addCiscoWidgets = function() {
				if (ciscoWidgetsIDs.length != 0) return;
				
				/* widget after which insert this element */
				var protoWidget = $("#pcicfgProto").parents("tr");
				
				/* interval */
				var interval;
				for (var i = 1; i <= 10; i++) {
					if (interval) interval += " " + i * 10;
					else interval = "" + i * 10;
				}
				field = { 
					"type": "select",
					"name": $.sprintf("sys_pcicfg_s%s_%s_cisco_int", pcislot, pcidev),
					"id": "cisco_int",
					"text": "Interval",
					"options": interval,
					"defaultValue": "10"
				};
				c.addWidget(field, protoWidget);
				ciscoWidgetsIDs.push("cisco_int");
				
				/* timeout */
				var to;
				for (var i = 1; i <= 20; i++) {
					if (to) to += " " + i * 5;
					else to = "" + i * 5;
				}				
				field = { 
					"type": "select",
					"name": $.sprintf("sys_pcicfg_s%s_%s_cisco_to", pcislot, pcidev),
					"id": "cisco_to",
					"text": "Timeout",
					"options": to,
					"defaultValue": "25"
				};
				c.addWidget(field, protoWidget);
				ciscoWidgetsIDs.push("cisco_to");
			};
			
			/* add, if not exist, widgets for framed mode */
			var addFramedWidgets = function() {
				var field;
				if (framedWidgetsIDs.length != 0) return;
				
				/* widget after which insert this element */
				var framWidget = $("#fram").parents("tr");
				
				/* add SMAP only for full capable interfaces */
				if (config.getCachedOutput($.sprintf("muxonly_%s", iface)) == 0) {
					var id = $.sprintf("sys_pcicfg_s%s_%s_smap", pcislot, pcidev);
					field = { 
						"type": "text",
						"name": id,
						"id": id,
						"text": "Slotmap",
						"descr": "example: 2-3,6-9,15-20",
						"validator": {"smap": true}
					};
					c.addWidget(field, framWidget);
					framedWidgetsIDs.push(id);
					
					/* subsystem can change slotmap value, so after request is performed, update it */
					options.onSuccess = function() {
						updateFields(id, true);
					};
				}
				
				field = { 
					"type": "checkbox",
					"name": $.sprintf("sys_pcicfg_s%s_%s_crc4", pcislot, pcidev),
					"id": "crc4",
					"text": "E1 CRC4 multiframe"
				};
				c.addWidget(field, framWidget);
				framedWidgetsIDs.push("crc4");
				
				field = { 
					"type": "checkbox",
					"name": $.sprintf("sys_pcicfg_s%s_%s_ts16", pcislot, pcidev),
					"id": "ts16",
					"text": "Use time slot 16",
					"onClick": onTS16Change,
					"defaultState": "checked"
				};
				c.addWidget(field, framWidget);
				framedWidgetsIDs.push("ts16");
			};
			
			/* remove, if exist, specified widgets */
			var removeWidgets = function(widgetsIDs) {
				if (widgetsIDs.length != 0) {
					$.each(widgetsIDs, function(num, value) {
						$("#" + value).parents("tr").remove();
					});
					
					/* remove all IDs from array */
					widgetsIDs.splice(0);
				}
			};
			
			/* handler to call when FRAM option changes */
			var onFramChange = function() {
				/* cancel updating for SMAP */
				delete options.onSuccess;
				
				/* remove all dynamic widgets */
				removeWidgets(framedWidgetsIDs);
				
				/* add widgets for framed mode */
				if ($("#fram").attr("checked")) {
					addFramedWidgets();
					onTS16Change();
				}
			};
			
			/* handler to call when TS16 options changes */
			var onTS16Change = function() {
				/* remove CAS options */
				$("#cas").parents("tr").remove();
				
				/* if TS16 option is not active, add CAS option */
				if (! $("#ts16").attr("checked")) {
					var field = { 
						"type": "checkbox",
						"name": $.sprintf("sys_pcicfg_s%s_%s_cas", pcislot, pcidev),
						"id": "cas",
						"text": "E1 CAS multiframe"
					};
					c.addWidget(field, $("#ts16").parents("tr"));
				}
			};
			
			/* handler to call when PCICFG PROTO option changes */
			var onPcicfgProtoChange = function() {
				/* remove all dynamic widgets */
				removeWidgets(hdlcWidgetsIDs);
				removeWidgets(ciscoWidgetsIDs);
				
				switch ($("#pcicfgProto").val()) {
					/* add widgets for *HDLC protocol */
					case "hdlc":
					case "hdlc-eth":
						addHdlcWidgets();
						break;
					/* add widgets for CISCO protocol */
					case "cisco":
						addCiscoWidgets();
						break;
				}
				
				/* set network interface proto */
				$("#ifaceProto").val($("#pcicfgProto").val() == "hdlc-eth" ? "ether" : "hdlc");
			};
			
			/* add widgets that always present */
			
			field = { 
				"type": "checkbox",
				"name": $.sprintf("sys_mux_%s_mxen", iface),
				"text": "Enable multiplexing",
				"descr": "Enable multiplexing on this interface",
				"tip": "This option is equivalent to MXEN on a multiplexing page."
			};
			c.addWidget(field);
			
			field = { 
				"type": "checkbox",
				"name": $.sprintf("sys_pcicfg_s%s_%s_fram", pcislot, pcidev),
				"id": "fram",
				"text": "E1 framed mode",
				"onClick": onFramChange
			};
			c.addWidget(field);
			
			field = { 
				"type": "checkbox",
				"name": $.sprintf("sys_pcicfg_s%s_%s_lhaul", pcislot, pcidev),
				"text": "E1 long haul mode"
			};
			c.addWidget(field);
			
			field = { 
				"type": "select",
				"name": $.sprintf("sys_pcicfg_s%s_%s_lcode", pcislot, pcidev),
				"text": "E1 HDB3/AMI line code",
				"options": {"1": "HDB3", "0": "AMI"}
			};
			c.addWidget(field);
			
			field = { 
				"type": "checkbox",
				"name": $.sprintf("sys_pcicfg_s%s_%s_llpb", pcislot, pcidev),
				"text": "Local Loopback",
				"descr": "Enable E1 Local Loopback"
			};
			c.addWidget(field);
			
			field = { 
				"type": "checkbox",
				"name": $.sprintf("sys_pcicfg_s%s_%s_rlpb", pcislot, pcidev),
				"text": "Remote Loopback",
				"descr": "Enable E1 Remote Loopback"
			};
			c.addWidget(field);
			
			/* add widgets for full capable interface */
			if (config.getCachedOutput($.sprintf("muxonly_%s", iface)) == 0) {
				/* protocol select */
				field = { 
					"type": "select",
					"name": $.sprintf("sys_pcicfg_s%s_%s_proto", pcislot, pcidev),
					"id": "pcicfgProto",
					"text": "HDLC protocol",
					"onChange": onPcicfgProtoChange,
					"options": {
						"hdlc": "HDLC", "hdlc-eth": "ETHER-HDLC", "cisco": "CISCO-HDLC", "fr": "FR",
						"ppp": "PPP", "x25": "X25"
					}
				};
				c.addWidget(field);
				
				field = { 
					"type": "checkbox",
					"name": $.sprintf("sys_pcicfg_s%s_%s_clk", pcislot, pcidev),
					"text": "E1 external transmit clock"
				};
				c.addWidget(field);
				
				field = { 
					"type": "select",
					"name": $.sprintf("sys_pcicfg_s%s_%s_hcrc", pcislot, pcidev),
					"text": "CRC",
					"descr": "Select HDLC CRC length",
					"options": {"0": "CRC32", "1": "CRC16"}
				};
				c.addWidget(field);
				
				field = { 
					"type": "select",
					"name": $.sprintf("sys_pcicfg_s%s_%s_fill", pcislot, pcidev),
					"text": "Fill",
					"descr": "Select HDLC fill byte value",
					"options": {"0": "FF", "1": "7E"}
				};
				c.addWidget(field);
				
				field = { 
					"type": "select",
					"name": $.sprintf("sys_pcicfg_s%s_%s_inv", pcislot, pcidev),
					"text": "Inversion",
					"descr": "Select HDLC inversion mode",
					"options": {"0": "off", "1": "on"}
				};
				c.addWidget(field);
				
				/*
				 * This key (interface protocol) is set in the /etc/init.d/e1, but we need
				 * it in web before current request will be completed.
				 */
				field = {
					"type": "hidden",
					"name": $.sprintf("sys_iface_%s_proto", iface),
					"id": "ifaceProto",
					"defaultValue":
						$("#pcicfgProto").val() == "hdlc-eth" ? "ether" : "hdlc"
				};
				c.addWidget(field);
			}
			
			c.addSubmit(options);
			
			onFramChange();
			onTS16Change();
			onPcicfgProtoChange();
		}
	});
	
	page.generateTabs();
};
