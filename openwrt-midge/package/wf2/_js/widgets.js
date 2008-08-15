/*
 * Add tabs to 'p' container.
 * tabs: hash, key — id, value — name of tab
 * options — optional settings for page, may include:
 * 	- subsystem — name of a subsystem for all tabs on the page.
 *  - help — name of html file with context help for all tabs on the page.
 * I18N for tab name.
 */
function pageTabs(p, tabsInfo, options) {
	/* clear page */
	$(p).empty();
	
	var tabsList = "<ul>";
	for (tab in tabsInfo) {
		tabsList += "<li><a href='#" + tab + "'><span>" + _(tabsInfo[tab]) + "</span></a></li>";
	}
	tabsList += "</ul>";
	$(tabsList).appendTo(p);
	this.tabs = new Array();
	for (tab in tabsInfo) {
		this.tabs[tab] = new TabContents($("<div id='" + tab + "'></div>").appendTo(p).get(), options);
	}
	
	/* update tabs */
	$(p).tabs({fxAutoHeight: true});
}

/*
 * Content of an one tab.
 * options — additional options for container.
 */
function TabContents(tab, options) {
	this.tab = tab;
	this.options = options;

	/*
	 * Adds container to tab.
	 * helpSection — if string, then this is a name of the section (e.g., "logging");
	 * 			   — if object, then - page: html page name,
	 * 								 - section: section
	 * 				 (e.g, {page: "logging", section: "asd"})
	 */
	this.addContainer = function(helpSection) {
		return new Container(this.tab, this.options, helpSection);
	}
	
	this.addBr = function() {
		$("<br />").appendTo(this.tab);
	}
}

/* show HTML page in popup window */
function popup(url) {
	 var width  = 608;
	 var height = 700;
	 var left   = (screen.width - width)/2;
	 var top    = (screen.height - height)/2;
	 var params = 'width='+width+', height='+height;
	 params += ', top='+top+', left='+left;
	 params += ', directories=no';
	 params += ', location=no';
	 params += ', menubar=no';
	 params += ', resizable=no';
	 params += ', scrollbars=1';
	 params += ', status=1';
	 params += ', toolbar=no';
	 newwin=window.open(url,'help', params);
	 if (window.focus) {newwin.focus()}
	 return false;
}

/*
 * Container for widgets
 * I18N for widgets.
 */
