		// %TITLE%
		// %FIELD_ID%
		// %TEXT%
		// %FIELD%
		// %DESCR%
		// %VALIDATOR%
		function Container(p) {
			this.parentContainer=p;
			this.table_title_tmpl = "<tr><td class='table_title' colspan='2'><table border='0'> <tbody><tr class='table_title'><td width='100%'>%TITLE%</td>" + 
				"<td></td></tr></tbody></table></td></tr> ";
			this.widget_tmpl = "<tr><td class='vncellt' width='35%'><label for='%FIELD_ID%'>%TEXT%</label></td><td class='listr' width='65%'> %FIELD%" +
				"<br><span class='inputDesc'>%DESCR%</span></td></tr>";
			this.text_tmpl = "<input class='edit' name='%FIELD_ID%' %VALIDATOR% checked='1' type='text'>";
			this.checkbox_tmpl = "<input class='edit' name='%FIELD_ID%' %VALIDATOR% checked='1' type='checkbox'>";
		}

		Container.prototype.addTitle = function (title) {
			var table_title = this.table_title_tmpl.replace("%TITLE%", title);
			$(table_title).appendTo(this.parentContainer);
		}

		Container.prototype.addWidget = function (w) {
			var field = this.widget_tmpl;
			var validator='';

			switch (w.type) {
			case "text": 
				field = field.replace('%FIELD%', this.text_tmpl); break;
			case "checkbox":
				field = field.replace('%FIELD%', this.checkbox_tmpl); break;
			case "checkbox":
				field = field.replace('%FIELD%', this.checkbox_tmpl); break;
			}
			field = field.replace('%FIELD_ID%', w.name).replace('%TEXT%', w.text || "").replace('%VALIDATOR%', validator || "").replace('%DESCR%', w.descr || "&nbsp;");
			$(field).appendTo(this.parentContainer);
		}

