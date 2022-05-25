#!/bin/bash

if [ -e "/tmp/settings.sh" ]; then
    OK=`bash -n /tmp/settings.sh`
    if [ "$?" == "0" ]; then
        source /tmp/settings.sh
    fi
fi

modprobe v4l2loopback devices=5

while true
do
    /usr/local/bin/seek_viewer -m v4l2 -o /dev/video4 -f ${SeekFramerate} -c ${SeekColormap} -r ${SeekRotate} -t ${SeekModel}
    echo "Restarting Seek Thermal"
    sleep 2
done
