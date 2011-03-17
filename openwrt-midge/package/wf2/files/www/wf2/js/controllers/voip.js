/*
* Parameters for codecs:
* - pkt_sz — default value for packetization time;
* - payload — default value for payload;
* - bitpack — default value for bitpack;
* - bitpacks — available values for bitpack;
* - pkt_sz_vals — available values for packetization time;
* - nMax — default value and maximum value for n_max_size depending on pkt_sz value;
* - nInitFixed — if exists, sets default and maximum value for n_init_size when JB type is fixed;
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

/* for all codecs n_min_size in 10 */
globalParameters.codecsNMin = 10;

/* default value for JB */
globalParameters.codecsJbDefault = "fixed";

globalParameters.codecsNInitDefault = {"fixed": 120, "adaptive": 60};
globalParameters.codecsNMaxFixedDefault = 200;

globalParameters.codecsLatValues = {"fixed": "off", "adaptive": "off on SI"};

/* on JB type changing */
globalParameters.codecsOnJbTypeChange = function(options) {
    var params, attrPrefix;

    if (options.type == "Codecs") {
        params     = globalParameters.codecsParameters[options.codec];
        attrPrefix = $.sprintf("sys_voip_codecs_%s_%s", options.scope, options.codec);
    } else {
        /* current selected codec */
        var codec  = $($.sprintf("#sys_voip_vf_channels_%s_codec", options.channel)).val();
        params     = globalParameters.codecsParameters[codec];
        attrPrefix = $.sprintf("sys_voip_vf_channels_%s", options.channel);
    }

    var jbType     = $($.sprintf("#%s_jb_type", attrPrefix)).val();
    var pkt_sz     = $($.sprintf("#%s_pkt_sz", attrPrefix)).val();

    /* set value for nInit */
    $($.sprintf("#%s_n_init_size", attrPrefix)).val(globalParameters.codecsNInitDefault[jbType]);

    /* set values for LAT */
    $($.sprintf("#%s_lat", attrPrefix)).setOptionsForSelect({
        "options": globalParameters.codecsLatValues[jbType],
        "defaultValue": "off"
    });

    /* nMax for current codec and pkt_sz */
    var nMax = params.nMax['all'] ? params.nMax['all'] : params.nMax[pkt_sz];

    /* set value for nMax */
    $($.sprintf("#%s_n_max_size", attrPrefix)).val(
            jbType == "fixed" ? globalParameters.codecsNMaxFixedDefault : nMax);

    globalParameters.codecsUpdateWidgets(options);
};

/* on pkt_sz value changing */
globalParameters.codecsOnPktSzChange = function(options) {
    var params, attrPrefix;

    if (options.type == "Codecs") {
        params     = globalParameters.codecsParameters[options.codec];
        attrPrefix = $.sprintf("sys_voip_codecs_%s_%s", options.scope, options.codec);
    } else {
        /* current selected codec */
        var codec  = $($.sprintf("#sys_voip_vf_channels_%s_codec", options.channel)).val();
        params     = globalParameters.codecsParameters[codec];
        attrPrefix = $.sprintf("sys_voip_vf_channels_%s", options.channel);
    }

    var nMinFieldName  = $.sprintf("%s_n_min_size", attrPrefix);
    var nMaxFieldName  = $.sprintf("%s_n_max_size", attrPrefix);
    var pkt_sz         = $($.sprintf("#%s_pkt_sz", attrPrefix)).val();
    var jbType         = $($.sprintf("#%s_jb_type", attrPrefix)).val();

    /* change max value for nMin and nMax */
    globalParameters.codecsValidators[nMinFieldName]['max']
            = globalParameters.codecsValidators[nMaxFieldName]['max']
            = params.nMax['all'] ? params.nMax['all'] : params.nMax[pkt_sz];

    /* change nMax default value */
    if (jbType == "adaptive") {
        $("#" + nMaxFieldName).val(globalParameters.codecsValidators[nMaxFieldName]['max']);
    }
};

/* on nMax value changing */
globalParameters.codecsOnNMaxChange = function(options) {
    var attrPrefix;

    if (options.type == "Codecs") {
        attrPrefix = $.sprintf("sys_voip_codecs_%s_%s", options.scope, options.codec);
    } else {
        attrPrefix = $.sprintf("sys_voip_vf_channels_%s", options.channel);
    }

    var nMinFieldName  = $.sprintf("%s_n_min_size", attrPrefix);
    var nMaxFieldName  = $.sprintf("%s_n_max_size", attrPrefix);

    /* set correct maximum value for nMin to current value of nMax */
    globalParameters.codecsValidators[nMinFieldName]['max'] = $("#" + nMaxFieldName).val();
};

