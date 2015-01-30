var conf;
Controllers.snmpd = function ()
{
	var page = this.Page();
	page.setSubsystem("snmpd");

	page.addTab({
		"id": "snmpd",
		"name": "Snmpd",
		"func": function()
		{
			var c, field;
			c = page.addContainer("snmpd");
			c.setHelpPage("snmpd");
			c.addTitle("Snmpd settings");
		
			field = {
				"type" : "checkbox",
				"text": "Enable snmpd",
				"name" : "sys_snmpd_enable",
				"tip" : "Enable SNMP agent"
			};
			c.addWidget(field);
			field = {
				"type" : "text",
				"text": "Readonly community name",
				"name" : "sys_snmpd_rocommname",
				"defaultValue" : "public"
			};
			c.addWidget(field);

			field = {
				"type" : "text",
				"text": "Readonly source",
				"name" : "sys_snmpd_rosource",
				"tip" : "192.168.0.1 or 192.168.0.1/255.255.255.0 or 192.168.0.1/24",
				"descr": "If empty from all hosts",
				"defaultValue" : ""
			};
			c.addWidget(field);


			field = {
				"type" : "text",
				"text": "Read/write community name",
				"name" : "sys_snmpd_rwcommname",
				"defaultValue" : "private"
			};
			c.addWidget(field);

			field = {
				"type" : "text",
				"text": "Read/write source",
				"name" : "sys_snmpd_rwsource",
				"tip" : "192.168.0.1 or 192.168.0.1/255.255.255.0 or 192.168.0.1/24",
				"descr": "If empty from all hosts",
				"defaultValue" : ""
			};
			c.addWidget(field);

			field = {
				"type" : "select",
				"text": "Trap version",
				"name" : "sys_snmpd_trapver",
				"options" : {"v1" : "v1", "v2c" : "v2c"}
			};
			c.addWidget(field);

			field = {
				"type" : "text",
				"text": "Adresses where to send trap",
				"name" : "sys_snmpd_trapaddr",
				"tip" : "192.168.0.1 192.168.0.2"
			};
			c.addWidget(field);


			c.addSubmit({'reload' : false});
/*
			field = {
				"type" : "text",
				"text": "snmpd command line options",
				"name" : "sys_snmpd_cmd",
				"tip" : "snmpd command line options"
			};
			c.addWidget(field);
*/			
/*			config.cmdExecute({
				"cmd" : "cat /etc/snmp/snmpd.conf",
				"async" : false,
				"callback": function(data) {
					conf = data;
				}
			});
			
			var p = page.getRaw("snmpd");
			$.create('div')
			var ta = $.create('textarea', {'name':'conf', 'value': conf, 'cols' : 90, 'rows' : 30, 'wrap':'off', 'id' : 'conf'}).appendTo(p);
			
			
			c.addSubmit({'preSubmit' : function()
			{
				var str = new String($("#conf").val());
				var str2 = new String("");
				var i;
				for (i = 0; i < str.length; i++)
				{
					switch (str.charCodeAt(i)) {
						case 10:
							str2 = str2 + '\\n';
						break;
						case 34:
							str2 = str2 + "\\\"";
						break;
						default:
							str2 = str2 + str.charAt(i);
					}
				}
				config.cmdExecute({"cmd": "echo -e \""+str2+"\" > /etc/snmp/snmpd.conf", "async" : false});
			}
			, 'reload' : false});
*/
			

		}
	});

/*
	page.addTab({
		"id": "snmpdlog",
		"name": "snmpd.log",
		"func": function() {
			var c;
			c = page.addContainer("snmpdlog");
			c.addTitle("snmpd.log");

			c.addConsole("cat /var/log/snmpd.log");
		}
	});
*/
	page.generateTabs();
};
