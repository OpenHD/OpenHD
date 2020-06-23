function MAIN_TETHER_FUNCTION {
    echo "================== CHECK TETHER (tty7) ==========================="
    
    if [ "${CAM}" == "0" ]; then
        echo "Waiting some time until everything else is running ..."
        sleep 6
        tether_check_function
    else
        echo "Cam found, we are air pi, check tether function disabled"
        sleep 365d
    fi
}


function tether_check_function {
    while true; do
        #
        # pause loop while saving is in progress
        #
        pause_while


        if [ -d "/sys/class/net/usb0" ]; then
            echo
            echo "USB tethering device detected. Configuring IP ..."


            nice pump -h wifibrdcast -i usb0 --no-dns --keep-up --no-resolvconf --no-ntp || {
                echo "ERROR: Could not configure IP for USB tethering device!"
                
                nice killall wbc_status > /dev/null 2>&1
                
                if [ "$ENABLE_QOPENHD" == "Y" ]; then
                    qstatus "ERROR: Could not configure IP for USB tethering device!" 3
                else
                    wbc_status "ERROR: Could not configure IP for USB tethering device!" 7 55 0 &
                fi
                
                collect_errorlog
                
                sleep 365d
            }

            #
            # Find out hotspot device IP so video can be sent to it
            #
            # We do not directly send video to connected devices, we use a splitter process to distibute
            # the same data to all connected devices
            #
            PHONE_IP=`ip route show 0.0.0.0/0 dev usb0 | cut -d\  -f3`
            /usr/local/share/RemoteSettings/dhcpeventThread.sh add $PHONE_IP &


            #
            # Check if hotspot device has disconnected
            #
            PHONETHERE=1            
            while [  $PHONETHERE -eq 1 ]; do
                if [ -d "/sys/class/net/usb0" ]; then
                    PHONETHERE=1
                    echo "Android device still connected ..."
                else
                    PHONETHERE=0

                fi
                sleep 1
            done
        else
            echo "Android device not detected ..."
        fi
        sleep 1
    done
}
