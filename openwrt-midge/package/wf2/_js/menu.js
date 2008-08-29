function generateMenu() {
	addItem("System", "Info", "info");
	addItem("System", "Webface", "webface");
	addItem("System", "General", "general");
	addItem("System", "Security", "security");
	addItem("System", "DNS", "dns");
	addItem("System", "Time", "time");
	addItem("System", "Logging", "logging");
	addItem("System", "Tools", "tools");
	
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