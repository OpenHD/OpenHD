function MAIN_UPLINK_FUNCTION {
	echo "================== uplink tx rx / rc rx / msp rx / (tty10) ==========================="
	
	sleep 7
	
	if [ "$CAM" == "1" ]; then # we are video TX and uplink RX
	    if [ "$TELEMETRY_UPLINK" != "disabled" ] || [ "$RC" != "disabled" ]; then
			echo "Uplink and/or R/C enabled ... we are RX"
			uplinkrx_and_rcrx_function &
			
			if [ "$TELEMETRY_UPLINK" == "msp" ]; then
				mspdownlinktx_function
			fi
			
			sleep 365d
		else
			echo "uplink and R/C not enabled in config"
	    fi
		
	    sleep 365d
	else # we are video RX and uplink TX
	    if [ "$TELEMETRY_UPLINK" != "disabled" ]; then
			echo "uplink  enabled ... we are uplink TX"
			uplinktx_function &
			
			if [ "$TELEMETRY_UPLINK" == "msp" ]; then
				mspdownlinkrx_function
			fi
			
			sleep 365d
	    else
			echo "uplink not enabled in config"
	    fi
		
	    sleep 365d
	fi
}

## runs on TX (air pi)
function uplinkrx_and_rcrx_function {
    echo "FC_TELEMETRY_SERIALPORT: $FC_TELEMETRY_SERIALPORT"
    echo "FC_MSP_SERIALPORT: $FC_MSP_SERIALPORT"
    echo "FC_RC_SERIALPORT: $FC_RC_SERIALPORT"
    echo
    echo -n "Waiting until nics are configured ..."
	
    while [ ! -f /tmp/nics_configured ]; do
		sleep 0.5
		#echo -n "."
    done
	
    sleep 1

    NICS=`ls /sys/class/net/ | nice grep -v eth0 | nice grep -v lo | nice grep -v usb | nice grep -v intwifi | nice grep -v relay | nice grep -v wifihotspot`
    echo -n "NICS:"
    echo $NICS
    echo

    stty -F $FC_TELEMETRY_SERIALPORT $FC_TELEMETRY_STTY_OPTIONS $FC_TELEMETRY_BAUDRATE

    echo "Starting Uplink telemetry and R/C RX ..."
    
    if [ "$EncryptionOrRange" == "Encryption" ]; then
   	RC="disabled"
    fi
	
    if [ "$RC" != "disabled" ]; then # with R/C
		case $RC in
			"msp")
			RC_PROTOCOL=0
			;;
			"mavlink")
			RC_PROTOCOL=1
			;;
			"sumd")
			RC_PROTOCOL=2
			;;
			"ibus")
			RC_PROTOCOL=3
			;;
			"srxl")
			RC_PROTOCOL=4
			;;
		esac
		if [ "$FC_TELEMETRY_SERIALPORT" == "$FC_RC_SERIALPORT" ]; then # TODO: check if this logic works in all cases
			if [ "$TELEMETRY_UPLINK" == "mavlink" ]; then # use the telemetry serialport and baudrate as it's the same anyway
				/home/pi/wifibroadcast-base/rx_rc_telemetry -p 3 -o 0 -b $FC_TELEMETRY_BAUDRATE -s $FC_TELEMETRY_SERIALPORT -r $RC_PROTOCOL $NICS
			else # use the configured r/c serialport and baudrate
				/home/pi/wifibroadcast-base/rx_rc_telemetry -p 3 -o 0 -b $FC_RC_BAUDRATE -s $FC_RC_SERIALPORT -r $RC_PROTOCOL $NICS
			fi
		else
			#/home/pi/wifibroadcast-base/setupuart -d 1 -s $FC_TELEMETRY_SERIALPORT -b $FC_TELEMETRY_BAUDRATE
			/home/pi/wifibroadcast-base/rx_rc_telemetry -p 3 -o 1 -b $FC_RC_BAUDRATE -s $FC_RC_SERIALPORT -r $RC_PROTOCOL $NICS > $FC_TELEMETRY_SERIALPORT
		fi
    else # without R/C
		#/home/pi/wifibroadcast-base/setupuart -d 1 -s $FC_TELEMETRY_SERIALPORT -b $FC_TELEMETRY_BAUDRATE
		nice /home/pi/wifibroadcast-base/rx_rc_telemetry  -p 3 -o 0 -b $FC_TELEMETRY_BAUDRATE -s $FC_TELEMETRY_SERIALPORT -r 98 $NICS
    fi
}


