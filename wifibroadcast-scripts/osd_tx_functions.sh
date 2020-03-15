## runs on TX (air pi)
function osdtx_function {
    # setup serial port
    stty -F $FC_TELEMETRY_SERIALPORT $FC_TELEMETRY_STTY_OPTIONS $FC_TELEMETRY_BAUDRATE

    echo
	
    echo -n "Waiting until nics are configured ..."
	
    while [ ! -f /tmp/nics_configured ]; do
		sleep 0.5
		#echo -n "."
    done
	
    sleep 1
	
    echo
    echo "nics configured, starting Downlink telemetry TX processes ..."

    NICS=`ls /sys/class/net/ | nice grep -v eth0 | nice grep -v lo | nice grep -v usb | nice grep -v intwifi | nice grep -v wlan | nice grep -v relay | nice grep -v wifihotspot | nice grep -v zt* | nice grep -v eth1`

    echo "telemetry CTS: $TELEMETRY_CTS"

    echo
    while true; do
        echo "Starting downlink telemetry transmission in $TXMODE mode (FC Serialport: $FC_TELEMETRY_SERIALPORT)"
        nice cat $FC_TELEMETRY_SERIALPORT | nice /home/pi/cameracontrol/RCParseCh $ChannelToListen | nice /home/pi/wifibroadcast-base/tx_telemetry -p 1 -c $TELEMETRY_CTS -r 2 -x $TELEMETRY_TYPE -d 12 -y 0 $NICS
        ps -ef | nice grep "cat $FC_TELEMETRY_SERIALPORT" | nice grep -v grep | awk '{print $2}' | xargs kill -9
        ps -ef | nice grep "tx_telemetry -p 1" | nice grep -v grep | awk '{print $2}' | xargs kill -9
	ps -ef | nice grep "RCParseCh" | nice grep -v grep | awk '{print $2}' | xargs kill -9
	
		echo "Downlink Telemetry TX exited - restarting ..."
		
        sleep 1
    done
}

