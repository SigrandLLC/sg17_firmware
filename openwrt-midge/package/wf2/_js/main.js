/* load KDB settings */
var config = new Config();
config.loadKDB( );

var lang = config.get("sys_interface_language");
if (lang != "en") {
	$("head").append("<link href='translation/" + lang + ".json' lang='" + lang + "' rel='gettext'/>");
}

$(document).ready(function() {
	$('#container').tabs({fxAutoHeight: true});
	
	$("#menu").treeview(
	{
		persist: "location",
		unique: true,
		collapsed: true
	});
});
