function generateMenu() {
	addItem("System", "Controllers.webface()", "Webface");
	addItem("System", "Controllers.general()", "General");
	addItem("System", "Controllers.security()", "Security");
	addItem("System", "Controllers.dns()", "DNS");
	addItem("System", "Controllers.time()", "Time");
	addItem("System", "Controllers.logging()", "Logging");
	
	/* generate list of interfaces */
	var ifaces = config.getParsed("sys_ifaces");
	/* if we have several interfaces */
	if (typeof ifaces == "object") {
		$(ifaces).each(function(name, value) {
			addItem("Network:Interfaces", "Controllers.iface('" + value + "')", value);
		});
	/* if we have only one interface */
	} else {
		addItem("Network:Interfaces", "Controllers.iface('" + ifaces + "')", ifaces);
	}
	
}