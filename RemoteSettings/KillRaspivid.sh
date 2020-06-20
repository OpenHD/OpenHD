#!/bin/bash

if [ -e "/tmp/settings.sh" ]; then
    OK=`bash -n /tmp/settings.sh`
    if [ "$?" == "0" ]; then
        source /tmp/settings.sh
    fi
fi

if [ "$ENABLE_OPENHDVID" == "Y" ]; then
	CAMERA_PROGRAM="openhdvid"
else
	CAMERA_PROGRAM="raspivid"
fi

ps -ef | nice grep "${CAMERA_PROGRAM}" | nice grep -v grep | awk '{print $2}' | xargs kill -9
ps -ef | nice grep "tx_rawsock -p 0 -b" | nice grep -v grep | awk '{print $2}' | xargs kill -9
