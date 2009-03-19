Controllers.voip = function() {
	var page = this.Page();
	
	/* settings tab */
	page.addTab({
		"id": "settings",
		"name": "Settings",
		"func": function() {
			var c, field;
			c = page.addContainer("settings");
			c.setSubsystem("svd-main");
			c.addTitle("VoIP settings");
			
			/* General settings */
			c.addTitle("General settings", {"internal": true, "help": {"page": "voip.settings"}});
			
			field = { 
				"type": "text",
				"name": "sys_voip_settings_rtp_port_first",
				"text": "RTP port start",
				"descr": "Begin of ports range to use for RTP.",
				"validator": {"required": true, "min": 0, "max": 65535}
			};
			c.addWidget(field);
			
			field = { 
				"type": "text",
				"name": "sys_voip_settings_rtp_port_last",
				"text": "RTP port end",
				"descr": "End of ports range to use for RTP.",
				"validator": {"required": true, "min": 0, "max": 65535}
			};
			c.addWidget(field);
			
			field = { 
				"type": "select",
				"name": "sys_voip_settings_log",
				"text": "Logging level",
				"descr": "Level of logging.",
				"options": function() {
					var options = {};
					
					options["-1"] = "off";
					for (var i = 0; i < 10; i++) {
						options[i] = i;
					}
					
					return options;
				}()
			};
			c.addWidget(field);
			
			/* SIP settings */
			c.addTitle("SIP settings", {"internal": true, "help": {"page": "voip.sip"}});
			
			field = { 
				"type": "text",
				"name": "sys_voip_sip_registrar",
				"text": "Registrar",
				"descr": "SIP registrar to register on.",
				"tip": "e.g., <i>sip:server</i>",
				"validator": {"voipRegistrar": true}
			};
			c.addWidget(field);
			
			field = { 
				"type": "text",
				"name": "sys_voip_sip_username",
				"text": "Username",
				"descr": "Username on SIP registrar.",
				"tip": "e.g., <i>user</i>"
			};
			c.addWidget(field);
			
			field = { 
				"type": "password",
				"name": "sys_voip_sip_password",
				"text": "Password",
				"descr": "Password on SIP registrar."
			};
			c.addWidget(field);
			
			field = { 
				"type": "text",
				"name": "sys_voip_sip_user_sip_uri",
				"text": "User SIP URI",
				"tip": "e.g., <i>sip:user@server</i>",
				"validator": {"voipSipUri": true}
			};
			c.addWidget(field);
			
			field = { 
				"type": "select",
				"name": "sys_voip_sip_chan",
				"text": "FXS channel",
				"descr": "FXS channel for incoming SIP-calls.",
				"options": function() {
					/* create array with FSX ports */
					var fxsChannels = [];
					var channels = config.getCachedOutput("voipChannels");
					
					if (channels) {
						$.each(channels.split("\n"), function(num, record) {
							if (record.length == 0) return true;
							
							/* channel[0] — number of channel, channel[1] — type of channel */
							var channel = record.split(":");
							if (channel[1] == "FXS") fxsChannels.push(channel[0]);
						});
					}
					
					return fxsChannels;
				}()
			};
			c.addWidget(field);
			
			c.addSubmit();
		}
	});
	
	/* Hotline tab */
	page.addTab({
		"id": "hotline",
		"name": "Hotline",
		"func": function() {
			var c, field;
			c = page.addContainer("hotline");
			c.setSubsystem("svd-hotline");
			c.setHelpPage("voip.hotline");
			c.addTitle("Hotline settings", {"colspan": 5});
			
			c.addTableHeader("Channel|Type|Hotline|Complete number|Comment");
			var channels = config.getCachedOutput("voipChannels").split("\n");
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
					"tip": "Enable hotline for this channel."
				};
				c.addTableWidget(field, row);
				
				field = { 
					"type": "text",
					"name": $.sprintf("sys_voip_hotline_%s_number", channel[0]),
					"tip": "Number to call on channel event.",
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
					"validator": {"alphanumU": true}
				};
				c.addTableWidget(field, row);
			});
			
			c.addSubmit();
		}
	});

	/* hardlink tab */
	page.addTab({
		"id": "hardlink",
		"name": "Hardlink",
		"func": function() {
			var c, field;
			c = page.addContainer("hardlink");
			c.setSubsystem("svd-hardlink");

			/* define arrays of local and remote channels */
			var localChannels = config.getCachedOutput("voipChannels").split("\n");
			var remoteChannels = [];
			for (var i = 0; i < 32; i++) {
				remoteChannels.push(((i < 10) ? "0" + i : i) + ":FXS");
			}

			var getUnusedChannels = function(channels) {
				/* list of local channels which are already in use */
				var inUse = "";
				$.each(config.getParsed("sys_voip_hardlink_*"), function(num, channel) {
					inUse += channel.ch_idx + " ";
				});

				$.each(channels, function(num, channel) {
					var channelParts = channel.split("/");
					if (inUse.search(channelParts[0]) != -1) {
						delete channels[channel];
					} else if (channelParts[1] && inUse.search(channelParts[1]) != -1) {
						delete channels[channel];
					}
				});

				return channels;
			};

			/* returns list of correct channels in proper format depending on hardink type (2w/4w) */
			var getChannelList = function(channels) {
				var result = {};

				$.each(channels, function(num, record) {
					if (record.length == 0) {
						return true;
					}

					/* channel[0] — number of channel, channel[1] — type of channel */
					var channel = record.split(":");

					/* only FXS channels */
					if (channel[1] == "FXO") {
						return true;
					}

					/* hardlink type */
					if ($("#wire_type").val() == "2w") {
						result[channel[0]] = channel[0];
					} else {
						var chNum = parseInt(channel[0], 10);
						/* even number */
						if ((chNum % 2) == 0) {
							var chString = ((chNum < 10) ? "0" + chNum : chNum)
									+ "/" + (((chNum + 1) < 10) ? "0" + (chNum + 1) : (chNum + 1));
							result[chString] = chString;
						}
					}
				});

				return result;
			};

			/* add widgets for adding/editing hardlink channels */
			var addHardlinkWidgets = function(list) {
				field = {
					"type": "select",
					"name": "wire_type",
					"text": "2-wire/4-wire",
					"descr": "Hardlink wire type: 2-wire or 4-wire.",
					"options": {"2w": "2-wire", "4w": "4-wire"},
					"onChange": function() {
						$("#ch_idx").setOptionsForSelect(
								{"options": getUnusedChannels(getChannelList(localChannels))});
						$("#pair_chan").setOptionsForSelect(
								{"options": getChannelList(remoteChannels)});
					},
					"validator": {"required": true}
				};
				list.addDynamicWidget(field);

				field = {
					"type": "select",
					"name": "ch_idx",
					"text": "Local channel",
					"descr": "FXS channel on current router.",
					"options": getUnusedChannels(getChannelList(localChannels)),
					"validator": {"required": true},
					"addCurrentValue": true
				};
				list.addDynamicWidget(field);

				field = {
					"type": "text",
					"name": "pair_route",
					"text": "Router ID",
					"descr": "ID of router to connect with.",
					"validator": {"required": true, "voipRouterIDWithSelf": true}
				};
				list.addDynamicWidget(field);

				field = {
					"type": "select",
					"name": "pair_chan",
					"text": "Remote channel",
					"descr": "FXS channel on remote router.",
					"options": getChannelList(remoteChannels),
					"validator": {"required": true}
				};
				list.addDynamicWidget(field);;

				field = {
					"type": "select",
					"name": "codec",
					"text": "Codec",
					"descr": "Codec to use.",
					"options": "aLaw uLaw",
					"validator": {"required": true}
				};
				list.addDynamicWidget(field);

				field = {
					"type": "select",
					"name": "pkt_sz",
					"text": "Packetization time (ms)",
					"descr": "Packetization time in ms.",
					"options": ["2.5", "5", "5.5", "10", "11", "20", "30", "40", "50", "60"],
					"defaultValue": "20",
					"validator": {"required": true}
				};
				list.addDynamicWidget(field);

				/* calculate volume values */
				var vol = "";
				for (var i = -24; i <= 24; i += 1) {
					vol += i + " ";
				}
				vol = $.trim(vol);

				field = {
					"type": "select",
					"name": "vol_tx",
					"text": "Tx_vol",
					"descr": "Transmit volume. This parameter has higher priority than Tx_vol on RTP tab.",
					"options": vol,
					"defaultValue": "0",
					"validator": {"required": true}
				};
				list.addDynamicWidget(field);

				field = {
					"type": "select",
					"name": "vol_rx",
					"text": "Rx_vol",
					"descr": "Recieve volume. This parameter has higher priority than Rx_vol on RTP tab.",
					"options": vol,
					"defaultValue": "0",
					"validator": {"required": true}
				};
				list.addDynamicWidget(field);
			};

			var list = c.createList({
				"tabId": "hardlink",
				"header": ["Type", "Local chan", "Router ID", "Remote chan", "Codec", "Packet. time",
						"Tx_vol", "Rx_vol"],
				"varList": ["wire_type", "ch_idx", "pair_route", "pair_chan", "codec", "pkt_sz",
						"vol_tx", "vol_rx"],
				"listItem": "sys_voip_hardlink_",
				"addMessage": "Add hardlink",
				"editMessage": "Edit hardlink",
				"listTitle": "Hardlink",
				"helpPage": "voip",
				"onAddOrEditItemRender": addHardlinkWidgets
			});

			list.generateList();
		}
	});
	
	/* route table tab */
	page.addTab({
		"id": "voipRoute",
		"name": "Routes",
		"func": function() {
			var c, field;
			c = page.addContainer("voipRoute");
			c.setSubsystem("svd-routet");
			c.setHelpPage("voip.route");
			
			var list = c.createList({
				"tabId": "voipRoute",
				"header": ["Router ID", "Address", "Comment"],
				"varList": ["router_id", "address", "comment"],
				"listItem": "sys_voip_route_",
				"addMessage": "Add VoIP route",
				"editMessage": "Edit VoIP route",
				"listTitle": "VoIP route table",
				"helpPage": "voip.route",
				"helpSection": "voip.route.add"
			});
			
			field = { 
				"type": "checkbox",
				"name": "enabled",
				"text": "Enabled",
				"descr": "Check this item to enable rule",
				"defaultState": "checked"
			};
			list.addWidget(field);
	
			field = { 
				"type": "text",
				"name": "router_id",
				"text": "Router ID",
				"descr": "Router ID",
				"validator": {"required": true, "voipRouterID": true}
			};
			list.addWidget(field);
			
			field = { 
				"type": "text",
				"name": "address",
				"text": "Address",
				"descr": "Router address",
				"validator": {"required": true, "ipAddr": true}
			};
			list.addWidget(field);
			
			field = { 
				"type": "text",
				"name": "comment",
				"text": "Comment",
				"descr": "Comment for this record"
			};
			list.addWidget(field);
			
			list.generateList();
		}
	});
	
	/* address book tab */
	page.addTab({
		"id": "address",
		"name": "Addresses",
		"func": function() {
			var c, field;
			c = page.addContainer("address");
			c.setSubsystem("svd-addressb");
			c.setHelpPage("voip.address");
			
			var list = c.createList({
				"tabId": "address",
				"header": ["Short number", "Complete number", "Comment"],
				"varList": ["short_number", "complete_number", "comment"],
				"listItem": "sys_voip_address_",
				"addMessage": "Add address",
				"editMessage": "Edit address",
				"listTitle": "Address book",
				"helpPage": "voip.address",
				"helpSection": "voip.address.add"
			});
			
			field = { 
				"type": "checkbox",
				"name": "enabled",
				"text": "Enabled",
				"descr": "Check this item to enable rule.",
				"defaultState": "checked"
			};
			list.addWidget(field);
	
			field = { 
				"type": "text",
				"name": "short_number",
				"text": "Short number",
				"descr": "Short number for speed dialing.",
				"validator": {"required": true, "voipShortNumber": true}
			};
			list.addWidget(field);
			
			field = { 
				"type": "text",
				"name": "complete_number",
				"text": "Complete number",
				"descr": "Complete telephone number.",
				"tip": "Enter phone number in format: router_id-router_channel-optional_number (e.g., 300-02 or 300-02-3345), " +
					"or SIP address in format: #sip:sip_uri# (e.g., #sip:user@domain#)",
				"validator": {"required": true, "voipCompleteNumber": true}
			};
			list.addWidget(field);
			
			field = { 
				"type": "text",
				"name": "comment",
				"text": "Comment",
				"descr": "Comment for this record."
			};
			list.addWidget(field);
			
			list.generateList();
		}
	});
	
	/* RTP tab */
	page.addTab({
		"id": "rtp",
		"name": "RTP",
		"func": function() {
			var c = page.addContainer("rtp");
			c.setSubsystem("svd-rtp");
			c.addTitle("Sound settings", {"colspan": 9});
			
			c.addTableHeader("Channel|OOB|OOB_play|nEventPT|nEventPlayPT|Tx_vol|Rx_vol|VAD|HPF");
			var channels = config.getCachedOutput("voipChannels").split("\n");
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
				for (var i = -24; i <= 24; i += 1) {
					vol += i + " ";
				}
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
	
	/* Quality tab */
	page.addTab({
		"id": "quality",
		"name": "Quality",
		"func": function() {
			var c = page.addContainer("quality");
			c.setSubsystem("svd-quality");
			c.addTitle("Codecs settings", {"colspan": 5});
			
			/* default values */
			var pktszDefault = {
				"aLaw": "20",
				"uLaw": "20",
				"g729": "10",
				"g723": "30",
				"iLBC_133": "30",
				"g729e": "10",
				"g726_16": "10",
				"g726_24": "10",
				"g726_32": "10",
				"g726_40": "10",
				"none": ""
			};
			
			var payloadDefault = {
				"aLaw": "08",
				"uLaw": "00",
				"g729": "12",
				"g723": "4",
				"iLBC_133": "100",
				"g729e": "101",
				"g726_16": "102",
				"g726_24": "103",
				"g726_32": "104",
				"g726_40": "105",
				"none": ""
			};
			
			var codecs = ["g729", "aLaw", "uLaw", "g723", "iLBC_133",
				"g729e", "g726_16", "g726_24", "g726_32", "g726_40", "none"];

			/*
			 * At one time only one type of codec can be selected.
             * If current codec type is none, set next to none too and disable all next.
			 * 
			 * scope — settings scope;
			 * i — codec index.
			 */
			var setUniqueType = function(scope, i) {
				setDefaults(scope, i);
				var newVal = $($.sprintf("#sys_voip_quality_%s_codec%s_type", scope, i)).val();
				if (newVal != "none") {
					$(".type_" + scope).not($.sprintf("#sys_voip_quality_%s_codec%s_type", scope, i))
                            .each(function(num, element) {
						if ($(element).val() == newVal) {
                            $(element).val("none");
                        }
					});
				}
				
				var allNone = true;
                var isNone = false;
				$(".type_" + scope).each(function(num, element) {
					/* set type of current codec to none, if we've set isNone to true in previous step */
                    if (isNone) {
						$(element).val("none").attr("readonly", true).attr("disabled", true);
                        setDefaults(scope, num);
                    /* if type of current codec is none, next will be set to none too */
                    } else if ($(element).val() == "none") {
                        isNone = true;
                        $(element).removeAttr("readonly").removeAttr("disabled");
					}

                    if ($(element).val() != "none") {
						allNone = false;
					}
				});

                /* if all types is "none", set first to aLaw. */
				if (allNone) {
                    $(".type_" + scope).eq(0).val("aLaw");
                    setDefaults(scope, 0);

                    $(".type_" + scope).eq(1).removeAttr("readonly").removeAttr("disabled");
                }
			};
			
			/*
			 * Set default values depending on selected code type.
			 * 
			 * scope — settings scope;
			 * i — codec index.
			 */
			var setDefaults = function(scope, i) {
				var codec = $($.sprintf("#sys_voip_quality_%s_codec%s_type", scope, i)).val();
				
				$($.sprintf("#sys_voip_quality_%s_codec%s_pktsz", scope, i)).val(pktszDefault[codec]);
				$($.sprintf("#sys_voip_quality_%s_codec%s_payload", scope, i)).val(payloadDefault[codec]);
				
				/* set default for bitpack */
				if (codec.search("g726") != -1) {
					$($.sprintf("#sys_voip_quality_%s_codec%s_bitpack", scope, i)).val("aal2");
				} else {
					$($.sprintf("#sys_voip_quality_%s_codec%s_bitpack", scope, i)).val("rtp");
				}
				
				setPtksz(scope, i);
				setBitpackReadonly(scope, i);
			};
			
			/*
			 * Set for some codecs bitpack readonly.
			 */
			var setBitpackReadonly = function(scope, i) {
				var codec = $($.sprintf("#sys_voip_quality_%s_codec%s_type", scope, i)).val();
				
				if (codec.search("g726") != -1) {
					$($.sprintf("#sys_voip_quality_%s_codec%s_bitpack", scope, i))
						.removeAttr("readonly").removeAttr("disabled");
				} else {
					$($.sprintf("#sys_voip_quality_%s_codec%s_bitpack", scope, i))
						.attr("readonly", true).attr("disabled", true);
				}
			};
			
			/*
			 * Set for some codecs pkt_sz readonly.
			 */
			var setPtksz = function(scope, i) {
				var codec = $($.sprintf("#sys_voip_quality_%s_codec%s_type", scope, i)).val();
				
				/* for iLBC* pkt_sz is read-only */
				if (codec == "iLBC_133") {
					/* set disabled and readonly attributes */
					$($.sprintf("#sys_voip_quality_%s_codec%s_pktsz", scope, i))
						.attr("readonly", true).attr("disabled", true);
				/* for g723 set fixed values: 30 and 60 */
                } else if (codec == "g723") {
                    var field = $($.sprintf("#sys_voip_quality_%s_codec%s_pktsz", scope, i));
                    field.setOptionsForSelect({"options": "30 60", "curValue": field.val()});
                } else {
					/* remove disabled and readonly attributes */
					$($.sprintf("#sys_voip_quality_%s_codec%s_pktsz", scope, i))
						.removeAttr("readonly").removeAttr("disabled");
				}
			};
			
			/*
			 * Add specified number of widgets for specified scope.
			 * 
			 * num — number of widgets;
			 * scope — scope (external, internal).
			 */
			var addCodecsWidgets = function(num, scope) {
				for (var i = 0; i < num; i++) {
					var field;
					var row = c.addTableRow();
					
					/* priority */
					field = {
						"type": "html",
						"name": "priority_" + scope + i,
						"str": "" + i
					};
					c.addTableWidget(field, row);
					
					/* type */
					field = {
						"type": "select",
						"name": $.sprintf("sys_voip_quality_%s_codec%s_type", scope, i),
						"options": codecs,
						"cssClass": "type_" + scope,
						/*
						 * We use double-closure here, because in single-closure each
						 * onChange callback will have var i with value of 3. More info
						 * in http://dklab.ru/chicken/nablas/39.html.
						 */
						"onChange": function(x) {
							return function() {
								setUniqueType(scope, x);
							}
						}(i)
					};
					if (num > 2) {
                        field.defaultValue = "none";
                    }
					c.addTableWidget(field, row);
					
					/* pkt_sz */
					field = {
						"type": "select",
						"name": $.sprintf("sys_voip_quality_%s_codec%s_pktsz", scope, i),
						"options": ["2.5", "5", "5.5", "10", "11", "20", "30", "40", "50", "60"]
					};
					c.addTableWidget(field, row);
					setPtksz(scope, i);
					
					/* payload */
					field = {
						"type": "text",
						"name": $.sprintf("sys_voip_quality_%s_codec%s_payload", scope, i),
						"cssClass": "voipQualityPayload payload_" + scope,
						"validator":
						{
							"required": function(x) {
								return function() {
									return $($.sprintf("#sys_voip_quality_%s_codec%s_type", scope, x)).val() != "none";
								}
							}(i),
							"voipPayload": true,
							"uniqueValue": ".payload_" + scope
						}
					};
					c.addTableWidget(field, row);
					
					/* bitpack */
					field = {
						"type": "select",
						"name": $.sprintf("sys_voip_quality_%s_codec%s_bitpack", scope, i),
						"options": "rtp aal2"
					};
					c.addTableWidget(field, row);
					setBitpackReadonly(scope, i);

                    if (i == (num - 1)) {
                        setUniqueType(scope, i);
                    }
				}
			};
			
			c.addTableHeader("Priority*|Type**|Packetization time (ms)|Payload|Bitpack");
			c.addTableTfootStr("* 0 — max priority.", 5);
            c.addTableTfootStr("** Each codec can be selected only once.", 5);
			
			c.addTitle("Internal", {"internal": true, "colspan": 5});
			addCodecsWidgets(codecs.length - 1, "int");
			
			c.addTitle("External", {"internal": true, "colspan": 5});
			addCodecsWidgets(codecs.length - 1, "ext");
			
			c.addTitle("Fax", {"internal": true, "colspan": 5});
			var row = c.addTableRow();
			
			/* add fake widget */
			c.addGeneralTableWidget({"name": "fax_fake1"}, row);
			
			/* fax type */
			var field = {
				"type": "select",
				"name": "sys_voip_quality_fax_type",
				"options": ["uLaw", "aLaw"]
			};
			c.addTableWidget(field, row);
			
			/* add three fake widgets */
			c.addGeneralTableWidget({"name": "fax_fake2"}, row);
			c.addGeneralTableWidget({"name": "fax_fake3"}, row);
			c.addGeneralTableWidget({"name": "fax_fake4"}, row);
			
			c.addSubmit({
				/* remove disabled attribute */
				"preSubmit": function() {
					$("[disabled]").removeAttr("disabled");
				},
				/* return disabled attribute back on readonly elements */
				"onSubmit": function() {
					$("[readonly]").attr("disabled", true);
				}
			});
		}
	});
	
	page.generateTabs();
};
