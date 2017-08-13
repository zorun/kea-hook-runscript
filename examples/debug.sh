#!/bin/sh

LOGFILE=/tmp/kea-hook-runscript-debug.log

echo "== $1 ==" >> $LOGFILE
date >> $LOGFILE
env >> $LOGFILE
echo >> $LOGFILE
echo >> $LOGFILE
