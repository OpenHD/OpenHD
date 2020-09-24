#!/bin/bash

if [ -e "/tmp/settings.sh" ]; then
    OK=`bash -n /tmp/settings.sh`
    if [ "$?" == "0" ]; then
        source /tmp/settings.sh
    fi
fi


ps -ef | nice grep "raspivid" | nice grep -v grep | awk '{print $2}' | xargs kill -9
ps -ef | nice grep "tx_rawsock -p 0 -b" | nice grep -v grep | awk '{print $2}' | xargs kill -9
