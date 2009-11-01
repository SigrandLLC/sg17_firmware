Controllers.dialin = function () {
    var page = this.Page();
    page.setSubsystem("dialin");

    var add_user = function(options) {
        var field;
        field = {
            "type": "text",
            "name": "login",
            "text": "Login",
            "validator" : {"required":true, "alphanumU":true}
        };
        options.list.addWidget(field);
        field = {
            "type": "password",
            "name": "password",
            "text": "Password",
            "validator" : {"required":true, "alphanumU":true}
        };
        options.list.addWidget(field);
    };

// -------------- General tab -----------------
    page.addTab({
        "id": "general",
        "name": "General",
        "func": function() {
            var c, field;
            c = page.addContainer("general");
            c.addTitle("Options");
            field = {
                "type": "checkbox",
                "name": "sys_dialin_terminal",
                "text": "Enable terminal access"
            };
            c.addWidget(field);
            field = {
                "type": "select",
                "name": "sys_dialin_secrets",
                "text": "Authentication type",
                "options": {"pap" : "pap","chap" : "chap"}
            };
            c.addWidget(field);
            c.addSubmit();
        }
    });
// --------------- Dial-in users tab ---------------
    page.addTab({
        "id": "dialin_users",
        "name": "Dial-in users",
        "func": function() {
            var c, field;
            c = page.addContainer("dialin_users");
            list = c.createList({
                "tabId": "dialin_users",
                "header": ["Login"],//, "Password"],
                "varList": ["login"],//, "password"],
                "listItem": "sys_dialin_user_",
                "listTitle": _("Dial-in users"),
                "addMesage": _("Add new user")
            });
            add_user({"list": list});
            list.generateList();
        }
    });

// ------------- Ports tab ----------------
    page.addTab({
        "id" : "ports",
        "name" : "Ports",
        "func": function() {
            var colSpan = 7;
            var c = page.addContainer("ports");
            c.addTitle("Dial-in ports", {"colspan": colSpan});
            c.addTableHeader("Port|Enable|Speed|Rings|Init-chat|Server IP|Client IP");
            var ifaces = config.getData(config.getOEM("MR17S_DRVNAME"));
//            alert( "ifaces = " + ifaces);
            $.each(ifaces, function(num, ifaceInfo) {
                var iface = ifaceInfo.iface;
                var id, field;
                var row = c.addTableRow();
                field = {
                    "type" : "html",
                    "name" : iface,
                    "str" : iface
                };
                c.addTableWidget(field, row);
                field = {
                    "type" : "checkbox",
                    "name" : $.sprintf("sys_dialin_%s_enable", iface),
                    "tip" : "Enable Dial-in port"
                };
                c.addTableWidget(field, row);

                field = {
                    "type" : "select",
                    "name" : $.sprintf("sys_dialin_%s_speed", iface),
                    "options": [230400,115200,57600,38400,28800,19200,14400,9600,7200,4800,3600,2400,1800,1200,600,300]
                };
                c.addTableWidget(field, row);
                field = {
                    "type" : "select",
                    "name" : $.sprintf("sys_dialin_%s_rings", iface),
                    "options": "1 2 3 4 5"
                };
                c.addTableWidget(field, row);
                field = {
                    "type" : "text",
                    "name" : $.sprintf("sys_dialin_%s_initchat", iface),
//                    "defaultValue" : "    '"" ATQ0E1V1H0 OK ATL0M0S0=0 OK AT&K3 OK'    ",
                    "tip" : ""
                };
                c.addTableWidget(field, row);
                field = {
                    "type" : "text",
                    "name" : $.sprintf("sys_dialin_%s_s_ip", iface),
                    "validator" : {"ipAddr": true},
                    "defaultValue": "0.0.0.0"
                };
                c.addTableWidget(field, row);
                field = {
                    "type" : "text",
                    "name" : $.sprintf("sys_dialin_%s_c_ip", iface),
                    "validator" : {"ipAddr": true},
                    "defaultValue": "0.0.0.0"
                };
                c.addTableWidget(field, row);
            });
//            c.setSuccessMessage("Device has to be rebooted to apply changes.");
            c.addSubmit();
        }
    });
    page.generateTabs();
};