/* update widgets attributes and validators */
globalParameters.codecsUpdateWidgets = function(options) {
    var mutableAttrs = ["n_min_size", "n_max_size"];
    var attrPrefix, params;

    if (options.type == "Codecs") {
        params     = globalParameters.codecsParameters[options.codec];
        attrPrefix = $.sprintf("sys_voip_codecs_%s_%s", options.scope, options.codec);
    } else {
        /* current selected codec */
        var codec  = $($.sprintf("#sys_voip_vf_channels_%s_codec", options.channel)).val();
        params     = globalParameters.codecsParameters[codec];
        attrPrefix = $.sprintf("sys_voip_vf_channels_%s", options.channel);
    }

    var jbType         = $($.sprintf("#%s_jb_type", attrPrefix)).val();
    var pkt_sz         = $($.sprintf("#%s_pkt_sz", attrPrefix)).val();
    var nInitFieldName = $.sprintf("%s_n_init_size", attrPrefix);

    if (jbType == "fixed") {
        /* remove dynamicRange validator from nInit */
        delete globalParameters.codecsValidators[nInitFieldName]['dynamicRange'];

        /* set min and max validators to nInit */
        globalParameters.codecsValidators[nInitFieldName]['min'] = globalParameters.codecsNMin;
        globalParameters.codecsValidators[nInitFieldName]['max']
                = params.nInitFixed ? params.nInitFixed
                                    : params.nMax['all'] ? params.nMax['all'] : params.nMax[pkt_sz];

        /* set nMin and nMax read only */
        $(mutableAttrs).each(function(num, attr) {
            $($.sprintf("#%s_%s", attrPrefix, attr)).attr("readonly", true);
        });
    } else {
        /* remove min and max validators from nInit */
        delete globalParameters.codecsValidators[nInitFieldName]['min'];
        delete globalParameters.codecsValidators[nInitFieldName]['max'];

        /* set dynamicRange validator to nInit */
        globalParameters.codecsValidators[nInitFieldName]['dynamicRange'] = [
                $.sprintf("#%s_n_min_size", attrPrefix),
                $.sprintf("#%s_n_max_size", attrPrefix)
        ];

        /* set nMin and nMax writable */
        $(mutableAttrs).each(function(num, attr) {
            $($.sprintf("#%s_%s", attrPrefix, attr)).removeAttr("readonly");
        });
    }
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
            c.setHelpPage("voip.settings");
            c.addTitle("VoIP settings");

            /* General settings */
            c.addTitle("General settings", {"internal": true});

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
            c.addTitle("SIP settings", {"internal": true});

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
            c.setHelpPage("voip.audio");
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
            c.setHelpPage("voip.echo");
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
            c.setHelpPage("voip.dialmode");
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

    /* function for generating page with codecs for specified scope */
    var codecsTab = function(container, scope, scopeTitle) {
        var c = page.addContainer(container);
        c.setSubsystem("svd-codecs");

        /* hash with fields validators, which can change dynamically */
        globalParameters.codecsValidators = {};

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
                    "name": $.sprintf("sys_voip_codecs_%s_%s_pkt_sz", scope, codec),
                    "options": params.pkt_sz_vals,
                    "defaultValue": params.pkt_sz,
                    "onChange": function() {
                        globalParameters.codecsOnPktSzChange({"type": "Codecs", "scope": scope, "codec": codec});
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
                    "defaultValue": globalParameters.codecsJbDefault,
                    "onChange": function() {
                        globalParameters.codecsOnJbTypeChange({"type": "Codecs", "scope": scope, "codec": codec});
                    }
                };
                c.addTableWidget(field, row);

                /* current jb_type */
                var jbType = $($.sprintf("#sys_voip_codecs_%s_%s_jb_type", scope, codec)).val();

                /* lat */
                field = {
                    "type": "select",
                    "name": $.sprintf("sys_voip_codecs_%s_%s_lat", scope, codec),
                    "options": globalParameters.codecsLatValues[jbType],
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
                var pkt_sz = $($.sprintf("#sys_voip_codecs_%s_%s_pkt_sz", scope, codec)).val();

                /* nMax for current codec and pkt_sz */
                var nMax = params.nMax['all'] ? params.nMax['all'] : params.nMax[pkt_sz];

                /* n_init_size. validator is depending on JB type and is setting later */
                var nInitFieldName = $.sprintf("sys_voip_codecs_%s_%s_n_init_size", scope, codec);
                globalParameters.codecsValidators[nInitFieldName] = {};
                field = {
                    "type": "text",
                    "name": nInitFieldName,
                    "defaultValue": globalParameters.codecsNInitDefault[globalParameters.codecsJbDefault],
                    "validator": globalParameters.codecsValidators[nInitFieldName]
                };
                c.addTableWidget(field, row);

                /* n_min_size must be less than current nMax */
                var nMinFieldName = $.sprintf("sys_voip_codecs_%s_%s_n_min_size", scope, codec);
                globalParameters.codecsValidators[nMinFieldName] = {"min": globalParameters.codecsNMin, "max": nMax};
                field = {
                    "type": "text",
                    "name": nMinFieldName,
                    "defaultValue": globalParameters.codecsNMin,
                    "validator": globalParameters.codecsValidators[nMinFieldName]
                };
                c.addTableWidget(field, row);

                /* n_max_size */
                var nMaxFieldName = $.sprintf("sys_voip_codecs_%s_%s_n_max_size", scope, codec);
                globalParameters.codecsValidators[nMaxFieldName] = {"min": globalParameters.codecsNMin, "max": nMax};
                field = {
                    "type": "text",
                    "name": nMaxFieldName,
                    "defaultValue": jbType == "fixed" ? globalParameters.codecsNMaxFixedDefault : nMax,
                    "validator": globalParameters.codecsValidators[nMaxFieldName],
                    "onChange": function() {
                        globalParameters.codecsOnNMaxChange({"type": "Codecs", "scope": scope, "codec": codec});
                    }
                };
                c.addTableWidget(field, row);

                /* set correct maximum value for nMin to current value of nMax */
                globalParameters.codecsValidators[nMinFieldName]['max'] = $("#" + nMaxFieldName).val();

                /* update widgets according to current JB type */
                globalParameters.codecsUpdateWidgets({"type": "Codecs", "scope": scope, "codec": codec});
            });
        };

        var colNum = 10;
        c.setHelpPage("voip.codecs");
        c.setHelpSection("voip.codecs.settings");
        c.addTitle(scopeTitle, {"colspan": colNum});
        c.addTableHeader("Codec|Pkt.time|Payload|Bitpack|JB type|LAT|nScaling|nInit|nMin|nMax");
        c.addTableTfootStr("JB Type: jitter buffer type.", colNum);
        c.addTableTfootStr("LAT: Local Adaptation Type:", colNum);
        c.addTableTfootStr(" - SI: on with sample interpollation.", colNum);
        c.addTableTfootStr("nScaling: average play out delay is equal to the estimated jitter times the scaling factor. Values: [1;16]", colNum);
        c.addTableTfootStr("nInit: initial size of the jitter buffer in ms. Values: JB Adaptive [nMin;nMax], JB Fixed: for aLaw [10; 150], for others [10; nMax]", colNum);
        c.addTableTfootStr("nMin: minimum size of the jitter buffer in ms. Values: [10; nMax]", colNum);
        c.addTableTfootStr("nMax: maximum size of the jitter buffer in ms. Values are depended on Codec and Pkt.time:", colNum);
        $.each(globalParameters.codecsParameters, function(codec, params) {
            var values = "";
            $.each(params.nMax, function(pkt_sz, value) {
                if (values.length > 0) {
                    values += ", ";
                }

                values += pkt_sz + ": " + value;
            });
            c.addTableTfootStr(" - " + codec + ": " + values, colNum);
        });

        c.addTableTfootStr("FAX settings: aLaw, JB fixed, Pkt.time 120 ms, WLEC NE, NLP off.", colNum);

        addCodecsWidgets(scope);

        c.addSubmit();
    };

    /* codecs priority */
    page.addTab({
        "id": "codecs_prt",
        "name": "Priority settings",
        "func": function() {
            var c = page.addContainer("codecs_prt");
            c.setSubsystem("svd-codecs");
            c.setHelpPage("voip.codecs");

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

    /* codecs internal settings */
    page.addTab({
        "id": "codecs_int",
        "name": "Internal",
        "func": function() {
            codecsTab("codecs_int", "int", "Internal settings");
        }
    });

    /* codecs external settings */
    page.addTab({
        "id": "codecs_ext",
        "name": "External",
        "func": function() {
            codecsTab("codecs_ext", "ext", "External settings");
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
            c.setHelpPage("voip.vf");

            /* hash with fields validators, which can change dynamically */
            globalParameters.codecsValidators = {};

            /* on codec value changing */
            var onCodecChange = function(channel) {
                var codec = $($.sprintf("#sys_voip_vf_channels_%s_codec", channel)).val();

                /* set pkt_sz values */
                $($.sprintf("#sys_voip_vf_channels_%s_pkt_sz", channel)).setOptionsForSelect({
                        "options": globalParameters.codecsParameters[codec].pkt_sz_vals
                });

                /* set pkt_sz default */
                $($.sprintf("#sys_voip_vf_channels_%s_pkt_sz", channel))
                        .val(globalParameters.codecsParameters[codec].pkt_sz);

                /* set payload default */
                $($.sprintf("#sys_voip_vf_channels_%s_payload", channel))
                        .val(globalParameters.codecsParameters[codec].payload);

                /* set bitpack values */
                $($.sprintf("#sys_voip_vf_channels_%s_bitpack", channel)).setOptionsForSelect({
                        "options": globalParameters.codecsParameters[codec].bitpacks
                });

                /* set bitpack default */
                $($.sprintf("#sys_voip_vf_channels_%s_bitpack", channel))
                        .val(globalParameters.codecsParameters[codec].bitpack);

                /* update widgets according to current JB type */
                globalParameters.codecsOnJbTypeChange({"type": "VF", "channel": channel});
            };

            var colNum = 15;
            c.addTitle("Voice frequency channels", {"colspan": colNum});

            var EN_button = $.create('input', {'type': 'button', 'className': 'buttonWidget', 'value': 'EN', 'id' : 'EN_button'});
            setTimeout(function() {
                document.getElementById('EN_button').onclick = function() {
                    for (i = 0; i <= 31; i++) {
                        if (document.getElementById($.sprintf("channels_%s_enabled", i > 9 ? i : "0"+i))) {
                            if (document.getElementById($.sprintf("channels_%s_enabled", i > 9 ? i : "0"+i)).checked)
                                document.getElementById($.sprintf("channels_%s_enabled", i > 9 ? i : "0"+i)).checked = false;
                            else
                                document.getElementById($.sprintf("channels_%s_enabled", i > 9 ? i : "0"+i)).checked = true;
                        }
                    }
                }
            }, 1000);
            var ID_button = $.create('input', {'type': 'button', 'className': 'buttonWidget', 'value': 'R.ID', 'id' : 'ID_button'});
            setTimeout(function() {
                document.getElementById('ID_button').onclick = function() {
                    var value = "";
                    for (i = 0; i <= 31; i++) {
                        if (document.getElementById($.sprintf("channels_%s_pair_route", i > 9 ? i : "0"+i)) && value == "") {
                            value = document.getElementById($.sprintf("channels_%s_pair_route", i > 9 ? i : "0"+i)).value;
                            continue;
                        }
                        if (document.getElementById($.sprintf("channels_%s_pair_route", i > 9 ? i : "0"+i)) && value != "")
                            document.getElementById($.sprintf("channels_%s_pair_route", i > 9 ? i : "0"+i)).value = value;
                    }
                }
            }, 1000);
            var chan_button = $.create('input', {'type': 'button', 'className': 'buttonWidget', 'value': 'Chan', 'id' : 'chan_button'});
            setTimeout(function() {
                document.getElementById('chan_button').onclick = function() {
                    var value = "";
                    for (i = 0; i <= 31; i++) {
                        if (document.getElementById($.sprintf("channels_%s_pair_chan", i > 9 ? i : "0"+i)) && value == "")
                            document.getElementById($.sprintf("channels_%s_pair_chan", i > 9 ? i : "0"+i)).value = i > 9 ? i : "0"+i;
                    }
                }
            }, 1000);
            var all_button = $.create('input', {'type': 'button', 'className': 'buttonWidget', 'value': 'Ping', 'id' : 'ping_all'});
            setTimeout(function() {
                document.getElementById('ping_all').onclick = function() {
                    for (i = 0; i <= 31; i++) {
                        if (document.getElementById($.sprintf("channels_%s_ping", i > 9 ? i : "0"+i))) {
                            if (document.getElementById($.sprintf("channels_%s_ping", i > 9 ? i : "0"+i)).checked)
                                document.getElementById($.sprintf("channels_%s_ping", i > 9 ? i : "0"+i)).checked = false;
                            else
                                document.getElementById($.sprintf("channels_%s_ping", i > 9 ? i : "0"+i)).checked = true;
                        }
                    }
                }
            }, 1000);

            $("thead", c.table).append(
                $.create("tr", {},
                   [$.create("th", {"colSpan": 1, 'align' : 'center'}, " "),
                   $.create("th", {"colSpan": 1, 'align' : 'center'}, EN_button),
                   $.create("th", {"colSpan": 1, 'align' : 'center'}, ID_button),
                   $.create("th", {"colSpan": 1, 'align' : 'center'}, chan_button),
                   $.create("th", {"colSpan": 10, 'align' : 'center'}, " "),
                   $.create("th", {"colSpan": 1, 'align' : 'center'}, all_button)]
               )
            );

            c.addTableHeader("#|EN|R.ID|Chan|Codec|P.time|Pay-d|Bitpack|JB type|LAT|nScal|nInit|nMin|nMax|Ping");
            c.addTableTfootStr("Chan - local channel.", colNum);
            c.addTableTfootStr("EN - enable channel.", colNum);
            c.addTableTfootStr("R.ID - ID of a router to connect with.", colNum);
            c.addTableTfootStr("Chan - VF channel on the remote router.", colNum);
            c.addTableTfootStr("Codec - codec to use.", colNum);
            c.addTableTfootStr("P.time - packetization time in ms.", colNum);
            c.addTableTfootStr("Pay-d - Payload, RTP codec identificator.", colNum);
            c.addTableTfootStr("Bitpack - bits packetization type.", colNum);
            c.addTableTfootStr("JB Type: jitter buffer type.", colNum);
            c.addTableTfootStr("LAT: Local Adaptation Type:", colNum);
            c.addTableTfootStr(" - SI: on wtih sample interpollation.", colNum);
            c.addTableTfootStr("nScal: average play out delay is equal to the estimated jitter times the scaling factor. Values: [1;16]", colNum);
            c.addTableTfootStr("nInit: initial size of the jitter buffer in ms. Values: JB Adaptive [nMin;nMax], JB Fixed: for aLaw [10; 150], for others [10; nMax]", colNum);
            c.addTableTfootStr("nMin: minimum size of the jitter buffer in ms. Values: [10; nMax]", colNum);
            c.addTableTfootStr("nMax: maximum size of the jitter buffer in ms. Values are depended on Codec and Pkt.time:", colNum);
            $.each(globalParameters.codecsParameters, function(codec, params) {
                var values = "";
                $.each(params.nMax, function(pkt_sz, value) {
                    if (values.length > 0) {
                        values += ", ";
                    }

                    values += pkt_sz + ": " + value;
                });
                c.addTableTfootStr(" - " + codec + ": " + values, colNum);
            });

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
                    "name": $.sprintf("sys_voip_vf_channels_%s_enabled", channel[0]),
                    "id"  : $.sprintf("channels_%s_enabled", channel[0])
                };
                c.addTableWidget(field, row);

                /* pair_route */
                field = {
                    "type": "text",
                    "name": $.sprintf("sys_voip_vf_channels_%s_pair_route", channel[0]),
                    "id"  : $.sprintf("channels_%s_pair_route", channel[0]),
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
                    "id"  : $.sprintf("channels_%s_pair_chan", channel[0]),
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
                        $.each(globalParameters.codecsParameters, function(codec, value) {
                            codecs.push(codec);
                        });
                        return codecs;
                    }(),
                    "onChange": function() {
                        onCodecChange(channel[0]);
                    }
                };
                c.addTableWidget(field, row);

                /* current selected codec */
                var codec = $($.sprintf("#sys_voip_vf_channels_%s_codec", channel[0])).val();

                /* pkt_sz */
                field = {
                    "type": "select",
                    "name": $.sprintf("sys_voip_vf_channels_%s_pkt_sz", channel[0]),
                    "options": globalParameters.codecsParameters[codec].pkt_sz_vals,
                    "defaultValue": globalParameters.codecsParameters[codec].pkt_sz,
                    "onChange": function() {
                        globalParameters.codecsOnPktSzChange({"type": "VF", "channel": channel[0]});
                    }
                };
                c.addTableWidget(field, row);

                /* payload */
                field = {
                    "type": "text",
                    "name": $.sprintf("sys_voip_vf_channels_%s_payload", channel[0]),
                    "defaultValue": globalParameters.codecsParameters[codec].payload,
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
                    "options": globalParameters.codecsParameters[codec].bitpacks,
                    "defaultValue": globalParameters.codecsParameters[codec].bitpack
                };
                c.addTableWidget(field, row);

                /* jb_type */
                field = {
                    "type": "select",
                    "name": $.sprintf("sys_voip_vf_channels_%s_jb_type", channel[0]),
                    "options": {"fixed": "Fixed", "adaptive": "Adapt."},
                    "defaultValue": globalParameters.codecsJbDefault,
                    "onChange": function() {
                        globalParameters.codecsOnJbTypeChange({"type": "VF", "channel": channel[0]});
                    }
                };
                c.addTableWidget(field, row);

                /* current jb_type */
                var jbType = $($.sprintf("#sys_voip_vf_channels_%s_jb_type", channel[0])).val();

                /* lat */
                field = {
                    "type": "select",
                    "name": $.sprintf("sys_voip_vf_channels_%s_lat", channel[0]),
                    "options": globalParameters.codecsLatValues[jbType],
                    "defaultValue": "off"
                };
                c.addTableWidget(field, row);

                /* n_scaling */
                field = {
                    "type": "text",
                    "name": $.sprintf("sys_voip_vf_channels_%s_n_scaling", channel[0]),
                    "defaultValue": "1.4",
                    "validator": {"min": 1, "max": 16}
                };
                c.addTableWidget(field, row);

                /* current pkt_sz */
                var pkt_sz = $($.sprintf("#sys_voip_vf_channels_%s_pkt_sz", channel[0])).val();

                /* nMax for current codec and pkt_sz */
                var nMax = globalParameters.codecsParameters[codec].nMax['all']
                         ? globalParameters.codecsParameters[codec].nMax['all']
                         : globalParameters.codecsParameters[codec].nMax[pkt_sz];

                /* n_init_size. validator is depending on JB type and is setting later */
                var nInitFieldName = $.sprintf("sys_voip_vf_channels_%s_n_init_size", channel[0]);
                globalParameters.codecsValidators[nInitFieldName] = {};
                field = {
                    "type": "text",
                    "name": nInitFieldName,
                    "defaultValue": globalParameters.codecsNInitDefault[globalParameters.codecsJbDefault],
                    "validator": globalParameters.codecsValidators[nInitFieldName]
                };
                c.addTableWidget(field, row);

                /* n_min_size must be less than current nMax */
                var nMinFieldName = $.sprintf("sys_voip_vf_channels_%s_n_min_size", channel[0]);
                globalParameters.codecsValidators[nMinFieldName] = {"min": globalParameters.codecsNMin, "max": nMax};
                field = {
                    "type": "text",
                    "name": nMinFieldName,
                    "defaultValue": globalParameters.codecsNMin,
                    "validator": globalParameters.codecsValidators[nMinFieldName]
                };
                c.addTableWidget(field, row);

                /* n_max_size */
                var nMaxFieldName = $.sprintf("sys_voip_vf_channels_%s_n_max_size", channel[0]);
                globalParameters.codecsValidators[nMaxFieldName] = {"min": globalParameters.codecsNMin, "max": nMax};
                field = {
                    "type": "text",
                    "name": nMaxFieldName,
                    "defaultValue": jbType == "fixed" ? globalParameters.codecsNMaxFixedDefault : nMax,
                    "validator": globalParameters.codecsValidators[nMaxFieldName],
                    "onChange": function() {
                        globalParameters.codecsOnNMaxChange({"type": "VF", "channel": channel[0]});
                    }
                };
                c.addTableWidget(field, row);

                /* set correct maximum value for nMin to current value of nMax */
                globalParameters.codecsValidators[nMinFieldName]['max'] = $("#" + nMaxFieldName).val();

                /* update widgets according to current JB type */
                globalParameters.codecsUpdateWidgets({"type": "VF", "channel": channel[0]});

                field = {
                    "type": "checkbox",
                    "name": $.sprintf("sys_voip_vf_channels_%s_ping", channel[0]),
                    "id"  : $.sprintf("channels_%s_ping", channel[0])
                };
                c.addTableWidget(field, row);

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
            c.setHelpPage("voip.vf");
            c.setHelpSection("voip.vf.settings");
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

