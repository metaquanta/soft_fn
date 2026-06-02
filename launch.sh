#!/bin/sh

function_row_physmap=/sys/devices/platform/i8042/serio0/function_row_physmap
function_row=

if [ -r "$function_row_physmap" ]; then
	IFS= read -r function_row < "$function_row_physmap"
fi

event=`find /sys/devices/platform/i8042/ -regex .*/event[0-9]* |\
	sed 's/^.*\(event[0-9]*\).*$/\1/'`

if [ -n "$function_row" ]; then
    echo $function_row $event
	exec /usr/sbin/soft_fn --function_row="$function_row" "/dev/input/$event"
fi

exec /usr/sbin/soft_fn "/dev/input/$event"
