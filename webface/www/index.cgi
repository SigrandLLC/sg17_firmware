#!/usr/bin/haserl
<? 
. common/header.sh 

if [ -r /tmp/$FORM_SESSIONID ]; then
	. /tmp/$FORM_SESSIONID
fi

controller=$FORM_controller
action=$FORM_action

[ "$controller" ] || controller="welcome"
[ "$action" ] || action="list"

if [ -f controllers/"$controller".sh ]; then
	. controllers/"$controller".sh
else
	echo "Error: controller $controller not found"
fi


. common/footer.sh 
?>
