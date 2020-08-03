#!/bin/sh

exec /usr/sbin/soft_fn /dev/input/`\
		find /sys/devices/platform/i8042/ -regex .*/event[0-9]* |\
	       	sed 's/^.*\(event[0-9]*\).*$/\1/'`
