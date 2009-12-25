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
	
	var cursors = new Array(16);
	var consoleDivs = new Array(16);
	var cmdSpans = new Array(16);

	var func = function() {
		config.cmdExecute({
			"cmd": $.sprintf("/sbin/tbuffctl -p* -a"),
			"dataType" : "json",
			"formatData": true,
			"callback": function(data) {
				if (typeof data == "object")
				{
					$.each(data.ttylist, function(index, value) {
						if ((value.text != "") && (typeof value == "object"))
						{
							buf[value.dev] += value.text;
							cursors[value.dev].before(value.text);
							cmdSpans[value.dev] = $.create("span").insertBefore(cursors[value.dev]);
							consoleDivs[value.dev].scrollTo($("#bottomAnchor"), 0);
						}
					});
				}
			}
		});
	};
	setInterval(func, 1000);
	var scroll = function() {
		var ifaces = config.getData(config.getOEM("MR17S_DRVNAME"));
		$.each(ifaces, function(num, ifaceInfo) {
			var iface = ifaceInfo.iface;
			if (config.getParsed($.sprintf("sys_demon_%s_enable", iface)) == 1)
			{
				consoleDivs[iface].scrollTo($("#bottomAnchor"), 0);
			}
		});
	};
//	setInterval(scroll, 1);


	$.each(ifaces, function(num, ifaceInfo) {
		var iface = ifaceInfo.iface;
		if (config.getParsed($.sprintf("sys_demon_%s_enable", iface)) == 1)
		{
			page.addTab({
				"id": $.sprintf("terminal%s", iface),
				"name": $.sprintf("Terminal %s", iface),
				"func": function()
				{
					var cmd2 = "";
					var p = page.getRaw($.sprintf("terminal%s", iface));
					consoleDivs[iface] = $.create("div", {"id": "consoleDiv", "className": "pre scrollable","tabindex": "0"}, "").appendTo(p);
					var cursor = $.create("span", {"id": "consoleCursor"}, "_").appendTo(consoleDivs[iface]);
					cursors[iface] = cursor;
					if (buf[iface] == undefined) buf[iface] = "";
					cursor.before(buf[iface]);
					cmdSpans[iface] = $.create("span").insertBefore(cursor);
					$.create("span", {"id": "bottomAnchor"}, "&nbsp;").appendTo(p);
					consoleDivs[iface].scrollTo($("#bottomAnchor"), 0);

					var onKeypress = function(src) {
						consoleDivs[iface].scrollTo($("#bottomAnchor"), 0);
						var ch;
						if (src.keyCode == 13) {
							if (cmd2 != "") {
								if (buf[iface] == undefined) buf[iface] = cmd2; else buf[iface] += cmd2;
								buf[iface] += "<br>";
								config.cmdExecute({
									"cmd": $.sprintf("/sbin/tbuffctl -p%s -w \"%s\"", iface, cmd2),
									"callback": function(data) {
										cursor.before("<br>");
										cmdSpans[iface] = $.create("span").insertBefore(cursor);
										consoleDivs[iface].scrollTo($("#bottomAnchor"), 0);
									}
								});
/*								config.cmdExecute({
									"cmd": $.sprintf("/sbin/tbuffctl -p%s -r 0", iface),
									"callback": function(data) {
										buf[iface] +=  data;
//										cursor.before(data);
										cmdSpan.text(data);
										cmdSpan = $.create("span").insertBefore(cursor);
										consoleDiv.scrollTo($("#bottomAnchor"), 0);
									}
								});*/
								cmd2 = "";
								consoleDivs[iface].scrollTo($("#bottomAnchor"), 0);
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
						cmdSpans[iface].append(ch);
						cmd2 += ch;
						return false;
					};
					var onBackspace = function() {
						cmd2 = cmd2.substring(0, cmd2.length - 1);
						cmdSpans[iface].text(cmd2);
						return false;
					};
//					var onmouseover = function() {
//						consoleDivs[iface].scrollTo($("#bottomAnchor"), 0);
//					};
//					consoleDivs[iface].mouseover(onmouseover);
					var onclick = function() {
						consoleDivs[iface].scrollTo($("#bottomAnchor"), 0);
					};
					onclick();
					consoleDivs[iface].click(onclick);
	               	consoleDivs[iface].keypress(onKeypress);
					consoleDivs[iface].keydown(function(src) {
						if (src.keyCode == 8) {
							return onBackspace();
						}
					});
				} //func
			}); //addTab
		} // if
	}); //each 
	
    page.generateTabs();
	
};
