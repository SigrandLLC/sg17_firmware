/* Global hash for controllers */
var Controllers = {
	/* container for displaying controllers content */
	container: "#container",
	
	/* delegates to Page() defined in widgets.js */
	Page: function() {
		return new Page(this.container);
	}
};

Controllers['info'] = function() {
	var page = this.Page();
	
	page.addTab({
		"id": "info",
		"name": "System information",
		"func": function() {
			var c, field;
			c = page.addContainer("info");
			c.addTitle("System information");
			
			field = {
				"type": "html",
				"name": "sys_hostname",
				"text": "Hostname",
				"kdb": "sys_hostname"
			};
			c.addWidget(field);
			
			field = {
				"type": "html",
				"name": "version",
				"text": "Firmware version",
				"cmd": "/bin/cat /etc/version"
			};
			c.addWidget(field);
			
			field = {
				"type": "html",
				"name": "time",
				"text": "Time",
				"cmd": "/bin/date"
			};
			c.addWidget(field);
			
			field = {
				"type": "html",
				"name": "uptime",
				"text": "Uptime",
				"cmd": "/usr/bin/uptime |/usr/bin/cut -f1 -d ','"
			};
			c.addWidget(field);
			
			field = {
				"type": "html",
				"name": "la",
				"text": "Load average",
				"cmd": "/bin/cat /proc/loadavg",
				"tip": "The first three numbers represent the number of active tasks on the system - processes that are actually running - averaged over the last 1, 5, and 15 minutes. The next entry shows the instantaneous current number of runnable tasks - processes that are currently scheduled to run rather than being blocked in a system call - and the total number of processes on the system. The final entry is the process ID of the process that most recently ran."
			};
			c.addWidget(field);
		}
	});
	
	page.generateTabs();
};

Controllers['webface'] = function() {
	var page = this.Page();

	page.addTab({
		"id": "webface",
		"name": "Webface",
		"func": function() {
			var c, field;
			c = page.addContainer("webface");
			c.addTitle("Webface settings");

			field = {
				type: "select",
				name: "sys_interface_language",
				text: "Interface language",
				descr: "Please select language",
				options: {"en": "English", "ru": _("Russian")} 
			};
			c.addWidget(field);
		
			c.addSubmit({reload: true});
		}
	});
	
	page.generateTabs();
};

Controllers['general'] = function() {
	var page = this.Page();
	page.setHelpPage("begin");

	page.addTab({
		"id": "general",
		"name": "General",
		"func": function() {
			var c, field;
			c = page.addContainer("general");
			c.setHelpSection("hostname");
			c.addTitle("General settings");

			field = {
				type: "text",
				name: "sys_hostname",
				text: "Hostname",
				descr: "Please enter router's hostname",
				validator: {required: true},
				message: "Enter hostname"
			};
			c.addWidget(field);
		
			c.addSubmit();
		}
	});
	
	page.generateTabs();
};

Controllers['security'] = function() {
	var page = this.Page();
	page.setHelpPage("begin");
	
	page.addTab({
		"id": "security",
		"name": "Security",
		"func": function() {
			var c, field;
			c = page.addContainer("security");
			c.setHelpSection("passwd");
			c.addTitle("Webface password");

			field = { 
				type: "password",
				name: "htpasswd",
				text: "Password",
				descr: "Password for webface user <i>admin</i>",
				validator: {"required": true},
				message: "Enter webface password"
			};
			c.addWidget(field);
		
			c.addSubmit();
			
			page.addBr("security");
			c = page.addContainer("security");
			c.setHelpSection("passwd");
			c.addTitle("System password");
		
			field = { 
				type: "password",
				name: "passwd",
				text: "Password",
				descr: "Password for system user <i>root</i>",
				validator: {required: true},
				message: "Enter system password"
			};
			c.addWidget(field);
		
			c.addSubmit();
		}
	});
	
	page.generateTabs();
}

Controllers['dns'] = function() {
	var page = this.Page();
	page.setSubsystem("dns");
	
	page.addTab({
		"id": "dns",
		"name": "DNS",
		"func": function() {
			var c, field;
			c = page.addContainer("dns");
			c.addTitle("DNS settings");

			field = { 
				"type": "text",
				"name": "sys_dns_nameserver",
				"text": "Upstream server",
				"descr": "Please enter ip address of upstream dns server",
				"tip": "DNS server used for resolving domain names. E.g., 192.168.2.1",
				"validator": {"ipAddr": true}
			};
			c.addWidget(field);
		
			field = { 
				type: "text",
				name: "sys_dns_domain",
				text: "Domain",
				descr: "Please enter your domain",
				tip: "Domain for this router. E.g., localnet"
			};
			c.addWidget(field);
		
			c.addSubmit();
		}
	});
	
	page.generateTabs();
}

