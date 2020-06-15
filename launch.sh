#!/bin/sh

/usr/sbin/soft_fn `find /sys/devices/platform/i8042/ -regex .*/event[0-9]`
