#!/bin/sh

function_row_physmap=/sys/devices/platform/i8042/serio0/function_row_physmap

event=`find /sys/devices/platform/i8042/ -regex .*/event[0-9]* |\
	sed 's/^.*\(event[0-9]*\).*$/\1/'`

if [ -r "$function_row_physmap" ]; then
	function_row=`/usr/sbin/toprow "/dev/input/$event" "$function_row_physmap"`
	exec /usr/sbin/soft_fn --function_row="$function_row" "/dev/input/$event"
fi

exec /usr/sbin/soft_fn "/dev/input/$event"
