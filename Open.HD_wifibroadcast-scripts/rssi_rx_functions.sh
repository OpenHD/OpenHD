function MAIN_RSSI_RX_FUNCTION {
	echo "================== RSSIRX (tty4) ==========================="
	if [ "$CAM" == "0" ]; then
		rssirx_function
	else
		echo "We are TX - rssirx not started"
	fi
	sleep 365d
}

function rssirx_function {
    echo
    echo -n "Waiting until video is running ..."
	
    VIDEORXRUNNING=0
	
    while [ $VIDEORXRUNNING -ne 1 ]; do
        sleep 0.5
        VIDEORXRUNNING=`pidof $DISPLAY_PROGRAM | wc -w`
        echo -n "."
    done
	
    echo
    
	# get NICS (exclude devices with wlanx)
    NICS=`ls /sys/class/net/ | nice grep -v eth0 | nice grep -v lo | nice grep -v usb | nice grep -v intwifi | nice grep -v wlan | nice grep -v relay | nice grep -v wifihotspot`
    
	echo "Starting RSSI RX ..."
    
	nice /home/pi/wifibroadcast-base/rssirx $NICS
}
