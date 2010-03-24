Controllers.ser2net = function ()
{
	var page = this.Page();
	page.setSubsystem("ser2net");

	page.addTab({
		"id": "options",
		"name": "Options",
		"func": function()
		{
			var colSpan = 5;
			var c0 = page.addContainer("options");
			var c = page.addContainer("options");

			c0.addTitle("Options", {"colspan": colSpan});

			c0.addTableHeader("Device |||Listening on|Connect to");
			c.addTableHeader("Device|Enable|Timeout||Host|Port||Host|Port|Enable");

			var ifaces = config.getData(config.getOEM("MR17S_DRVNAME"));

			$.each(ifaces, function(num, ifaceInfo) {
				var iface = ifaceInfo.iface;
				var field, row;

				row = c.addTableRow();

				field = {
					"type" : "html",
					"name" : iface,
					"str" : iface
				};
				c.addTableWidget(field, row);

				field = {
					"type" : "checkbox",
					"name" : $.sprintf("sys_ser2net_%s_enable", iface),
				};
				c.addTableWidget(field, row);

				field = {
					"type" : "text",
					"name" : $.sprintf("sys_ser2net_%s_timeout", iface)
				};
				c.addTableWidget(field, row);

				//br
				field = {
					"type": "html",
					"str": ""
				};
				c.addTableWidget(field, row);


				field = {
					"type": "text",
					"name": $.sprintf("sys_ser2net_%s_listen_host", iface)
				};
				c.addTableWidget(field, row);

				field = {
					"type": "text",
					"name": $.sprintf("sys_ser2net_%s_listen_port", iface)
				};
				c.addTableWidget(field, row);

				//br
				field = {
					"type": "html",
					"str": ""
				};
				c.addTableWidget(field, row);


				field = {
					"type" : "text",
					"name" : $.sprintf("sys_ser2net_%s_host", iface)
				};
				c.addTableWidget(field, row);

				field = {
					"type" : "text",
					"name" : $.sprintf("sys_ser2net_%s_port", iface)
				};
				c.addTableWidget(field, row);

				field = {
					"type" : "checkbox",
					"name" : $.sprintf("sys_ser2net_%s_socat_enable", iface),
				};
				c.addTableWidget(field, row);

			});

			c.addSubmit();
		}
	});

	page.generateTabs();
};

