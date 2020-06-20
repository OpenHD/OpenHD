#!/bin/bash

#!/bin/bash

while ! [ -e /tmp/IsTerminateRemoteSettingsPath.txt ]
do
        echo "RSSI" > /dev/udp/127.0.0.1/9292
        sleep 1
done
