// <Vlad>
// Filters
tmt_globalFilters.nobackquotes = tmt_filterInfo('`', "");
tmt_globalFilters.nomagic = tmt_filterInfo('[`$();]', "");


// IP Addr validation
tmt_globalPatterns.ipaddr = new RegExp("^\(0?0?\\d|[01]?\\d\\d|2[0-4]\\d|25[0-5])\\.\(0?0?\\d|[01]?\\d\\d|2[0-4]\\d|25[0-5])\\.\(0?0?\\d|[01]?\\d\\d|2[0-4]\\d|25[0-5])\\.\(0?0?\\d|[01]?\\d\\d|2[0-4]\\d|25[0-5])$");
// Netmask validation
tmt_globalPatterns.netmask = new RegExp("^\(0?0?\\d|[01]?\\d\\d|2[0-4]\\d|25[0-5])\\.\(0?0?\\d|[01]?\\d\\d|2[0-4]\\d|25[0-5])\\.\(0?0?\\d|[01]?\\d\\d|2[0-4]\\d|25[0-5])\\.\(0?0?\\d|[01]?\\d\\d|2[0-4]\\d|25[0-5])$");
// MAC Address validation
tmt_globalPatterns.macaddr = new RegExp("^\([0-9a-fA-F][0-9a-fA-F]:\){5}\([0-9a-fA-F][0-9a-fA-F]\)$");
// IP net validation (ip/bitmask)
tmt_globalPatterns.ipnet  = new RegExp("^(0?0?\\d|[01]?\\d\\d|2[0-4]\\d|25[0-5])\\.(0?0?\\d|[01]?\\d\\d|2[0-4]\\d|25[0-5])\\.(0?0?\\d|[01]?\\d\\d|2[0-4]\\d|25[0-5])\\.(0?0?\\d|[01]?\\d\\d|2[0-4]\\d|25[0-5])(/\\d\\d*)?$");
tmt_globalPatterns.ipaddrport = new RegExp("^(0?0?\\d|[01]?\\d\\d|2[0-4]\\d|25[0-5])\\.(0?0?\\d|[01]?\\d\\d|2[0-4]\\d|25[0-5])\\.(0?0?\\d|[01]?\\d\\d|2[0-4]\\d|25[0-5])\\.(0?0?\\d|[01]?\\d\\d|2[0-4]\\d|25[0-5])(\\:\\d*)?$");
tmt_globalPatterns.ipportrange = new RegExp("^((\\d)+(:)?((\\d)+)?|(:)((\\d)+)|any)$");


// DNS Zone validation
tmt_globalPatterns.dnszone = new RegExp("^\([a-zA-Z0-9\-\.]\)+$");
// DNS Host validation
tmt_globalPatterns.dnsdomain = new RegExp("^\(\([a-zA-Z0-9\-\.]\)+|@\)$");

tmt_globalPatterns.dnsdomainoripaddr = new RegExp("^\(\([a-zA-Z0-9\-\.]\)+|\(\(0?0?\\d|[01]?\\d\\d|2[0-4]\\d|25[0-5])\\.\(0?0?\\d|[01]?\\d\\d|2[0-4]\\d|25[0-5])\\.\(0?0?\\d|[01]?\\d\\d|2[0-4]\\d|25[0-5])\\.\(0?0?\\d|[01]?\\d\\d|2[0-4]\\d|25[0-5]))\)$");


tmt_globalPatterns.qoslatency = new RegExp("^\([0-9]+ms\)$");
tmt_globalPatterns.qosbandw = new RegExp("^\([0-9]+\(k|M\)\(bit|bps\)\)$");
// </Vlad>
