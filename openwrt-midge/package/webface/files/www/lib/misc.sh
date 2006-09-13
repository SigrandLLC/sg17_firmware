#!/bin/sh
# (c) Vladislav Moskovets 2005
# Sigrand webface project
# 

debug(){
	[ $DEBUG ] && echo $* "<br>"
}

warn(){
	[ $WARN ] && echo $* "<br>"
}

humanUnits(){
    local k="0";
    if [ "$1" = "-k" ]; then
        k=1;
        shift
    fi 
    local l="$1"
    local units="$2"
    local base=1024;
    local s=0;
    local u;
    local dc;
    /usr/bin/dc -e "1" 2>/dev/null && dc="dc -e "
    /usr/bin/dc "1" 2>/dev/null && dc="dc "
    [ "$3" ] && base=$3
    if [ "$l" -ge $(($base*$base*$base)) ]; then
        [ "$dc" ] && t=$($dc $l 1024 / 1024 / 1024 / p) || t=$(($l/$base/$base/$base)) 
        s=$((3+$k));
    elif [ "$l" -ge $(($base*$base)) ]; then
        [ "$dc" ] && t=$($dc $l 1024 / 1024 / p) || t=$(($l/$base/$base)) 
        t=$(($l/$base/$base)) 
        s=$((2+$k));
    elif [ "$l" -ge $(($base)) ]; then
        [ "$dc" ] && t=$($dc $l 1024 / p) || t=$(($l/$base)) 
        t=$(($l/$base))
        s=$((1+$k));
    else
        t=$l 
        s=$k;
    fi
    
    [ "$s" = "3" ] && u='G'
    [ "$s" = "2" ] && u='M'
    [ "$s" = "1" ] && u='K'
    [ "$s" = "0" ] && u=''
    echo "$t ${u}${units}";
}

getFsType(){
    local dev=$1;
    type=$(mount| grep "^$dev " | cut -f5 -d" " | head -1)
    echo $type
}

#int2ip() {
#	i=$1
#	d0=$(($i/256/256/256));
#	d1=$((($i-$d0*256*256*256)/256/256));
#	d2=$((($i-$d0*256*256*256-$d1*256*256)/256));
#	d3=$(($i-$d0*256*256*256-$d1*256*256-$d2*256));
#	echo "$d0.$d1.$d2.$d3";
#}

#ip2int2() {
#	echo $1 | { IFS="." read a0 a1 a2 a3; echo $(($a0*256*256*256+$a1*256*256+$a2*256+$a3)); }
#}


valid_netmask () {
  return $((-($1)&~$1))
}

ip2int () (
  set $(echo $1 | tr '\.' ' ')
  echo $(($1<<24|$2<<16|$3<<8|$4))
)

int2ip () {
  echo $(($1>>24&255)).$(($1>>16&255)).$(($1>>8&255)).$(($1&255))
}


get_new_serial() {
	local today=$(date +%Y%d%m)

}