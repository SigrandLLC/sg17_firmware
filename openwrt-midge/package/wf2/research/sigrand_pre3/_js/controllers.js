function cGeneral(container) {
	var tabs = new pageTabs(container, {general: "General"});
	var c = tabs.tabs['general'].addContainer();
	var field;

	c.addTitle("General settings");

	field = { 
		type: 'text',
		name: 'sys_hostname',
		text: 'Hostname',
		descr: 'Please enter router\'s hostname',
		required: true, 
		filters: 'ltrim,rtrim', 
		message: 'Please input correct dns domain name', 
		pattern: 'dnsdomainoripaddr' 
	}
	c.addWidget(field);

	c.addSubmit();
}

function cSecurity(container) {
	var tabs = new pageTabs(container, {security: "Security"});

	var c = tabs.tabs['security'].addContainer();
	var field;

	c.addTitle("Webface admin password");

	field = { 
		type: 'text',
		name: 'sys_hostname',
		text: 'Password',
		descr: '',
		required: true, 
		filters: 'ltrim,rtrim', 
		message: 'Please input correct dns domain name', 
		pattern: 'dnsdomainoripaddr' 
	}
	c.addWidget(field);

	c.addSubmit();
	
	tabs.tabs['security'].addBr();
	c = tabs.tabs['security'].addContainer();

	c.addTitle("root system password");

	field = { 
		type: 'text',
		name: 'sys_hostname',
		text: 'Password',
		descr: '',
		required: true, 
		filters: 'ltrim,rtrim', 
		message: 'Please input correct dns domain name', 
		pattern: 'dnsdomainoripaddr' 
	}
	c.addWidget(field);

	c.addSubmit();
}

function cDns(container) {
	var tabs = new pageTabs(container, {dns: "DNS"});
	var c = tabs.tabs['dns'].addContainer();
	var field;

	c.addTitle("DNS settings");

	field = { 
		type: 'text',
		name: 'sys_hostname',
		text: 'Upstream server',
		descr: 'Please enter ip address of upstream dns server',
		required: true, 
		filters: 'ltrim,rtrim', 
		message: 'Please input correct dns domain name', 
		pattern: 'dnsdomainoripaddr' 
	}
	c.addWidget(field);
	
	field = { 
		type: 'text',
		name: 'sys_hostname',
		text: 'Domain',
		descr: 'Please enter your domain',
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
