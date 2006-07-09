#!/bin/sh
# (c) Vladislav Moskovets 2005
# Sigrand webface project
#

[ "$refresh" ] && echo "Refresh: $refresh;url=$refresh_url"
echo 'Cache-Control: no-cache
Content-Type: text/html

<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<html>
<head>'

echo "<title>"$title'</title>
<script type="text/javascript" src="/js/overlib.js"></script>
<script type="text/javascript" src="/js/overlib_bubble.js"> </script>
<script type="text/javascript" src="/js/script_tmt_validator.js"> </script>
<script type="text/javascript" src="/js/vlad_tmt_validator.js"> </script>
</head>
<link rel="stylesheet" type="text/css" href="/css/content.css">
<body>
<div id="overDiv" style="position:absolute; visibility:hidden; z-index:1000;"></div>
<div class=background>'
