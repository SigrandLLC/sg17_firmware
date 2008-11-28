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
document.title = config.get("sys_hostname");

/* set interface lguage */
var lang = config.get("sys_interface_language");
if (lang != "en") {
	$("head").append("<link href='translation/" + lang + ".json' lang='" + lang + "' rel='gettext'/>");
}

$(document).ready(function() {
	$("#status").html(
		$.sprintf("%s: <b>%s</b>, %s: <b><span id='status_state'>%s</span></b>, %s: <b><span id='status_tasks'>%s</span></b>, %s: <b><span id='status_ajax'>%s</span></b>",
			_("Hostname"), config.get("sys_hostname"), _("status"), _("online"), _("tasks"), _("none"),
			_("ajax"), _("none"))
	);
	$("#status").attr("title",
		"<ul><li>Hostname - device's hostname;</li><li>Status - is router online or offline;</li><li>Tasks - number of performing and queuened tasks;</li><li>Ajax - number of active ajax requests.</li></ul>"
		).tooltip({"track": true});
	
	generateMenu();
	
	Controllers['info']();
});
