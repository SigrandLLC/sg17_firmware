/*
 * Returns number of propertiens in object.
 */
(function($){  
	$.extend({
		len: function(object) {
			var num = 0;
			for (var i in object) num++;
			return num;
		}
	});
})(jQuery);

/*
 * Creates new object like { name: 'property', value: 'value' }
 * and adds it to array.
 * 
 * array — destination array.
 * property — name of property.
 * value — value of property.
 */
(function($){  
	$.extend({
		addObjectWithProperty: function(array, property, value) {
			var object = new Object();
			object['name'] = property;
			object['value'] = value;
			array.push(object);
		}
	});
})(jQuery);