function Container(p, options, helpSection) {
	if (options && options.subsystem) this.subsystem = options.subsystem;
	this.validator_rules = new Object();
	this.validator_messages = new Object();
	this.info_message = "info_message_" + $(p).attr("id");
	if ($("div[id='" + this.info_message + "']").length == 0) {
		$("<div class='message'></div>").attr("id", this.info_message).appendTo(p);
	}
	this.form = $("<form action=''></form>").appendTo(p).get();
	this.table = $("<table id='conttable' cellpadding='0' cellspacing='0' border='0'></table>").appendTo(this.form).get();

	this.setSubsystem = function(subsystem) {
		this.subsystem = subsystem;
	}

	/* template for table title */
	this.table_title_tpl = function() {
		var url;
		if (helpSection && typeof helpSection == "object") {
			url = "popup('/help/" + helpSection.page + ".html#" + helpSection.section + "')";
		} else if (options && options.help) {
			url = helpSection ? "popup('/help/" + options.help + ".html#" + helpSection + "')" :
				"popup('/help/" + options.help + ".html')";
		} else {
			url = null;
		}
		var help = url ? " <a href='#' onclick=" + url + " class='helpLink'>[?]</a>" : "";
		return [
			'tr', {}, [
				'th', {colspan: '2'}, this.title + help
			]
		];
	};
	
	this.subsystem_tpl = function() {
		var attrs = {
						type: "hidden",
						name: "subsystem",
						value: this.subsystem
		};
		return ['input', attrs];
	};
	
	/* Common widget's template
	 * I18N for text, descr
	 */
	this.widget_tpl = function() {
		var description = this.descr ? "<br /><p>" + _(this.descr) + "</p>" : "";
		var required = (this.validator && this.validator['required']) ? " *" : "";
		
		return [
			'tr', {}, [
				'td', {className: 'tdleft'}, _(this.text) + required,
				'td', {id: 'td_' + this.name}, description
			]
		];
	};
	
	/* template for text widget */
	this.text_tpl = function() {
		var attrs = {
						type: 'text',
						name: this.name,
						value: config.get(this.name),
						size: '25'
		};
		this.id && (attrs.id = this.id);
		this.tip && (attrs.title = this.tip);
		return ['input', attrs];
	};
	
	/* template for password widget */
	this.password_tpl = function() {
		var attrs = {
						type: 'password',
						name: this.name,
						size: '25'
		};
		this.id && (attrs.id = this.id);
		return ['input', attrs];
	};
	
	/* template for checkbox widget */
	this.checkbox_tpl = function() {
		var attrs = {
						type: 'checkbox',
						name: this.name,
						className: 'check',
						value: 1
		};
		this.id && (attrs.id = this.id);
		this.tip && (attrs.title = this.tip);
		if (config.get(this.name) == "1") attrs['checked'] = true;
		return ['input', attrs];
	};
	
	/* template for select widget */
	this.select_tpl = function() {
		var attrs = { name: this.name };
		this.id && (attrs.id = this.id);
		this.tip && (attrs.title = this.tip);
		return ['select', attrs];
	};
	
	/* template for option
	 * I18N for name
	 */
	this.option_tpl = function() {
		var attrs = { optionValue: this.value};
		if (this.selected) attrs['selected'] = this.selected;
		return ['option', attrs, _(this.name)];
	};
	
	/* Adds title to table
	 * I18N for title
	 */
	this.addTitle = function(title) {
		var json = {title: _(title)};
		$(this.table).tplAppend(json, this.table_title_tpl);
	};

	/* add widget to container */
	this.addWidget = function(w) {
		/* add common widget's data */
		$(this.table).tplAppend(w, this.widget_tpl);
		switch (w.type) {
			case "text": 
				$('#td_' + w.name).tplPrepend(w, this.text_tpl);
				break;
			case "password": 
				$('#td_' + w.name).tplPrepend(w, this.password_tpl);
				break;
			case "checkbox":
				$('#td_' + w.name).tplPrepend(w, this.checkbox_tpl);
				break;
			case "select":
				$('#td_' + w.name).tplPrepend(w, this.select_tpl);
				for (var value in w.options) {
					var option = {
						value: value,
						name: w.options[value]
					};
					if (config.get(w.name) == option['value']) option['selected'] = true;
					/* add select's option to previously added select element */
					$('#td_' + w.name + ' select').tplAppend(option, this.option_tpl);
				}
				break;
		}
		w.validator && (this.validator_rules[w.name] = w.validator);
		/* I18N for element's error messages */
		w.message && (this.validator_messages[w.name] = _(w.message));
	};
	
	/* template for console output */
	this.console_tpl = function() {
		return [
			'tr', {}, [
				'td', {}, [
					'pre', {}, [
						'b', {}, this.cmd,
						'br', {}, "",
						'p'
					]
				]
			]
		];
	};
	
	/*
	 * Add output of command execution to the page.
	 * cmd — string or array with cmds' to execute.
	 */
	this.addConsole = function(cmd) {
		var outer = this;
		
		/* adds command name to the page, and makes AJAX request to the server */
		var addConsoleOut = function(name, value) {
			$(outer.table).tplAppend({cmd: value}, outer.console_tpl);
			$("tr > td > pre > b:contains('" + value + "')", outer.table).nextAll("p")
				.load("kdb/execute.cgi", {cmd: value});
		}
		
		/* we can have one or several commands */
		if (typeof cmd == "object") {
			$(cmd).each(function(name, value) {
				addConsoleOut(name, value);
			});
		} else {
			addConsoleOut(0, cmd);
		}
	}

	/**
	 * Adds submit button, form validation rules and submit's events handlers.
	 * options.ajaxTimeout — time in seconds to wait for server reply before show an error message.
	 * options.reload — reload page after AJAX request (e.g., for update translation)
	 */
	this.addSubmit = function(options) {
		var timeout = (options && options.ajaxTimeout) ? options.ajaxTimeout * 1000 : null;
		var id_info_message = "#" + this.info_message;
		
		/* sets error message
		 * I18N for text
		 */
		var setError = function(text) {
			$(id_info_message).html(_(text));
			if ($(id_info_message).hasClass("success_message")) {
				$(id_info_message).removeClass("success_message");
			}
			$(id_info_message).addClass("error_message");
		};
		
		/* sets info message
		 * I18N for text
		 */
		var setInfo = function(text) {
			$(id_info_message).html(_(text));
			if ($(id_info_message).hasClass("error_message")) {
				$(id_info_message).removeClass("error_message");
			}
			$(id_info_message).addClass("success_message");
		};
		
		/* shows message */
		var showMsg = function() {
			$(id_info_message).show();
		};

		/* if subsystem is set — add it to the form */
		if (this.subsystem) {
			$("<input type='hidden' name='subsystem' value='" + this.subsystem + "'/>").appendTo(this.form);
		}

		/* create submit button */
		$("<input type='submit' class='button' value='" + _("Save") + "'/>").appendTo(this.form);
		
		$("input").tooltip({track: true});
		
		/* apply validate rules to form */
		$(this.form).validate({
			rules: this.validator_rules,
			messages: this.validator_messages,
			
			/* container where to show error */
			errorContainer: id_info_message,
			
			/* Set error text to container (closure to setError var) */
			showErrors: function(errorMap, errorList) {
				setError("Please, enter a valid data into the form below to be able to save it successfully.");
				this.defaultShowErrors();
			},
			
			errorPlacement: function(error, element) {
     			error.prependTo(element.parent());
     		},
     		
     		/* (closure to timeout var) */
     		submitHandler: function(form) {
     			/*
     			 * All checkboxes return values, even they are unchecked.
     			 * Here we find all unchecked checkboxes, temporarily check them, set
     			 * their value, and set their class to doUncheck, to uncheck them later.
     			 */
     			$(":checkbox").not(":checked").each(function() {
					this.checked = true;
					this.value = 0;
				}).addClass("doUncheck");
				
     			$(form).ajaxSubmit({
     				url: "kdb/kdb_save.cgi",
     				type: "POST",
     				timeout: timeout,
     				
     				/* show error when unable to save data (closure to setError and showMsg var) */
     				error: function() {
     					setError("Error: unable to save data.");
     					showMsg();
     				},
     				
     				/* save data to tmp local cache and show message before submit data
     				 * (closure to setInfo and showMsg var)
     				 */
					beforeSubmit: function() {
						/* Here we uncheck temporarily checked checkboxes */
						$(".doUncheck").each(function() {
							this.checked = false;
							this.value = 1;
						}).removeClass("doUncheck");
						
						config.saveTmpVals(form);
						setInfo("Saving data...");
						showMsg();
					},
					
					/* sava data to local cache and show message after submit data
					 * (closure to setInfo and showMsg var)
					 */
					success: function() {
						config.saveVals();
						setInfo("Data saved successfully.");
						showMsg();
						if (options && options.reload) document.location.reload();
					}
				});
     		}
		});
	};
}

