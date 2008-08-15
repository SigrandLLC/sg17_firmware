/* Global hash for controllers */
var Controllers = {
	/* container for displaying controllers content */
	container: "#container",
	
	/* delegates to pageTabs() defined in widgets.js */
	pageTabs: function(tabs, options) {
		return new pageTabs(this.container, tabs, options);
	}
};

Controllers['webface'] = function() {
	var tabs = this.pageTabs({general: "Webface"});
	var c = tabs.tabs['general'].addContainer();
	var field;

	c.addTitle("Webface settings");

	field = {
		type: "select",
		name: "sys_interface_language",
		text: "Interface language",
		descr: "Please select language",
		options: {"en": "English", "ru": "Russian"} 
	}
	c.addWidget(field);

	c.addSubmit({reload: true});
};

Controllers['general'] = function() {
	/* create tabs on the page with common help page */
	var tabs = this.pageTabs({general: "General"}, {help: "begin"});
	var c = tabs.tabs['general'].addContainer("hostname");
	var field;

	c.addTitle("General settings");

	field = {
		type: "text",
		name: "sys_hostname",
		text: "Hostname",
		descr: "Please enter router's hostname",
		validator: {required: true},
		message: "Enter hostname"
	}
	c.addWidget(field);

	c.addSubmit();
};

Controllers['security'] = function() {
	var tabs = this.pageTabs({security: "Security"}, {help: "begin"});
	var c = tabs.tabs['security'].addContainer("passwd");
	var field;

	c.addTitle("Webface password");

	field = { 
		type: "password",
		name: "htpasswd",
		text: "Password",
		descr: "Password for webface user <i>admin</i>",
		validator: {required: true},
		message: "Enter webface password"
	}
	c.addWidget(field);

	c.addSubmit();
	
	tabs.tabs['security'].addBr();
	c = tabs.tabs['security'].addContainer();

	c.addTitle("System password");

	field = { 
		type: "password",
		name: "passwd",
		text: "Password",
		descr: "Password for system user <i>root</i>",
		validator: {required: true},
		message: "Enter system password"
	}
	c.addWidget(field);

	c.addSubmit();
}

Controllers['dns'] = function() {
	var tabs = this.pageTabs({dns: "DNS"});
	var c = tabs.tabs['dns'].addContainer();
	var field;

	c.addTitle("DNS settings");

	field = { 
		type: "text",
		name: "sys_dns_nameserver",
		text: "Upstream server",
		descr: "Please enter ip address of upstream dns server",
		tip: "E.g., 192.168.2.1"
	}
	c.addWidget(field);

	field = { 
		type: "text",
		name: "sys_dns_domain",
		text: "Domain",
		descr: "Please enter your domain",
		tip: "E.g., localnet"
	}
	c.addWidget(field);

	c.addSubmit();
}

Controllers['time'] = function() {
	var tabs = this.pageTabs({time: "Time settings"});
	var c = tabs.tabs['time'].addContainer();
	var field;

	c.addTitle("Time settings");

	field = { 
		type: "checkbox",
		name: "sys_ntpclient_enabled",
		text: "Use time synchronizing",
		descr: "Check this item if you want use time synchronizing",
		tip: "Time synchronization via NTP protocol"
	}
	c.addWidget(field);
	
	field = { 
		type: "text",
		name: "sys_ntpclient_server",
		text: "Time server",
		descr: "Please input time server's hostname or ip address"
	}
	c.addWidget(field);
	
	field = {
		type: "select",
		name: "sys_timezone",
		text: "Time zone",
		descr: "Please select timezone",
		options: {"bad": "Please select timezone", "-12": "GMT-12", "-11": "GMT-11", "-10": "GMT-10",
					"-9": "GMT-9", "-8": "GMT-8", "-7": "GMT-7", "-6": "GMT-6", "-5": "GMT-5", 
					"-4": "GMT-4", "-3": "GMT-3", "-2": "GMT-2", "-1": "GMT-1", "0": "GMT", 
					"1": "GMT+1", "2": "GMT+2", "3": "GMT+3", "4": "GMT+4", "5": "GMT+5", "6": "GMT+6", 
					"7": "GMT+7", "8": "GMT+8", "9": "GMT+9", "10": "GMT+10", "11": "GMT+11", 
					"12": "GMT+12"}
	}
	c.addWidget(field);
	
	c.addSubmit();
}

