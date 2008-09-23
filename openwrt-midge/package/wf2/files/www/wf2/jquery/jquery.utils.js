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
