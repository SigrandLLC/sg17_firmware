#!/bin/sh

prog=./rs232-ip-extender
#host=0.0.0.0
host=localhost
port=3000
 listen_tty=ttyUSB0
connect_tty=ttyUSB1

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
pidfile=/var/run/rs232-ip-extender..${tty}-${mode}.pid

case $action in
   start)
	$prog $ttydev $host $port $mode $pidfile
	;;
    stop)
	while $0 status $mode >/dev/null; do kill `cat $pidfile`; done
	;;
 restart)
	$0 stop  $mode
	#sleep 0.5s
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
