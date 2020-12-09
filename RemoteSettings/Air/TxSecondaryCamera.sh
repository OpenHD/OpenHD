#!/bin/bash
source /tmp/settings.sh

cd /usr/local/share/cameracontrol/IPCamera/svpcom_wifibroadcast/


NICS_LIST=`ls /sys/class/net/ | nice grep -v eth0 | nice grep -v lo | nice grep -v usb | nice grep -v intwifi | nice grep -v wlan | nice grep -v relay | nice grep -v wifihotspot`

VIDEO_FRAMETYPE=$(cat /tmp/video_frametype)

while  [ 1 ]
do
        echo "start wfb_tx -u 5600 -t 2 -p 23 -B 20 -M $DATARATE_SECONDARY_8812AU -n $VIDEO_BLOCKS_SECONDARY -k $VIDEO_FECS_SECONDARY $NICS_LIST \n"
        /usr/local/share/cameracontrol/IPCamera/svpcom_wifibroadcast/wfb_tx -u 5600 -t ${VIDEO_FRAMETYPE} -p 23 -B 20 -M $DATARATE_SECONDARY_8812AU -n $VIDEO_BLOCKS_SECONDARY -k $VIDEO_FECS_SECONDARY -K /tmp/tx.key $NICS_LIST

        NICS_LIST=`ls /sys/class/net/ | nice grep -v eth0 | nice grep -v lo | nice grep -v usb | nice grep -v intwifi | nice grep -v wlan | nice grep -v relay | nice grep -v wifihotspot`
        echo "./wfb_tx -u 5600 -t 2 -p 23 -B 20  -M $DATARATE_SECONDARY_8812AU -n $VIDEO_BLOCKS_SECONDARY -k $VIDEO_FECS_SECONDARY Restating with:  $NICS_LIST \n"
        sleep 2
done
