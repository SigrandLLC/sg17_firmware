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
	this.addTask = function(values, timeout, reload) {
		/* check if queue is blocked */
		if (this.block) return;
		
		/* create task and add it to queue */
		task = {
			'values': values,
			'timeout': timeout,
			'reload': reload
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
		/* save values to temporary array */
		config.saveTmpVals(task['values']);
		
		var outer = this;
		var options = {
			url: "kdb/kdb_save.cgi",
			type: "POST",
			data: task['values'],
			
			/* network error */
			error: function() {
				/* clear temporary array */
				config.removeTmpVals();
				
				/* unblock queue */
				outer.block = false;
				
				/* clear task's queue */
				outer.queue = new Array();
				
				outer.setError("Connection error");
			},
			
			success: function() {
				/* if task need page reloading — reload */
				if (task['reload']) {
					this.block = false;
					document.location.reload();
				}
				
				/* save values from temporary array in current KDB object */
				config.saveVals();
				
				/* remove completed task from queue */
				outer.queue.shift();
				
				outer.updateMessage();
				
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

function Config() {
	this.confTmp;
	this.conf = new Object();
	this.kdbQueue = new KDBQueue();
	
	/* submit task for execution */
	this.kdbSubmit = function(form, timeout, reload) {
		var values = $(form).formSerialize();
		this.kdbQueue.addTask(values, timeout, reload);
	};
	
	/* before Ajax request, saves new settings in temporary object */
	this.saveTmpVals = function(values) {
		this.confTmp = this.parseUrl(values);
	};
	
	/* after Ajax request, copy new settings to current config object */
	this.saveVals = function() {
		for (var name in this.confTmp) {
			this.conf[name] = this.confTmp[name];
		}
		this.removeTmpVals();
	};
	
	this.removeTmpVals = function() {
		delete this.confTmp;
	};
	
	/* parse URL (par1=val1&par2=val2), return hash */
	this.parseUrl = function(url) {
		var parsed = new Object();
		var data = url.split("&");
		
		$.each(data, function(name, value) {
			var variable = value.split("=");
			parsed[variable[0]] = variable[1];
		});
		
		return parsed;
	};
	
	/*
	 * Return key's value with replaced special characters.
	 */
	this.get = function(name) {
		return this.conf[name] ? this.replaceSpecialChars(this.conf[name]) : null;
	};
	
	/*
	 * Return key's parsed value.
	 */
	this.getParsed = function(name) {
		return this.conf[name] ? this.parseRecord(this.conf[name]) : null;
	};
	
	/* parse KDB file */
	this.parseRawKDB = function(data) {
		/* we need the context of this function in each's callback */
		var outer = this;
		
		var lines = data.split("\n");
		$.each(lines, function(name, line) {
			if (line == "KDB" || line.length == 0) return true;
			var record = line.split("=");
			if (record.length > 1) {
				outer.conf[record[0]] = record[1];
			}
		});
	};
	
	/* 
	 * Parse record from KDB. If it consist of several variables — return array.
	 * Variables are separated by '\040' character or by '\n'.
	 */
	this.parseRecord = function(record) {
		var parsedRecord = new Array();
		/* \040 is a " " symbol */
		var variableSet = record.split(/\\040|\\n/);
		
		/* if we have single variable in the record — simply return it */
		if (variableSet.length == 1) return record;
		
		/* parse every variable in record */
		$.each(variableSet, function(name, value) {
			/* \075 is a "=" symbol */
			var variable = value.split("\\075");
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
		var str1 = value.replace(/\\040|\\n/, ' ');
		return str1.replace(/\\075/, '=');
	};
	
	/* load KDB file from server */
	this.loadKDB = function(params) {
		var url = "kdb/kdb_load.cgi";
		if (params) {
			if (params.url) url = params.url;
		}
		this.parseRawKDB($.ajax({
			type: "GET",
			url: url,
			dataType: "text",
			async: false
		}).responseText);
	};
}
