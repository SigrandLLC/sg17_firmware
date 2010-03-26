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
				"name": $.sprintf("sys_rs232ip_s%s_%s_src_host", pcislot, pcidev),
				"defaultValue": "0.0.0.0"
			};
			c.addWidget(field);

			field = {
				"type": "text",
				"text": "Listening port",
				"name": $.sprintf("sys_rs232ip_s%s_%s_src_port", pcislot, pcidev),
				"defaultValue": "300" + pcidev
			};
			c.addWidget(field);

			field = {
				"type": "text",
				"text": "Polling interval",
				"name": $.sprintf("sys_rs232ip_s%s_%s_poll_interval", pcislot, pcidev),
				"defaultValue": "100"
			};
			c.addWidget(field);

			field = {
				"type" : "select",
				"text" : "Log level",
				"name" : $.sprintf("sys_rs232ip_s%s_%s_loglevel", pcislot, pcidev),
				"options": {0: "No log", 1:1, 2:2, 3:3, 4:4, 5:5, 6:6, 7:"Full debug"},
				"defaultValue": 2
			};
			c.addWidget(field);

			field = {
				"type" : "checkbox",
				"text" : "Connect to",
				"name" : $.sprintf("sys_rs232ip_s%s_%s_socat_enable", pcislot, pcidev)
			};
			c.addWidget(field);

			field = {
				"type" : "text",
				"text": "Host",
				"name" : $.sprintf("sys_rs232ip_s%s_%s_dst_host", pcislot, pcidev)
			};
			c.addWidget(field);

			field = {
				"type" : "text",
				"text" : "Port",
				"name" : $.sprintf("sys_rs232ip_s%s_%s_dst_port", pcislot, pcidev)
			};
			c.addWidget(field);

			field = {
				"type" : "select",
				"text": "Log level",
				"name" : $.sprintf("sys_rs232ip_s%s_%s_socat_loglevel", pcislot, pcidev),
				"options": {"0": "Error", "1": "Warning", "2": "Notice", "3": "Info", "4": "Debug"}
			};
			c.addWidget(field);


			c.addSubmit();
		}
	});

	page.generateTabs();
};

