jQuery.validator.addMethod("ipAddr", function(value, element) {
	return this.optional(element) ||
		/^(\d{1,2}|1\d\d|2[0-4]\d|25[0-5])\.(\d{1,2}|1\d\d|2[0-4]\d|25[0-5])\.(\d{1,2}|1\d\d|2[0-4]\d|25[0-5])\.(\d{1,2}|1\d\d|2[0-4]\d|25[0-5])$/.test(value);
}, "Please enter correct IP address.");

jQuery.validator.addMethod("mxsmap", function(value, element) {
	return this.optional(element) ||
		/^(([0-9]+,)|([0-9]+-[0-9]+(,)?)|([0-9]+$))+$/.test(value);
}, "Please enter correct MXSMAP.");

jQuery.validator.addMethod("voipRouterID", function(value, element) {
	return this.optional(element) || /^([0-9]){3}$/.test(value);
}, "Please enter correct Router ID.");