Controllers['time'] = function() {
	var page = this.Page();
	
	page.addTab({
		"id": "time",
		"name": "Time settings",
		"func": function() {
			var c, field;
			c = page.addContainer("time");
			c.addTitle("Time settings");

			field = { 
				type: "checkbox",
				name: "sys_ntpclient_enabled",
				text: "Use time synchronizing",
				descr: "Check this item if you want use time synchronizing",
				tip: "Time synchronization via NTP protocol"
			};
			c.addWidget(field);
			
			field = { 
				type: "text",
				name: "sys_ntpclient_server",
				text: "Time server",
				descr: "Please input time server's hostname or ip address"
			};
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
			};
			c.addWidget(field);
			
			c.addSubmit();
		}
	});
	
	page.generateTabs();
}

Controllers['logging'] = function() {
	var page = this.Page();
	page.setHelpPage("logging");
	page.setSubsystem("logging");
	
	page.addTab({
		"id": "logging",
		"name": "Logging",
		"func": function() {
			var c, field;
			c = page.addContainer("logging");
			c.addTitle("Logging settings");

			field = { 
				type: "select",
				name: "sys_log_dmesg_level",
				text: "Kernel console priority logging",
				descr: "Set the level at which logging of messages is done to the console",
				options: {"1": "1", "2": "2", "3": "3", "4": "4", "5": "5", "6": "6", "7": "7"}
			};
			c.addWidget(field);
		
			field = { 
				type: "select",
				name: "sys_log_buf_size",
				text: "Circular buffer",
				descr: "Circular buffer size",
				options: {"0": "0k", "8": "8k", "16": "16k", "32": "32k", "64": "64k", "128": "128k",
							"256": "256k", "512": "512k"}
			};
			c.addWidget(field);
			
			field = { 
				type: "checkbox",
				name: "sys_log_remote_enabled",
				text: "Enable remote syslog logging",
				descr: "Check this item if you want to enable remote logging"
			};
			c.addWidget(field);
			
			field = { 
				type: "text",
				name: "sys_log_remote_server",
				text: "Remote syslog server",
				descr: "Domain name or ip address of remote syslog server"
			};
			c.addWidget(field);
		
			c.addSubmit();
		}
	});
	
	page.generateTabs();
}

Controllers['tools'] = function() {
	var page = this.Page();

	page.addTab({
		"id": "syslog",
		"name": "syslog",
		"func": function() {
			var c;
			c = page.addContainer("syslog");
			c.addTitle("syslog");
			
			/* working directory for script is ./wf2/sh, where execute.cgi is located */
			c.addConsole("/sbin/logread |./colorizelog.sh");
		}
	});
	
	page.addTab({
		"id": "dmesg",
		"name": "dmesg",
		"func": function() {
			var c;
			c = page.addContainer("dmesg");
			c.addTitle("dmesg");
			c.addConsole("/bin/dmesg");
		}
	});
	
	page.addTab({
		"id": "ping",
		"name": "ping",
		"func": function() {
			var c, field;
			c = page.addContainer("ping");
			c.addTitle("ping");
			
			field = { 
				"type": "text",
				"name": "host",
				"text": "Host",
				"defaultValue": "localhost"
			};
			c.addWidget(field);
			
			field = { 
				"type": "text",
				"name": "count",
				"text": "Count",
				"defaultValue": "5"
			};
			c.addWidget(field);
			
			c.addRun("/bin/ping -c %ARG %ARG", "count", "host");
		}
	});
	
	page.addTab({
		"id": "mtr",
		"name": "mtr",
		"func": function() {
			var c, field;
			c = page.addContainer("mtr");
			c.addTitle("mtr");
			
			field = { 
				"type": "text",
				"name": "mtr_host",
				"text": "Host",
				"defaultValue": "localhost"
			};
			c.addWidget(field);
			
			field = { 
				"type": "text",
				"name": "mtr_count",
				"text": "Count",
				"defaultValue": "5"
			};
			c.addWidget(field);
			
			c.addRun("/usr/sbin/mtr -r -n -s 100 -c %ARG %ARG", "mtr_count", "mtr_host");
		}
	});
	
	page.generateTabs();
};

Controllers['reboot'] = function() {
	var page = this.Page();
	
	page.addTab({
		"id": "reboot",
		"name": "Reboot",
		"func": function() {
			var c;
			c = page.addContainer("reboot");
			c.addTitle("Reboot");
			
			c.addAction("Reboot", "/sbin/reboot");
		}
	});
	
	page.generateTabs();
}
