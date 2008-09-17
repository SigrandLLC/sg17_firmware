function generateMenu() {
	addItem("System", "Info", "info");
	addItem("System", "Webface", "webface");
	addItem("System", "General", "general");
	addItem("System", "Security", "security");
	addItem("System", "DNS", "dns");
	addItem("System", "Time", "time");
	addItem("System", "Logging", "logging");
	addItem("System", "Tools", "tools");
	addItem("System", "Reboot", "reboot");
	
	addItem("Hardware", "Multiplexing", "multiplexing");
	
	/* get list of PCI slots */
	var slots = config.getParsed("sys_pcitbl_slots");
	
	/* slots always should be of array type */
	if (typeof slots == "string") {
		var slot = slots;
		slots = new Array();
		slots.push(slot);
	}
	
	/* generate list of SHDSL interfaces */
	$.each(slots, function(num, pcislot) {
		var type = config.get("sys_pcitbl_s" + pcislot + "_iftype");
		if (type != config.getOEM("MR16H_DRVNAME") && type != config.getOEM("MR17H_DRVNAME")) {
			return true;
		}
		var ifaces = config.getParsed("sys_pcitbl_s" + pcislot + "_ifaces");
		$.each(ifaces, function(num, iface) {
			addItem("Hardware:SHDSL", iface, "dsl", [iface, pcislot, num]);
		});
	});
	
	/* generate list of E1 inrefaces */
	$.each(slots, function(num, pcislot) {
		if (config.get("sys_pcitbl_s" + pcislot + "_iftype") != "mr16g") return true;
		var ifaces = config.getParsed("sys_pcitbl_s" + pcislot + "_ifaces");
		$.each(ifaces, function(num, iface) {
			addItem("Hardware:E1", iface, "e1", [iface, pcislot, num]);
		});
	});
	
	/* generate list of interfaces */
	var ifaces = config.getParsed("sys_ifaces");
	/* if we have several interfaces */
	if (typeof ifaces == "object") {
		$(ifaces).each(function(name, value) {
			addItem("Network:Interfaces", value, "iface", [value]);
		});
	/* if we have only one interface */
	} else {
		addItem("Network:Interfaces", ifaces, "iface", [ifaces]);
	}
}
