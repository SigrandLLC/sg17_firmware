/*
* Parameters for codecs:
* - pkt_sz — default value;
* - pkt_sz_ro — read-only (default set to false);
* - pkt_sz_vals — available values (by default "2.5 5 5.5 10 11 20 30 40 50 60");
* - payload — default value;
* - bitpack — default value;
* - bitpack_ro — read-only (default set to false);
*/
globalParameters.codecsParameters = {
    "aLaw":     {"pkt_sz": "60", "payload": "08",  "bitpack": "rtp",  "bitpacks": "rtp",      "pkt_sz_vals": "5 5.5 10 11 20 30 40 50 60", "nMax": {"5": 100, "5.5": 110, "10": 200, "11": 110, "20": 200, "30": 200, "40": 200, "50": 200, "60": 200}, "nInitFixed": 150},
    "g729":     {"pkt_sz": "60", "payload": "18",  "bitpack": "rtp",  "bitpacks": "rtp",      "pkt_sz_vals": "10 20 30 40 60",             "nMax": {"all": 200}},
    "g723":     {"pkt_sz": "60", "payload": "4",   "bitpack": "rtp",  "bitpacks": "rtp",      "pkt_sz_vals": "30 60",                      "nMax": {"all": 600}},
    "iLBC_133": {"pkt_sz": "30", "payload": "100", "bitpack": "rtp",  "bitpacks": "rtp",      "pkt_sz_vals": "30",                         "nMax": {"all": 600}},
    "g729e":    {"pkt_sz": "60", "payload": "101", "bitpack": "rtp",  "bitpacks": "rtp",      "pkt_sz_vals": "10 20 30 40 60",             "nMax": {"all": 200}},
    "g726_16":  {"pkt_sz": "60", "payload": "102", "bitpack": "aal2", "bitpacks": "rtp aal2", "pkt_sz_vals": "5 5.5 10 11 20 30 40 50 60", "nMax": {"5": 100, "5.5": 110, "10": 200, "11": 110, "20": 200, "30": 200, "40": 200, "50": 200, "60": 200}},
    "g726_24":  {"pkt_sz": "60", "payload": "103", "bitpack": "aal2", "bitpacks": "rtp aal2", "pkt_sz_vals": "5 10 20 30 40 50 60",        "nMax": {"5": 100, "10":  200, "20": 200, "30": 200, "40": 200, "50": 200, "60": 200}},
    "g726_32":  {"pkt_sz": "60", "payload": "104", "bitpack": "aal2", "bitpacks": "rtp aal2", "pkt_sz_vals": "5 5.5 10 11 20 30 40 50 60", "nMax": {"5": 100, "5.5": 110, "10": 200, "11": 110, "20": 200, "30": 200, "40": 200, "50": 200, "60": 200}},
    "g726_40":  {"pkt_sz": "60", "payload": "105", "bitpack": "aal2", "bitpacks": "rtp aal2", "pkt_sz_vals": "5 10 20 30 40 50 60",        "nMax": {"5": 100, "10":  200, "20": 200, "30": 200, "40": 200, "50": 200, "60": 200}}
};

/* General settings */
Controllers.voipSettings = function() {
    var page = this.Page();
    
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

            /* ToS settings */
            c.addTitle("ToS settings", {"internal": true});

            field = {
                "type": "text",
                "name": "sys_voip_settings_rtp_tos",
                "text": "RTP ToS",
                "descr": "ToS (8 bits) for RTP packets.",
                "validator": {"required": true, "tos": true}
            };
            c.addWidget(field);

            field = {
                "type": "text",
                "name": "sys_voip_settings_sip_tos",
                "text": "SIP ToS",
                "descr": "ToS (8 bits) for SIP packets.",
                "validator": {"required": true, "tos": true}
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
                            if (channel[1] == "FXS") {
                                fxsChannels.push(channel[0]);
                            }
                        });
                    }
                    
                    return fxsChannels;
                }()
            };
            c.addWidget(field);
            
            c.addSubmit();
        }
    });
    
    page.generateTabs();
};

/* Hotline, FXO, FXS */
Controllers.voipHotline = function() {
    var page = this.Page();

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
                if (record.length == 0) {
                    return true;
                }

                /* channel[0] — number of channel, channel[1] — type of channel */
                var channel = record.split(":");

                /* VF channels are not supported */
                if (channel[1] == "VF") {
                    return true;
                }

                var row = c.addTableRow();

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

    page.generateTabs();
};

