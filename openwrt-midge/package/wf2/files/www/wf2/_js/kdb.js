/* Object for commands' queue */
function KDBQueue() {
	this.queue = new Array();
	this.block = false;
	
	/* set info message */
	this.setMessage = function(msg) {
		$("#kdb").html(msg);
	};
	
	/* set error message */
	this.setError = function(msg) {
		$("#kdb").html("<span style='color: red'>" + msg + "</span>");
	};
	
	/*
	 * Update info message.
	 * I18N for message.
	 */
	this.updateMessage = function() {
		if (this.queue.length > 0) {
			var word = this.queue.length == 1 ? _("task") : _("tasks");
			this.setMessage(this.queue.length + " " + word + " " + _("in queue"));
		} else {
			this.setMessage("&nbsp;");
		}
	};
	
	/*
	 * Add task to queue.
	 * 
	 * values — values for sending to router.
	 * timeout — request timeout.
	 * reload — reload page after finishing request (on success).
	 * onSuccess — callback on success.
	 * kdbCmd — command for KDB. If not set — it saves values,
	 * if set to "lrm" — removes key from KDB with name passed in parameter "item".
	 */
	this.addTask = function(values, timeout, reload, onSuccess, kdbCmd) {
		/* check if queue is blocked */
		if (this.block) return;
		
		/* create task and add it to queue */
		task = {
			"values": values,
			"timeout": timeout,
			"reload": reload,
			"onSuccess": onSuccess,
			"kdbCmd": kdbCmd
		};
		this.queue.push(task);
		
		/* if after task's completion we need to reload page — block queue */
		if (reload) {
			this.block = true;
		}
		
		/* try to start task */
		this.runTask();
	};
	
	/* if nothing is running — run task from queue */
	this.runTask = function() {
		this.updateMessage();
		if (this.queue.length == 1) {
			this.sendTask(this.queue[0]);
		}
	};
	
	/* run next task in queue */
	this.nextTask = function() {
		this.updateMessage();
		if (this.queue.length > 0) {
			this.sendTask(this.queue[0]);
		}
	};
	
	/* send task to router */
	this.sendTask = function(task) {
		var outer = this;
		var url;
		
		/* decide what to do */
		switch (task['kdbCmd']) {
			case "lrm":
				url = "sh/kdb_del_list.cgi";
				break;
			case "rm":
				url = "sh/kdb_del.cgi";
				break;
			default:
				url = "sh/kdb_save.cgi";
				break;
		}
		
		var options = {
			/* if kdbCmd is lrm, call kdb_del_list.cgi */
			"url": url,
			"type": "POST",
			"data": task['values'],
			
			/* network error */
			"error": function() {
				/* unblock queue */
				outer.block = false;
				
				/* clear task's queue */
				outer.queue = new Array();
				
				outer.setError("Connection error. You should reload page");
			},
			
			"success": function() {
				/* if task need page reloading — reload */
				if (task['reload']) {
					this.block = false;
					document.location.reload();
				}
				
				/* remove completed task from queue */
				outer.queue.shift();
				
				outer.updateMessage();
				
				/* if callback is set — call it */
				if (task['onSuccess']) task['onSuccess']();
				
				/* run next task */
				outer.nextTask();
			}
		};
		
		/* if timeout is passed — set it */
		if (task['timeout']) {
			options['timeout'] = task['timeout'];
		}
		
		/* perform request */
		$.ajax(options);
	};
}

/*
 * Local config.
 */
