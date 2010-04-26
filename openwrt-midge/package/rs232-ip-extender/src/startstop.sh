#!/bin/sh

prog=./rs232-ip-extender
#host=0.0.0.0
host=localhost
port=3000

usage()
{
	echo 1>&2 "Usage: $0 {start | stop} {listen | connect} tty"
	exit 1
}

if test "$#" -ne 3; then
	usage
fi

action=$1; shift
  mode=$1; shift
   tty=$1; shift

ttydev=/dev/$tty
pidfile=/var/run/rs232-ip-extender..${tty}-${mode}.pid

case $mode in
	listen)  ;;
	connect) ;;
	*) usage;;
esac

case $action in
   start)
	$prog $ttydev $host $port $mode $pidfile ;;
    stop)
	kill `cat $pidfile` ;;
       *)
	usage;;
esac
