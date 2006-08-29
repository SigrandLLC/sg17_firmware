#!/usr/bin/haserl
<? 
if [ -n "$FORM_frame" ]; then
	. common/frame_header.sh
elif [ -n "$FORM_popup" ] then
	. common/popup_header.sh
else
	. common/header.sh 
fi

. conf/conf.sh 
. lib/cfg.sh
. lib/kdb.sh

if [ -r /tmp/$FORM_SESSIONID ]; then
	. /tmp/$FORM_SESSIONID
fi

controller=$FORM_controller
action=$FORM_action

[ "$controller" ] || controller="welcome"
[ "$action" ] || action="list"

[ $DEBUG ] && echo "main(): controller=$controller action=$action <br>"

if [ -f controllers/"$controller".sh ]; then
	. controllers/"$controller".sh
else
	echo "Error: controller $controller not found"
fi


if [ -n "$FORM_frame" ]; then
	. common/frame_footer.sh
elif [ -n "$FORM_popup" ] then
	. common/popup_footer.sh
else
	. common/footer.sh
fi
	
?>
