function MAIN_RC_TX_RX_FUNCTION {
    echo "================== R/C TX (tty3) ==========================="
    
    #
    # Only run rctx if no cam found and rc is not disabled
    #
    if [ "$CAM" == "0" ] && [ "$RC" != "disabled" ]; then
        echo "Running on ground pi, RC enabled"
        rctx_function
    fi
    
    echo "RC not enabled in config file, or running on air pi"
    
    sleep 365d
}

# runs on RX (ground pi)
function rctx_function {
    #
    # Convert joystick config from DOS format to UNIX format
    #
    ionice -c 3 nice dos2unix -n /boot/joyconfig.txt /tmp/rctx.h > /dev/null 2>&1
    
    echo
    
    echo "Building rctx"
    
    cd /home/pi/wifibroadcast-rc-Ath9k/
    
    if [ "$PrimaryCardMAC" == "0" ]; then
        echo "PrimaryCardMAC not selected. RC Joystick program will use WiFi card with best RSSI as tx."
    else
        echo "PrimaryCardMAC selected to: $PrimaryCardMAC"

        if [ "$IsBandSwicherEnabled" == "1" ]; then
            echo "BandSwicher Enabled"
        fi
    fi
  
    
    ./build.sh

    cp rctx /tmp/
    
    #
    # Wait until video is running to make sure NICS are configured and wifibroadcast_rx_status shared memory is available
    #

    echo
    echo -n "Waiting until nics are configured ..."
    while [ ! -f /tmp/nics_configured ]; do
        sleep 0.5
        echo -n "."
    done
    
    sleep 0.5

    NICS=`ls /sys/class/net/ | nice grep -v eth0 | nice grep -v lo | nice grep -v usb | nice grep -v intwifi | nice grep -v relay | nice grep -v wifihotspot`
        
    pause_while
    

    echo
    echo "Starting RC TX..."
    
    FirstTimeRC=0
    IsEncrypt=0
    IsIPCameraSwitcherEnabled=0
    

    #
    # USB and IP cameras always use the SVPCOM system at the moment
    #
    if [ "$SecondaryCamera" == "USB" ]; then
        IsIPCameraSwitcherEnabled=1
    fi
    
    if [ "$SecondaryCamera" == "IP" ]; then
        IsIPCameraSwitcherEnabled=1
    fi

    if [ "$EncryptionOrRange" == "Range" ]; then
        IsEncrypt=0
    fi

    if [ "$EncryptionOrRange" == "Encryption" ]; then
        IsEncrypt=1
    fi


    while true; do
    
        if [ "$PrimaryCardMAC" == "0" ]; then
            if [ "$IsBandSwicherEnabled" == "1" ]; then
                echo "To use BandSwitcher select dedicated WiFi card for Tx. Set PrimaryCardMAC"
                IsBandSwicherEnabled=0
            fi

            if [ $FirstTimeRC == 0  ]; then
                FirstTimeRC=1
                /home/pi/wifibroadcast-rc-Ath9k/rctxUDP.sh $ChannelToListen2 $ChannelIPCamera $IsBandSwicherEnabled $IsIPCameraSwitcherEnabled $IsEncrypt $NICS &
            fi

            nice -n -5 /tmp/rctx
            
            sleep 1
            
            NICS=`ls /sys/class/net/ | nice grep -v eth0 | nice grep -v lo | nice grep -v usb | nice grep -v intwifi | nice grep -v relay | nice grep -v wifihotspot`
        else
            #if [ "$IsBandSwicherEnabled" == "1" ]; then
                if [ $FirstTimeRC == 0  ]; then
                    FirstTimeRC=1
                    /home/pi/wifibroadcast-rc-Ath9k/rctxUDP.sh $ChannelToListen2 $ChannelIPCamera $IsBandSwicherEnabled $IsIPCameraSwitcherEnabled $IsEncrypt $PrimaryCardMAC &
                fi

                #nice -n -5 /tmp/rctx
                #sleep 1	
            #else
                #if [ $FirstTimeRC == 0  ]; then
                    #FirstTimeRC=1
                    #/home/pi/wifibroadcast-rc-Ath9k/rctxUDP.sh 0 $PrimaryCardMAC &
                #fi
                
                #nice -n -5 /tmp/rctx
                
                #sleep 1
            #fi


            nice -n -5 /tmp/rctx
            sleep 1
        fi
    done
}
