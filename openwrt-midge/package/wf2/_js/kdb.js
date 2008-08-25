function Config() {
	this.confTmp;
	this.conf = new Object();
	
	/* before Ajax request, saves new settings in temporary object */
	this.saveTmpVals = function(form) {
		var values = $(form).formSerialize();
		this.confTmp = this.parseUrl(values);
	};
	
	/* after Ajax request, copy new settings to current config object */
	this.saveVals = function(form) {
		for (var name in this.confTmp) {
			this.conf[name] = this.confTmp[name];
		}
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
