Controllers['multiplexing'] = function() {
	var page = this.Page();
	page.setHelp("multiplexing");
	page.setSubsystem("mux");
	
	page.addTab({
		"id": "multiplexing",
		"name": "Multiplexing",
		"func": function() {
			var c, field, id;
			c = page.addContainer("multiplexing");
			c.addTitle("Multiplexing", 10);
			
			c.addTableHeader("DEV MXEN CLKM CLKAB CLKR RLINE TLINE RFS TFS MXRATE/MXSMAP");
			
			/* enables/disables CLKR field depending on CLKM value */
			var onMuxChange = function(iface) {				
				if ($($.sprintf("#sys_mux_%s_clkm", iface)).val() == "0") {
					$($.sprintf("#sys_mux_%s_clkr", iface)).attr("readonly", true);
				} else {
					$($.sprintf("#sys_mux_%s_clkr", iface)).removeAttr("readonly");
				}
			};
			
			$.each(config.getParsed("sys_mux_ifaces", true), function(num, iface) {
				var row = c.addTableRow();
				
				field = {
					"type": "html",
					"name": iface,
					"str": iface
				};
				c.addTableWidget(field, row);
				
				field = { 
					"type": "checkbox",
					"name": $.sprintf("sys_mux_%s_mxen", iface),
					"tip": "Enable multiplexing on this interface"
				};
				c.addTableWidget(field, row);
				
				id = $.sprintf("sys_mux_%s_clkm", iface);
				field = { 
					"type": "select",
					"name": id,
					"id": id,
					"options": {"0": "clock-slave", "1": "clock-master"},
					"tip": "Select interface mode: <i>clock-master</i> or <i>clock-slave</i>",
					"onChange": function() {
						onMuxChange(iface);
					}
				};
				c.addTableWidget(field, row);
				
				field = { 
					"type": "select",
					"name": $.sprintf("sys_mux_%s_clkab", iface),
					"options": {"0": "A", "1": "B"},
					"tip": "Select interface clock domain: <i>A</i> or <i>B</i>"
				};
				c.addTableWidget(field, row);
				
				id = $.sprintf("sys_mux_%s_clkr", iface);
				field = { 
					"type": "select",
					"name": id,
					"id": id,
					"options": {"0": "local", "1": "remote"},
					"tip": "Select clock source: <i>remote</i> or <i>local</i> (for <i>clock-master</i> interface only)"
				};
				c.addTableWidget(field, row);
				
				field = { 
					"type": "text",
					"name": $.sprintf("sys_mux_%s_rline", iface),
					"defaultValue": "0",
					"tip": "Enter rline number (<i>0-15</i>)"
				};
				c.addTableWidget(field, row);
				
				field = { 
					"type": "text",
					"name": $.sprintf("sys_mux_%s_tline", iface),
					"defaultValue": "0",
					"tip": "Enter tline number (<i>0-15</i>)"
				};
				c.addTableWidget(field, row);
				
				field = { 
					"type": "text",
					"name": $.sprintf("sys_mux_%s_rfs", iface),
					"defaultValue": "0",
					"tip": "Enter recieve frame start number (<i>0-255</i>)"
				};
				c.addTableWidget(field, row);
				
				field = { 
					"type": "text",
					"name": $.sprintf("sys_mux_%s_tfs", iface),
					"defaultValue": "0",
					"tip": "Enter transmit frame start number (<i>0-255</i>)"
				};
				c.addTableWidget(field, row);
				
				var rate;
				var tip;
				if (iface.search("dsl") != -1) {
					rate = "mxrate";
					tip = "Enter <i>mxrate</i> for DSL interface. <i>mxrate</i> is a number of time-slots (e.g., <i>12</i>).";
				} else {
					rate = "mxsmap";
					tip = "Enter <i>mxsmap</i> for E1 interface. <i>mxsmap</i> is a map of time-slots (e.g., <i>0-31</i>). This value can be changed after saving.";
				}
				
				id = $.sprintf("sys_mux_%s_%s", iface, rate);
				field = { 
					"type": "text",
					"name": id,
					"id": id,
					"defaultValue": "0",
					"tip": tip
				};
				c.addTableWidget(field, row);
				
				onMuxChange(iface);
			});
			
			c.addSubmit({
				"onSubmit": function() {
					/* remove previous output */
					$(".cmdOutput").remove();
				},
				"onSuccess": function() {
					/* MXSMAP can be cnanged by system */
					$.each(config.getParsed("sys_mux_ifaces", true), function(num, iface) {
						if (iface.search("E1") != -1) {
							updateField($.sprintf("sys_mux_%s_mxsmap", iface));
						}
					});
					/* execute command */
					c.addConsoleToForm("/sbin/mxconfig --check");
				}
			});

			c.addConsoleToForm("/sbin/mxconfig --check");
		}
	});
	
	page.generateTabs();
};
