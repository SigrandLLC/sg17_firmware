Controllers.rs232ip = function(node, pcislot, pcidev)
{
	var page = this.Page();
	page.setSubsystem($.sprintf("rs232ip.%s.%s", pcislot, pcidev));

	page.addTab({
		"id": "options",
		"name": "Options",
		"func": function()
		{
			var c, field;
			c = page.addContainer("options");

			c.addTitle($.sprintf("%s (module %s, slot %s) over ip settings", node,
				mr17sModuleName(pcislot), pcislot - 2));

			field = {
				"type" : "checkbox",
				"text" : "Enable",
				"name" : $.sprintf("sys_rs232ip_s%s_%s_enable", pcislot, pcidev)
			};
			c.addWidget(field);

			field = {
				"type": "text",
				"text": "Listening host",
				"name": $.sprintf("sys_rs232ip_s%s_%s_listen_host", pcislot, pcidev)
			};
			c.addWidget(field);

			field = {
				"type": "text",
				"text": "Listening port",
				"name": $.sprintf("sys_rs232ip_s%s_%s_listen_port", pcislot, pcidev)
			};
			c.addWidget(field);

			field = {
				"type" : "select",
				"text" : "Log level",
				"name" : $.sprintf("sys_rs232ip_s%s_%s_loglevel", pcislot, pcidev),
				"options": [0, 1, 2, 3, 4, 5, 6, 7]
			};
			c.addWidget(field);


			c.addHrField("Connect to options:");

			field = {
				"type" : "checkbox",
				"text" : "Enable",
				"name" : $.sprintf("sys_rs232ip_s%s_%s_socat_enable", pcislot, pcidev)
			};
			c.addWidget(field);

			field = {
				"type" : "text",
				"text": "Host",
				"name" : $.sprintf("sys_rs232ip_s%s_%s_socat_host", pcislot, pcidev)
			};
			c.addWidget(field);

			field = {
				"type" : "text",
				"text" : "Port",
				"name" : $.sprintf("sys_rs232ip_s%s_%s_socat_port", pcislot, pcidev)
			};
			c.addWidget(field);

			field = {
				"type" : "select",
				"text": "Log level",
				"name" : $.sprintf("sys_rs232ip_s%s_%s_socat_loglevel", pcislot, pcidev),
				"options": [0, 1, 2, 3, 4]
			};
			c.addWidget(field);


			c.addSubmit();
		}
	});

	page.generateTabs();
};

