#!/bin/sh

if [ "$(whoami)" != "root" ]
then
	echo "$0 must be run as root, invoking sudo"
	exec sudo su -c "$0" "$@"
fi

echo "Seeing if Osxie is currently running"

PID=$(pgrep -f launchd)

while [ -n "$PID" ]
do
	THISPID=$(echo $PID | head -n1)
	RUNNING_USER=$(ps -o uname= -p $THISPID | head -n1)
	echo "Osxie currently running for $RUNNING_USER, shutting it down..."
	su --login "$RUNNING_USER" -c "osxie shutdown"
	sleep 2
	PID=$(pgrep -f launchd)
done

echo "No instances running now"
