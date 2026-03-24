#!/bin/sh

if [[ -e /dev/input/touchscreen0 ]]; then
    export TSLIB_TSDEVICE=/dev/input/touchscreen0
    export TSLIB_FBDEVICE=/dev/fb0
    export TSLIB_CALIBFILE=/etc/pointercal
    export TSLIB_CONFFILE=/etc/ts.conf
    export TSLIB_PLUGINDIR=/usr/lib/ts
    export TSLIB_CONSOLEDEVICE=none
    export TSLIB_TSEVENTTYPE=input
fi
