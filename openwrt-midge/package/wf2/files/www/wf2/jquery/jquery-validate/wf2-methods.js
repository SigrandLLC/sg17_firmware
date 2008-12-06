/* IP address */
jQuery.validator.addMethod("ipAddr", function(value, element) {
	return this.optional(element) ||
		/^(\d{1,2}|1\d\d|2[0-4]\d|25[0-5])\.(\d{1,2}|1\d\d|2[0-4]\d|25[0-5])\.(\d{1,2}|1\d\d|2[0-4]\d|25[0-5])\.(\d{1,2}|1\d\d|2[0-4]\d|25[0-5])$/.test(value);
}, "Please enter correct IP address.");

/* IP port */
jQuery.validator.addMethod("ipPort", function(value, element) {
	return this.optional(element) || /^((\d)+|any)$/.test(value);
}, "Please enter correct IP port.");

/* IP address with optional port */
jQuery.validator.addMethod("ipAddrPort", function(value, element) {
	return this.optional(element) ||
		/^(0?0?\d|[01]?\d\d|2[0-4]\d|25[0-5])\.(0?0?\d|[01]?\d\d|2[0-4]\d|25[0-5])\.(0?0?\d|[01]?\d\d|2[0-4]\d|25[0-5])\.(0?0?\d|[01]?\d\d|2[0-4]\d|25[0-5])(:\d+)?$/.test(value);
}, "Please enter correct IP address with optional port.");

/* IP port range */
jQuery.validator.addMethod("ipPortRange", function(value, element) {
	return this.optional(element) || /^((\d)+(:)?((\d)+)?|(:)((\d)+)|any)$/.test(value);
}, "Please enter correct IP port or port range.");

/* IP netmask */
jQuery.validator.addMethod("netmask", function(value, element) {
	return this.optional(element) ||
		/^(0?0?\d|[01]?\d\d|2[0-4]\d|25[0-5])\.(0?0?\d|[01]?\d\d|2[0-4]\d|25[0-5])\.(0?0?\d|[01]?\d\d|2[0-4]\d|25[0-5])\.(0?0?\d|[01]?\d\d|2[0-4]\d|25[0-5])$/.test(value);
}, "Please enter correct ip netmask.");

/* IP address with optional netmask */
jQuery.validator.addMethod("ipNetMaskIptables", function(value, element) {
	return this.optional(element) ||
		/^[!]?(0?0?\d|[01]?\d\d|2[0-4]\d|25[0-5])\.(0?0?\d|[01]?\d\d|2[0-4]\d|25[0-5])\.(0?0?\d|[01]?\d\d|2[0-4]\d|25[0-5])\.(0?0?\d|[01]?\d\d|2[0-4]\d|25[0-5])(\/\d\d*)?$/.test(value);
}, "Please enter correct ip address with optional mask.");

/* MAC address */
jQuery.validator.addMethod("macAddr", function(value, element) {
	return this.optional(element) ||
		/^([0-9a-fA-F][0-9a-fA-F]:){5}([0-9a-fA-F][0-9a-fA-F])$/.test(value);
}, "Please enter correct mac address.");

/* Domain name */
jQuery.validator.addMethod("domainName", function(value, element) {
	return this.optional(element) || /^[a-zA-Z0-9]+([a-zA-Z0-9\-\.]+)?$/.test(value);
}, "Please enter correct domain name.");

/* DNS record domain */
jQuery.validator.addMethod("dnsRecordDomainOrIpAddr", function(value, element) {
	return this.optional(element) || /(^@$)|(^[a-zA-Z0-9]+([a-zA-Z0-9\-\.]+)?$)|(^(\d{1,2}|1\d\d|2[0-4]\d|25[0-5])\.(\d{1,2}|1\d\d|2[0-4]\d|25[0-5])\.(\d{1,2}|1\d\d|2[0-4]\d|25[0-5])\.(\d{1,2}|1\d\d|2[0-4]\d|25[0-5])$)$/.test(value);
}, "Please enter correct domain name.");

