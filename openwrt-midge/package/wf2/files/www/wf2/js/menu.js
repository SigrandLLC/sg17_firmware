function generateMenu() {
	$("#menu").empty();
	addItem("System", "Info", "info");
	addItem("System", "Webface", "webface");
	addItem("System", "General", "general");
	addItem("System", "Security", "security");
	addItem("System", "DNS", "dns");
	addItem("System", "Time", "time");
	addItem("System", "Logging", "logging");
	addItem("System", "Tools", "tools");
	addItem("System", "Reboot", "reboot");
	addItem("System", "Configuration", "cfg");
	addItem("Network", "Firewall", "fw");
	addItem("Network:Dynamic interfaces", "Manage", "dynamic_ifaces");
	addItem("Hardware", "Switch", "adm5120sw");
	addItem("Services", "DHCP server", "dhcp");
	addItem("Services", "DNS server", "dns_server");

	/* if we have support for linkdeps */
	if (config.getCachedOutput("linkdeps") == "1") {
		addItem("Hardware", "Linkdeps", "linkdeps");
	}

	/* if we have interfaces with multiplexing support, add item to the menu */
	if (config.getParsed("sys_mux_ifaces").length > 0) {
		addItem("Hardware", "Multiplexing", "multiplexing");
	}
	
	/* Add VoIP controller */
	if (config.get("sys_voip_present") == "1") {
		/* cache channels info */
		config.runCmd("/bin/cat /proc/driver/sgatab/channels");

		addItem("Hardware", "VoIP", "voip");
	}
	
	/* get array of PCI slots */
	var slots = config.getParsed("sys_pcitbl_slots");
	
	/* generate list of SHDSL/E1/RS232 interfaces */
	$.each(slots, function(num, pcislot) {
		var type = config.get($.sprintf("sys_pcitbl_s%s_iftype", pcislot));
		var ifaces = config.getParsed($.sprintf("sys_pcitbl_s%s_ifaces", pcislot));
		
		/* save interfaces info */
		config.saveData(type, ifaces);
		
		/* go through ifaces of this slot */
		$.each(ifaces, function(num, iface) {
			switch (type) {
				/* SHDSL */
				case config.getOEM("MR16H_DRVNAME"):
				case config.getOEM("MR17H_DRVNAME"):
					if (type == config.getOEM("MR17H_DRVNAME")) {
						var confPath = $.sprintf("%s/%s/sg17_private", config.getOEM("sg17_cfg_path"), iface);
						
						config.runCmd($.sprintf("/bin/cat %s/chipver", confPath));
						config.runCmd($.sprintf("/bin/cat %s/pwr_source", confPath));
					}
					
					addItem("Hardware:SHDSL", iface, "dsl", [iface, pcislot, num]);
					break;
				
				/* E1 */
				case config.getOEM("MR16G_DRVNAME"):
				case config.getOEM("MR17G_DRVNAME"):
					config.runCmd(
						$.sprintf("[ -f /sys/class/net/%s/hw_private/muxonly ] && cat /sys/class/net/%s/hw_private/muxonly || echo -n 0", iface, iface),
						$.sprintf("muxonly_%s", iface));
					addItem("Hardware:E1", iface, "e1", [iface, pcislot, num]);
					break;
					
				/* RS232 */
				case config.getOEM("MR17S_DRVNAME"):
					addItem("Hardware:RS232", iface, "rs232", [iface, pcislot, num]);
					break;
			}
		});
	});
	
	/* generate list of network interfaces */
	var ifaces = config.getParsed("sys_ifaces");
	$(ifaces).each(function(name, iface) {
		/* add dynamic interfaces */
		if (iface.search(/\w+\d+v\d+/) != -1 || iface.search(/eth|dsl|E1/) == -1) {
			addItem("Network:Dynamic interfaces", iface, "iface", [iface]);
		/* add physical interfaces */
		} else {
			addItem("Network:Interfaces", iface, "iface", [iface]);
		}
	});
	
	/* generate menu */
	$("#menu").treeview({
		"unique": true,
		"collapsed": true,
		"persist": "cookie"
	});
}
