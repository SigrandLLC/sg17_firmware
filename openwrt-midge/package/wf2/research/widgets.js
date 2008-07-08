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
			$('#container').tabs({fxAutoHeight: true});
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
		
		// %TITLE%
		// %FIELD_ID%
		// %TEXT%
		// %FIELD%
		// %DESCR%
		// %VALIDATOR%		
		function Container(p) {
			this.form = $("<form action='#' onsubmit='return false'></form>").appendTo(p).get();
			this.table = $("<table id='conttable' cellpadding='0' cellspacing='0' border='0'></table>").appendTo(this.form).get();

			this.table_title_tmpl = "<tr><th colspan='2'>%TITLE%</th></tr>";
			this.widget_tmpl = "<tr><td class='tdleft'>%TEXT%</td><td> %FIELD%" +
				"<br /><p>%DESCR%</p></td></tr>";
			this.text_tmpl = "<input name='%FIELD_ID%' id='%FIELD_ID%' value='%VALUE%' %VALIDATOR% type='text' size='25' maxlength='255' />";
			this.password_tmpl = "<input name='%FIELD_ID%' type='password' size='25' maxlength='255' />";
			this.checkbox_tmpl = "<input class='check' name='%FIELD_ID%' %VALIDATOR% checked='1' type='checkbox' />";
			this.select_tmpl = "<select name='%FIELD_ID%'>%OPTIONS%</select>";
			this.option_tmpl = "<option value='%VALUE%'>%NAME%</option>";
			
			this.addTitle = function (title) {
				var table_title = this.table_title_tmpl.replace("%TITLE%", title);
				$(table_title).appendTo(this.table);
			}
	
			this.addWidget = function (w) {
				var field = this.widget_tmpl;
				var validator='';
	
				switch (w.type) {
				case "text": 
					field = field.replace('%FIELD%', this.text_tmpl); break;
				case "password": 
					field = field.replace('%FIELD%', this.password_tmpl); break;
				case "checkbox":
					field = field.replace('%FIELD%', this.checkbox_tmpl); break;
				case "select":
					field = field.replace('%FIELD%', this.select_tmpl);
					var options;
					for (var value in w.options) {
						var option = this.option_tmpl.replace('%VALUE%', value);
						options += option.replace('%NAME%', w.options[value]);
					}
					field = field.replace('%OPTIONS%', options);
					break;
				}
				field = field.replace('%FIELD_ID%', w.name).replace('%TEXT%', w.text || "")
					.replace('%VALIDATOR%', validator || "").replace('%DESCR%', w.descr || "&nbsp;")
					.replace('%VALUE%', w.value).replace('%FIELD_ID%', w.name);
				$(field).appendTo(this.table);
			}
	
			this.addSubmit = function() {
				//$("<input type='submit' class='button' name='save' value='Save' />").appendTo(this.form);
				var val = $('#sys_hostname');
				var button = $("<button type='button' class='button'>Save</button>");
				button.attr('onclick', 'config.Save($("#sys_hostname").attr("value"))');
				button.appendTo(this.form);
			}
		}
