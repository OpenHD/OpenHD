function MAIN_RC_TX_RX_FUNCTION {
	echo "================== R/C TX (tty3) ==========================="
	
	# only run rctx if no cam found and rc is not disabled
	if [ "$CAM" == "0" ] && [ "$RC" != "disabled" ]; then
	    echo "R/C enabled ... we are R/C TX (Ground Pi)"
	    rctx_function
	fi
	
	echo "R/C not enabled in configfile or we are R/C RX (Air Pi)"
	
	sleep 365d
}

# runs on RX (ground pi)
function rctx_function {
    # Convert joystick config from DOS format to UNIX format
    ionice -c 3 nice dos2unix -n /boot/joyconfig.txt /tmp/rctx.h > /dev/null 2>&1
	
    echo
	
    echo Building RC ...
    
    cd /home/pi/wifibroadcast-rc-Ath9k/
    
    if [ "$PrimaryCardMAC" == "0" ]; then
    	echo "PrimaryCardMAC not selected. RC Joystick program will use WiFi card with best RSSI as tx."
    else
    	echo "PrimaryCardMAC selected to: $PrimaryCardMAC"
    	if [ "$IsBandSwicherEnabled" == "1" ]; then
		echo "BandSwicher Enabled"
#        	cd /home/pi/wifibroadcast-rc-Ath9k/
    	fi
    fi
  
    
    ionice -c 3 nice gcc -lrt -lwiringPi -lpcap -I/home/pi/wifibroadcast-base rctx.c -o /tmp/rctx `sdl-config --libs` `sdl-config --cflags` || {
		echo "ERROR: Could not build RC, check joyconfig.txt!"
    }
	
    # wait until video is running to make sure NICS are configured and wifibroadcast_rx_status shmem is available

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
    echo "Starting R/C TX ..."
	
    FirstTimeRC=0
    IsEncrypt=0
    IsIPCameraSwitcherEnabled=0
    
    
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

		nice -n -5 /tmp/rctx $ChannelToListen2 $ChannelIPCamera $IsBandSwicherEnabled $IsIPCameraSwitcherEnabled $IsEncrypt $TrimChannel $Action $PWMCount $ActivateChannel $NICS
		sleep 1
		NICS=`ls /sys/class/net/ | nice grep -v eth0 | nice grep -v lo | nice grep -v usb | nice grep -v intwifi | nice grep -v relay | nice grep -v wifihotspot`
	else
		#if [ "$IsBandSwicherEnabled" == "1" ]; then
			if [ $FirstTimeRC == 0  ]; then
	                    FirstTimeRC=1
        	           /home/pi/wifibroadcast-rc-Ath9k/rctxUDP.sh $ChannelToListen2 $ChannelIPCamera $IsBandSwicherEnabled $IsIPCameraSwitcherEnabled $IsEncrypt $PrimaryCardMAC &
                	fi

		#	nice -n -5 /tmp/rctx $ChannelToListen2 $ChannelIPCamera $PrimaryCardMAC
		#	nice -n -5 /tmp/rctx $ChannelToListen2 $ChannelIPCamera $IsBandSwicherEnabled $IsIPCameraSwitcherEnabled $IsEncrypt $PrimaryCardMAC
		#	sleep 1	
		#else
		#	if [ $FirstTimeRC == 0  ]; then
                #            FirstTimeRC=1
                #           /home/pi/wifibroadcast-rc-Ath9k/rctxUDP.sh 0 $PrimaryCardMAC &
                #        fi
		#	nice -n -5 /tmp/rctx $PrimaryCardMAC
		#	sleep 1
		#fi

		nice -n -5 /tmp/rctx $ChannelToListen2 $ChannelIPCamera $IsBandSwicherEnabled $IsIPCameraSwitcherEnabled $IsEncrypt $TrimChannel $Action $PWMCount $ActivateChannel $PrimaryCardMAC
		sleep 1
	fi

    done
}
