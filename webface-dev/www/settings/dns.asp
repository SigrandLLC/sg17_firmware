#!/usr/bin/haserl
<? 
	. ../conf/conf.sh
	title="DNS settings"
	. ../common/header.sh
	. ../lib/misc.sh
	. ../lib/widgets.sh
 ?>

<? printTitle ?>


<?

	eval `$kdb -qq list sys_`
	printFormBegin dns dns_save.asp
	printTableTitle "$title" 2 

	# sys_dns_nameserver
	tip="Dns server for your router BLA BLA BLA"
	desc="Please enter ip address of upstream dns server"
	validator='tmt:filters="ltrim,rtrim,nohtml,nospaces,nocommas,nomagic"  tmt:message="Please input correct ip address" tmt:pattern="ipaddr"'
	printInput text "Upstream server" sys_dns_nameserver

	# sys_dns_domain
	tip="Domain for your net BLA BLA BLA"
	desc="Please domain"
	validator=''
	printInput text "Domain" sys_dns_domain

	printFormSumbit
	printFormEnd

?>

<? . ../common/footer.sh ?>
