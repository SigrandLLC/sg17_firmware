#!/usr/bin/haserl
<? 
	. ../conf/conf.sh
	title="Modem settings"
	. ../common/header.sh
	. ../lib/misc.sh
	. ../lib/widgets.sh
 ?>

<? printTitle ?>

<?

	eval `$kdb -qq list sys_`
	printFormBegin modem modem_save.asp
	printTableTitle "Modem settings" 2 

	# sys_modem_speed
	tip="Select modem speed"
	desc="Please select modem speed"
	validator='tmt:message="Please select modem speed"'
	printInput select "Modem speed" sys_modem_speed auto Auto 64 64k 128 128k 256 256k

	# sys_modem_mode
	tip=""
	desc="Please select modem mode"
	validator='tmt:message="Please select modem mode"'
	printInput select "Mode" sys_modem_mode central Central remote Remote

	printFormSumbit
	printFormEnd

?>

<? . ../common/footer.sh ?>
