$("head").append("<link href='translation/ru.json' lang='ru' rel='gettext'/>");

/* current settings */
var config = new Config();
config.loadKDB( );

$(document).ready(function() {
	$('#container').tabs({fxAutoHeight: true});
	
	$("#menu").treeview(
	{
		persist: "location",
		unique: true,
		collapsed: true
	});
});