Controllers['logging'] = function() {
	/* create tabs for the page with specified subsystem and common help page */
	var tabs = this.pageTabs({logging: "Logging"}, {subsystem: "logging", help: "logging"});
	var c = tabs.tabs['logging'].addContainer();
	var field;

	c.addTitle("Logging settings");

	field = { 
		type: "select",
		name: "sys_log_dmesg_level",
		text: "Kernel console priority logging",
		descr: "Set the level at which logging of messages is done to the console",
		options: {"1": "1", "2": "2", "3": "3", "4": "4", "5": "5", "6": "6", "7": "7"}
	}
	c.addWidget(field);

	field = { 
		type: "select",
		name: "sys_log_buf_size",
		text: "Circular buffer",
		descr: "Circular buffer size",
		options: {"0": "0k", "8": "8k", "16": "16k", "32": "32k", "64": "64k", "128": "128k",
					"256": "256k", "512": "512k"}
	}
	c.addWidget(field);
	
	field = { 
		type: "checkbox",
		name: "sys_log_remote_enabled",
		text: "Enable remote syslog logging",
		descr: "Check this item if you want to enable remote logging"
	}
	c.addWidget(field);
	
	field = { 
		type: "text",
		name: "sys_log_remote_server",
		text: "Remote syslog server",
		descr: "Domain name or ip address of remote syslog server"
	}
	c.addWidget(field);

	c.addSubmit();
}

Controllers['iface'] = function(iface) {
	var field;
	var c;
	var tabs = this.pageTabs({status: "Status", general: "General", method: "Method",
		options: "Options", specific: "Specific", qos: "QoS", routes: "Routes"},
		{subsystem: "network", help: "iface"});
	
	/* status tab */
	c = tabs.tabs['status'].addContainer();
	c.addTitle("Interface status");
	c.addConsole(["/sbin/ifconfig " + iface, "/usr/sbin/ip addr show dev " + iface,
		"/usr/sbin/ip link show dev " + iface]);
	
	c = tabs.tabs['status'].addBr();
	c = tabs.tabs['status'].addContainer();
	c.addTitle("Routes");
	c.addConsole("/usr/sbin/ip route show dev " + iface);
	
	c = tabs.tabs['status'].addBr();
	c = tabs.tabs['status'].addContainer();
	c.addTitle("ARP");
	c.addConsole("/usr/sbin/ip neigh show dev " + iface);
	
	/* general tab */
	c = tabs.tabs['general'].addContainer();
	c.addTitle("Interface general settings");

	field = { 
		type: "text",
		name: "sys_iface_" + iface + "_desc",
		text: "Description"
	}
	c.addWidget(field);

	field = { 
		type: "checkbox",
		name: "sys_iface_" + iface + "_enabled",
		text: "Enabled"
	}
	c.addWidget(field);
	
	field = { 
		type: "checkbox",
		name: "sys_iface_" + iface + "_auto",
		text: "Auto"
	}
	c.addWidget(field);
	
	field = { 
		type: "select",
		name: "sys_iface_" + iface + "_method",
		text: "Method",
		descr: "Select method of setting IP address",
		options: {"none": "None", "static": "Static address", "zeroconf": "Zero Configuration", "dynamic": "Dynamic address"}
	}
	c.addWidget(field);
	
	field = { 
		type: "select",
		name: "sys_iface_" + iface + "_depend_on",
		text: "Depended on",
		options: {}
	}
	c.addWidget(field);

	c.addSubmit();
	
	/* method tab */
	var c = tabs.tabs['method'].addContainer();

	c.addTitle("Static address settings");

	field = { 
		type: "text",
		name: "sys_iface_" + iface + "_ipaddr",
		text: "Static address",
		desc: "Address (dotted quad) <b>required</b>",
		validator: {required: true},
		message: "Please enter correct IP address"
	}
	c.addWidget(field);
	
	field = { 
		type: "text",
		name: "sys_iface_" + iface + "_netmask",
		text: "Netmask",
		desc: "Netmask (dotted quad) <b>required</b>",
		validator: {required: true},
		message: "Please enter correct IP netmask"
	}
	c.addWidget(field);
	
	field = { 
		type: "text",
		name: "sys_iface_" + iface + "_broadcast",
		text: "Broadcast",
		desc: "Broadcast (dotted quad)"
	}
	c.addWidget(field);
	
	field = { 
		type: "text",
		name: "sys_iface_" + iface + "_gateway",
		text: "Gateway",
		desc: "Default gateway (dotted quad)"
	}
	c.addWidget(field);
	
	c.addSubmit();
}
