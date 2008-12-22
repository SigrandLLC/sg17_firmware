Controllers.linkdeps = function() {
	var page = this.Page();
	page.setSubsystem("linkdeps");
	
	page.addTab({
		"id": "linkdeps",
		"name": "Linkdeps",
		"func": function() {
			var c, field;
			c = page.addContainer("linkdeps");
			
			var list = c.createList({
				"tabId": "linkdeps",
				"header": ["Link-master", "Link-slave"],
				"varList": ["link_master", "link_slave"],
				"listItem": "sys_linkdeps_",
				"addMessage": "Add link dependency",
				"editMessage": "Edit link dependency",
				"listTitle": "Link dependency"
			});
			
			field = { 
				"type": "select",
				"name": "link_master",
				"text": "Link-master",
				"descr": "If link of this interface is down, link of link-slave interface will be down too.",
				"options": function() {
					var ifaces = [];
					
					/* only SHDSL interfaces can be link-master */
					$(config.getParsed("sys_ifaces")).each(function(name, iface) {
						if (iface.search(/dsl/) != -1) {
							ifaces.push(iface);
						}
					});
					
					return ifaces;
				}()
			};
			list.addWidget(field);
			
			field = { 
				"type": "select",
				"name": "link_slave",
				"text": "Link-slave",
				"descr": "Interface to down if link-master is down.",
				"validator": {"required": true},
				"options": function() {
					var ifaces = [];
					
					/*
					 * only ethernet and E1 v16 can be link-slave,
					 * and each interface can be link-slave only one time.
					 */
					
					/* list of interfaces which are already link-slave */
					var linkSlaves = "";
					$.each(config.getParsed("sys_linkdeps_*"), function(num, linkdep) {
						linkSlaves += linkdep.link_slave + " ";
					});
					
					/* add ethernet interfaces */
					$(config.getParsed("sys_ifaces")).each(function(name, iface) {
						if (linkSlaves.search(iface) == -1 && iface.search(/eth/) != -1) {
							ifaces.push(iface);
						}
					});
					
					/* add E1 v16 interfaces */
					var e1Ifaces = config.getData(config.getOEM("MR16G_DRVNAME"));
					if (e1Ifaces) {
						$.each(e1Ifaces, function(num, iface) {
							if (linkSlaves.search(iface) == -1) {
								ifaces.push(iface);
							}
						});
					}
					
					return ifaces;
				}()
			};
			list.addWidget(field);
			
			list.generateList();
		}
	});
	
	page.generateTabs();
};
