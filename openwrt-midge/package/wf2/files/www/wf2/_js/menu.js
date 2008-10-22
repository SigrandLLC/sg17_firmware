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
	addItem("Network", "Firewall", "fw");
	
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
	
	/* generate list of SHDSL interfaces */
	$.each(slots, function(num, pcislot) {
		var type = config.get($.sprintf("sys_pcitbl_s%s_iftype", pcislot));
		if (type != config.getOEM("MR16H_DRVNAME") && type != config.getOEM("MR17H_DRVNAME")) {
			return true;
		}
		var ifaces = config.getParsed($.sprintf("sys_pcitbl_s%s_ifaces", pcislot));
		$.each(ifaces, function(num, iface) {
			if (type == config.getOEM("MR17H_DRVNAME")) {
				var confPath = $.sprintf("%s/%s/sg17_private", config.getOEM("sg17_cfg_path"), iface);
				
				config.runCmd($.sprintf("/bin/cat %s/chipver", confPath));
				config.runCmd($.sprintf("/bin/cat %s/pwr_source", confPath));
			}
			
			addItem("Hardware:SHDSL", iface, "dsl", [iface, pcislot, num]);
		});
	});
	
	/* generate list of E1 inrefaces */
	$.each(slots, function(num, pcislot) {
		if (config.get($.sprintf("sys_pcitbl_s%s_iftype", pcislot)) != config.getOEM("MR16G_DRVNAME")) {
			return true;
		}
		var ifaces = config.getParsed($.sprintf("sys_pcitbl_s%s_ifaces", pcislot));
		$.each(ifaces, function(num, iface) {
			addItem("Hardware:E1", iface, "e1", [iface, pcislot, num]);
		});
	});
	
	/* generate list of RS232 inrefaces */
	$.each(slots, function(num, pcislot) {
		if (config.get($.sprintf("sys_pcitbl_s%s_iftype", pcislot)) != config.getOEM("MR17S_DRVNAME")) {
			return true;
		}
		var ifaces = config.getParsed($.sprintf("sys_pcitbl_s%s_ifaces", pcislot));
		$.each(ifaces, function(num, node) {
			addItem("Hardware:RS232", node, "rs232", [node, pcislot, num]);
		});
	});
	
	/* add dynamic interface controller */
	addItem("Network", "Dynamic interfaces", "dynamic_ifaces");
	
	/* generate list of network interfaces */
	var ifaces = config.getParsed("sys_ifaces");
	$(ifaces).each(function(name, iface) {
		addItem("Network:Interfaces", iface, "iface", [iface]);
	});
	
	/* generate menu */
	$("#menu").treeview({
		unique: true,
		collapsed: true
	});
}
