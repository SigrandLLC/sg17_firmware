/*
 * Add tabs to 'p' container.
 * tabs: hash, key — id, value — name of tab
 */
function pageTabs(p, tabsInfo) {
	/* clear page */
	$(p).empty();
	
	var tabsList = "<ul>";
	for (tab in tabsInfo) {
		tabsList += "<li><a href='#" + tab + "'><span>" + tabsInfo[tab] + "</span></a></li>";
	}
	tabsList += "</ul>";
	$(tabsList).appendTo(p);
	this.tabs = new Array();
	for (tab in tabsInfo) {
		this.tabs[tab] = new TabContents($("<div id='" + tab + "'></div>").appendTo(p).get());
	}
	
	/* update tabs */
	$(p).tabs({fxAutoHeight: true});
}

/*
 * Contents of one tab.
 */
function TabContents(tab) {
	this.tab = tab;

	this.addContainer = function() {
		return new Container(this.tab);
	}
	
	this.addBr = function() {
		$("<br />").appendTo(this.tab);
	}
}

/*
 * Container for widgets.
 */
function Container(p) {
	this.validator_rules = new Object();
	this.validator_messages = new Object();
	$("<div class='message' id='info_message'></div>").appendTo(p);
	this.form = $("<form action=''></form>").appendTo(p).get();
	this.table = $("<table id='conttable' cellpadding='0' cellspacing='0' border='0'></table>").appendTo(this.form).get();

	/* template for table title */
	this.table_title_tpl = function() {
		return [
			'tr', {}, [
				'th', {colspan: '2'}, this.title
			]
		];
	};
	
	/* common widget's template */
	this.widget_tpl = function() {
		return [
			'tr', {}, [
				'td', {className: 'tdleft'}, this.text,
				'td', {id: 'td_' + this.name}, '<br /><p>' + this.descr + '</p>'
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
						className: 'check'
		};
		this.id && (attrs.id = this.id);
		return ['input', attrs];
	};
	
	/* template for select widget */
	this.select_tpl = function() {
		var attrs = { name: this.name };
		this.id && (attrs.id = this.id);
		return ['select', attrs];
	};
	
	/* template for option */
	this.option_tpl = function() {
		var attrs = { value: this.value };
		return ['option', attrs, this.name];
	};
	
	/* add title to table */
	this.addTitle = function(title) {
		var json = {title: title};
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
					/* add select's option to previously added select element */
					$('#td_' + w.name + ' select').tplAppend(option, this.option_tpl);
				}
				break;
		}
		
		w.validator && (this.validator_rules[w.name] = w.validator);
		w.message && (this.validator_messages[w.name] = w.message);
	};

	this.addSubmit = function() {
		var setError = function(text) {
			$("#info_message").html(_(text));
			if ($("#info_message").hasClass("success_message")) {
				$("#info_message").removeClass("success_message");
			}
			$("#info_message").addClass("error_message");
		};
		
		var showError = function() {
			$("#info_message").show();
		};

		$("<input type='submit' class='button' value='" + _("Save") + "'/>").appendTo(this.form);
		
		/* apply validate rules to form */
		$(this.form).validate({
			rules: this.validator_rules,
			messages: this.validator_messages,
			
			/* container where to show error */
			errorContainer: "#info_message",
			
			/* Set error text to container (closure to setError var) */
			showErrors: function(errorMap, errorList) {
				setError("Please, enter a valid data into the form below to be able to save it successfully.");
				this.defaultShowErrors();
			},
			errorPlacement: function(error, element) {
     			error.prependTo(element.parent());
     		},
     		submitHandler: function(form) {
     			$(form).ajaxSubmit({
     				url: "kdb/kdb_save.cgi",
     				type: "POST",
     				timeout: 3000,
     				/* show error when unable to save data (closure to setError and showError var) */
     				error: function() {
     					setError("Unable to save data.");
     					showError();
     				},
					beforeSubmit: function() {
						config.saveTmpVals(form);
					},
					success: function() {
						config.saveVals();
						$("#info_message").html(_("Data saved successfully."));
						if ($("#info_message").hasClass("error_message")) {
							$("#info_message").removeClass("error_message");
						}
						$("#info_message").addClass("success_message");
						$("#info_message").show();
					}
				});
     		}
		});
	};
}
