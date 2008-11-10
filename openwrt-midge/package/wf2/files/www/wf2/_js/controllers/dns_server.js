Controllers['dns_server'] = function() {
	var page = this.Page();
	page.setSubsystem("dns_server");
	page.setHelpPage("dns_server");
	
	/* settings tab */
	page.addTab({
		"id": "settings",
		"name": "Settings",
		"func": function() {
			var c, field;
			c = page.addContainer("settings");
			c.addTitle("DNS settings");
		
			field = { 
				"type": "checkbox",
				"name": "svc_dns_enabled",
				"text": "Enable DNS server",
				"descr": "Check this item if you want to use DNS server on your router"
			};
			c.addWidget(field);
			
			field = { 
				"type": "text",
				"name": "svc_dns_forwarder1",
				"text": "Forwarder DNS 1",
				"descr": "Forward queries to DNS server",
				"validator": {"ipAddr": true}
			};
			c.addWidget(field);
			
			field = { 
				"type": "text",
				"name": "svc_dns_forwarder2",
				"text": "Forwarder DNS 2",
				"descr": "Forward queries to DNS server",
				"validator": {"ipAddr": true}
			};
			c.addWidget(field);
			
			c.addSubmit();
		}
	});
	
	/*
	 * Event handler.
	 * When adding or editing zone's record: when selected record type MX, add prio widget.
	 * 
	 * list — current list to add new widget to (passed automatically by framework).
	 */
	var onChangeRecordType = function(list) {
		if ($("#datatype").val() == "MX" && $("#prio").length == 0) {
			/* add new field */
			var field = { 
				"type": "text",
				"name": "prio",
				"id": "prio",
				"text": "Priority",
				"descr": "Priority for MX record.",
				"validator": {"required": true, "min": 1, "max": 999}
			};
			list.addDynamicWidget(field, $("#data").parents("tr"));
		/* remove field */
		} else {
			$("#prio").parents("tr").remove();
		}
	};
	
	/*
	 * Page for manipulation with zone's records:
	 *  - show list of records;
	 *  - provides adding/editing/deleting capabilities.
	 * 
	 * c — container where to show page;
	 * zoneid — ID of zone.
	 */
	var zoneRecordsPage = function(c, zoneid) {
		var field;
		
		/* clear this container */
		c.initContainer({"clear": true});
		
		/* create list of zone's records */
		var list = c.createList({
			"tabId": "zones",
			"header": ["Domain", "Type", "Data", "Priority"],
			"varList": ["domain", "datatype", "data", "prio"],
			"listItem": $.sprintf("svc_dns_zone_%s_", zoneid),
			"showPage": function() {
				zoneRecordsPage(c, zoneid);
			},
			"onAddOrEditItemRender": onChangeRecordType,
			"addMessage": $.sprintf("Add record to %s zone", zoneid),
			"editMessage": $.sprintf("Edit record in %s zone", zoneid),
			"listTitle": $.sprintf("Records for %s zone", zoneid),
			"helpPage": "dns_server",
			"helpSection": "dns_server.zone_record_add"
		});
		
		field = { 
			"type": "select",
			"name": "datatype",
			"id": "datatype",
			"text": "Type of record",
			"descr": "Select type of record.",
			"options": "A CNAME MX NS PTR TXT",
			"onChange": onChangeRecordType
		};
		list.addWidget(field);
		
		field = { 
			"type": "text",
			"name": "domain",
			"text": "Domain or host",
			"descr": "Enter domain name or host name.",
			"validator": {"required": true, "dnsRecordDomainOrIpAddr": true},
			"tip": "Use @ for current zone."
		};
		list.addWidget(field);
		
		field = { 
			"type": "text",
			"name": "data",
			"id": "data",
			"text": "Data",
			"descr": "Data of the record.",
			"validator": {"required": true, "domainNameOrIpAddr": true},
			"tip": "If the record points to an EXTERNAL server (not defined in this zone) it MUST " +
				"end with a <i>.</i> (dot), e.g. ns1.example.net. If the name server is defined in " +
				"this domain (in this zone file) it can be written as ns1 (without the dot)."
		};
		list.addWidget(field);

		list.generateList();
		
		/* create button for returning back to the list of zones */
		field = {
			"type": "button",
			"text": "Back to list of zones",
			"func": function() {
				$("#tab_zones_link").click();
			}
		};
		c.addSubWidget(field);
	};
	
	/* zones tab */
	page.addTab({
		"id": "zones",
		"name": "Zones",
		"func": function() {
			var c, field;
			c = page.addContainer("zones");
			c.setHelpSection("dns_server.dns_zones");
		
			/* create list of zones */
			var list = c.createList({
				"tabId": "zones",
				"header": ["ID", "Name", "Admin", "Serial"],
				"varList": ["zoneid", "zone", "admin", "serial"],
				"varFunctions": {
					/* on user click on cell with zone ID show page with records for that zone */
					"zoneid": {
						"func": function(zoneid) {
							zoneRecordsPage(c, zoneid);
						},
						"tip": "Click to edit this zone."
					}
				},
				"listItem": "svc_dns_zonelist_",
				"onEditItemRender": function() {
					/* make Zone ID readonly in editing page */
					$("#zoneid").attr("readonly", true);
				},
				"addMessage": "Add DNS zone",
				"editMessage": "Edit DNS zone",
				"listTitle": "Zones",
				"helpPage": "dns_server",
				"helpSection": "dns_server.zone_add"
			});
			
			field = { 
				"type": "text",
				"name": "zoneid",
				"id": "zoneid",
				"text": "Zone ID",
				"descr": "Identifier of zone - just a simple name",
				"validator": {"required": true, "alphanum": true},
				"tip": "E.g., <i>domain2</i>"
			};
			list.addWidget(field);
			
			field = { 
				"type": "hidden",
				"name": "zonetype",
				"defaultValue": "master"
			};
			list.addWidget(field);
			
			field = { 
				"type": "checkbox",
				"name": "enabled",
				"text": "Enable",
				"descr": "Check this item to enable zone"
			};
			list.addWidget(field);
			
			/* generate zone's serial */
			var date = new Date();
			field = { 
				"type": "text",
				"name": "serial",
				"text": "Serial",
				"descr": "Serial number of zone",
				"validator": {"required": true, "min": 1},
				"tip": "Common practice is to use as serial number date of last modification. " +
					"E.g., <i>2008110601</i> means year 2008, month 11, day 06 and day edition 01.",
				"defaultValue": "" + date.getFullYear()
					+ (date.getMonth() + 1 < 10 ? "0" + (date.getMonth() + 1) : (date.getMonth() + 1))
					+ (date.getDate() < 10 ? "0" + date.getDate() : date.getDate())
					+ "01"
			};
			list.addWidget(field);

			field = { 
				"type": "text",
				"name": "zone",
				"text": "Zone",
				"descr": "Name of zone",
				"validator": {"required": true, "domainName": true},
				"tip": "E.g., <i>domain2.org</i>"
			};
			list.addWidget(field);
			
			field = { 
				"type": "text",
				"name": "nameserver",
				"text": "Name server",
				"descr": "A name server that will respond authoritatively for the domain",
				"validator": {"required": true, "domainName": true},
				"tip": "This is most commonly written as a Fully-qualified Domain Name " +
					"(FQDN and ends with a dot). If the record points to an EXTERNAL server " +
					"(not defined in this zone) it MUST end with a . (dot) e.g. ns1.example.net."
			};
			list.addWidget(field);
			
			field = { 
				"type": "text",
				"name": "admin",
				"text": "Admin",
				"descr": "Email of zone admin",
				"validator": {"required": true, "email": true}
			};
			list.addWidget(field);
			
			field = { 
				"type": "text",
				"name": "refresh",
				"text": "Refresh",
				"descr": "Time (in seconds) when the slave will try to refresh the zone from the master.",
				"validator": {"required": true, "min": 1200, "max": 500000},
				"defaultValue": "28800",
				"tip": "Indicates the time when the slave will try to refresh the zone from the " +
					"master.<br>RFC 1912 recommends 1200 to 43200 seconds."
			};
			list.addWidget(field);
			
			field = { 
				"type": "text",
				"name": "ttl",
				"text": "TTL",
				"descr": "Time (in seconds) to live.",
				"validator": {"required": true, "min": 1, "max": 500000},
				"defaultValue": "86400",
				"tip": "TTL in the DNS context defines the duration in seconds that the record " +
					"may be cached. Zero indicates the record should not be cached."
			};
			list.addWidget(field);
			
			field = { 
				"type": "text",
				"name": "retry",
				"text": "Retry",
				"descr": "Defines the time (seconds) between retries if the slave (secondary) " +
					"fails to contact the master when refresh (above) has expired.",
				"validator": {"required": true, "min": 180, "max": 20000},
				"defaultValue": "7200",
				"tip": "Typical values should be 180 (3 minutes) to 900 (15 minutes), or higher."
			};
			list.addWidget(field);
			
			field = { 
				"type": "text",
				"name": "expire",
				"text": "Expire",
				"descr": "Indicates when (in seconds) the zone data is no longer authoritative.",
				"validator": {"required": true, "min": 10000, "max": 90000000},
				"defaultValue": "1209600",
				"tip": "Slave servers stop responding to queries for the zone when this time has " +
					"expired and no contact has been made with the master<br>RFC 1912 " +
					"recommends 1209600 to 2419200 seconds (2-4 weeks) to allow for major outages " +
					"of the master."
			};
			list.addWidget(field);
			
			list.generateList();
		}
	});
	
	page.generateTabs();
};
