#!/bin/sh
# (c) Vladislav Moskovets 2005
# Sigrand webface project
# 


getIfaceParams(){
    local iface="$1"
    local status="DOWN";
    local mtu;
    echo $(/usr/sbin/ip link show dev $iface | head -1 ) | while read f1 f2 f3 f4 f5 f6 f7 f8 f9; do
        status=$(echo $f3 | tr '<>,' ' ')
        [ "$f4" = "mtu" ] && mtu="$f5"
        echo "ifstatus=\"$status\""
        echo "ifmtu=\"$mtu\""
    done
}

