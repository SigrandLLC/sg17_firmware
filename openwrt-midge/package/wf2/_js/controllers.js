/* Global hash for controllers */
var Controllers = {
	/* container for displaying controllers content */
	container: "#container",
	
	/* delegates to pageTabs() defined in widgets.js */
	pageTabs: function(tabs) {
		return new pageTabs(this.container, tabs);
	}
};

Controllers['interface'] = function() {
	var tabs = this.pageTabs({general: "Interface"});
	var c = tabs.tabs['general'].addContainer();
	var field;

	c.addTitle("Interface settings");

	field = {
		type: "select",
		name: "sys_interface_language",
		text: "Interface language",
		descr: "Please select language",
		options: {"en": "English", "ru": "Russian"} 
	}
	c.addWidget(field);

	c.addSubmit();
};

Controllers['general'] = function() {
	var tabs = this.pageTabs({general: "General"});
	var c = tabs.tabs['general'].addContainer();
	var field;

	c.addTitle("General settings");

	field = {
		type: "text",
		name: "sys_hostname",
		text: "Hostname",
		descr: "Please enter router's hostname",
		validator: {required: true, minlength: 3},
		message: "Enter hostname"
	}
	c.addWidget(field);

	c.addSubmit();
};

Controllers['dns'] = function() {
	var tabs = new pageTabs(this.container, {dns: _("DNS")});
	var c = tabs.tabs['dns'].addContainer();
	var field;

	c.addTitle(_("DNS settings"));

	field = { 
		type: 'text',
		name: 'sys_dns_nameserver',
		text: 'Upstream server',
		descr: 'Please enter ip address of upstream dns server'
	}
	c.addWidget(field);

	field = { 
		type: 'text',
		name: 'sys_dns_domain',
		text: 'Domain',
		descr: 'Please enter your domain'
	}
	c.addWidget(field);

	c.addSubmit();
}

Controllers['security'] = function() {
	var tabs = new pageTabs(this.container, {security: _("Security")});
	var c = tabs.tabs['security'].addContainer();
	var field;

	c.addTitle(_("Webface admin password"));

	field = { 
		type: 'password',
		name: 'htpasswd',
		text: _('Password'),
		descr: '',
		required: true,
		filters: '',
		message: '',
		pattern: ''
	}
	c.addWidget(field);

	c.addSubmit();
	
	tabs.tabs['security'].addBr();
	c = tabs.tabs['security'].addContainer();

	c.addTitle(_("root system password"));

	field = { 
		type: 'password',
		name: 'passwd',
		text: _('Password'),
		descr: '',
		required: true,
		filters: '',
		message: '',
		pattern: ''
	}
	c.addWidget(field);

	c.addSubmit();
}

// TODO
function cTime(container) {
	var tabs = new pageTabs(container, {time: "Time settings"});
	var c = tabs.tabs['time'].addContainer();
	var field;

	c.addTitle("Time settings");

	field = { 
		type: 'checkbox',
		name: 'sys_ntpclient_enabled',
		text: 'Use time synchronizing',
		descr: 'Check this item if you want use time synchronizing',
		required: true, 
		filters: 'ltrim,rtrim', 
		message: 'Please input correct dns domain name', 
		pattern: 'dnsdomainoripaddr' 
	}
	c.addWidget(field);
	
	field = { 
		type: 'text',
		name: 'sys_ntpclient_server',
		text: 'Time server',
		descr: 'Please input hostname or ip address time server',
		required: true, 
		filters: 'ltrim,rtrim', 
		message: 'Please input correct dns domain name', 
		pattern: 'dnsdomainoripaddr' 
	}
	c.addWidget(field);	
	
	field = {
		type: 'select',
		name: 'sys_timezone',
		text: 'Time zone',
		descr: 'Please select timezone',
		options: {"bad": "Please select timezone", "-12": "GMT-12", "-11": "GMT-11", "-10": "GMT-10",
					"-9": "GMT-9", "-8": "GMT-8", "-7": "GMT-7", "-6": "GMT-6", "-5": "GMT-5", 
					"-4": "GMT-4", "-3": "GMT-3", "-2": "GMT-2", "-1": "GMT-1", "0": "GMT", 
					"1": "GMT+1", "2": "GMT+2", "3": "GMT+3", "4": "GMT+4", "5": "GMT+5", "6": "GMT+6", 
					"7": "GMT+7", "8": "GMT+8", "9": "GMT+9", "10": "GMT+10", "11": "GMT+11", 
					"12": "GMT+12"},
		required: true, 
		filters: 'ltrim,rtrim', 
		message: 'Please input correct dns domain name', 
		pattern: 'dnsdomainoripaddr' 
	}
	c.addWidget(field);
	
	c.addSubmit();
}




