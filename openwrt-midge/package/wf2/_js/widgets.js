/*
 * Add tabs to 'p' container.
 * tabs: hash, key — id, value — name of tab
 * subsystem — name of a subsystem for all tabs on the page.
 * I18N for tab name.
 */
function pageTabs(p, tabsInfo, subsystem) {
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
		this.tabs[tab] = new TabContents($("<div id='" + tab + "'></div>").appendTo(p).get(), subsystem);
	}
	
	/* update tabs */
	$(p).tabs({fxAutoHeight: true});
}

/*
 * Contents of one tab.
 */
function TabContents(tab, subsystem) {
	this.tab = tab;
	this.subsystem = subsystem;

	this.addContainer = function() {
		return new Container(this.tab, this.subsystem);
	}
	
	this.addBr = function() {
		$("<br />").appendTo(this.tab);
	}
}

/*
 * Container for widgets
 * I18N for widgets.
 */
function Container(p, subsystem) {
	this.subsystem = subsystem;
	this.validator_rules = new Object();
	this.validator_messages = new Object();
	this.info_message = "info_message_" + $(p).attr("id");
	$("<div class='message'></div>").attr("id", this.info_message).appendTo(p);
	this.form = $("<form action=''></form>").appendTo(p).get();
	this.table = $("<table id='conttable' cellpadding='0' cellspacing='0' border='0'></table>").appendTo(this.form).get();

	this.setSubsystem = function(subsystem) {
		this.subsystem = subsystem;
	}

	/* template for table title */
	this.table_title_tpl = function() {
		return [
			'tr', {}, [
				'th', {colspan: '2'}, this.title
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
		return [
			'tr', {}, [
				'td', {className: 'tdleft'}, _(this.text),
				'td', {id: 'td_' + this.name}, '<br /><p>' + _(this.descr || "") + '</p>'
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
		if (config.get(this.name) == "1") attrs['checked'] = true;
		return ['input', attrs];
	};
	
	/* template for select widget */
	this.select_tpl = function() {
		var attrs = { name: this.name };
		this.id && (attrs.id = this.id);
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