/* VF */
Controllers.voipVF = function() {
    var page = this.Page();

    page.addTab({
        "id": "vf",
        "name": "Voice frequency channels",
        "func": function() {
            var c = page.addContainer("vf");
            c.setSubsystem("svd-vf");

            var colNum = 8;
            c.addTitle("Voice frequency channels", {"colspan": colNum});

            /*
             * Parameters for codecs:
             * - pkt_sz — default value;
             * - pkt_sz_ro — read-only (default set to false);
             * - pkt_sz_vals — available values (by default "2.5 5 5.5 10 11 20 30 40 50 60");
             * - payload — default value;
             * - bitpack — default value;
             * - bitpack_ro — read-only (default set to false);
             */
            var codecsParameters = {
                "aLaw": {"pkt_sz": "60", "payload": "08", "bitpack": "rtp", "bitpack_ro": true, "pkt_sz_vals": "5 5.5 10 11 20 30 40 50 60"},
                "g729": {"pkt_sz": "60", "payload": "18", "bitpack": "rtp", "bitpack_ro": true, "pkt_sz_vals": "10 20 30 40 60"},
                "g723": {"pkt_sz": "60", "payload": "4", "bitpack": "rtp", "bitpack_ro": true, "pkt_sz_vals": "30 60"},
                "iLBC_133": {"pkt_sz": "30", "payload": "100", "bitpack": "rtp", "bitpack_ro": true, "pkt_sz_ro": true},
                "g729e": {"pkt_sz": "60", "payload": "101", "bitpack": "rtp", "bitpack_ro": true, "pkt_sz_vals": "10 20 30 40 60"},
                "g726_16": {"pkt_sz": "60", "payload": "102", "bitpack": "aal2", "pkt_sz_vals": "5 5.5 10 11 20 30 40 50 60"},
                "g726_24": {"pkt_sz": "60", "payload": "103", "bitpack": "aal2", "pkt_sz_vals": "5 10 20 30 40 50 60"},
                "g726_32": {"pkt_sz": "60", "payload": "104", "bitpack": "aal2", "pkt_sz_vals": "5 5.5 10 11 20 30 40 50 60"},
                "g726_40": {"pkt_sz": "60", "payload": "105", "bitpack": "aal2", "pkt_sz_vals": "5 10 20 30 40 50 60"}
            };

            var pktszDefaultValues = "2.5 5 5.5 10 11 20 30 40 50 60";

            /* set codec parameters */
            var onCodecChange = function(channel) {
                var codec = $($.sprintf("#sys_voip_vf_channels_%s_codec", channel)).val();

                /* set values for pkt_sz */
                $($.sprintf("#sys_voip_vf_channels_%s_pkt_sz", channel)).setOptionsForSelect({
                        "options": codecsParameters[codec].pkt_sz_vals == undefined
                                ? pktszDefaultValues
                                : codecsParameters[codec].pkt_sz_vals
                });

                /* set pkt_sz default */
                $($.sprintf("#sys_voip_vf_channels_%s_pkt_sz", channel))
                        .val(codecsParameters[codec].pkt_sz);

                /* set/unset pkt_sz read-only */
                $($.sprintf("#sys_voip_vf_channels_%s_pkt_sz", channel))
                        .setSelectReadonly(codecsParameters[codec].pkt_sz_ro == undefined
                                ? false
                                : codecsParameters[codec].pkt_sz_ro);

                /* set payload default */
                $($.sprintf("#sys_voip_vf_channels_%s_payload", channel))
                        .val(codecsParameters[codec].payload);

                /* set bitpack default */
                $($.sprintf("#sys_voip_vf_channels_%s_bitpack", channel))
                        .val(codecsParameters[codec].bitpack);

                /* set/unset bitpack read-only */
                $($.sprintf("#sys_voip_vf_channels_%s_bitpack", channel))
                        .setSelectReadonly(codecsParameters[codec].bitpack_ro == undefined
                                ? false
                                : codecsParameters[codec].bitpack_ro);
            };

            c.addTableHeader("Chan|EN|Router ID|R. chan|Codec|Packet. time|Payload|Bitpack");
            c.addTableTfootStr("Chan - local channel.", colNum);
            c.addTableTfootStr("EN - enable channel.", colNum);
            c.addTableTfootStr("Router ID - ID of a router to connect with.", colNum);
            c.addTableTfootStr("R. chan - VF channel on the remote router.", colNum);
            c.addTableTfootStr("Codec - codec to use.", colNum);
            c.addTableTfootStr("Packet. time - packetization time in ms.", colNum);
            c.addTableTfootStr("Payload - RTP codec identificator.", colNum);
            c.addTableTfootStr("Bitpack - bits packetization type.", colNum);

            var channels = config.getCachedOutput("voipChannels").split("\n");
            $.each(channels, function(num, record) {
                var field;
                if (record.length == 0) {
                    return true;
                }
                var row = c.addTableRow();

                /* channel[0] — number of channel, channel[1] — type of channel */
                var channel = record.split(":");

                /* only VF channels */
                if (channel[1] != "VF") {
                    return true;
                }

                /* local channel */
                field = {
                    "type": "html",
                    "name": channel[0],
                    "str": channel[0]
                };
                c.addTableWidget(field, row);

                /* enabled */
                field = {
                    "type": "checkbox",
                    "name": $.sprintf("sys_voip_vf_channels_%s_enabled", channel[0])
                };
                c.addTableWidget(field, row);

                /* pair_route */
                field = {
                    "type": "text",
                    "name": $.sprintf("sys_voip_vf_channels_%s_pair_route", channel[0]),
                    "validator": {
                            "required": $.sprintf("#sys_voip_vf_channels_%s_enabled:checked", channel[0]),
                            "voipRouterIDWithSelf": true
                    }
                };
                c.addTableWidget(field, row);

                /* pair_chan */
                field = {
                    "type": "select",
                    "name": $.sprintf("sys_voip_vf_channels_%s_pair_chan", channel[0]),
                    "options": function() {
                        var remoteChannels = [];
                        for (var i = 0; i < 32; i++) {
                            remoteChannels.push(((i < 10) ? "0" + i : i));
                        }
                        return remoteChannels;
                    }()
                };
                c.addTableWidget(field, row);

                /* codec */
                field = {
                    "type": "select",
                    "name": $.sprintf("sys_voip_vf_channels_%s_codec", channel[0]),
                    "options": function() {
                        var codecs = [];
                        $.each(codecsParameters, function(key) {
                            codecs.push(key);
                        });
                        return codecs;
                    }(),
                    "onChange": function() {
                        onCodecChange(channel[0]);
                    }
                };
                c.addTableWidget(field, row);

                var codec = $($.sprintf("#sys_voip_vf_channels_%s_codec", channel[0])).val();

                /* pkt_sz */
                field = {
                    "type": "select",
                    "name": $.sprintf("sys_voip_vf_channels_%s_pkt_sz", channel[0]),
                    "options": codecsParameters[codec].pkt_sz_vals == undefined
                            ? pktszDefaultValues
                            : codecsParameters[codec].pkt_sz_vals,
                    "defaultValue": codecsParameters[codec].pkt_sz
                };
                c.addTableWidget(field, row);

                /* set/unset pkt_sz read-only */
                $($.sprintf("#sys_voip_vf_channels_%s_pkt_sz", channel[0]))
                        .setSelectReadonly(codecsParameters[codec].pkt_sz_ro == undefined
                                ? false
                                : codecsParameters[codec].pkt_sz_ro);

                /* payload */
                field = {
                    "type": "text",
                    "name": $.sprintf("sys_voip_vf_channels_%s_payload", channel[0]),
                    "defaultValue": codecsParameters[codec].payload,
                    "validator": {
                            "required": $.sprintf("#sys_voip_vf_channels_%s_enabled:checked", channel[0]),
                            "voipPayload": true
                    }
                };
                c.addTableWidget(field, row);

                /* bitpack */
                field = {
                    "type": "select",
                    "name": $.sprintf("sys_voip_vf_channels_%s_bitpack", channel[0]),
                    "options": "rtp aal2",
                    "defaultValue": codecsParameters[codec].bitpack
                };
                c.addTableWidget(field, row);

                /* set/unset bitpack read-only */
                $($.sprintf("#sys_voip_vf_channels_%s_bitpack", channel[0]))
                        .setSelectReadonly(codecsParameters[codec].bitpack_ro == undefined
                                ? false
                                : codecsParameters[codec].bitpack_ro);
            });

            c.addSubmit();
        }
    });

    page.addTab({
        "id": "vf_settings",
        "name": "Settings",
        "func": function() {
            var c = page.addContainer("vf_settings");
            c.setSubsystem("svd-vf_settings");
            c.addTitle("Settings", {"colspan": 3});

            c.addTableHeader("Channel|Wires|Transmit type");
            c.addTableTfootStr("4-Wire Normal: Tx (In) = -13 dBr, Rx (Out) = +4 dBr", 3);
            c.addTableTfootStr("4-Wire Transit: Tx (In) = +4 dBr, Rx (Out) = +4 dBr", 3);
            c.addTableTfootStr("2-Wire Normal: Tx (In) = 0 dBr, Rx (Out) = -7 dBr", 3);
            c.addTableTfootStr("2-Wire Transit: Tx (In) = -3.5 dBr, Rx (Out) = -3.5 dBr", 3);

            var channels = config.getCachedOutput("voipChannels").split("\n");
            $.each(channels, function(num, record) {
                var field;
                if (record.length == 0) {
                    return true;
                }
                var row = c.addTableRow();

                /* channel[0] — number of channel, channel[1] — type of channel */
                var channel = record.split(":");

                /* only VF channels */
                if (channel[1] != "VF") {
                    return true;
                }

                field = {
                    "type": "html",
                    "name": channel[0],
                    "str": channel[0]
                };
                c.addTableWidget(field, row);

                field = {
                    "type": "select",
                    "name": $.sprintf("sys_voip_vf_settings_%s_wire_type", channel[0]),
                    "options": {"2w": "2-wire", "4w": "4-wire"},
                    "defaultValue": "4w"
                };
                c.addTableWidget(field, row);

                field = {
                    "type": "select",
                    "name": $.sprintf("sys_voip_vf_settings_%s_transmit_type", channel[0]),
                    "options": "normal transit"
                };
                c.addTableWidget(field, row);
            });

            c.addSubmit();
        }
    });

    page.generateTabs();
};

