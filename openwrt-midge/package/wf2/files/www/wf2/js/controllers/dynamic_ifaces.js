Controllers.dynamic_ifaces = function() {
	var page = this.Page();
	page.setHelpPage("ifaces");
	
	page.addTab({
		"id": "dynamic_ifaces",
		"name": "Dynamic ifaces",
		"func": function() {
			/* update list of ifaces for deletion */
			var setIfaces = function() {
				var ifaces = new Array();
				$.each(config.getParsed("sys_ifaces"), function(name, value) {
					if (value.search(/\w+\d+v\d+/) != -1 || value.search(/eth|dsl|E1/) == -1) {
						ifaces.push(value);
					}
				});
				
				$("#del_iface").setOptionsForSelect(ifaces);
			};
			
			
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
					"text": "Physical interface",
					"options": physIfaces
				};
				c.addWidget(field);
				
				/* vlan_id */
				vlanWidgetsIDs.push("vlan_id");
				field = { 
					"type": "text",
					"name": "vlan_id",
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
				"options": {"bridge": "Bridge", "bonding": "Bonding", "vlan": "VLAN"},
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
					var parameters = {"proto": $("#iface_proto").val()};
					
					/* set parameters for VLAN interface */
					if ($("#iface_proto").val() == "vlan") {
						/* set real (in system) interface name */
						parameters.real =
							$.sprintf("%s.%s", $("#phys_iface").val(), $("#vlan_id").val());
							
						/* set interface name, used in KDB (and webface) */
						parameters.iface =
							$.sprintf("%sv%s", $("#phys_iface").val(), $("#vlan_id").val());
						
						/* set dependOn interface and VLAN ID */
						parameters.dependOn = $("#phys_iface").val();
						parameters.vlanId = $("#vlan_id").val();
					}
					var newIface = config.addIface(parameters);
					
					setIfaces();

                    if ($("#iface_proto").val() == "bridge" || $("#iface_proto").val() == "bonding") {
                        Controllers.wizard(newIface);
                    }
				}
			});
			
			page.addBr("dynamic_ifaces");
			var c2 = page.addContainer("dynamic_ifaces");
			c2.addTitle("Delete dynamic network interface");
			
			field = { 
				"type": "select",
				"name": "del_iface",
				"id": "del_iface",
				"text": "Interface",
				"descr": "Interface to delete",
				"options": ""
			};
			c2.addWidget(field);
			
			setIfaces();
		
			c2.addSubmit({
				"submitName": "Delete",
				"noSubmit": true,
				"onSubmit": function() {
					if ($("#del_iface").val() != null) {
						config.delIface($("#del_iface").val());
						setIfaces();
                        return true;
					} else {
                        return false;
                    }
				}
			});
		}
	});
	
	page.generateTabs();
};

/*
 * Controller for fast & simple bridge setup.
 */
Controllers.wizard = function(iface) {
    var page = this.Page();
	page.setSubsystem("network." + iface);

	page.addTab({
		"id": "bridge",
		"name": "Bridge",
		"func": function() {
			var c, field;
			c = page.addContainer("bridge");

			if (iface.search("br") != -1) {
				c.addTitle("Bridge setup wizard");
			} else {
				c.addTitle("Bonding setup wizard");
			}

			var showMethod = function() {
				/* remove method's widgets */
				$(".tmpMethod").parents("tr").remove();

				if ($("#method").val() == "static") {
					field = {
						"type": "text",
						"name": $.sprintf("sys_iface_%s_ipaddr", iface),
						"text": "Static address",
						"descr": "IP address.",
						"tip": "e.g., 192.168.2.100",
						"validator": {"required": true, "ipAddr": true},
						"cssClass": "tmpMethod"
					};
					c.addWidget(field);

					field = {
						"type": "text",
						"name": $.sprintf("sys_iface_%s_netmask", iface),
						"text": "Netmask",
						"descr": "Network mask.",
						"tip": "e.g., 255.255.255.0",
						"validator": {"required": true, "netmask": true},
						"cssClass": "tmpMethod"
					};
					c.addWidget(field);

					field = {
						"type": "text",
						"name": $.sprintf("sys_iface_%s_broadcast", iface),
						"text": "Broadcast",
						"descr": "Broadcast address.",
						"tip": "e.g., 192.168.2.255",
						"validator": {"ipAddr": true},
						"cssClass": "tmpMethod"
					};
					c.addWidget(field);

					field = {
						"type": "text",
						"name": $.sprintf("sys_iface_%s_gateway", iface),
						"text": "Gateway",
						"descr": "Default gateway.",
						"tip": "e.g., 192.168.2.1",
						"validator": {"ipAddr": true},
						"cssClass": "tmpMethod"
					};
					c.addWidget(field);
				}
			};

			field = {
				"type": "text",
				"name": $.sprintf("sys_iface_%s_desc", iface),
				"text": "Description",
				"descr": "Description of interface."
			};
			c.addWidget(field);

			/* we setup bridge */
			if (iface.search("br") != -1) {
				field = {
					"type": "text",
					"name": $.sprintf("sys_iface_%s_br_ifaces", iface),
					"id": "ifaces",
					"text": "Bridge interfaces",
					"descr": "Interfaces for bridge separated by space.",
					"tip": "<b>Example:</b> eth0 eth1 dsl0<br><b>Note:</b> You can use only" +
							" Ethernet-like interfaces, like ethX, dslX<br><b>Note:</b> Interfaces should" +
							" be enabled, but <b>auto</b> should be switched <b>off</b>.",
					"validator": {"required": true}
				};
				c.addWidget(field);
			/* we setup bonding */
			} else {
				field = {
					"type": "text",
					"name": $.sprintf("sys_iface_%s_bond_ifaces", iface),
					"id": "ifaces",
					"text": "Bonding interfaces",
					"descr": "Interfaces for bonding separated by space.",
					"tip": "<b>Example:</b>eth0 eth1 dsl0<br><b>Note:</b>You can use only Ethernet-like" +
							" interfaces, like ethX, dslX, bondX<br><b>Note:</b> Interfaces should be" +
							" enabled, but <b>auto</b> should be switched <b>off</b>.",
					"validator": {"required": true}
				};
				c.addWidget(field);
			}

			field = {
				"type": "checkbox",
				"name": $.sprintf("sys_iface_%s_enabled", iface),
				"text": "Enabled",
				"descr": "If set, interface can be start on boot or by another interface."
			};
			c.addWidget(field);

			field = {
				"type": "checkbox",
				"name": $.sprintf("sys_iface_%s_auto", iface),
				"text": "Auto",
				"descr": "If set and interface is enabled, it will be start on boot."
			};
			c.addWidget(field);

			field = {
				"type": "select",
				"name": $.sprintf("sys_iface_%s_method", iface),
				"id": "method",
				"text": "Method",
				"descr": "Method of setting IP address.",
				"options": {"none": "None", "static": "Static address", "zeroconf": "Zero Configuration",
							"dynamic": "Dynamic address"},
				"onChange": showMethod
			};
			c.addWidget(field);

			/* set auto=0 enabled=1 for depending interfaces */
			var additionalKeys = [];
			c.addSubmit({
				"additionalKeys": additionalKeys,
				"preSubmit": function() {
					$.each($("#ifaces").val().split(" "),
						function(num, value) {
							$.addObjectWithProperty(additionalKeys, $.sprintf("sys_iface_%s_auto", value), "0");
							$.addObjectWithProperty(additionalKeys, $.sprintf("sys_iface_%s_enabled", value), "1");
						}
					);
				}
			});
		}
	});

	page.generateTabs();
};
