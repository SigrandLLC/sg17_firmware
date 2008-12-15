/* WF2 logs */
var wf2Logs = new function() {
	var logsNum = 10;
	var logsMax = 15;
	this.logs = [];

	this.addLog = function(title, text) {
		this.logs.push({"title": title, "text": text});
		
		/* rotate log */
		if (this.logs.length > logsMax) {
			this.logs = this.logs.slice(this.logs.length - logsNum);
		}
	};
};

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

/* get availability for context-help */
config.runCmd("[ -r '/www/help/index.html' ] && echo -n 1 || echo -n 0", "context-help");

/* check router every 10 seconds */
config.startCheckStatus(10);

/* set page title to router's hostname */
document.title = config.get("sys_hostname");

/* set interface lguage */
var lang = config.get("sys_interface_language");
if (lang != "en") {
	$("head").append("<link href='translation/" + lang + ".json' lang='" + lang + "' rel='gettext'/>");
}

$(document).ready(function() {
	/* on click on status bar with CTRL key, show debug */
	$("#status").click(function(e){
		if (e.ctrlKey == true) {
			setTimeout(function() {
				$(".tabs-container").empty();
				scrollTo(0, 0);
				
				Controllers.debug();
			}, 10);
		}
	});
	
	/* add status bar contetnts */
	$("#status").html(
		$.sprintf("%s: <b>%s</b>, %s: <b><span id='status_state'>%s</span></b>, %s: <b><span id='status_tasks'>%s</span></b>, %s: <b><span id='status_ajax'>%s</span></b>",
			_("Hostname"), config.get("sys_hostname"), _("status"), _("online"), _("tasks"), _("none"),
			_("ajax"), _("none"))
	);
	
	/* add status bar tip */
	$("#status").attr("title",
		"<ul><li>Hostname - device's hostname;</li><li>Status - is router online or offline;</li><li>Tasks - number of performing and queuened tasks;</li><li>Ajax - number of active ajax requests.</li></ul>"
		).tooltip({"track": true});
	
	generateMenu();
	
	/* call info controller when all config.runCmd will be finished */
	config.onCmdCacheFinish(function() {
		Controllers.info();
	});
});
