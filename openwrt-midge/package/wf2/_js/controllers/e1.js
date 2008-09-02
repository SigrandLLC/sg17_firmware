Controllers['e1'] = function(iface, pcislot, pcidev) {
	var page = this.Page();
	//page.setSubsystem("e1." + pcislot + "." + pcidev);
	
	page.addTab({
		"id": "e1",
		"name": "E1",
		"func": function() {
			var c, field;
			c = page.addContainer("e1");
			c.addTitle(iface + " (module " + config.getOEM("MR16G_MODNAME") + ") settings");
			
			var addHdlcWidgets = function() {
				var options = new Object();
				var encodings = "nrz nrzi fm-mark fm-space manchester";
				$.each(encodings.split(" "), function(num, value) {
					options[value] = value;
				});
				field = { 
					"type": "select",
					"name": "sys_pcicfg_s" + pcislot + "_" + pcidev + "_enc",
					"id": "sys_pcicfg_s" + pcislot + "_" + pcidev + "_enc",
					"text": "Encoding",
					"options": options
				};
				c.addWidget(field);
			};
			
			var selectProtoId = "sys_pcicfg_s" + pcislot + "_" + pcidev + "_proto";
			field = { 
				"type": "select",
				"name": "sys_pcicfg_s" + pcislot + "_" + pcidev + "_proto",
				"id": selectProtoId,
				"text": "HDLC protocol",
				"onChange": function() {
					if ($("#" + selectProtoId).val() == "hdlc" || $("#" + selectProtoId).val() == "hdlc-eth") {
						if ($("#sys_pcicfg_s" + pcislot + "_" + pcidev + "_enc").size() == 0) {
							addHdlcWidgets();
						}
					} else {
						if ($("#sys_pcicfg_s" + pcislot + "_" + pcidev + "_enc").size() != 0) {
							$("#sys_pcicfg_s" + pcislot + "_" + pcidev + "_enc").parents("tr").remove();
						}
					}
				},
				"options": {"hdlc": "HDLC", "hdlc-eth": "ETHER-HDLC", "cisco": "CISCO-HDLC", "fr": "FR",
					"ppp": "PPP", "x25": "X25"}
			};
			c.addWidget(field);
			
			//var proto = config.get("sys_pcicfg_s" + pcislot + "_" + pcidev + "_proto");
			var proto = $("#" + selectProtoId).val();
			if (proto == "hdlc" || proto == "hdlc-eth") {
				addHdlcWidgets();
			}
			
			c.addSubmit();
		}
	});
	
	page.generateTabs();
};