
function Config() {
	this.conf=new Array();
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

config = new Config();
config.LoadKDB( );
