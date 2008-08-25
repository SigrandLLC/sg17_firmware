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
 * options — additional options for container, passed from pageTabs.
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
	 newwin = window.open(url,'help', params);
	 if (window.focus) {newwin.focus()}
	 return false;
}

/*
 * Container for widgets.
 * p — parent container.
 * options — container options (subsystem & help), initially passed to pageTabs.
 * helpSection — see details in addContainer method of TabContents object.
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
	
	/* 
	 * Adds title and context help link to container and adds it to container's table.
	 * I18N for title.
	 */
	this.addTitle = function(title) {
		var url;
		
		/* if helpSection is object, it contains page and section names */
		if (helpSection && typeof helpSection == "object" && helpSection.page && helpSection.section) {
			url = "/help/" + helpSection.page + ".html#" + helpSection.section;
		/* if we have common help page setted for all tabs */
		} else if (options && options.help) {
			/* if helpSection is set — it is string with section name */
			url = helpSection ? "/help/" + options.help + ".html#" + helpSection :
				"/help/" + options.help + ".html";
		} else {
			url = null;
		}
		
		/* if url is set — create context help link object, otherwise set it to null */
		var help = url ? $.create('a', {'href': '#', 'className': 'helpLink'}, '[?]')
			.click(function() {
				popup(url);
			}) : null;
		
		/* create table's row for title and context help link */
		$.create('tr', {},
			$.create('th', {'colSpan': '2'}, [
					_(title),
					" ",
					help
				]
			)
		).appendTo(this.table);
	};
	
	/*
	 * Adds general for all widgets elements.
	 * w — widget's info.
	 * p — destination container.
	 * I18N for text and description.
	 */
	this.addGeneralWidget = function(w, p) {
		/* if this field is required — show "*" */
		var required = (w.validator && w.validator['required']) ? " *" : "";
	
		/* if description is specified — show it */
		var tdElements;
		if (w.descr) {
			tdElements = new Array();
			tdElements.push($.create('br'));
			tdElements.push($.create('p', {}, _(w.descr)));
		}
		
		/* create table row for widget */
		$.create('tr', {}, [
				$.create('td', {'className': 'tdleft'}, _(w.text) + required),
				$.create('td', {'id': 'td_' + w.name}, tdElements)
			]
		).appendTo(p);
	};
	
	/*
	 * Add text widget.
	 */
	this.addTextWidget = function(w, p) {
		var attrs = {
			'type': 'text',
			'name': w.name,
			'size': '25'
		};
		w.id && (attrs['id'] = w.id);
		w.tip && (attrs['title'] = w.tip);
		
		/* set KDB value */
		if (config.get(w.name)) {
			attrs['value'] = config.get(w.name);
		/* if KDB value does't exists — set default value, if it exists */
		} else if (w.defaultValue) {
			attrs['value'] = w.defaultValue;
		}
		
		$.create('input', attrs).prependTo(p);
	};
	
	/*
	 * Add password widget.
	 */
	this.addPasswordWidget = function(w, p) {
		var attrs = {
			'type': 'password',
			'name': w.name,
			'size': '25'
		};
		w.id && (attrs['id'] = w.id);
		w.tip && (attrs['title'] = w.tip);
		
		$.create('input', attrs).prependTo(p);
	};
	
	/*
	 * Add checkbox widget.
	 */
	this.addCheckboxWidget = function(w, p) {
		var attrs = {
			'type': 'checkbox',
			'name': w.name,
			'className': 'check',
			'value': '1'
		};
		w.id && (attrs['id'] = w.id);
		w.tip && (attrs['title'] = w.tip);
		if (config.get(w.name) == "1") attrs['checked'] = true;
		
		$.create('input', attrs).prependTo(p);
	};
	
	/*
	 * Add select widget.
	 */
	this.addSelectWidget = function(w, p) {
		var attrs = { 'name': w.name };
		w.id && (attrs['id'] = w.id);
		w.tip && (attrs['title'] = w.tip);
		
		$.create('select', attrs).prependTo(p);
	};

	/* add widget to container */
	this.addWidget = function(w) {
		/* add common widget's data */
		this.addGeneralWidget(w, this.table);
		switch (w.type) {
			case "text": 
				this.addTextWidget(w, '#td_' + w.name);
				break;
			case "password": 
				this.addPasswordWidget(w, '#td_' + w.name);
				break;
			case "checkbox":
				this.addCheckboxWidget(w, '#td_' + w.name);
				break;
			case "select":
				this.addSelectWidget(w, '#td_' + w.name);

				var selectedIndex = -1;
				var defaultIndex = -1;
				var selectedItem;
				
				/* go though list of options */
				$.each(w.options, function(name, value) {
					/*
					 * Heh. In options list property name is the value of option, and
					 * property value is the text of option.
					 */
					var attrs = { 'value': name };
					
					/* if current option should be selected */
					if (config.get(w.name) == name) {
						selectedItem = name;
					}
					
					/* add option to previously added select element */
					$.create('option', attrs, _(value)).prependTo('#td_' + w.name + ' select');
				});
				
				/* find selectedIndex and defaultIndex */
				$('#td_' + w.name + ' select option').each(function(idx) {
					this.value == selectedItem && (selectedIndex = idx);
					this.value == w.optionDefault && (defaultIndex = idx);
				});

				/* set selected index in select element */
				if (selectedIndex != -1) {
					$('#td_' + w.name + ' select').attr("selectedIndex", selectedIndex);
				}
				/* if nothing is selected — select default item */
				else if (defaultIndex != -1) {
					$('#td_' + w.name + ' select').attr("selectedIndex", defaultIndex);
				}

				break;
		}
		w.validator && (this.validator_rules[w.name] = w.validator);
		/* I18N for element's error messages */
		w.message && (this.validator_messages[w.name] = _(w.message));
	};
	
	/*
	 * Adds HTML code for command output.
	 */
	this.addConsoleHTML = function(cmd, p) {
		$.create('tr', {},
			$.create('td', {}, [
					$.create('b', {}, cmd),
					$.create('br'),
					$.create('div', {'className': 'pre'})
				]
			)
		).appendTo(p);
	};
	
	/*
	 * Add output of command execution to the page.
	 * cmd — string or array with cmds' to execute.
	 */
	this.addConsole = function(cmd) {
		var outer = this;
		
		/*
		 * Do AJAX request for command execution, and
		 * replace '\n' in output with '<br>'.
		 */
		var cmdExecute = function(url, cmd, p) {
			$.ajax({
				type: "POST",
				url: url,
				dataType: "text",
				data: cmd,
				dataFilter: function(data, type) {
					return data.replace(/\n/g, "<br>");
				},
				success: function(html) {
					$(p).html(html);
				}
			});
		};
		
		/* adds command's HTML to the page, and makes AJAX request to the server */
		var addConsoleOut = function(name, value) {
			outer.addConsoleHTML(value, outer.table);
			cmdExecute("kdb/execute.cgi", {cmd: value},
				$("tr > td > b:contains('" + value + "')", outer.table).nextAll("div.pre"));
		};
		
		/* we can have one or several commands */
		if (typeof cmd == "object") {
			$(cmd).each(function(name, value) {
				addConsoleOut(name, value);
			});
		} else {
			addConsoleOut(0, cmd);
		}
	}

	/*
	 * Adds submit button, form validation rules and submit's events handlers.
	 * options.ajaxTimeout — time in seconds to wait for server reply before show an error message.
	 * options.reload — reload page after AJAX request (e.g., for update translation)
	 */
	this.addSubmit = function(options) {
		var timeout = (options && options.ajaxTimeout) ? options.ajaxTimeout * 1000 : null;
		var id_info_message = "#" + this.info_message;
		
		/* 
		 * sets error message
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
 * path — place for a new item (e.g., "Network:Interfaces" means menu Network, submenu Interfaces).
 * name — name of the menu item.
 * func — name of the function in a Controllers object to call when user clicks on the menu item.
 * params — function parameters.
 * 
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
function addItem(path, name, func, params) {
	/* menu element */
	var idMenu = "#menu";
	
	/* context which is set when menu functions are called */
	var defaultContext = Controllers;
	
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
	
	/* create link object */
	var link = $.create('a', {'href': '#'}, _(name))
		.click(function() {
			if (params) defaultContext[func](params);
			else defaultContext[func]();
			
			/* highlight selected item */
			$("a", idMenu).removeClass("clicked");
			$(this).addClass("clicked");
		});
	
	/* create menu item and add it to the menu */
	$.create('li', {}, $.create('span', {}, link)).appendTo(curLevel);
}
