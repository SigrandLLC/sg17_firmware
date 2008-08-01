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
		var par;
		var idx;
		var name;
		var value;
		
		for (var key in data) {
			par = data[key];
			idx = par.indexOf("=");
			name = par.substring(0, idx);
			value = par.substring(idx + 1);
			parsed[name] = value;
		}
		
		return parsed;
	};
	
	/* return key's value */
	this.get = function(name) {
		return this.conf[name] ? this.conf[name] : "";
	};
	
	/* parse KDB file */
	this.parseRawKDB = function(data) {
		var str = data.split("\n");
		var line;
		var name;
		var value;

		for (key in str) {
			line = str[key];
			var eqindex = line.indexOf("=");
			if (line == "KDB" || eqindex == -1 || line == "") continue;
			name = line.substring(0, eqindex);
			value = line.substring(eqindex + 1);
			this.conf[name] = value;
		}
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
