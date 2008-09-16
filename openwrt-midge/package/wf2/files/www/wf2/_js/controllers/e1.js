Controllers['e1'] = function(iface, pcislot, pcidev) {
	var page = this.Page();
	page.setHelp("e1");
	page.setSubsystem($.sprintf("e1.%s.%s", pcislot, pcidev));
	
	page.addTab({
		"id": "e1",
		"name": "E1",
		"func": function() {
			var c, field, id;
			c = page.addContainer("e1");
			c.addTitle($.sprintf("%s (module %s) settings", iface, config.getOEM("MR16G_MODNAME")));
			
			/* IDs of *HDLC widgets */
			var hdlcWidgetsIDs = new Array();
			
			/* add, if not exist, widgets for HDLC and ETHER-HDLC protocol */
			var addHdlcWidgets = function() {
				if (hdlcWidgetsIDs.length != 0) return;
				
				/* widget after which insert this element */
				var protoWidget = $($.sprintf("#sys_pcicfg_s%s_%s_proto", pcislot, pcidev)).parents("tr");
				
				/* encoding */
				var id = $.sprintf("sys_pcicfg_s%s_%s_hdlc_enc", pcislot, pcidev);
				var encodings = "nrz nrzi fm-mark fm-space manchester";
				hdlcWidgetsIDs.push(id);
				field = { 
					"type": "select",
					"name": id,
					"id": id,
					"text": "Encoding",
					"options": encodings
				};
				c.addWidget(field, protoWidget);

				/* parity */
				id = $.sprintf("sys_pcicfg_s%s_%s_hdlc_parity", pcislot, pcidev);
				var parity = "crc16-itu no-parity crc16 crc16-pr0 crc16-itu-pr0 crc32-itu";
				hdlcWidgetsIDs.push(id);
				field = { 
					"type": "select",
					"name": id,
					"id": id,
					"text": "Parity",
					"options": parity
				};
				c.addWidget(field, protoWidget);
			};
			
			/* remove, if exist, widgets for HDLC and ETHER-HDLC protocol */
			var removeHdlcWidgets = function() {
				if (hdlcWidgetsIDs.length != 0) {
					$.each(hdlcWidgetsIDs, function(num, value) {
						$("#" + value).parents("tr").remove();
					});
					hdlcWidgetsIDs = new Array();
				}
			};
			
			/* IDs of CISCO widgets */
			var ciscoWidgetsIDs = new Array();
			
			/* add, if not exist, widgets for CISCO protocol */
			var addCiscoWidgets = function() {
				if (ciscoWidgetsIDs.length != 0) return;
				
				/* widget after which insert this element */
				var protoWidget = $($.sprintf("#sys_pcicfg_s%s_%s_proto", pcislot, pcidev)).parents("tr");
				
				/* interval */
				var id = $.sprintf("sys_pcicfg_s%s_%s_cisco_int", pcislot, pcidev);
				var interval;
				for (var i = 1; i <= 10; i++) {
					if (interval) interval += " " + i * 10;
					else interval = "" + i * 10;
				}
				ciscoWidgetsIDs.push(id);
				field = { 
					"type": "select",
					"name": id,
					"id": id,
					"text": "Interval",
					"options": interval,
					"defaultValue": "10"
				};
				c.addWidget(field, protoWidget);
				
				/* timeout */
				id = $.sprintf("sys_pcicfg_s%s_%s_cisco_to", pcislot, pcidev);
				var to;
				for (var i = 1; i <= 20; i++) {
					if (to) to += " " + i * 5;
					else to = "" + i * 5;
				}
				ciscoWidgetsIDs.push(id);
				field = { 
					"type": "select",
					"name": id,
					"id": id,
					"text": "Timeout",
					"options": to,
					"defaultValue": "25"
				};
				c.addWidget(field, protoWidget);
			};
			
			/* remove, if exist, widgets for CISCO protocol */
			var removeCiscoWidgets = function() {
				if (ciscoWidgetsIDs.length != 0) {
					$.each(ciscoWidgetsIDs, function(num, value) {
						$("#" + value).parents("tr").remove();
					});
					ciscoWidgetsIDs = new Array();
				}
			};
			
			/* IDs of framed widgets */
			var framedWidgetsIDs = new Array();
			
			/* add, if not exist, widgets for framed mode */
			var addFramedWidgets = function() {
				var id;
				if (framedWidgetsIDs.length != 0) return;
				
				/* widget after which insert this element */
				var framWidget = $($.sprintf("#sys_pcicfg_s%s_%s_fram", pcislot, pcidev)).parents("tr");
				
				id = $.sprintf("sys_pcicfg_s%s_%s_ts16", pcislot, pcidev);
				framedWidgetsIDs.push(id);
				field = { 
					"type": "checkbox",
					"name": id,
					"id": id,
					"text": "Use time slot 16"
				};
				c.addWidget(field, framWidget);
				
				id = $.sprintf("sys_pcicfg_s%s_%s_smap", pcislot, pcidev);
				framedWidgetsIDs.push(id);
				field = { 
					"type": "text",
					"name": id,
					"id": id,
					"text": "Slotmap",
					"descr": "example: 2-3,6-9,15-20"
				};
				c.addWidget(field, framWidget);
				
				id = $.sprintf("sys_pcicfg_s%s_%s_crc4", pcislot, pcidev);
				framedWidgetsIDs.push(id);
				field = { 
					"type": "checkbox",
					"name": id,
					"id": id,
					"text": "E1 CRC4 multiframe"
				};
				c.addWidget(field, framWidget);
				
				id = $.sprintf("sys_pcicfg_s%s_%s_cas", pcislot, pcidev);
				framedWidgetsIDs.push(id);
				field = { 
					"type": "checkbox",
					"name": id,
					"id": id,
					"text": "E1 CAS multiframe"
				};
				c.addWidget(field, framWidget);
			};
			
			/* remove, if exist, widgets for framed mode */
			var removeFramedWidgets = function() {
				if (framedWidgetsIDs.length != 0) {
					$.each(framedWidgetsIDs, function(num, value) {
						$("#" + value).parents("tr").remove();
					});
					framedWidgetsIDs = new Array();
				}
			};

			/* protocol select */
			id = $.sprintf("sys_pcicfg_s%s_%s_proto", pcislot, pcidev);
			field = { 
				"type": "select",
				"name": id,
				"id": id,
				"text": "HDLC protocol",
				"onChange": function() {
					var id = $.sprintf("sys_pcicfg_s%s_%s_proto", pcislot, pcidev);
					
					/* if selected *HDLC protocol */
					if ($("#" + id).val() == "hdlc" || $("#" + id).val() == "hdlc-eth") {
						/* add widgets for *HDLC protocol */
						addHdlcWidgets();
						removeCiscoWidgets();
					/* if selected CISCO protocol */
					} else if ($("#" + id).val() == "cisco") {
						addCiscoWidgets();
						removeHdlcWidgets();
					/* if selected another protocol */
					} else {
						removeCiscoWidgets();
						removeHdlcWidgets();
					}
				},
				"options": {"hdlc": "HDLC", "hdlc-eth": "ETHER-HDLC", "cisco": "CISCO-HDLC", "fr": "FR",
					"ppp": "PPP", "x25": "X25"}
			};
			c.addWidget(field);
			
			/* depending on protocol add specific widgets */
			var proto = $($.sprintf("#sys_pcicfg_s%s_%s_proto", pcislot, pcidev)).val();
			if (proto == "hdlc" || proto == "hdlc-eth")	addHdlcWidgets();
			else if (proto == "cisco") addCiscoWidgets();
			
			/* mode */
			id = $.sprintf("sys_pcicfg_s%s_%s_fram", pcislot, pcidev);
			field = { 
				"type": "checkbox",
				"name": id,
				"id": id,
				"text": "E1 framed mode",
				"onClick": function() {
					var id = $.sprintf("sys_pcicfg_s%s_%s_fram", pcislot, pcidev);
					
					if ($("#" + id).attr("checked")) addFramedWidgets();
					else removeFramedWidgets();
				}
			};
			c.addWidget(field);
			
			var framed = $($.sprintf("#sys_pcicfg_s%s_%s_fram", pcislot, pcidev)).attr("checked");
			if (framed) addFramedWidgets();
			
			field = { 
				"type": "checkbox",
				"name": $.sprintf("sys_pcicfg_s%s_%s_clk", pcislot, pcidev),
				"text": "E1 external transmit clock"
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
			
			/*
			 * subsystem can change slotmap value, so after request is performed, update it.
			 */
			c.addSubmit({
				"onSuccess": function() {
					updateField($.sprintf("sys_pcicfg_s%s_%s_smap", pcislot, pcidev));
				}
			});
		}
	});
	
	page.generateTabs();
};
