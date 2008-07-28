function Config() {
	this.confTmp;
	this.conf = new Array();
	
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
}

Config.prototype.ParseRawKDB = function (data) {
	var str = data.split("\n");
	var line, name, value;
	for (var i = 0; i < str.length; i++) {
		line = str[i];
		var eqindex=line.indexOf("=");
		if ( line == "KDB" || eqindex == -1 || line =="" ) continue;
		name = line.substring(0, eqindex);
		value = line.substring(eqindex+1);
		this.conf.push({name: name, value: value});
	}
};

Config.prototype.LoadKDB = function (params) {
	var url = "kdb/kdb_load.cgi";
	if (params) {
		if (params.url) url = params.url;
	}
	this.ParseRawKDB( $.ajax({
		type: "GET",
		url: url,
		dataType: "text",
		async: false
	}).responseText );
}

Config.prototype.Get = function (name) {
	for (var i = 0; i < this.conf.length; i++) {
		if ( this.conf[i].name == name ) 
			return this.conf[i].value;
	}
	return "";
}

Config.prototype.Save = function (val) {
	for (var i = 0; i < this.conf.length; i++) {
		if ( this.conf[i].name == "sys_hostname" ) 
			this.conf[i].value = val;
	}
	$.get(
		'kdb/kdb_save.cgi',
		{hostname: val}
	);
}

/* current settings */
var config = new Config();
config.LoadKDB( );
