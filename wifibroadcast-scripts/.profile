# Change to script folder and execute main entry point
TTY=`tty`

export PATH=/usr/local/bin:${PATH}

if [ "$TTY" == "/dev/tty1" ]; then
    echo "tty1"

    service ssh start

    systemctl start openhd_security

    python /usr/local/bin/gpio-IsAir.py
    

    #
    # OpenHDRouter and the associated ptys are used for openhd microservices, our internal 
    # GCS<->Ground<->Air communications channel for things like GPIO support, live camera 
    # settings, air/ground power status, safe shutdown, etc
    #
    ionice -c 3 nice socat -lf /wbc_tmp/socat3.log -d -d pty,raw,echo=0,link=/dev/openhd_microservice1 pty,raw,echo=0,link=/dev/openhd_microservice2 & > /dev/null 2>&1
    sleep 1
    stty -F /dev/openhd_microservice1 -icrnl -ocrnl -imaxbel -opost -isig -icanon -echo -echoe -ixoff -ixon 115200


    i2cdetect -y 1 | grep  "70: 70"
    grepRet=$?
    if [[ $grepRet -eq 0 ]] ; then
        /usr/bin/python3 /usr/local/share/cameracontrol/InitArduCamV21Ch1.py
    fi

    
    export CAM=`/usr/bin/vcgencmd get_camera | python3 -c 'import sys, re; s = sys.stdin.read(); s=re.sub("supported=\d+ detected=", "", s); print(s);'`


    AIR="0"

    #
    # Note: this is duplicated logic (mostly) doing the same thing check_camera_attached does, but it won't 
    #       be here much longer and it's only a few lines.
    #
    if [[ "${CAM}" -ge 1 ]] ; then
        #
        # Normal pi cameras found, this is definitely air side so we will start the air side of SmartSync
        #
        AIR="1"
    else
        #
        # No pi cameras found, however this may still be air side so we check to see if GPIO7 is pulled low, signaling
        # should force boot as air pi anyway
        #
        if [ -e /tmp/Air ]; then
            echo "force boot as Air via GPIO"
            CAM="1"
            AIR="1"
        fi

        #
        # No pi camera detected, but we still might have a VEYE, and the only way to detect
        # it is to have i2c_vc enabled already, and then probe the i2c-0 bus
        #
        i2cdetect -y 0 | grep  "30: -- -- -- -- -- -- -- -- -- -- -- 3b -- -- -- --"
        grepRet=$?

        if [[ $grepRet -eq 0 ]] ; then
            CAM="1"
            AIR="1"
        fi
    fi


    if [ "$AIR" == "1" ]; then
        #
        # Air is always sysid 253 for all services
        #
        echo "SYSID=253" > /etc/openhd/openhd_microservice.conf
        
        systemctl start openhd_router
        sleep 1

        systemctl start openhd_microservice@status
        while [ ! -f /tmp/status_service ]; do
            sleep 0.1
        done

        /usr/local/share/wifibroadcast-scripts/configure_nics.sh

        qstatus "Configured NIC(s)" 5
        qstatus "Running SmartSync" 5

        /usr/bin/python3 /usr/local/share/RemoteSettings/Air/RemoteSettingSyncAir.py

        echo "0" > /tmp/ReadyToGo
    else
        systemctl start openhdboot
        
        #
        # Ground is always sysid 254 for all services
        #
        echo "SYSID=254" > /etc/openhd/openhd_microservice.conf
        
        systemctl start openhd_router
        
        sleep 1

        systemctl start openhd_microservice@status
        while [ ! -f /tmp/status_service ]; do
            sleep 0.1
        done
        
        #
        # No cameras found, and we did not see GPIO7 pulled low, so this is a ground station
        #
        /usr/local/share/RemoteSettings/Ground/helper/AirRSSI.sh &
        /usr/local/share/wifibroadcast-scripts/configure_nics.sh
        retCode=$?

        qstatus "Configured NIC(s)" 5
        qstatus "Running SmartSync" 5

        # now we will run SmartSync, using either GPIOs or Joystick to control it

        if [ $retCode == 1 ]; then
            # joystick selected as SmartSync control
            /usr/bin/python3 /usr/local/share/RemoteSettings/Ground/RemoteSettingsSync.py -ControlVia joystick
        fi

        if [ $retCode == 2 ]; then
            # GPIO  selected as SmartSync control
            /usr/bin/python3 /usr/local/share/RemoteSettings/Ground/RemoteSettingsSync.py -ControlVia GPIO
        fi
    
        echo "0" > /tmp/ReadyToGo
        
        echo "Configuring system"
        qstatus "Configuring system" 5
    fi
fi


while [ ! -f /tmp/ReadyToGo ]
do
    echo "sleep..."
    sleep 1
done

cd /usr/local/share/wifibroadcast-scripts
source main.sh
