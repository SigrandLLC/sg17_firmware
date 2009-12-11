Controllers.terminal = function ()
{
    var page = this.Page();
    page.setSubsystem("terminal");

    page.addTab({
        "id": "options",
        "name": "Options",
        "func": function()
       	{
       	
       	
            var colSpan = 3;
            var c = page.addContainer("options");
            c.addTitle("Options", {"colspan": colSpan});
			
            c.addTableHeader("Port|Enable|Speed");

			var ifaces = config.getData(config.getOEM("MR17S_DRVNAME"));
			$.each(ifaces, function(num, ifaceInfo) {
				var iface = ifaceInfo.iface;
				var field = { 
					"type": "hidden",
					"name": "sys_demon_iface_name",
					"defaultValue": iface
//					"id": iface
				};
				c.addWidget(field);
				var field = { 
					"type": "hidden",
					"name": "sys_demon_buf_size",
					"defaultValue": 1024
//					"id": iface
				};
				c.addWidget(field);
//				c.createHiddenWidget(field, iface);
				
				
                var field;
                var row = c.addTableRow();
                field = {
                    "type" : "html",
                    "name" : iface,
                    "str" : iface
                };
                c.addTableWidget(field, row);
                field = {
                    "type" : "checkbox",
                    "name" : $.sprintf("sys_demon_%s_enable", iface),
                    "tip" : "Enable port listenning"
                };
                c.addTableWidget(field, row);

                field = {
                    "type" : "select",
                    "name" : $.sprintf("sys_demon_%s_speed", iface),
                    "options": [230400,115200,38400,19200,9600,4800,2400,1200],
                    "defaultValue" : 230400
                };
                c.addTableWidget(field, row);
			});

	        c.addSubmit({"reload": true});
			page.addBr("options");


            var c, field;
            c = page.addContainer("options");
            c.addTitle("Options for all ports");
            field = {
                "type": "text",
                "name": "sys_demon_buf_size",
                "text": "Buffer size",
				"defaultValue": 1024,
                "validator": {"required": true, "min": 10},
                "descr": "The buffer size for data from port."
            };
            c.addWidget(field);
	        c.addSubmit({"reload": true});
        }
    });
    

    var ifaces = config.getData(config.getOEM("MR17S_DRVNAME"));
	var buf = new Array(16);
    $.each(ifaces, function(num, ifaceInfo) {
			var iface = ifaceInfo.iface;
			var tmp = config.getParsed($.sprintf("sys_demon_%s_enable", iface));
			if (tmp == 1)
{
			buf[iface] = "";
			page.addTab({
				"id": $.sprintf("terminal%s", iface),
				"name": $.sprintf("Terminal %s", iface),
				"func": function()
				{
					var cmd2 = "";
					var p = page.getRaw($.sprintf("terminal%s", iface));
					var consoleDiv = $.create("div", {"id": "consoleDiv", "className": "pre scrollable",
							"tabindex": "0"}, "").appendTo(p).focus();
					$.create("span", {"id": "bottomAnchor"}, "&nbsp;").appendTo(p);
					var cursor = $.create("span", {"id": "consoleCursor"}, "_").appendTo(consoleDiv);
					var cursorAnimate = function() {
						var nextDisplay = cursor.css("display") == "inline" ? "none" : "inline";
						cursor.css("display", nextDisplay);
					};
//					setInterval(cursorAnimate, 500);
//					var cmdSpan = $.create("span").insertBefore(cursor);
					var keypressDisabled = false;
					cursor.before(buf[iface]);
					cmdSpan = $.create("span").insertBefore(cursor);
					consoleDiv.scrollTo($("#bottomAnchor"), 700);
					
					var func = function() {
							config.cmdExecute({
								"cmd": $.sprintf("/sbin/tbuffctl -p%s -r 0", iface),
								"formatData": true,
								"callback": function(data) {
									if (data)
									{
										buf[iface] +=  data;
										keypressDisabled = false;
										$("#executingCmd").remove();
										cursor.before(data);
										cmdSpan = $.create("span").insertBefore(cursor);
										consoleDiv.scrollTo($("#bottomAnchor"), 700);
									}
								}
							});
					};
					setInterval(func, 3000);
					
					
					var onKeypress = function(src) {
						var ch;
						if (keypressDisabled) {
							return false;
						}
						if (src.keyCode == 13) {
						if (cmd2 != "") {
							buf[iface] += cmd2;
							buf[iface] += "<br>";
							config.cmdExecute({
								"cmd": $.sprintf("/sbin/tbuffctl -p%s -w \"%s\"", iface, cmd2),
								"formatData": true,
								"callback": function(data) {
									buf[iface] +=  data;
//									alert("buf[iface] = " + buf[iface]);
									keypressDisabled = false;
									$("#executingCmd").remove();
									cursor.before(data);
									cmdSpan = $.create("span").insertBefore(cursor);
									consoleDiv.scrollTo($("#bottomAnchor"), 700);
								}
							});
							config.cmdExecute({
								"cmd": $.sprintf("/sbin/tbuffctl -p%s -r 0", iface),
								"formatData": true,
								"callback": function(data) {
									buf[iface] +=  cmd2;
									keypressDisabled = false;
									$("#executingCmd").remove();
									cursor.before(data);
									cmdSpan = $.create("span").insertBefore(cursor);
									consoleDiv.scrollTo($("#bottomAnchor"), 700);
								}
							});
							keypressDisabled = true;
							cmd2 = "";
		           			cursor.before("<br/>");
							cursor.before($.create("span", {"id": "executingCmd"}, _("executing command...")));
							consoleDiv.scrollTo($("#bottomAnchor"), 200);
							return false;
						}
						} else if (src.keyCode == 16 || src.keyCode == 17 || src.keyCode == 18
								|| src.keyCode == 37 || src.keyCode == 38 || src.keyCode == 39
								|| src.keyCode == 40 || src.keyCode == 8) {
							return false;
						}
						if (src.which == null) {
							ch = String.fromCharCode(src.keyCode);
						} else if (src.which > 0) {
							ch = String.fromCharCode(src.which);
						}
						cmdSpan.append(ch);
						cmd2 += ch;
						return false;
					};
					var onBackspace = function() {
						cmd2 = cmd2.substring(0, cmd2.length - 1);
						cmdSpan.text(cmd2);
						return false;
					};
	               	consoleDiv.keypress(onKeypress);
					consoleDiv.keydown(function(src) {
						if (src.keyCode == 8) {
							return onBackspace();
						}
					});
				}
			});
};
	}); // each

    page.generateTabs();
};
