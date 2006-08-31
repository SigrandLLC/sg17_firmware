// <Vlad>
// Filters
tmt_globalFilters.nobackquotes = tmt_filterInfo('`', "");
tmt_globalFilters.nomagic = tmt_filterInfo('[`$();]', "");


// Data validation
tmt_globalPatterns.ipaddr = new RegExp("^\(0?0?\\d|[01]?\\d\\d|2[0-4]\\d|25[0-5])\\.\(0?0?\\d|[01]?\\d\\d|2[0-4]\\d|25[0-5])\\.\(0?0?\\d|[01]?\\d\\d|2[0-4]\\d|25[0-5])\\.\(0?0?\\d|[01]?\\d\\d|2[0-4]\\d|25[0-5])$");
tmt_globalPatterns.netmask = new RegExp("^\(0?0?\\d|[01]?\\d\\d|2[0-4]\\d|25[0-5])\\.\(0?0?\\d|[01]?\\d\\d|2[0-4]\\d|25[0-5])\\.\(0?0?\\d|[01]?\\d\\d|2[0-4]\\d|25[0-5])\\.\(0?0?\\d|[01]?\\d\\d|2[0-4]\\d|25[0-5])$");
// Data validation
tmt_globalPatterns.macaddr = new RegExp("^\([0-9a-fA-F][0-9a-fA-F]:\){5}\([0-9a-fA-F][0-9a-fA-F]\)$");
// </Vlad>

