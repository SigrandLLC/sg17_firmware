#!/bin/sh
# (c) Vladislav Moskovets 2005
# Sigrand webface project
# 

render_chart_h(){
    local t="$1";
    local l="$2";
    local f="$3";
    local p=$(($l*100/$f))
    local w=$(($p*2))
    echo '<img src=/images/bar_left.gif><img src=/images/bar_middle.gif height=16 width='$w'><img src=/images/bar_right.gif>' $p '%'
}

render_title(){
    local txt;
    [ "$title" ] && txt="$title"
    [ "$1" ] && txt="$1"
    [ "$txt" ] && echo '<h1>'$txt'</h1>';
}

render_table_title(){
    local text="$1"
    local colspan;
    [ "$2" ] && colspan="colspan='$2'"
	echo "<tr> <td $colspan class='listtopic'>$text</td> </tr>"
}

displayEnv() 
{
    echo "<table>"
    render_table_title env
    echo "<tr><td><pre class='code'>"
    set
    echo "</pre></td></tr></table>"
}
displayFile() 
{
    file="$1"
    echo "<table>"
    render_table_title "$file" 
    echo "<tr><td><pre class='code'>"
    cat $file
    echo "</pre></td></tr></table>"
}

displayString() 
{
    echo "<table>"
    render_table_title $*
    echo "<tr><td><pre class='code'>"
    echo $*
    echo "</pre></td></tr></table>"
}

render_message_box() 
{
    local title="$1"
	local text="$2"
    echo "<table>"
    render_table_title "$title" 
    echo "<tr><td>$text</td></tr></table>"
}

render_save_message(){
	echo "<table id='message' width='100%' border='0' cellspacing='0' cellpadding='0'>"
	[ -z "$ERROR_MESSAGE" ] && render_table_title "Information" || render_table_title "Error"
	
	echo "<tr><td width='100%' class='listr'>" 
	[ -z "$ERROR_MESSAGE" ] && echo $ok_str || echo "$ERROR_MESSAGE <br> $fail_str"
	echo "</td></tr>";
	echo "</table> <br /> <br />"
	render_js_hide_message
}

render_form_header(){
	#local act="$SCRIPT_NAME";
	local lname="midge_form"
	[ "$1" ] && lname="$1"
	[ "$action" ] && act="$action"
    [ "$2" ] && act="$2"

	echo "<form name='$lname' method='post' tmt:validate='true'>"
	#echo "<input type=hidden name=SESSIONID value='$SESSIONID'>"
	echo "<input type=hidden name=action value='$action'>"
	echo "<input type=hidden name=controller value='$controller'>"
	echo "<table width='100%' border='0' cellspacing='0' cellpadding='0'>"
}

render_input_field(){
	local type="$1"
	local text="$2"
	local inputname="$3"
	local inputsize='25'
	local maxlenght='255'
    local tipcode=''
    [ "$tip" ] && tipcode="onmouseover=\"return overlib('$tip', BUBBLE, BUBBLETYPE, 'roundcorners')\" onmouseout=\"return nd();\""
    eval 'value=$'$inputname

	shift 3

	echo "
<!-- ------- render_input_field $type $text $inputname $* -->"


    [ ! $type = "hidden" ] && echo "<tr>
<td width="25%" class='vncellt'><label for='$inputname' $tipcode>$text</label></td>
<td width="75%" class='listr'>";

    case $type in
    text)
        echo "	<input type='text' class='edit' $tipcode name='$inputname' size='$inputsize' maxlength='$maxlenght' $validator tmt:errorclass='invalid' value='$value'> "
        ;;
    checkbox)
        echo -n "	<input type='checkbox' class='edit' $tipcode name='$inputname' $validator tmt:errorclass='invalid'"
        for i in ${value%%0}; do echo -n " checked=1 "; done
        echo '> '
        ;;
    radio)
		while [ "$1" ]; do
			echo -n "<label $tipcode><input type='radio' class='button' $tipcode name='$inputname' $validator tmt:errorclass='invalid'"
			[ "$value" = "$1" ] && echo -n " checked "
			echo "value='$1'>$2</label><br>"
			validator=""
			shift 2
		done
		;;
	select)
		echo -n "<select  $tipcode name='$inputname' class='edit' $validator tmt:errorclass='invalid'>"
		while [ "$1" ]; do
			echo "<option value=$1"
			[ "$value" = "$1" ] && echo -n " selected "
			echo ">$2</option>"
			shift 2
		done
		;;
    hidden)
		value="$1"
		#echo "value="$value > /www/settings/tmpf
        echo "<input type='hidden' name='$inputname' value='$value'>"
        ;;
	esac
        
    [ ! $type = "hidden" ] && echo "<br><span class='inputDesc' $tipcode>$desc</span></td></tr>"
	echo "<!-- ------- /render_input_field $type $text $inputname $* -->"
    tip=''
    desc=''
    validator=''
}

render_submit_field(){
	local btn="Ok";
	[ "$1" ] && btn="$1"
	echo "<tr> <td colspan=2 style='text-align: center;'> <input class='button' type='submit' name='submit' value='$btn'> </td> </tr>";
}

render_form_tail(){
	echo "</table> <!-- /fieldset--> </form><br/><br/>";
}

render_iframe_list(){
	local controller="$1"
	echo "<tr><td><iframe name=$controller src='index.cgi?controller=$controller&frame=1' frameborder=1 width='$IFRAME_WIDTH' height='$IFRAME_HEIGHT' scrolling='auto'></iframe></td></tr>"
}


render_button_list_add(){
	local controller="$1"
	local item="$2"
	echo "<tr><td align='center'><a href='javascript:openPopup(window, \"${controller}_edit\", \"$item\", \"additem=1\");'><img src='images/plus.gif' title='Add item' width='17' height='17' border='0'>Add item</a></td></tr>"
}

render_list_header(){
		local s1="<tr>"
		local s2="<td class='listtopic'>"
		local s3="</td>"
		local s4="</tr>"
		
		echo $s1
		for n in "$@"; do
			echo $s2 $n $s3
		done
		echo $s2 Action $s3
		echo $s4
}

render_list_btns(){
		local controller=$1
		local item=$2
		
		# edit
		echo "<a href='javascript:openPopup(window, \"${controller}_edit\", \"$item\");'><img src='images/e.gif' title='Edit item' width='17' height='17' border='0'></a>"
		
		# del
		echo "<a href='index.cgi?controller=${controller}&do=del&item=${item}&frame=1' target='_self' onclick='return confirmSubmit()'><img src='images/x.gif' title='Delete item' width='17' height='17' border='0'></a>"
	
}

render_js_close_popup() {
	local timeout=${1:-2500}
	echo "<script language=\"JavaScript\">setTimeout('window.close()',$timeout);</script>"
}

render_js_refresh_parent() {
	echo "<script language=\"JavaScript\">window.opener.location = window.opener.location</script>"
}

render_js_refresh_window() {
	local timeout=${1:-2000}
	echo "<script language=\"JavaScript\">setTimeout('window.location = window.location', $timeout);</script>"
}

render_js_hide_message(){
	local timeout=${1:-1500}
	echo "<script language=\"JavaScript\">setTimeout('document.getElementById(\"message\").style.display = \"none\";', $timeout);</script>"
}