function Config() {
	var online = true;
	var offlineMessage = _("Router is OFFLINE! Check that router is available via your network.");
	var kdbQueue = new KDBQueue();
	var cmdCache = new CmdCache();
	
	this.isOnline = function() {
		return online;
	};
	
	/*
	 * Submit task for execution.
	 * 
	 * fields — array with fields
	 * (e.g., [ { name: 'username', value: 'jresig' }, { name: 'password', value: 'secret' } ]).
	 * timeout — timeout for AJAX request.
	 * reload — reload page after request is finished.
	 * onSuccess — callback on request success.
	 */
	this.kdbSubmit = function(fields, timeout, reload, onSuccess) {
		/* if router is offline, show error message and do nothing */
		if (!online) {
			alert(offlineMessage);
			return;
		}
		
		this.saveVals(fields);
		
		/* encode fields with $.param() */
		kdbQueue.addTask($.param(fields), timeout, reload, onSuccess);
	};
	
	/*
	 * Delete list key.
	 * 
	 * item — key to delete.
	 * subsystem — subsystem to restart.
	 * timeout — timeout for request.
	 */
	this.kdbDelListKey = function(item, subsystem, timeout) {
		/* if router is offline, show error message and do nothing */
		if (!online) {
			alert(offlineMessage);
			return;
		}
		
		/* delete item for local KDB */
		this.delListKey(item);
		
		/* create fields for sending to router */
		fields = [
			{"name": "item", "value": item},
			{"name": "subsystem", "value": subsystem}
		];
		
		/* encode fields with $.param(), and set kdbCmd to "lrm" */
		kdbQueue.addTask($.param(fields), timeout, null, null, "lrm");
	};
	
	/*
	 * Save values to local KDB.
	 */
	this.saveVals = function(fields) {
		var outer = this;
		$.each(fields, function(num, field) {
			outer.conf[field['name']] = field['value'];
		});
	};
	
	/*
	 * Return KDB key's value with replaced special characters.
	 */
	this.get = function(name) {
		return this.conf[name] != undefined ? this.conf[name] : null;
	};
	
	/*
	 * Return OEM key's value with replaced special characters.
	 */
	this.getOEM = function(name) {
		return this.oem[name] != undefined ? this.oem[name] : null;
	};
	
	/*
	 * Return key's parsed value. Always returns array, even there is no such key.
	 * If key's string ends with "*", returns array with keys, where "*" replaced with number
	 * (this method is faster than getByRegexp).
	 * 
	 * name — field's name.
	 */
	this.getParsed = function(key) {
		/* if key have mask */
		if (key.search(/\*$/) != -1) {
			var values = new Object();
			key = key.replace(/\*$/g, "");
			for (var i = 0; ; i++) {
				/* add number to an end of key */
				var curKey = key + i;
				if (this.conf[curKey] != undefined) {
					values[curKey] = this.parseRecord(this.conf[curKey]);
				} else {
					return values;
				}
			}
		}
		return this.conf[key] != undefined ? this.parseRecord(this.conf[key]) : new Array();
	};
	
	/*
	 * Returns object with keys, that match the regexp.
	 * 
	 * regexp — regexp to match.
	 * parse — if true, returns parsed values, otherwise returns raw values.
	 */
	this.getByRegexp = function(regexp, parse) {
		var outer = this;
		var result = new Object();
		$.each(this.conf, function(key, value) {
			if (regexp.test(key)) {
				result[key] = parse ? outer.parseRecord(value) : value;
			}
		});
		return result;
	};
	
	/*
	 * Deletes key from local KDB.
	 */
	this.del = function(key) {
		delete this.conf[key];
	};
	
	/*
	 * Deletes keys from local KDB by regexp.
	 */
	this.delByRegexp = function(regexp) {
		var outer = this;
		$.each(this.conf, function(key, value) {
			if (regexp.test(key)) {
				delete outer.conf[key];
			}
		});
	};
	
	/*
	 * Delete list key from local KDB.
	 */
	this.delListKey = function(key) {
		/* delete key from local KDB */
		this.del(key);
		
		/* remove key list's index */
		key = key.replace(/[0-9]+$/g, "");
		
		/* rename remaining keys */
		for (var oldIdx = 0, newIdx = 0; ; oldIdx++) {
			if (this.conf[key + oldIdx] != undefined) {
				this.conf[key + newIdx] = this.conf[key + oldIdx];
				newIdx++;
			/* 
			 * if next index is also undefined and we've deleted key not 
			 * from the end — delete last key, because we copied it to previous index.
			 */
			} else if (this.conf[key + (oldIdx + 1)] == undefined && oldIdx != newIdx) {
				delete this.conf[key + (oldIdx - 1)];
				break;
			}
		}
	};
	
	/*
	 * Parse config file. Decodes KDB special characters.
	 * 
	 * data — data to parse.
	 * config — where to write parsed values.
	 */
	this.parseConfig = function(data, config) {
		var lines = data.split("\n");
		var outer = this;
		$.each(lines, function(name, line) {
			if (line == "KDB" || line.length == 0) return true;
			var record = line.split("=");
			if (record.length > 1) {
				/* remove double qoutes " at beginning and end of the line */
				var value = record[1].replace(/^"/g, "");
				value = value.replace(/"$/g, "");
				
				/* decode space,=,#, encoded by KDB's export */
				config[record[0]] = outer.replaceSpecialChars(value);
			}
		});
	};
	
	/* 
	 * Parse record from KDB. If it consist of several variables — return array.
	 * Variables are separated by '\040', space character or by '\n'.
	 * 
	 * record — record to parse.
	 */
	this.parseRecord = function(record) {
		var parsedRecord = new Array();
		
		if (record.length == 0) return parsedRecord;
		
		/* \040 is a " " symbol. split records by space or new line */
		var variableSet = record.split(/\\040|\\n| /);
		
		/* if we have single variable in the record, add it to the array and return */
		if (variableSet.length == 1) {
			parsedRecord.push(record);
			return parsedRecord;
		}
		
		/* parse every variable in record */
		$.each(variableSet, function(name, value) {
			/* \075 is a "=" symbol */
			var variable = value.split(/\\075|=/);
			/* if we have only value */
			if (variable.length == 1) {
				parsedRecord.push(value);
			/* if we have key and value like key=value */
			} else {
				parsedRecord[variable[0]] = variable[1];
			}
		});
		return parsedRecord;
	};
	
	/*
	 * Replaces special KDB characters with next characters:
	 * \040 — ' ';
	 * \075 — '=';
	 * \043 — '#';
	 */
	this.replaceSpecialChars = function(value) {
		value = value.replace(/\\040|\\n/g, " ");
		value = value.replace(/\\043/g, "#");
		return value.replace(/\\075/g, "=");
	};
	
	/*
	 * Load KDB file from router.
	 */
	this.loadKDB = function() {
		this.conf = new Object();
		var url = "sh/kdb_load.cgi";
		this.parseConfig($.ajax({
			type: "GET",
			url: url,
			dataType: "text",
			async: false
		}).responseText, this.conf);
	};
	
	/*
	 * Updates specified key's values from router's KDB.
	 * 
	 * keys — keys to update.
	 */
	this.updateValues = function(keys) {
		/* prepare KDB command */
		var kdbArg = "";
		$.each(keys, function(num, key) {
			kdbArg += $.sprintf("get %s : ", key);
		});
		
		/* execute command */
		var newVals = cmdExecute($.sprintf("/usr/bin/kdb %s", kdbArg), {"sync": true}).split("<br>");
		
		/* update keys in local KDB with new values */
		var conf = this.conf;
		$.each(keys, function(num, key) {
			conf[key] = newVals[num];
		});
	};
	
	/*
	 * Load OEM file from router.
	 */
	this.loadOEM = function() {
		this.oem = new Object();
		var url = "sh/oem_load.cgi";
		this.parseConfig($.ajax({
			type: "GET",
			url: url,
			dataType: "text",
			async: false
		}).responseText, this.oem);
	};
	
	/*
	 * Update list of network interfaces.
	 * Private function.
	 */
	var outer = this;
	var updateIfaces = function() {
		/* get "valid" records for interfaces */
		var validIfaces = config.getByRegexp(/(sys_iface_)*(_valid)/);
		
		/* create sorted array with interfaces */
		var ifaces = new Array();
		$.each(validIfaces, function(key, value) {
			/* push to array only interface name */
			key = key.replace(/sys_iface_/, "");
			ifaces.push(key.replace(/_valid\w*/, ""));
		});
		ifaces = $.unique(ifaces);
		ifaces.sort();
		ifaces = ifaces.toString().replace(/,/g, " ");

		/* create data for submission */		
		var ifacesProp = new Array();
		$.addObjectWithProperty(ifacesProp, "sys_ifaces", ifaces);
		outer.kdbSubmit(ifacesProp);
		
		/* update menu */
		generateMenu();
	};
	
	/*
	 * Add new interface to KDB.
	 * 
	 * options — interface parameters.
	 */
	this.addIface = function(options) {
		/* if router is offline, show error message and do nothing */
		if (!online) {
			alert(offlineMessage);
			return;
		}
		
		var outer = this;
		
		/* return next interface name for given protocol */
		var getNextIface = function(proto) {
			/* replace protocol with inteface name */
			proto = proto.replace("bonding", "bond");
			proto = proto.replace("bridge", "br");
			
			/* find max interface index */
			var maxIdx = -1;
			var ifaces = outer.getParsed("sys_ifaces");
			$.each(ifaces, function(num, iface) {
				if (iface.search(proto) != -1) {
					var idx = parseInt(iface.replace(proto, ""));
					if (idx > maxIdx) maxIdx = idx;
				}
			});
			
			/* return next interface's name */
			return proto + (maxIdx + 1);
		};
		
		/* get interface name */
		var iface = options['iface'] ? options['iface'] : getNextIface(options['proto']);
		
		/* create interface parameters */
		var ifaceProp = new Array();
		$.addObjectWithProperty(ifaceProp, $.sprintf("sys_iface_%s_proto", iface), options['proto']);
		$.addObjectWithProperty(ifaceProp, $.sprintf("sys_iface_%s_real", iface),
			options['real'] ? options['real'] : iface);
		$.addObjectWithProperty(ifaceProp, $.sprintf("sys_iface_%s_depend_on", iface),
			options['dependOn'] ? options['dependOn'] : "none");
		$.addObjectWithProperty(ifaceProp, $.sprintf("sys_iface_%s_valid", iface), "1");
		$.addObjectWithProperty(ifaceProp, $.sprintf("sys_iface_%s_auto", iface), "0");
		$.addObjectWithProperty(ifaceProp, $.sprintf("sys_iface_%s_method", iface), "none");
		if (options['vlanId']) {
			$.addObjectWithProperty(ifaceProp, $.sprintf("sys_iface_%s_vlan_id", iface),
				options['vlanId']);
		}
		
		this.kdbSubmit(ifaceProp);
		updateIfaces();
	};
	
	/*
	 * Delete network interface from KDB.
	 * 
	 * iface — interface to delete.
	 */
	this.delIface = function(iface) {
		/* if router is offline, show error message and do nothing */
		if (!online) {
			alert(offlineMessage);
			return;
		}
		
		/* delete all interface parameters from local KDB */
		this.delByRegexp(new RegExp($.sprintf("sys_iface_%s_\w*", iface)));
		
		/* create data for submission */		
		var submitData = new Array();
		$.addObjectWithProperty(submitData, "item", $.sprintf("sys_iface_%s_*", iface));
		$.addObjectWithProperty(submitData, "subsystem", $.sprintf("iface_del.%s", iface));
		
		/* encode data with $.param(), and set kdbCmd to "rm" */
		kdbQueue.addTask($.param(submitData), null, null, null, "rm");
		updateIfaces();
	};
	
	this.runCmd = cmdCache.runCmd;
	
	this.getCachedOutput = cmdCache.getCachedOutput;
	
	/*
	 * Start checking for router state (online or offline) every timeout seconds.
	 * If router is offline, show OFFLINE message above the menu.
	 */
	this.startCheckStatus = function(timeout) {
		var checkStatus = function() {
			var options = {
				"type": "POST",
				"url": "sh/execute.cgi",
				"dataType": "text",
				"data": {"cmd": "/bin/true"},
				"timeout": timeout * 1000,
				"success": function(data) {
					if (!online) {
						$("#kdb").html("&nbsp;");
						online = true;
					}
				},
				"error": function() {
					online = false;
					$("#kdb").html("<span id='offline' style='color: red'><b>Router is OFFLINE</b></span>");
				}
			};
			$.ajax(options);
		}
		
		setInterval(checkStatus, timeout * 1000);
	};
}

/*
 * Cache for command's output.
 */
function CmdCache() {
	var cache = new Object();
	
	/*
	 * Run asynchronously cmd and add it's output to cache.
	 */
	this.runCmd = function(cmd) {
		cmdExecute(cmd, {
			"callback": function(data) {
				cache[cmd] = data;
			}
		});
	};
	
	/*
	 * Get output of cmd.
	 */
	this.getCachedOutput = function(cmd) {
		return cache[cmd] ? cache[cmd] : cmdExecute(cmd, {"sync": true});
	};
};
