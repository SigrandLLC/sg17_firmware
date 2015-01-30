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
				"type"  : "hidden",
				"name"  : $.sprintf("sys_rs232_tcpext_%s", node),
				"defaultValue" : $.sprintf("%s_%s", pcislot, pcidev)
			};
			c.addWidget(field);

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
				"name": $.sprintf("sys_rs232_tcpext_s%s_%s_iptos", pcislot, pcidev),
				"text": "ToS",
				"descr": "Type of Service (8 bits) for IP packets.",
				"defaultValue": "0x00"
			};
			c.addWidget(field);

			field = {
				"type": "checkbox",
				"name": $.sprintf("sys_rs232_tcpext_s%s_%s_tcp_cork", pcislot, pcidev),
				"text": "TCP_CORK (200 ms)",
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

Controllers.rs232_tcpdmx = function(rs232_list) {
	var page = this.Page();
	page.setSubsystem("rs232_tcpdmx");

	page.addTab({
		"id": "options",
		"name": "Options",
		"func": function() {
			var c, field;
			c = page.addContainer("options");
			c.addTitle("RS232 over tcp/ip demultiplexer settings");

			var rs_arr = rs232_list.split(" ");
			var rs232_list_str = "";
			for (rs in rs_arr) {
				var tmp = config.get($.sprintf("sys_rs232_tcpext_%s", rs_arr[rs]));
				if (tmp != null) {
					var mode = config.get($.sprintf("sys_rs232_tcpext_s%s_mode", tmp));
					if ((mode == "listen") || (mode == "connect")) {
					} else {
						if (rs232_list_str == "")
							rs232_list_str += rs_arr[rs];
						else
							rs232_list_str += " "+rs_arr[rs];
					}
				} else {
					if (rs232_list_str == "")
						rs232_list_str += rs_arr[rs];
					else
						rs232_list_str += " "+rs_arr[rs];
				}
			}
			field = {
				"type" : "select",
				"text": "Port",
				"name" : "sys_rs232_tcpdmx_port",
				"options" : "none "+rs232_list_str,
				"dafaultValue": "none"
			};
			c.addWidget(field);
			c.addSubmit();

			page.addBr("options");
			c = page.addContainer("options");

		var list = c.createList({
			"tabId": "zones",
			"header": ["Host", "Port"],
			"varList": ["host", "port"],
			"listItem": "sys_rs232_tcpdmx_list",
			"showPage": function() {
				Controllers.rs232_tcpdmx(rs232_list);
			},
			"addMessage": "Host successully added!",
			"editMessage": "Host successully changed!",
			"listTitle": "RS232 over tcp/ip demultiplexer clients"
		});


		field = {
			"type": "text",
			"name": "host",
			"text": "Host",
			"validator": {"required": true, "ipAddr": true}
		};
		list.addWidget(field);

		field = {
			"type": "text",
			"name": "port",
			"text": "Port",
			"validator": {"required": true, "min":0,"max":65535}
		};
		list.addWidget(field);
		list.generateList();
		}
	});

	page.generateTabs();
};