## runs on TX (air pi)
function mspdownlinktx_function {
    # disabled for now
    sleep 365d
    # setup serial port
    stty -F $FC_MSP_SERIALPORT -imaxbel -opost -isig -icanon -echo -echoe -ixoff -ixon $FC_MSP_BAUDRATE
    #/home/pi/wifibroadcast-base/setupuart -d 0 -s $FC_MSP_SERIALPORT -b $FC_MSP_BAUDRATE

    # wait until tx is running to make sure NICS are configured
    echo
    echo -n "Waiting until video TX is running ..."
    VIDEOTXRUNNING=0

	if [ "$ENABLE_OPENHDVID" == "Y" ]; then
		CAMERA_PROGRAM="openhdvid"
	else
		CAMERA_PROGRAM="raspivid"
	fi
	
    while [ $VIDEOTXRUNNING -ne 1 ]; do
		sleep 0.5
		VIDEOTXRUNNING=`ps -ef | nice grep "${CAMERA_PROGRAM}" | nice grep -v grep | awk '{print $2}' | wc -w`
		echo -n "."
    done
	
    echo

    echo "Video running, starting MSP processes ..."

    NICS=`ls /sys/class/net/ | nice grep -v eth0 | nice grep -v lo | nice grep -v usb | nice grep -v intwifi`

    echo
	
    while true; do
        echo "Starting MSP transmission, FC MSP Serialport: $FC_MSP_SERIALPORT"
		nice cat $FC_MSP_SERIALPORT | nice /home/pi/wifibroadcast-base/tx_telemetry -p 4 -c $TELEMETRY_CTS -r 2 -x 1 -d 12 -y 0 $NICS
        ps -ef | nice grep "cat $FC_MSP_SERIALPORT" | nice grep -v grep | awk '{print $2}' | xargs kill -9
        ps -ef | nice grep "tx_telemetry -p 4" | nice grep -v grep | awk '{print $2}' | xargs kill -9
	echo "MSP telemetry TX exited - restarting ..."
        sleep 1
    done
}


