# Change to script folder and execute main entry point
TTY=`tty`


if [ "$TTY" == "/dev/tty1" ]; then
    echo "tty1"

    service ssh start

    python /root/wifibroadcast_misc/gpio-IsAir.py

    i2cdetect -y 1 | grep  "70: 70"
    grepRet=$?
    if [[ $grepRet -eq 0 ]] ; then
        /usr/bin/python3 /home/pi/cameracontrol/InitArduCamV21Ch1.py
    fi

    
    CAM=`/usr/bin/vcgencmd get_camera | python3 -c 'import sys, re; s = sys.stdin.read(); s=re.sub("supported=\d+ detected=", "", s); print(s);'`

    i2cdetect -y 0 | grep  "30: -- -- -- -- -- -- -- -- -- -- -- 3b -- -- -- --"
    grepRet=$?

    if [[ $grepRet -eq 0 ]] ; then
        #
        # Normal pi cameras found, this is definitely air side so we will start the air side of SmartSync
        #
        /usr/bin/python3 /home/pi/RemoteSettings/Air/RemoteSettingSyncAir.py
        CAM="1"
        echo "0" > /tmp/ReadyToGo
    else
        #
        # No pi cameras found, however this may still be air side so we check to see if GPIO7 is pulled low, signaling
        # should force boot as air pi anyway
        #
        if [ -e /tmp/Air ]; then
            echo "force boot as Air via GPIO"
            CAM="1"
        fi

        if [ "$CAM" == "0" ]; then
            #
            # No cameras found, and we did not see GPIO7 pulled low, so this is a ground station
            #
            /home/pi/RemoteSettings/Ground/helper/AirRSSI.sh &
            /home/pi/RemoteSettings/Ground/helper/DisplayProgram/DisplayProgram &
            /home/pi/RemoteSettings/Ground/helper/ConfigureNics.sh
            retCode=$?
        
            # now we will run SmartSync, using either GPIOs or Joystick to control it

            if [ $retCode == 1 ]; then
                # joystick selected as SmartSync control
                /usr/bin/python3 /home/pi/RemoteSettings/Ground/RemoteSettingsSync.py -ControlVia joystick
            fi

            if [ $retCode == 2 ]; then
                # GPIO  selected as SmartSync control
                /usr/bin/python3 /home/pi/RemoteSettings/Ground/RemoteSettingsSync.py -ControlVia GPIO
            fi

            # kill the SmartSync background now that SmartSync is finished
            killall omxplayer  > /dev/null 2>/dev/null
            killall omxplayer.bin  > /dev/null 2>/dev/null
            /usr/bin/omxplayer --loop /home/pi/RemoteSettings/Ground/helper/DisplayProgram/video/AfterSSync.mp4 > /dev/null 2>/dev/null &
    
            echo "0" > /tmp/ReadyToGo
        else
            # we were forced to start as an air pi via GPIO7
            /usr/bin/python3 /home/pi/RemoteSettings/Air/RemoteSettingSyncAir.py

            echo "0" > /tmp/ReadyToGo
        fi
    fi
fi


while [ ! -f /tmp/ReadyToGo ]
do
    echo "sleep..."
    sleep 1
done

echo "SmartSync finished, beginning OpenHD hardware+video configuration"

cd /home/pi/wifibroadcast-scripts
source main.sh