function cInterface(container, iface) {
	var field;
	var tabs = new pageTabs(container, {general: "General", method: "Method", options: "Options",
		specific: "Specific"});

	/* general */	
	var c = tabs.tabs['general'].addContainer();
	c.addTitle("Interface general settings");

	field = {
		type: 'text',
		name: 'sys_hostname',
		text: 'Description',
		descr: '',
		required: true,
		filters: 'ltrim,rtrim',
		message: 'Please input correct dns domain name',
		pattern: 'dnsdomainoripaddr'
	}
	c.addWidget(field);
	
	field = {
		type: 'checkbox',
		name: 'sys_hostname',
		text: 'Enabled',
		descr: '',
		required: true,
		filters: 'ltrim,rtrim',
		message: 'Please input correct dns domain name',
		pattern: 'dnsdomainoripaddr'
	}
	c.addWidget(field);

	field = {
		type: 'checkbox',
		name: 'sys_hostname',
		text: 'Auto',
		descr: '',
		required: true,
		filters: 'ltrim,rtrim',
		message: 'Please input correct dns domain name',
		pattern: 'dnsdomainoripaddr'
	}
	c.addWidget(field);

	c.addSubmit();

	/* method */	
	var c = tabs.tabs['method'].addContainer();
	c.addTitle("Static address settings");

	field = {
		type: 'text',
		name: 'sys_hostname',
		text: 'Static address',
		descr: 'Address (dotted quad) <b>required</b>',
		required: true,
		filters: 'ltrim,rtrim',
		message: '',
		pattern: ''
	}
	c.addWidget(field);
	
	field = {
		type: 'text',
		name: 'sys_hostname',
		text: 'Netmask',
		descr: 'Netmask (dotted quad) <b>required</b>',
		required: true,
		filters: 'ltrim,rtrim',
		message: '',
		pattern: ''
	}
	c.addWidget(field);
	
	field = {
		type: 'text',
		name: 'sys_hostname',
		text: 'Broadcast',
		descr: 'Broadcast (dotted quad)',
		required: true,
		filters: 'ltrim,rtrim',
		message: '',
		pattern: ''
	}
	c.addWidget(field);
	
	field = {
		type: 'text',
		name: 'sys_hostname',
		text: 'Gateway',
		descr: 'Default gateway (dotted quad)',
		required: true,
		filters: 'ltrim,rtrim',
		message: '',
		pattern: ''
	}
	c.addWidget(field);
	
	c.addSubmit();
	
	/* options */	
	var c = tabs.tabs['options'].addContainer();
	c.addTitle("Interface options");

	field = {
		type: 'checkbox',
		name: 'sys_hostname',
		text: 'Accept redirects',
		descr: '',
		required: true,
		filters: 'ltrim,rtrim',
		message: '',
		pattern: ''
	}
	c.addWidget(field);
	
	field = {
		type: 'checkbox',
		name: 'sys_hostname',
		text: 'Forwarding',
		descr: '',
		required: true,
		filters: 'ltrim,rtrim',
		message: '',
		pattern: ''
	}
	c.addWidget(field);
	
	field = {
		type: 'checkbox',
		name: 'sys_hostname',
		text: 'Proxy ARP',
		descr: '',
		required: true,
		filters: 'ltrim,rtrim',
		message: '',
		pattern: ''
	}
	c.addWidget(field);
	
	field = {
		type: 'checkbox',
		name: 'sys_hostname',
		text: 'RP Filter',
		descr: '',
		required: true,
		filters: 'ltrim,rtrim',
		message: '',
		pattern: ''
	}
	c.addWidget(field);
		
	c.addSubmit();
	
	/* specific */	
	var c = tabs.tabs['specific'].addContainer();
	c.addTitle("Ethernet Specific parameters");

	field = {
		type: 'text',
		name: 'sys_hostname',
		text: 'MAC address',
		descr: 'MAC address for interface',
		required: true,
		filters: 'ltrim,rtrim',
		message: '',
		pattern: ''
	}
	c.addWidget(field);

	c.addSubmit();
}
