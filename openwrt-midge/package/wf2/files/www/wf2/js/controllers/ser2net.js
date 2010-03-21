Controllers.ser2net = function ()
{
	var page = this.Page();
	page.setSubsystem("ser2net");

	page.addTab({
		"id": "options",
		"name": "Options",
		"func": function()
		{
			var colSpan = 6;
			var c = page.addContainer("options");

			c.addTitle("Options", {"colspan": colSpan});

			c.addTableHeader("Device|Enable|Listening Port|Timeout|Host|Port");

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
					"type": "text",
					"name": $.sprintf("sys_ser2net_%s_listen_port", iface)
				};
				c.addTableWidget(field, row);

				field = {
					"type" : "text",
					"name" : $.sprintf("sys_ser2net_%s_timeout", iface)
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



			});

			c.addSubmit();
		}
	});

	page.generateTabs();
};

