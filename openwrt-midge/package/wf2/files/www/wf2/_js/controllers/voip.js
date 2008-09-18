Controllers['voip'] = function() {
	var page = this.Page();
	page.setSubsystem("voip");
	page.setHelp("voip");
	
	/* settings tab */
	page.addTab({
		"id": "settings",
		"name": "Settings",
		"func": function() {
			var c, field;
			c = page.addContainer("settings");
			c.addTitle("VoIP settings");
		
			field = { 
				"type": "text",
				"name": "sys_voip_settings_selfnumber",
				"text": "Router ID"
			};
			c.addWidget(field);
			
			field = { 
				"type": "text",
				"name": "sys_voip_settings_selfip",
				"text": "Router IP"
			};
			c.addWidget(field);
			
			field = { 
				"type": "text",
				"name": "sys_voip_settings_rtp_port_first",
				"text": "RTP port start",
				"descr": "Begin of ports range to use for RTP"
			};
			c.addWidget(field);
			
			field = { 
				"type": "text",
				"name": "sys_voip_settings_rtp_port_last",
				"text": "RTP port end",
				"descr": "End of ports range to use for RTP"
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
			c.addTitle("SIP settings");
		
			field = { 
				"type": "text",
				"name": "sys_voip_sip_registrar",
				"text": "Registrar",
				"descr": "SIP registrar to register on",
				"tip": "e.g., <i>sip:server</i>"
			};
			c.addWidget(field);
			
			field = { 
				"type": "text",
				"name": "sys_voip_sip_username",
				"text": "Username",
				"descr": "Username on SIP registrar",
				"tip": "e.g., <i>user</i>"
			};
			c.addWidget(field);
			
			field = { 
				"type": "password",
				"name": "sys_voip_sip_password",
				"text": "Password",
				"descr": "Password on SIP registrar"
			};
			c.addWidget(field);
			
			field = { 
				"type": "text",
				"name": "sys_voip_sip_user_sip_uri",
				"text": "User SIP URI",
				"tip": "e.g., <i>sip:user@server</i>"
			};
			c.addWidget(field);
			
			/* TODO */
			field = { 
				"type": "select",
				"name": "sys_voip_sip_chan",
				"text": "FXS channel",
				"descr": "FXS channel for incoming SIP-calls"
			};
			c.addWidget(field);
			
			c.addSubmit();
		}
	});
	
	/* route table tab */
	page.addTab({
		"id": "route",
		"name": "Route table",
		"func": function() {
			var c, field;
			c = page.addContainer("route");
			c.addTitle("Route table");
		
			
		}
	});
	
	page.generateTabs();
}