#!/usr/bin/haserl
<?
	echo "Content-type: text/plain"
	echo ""
	/bin/grep -v "/bin/sh" /www/oem.sh
?>
