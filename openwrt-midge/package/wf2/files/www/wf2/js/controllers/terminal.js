var timer = 0;
var buf = new Array(16);
var cursors = new Array(16);
var consoleDivs = new Array(16);
var cmdSpans = new Array(16);
var bufSpans = new Array(16);
var t0 = 0, t1 = 0, t2, t3;
var func_en = 0;
var requesttime = 300;
var cur_iface;
var isctrl = false;
var tab = "";
var tout;

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
                    "validator": {"alphanumU": true},
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

					if (config.getParsed($.sprintf("sys_demon_%s_enable", iface)) != 1)
					{
						$($.sprintf("#sys_demon_%s_name", iface)).val(iface);
						config.cmdExecute({"cmd": $.sprintf("kdb set sys_demon_%s_name=%s", iface, iface), "async" : false});
						buf[iface] = "";
					}
					config.loadKDB();
					config.loadOEM();
				});
				Controllers["terminal"]();
			},
			 "reload" : false});
        }
    });

    var ifaces = config.getData(config.getOEM("MR17S_DRVNAME"));
	func_en = 0;
	$.each(ifaces, function(num, ifaceInfo) {
		var iface = ifaceInfo.iface;
		if (config.getParsed($.sprintf("sys_demon_%s_enable", iface)) == 1) func_en = 1;
	});

	function getXmlHttp()
	{
		var xmlhttp = window.ActiveXObject ? new ActiveXObject("Microsoft.XMLHTTP") : new XMLHttpRequest();
		return xmlhttp;
	}

	var scrollTab = function() {
		var obj = document.getElementById('consoleDiv');
		if (obj) obj.scrollTop = obj.scrollHeight;
	};

	var parse_answer = function(data, who) {
		var str = new String(data);
		var str2 = new String("");
		var i, j = 0;
		who = '';
//		$("#status").html($("#status").html()+" from: "+who+"  data: ");
		for (i = 0; i < str.length; i++)
		{
//			$("#status").html($("#status").html()+" "+str.charCodeAt(i)+"["+str.charAt(i)+"]");
			switch (str.charCodeAt(i)) {
				case 8:
					if (j > 0)
					{
						if (str2.substring(str2.length - 6 , str2.length) == '&nbsp;')
						{
							str2 = str2.substring(0, str2.length - 6);
							j -=5 ;
						} else {
							if ((1040 <= str2.charCodeAt(str2.length-1)) && (str2.charCodeAt(str2.length-1) <= 1103)
								|| (str2.charCodeAt(str2.length-1) == 1025) || (str2.charCodeAt(str2.length-1) == 1105))
							{
								var xmlhttp1 = getXmlHttp();
								cmd = $.sprintf("sh/tbuffctl?%s;c;8", cur_iface);
								xmlhttp1.open('GET', cmd, false);
								xmlhttp1.setRequestHeader("If-Modified-Since", "Sat, 1 Jan 2000 00:00:00 GMT");
								xmlhttp1.send(null);
							}
							str2 = str2.substring(0, str2.length - 1);
							j--;
						}
					} else {
						var str3 = new String(bufSpans[cur_iface].html());
						if (str3.substring(str3.length - 6 , str3.length) == '&nbsp;')
							bufSpans[cur_iface].html(str3.substring(0, str3.length - 6));
						else
							{
								if ((1040 <= str3.charCodeAt(str3.length-1)) && (str3.charCodeAt(str3.length-1) <= 1103)
									|| (str3.charCodeAt(str3.length-1) == 1025) || (str3.charCodeAt(str3.length-1) == 1105))
								{
									var xmlhttp1 = getXmlHttp();
									cmd = $.sprintf("sh/tbuffctl?%s;c;8", cur_iface);
									xmlhttp1.open('GET', cmd, false);
									xmlhttp1.setRequestHeader("If-Modified-Since", "Sat, 1 Jan 2000 00:00:00 GMT");
									xmlhttp1.send(null);
								}
								bufSpans[cur_iface].html(str3.substring(0, str3.length - 1));
							}
						buf[cur_iface] = bufSpans[cur_iface].html();
					}
				break;
				case 10:
					str2 += "<br>";
					j++;
				break;
				case 13:
					if (str.charCodeAt(i+1) == 10) break;
					if (str.length == 1) break;
					var str321 = new String(bufSpans[cur_iface].html());
					var substr = new String("<BR>");
					if (str321.lastIndexOf(substr) != -1) {
						bufSpans[cur_iface].html(str321.substring(0, str321.lastIndexOf(substr) + 4));
						buf[cur_iface] = bufSpans[cur_iface].html();
					}
					var substr2 = new String("<br>");
					if (str321.lastIndexOf(substr2) != -1) {
						bufSpans[cur_iface].html(str321.substring(0, str321.lastIndexOf(substr2) + 4));
						buf[cur_iface] = bufSpans[cur_iface].html();
					}
				break;
				case 7:
				break;
				case 32:
					str2 += '&nbsp;';
					j += 6;
				break;
//				case 45:
//					str2 += "-";
//					j++;
//				break;
				default:
					if (str.charCodeAt(i) > 31) {
						str2 += str.charAt(i);
					}
					j++;
			}
		}
		if (j > 0)
		{
			if (bufSpans[cur_iface] != null)
			{
				bufSpans[cur_iface].append(str2);
				buf[cur_iface] = bufSpans[cur_iface].html();
				scrollTab();
			}
		}
//		$("#status").html($("#status").html()+"<br>");
	};


	var func = function(param) {
		config.cmdExecute({
			"cmd": "/sbin/tbuffctl -p* -a",
			"async": false,
			"dataType" : "json",
			"formatData": true,
			"status" : false,
			"callback": function(data) {
				if (typeof data == "object")
				{
					$.each(data.ttylist, function(index, value) {
						if ((value.text != "") && (typeof value == "object"))
						{
							cur_iface = value.dev;
							parse_answer(value.text, "func");
						}
					});
				}
			}
		});
		if (param != true)
		{
			if (func_en == 1) {
				tout = setTimeout(func, requesttime);
			} else timer = 0;
		}
	};

	if ((timer == 0) && (func_en == 1))
	{
		tout = setTimeout(func, requesttime+2000);
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
					tab = iface;
					var p = page.getRaw($.sprintf("terminal%s", iface));
					consoleDivs[iface] = $.create("div", {"id": "consoleDiv", "className": "pre scrollable","tabindex": "0"}, "").appendTo(p).focus();

					cursors[iface] = $.create("span", {"id": "consoleCursor"}, "_").appendTo(consoleDivs[iface]);
					bufSpans[iface] = $.create("span", {"id": $.sprintf("bufSpan%s", iface)}, "").insertBefore(cursors[iface]);
					if (buf[iface] == undefined) buf[iface] = "";
					bufSpans[iface].html(buf[iface]);

					var onKeypress = function(src) {
/*
						var i;
						for (i = 0; i < 256; i++)
								$("#status").html($("#status").html()+i+" - "+String.fromCharCode(i)+"<br>");
						return 0;
*/
//						clearTimeout(tout);
						var ch;
						if (src.which == null) {
							/* IE */
							ch = src.keyCode;
						} else if (src.which > 0) {
							/* others */
							ch = src.which;
						}
//						alert("which = "+src.which+" keyCode = "+src.keyCode);
						var str123 = "";

						if (isctrl == true)
						{
//							alert("crtl");
							src.which = undefined;
							if (src.keyCode>=65 && src.keyCode<=90) str123 = String.fromCharCode(src.keyCode-64); // Ctrl-A..Z
							else if (src.keyCode>=97 && src.keyCode<=122) str123 = String.fromCharCode(src.keyCode-96); // Ctrl-A..Z
							else if (src.keyCode==54) str123 = String.fromCharCode(30);  // Ctrl-^
							else if (src.keyCode==109) str123 = String.fromCharCode(31); // Ctrl-_
							else if (src.keyCode==219) str123 = String.fromCharCode(27); // Ctrl-[
							else if (src.keyCode==220) str123 = String.fromCharCode(28); // Ctrl-\
							else if (src.keyCode==221) str123 = String.fromCharCode(29); // Ctrl-]
							else if (src.keyCode==219) str123 = String.fromCharCode(29); // Ctrl-]
							else if (src.keyCode==219) str123 = String.fromCharCode(0);  // Ctrl-@
							else src.which = 1;
						} else
						switch (src.keyCode)
						{
							case 33:
								str123 = String.fromCharCode(27)+"[5~"; //PgUp
							break;
							case 34:
								str123 = String.fromCharCode(27)+"[6~"; //PgDn
							break;
							case 35:
								str123 = String.fromCharCode(27)+"[4~"; // End
							break;
							case 36:
								str123 = String.fromCharCode(27)+"[1~"; // Home
							break;
							case 37:
								str123 = String.fromCharCode(27)+"[D"; // Left
							break;
							case 38:
								str123 = String.fromCharCode(27)+"[A"; // Up
							break;
							case 39:
								str123 = String.fromCharCode(27)+"[C"; // Right
							break;
							case 40:
								str123 = String.fromCharCode(27)+"[B"; // Down
							break;
							case 45:
								str123 = String.fromCharCode(27)+"[2~"; // Ins
							break;
							case 46:
								str123 = String.fromCharCode(27)+"[3~"; // Del
							break;
							case 112:
								str123 = String.fromCharCode(27)+"[[A"; //F1
							break;
							case 113:
								str123 = String.fromCharCode(27)+"[[B";
							break;
							case 114:
								str123 = String.fromCharCode(27)+"[[C";
							break;
							case 115:
								str123 = String.fromCharCode(27)+"[[D";
							break;
							case 116:
								str123 = String.fromCharCode(27)+"[[E";
							break;
							case 117:
								str123 = String.fromCharCode(27)+"[17~";
							break;
							case 118:
								str123 = String.fromCharCode(27)+"[18~";
							break;
							case 119:
								str123 = String.fromCharCode(27)+"[19~";
							break;
							case 120:
								str123 = String.fromCharCode(27)+"[20~";
							break;
							case 121:
								str123 = String.fromCharCode(27)+"[21~";
							break;
							case 122:
								return false;
								str123 = String.fromCharCode(27)+"[22~";
							break;
							case 123:
								str123 = String.fromCharCode(27)+"[23~"; // F12
							break;
						}

						if (ch > 1000)
						{
							src.which = undefined;
							str123 = ""+String.fromCharCode(ch);
						}

						if ((str123 != "") && (src.which == undefined))
						{
							cmd = $.sprintf("/sbin/tbuffctl -p%s -w \"%s\"", iface, str123);
							cur_iface = iface;
							config.cmdExecute({
								"cmd": cmd,
								"async": false,
								"callback": parse_answer
							});
//							func();
							return false;
						}

						var xmlhttp = getXmlHttp();
						cmd = $.sprintf("sh/tbuffctl?%s;c;%s", iface, ch);
						xmlhttp.open('GET', cmd, false);
						xmlhttp.setRequestHeader("If-Modified-Since", "Sat, 1 Jan 2000 00:00:00 GMT");
						xmlhttp.send(null);
						cur_iface = iface;
						if (xmlhttp.status == 200) {
							parse_answer(xmlhttp.responseText, "ch");
							if (xmlhttp.responseText == "")
							{
//								if ((ch >= 32) && (ch <= 126) || (ch == 10))
//								parse_answer(String.fromCharCode(ch));
//								alert("empty answer");
							}
						} else {
//							if ((ch >= 32) && (ch <= 126) || (ch == 10))
//							parse_answer(String.fromCharCode(ch));
//							alert("status"+xmlhttp.status);
						}
//						func(true);
						scrollTab();
						return false;
					};

					consoleDivs[iface].keypress(onKeypress);

					consoleDivs[iface].keydown(function(src) {
						var k = src.keyCode;
						if (src.which == 17) {
							isctrl = true;
//							alert("true");
						}
						if (isctrl == true && src.which >= 65 && src.which <= 90) onKeypress(src);
						if ((k == 8) || (k == 9) || (k == 27) || (k == 33) || (k == 34) || (k == 35) || (k == 36) || (k == 37) || (k == 38)
						|| (k == 39) || (k == 40) || (k == 45) || (k == 46) || (k == 112) || (k == 113) || (k == 114) || (k == 115) || (k == 116)
						|| (k == 117) || (k == 118) || (k == 119) || (k == 120) || (k == 121) || (k == 122) || (k == 123))
						{
							src.which = undefined;
							onKeypress(src);
							return false;
						}
					});
					consoleDivs[iface].keyup(function(src) {
						if (src.which == 17) {
							isctrl = false;
//							alert("false")
						}
					});
					
					setTimeout(function () {
						scrollTab();
						$("#consoleDiv").focus();
						$("#consoleDiv").blur(function(){$("#consoleDiv").focus();});
					}, 10);					
  				} //func
			}); //addTab
		} // if
	}); //each

    page.generateTabs();

};
