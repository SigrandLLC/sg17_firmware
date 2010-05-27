Controllers.rs232_tcpext = function(node, pcislot, pcidev)
{
	var page = this.Page();
	page.setSubsystem($.sprintf("rs232_tcpext.%s.%s", pcislot, pcidev));

	page.addTab({
		"id": "options",
		"name": "Options",
		"func": function()
		{
			var c, field;
			c = page.addContainer("options");

			c.addTitle($.sprintf("%s (module %s, slot %s) over tcp/ip settings", node,
				mr17sModuleName(pcislot), pcislot - 2));

			field = {
				"type" : "select",
				"text": "mode",
				"name" : $.sprintf("sys_rs232_tcpext_s%s_%s_mode", pcislot, pcidev),
				"options" : {	"disable" : "Disable",
						"listen"  : "Listen for",
						"connect" : "Connect to"
				}
			};
			c.addWidget(field);

			field = {
				"type": "text",
				"text": "host",
				"name": $.sprintf("sys_rs232_tcpext_s%s_%s_host", pcislot, pcidev),
				"defaultValue": "0.0.0.0"
			};
			c.addWidget(field);

			field = {
				"type": "text",
				"text": "port",
				"name": $.sprintf("sys_rs232_tcpext_s%s_%s_port", pcislot, pcidev),
				"defaultValue": "300" + pcidev
			};
			c.addWidget(field);

			field = {
				"type": "text",
				"text": "Modem lines polling interval (msec)",
				"name": $.sprintf("sys_rs232_tcpext_s%s_%s_poll_interval", pcislot, pcidev),
				"defaultValue": "100"
			};
			c.addWidget(field);

			field = {
				"type": "text",
				"text": "Restart delay (msec)",
				"name": $.sprintf("sys_rs232_tcpext_s%s_%s_restart_delay", pcislot, pcidev),
				"defaultValue": "1000"
			};
			c.addWidget(field);

			c.addSubmit();
		}
	});

	page.generateTabs();
};

