#!/bin/sh

if [ "$(whoami)" != "root" ]
then
	echo "$0 must be run as root, attempting privilege escalation..."
	if command -v pkexec >/dev/null 2>&1; then
		exec pkexec "$0" "$@"
	elif command -v sudo >/dev/null 2>&1; then
		exec sudo su -c "$0" "$@"
	else
		echo "Error: need root. Install polkit (pkexec) or sudo."
		exit 1
	fi
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
