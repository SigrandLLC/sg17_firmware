Controllers['dynamic_ifaces'] = function() {
	var page = this.Page();
	
	page.addTab({
		"id": "dynamic_ifaces",
		"name": "Dynamic ifaces",
		"func": function() {
			var c, field;
			c = page.addContainer("dynamic_ifaces");
			c.addTitle("Add dynamic network interface");
			
			/* IDs of VLAN widgets */
			var vlanWidgetsIDs = new Array();
			
			/* add, if not exist, widgets for VLAN */
			var addVlanWidgets = function() {
				if (vlanWidgetsIDs.length != 0) return;
				
				/* phys_iface */
				var physIfaces = new Array();
				$.each(config.getParsed("sys_ifaces"), function(name, value) {
					if (value.search(/\w+\d+v\d+/) == -1) {
						physIfaces.push(value);
					}
				});
				
				vlanWidgetsIDs.push("phys_iface");
				field = { 
					"type": "select",
					"name": "phys_iface",
					"id": "phys_iface",
					"text": "Physical interface",
					"options": physIfaces
				};
				c.addWidget(field);
				
				/* vlan_id */
				vlanWidgetsIDs.push("vlan_id");
				field = { 
					"type": "text",
					"name": "vlan_id",
					"id": "vlan_id",
					"text": "VLAN ID"
				};
				c.addWidget(field);
			};
			
			/* remove, if exist, widgets for VLAN */
			var removeVlanWidgets = function() {
				if (vlanWidgetsIDs.length != 0) {
					$.each(vlanWidgetsIDs, function(num, value) {
						$("#" + value).parents("tr").remove();
					});
					vlanWidgetsIDs = new Array();
				}
			};
			
			field = { 
				"type": "select",
				"name": "iface_proto",
				"id": "iface_proto",
				"text": "Protocol",
				"descr": "Please select interface protocol",
				"options": {"bridge": "Bridge", "bonding": "Bonding", "vlan": "VLAN",
					"pppoe": "PPPoE", "pptp": "PPtP"},
				"onChange": function() {
					if ($("#iface_proto").val() == "vlan") {
						addVlanWidgets();
					} else {
						removeVlanWidgets();
					}
				}
			};
			c.addWidget(field);
			
			c.addSubmit({
				"submitName": "Add",
				"noSubmit": true,
				"onSubmit": function() {
					config.addIface({"proto": $("#iface_proto").val()});
				}
			});
			
			page.addBr("dynamic_ifaces");
			var c2 = page.addContainer("dynamic_ifaces");
			c2.addTitle("Delete dynamic network interface");
			
			var ifaces = new Array();
			$.each(config.getParsed("sys_ifaces"), function(name, value) {
				if (value.search(/\w+\d+v\d+/) != -1 || value.search(/eth|dsl|E1/) == -1) {
					ifaces.push(value);
				}
			});
			
			field = { 
				"type": "select",
				"name": "del_iface",
				"text": "Interface",
				"descr": "Interface to delete",
				"options": ifaces
			};
			c2.addWidget(field);
		
			c2.addSubmit({"submitName": "Delete"});
		}
	});
	
	page.generateTabs();
};