/* Domain name or IP address */
jQuery.validator.addMethod("domainNameOrIpAddr", function(value, element) {
	return this.optional(element) || /(^[a-zA-Z0-9]+([a-zA-Z0-9\-\.]+)?$)|(^(\d{1,2}|1\d\d|2[0-4]\d|25[0-5])\.(\d{1,2}|1\d\d|2[0-4]\d|25[0-5])\.(\d{1,2}|1\d\d|2[0-4]\d|25[0-5])\.(\d{1,2}|1\d\d|2[0-4]\d|25[0-5])$)/.test(value);
}, "Please enter correct dns domain name or IP-address.");

/* map of slots for E1 */
jQuery.validator.addMethod("smap", function(value, element) {
	return this.optional(element) ||
		/^(([0-9]+,)|([0-9]+-[0-9]+(,)?)|([0-9]+$))+$/.test(value);
}, "Please enter correct slot map.");

/* VoIP's router id */
jQuery.validator.addMethod("voipRouterID", function(value, element) {
	return this.optional(element) || /^([0-9]){3}$/.test(value);
}, "Please enter correct Router ID.");

/* VoIP's registrar */
jQuery.validator.addMethod("voipRegistrar", function(value, element) {
	return this.optional(element) || /^sip:(([a-zA-Z0-9]+([a-zA-Z0-9\-\.]+)?)|((\d{1,2}|1\d\d|2[0-4]\d|25[0-5])\.(\d{1,2}|1\d\d|2[0-4]\d|25[0-5])\.(\d{1,2}|1\d\d|2[0-4]\d|25[0-5])\.(\d{1,2}|1\d\d|2[0-4]\d|25[0-5])))$/.test(value);
}, "Please enter correct registrar name or it's IP-address, with sip: prefix.");

/* VoIP's SIP URI */
jQuery.validator.addMethod("voipSipUri", function(value, element) {
	return this.optional(element) || /^sip:([_a-zA-Z0-9](\.[_a-zA-Z0-9])?)+@([_a-zA-Z0-9]\.?)+/.test(value);
}, "Please enter correct user SIP URI, with sip: prefix.");

/* VoIP's short number */
jQuery.validator.addMethod("voipShortNumber", function(value, element) {
	return this.optional(element) || /^([0-9]){2}$/.test(value);
}, "Please enter 2-digit positive number.");

/* VoIP's complete number */
jQuery.validator.addMethod("voipCompleteNumber", function(value, element) {
	return this.optional(element) || /(^([0-9]{3}|[\*])([-,]*)([0-9]{2}|[\*])([-,]*)([-0-9,]*)$)|(^#sip:([_a-zA-Z0-9](\.[_a-zA-Z0-9])?)+@([_a-zA-Z0-9]\.?)+#$)/.test(value);
}, "Please enter correct complete number.");

/* VoIP's payload */
jQuery.validator.addMethod("voipPayload", function(value, element) {
	return this.optional(element) || /(^0x[0-9a-fA-F]+$)|(^[0-9]+$)/.test(value);
}, "Please enter correct payload type.");

/* PBO value */
jQuery.validator.addMethod("pbo", function(value, element) {
	return this.optional(element) || /^(\d+(:\d+)*)+$/.test(value);
}, "Please enter correct PBO value.");

/* Alphanumeric and underline only */
jQuery.validator.addMethod("alphanumU", function(value, element) {
	return this.optional(element) || /^[_a-zA-Z0-9]+$/.test(value);
}, "Please enter alphanumeric and underline characters only (without spaces).");

/* Alphanumeric only */
jQuery.validator.addMethod("alphanum", function(value, element) {
	return this.optional(element) || /^[a-zA-Z0-9]+$/.test(value);
}, "Please enter alphanumeric characters only (without spaces).");

/* QoS bandwith */
jQuery.validator.addMethod("qosBandwith", function(value, element) {
	return this.optional(element) || /^([0-9]+(k|M)(bit|bps))$/.test(value);
}, "Please enter correct bandwith.");

/* Unique value among all elements specified with selector in parameter */
jQuery.validator.addMethod("uniqueValue", function(value, element, params) {
	if (this.optional(element)) return true;
	var unique = true;
	$(params).not(element).each(function(num, anotherValue) {
		if ($(anotherValue).val() == value) {
			unique = false;
			return false;
		}
	});
	return unique;
}, "Please enter unique values.");
