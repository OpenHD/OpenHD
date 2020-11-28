#!/bin/bash

if [ -e "/tmp/settings.sh" ]; then
    OK=`bash -n /tmp/settings.sh`
    if [ "$?" == "0" ]; then
        source /tmp/settings.sh
    fi
fi

modprobe v4l2loopback devices=5

FlatFieldFile=/var/run/openhd/flat_field.png

/usr/local/bin/seek_create_flat_field -o ${FlatFieldFile} -t ${SeekModel}

while true
do
    /usr/local/bin/seek_viewer -m v4l2 -o /dev/video4 -f ${SeekFramerate} -c ${SeekColormap} -r ${SeekRotate} -t ${SeekModel} -F ${FlatFieldFile}
    echo "Restarting Seek Thermal"
    sleep 2
done
