/* load KDB settings */
var config = new Config();
config.loadKDB( );

/* set page title to router's hostname */
document.title = config.get("sys_hostname") ? config.get("sys_hostname") : "";

/* set interface language */
var lang = config.get("sys_interface_language");
if (lang != "en") {
	$("head").append("<link href='translation/" + lang + ".json' lang='" + lang + "' rel='gettext'/>");
}

$(document).ready(function() {
	generateMenu();
	
	$("#menu").treeview(
	{
		persist: "location",
		unique: true,
		collapsed: true
	});
});
