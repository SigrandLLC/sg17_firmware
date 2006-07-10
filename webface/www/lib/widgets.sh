#!/bin/sh
# (c) Vladislav Moskovets 2005
# Sigrand webface project
# 

ProgressBarH(){
    local t="$1";
    local l="$2";
    local f="$3";
    local p=$(($l*100/$f))
    local w=$(($p*2))
    echo '<img src=/images/bar_left.gif><img src=/images/bar_middle.gif height=16 width='$w'><img src=/images/bar_right.gif>' $p '%'
}

printTitle(){
    local txt;
    [ "$title" ] && txt="$title"
    [ "$1" ] && txt="$1"
    [ "$txt" ] && echo '<h1>'$txt'</h1>';
}

printTableTitle(){
    local text="$1"
    local colspan;
    [ "$2" ] && colspan="colspan=$2"


    #echo "<tr><th class='caption' $colspan>$text</th></tr>";

    echo "<tr style='height: 15px'><th $colspan style='margin:0; padding:0;'> \
<table width=100% border=0 cellspacing=0 cellpadding=0 style='margin:0; padding: 0; border:0; height:22'><tr>
<td width=73 background='/images/cap_left.png' style='border-bottom:0'>
<td align=center style='border:0; margin:0; padding:0; background: transparent url(/images/cap_middle.png) repeat-x;'><b>$text</b></td>
<td width=73 background='/images/cap_right.png'</td></tr></table></th></tr>";
}

displayEnv() 
{
    echo "<table>"
    printTableTitle env
    echo "<tr><td><pre class='code'>"
    set
    echo "</pre></td></tr></table>"
}
displayFile() 
{
    file="$1"
    echo "<table>"
    printTableTitle "$file" 
    echo "<tr><td><pre class='code'>"
    cat $file
    echo "</pre></td></tr></table>"
}

displayString() 
{
    echo "<table>"
    printTableTitle $*
    echo "<tr><td><pre class='code'>"
    echo $*
    echo "</pre></td></tr></table>"
}
printFormBegin(){
	local act="$SCRIPT_NAME";
	local lname="midge_form"
	[ "$1" ] && lname="$1"
	[ "$action" ] && act="$action"
    [ "$2" ] && act="$2"

	echo "<form action='$act' name='$lname' method='post' tmt:validate='true'>"
	#echo "<input type=hidden name=SESSIONID value='$SESSIONID'>"
	echo "<table class='inputTable'>"
}

printInput(){
	local type="$1"
	local text="$2"
	local inputname="$3"
	#local options="$4"
	local inputsize='25'
	local maxlenght='255'
    local tipcode=''
    [ "$tip" ] && tipcode="onmouseover=\"return overlib('$tip', BUBBLE, BUBBLETYPE, 'roundcorners')\" onmouseout=\"return nd();\""
    eval 'value=$'$inputname

	shift 3

	echo "
<!-- ------- printInput $* -->"

    [ ! $type = "hidden" ] && echo "<tr>
<td class='inputTable' width=200><label for='$inputname' $tipcode>$text</label>
<br><span class='inputDesc' $tipcode>$desc</span>
</td>
<td class='inputTable'>";

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
		#echo "<table class=radioTable>"
		while [ "$1" ]; do
			#echo "<tr><td>"
			echo -n "<label $tipcode><input type='radio' class='button' $tipcode name='$inputname' $validator tmt:errorclass='invalid'"
			[ "$value" = "$1" ] && echo -n " checked "
			echo "value='$1'>$2</label><br>"
			#echo "</td> "
			#echo "<td> $2</td></tr>"
			validator=""
			shift 2
		done
		#echo "</table>"
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
        
    [ ! $type = "hidden" ] && echo "</td></tr>"
	echo "<!-- ------- /printInput $* -->"
    tip=''
    desc=''
    validator=''
}

printFormSubmit(){
	local btn="Ok";
	[ "$1" ] && btn="$1"
	echo "<tr> <td colspan=2 style='text-align: center;'> <input class='button' type='submit' name='submit' value='$btn'> </td> </tr>";
}

printFormEnd(){
	echo "</table> <!-- /fieldset--> </form>";
}

displayMessageBox() 
{
    local title="$1"
	local text="$2"
    echo "<table>"
    printTableTitle "$title" 
    echo "<tr><td>$text</td></tr></table>"
}

flashMessage(){
	if [ "" ]; then
		echo df
	fi
}
