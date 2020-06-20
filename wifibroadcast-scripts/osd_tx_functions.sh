## runs on TX (air pi)
function osdtx_function {

    #
    # Setup serial port
    #
    stty -F ${FC_TELEMETRY_SERIALPORT} ${FC_TELEMETRY_STTY_OPTIONS} ${FC_TELEMETRY_BAUDRATE}

    echo
    
    echo -n "Waiting until nics are configured ..."
    
    while [ ! -f /tmp/nics_configured ]; do
        sleep 0.5
    done
    

    sleep 1

    NICS=`ls /sys/class/net/ | nice grep -v eth0 | nice grep -v lo | nice grep -v usb | nice grep -v intwifi | nice grep -v wlan | nice grep -v relay | nice grep -v wifihotspot | nice grep -v zt* | nice grep -v eth1`

    if [ "${TELEMETRY_CTS}" == "1" ]; then
        echo "Telemetry CTS: enabled"
        qstatus "Telemetry CTS: enabled" 5
    else
        echo "Telemetry CTS: disabled"
        qstatus "Telemetry CTS: disabled" 5
    fi



    echo
    while true; do
        echo "Starting telemetry TX @${FC_TELEMETRY_BAUDRATE} baud"
        if [ "${TELEMETRY_TYPE}" == "0" ]; then        
            TELEMETRY_DOWNLINK="mavlink"
        else
            TELEMETRY_DOWNLINK="non-mavlink"
        fi

        qstatus "Starting ${TELEMETRY_DOWNLINK} telemetry downlink @${FC_TELEMETRY_BAUDRATE} baud" 5


        nice cat ${FC_TELEMETRY_SERIALPORT} | nice /usr/local/bin/RCParseCh ${ChannelToListen} | nice /usr/local/bin/tx_telemetry -p 1 -c ${TELEMETRY_CTS} -r 2 -x ${TELEMETRY_TYPE} -d 12 -y 0 ${NICS}
        
        ps -ef | nice grep "cat ${FC_TELEMETRY_SERIALPORT}" | nice grep -v grep | awk '{print $2}' | xargs kill -9
        ps -ef | nice grep "tx_telemetry -p 1" | nice grep -v grep | awk '{print $2}' | xargs kill -9
        ps -ef | nice grep "RCParseCh" | nice grep -v grep | awk '{print $2}' | xargs kill -9

        sleep 1
    done
}

