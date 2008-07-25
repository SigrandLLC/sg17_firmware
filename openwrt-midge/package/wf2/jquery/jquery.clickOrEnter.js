/*
@author Bryan Gullan
@version 1.2
@description Bind mouse click and enter key to a given element. On Click or Enter, specified function is called and default event action blocked. The function called is aware of the target of the click / enter keypress.

Sample use is to add popup to a link, where the href would be followed if javascript were turned off.

var popup = function(target) {
	alert('activated'+ $(target).attr('href'));
};
$(document).ready(function() {
	$.clickOrEnter('a',popup);
});

(c) 2007 Bryan Gullan.
Use and distribute freely with this header intact
*/

jQuery.clickOrEnter = function(element,callback) {
	jQuery(element).bind('click', function(event) {  		callback(event.target);
  		event.preventDefault(); //prevent browser from following the actual href
	});
	jQuery(element).bind('keypress', function(event) {		var code=event.charCode || event.keyCode;		if(code && code == 13) {// if enter is pressed
  			callback(event.target);
  			event.preventDefault(); //prevent browser from following the actual href
		};
	});
};

