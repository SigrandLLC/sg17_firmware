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
	
	/* generate list of E1 inrefaces */
	var slots = config.getParsed("sys_pcitbl_slots");
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