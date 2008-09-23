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
	
	/* add task to queue */
	this.addTask = function(values, timeout, reload, onSuccess) {
		/* check if queue is blocked */
		if (this.block) return;
		
		/* create task and add it to queue */
		task = {
			"values": values,
			"timeout": timeout,
			"reload": reload,
			"onSuccess": onSuccess
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
		var options = {
			url: "sh/kdb_save.cgi",
			type: "POST",
			data: task['values'],
			
			/* network error */
			error: function() {
				/* unblock queue */
				outer.block = false;
				
				/* clear task's queue */
				outer.queue = new Array();
				
				outer.setError("Connection error. You should reload page");
			},
			
			success: function() {
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
	this.kdbQueue = new KDBQueue();
	
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
		this.saveVals(fields);
		
		/* encode fields with $.param() */
		this.kdbQueue.addTask($.param(fields), timeout, reload, onSuccess);
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
		return this.conf[name] != undefined ? this.replaceSpecialChars(this.conf[name]) : null;
	};
	
	/*
	 * Return OEM key's value with replaced special characters.
	 */
	this.getOEM = function(name) {
		return this.oem[name] != undefined ? this.replaceSpecialChars(this.oem[name]) : null;
	};
	
	/*
	 * Return key's parsed value. Always returns array, even there is no such key.
	 * If key's string ends with "*", returns array with keys, where "*" replaced with number.
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
	 * Parse config file.
	 * data — data to parse.
	 * config — where to write parsed values.
	 */
	this.parseConfig = function(data, config) {
		var lines = data.split("\n");
		$.each(lines, function(name, line) {
			if (line == "KDB" || line.length == 0) return true;
			var record = line.split("=");
			if (record.length > 1) {
				/* remove double qoutes " at beginning and end of the line */
				var value = record[1].replace(/^"/g, "");
				value = value.replace(/"$/g, "");
				
				config[record[0]] = value;
			}
		});
	};
	
	/* 
	 * Parse record from KDB. If it consist of several variables — return array.
	 * Variables are separated by '\040' character or by '\n'.
	 * 
	 * record — record to parse.
	 */
	this.parseRecord = function(record) {
		var parsedRecord = new Array();
		
		if (record.length == 0) return parsedRecord;
		
		/* \040 is a " " symbol */
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
	 * \040 — ' '
	 * \075 — '='
	 */
	this.replaceSpecialChars = function(value) {
		var str1 = value.replace(/\\040|\\n/g, " ");
		return str1.replace(/\\075/g, "=");
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
}
