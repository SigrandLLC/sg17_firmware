(function($){  
	$.fn.setOptionsForSelect = function(options, curValue, defaultValue) {
		var selectedIndex = -1;
		var defaultIndex = -1;
		var selectedItem;
		
		var selectObject = $(this);
		
		/* remove previous options */
		selectObject.empty();
		
		/* if option's list is string — convert it to hash */
		if (typeof options == "string") {
			var vals = options;
			options = new Object();
			$.each(vals.split(" "), function(num, value) {
				options[value] = value;
			});
		}
		
		/* if option's list is array — convert it to hash */
		if (options.constructor == Array) {
			var arr = options;
			options = new Object();
			$.each(arr, function(num, value) {
				/* values have to be strings */
				options[value + ""] = value + "";
			});
		}
		
		/* go though list of options */
		$.each(options, function(name, value) {
			/*
			 * Heh. In options list property name is the value of option, and
			 * property value is the text of option.
			 */
			var attrs = {'value': name};
			
			/* if current option should be selected */
			if (curValue == name) {
				selectedItem = name;
			}
			
			/* add option to select element */
			$.create('option', attrs, value).appendTo(selectObject);
		});
		
		/* find selectedIndex and defaultIndex */
		$('option', selectObject).each(function(idx) {
			this.value == selectedItem && (selectedIndex = idx);
			this.value == defaultValue && (defaultIndex = idx);
		});

		/* set selected index in select element */
		if (selectedIndex != -1) {
			$(selectObject).attr("selectedIndex", selectedIndex);
		}
		/* if nothing is selected — select default item */
		else if (defaultIndex != -1) {
			$(selectObject).attr("selectedIndex", defaultIndex);
		}
	}
})(jQuery);
