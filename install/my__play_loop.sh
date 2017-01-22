#!/bin/bash
source /bin/my__osd_cat.sh 2> /dev/null
if [ ! -z "$1" ] ; then
    f_echo_small "$1"  2> /dev/null > /dev/null  &
fi

if [ ! -z "$1" ] ; then
    f_echo_small "$1" 2> /dev/null > /dev/null  &
fi
