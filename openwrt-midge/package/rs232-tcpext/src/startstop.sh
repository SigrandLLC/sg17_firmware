#!/bin/sh

prog=./rs232-tcpext
#host=0.0.0.0
host=localhost
port=3000
 listen_tty=ttyUSB0
connect_tty=ttyUSB1

mstate_intval=4000
restart_delay=1000

usage()
{
	echo 1>&2 "Usage: $0 {[re]start | stop | status} {listen | connect}"
	exit 1
}

if test "$#" -ne 2; then
	usage
fi

action=$1; shift
  mode=$1; shift
#  tty=$1; shift

case $mode in
	listen)  tty=$listen_tty;;
	connect) tty=$connect_tty;;
	*) usage;;
esac

ttydev=/dev/$tty
pidfile=/var/run/rs232-tcpext..${tty}-${mode}.pid

case $action in
   start)
	$prog $ttydev $host $port $mode $pidfile $mstate_intval $restart_delay
	;;
    stop)
	if $0 status $mode >/dev/null; then
		kill `cat $pidfile`
	fi
	while $0 status $mode >/dev/null; do sleep 0.1s; done
	;;
 restart)
	$0 stop  $mode
	$0 start $mode
	;;
  status)
	if test -f $pidfile; then
		pid=`cat $pidfile`
		if kill -0 $pid 2>/dev/null; then
			echo "Running"
			exit 0
		else
			echo "Not running with orphaned pid file"
			exit 1
		fi
	else
		echo "Not running"
		exit 1
	fi
	;;
       *)
	usage
	;;
esac
