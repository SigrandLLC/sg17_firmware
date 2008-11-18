/*
 * Set AJAX options.
 * Without this option IE caches all AJAX requests.
 */
$.ajaxSetup({
	cache: false
});

/* load KDB settings */
var config = new Config();
config.loadKDB();
config.loadOEM();

/* load firmware version */
config.runCmd("/bin/cat /etc/version");

/* check router every 5 seconds */
config.startCheckStatus(5);

/* set page title to router's hostname */
document.title = config.get("sys_hostname") ? config.get("sys_hostname") : "";

/* set interface language */
var lang = config.get("sys_interface_language");
if (lang != "en") {
	$("head").append("<link href='translation/" + lang + ".json' lang='" + lang + "' rel='gettext'/>");
}

$(document).ready(function() {
	generateMenu();
	
	Controllers['info']();
});
