var timer = 0;
var buf = new Array(16);
var cursors = new Array(16);
var consoleDivs = new Array(16);
var cmdSpans = new Array(16);
var t0 = 0, t1, t2, t3;
var block = new Array(16);
var block2 = 0;
var func_en = 0;

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
			
            c.addTableHeader("Port|Enable|Name");

			var ifaces = config.getData(config.getOEM("MR17S_DRVNAME"));
			function sortfunc(i, ii) {
				if (i.iface.length > ii.iface.length) return 1;
				else if (i.iface.length < ii.iface.length) return -1; else
		    	if (i.iface > ii.iface)
					return 1;
				else if (i.iface < ii.iface)
					return -1;
				else
					return 0;
			}
			ifaces.sort(sortfunc);

			$.each(ifaces, function(num, ifaceInfo) {
				var iface = ifaceInfo.iface;
				var field = { 
					"type": "hidden",
					"name": "sys_demon_iface_name",
					"defaultValue": iface
				};
				c.addWidget(field);
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
                    "type" : "text",
                    "name" : $.sprintf("sys_demon_%s_name", iface),
                    "defaultValue" : iface
                };
                c.addTableWidget(field, row);
			});
            var row = c.addTableRow();
			var field;
			field = {
            	"type" : "html",
                "str" : "Buffer size",
				"name" : "1"
            };
		    c.addTableWidget(field, row, 2);
			field = {
            	"type" : "text",
            	"name" : "sys_demon_buf_size",
				"validator": {"required": true, "min": 1024},
                "defaultValue" : 4096
            };
		    c.addTableWidget(field, row, 2);
			
			c.addTableTfootStr("Ports settings see in Hardware/RS232", colSpan);

	        c.addSubmit({"onSuccess" : function()
			{
				var ifaces = config.getData(config.getOEM("MR17S_DRVNAME"));
				$.each(ifaces, function(num, ifaceInfo) {
					var iface = ifaceInfo.iface;
					buf[iface] = "";
				});
				Controllers["terminal"]();
			}, "reload" : false});
        }
    });
	
//	var el = document.getElementById("status_ajax");
//	if (el) el.parentNode.removeChild(el);
//	el.style.visibility='hidden'; 'inherit';
	

    var ifaces = config.getData(config.getOEM("MR17S_DRVNAME"));

	func_en = 0;
	$.each(ifaces, function(num, ifaceInfo) {
		var iface = ifaceInfo.iface;
		if (config.getParsed($.sprintf("sys_demon_%s_enable", iface)) == 1) func_en = 1;
	});


	var func = function() {
		if (!block2)
		{
		t2 = new Date().getTime();
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
							if (buf[value.dev].length > 40*1024) buf[value.dev] = buf[value.dev].substring(buf[value.dev].length - 40*1024, buf[value.dev].length);
							cursors[value.dev].before(value.text);
							cmdSpans[value.dev] = $.create("span").insertBefore(cursors[value.dev]);
							
							setTimeout(function () {consoleDivs[value.dev].scrollTo('100%', 0);}, 10);
							
							block[value.dev] = 0;
							
							t3 = new Date().getTime();
//							if (t0 != 0) alert((t1-t0)/1000 + " " + (t2-t1)/1000 + " " + (t3-t2)/1000);
							t0 = 0;
						}
					});
				}
			}
		});
		}
		if (func_en == 1) setTimeout(func, 500); else timer = 0;
	};
	if ((timer == 0) && (func_en == 1))
	{
		setTimeout(func, 500);
		timer = 1;
	}
	function sortfunc(i, ii) {
		if (i.iface.length > ii.iface.length) return 1;
		else if (i.iface.length < ii.iface.length) return -1; else
    	if (i.iface > ii.iface)
			return 1;
		else if (i.iface < ii.iface)
			return -1;
		else
			return 0;
	}
	ifaces.sort(sortfunc);
	$.each(ifaces, function(num, ifaceInfo) {
		var iface = ifaceInfo.iface;
		if (config.getParsed($.sprintf("sys_demon_%s_enable", iface)) == 1)
		{
			var name = config.getParsed($.sprintf("sys_demon_%s_name", iface));
			
			page.addTab({
				"id": $.sprintf("terminal%s", iface),
				"name": $.sprintf("%s(%s)", name, iface),
				"func": function()
				{
					var cmd2 = "";
					var p = page.getRaw($.sprintf("terminal%s", iface));
					consoleDivs[iface] = $.create("div", {"id": "consoleDiv", "className": "pre scrollable","tabindex": "0"}, "").appendTo(p);

					cursors[iface] = $.create("span", {"id": "consoleCursor"}, "_").appendTo(consoleDivs[iface]);
					
					if (buf[iface] == undefined) buf[iface] = "";
					cursors[iface].before(buf[iface]);
					cmdSpans[iface] = $.create("span").insertBefore(cursors[iface]);
					$.create("span", {"id": $.sprintf("bottomAnchor%s", iface)}, "&nbsp;").appendTo(p);

					var onKeypress = function(src) {
						consoleDivs[iface].scrollTo('100%', 0);
						var ch;
//						alert(src.keyCode);

						// Tab
						if (src.keyCode == 9)
						{
							block2 = 1;
							config.cmdExecute({
								"cmd": $.sprintf("/sbin/tbuffctl -p%s -t \"%s\"", iface, cmd2),
							});
						
							setTimeout(function () {
								config.cmdExecute({
									"cmd": $.sprintf("/sbin/tbuffctl -p%s -r 0", iface),
									"callback": function(data) {
										var str = new String(data);
										alert(str.length);
										cmd2 = "";//data;
										cmdSpans[iface].text(data);
										block2 = 0;
									}
								});
								config.cmdExecute({
									"cmd": $.sprintf("/sbin/tbuffctl -p%s -d", iface),
								});

							}, 50);

							
							return false;
						}
						// Enter
						if (src.keyCode == 13) {
							if (!block[iface])
							{
							block[iface] = 1;
						
							if (buf[iface] == undefined) buf[iface] = cmd2; else buf[iface] += cmd2;
							buf[iface] += "<br>";
							if (buf[iface].length > 40*1024) buf[iface] = buf[iface].substring(buf[iface].length - 40*1024, buf[iface].length);
							t0 = new Date().getTime();
							config.cmdExecute({
								"cmd": $.sprintf("/sbin/tbuffctl -p%s -w \"%s\"", iface, cmd2),
								"callback": function(data) {
									cursors[iface].before("<br>");
									cmdSpans[iface] = $.create("span").insertBefore(cursors[iface]);
								}
							});
							
							}
							
							t1 = new Date().getTime();
							cmd2 = "";
							return false;
							
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
						if (ch != undefined) cmd2 += ch;
						return false;
					};
					var onBackspace = function() {
						cmd2 = cmd2.substring(0, cmd2.length - 1);
						cmdSpans[iface].text(cmd2);
						return false;
					};
					var onclick = function() {
						consoleDivs[iface].scrollTo('100%', 0);
					};
					setTimeout(function() {
						consoleDivs[iface].scrollTo('100%', 0);
						consoleDivs[iface].focus();
					}, 10);
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