/* Routes */
Controllers.voipRoutes = function() {
    var page = this.Page();

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
                "header": ["Router ID", "IP-address", "Comment"],
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

    page.generateTabs();
};

/* Phone book */
Controllers.voipPhoneBook = function() {
    var page = this.Page();

    page.addTab({
        "id": "address",
        "name": "Phone book",
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
                "addMessage": "Add record",
                "editMessage": "Edit record",
                "listTitle": "Phone book",
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

    page.generateTabs();
};

/* Audio, FXO, FXS, VF */
Controllers.voipAudio = function() {
    var page = this.Page();

    page.addTab({
        "id": "rtp",
        "name": "Audio",
        "func": function() {
            var c = page.addContainer("rtp");
            var colNum = 6;
            c.setSubsystem("svd-rtp");
            c.addTitle("Audio settings", {"colspan": colNum});

            c.addTableHeader("Channel|Type|Tx.C|Rx.C|VAD|HPF");
            c.addTableTfootStr("Tx.C: Transmit volume settings for Coder module (outcome volume level).", colNum);
            c.addTableTfootStr("Rx.C: Receive volume settings for Coder module (income volume level).", colNum);
            c.addTableTfootStr("VAD:", colNum);
            c.addTableTfootStr(" - On: voice activity detection on, in this case also comfort noise and spectral information (nicer noise) is switched on.", colNum);
            c.addTableTfootStr(" - Off: no voice activity detection.", colNum);
            c.addTableTfootStr(" - G711: voice activity detection on with comfort noise generation without spectral information.", colNum);
            c.addTableTfootStr(" - CNG_only: voice activity detection on with comfort noise generation without silence compression.", colNum);
            c.addTableTfootStr(" - SC_only: voice activity detection on with silence compression without comfort noise generation.", colNum);
            c.addTableTfootStr("HPF: income high-pass filter.", colNum);

            var channels = config.getCachedOutput("voipChannels").split("\n");
            $.each(channels, function(num, record) {
                var field;
                if (record.length == 0) {
                    return true;
                }
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
                    "name": channel[0] + "_type",
                    "str": channel[1]
                };
                c.addTableWidget(field, row);

                /* calculate volume values */
                var vol = "";
                for (var i = -24; i <= 24; i += 1) {
                    vol += i + " ";
                }
                vol = $.trim(vol);

                /* Tx.C */
                field = {
                    "type": "select",
                    "name": $.sprintf("sys_voip_sound_%s_cod_tx_vol", channel[0]),
                    "options": vol,
                    "defaultValue": "0"
                };
                c.addTableWidget(field, row);

                /* Rx.C */
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
};

/* Echo, FXO, FXS, VF */
Controllers.voipEcho = function() {
    var page = this.Page();

    page.addTab({
        "id": "wlec",
        "name": "Echo",
        "func": function() {
            var c = page.addContainer("wlec");
            var colNum = 6;
            c.setSubsystem("svd-wlec");
            c.addTitle("Window-based Line Echo Canceller", {"colspan": colNum});

            c.addTableHeader("Channel|Type|WLEC type|NLP|Near-end window|Far-end window");
            c.addTableTfootStr("WLEC type: ", colNum);
            c.addTableTfootStr("- NE: near-end only.", colNum);
            c.addTableTfootStr("- NFE: near-end and far-end.", colNum);
            c.addTableTfootStr("NLP: Non-linear processing.", colNum);
            c.addTableTfootStr("Near-end window: Near-end window (narraw band).", colNum);
            c.addTableTfootStr("Far-end window: Far-end window (narrow band).", colNum);

            var channels = config.getCachedOutput("voipChannels").split("\n");
            $.each(channels, function(num, record) {
                var field;
                if (record.length == 0) {
                    return true;
                }
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
                    "name": channel[0] + "_type",
                    "str": channel[1]
                };
                c.addTableWidget(field, row);

                /* Type, by default for VF is OFF, for others is NE */
                field = {
                    "type": "select",
                    "name": $.sprintf("sys_voip_wlec_%s_type", channel[0]),
                    "options": "off NE NFE",
                    "defaultValue": channel[1] == "VF" ? "off" : "NE",
                    "onChange": function() {
                        var values = $("#" + this.id).val() == "NFE" ? "4 6 8" : "4 6 8 16";
                        $($.sprintf("#sys_voip_wlec_%s_new_nb", channel[0])).setOptionsForSelect({
                                "options": values
                        });
                        $($.sprintf("#sys_voip_wlec_%s_few_nb", channel[0])).setOptionsForSelect({
                                "options": values
                        });
                    }
                };
                c.addTableWidget(field, row);

                /* NLP, by default for FXO is ON, for others is OFF */
                field = {
                    "type": "select",
                    "name": $.sprintf("sys_voip_wlec_%s_nlp", channel[0]),
                    "options": "off on",
                    "defaultValue": channel[1] == "FXO" ? "on" : "off"
                };
                c.addTableWidget(field, row);

                /* Near-end window NB */
                var type = $($.sprintf("#sys_voip_wlec_%s_type", channel[0])).val();
                field = {
                    "type": "select",
                    "name": $.sprintf("sys_voip_wlec_%s_new_nb", channel[0]),
                    "options": type == "NFE" ? "4 6 8" : "4 6 8 16",
                    "defaultValue": "4"
                };
                c.addTableWidget(field, row);

                /* Far-end window NB */
                field = {
                    "type": "select",
                    "name": $.sprintf("sys_voip_wlec_%s_few_nb", channel[0]),
                    "options": type == "NFE" ? "4 6 8" : "4 6 8 16",
                    "defaultValue": "4"
                };
                c.addTableWidget(field, row);
            });

            c.addSubmit();
        }
    });

    page.generateTabs();
};

/* Dial mode, FXO */
Controllers.voipDialMode = function() {
    var page = this.Page();

    page.addTab({
        "id": "fxo",
        "name": "Dial mode",
        "func": function() {
            var c = page.addContainer("fxo");
            c.setSubsystem("svd-fxo");
            c.addTitle("Dial mode settings for FXO channels", {"colspan": 3});

            c.addTableHeader("Channel|Type|PSTN type*");
            c.addTableTfootStr("tone/pulse - tone or pulse.", 3);
            c.addTableTfootStr("pulse - pulse only.", 3);

            var channels = config.getCachedOutput("voipChannels").split("\n");
            $.each(channels, function(num, record) {
                var field;
                if (record.length == 0) {
                    return true;
                }
                var row = c.addTableRow();

                /* channel[0] — number of channel, channel[1] — type of channel */
                var channel = record.split(":");

                /* only FXO channels */
                if (channel[1] != "FXO") {
                    return true;
                }

                field = {
                    "type": "html",
                    "name": channel[0],
                    "str": channel[0]
                };
                c.addTableWidget(field, row);

                field = {
                    "type": "html",
                    "name": channel[0] + "_type",
                    "str": channel[1]
                };
                c.addTableWidget(field, row);

                /* PSTN_type */
                field = {
                    "type": "select",
                    "name": $.sprintf("sys_voip_fxo_%s_pstn_type", channel[0]),
                    "options": {"tone": "tone/pulse", "pulse": "pulse"}
                };
                c.addTableWidget(field, row);
            });

            c.addSubmit();
        }
    });

    page.generateTabs();
};

/* Codecs settings */
Controllers.voipCodecs = function() {
    var page = this.Page();

    /* function for generating page with codecs */
    var codecsTab = function(container, scope, scopeTitle) {
        var c = page.addContainer(container);
        c.setSubsystem("svd-codecs");

        /* hash with fields validators, which can change dynamically */
        var validators = {};
        
        /* for all codecs nMin is 10 */
        var nMin = 10;

        /* on pkt_sz value changing */
        var onPktszChange = function(scope, codec) {
            var params        = globalParameters.codecsParameters[codec];
            var nMinFieldName = $.sprintf("sys_voip_codecs_%s_%s_n_min_size", scope, codec);
            var nMaxFieldName = $.sprintf("sys_voip_codecs_%s_%s_n_max_size", scope, codec);
            
            /* current pkt_sz */
            var pkt_sz        = $($.sprintf("#sys_voip_codecs_%s_%s_pktsz", scope, codec)).val();
            
            /* change max value for nMin and nMax */
            validators[nMinFieldName]['max'] = validators[nMaxFieldName]['max']
                                             = params.nMax['all'] ? params.nMax['all'] : params.nMax[pkt_sz];
            
            /* change nMax default value */
            $("#" + nMaxFieldName).val(validators[nMaxFieldName]['max']);
        };
        
        /* on nMax value changing */
        var onNMaxChange = function(scope, codec) {
            var nMinFieldName = $.sprintf("sys_voip_codecs_%s_%s_n_min_size", scope, codec);
            var nMaxFieldName = $.sprintf("sys_voip_codecs_%s_%s_n_max_size", scope, codec);
            
            /* set correct maximum value for nMin to current value of nMax */
            validators[nMinFieldName]['max'] = $("#" + nMaxFieldName).val();
        };
        
        /* on JB type changing */
        var onJbTypeChange = function(scope, codec) {
            var params         = globalParameters.codecsParameters[codec];
            var jbType         = $($.sprintf("#sys_voip_codecs_%s_%s_jb_type", scope, codec)).val();
            var nInitFieldName = $.sprintf("sys_voip_codecs_%s_%s_n_init_size", scope, codec);
            var mutableAttrs   = ["n_min_size", "n_max_size"];
            
            /* current pkt_sz */
            var pkt_sz         = $($.sprintf("#sys_voip_codecs_%s_%s_pktsz", scope, codec)).val();
            
            /* set/unset local_at read-only */
            $($.sprintf("#sys_voip_codecs_%s_%s_lat", scope, codec))
                    .setSelectReadonly(jbType == "adaptive" ? false : true);
            
            if (jbType == "fixed") {
                /* remove dynamicRange validator from nInit */
                delete validators[nInitFieldName]['dynamicRange'];
                
                /* set min and max validators to nInit */
                validators[nInitFieldName]['min'] = nMin;
                validators[nInitFieldName]['max'] = params.nInitFixed ? params.nInitFixed :
                                                    params.nMax['all'] ? params.nMax['all'] : params.nMax[pkt_sz];

                /* set nMin and nMax read only */
                $(mutableAttrs).each(function(num, attr) {
                    $($.sprintf("#sys_voip_codecs_%s_%s_%s", scope, codec, attr)).attr("readonly", true);
                });
            } else {
                /* remove min and max validators from nInit */
                delete validators[nInitFieldName]['min'];
                delete validators[nInitFieldName]['max'];
                
                /* set dynamicRange validator to nInit */
                validators[nInitFieldName]['dynamicRange'] = [
                        $.sprintf("#sys_voip_codecs_%s_%s_n_min_size", scope, codec),
                        $.sprintf("#sys_voip_codecs_%s_%s_n_max_size", scope, codec)
                ];
                
                /* set nMin and nMax writable */
                $(mutableAttrs).each(function(num, attr) {
                    $($.sprintf("#sys_voip_codecs_%s_%s_%s", scope, codec, attr)).removeAttr("readonly");
                });
            }
        };

        /* add widgets for specified scope */
        var addCodecsWidgets = function(scope) {
            var field;
            
            $.each(globalParameters.codecsParameters, function(codec, params) {
                var row = c.addTableRow();
                
                /* codec name */
                field = {
                    "type": "html",
                    "name": $.sprintf("codec_%s_%s", scope, codec),
                    "str": codec
                };
                c.addTableWidget(field, row);

                /* pkt_sz */
                field = {
                    "type": "select",
                    "name": $.sprintf("sys_voip_codecs_%s_%s_pktsz", scope, codec),
                    "options": params.pkt_sz_vals,
                    "defaultValue": params.pkt_sz,
                    "onChange": function() {
                        onPktszChange(scope, codec);
                    }
                };
                c.addTableWidget(field, row);

                /* payload */
                field = {
                    "type": "text",
                    "name": $.sprintf("sys_voip_codecs_%s_%s_payload", scope, codec),
                    "defaultValue": params.payload
                };
                c.addTableWidget(field, row);

                /* bitpack */
                field = {
                    "type": "select",
                    "name": $.sprintf("sys_voip_codecs_%s_%s_bitpack", scope, codec),
                    "options": params.bitpacks,
                    "defaultValue": params.bitpack
                };
                c.addTableWidget(field, row);
                
                /* jb_type */
                field = {
                    "type": "select",
                    "name": $.sprintf("sys_voip_codecs_%s_%s_jb_type", scope, codec),
                    "options": {"fixed": "Fixed", "adaptive": "Adaptive"},
                    "defaultValue": "adaptive",
                    "onChange": function() {
                        onJbTypeChange(scope, codec);
                    }
                };
                c.addTableWidget(field, row);

                /* lat */
                field = {
                    "type": "select",
                    "name": $.sprintf("sys_voip_codecs_%s_%s_lat", scope, codec),
                    "options": "off on SI",
                    "defaultValue": "off"
                };
                c.addTableWidget(field, row);

                /* n_scaling */
                field = {
                    "type": "text",
                    "name": $.sprintf("sys_voip_codecs_%s_%s_n_scaling", scope, codec),
                    "defaultValue": "1.4",
                    "validator": {"min": 1, "max": 16}
                };
                c.addTableWidget(field, row);
                
                /* current pkt_sz */
                var pkt_sz = $($.sprintf("#sys_voip_codecs_%s_%s_pktsz", scope, codec)).val();
                
                /* nMax for current codec and pkt_sz */
                var nMax = params.nMax['all'] ? params.nMax['all'] : params.nMax[pkt_sz];

                /* n_init_size. validator is depending on JB type and is setting later */
                var nInitFieldName = $.sprintf("sys_voip_codecs_%s_%s_n_init_size", scope, codec);
                validators[nInitFieldName] = {};
                field = {
                    "type": "text",
                    "name": nInitFieldName,
                    "defaultValue": (nMin + nMax) / 2,
                    "validator": validators[nInitFieldName]
                };
                c.addTableWidget(field, row);

                /* n_min_size must be less than current nMax */
                var nMinFieldName = $.sprintf("sys_voip_codecs_%s_%s_n_min_size", scope, codec);
                validators[nMinFieldName] = {"min": 5, "max": nMax};
                field = {
                    "type": "text",
                    "name": nMinFieldName,
                    "defaultValue": nMin,
                    "validator": validators[nMinFieldName]
                };
                c.addTableWidget(field, row);

                /* n_max_size */
                var nMaxFieldName = $.sprintf("sys_voip_codecs_%s_%s_n_max_size", scope, codec);
                validators[nMaxFieldName] = {"min": 5, "max": nMax};
                field = {
                    "type": "text",
                    "name": nMaxFieldName,
                    "defaultValue": nMax,
                    "validator": validators[nMaxFieldName],
                    "onChange": function() {
                        onNMaxChange(scope, codec);
                    }
                };
                c.addTableWidget(field, row);
                
                /* set correct maximum value for nMin to current value of nMax */
                validators[nMinFieldName]['max'] = $("#" + nMaxFieldName).val();
                
                onJbTypeChange(scope, codec);
            });
        };

        var colNum = 10;
        c.addTitle(scopeTitle, {"colspan": colNum});
        c.addTableHeader("Codec|Pkt.time|Payload|Bitpack|JB type|LAT|nScaling|nInit|nMin|nMax");
        c.addTableTfootStr("JB Type: jitter buffer type.", colNum);
        c.addTableTfootStr("LAT: Local Adaptation Type:", colNum);
        c.addTableTfootStr(" - SI: on wtih sample interpollation.", colNum);
        c.addTableTfootStr("nScaling [1;16]: average play out delay is equal to the estimated jitter times the scaling factor.", colNum);
        c.addTableTfootStr("nInit [5;] (for Adaptive mode nMin <= nInit <= nMax): initial size of the jitter buffer in ms.", colNum);
        c.addTableTfootStr("nMin [5;]: minimum size of the jitter buffer in ms.", colNum);
        c.addTableTfootStr("nMax [5;]: maximum size of the jitter buffer in ms.", colNum);
        c.addTableTfootStr("FAX settings: aLaw, JB fixed, Pkt.time 120 ms, WLEC NE, NLP off.", colNum);

        addCodecsWidgets(scope);
        
        c.addSubmit();
    };

    page.addTab({
        "id": "codecs_prt",
        "name": "Priority settings",
        "func": function() {
            var c = page.addContainer("codecs_prt");
            c.setSubsystem("svd-codecs");
            
            var colNum = 3;
            c.addTitle("Priority settings", {"colspan": colNum});

            /* array with codecs */
            var codecs = ["not in use"];
            $.each(globalParameters.codecsParameters, function(codec, value) {
                codecs.push(codec);
            });
            
            c.addTableHeader("Priority|Internal|External");
            
            $.each(codecs, function(i, codec) {
                var field;
                var row = c.addTableRow();
                
                /* codec name */
                field = {
                    "type": "html",
                    "name": $.sprintf("prority_%s", i),
                    "str": $.sprintf("Priority %s", i)
                };
                c.addTableWidget(field, row);

                var field = {
                    "type": "select",
                    "name": $.sprintf("sys_voip_codecs_prt_int_%s", i),
                    "options": codecs,
                    "cssClass": "codec_int",
                    "onChange": function() {
                        onCodecChange("int", i);
                    }
                };
                c.addTableWidget(field, row);
                
                var field = {
                    "type": "select",
                    "name": $.sprintf("sys_voip_codecs_prt_ext_%s", i),
                    "options": codecs,
                    "cssClass": "codec_ext",
                    "onChange": function() {
                        onCodecChange("ext", i);
                    }
                };
                c.addTableWidget(field, row);
            });
            
            /* when codec for a priority is changing, check that it is unique */
            var onCodecChange = function(scope, prt) {
                var newVal = $($.sprintf("#sys_voip_codecs_prt_%s_%s", scope, prt)).val();
                
                if (newVal != "not in use") {
                    $(".codec_" + scope).not($.sprintf("#sys_voip_codecs_prt_%s_%s", scope, prt)).each(function(num, element) {
                        if ($(element).val() == newVal) {
                            $(element).val("not in use");
                        }
                    });
                }
            };
            
            c.addSubmit();
        }
    });

    page.addTab({
        "id": "codecs_int",
        "name": "Internal",
        "func": function() {
            codecsTab("codecs_int", "int", "Internal settings");
        }
    });
    
    page.addTab({
        "id": "codecs_ext",
        "name": "External",
        "func": function() {
            codecsTab("codecs_ext", "ext", "External settings");
        }
    });

    page.generateTabs();
};