/*
 * Adds a new item to the menu.
 * path — place for a new item (e.g., "Network:Interfaces" means menu Network, submenu Interfaces)
 * func — function to call when user clicks on the menu item.
 * name — name of the menu item.
 * Example of the menu structure is given below.
 * <ul class="treeview" id="menu">
 *		<li><span>System</span>
 *			<ul>
 *				<li><span><a href="#" onclick="Controllers.webface()">Interface</a></span></li>
 *				<li><span><a href="#" onclick="Controllers.general()">General</a></span></li>
 *			</ul>
 *		</li>
 *		<li><span>Network</span>
 *		    <ul>
 *		        <li><span>Interfaces</span>
 *					<ul>
 *						<li><span><a href="#" onclick="Controllers.iface('eth0')">eth0</a></span></li>
 *						<li><span><a href="#" onclick="Controllers.iface('eth1')">eth1</a></span></li>
 *					</ul>
 *				</li>
 *			</ul>
 *		</li>
 *	</ul>
 */
function addItem(path, func, name) {
	/* menu element */
	var idMenu = "#menu";
	
	var curLevel = idMenu;
	var pathElems = path.split(":");
	for (var pathElem in pathElems) {
		/* check if the corresponding submenu is exist */
		if ($(" > li > span:contains('" + _(pathElems[pathElem]) + "')", curLevel).length == 0) {
			/* if not, add it */
			$(curLevel).append("<li><span>" + _(pathElems[pathElem]) + "</span><ul></ul></li>");
		}
		/* change current level in the menu */
		curLevel = $(" > li > span:contains('" + _(pathElems[pathElem]) + "')", curLevel).next();
	}
	
	/* add new item */
	$(curLevel).append("<li><span><a href='#' onclick=" + func + ">" + _(name) + "</a></span></li>");
	
	/* highlight selected item */
	$(" > li > span > a:contains('" + _(name) + "')", curLevel).click(function() {
		$("a", idMenu).removeClass("clicked");
		$(this).addClass("clicked");
	});
}