## runs on RX (ground pi)
function uplinktx_function {
    # wait until video is running to make sure NICS are configured
    echo
	if [ "$ENABLE_QOPENHD" != "Y" ]; then
		echo -n "Waiting until video is running ..."
		VIDEORXRUNNING=0
		while [ $VIDEORXRUNNING -ne 1 ]; do
			VIDEORXRUNNING=`pidof $DISPLAY_PROGRAM | wc -w`
			sleep 1
			echo -n "."
		done
	fi
	
    sleep 1
	
    echo
    echo

    while true; do
        echo "Starting uplink telemetry transmission"
		
		if [ "$TELEMETRY_TRANSMISSION" == "wbc" ]; then
			echo "telemetry transmission = wbc, starting tx_telemetry ..."
			NICS=`ls /sys/class/net/ | nice grep -v eth0 | nice grep -v lo | nice grep -v usb | nice grep -v intwifi | nice grep -v relay | nice grep -v wifihotspot`
			echo -n "NICS:"
			echo $NICS
			if [ "$TELEMETRY_UPLINK" == "mavlink" ]; then
				VSERIALPORT=/dev/pts/0
				UPLINK_TX_CMD="nice /home/pi/wifibroadcast-base/tx_telemetry -p 3 -c 0 -r 2 -x 0 -d 12 -y 0"
			else # MSP
				VSERIALPORT=/dev/pts/2
				UPLINK_TX_CMD="nice /home/pi/wifibroadcast-base/tx_telemetry -p 3 -c 0 -r 2 -x 1 -d 12 -y 0"
			fi

			if [ "$DEBUG" == "Y" ]; then
				nice cat $VSERIALPORT | $UPLINK_TX_CMD -z 1 $NICS 2>/wbc_tmp/telemetryupdebug.txt
			else
				nice cat $VSERIALPORT | $UPLINK_TX_CMD $NICS
			fi
		else
			echo "telemetry transmission = external, sending data to $EXTERNAL_TELEMETRY_SERIALPORT_GROUND ..."
			nice stty -F $EXTERNAL_TELEMETRY_SERIALPORT_GROUND $EXTERNAL_TELEMETRY_SERIALPORT_GROUND_STTY_OPTIONS $EXTERNAL_TELEMETRY_SERIALPORT_GROUND_BAUDRATE
			if [ "$TELEMETRY_UPLINK" == "mavlink" ]; then
				VSERIALPORT=/dev/pts/0
			else # MSP
				VSERIALPORT=/dev/pts/2
			fi
			
			UPLINK_TX_CMD="$EXTERNAL_TELEMETRY_SERIALPORT_GROUND"
			
			if [ "$DEBUG" == "Y" ]; then
				nice cat $VSERIALPORT > $UPLINK_TX_CMD
			else
				nice cat $VSERIALPORT > $UPLINK_TX_CMD
			fi
		fi
		
    	ps -ef | nice grep "cat $VSERIALPORT" | nice grep -v grep | awk '{print $2}' | xargs kill -9
    	ps -ef | nice grep "tx_telemetry -p 3" | nice grep -v grep | awk '{print $2}' | xargs kill -9
    done
}


# runs on RX (ground pi)
function mspdownlinkrx_function {
    echo
    if [ "$ENABLE_QOPENHD" != "Y" ]; then
		echo -n "Waiting until video is running ..."

		VIDEORXRUNNING=0
		
		while [ $VIDEORXRUNNING -ne 1 ]; do
			sleep 0.5
			VIDEORXRUNNING=`pidof $DISPLAY_PROGRAM | wc -w`
			echo -n "."
		done
	fi
	
    echo
    echo "Video running ..."

    # disabled for now
    sleep 365d
	
    while true; do
		#
		#if [ "$RELAY" == "Y" ]; then
		#    ionice -c 1 -n 4 nice -n -9 cat /root/telemetryfifo4 | /home/pi/wifibroadcast-base/tx_rawsock -p 1 -b $RELAY_TELEMETRY_BLOCKS -r $RELAY_TELEMETRY_FECS -f $RELAY_TELEMETRY_BLOCKLENGTH -m $TELEMETRY_MIN_BLOCKLENGTH -y 0 relay0 > /dev/null 2>&1 &
		#fi
		# update NICS variable in case a NIC has been removed (exclude devices with wlanx)
		NICS=`ls /sys/class/net/ | nice grep -v eth0 | nice grep -v lo | nice grep -v usb | nice grep -v intwifi | nice grep -v wlan | nice grep -v relay | nice grep -v wifihotspot`
		#nice /home/pi/wifibroadcast-base/rx -p 4 -d 1 -b $TELEMETRY_BLOCKS -r $TELEMETRY_FECS -f $TELEMETRY_BLOCKLENGTH $NICS | ionice nice /home/pi/wifibroadcast-misc/ftee /root/mspfifo > /dev/null 2>&1
		echo "Starting msp downlink rx ..."
		nice /home/pi/wifibroadcast-base/rx_rc_telemetry -p 4 -o 1 -r 99 $NICS | ionice nice /home/pi/wifibroadcast-misc/ftee /root/mspfifo > /dev/null 2>&1
		echo "ERROR: MSP RX has been stopped - restarting ..."
		ps -ef | nice grep "rx_rc_telemetry -p 4" | nice grep -v grep | awk '{print $2}' | xargs kill -9
		ps -ef | nice grep "ftee /root/mspfifo" | nice grep -v grep | awk '{print $2}' | xargs kill -9
		sleep 1
    done
}
