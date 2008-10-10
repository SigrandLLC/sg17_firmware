Controllers['voip'] = function() {
	var page = this.Page();
	page.setSubsystem("voip");
	
	/* settings tab */
	page.addTab({
		"id": "settings",
		"name": "Settings",
		"func": function() {
			var c, field;
			c = page.addContainer("settings");
			c.setHelpPage("voip.settings");
			c.addTitle("VoIP settings");
		
			field = { 
				"type": "text",
				"name": "sys_voip_settings_selfnumber",
				"text": "Router ID",
				"validator": {"required": true, "voipRouterID": true}
			};
			c.addWidget(field);
			
			field = { 
				"type": "text",
				"name": "sys_voip_settings_selfip",
				"text": "Router IP",
				"validator": {"required": true, "ipAddr": true}
			};
			c.addWidget(field);
			
			field = { 
				"type": "text",
				"name": "sys_voip_settings_rtp_port_first",
				"text": "RTP port start",
				"descr": "Begin of ports range to use for RTP",
				"validator": {"required": true, "min": 0, "max": 65535}
			};
			c.addWidget(field);
			
			field = { 
				"type": "text",
				"name": "sys_voip_settings_rtp_port_last",
				"text": "RTP port end",
				"descr": "End of ports range to use for RTP",
				"validator": {"required": true, "min": 0, "max": 65535}
			};
			c.addWidget(field);
			
			field = { 
				"type": "select",
				"name": "sys_voip_settings_codec_ext_quality",
				"text": "External quality",
				"descr": "External call quality",
				"tip": "Quality of calls through SIP-server",
				"options": {"speed": "Speed", "quality": "Quality"}
			};
			c.addWidget(field);
			
			field = { 
				"type": "select",
				"name": "sys_voip_settings_codec_int_quality",
				"text": "Internal quality",
				"descr": "Internal call quality",
				"tip": "Quality of calls between routers",
				"options": {"speed": "Speed", "quality": "Quality"}
			};
			c.addWidget(field);
			
			field = { 
				"type": "select",
				"name": "sys_voip_settings_log",
				"text": "Logging level",
				"descr": "Level of logging",
				"options": "0 1 2 3 4 5 6 7 8 9"
			};
			c.addWidget(field);
			
			c.addSubmit();
		}
	});
	
	/* settings tab */
	page.addTab({
		"id": "sip",
		"name": "SIP settings",
		"func": function() {
			var c, field;
			c = page.addContainer("sip");
			c.setHelpPage("voip.sip");
			c.addTitle("SIP settings");
		
			field = { 
				"type": "text",
				"name": "sys_voip_sip_registrar",
				"text": "Registrar",
				"descr": "SIP registrar to register on",
				"tip": "e.g., <i>sip:server</i>",
				"validator": {"required": true, "voipRegistrar": true}
			};
			c.addWidget(field);
			
			field = { 
				"type": "text",
				"name": "sys_voip_sip_username",
				"text": "Username",
				"descr": "Username on SIP registrar",
				"tip": "e.g., <i>user</i>",
				"validator": {"required": true}
			};
			c.addWidget(field);
			
			field = { 
				"type": "password",
				"name": "sys_voip_sip_password",
				"text": "Password",
				"descr": "Password on SIP registrar",
				"validator": {"required": true}
			};
			c.addWidget(field);
			
			field = { 
				"type": "text",
				"name": "sys_voip_sip_user_sip_uri",
				"text": "User SIP URI",
				"tip": "e.g., <i>sip:user@server</i>",
				"validator": {"required": true, "voipSipUri": true}
			};
			c.addWidget(field);
			
			/* create array with FSX ports */
			var fxsChannels = new Array();
			var channels = config.getCachedOutput("/bin/cat /proc/driver/sgatab/channels").split("<br>");
			$.each(channels, function(num, record) {
				if (record.length == 0) return true;
				
				/* channel[0] — number of channel, channel[1] — type of channel */
				var channel = record.split(":");
				if (channel[1] == "FXS") fxsChannels.push(channel[0]);
			});
			
			field = { 
				"type": "select",
				"name": "sys_voip_sip_chan",
				"text": "FXS channel",
				"descr": "FXS channel for incoming SIP-calls",
				"options": fxsChannels
			};
			c.addWidget(field);
			
			c.addSubmit();
		}
	});
	
	/* route table tab */
	
	/* route item */
	var routeItem = "sys_voip_route_";
	
	/* generate page with fields for adding new route */
	var addRouteFunc = function(item) {
		var c, field;
		page.clearTab("route");
		c = page.addContainer("route");
		c.setHelpPage("voip.route");
		c.setHelpSection("voip.route.add");

		if (!item) {
			c.addTitle("Add route");
			values = config.getParsed(routeItem + "*");
			item = routeItem + $.len(values);
		} else c.addTitle("Edit route");

		field = { 
			"type": "checkbox",
			"item": item,
			"name": "enabled",
			"text": "Enabled",
			"descr": "Check this item to enable rule"
		};
		c.addWidget(field);

		field = { 
			"type": "text",
			"item": item,
			"name": "router_id",
			"text": "Router ID",
			"descr": "Router ID",
			"validator": {"required": true, "voipRouterID": true}
		};
		c.addWidget(field);
		
		field = { 
			"type": "text",
			"item": item,
			"name": "address",
			"text": "Address",
			"descr": "Router address",
			"validator": {"required": true, "ipAddr": true}
		};
		c.addWidget(field);
		
		field = { 
			"type": "text",
			"item": item,
			"name": "comment",
			"text": "Comment",
			"descr": "Comment for this record"
		};
		c.addWidget(field);
		
		c.addSubmit({
			"complexValue": item,
			"submitName": "Add/Update",
			"extraButton": {
				"name": "Back",
				"func": showRoutes
			},
			"onSubmit": showRoutes
		});
	};
	
	var showRoutes = function() {
		var c;
		page.clearTab("route");
		c = page.addContainer("route");
		c.setHelpPage("voip.route");
		c.addTitle("Route table", 5);
	
		c.addTableHeader("Router ID|Address|Comment", addRouteFunc);
		c.generateList(routeItem + "*", "router_id address comment", addRouteFunc, showRoutes);
	};
	
	page.addTab({
		"id": "route",
		"name": "Route table",
		"func": showRoutes
	});
	
	/* address book tab */
	
	/* address item */
	var addressItem = "sys_voip_address_";
	
	/* generate page with fields for adding new route */
	var addAddressFunc = function(item) {
		var c, field;
		page.clearTab("address");
		c = page.addContainer("address");
		c.setHelpPage("voip.address");
		c.setHelpSection("voip.address.add");

		if (!item) {
			c.addTitle("Add address");
			values = config.getParsed(addressItem + "*");
			item = addressItem + $.len(values);
		} else c.addTitle("Edit address");

		field = { 
			"type": "checkbox",
			"item": item,
			"name": "enabled",
			"text": "Enabled",
			"descr": "Check this item to enable rule"
		};
		c.addWidget(field);

		field = { 
			"type": "text",
			"item": item,
			"name": "short_number",
			"text": "Short number",
			"descr": "Short number for speed dialing",
			"validator": {"required": true, "voipShortNumber": true}
		};
		c.addWidget(field);
		
		field = { 
			"type": "text",
			"item": item,
			"name": "complete_number",
			"text": "Complete number",
			"descr": "Complete telephone number",
			"tip": "Enter phone number in format: router_id-router_channel-optional_number (e.g., 300-02 or 300-02-3345), " +
				"or SIP address in format: #sip:sip_uri# (e.g., #sip:user@domain#)",
			"validator": {"required": true, "voipCompleteNumber": true}
		};
		c.addWidget(field);
		
		field = { 
			"type": "text",
			"item": item,
			"name": "comment",
			"text": "Comment",
			"descr": "Comment for this record"
		};
		c.addWidget(field);
		
		c.addSubmit({
			"complexValue": item,
			"submitName": "Add/Update",
			"extraButton": {
				"name": "Back",
				"func": showAddresses
			},
			"onSubmit": showAddresses
		});
	};
	
	var showAddresses = function() {
		var c;
		page.clearTab("address");
		c = page.addContainer("address");
		c.setHelpPage("voip.address");
		c.addTitle("Address book", 5);
	
		c.addTableHeader("Short number|Complete number|Comment", addAddressFunc);
		c.generateList(addressItem + "*", "short_number complete_number comment", addAddressFunc, showAddresses);
	};
	
	page.addTab({
		"id": "address",
		"name": "Address book",
		"func": showAddresses
	});
	
	/* Hotline tab */
	page.addTab({
		"id": "hotline",
		"name": "Hotline",
		"func": function() {
			var c, field;
			c = page.addContainer("hotline");
			c.setHelpPage("voip.hotline");
			c.addTitle("Hotline settings", 5);
			
			c.addTableHeader("Channel|Type|Hotline|Complete number|Comment");
			var channels = config.getCachedOutput("/bin/cat /proc/driver/sgatab/channels").split("<br>");
			$.each(channels, function(num, record) {
				var field;
				if (record.length == 0) return true;
				var row = c.addTableRow();
				
				/* channel[0] — number of channel, channel[1] — type of channel */
				var channel = record.split(":");
				
				field = {
					"type": "html",
					"name": channel[0],
					"str": channel[0]
				};
				c.addTableWidget(field, row);
				
				field = {
					"type": "html",
					"name": channel[1] + channel[0],
					"str": channel[1]
				};
				c.addTableWidget(field, row);
				
				field = { 
					"type": "checkbox",
					"name": $.sprintf("sys_voip_hotline_%s_hotline", channel[0]),
					"id": $.sprintf("sys_voip_hotline_%s_hotline", channel[0]),
					"tip": "Enable hotline for this channel"
				};
				c.addTableWidget(field, row);
				
				field = { 
					"type": "text",
					"name": $.sprintf("sys_voip_hotline_%s_number", channel[0]),
					"tip": "Number to call on channel event",
					"validator": 
						{
							"required": $.sprintf("#sys_voip_hotline_%s_hotline:checked", channel[0]),
							"voipCompleteNumber": true
						}
				};
				c.addTableWidget(field, row);
				
				field = { 
					"type": "text",
					"name": $.sprintf("sys_voip_hotline_%s_comment", channel[0]),
					"validator": {"alphanum": true}
				};
				c.addTableWidget(field, row);
			});
			
			c.addSubmit();
		}
	});
	
	/* Sound tab */
	page.addTab({
		"id": "sound",
		"name": "Sound settings",
		"func": function() {
			var c, field, id;
			c = page.addContainer("sound");
			c.addTitle("Sound settings", 9);
			
			c.addTableHeader("Channel|OOB|OOB_play|nEventPT|nEventPlayPT|Tx_vol|Rx_vol|VAD|HPF");
			var channels = config.getCachedOutput("/bin/cat /proc/driver/sgatab/channels").split("<br>");
			$.each(channels, function(num, record) {
				var field;
				if (record.length == 0) return true;
				var row = c.addTableRow();
				
				/* channel[0] — number of channel, channel[1] — type of channel */
				var channel = record.split(":");
				
				field = {
					"type": "html",
					"name": channel[0],
					"str": channel[0]
				};
				c.addTableWidget(field, row);
				
				/* OOB */
				field = {
					"type": "select",
					"name": $.sprintf("sys_voip_sound_%s_oob", channel[0]),
					"options": "default in-band out-of-band both block",
					"defaultValue": "default"
				};
				c.addTableWidget(field, row);
				
				/* OOB_play */
				field = {
					"type": "select",
					"name": $.sprintf("sys_voip_sound_%s_oob_play", channel[0]),
					"options": "default play mute play_diff_pt",
					"defaultValue": "default"
				};
				c.addTableWidget(field, row);
				
				/* nEventPT */
				field = {
					"type": "text",
					"name": $.sprintf("sys_voip_sound_%s_neventpt", channel[0]),
					"defaultValue": "0x62"
				};
				c.addTableWidget(field, row);
				
				/* nEventPlayPT */
				field = {
					"type": "text",
					"name": $.sprintf("sys_voip_sound_%s_neventplaypt", channel[0]),
					"defaultValue": "0x62"
				};
				c.addTableWidget(field, row);
				
				/* calculate volume values */
				var vol = "";
				for (var i = -24; i <= 24; i += 2) vol += i + " ";
				vol = $.trim(vol);
				
				/* COD_Tx_vol */
				field = {
					"type": "select",
					"name": $.sprintf("sys_voip_sound_%s_cod_tx_vol", channel[0]),
					"options": vol,
					"defaultValue": "0"
				};
				c.addTableWidget(field, row);
				
				/* COD_Rx_vol */
				field = {
					"type": "select",
					"name": $.sprintf("sys_voip_sound_%s_cod_rx_vol", channel[0]),
					"options": vol,
					"defaultValue": "0"
				};
				c.addTableWidget(field, row);
				
				/* VAD */
				field = {
					"type": "select",
					"name": $.sprintf("sys_voip_sound_%s_vad", channel[0]),
					"options": "off on g711 CNG_only SC_only",
					"defaultValue": "off"
				};
				c.addTableWidget(field, row);
				
				/* HPF */
				field = {
					"type": "select",
					"name": $.sprintf("sys_voip_sound_%s_hpf", channel[0]),
					"options": {"0": "off", "1": "on"},
					"defaultValue": "0"
				};
				c.addTableWidget(field, row);
			});
			
			c.addSubmit();
		}
	});
	
	page.generateTabs();
}
