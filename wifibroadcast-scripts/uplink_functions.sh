

function MAIN_UPLINK_FUNCTION {

    echo "================== uplink tx rx / rc rx / msp rx / (tty10) ==========================="
    
    # 
    # TODO: this is a magic number, and possibly a race condition sleeping for an arbitrary amount
    #       of time
    # 
    sleep 7

    if [ "${CAM}" -ge 1 ]; then
        # 
        # Air side
        # 

        #
        # Start the microservice channel and the microservices that run on the air side
        #
        microservice_air_rx_function &
        microservice_air_tx_function &

        #
        # Start the telemetry and RC receiver (downlink telemetry is started in another area)
        #
        if [ "$TELEMETRY_UPLINK" != "disabled" ] || [ "$RC" != "disabled" ]; then
            uplinkrx_and_rcrx_function &
            
            if [ "$TELEMETRY_UPLINK" == "msp" ]; then
                mspdownlinktx_function
            fi
            
            sleep 365d
        else
            echo "Telemetry uplink/RC disabled"
            qstatus "Telemetry uplink/RC disabled" 5
        fi
        
        sleep 365d
    else
        # 
        # Ground side
        # 

        #
        # Start the microservice channel and the microservices that run on the ground side
        #
        microservice_ground_rx_function &
        microservice_ground_tx_function &

        #
        # Start the telemetry uplink
        #
        if [ "$TELEMETRY_UPLINK" != "disabled" ]; then
            echo "Telemetry uplink enabled"
            qstatus "Telemetry uplink enabled" 5

            uplinktx_function &
            
            if [ "$TELEMETRY_UPLINK" == "msp" ]; then
                mspdownlinkrx_function
            fi
            
            sleep 365d
        else
            echo "Telemetry uplink disabled"
            qstatus "Telemetry uplink disabled" 5
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


    #
    # Configure the flight controller serial port
    #
    stty -F $FC_TELEMETRY_SERIALPORT $FC_TELEMETRY_STTY_OPTIONS $FC_TELEMETRY_BAUDRATE

    echo "Starting ${TELEMETRY_UPLINK} telemetry uplink @${FC_TELEMETRY_BAUDRATE} baud"
    echo "Starting ${RC} RC RX"

    qstatus "Starting ${TELEMETRY_UPLINK} telemetry uplink @${FC_TELEMETRY_BAUDRATE} baud" 5
    qstatus "Starting ${RC} RC RX"
    

    if [ "$EncryptionOrRange" == "Encryption" ]; then
        RC="disabled"
    fi
    


    if [ "$RC" != "disabled" ]; then
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


        if [ "$FC_TELEMETRY_SERIALPORT" == "$FC_RC_SERIALPORT" ]; then
            #
            # TODO: check if this logic works in all cases
            #

            if [ "$TELEMETRY_UPLINK" == "mavlink" ]; then
                #
                # Use the telemetry serial port and baudrate, as it's the same anyway
                #
                /home/pi/wifibroadcast-base/rx_rc_telemetry -p 3 -o 0 -b $FC_TELEMETRY_BAUDRATE -s $FC_TELEMETRY_SERIALPORT -r $RC_PROTOCOL $NICS
            else
                #
                # Use the configured r/c serialport and baudrate
                #
                
                /home/pi/wifibroadcast-base/rx_rc_telemetry -p 3 -o 0 -b $FC_RC_BAUDRATE -s $FC_RC_SERIALPORT -r $RC_PROTOCOL $NICS
            fi
        else
            /home/pi/wifibroadcast-base/rx_rc_telemetry -p 3 -o 1 -b $FC_RC_BAUDRATE -s $FC_RC_SERIALPORT -r $RC_PROTOCOL $NICS > $FC_TELEMETRY_SERIALPORT
        fi
    else
        #
        # No RC, just telemetry
        #
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

function microservice_ground_tx_function {
    while true; do
        echo "Starting ground microservice tx"
        
        NICS=`ls /sys/class/net/ | nice grep -v eth0 | nice grep -v lo | nice grep -v usb | nice grep -v intwifi | nice grep -v relay | nice grep -v wifihotspot`
        
        OPENHD_MICROSERVICE_GROUND_TX_CMD="nice /home/pi/wifibroadcast-base/tx_telemetry -p 30 -c 0 -r 2 -x 0 -d 12 -y 0"
        
        nice socat -u /dev/openhd_microservice2 STDOUT | $OPENHD_MICROSERVICE_GROUND_TX_CMD $NICS
        ps -ef | nice grep "socat -u /dev/openhd_microservice2 STDOUT" | nice grep -v grep | awk '{print $2}' | xargs kill -9
        ps -ef | nice grep "tx_telemetry -p 30" | nice grep -v grep | awk '{print $2}' | xargs kill -9
        sleep 1
    done
}

function microservice_ground_rx_function {
    while true; do
        echo "Starting ground microservice rx"
        
        NICS=`ls /sys/class/net/ | nice grep -v eth0 | nice grep -v lo | nice grep -v usb | nice grep -v intwifi | nice grep -v relay | nice grep -v wifihotspot`
        
        nice /home/pi/wifibroadcast-base/rx_rc_telemetry_buf -n 1 -p 31 -o 1 -r 99 -b 115200 -s /dev/null $NICS | ionice nice socat -u STDIN /dev/openhd_microservice2
        
        ps -ef | nice grep "nice socat -u STDIN /dev/openhd_microservice2" | nice grep -v grep | awk '{print $2}' | xargs kill -9
        ps -ef | nice grep "rx_rc_telemetry -n 1 -p 31" | nice grep -v grep | awk '{print $2}' | xargs kill -9
        sleep 1
    done
}

function microservice_air_tx_function {
    while true; do
        echo "Starting air microservice tx"
        
        NICS=`ls /sys/class/net/ | nice grep -v eth0 | nice grep -v lo | nice grep -v usb | nice grep -v intwifi | nice grep -v relay | nice grep -v wifihotspot`
        
        OPENHD_MICROSERVICE_AIR_TX_CMD="nice /home/pi/wifibroadcast-base/tx_telemetry -p 31 -c 0 -r 2 -x 0 -d 12 -y 0"
        
        nice socat -u /dev/openhd_microservice2 STDOUT | $OPENHD_MICROSERVICE_AIR_TX_CMD $NICS

        ps -ef | nice grep "socat -u /dev/openhd_microservice2 STDOUT" | nice grep -v grep | awk '{print $2}' | xargs kill -9
        ps -ef | nice grep "tx_telemetry -p 31" | nice grep -v grep | awk '{print $2}' | xargs kill -9
        sleep 1
    done
}

function microservice_air_rx_function {
    while true; do
        echo "Starting air microservice rx"
        
        NICS=`ls /sys/class/net/ | nice grep -v eth0 | nice grep -v lo | nice grep -v usb | nice grep -v intwifi | nice grep -v relay | nice grep -v wifihotspot`
        
        nice /home/pi/wifibroadcast-base/rx_rc_telemetry_buf -n 1 -p 30 -o 1 -r 99 -b 115200 -s /dev/null $NICS |  ionice nice socat -u STDIN /dev/openhd_microservice2
        
        ps -ef | nice grep "nice socat -u STDIN /dev/openhd_microservice2" | nice grep -v grep | awk '{print $2}' | xargs kill -9
        ps -ef | nice grep "rx_rc_telemetry -n 1 -p 30" | nice grep -v grep | awk '{print $2}' | xargs kill -9
        sleep 1
    done
}

## runs on RX (ground pi)
function uplinktx_function {
    #
    # Wait until video is running to make sure NICS are configured
    #
    # TODO: make this directly check the NIC status instead of the video
    #
    echo

    echo -n "Waiting until video is running ..."

    VIDEORXRUNNING=0
    while [ $VIDEORXRUNNING -ne 1 ]; do
        VIDEORXRUNNING=`pidof $DISPLAY_PROGRAM | wc -w`
        sleep 1
        echo -n "."
    done

    
    sleep 1
    
    echo
    echo

    while true; do
        echo "Starting ${TELEMETRY_UPLINK} telemetry uplink"
        qstatus "Starting ${TELEMETRY_UPLINK} telemetry uplink" 5
        
        if [ "$TELEMETRY_TRANSMISSION" == "wbc" ]; then
            echo "telemetry transmission = wbc, starting tx_telemetry ..."

            NICS=`ls /sys/class/net/ | nice grep -v eth0 | nice grep -v lo | nice grep -v usb | nice grep -v intwifi | nice grep -v relay | nice grep -v wifihotspot`

            echo -n "NICS:"
            echo $NICS

            if [ "$TELEMETRY_UPLINK" == "mavlink" ]; then
                #
                # For Mavlink telemetry, tx_telemetry parses the messages to find the boundary and separate them for 
                # transmission, to make it more likely they will get through.
                #
                # We can and should do the same for the other protocols, but we don't yet
                #
                VSERIALPORT=/dev/openhd_mavlink1
                UPLINK_TX_CMD="nice /home/pi/wifibroadcast-base/tx_telemetry -p 3 -c 0 -r 2 -x 0 -d 12 -y 0"
            else
                #
                # Non-mavlink telemetry is all handled the same
                # 
                VSERIALPORT=/dev/openhd_msp1
                UPLINK_TX_CMD="nice /home/pi/wifibroadcast-base/tx_telemetry -p 3 -c 0 -r 2 -x 1 -d 12 -y 0"
            fi

            # 
            # This starts the telemetry transmission by taking all data received from the fake serial port (connected to
            # mavlink-router) and piping it to the tx_telemetry command we configured a few lines earlier
            #
            # TODO: document how the connection is made for non-mavlink protocols
            #
            if [ "$DEBUG" == "Y" ]; then
                nice cat $VSERIALPORT | $UPLINK_TX_CMD -z 1 $NICS 2>/wbc_tmp/telemetryupdebug.txt
            else
                nice cat $VSERIALPORT | $UPLINK_TX_CMD $NICS
            fi
        else
            echo "telemetry transmission = external, sending data to $EXTERNAL_TELEMETRY_SERIALPORT_GROUND ..."
            
            nice stty -F $EXTERNAL_TELEMETRY_SERIALPORT_GROUND $EXTERNAL_TELEMETRY_SERIALPORT_GROUND_STTY_OPTIONS $EXTERNAL_TELEMETRY_SERIALPORT_GROUND_BAUDRATE
            
            if [ "$TELEMETRY_UPLINK" == "mavlink" ]; then
                VSERIALPORT=/dev/openhd_mavlink1
            else
                VSERIALPORT=/dev/openhd_msp1
            fi
            
            UPLINK_TX_CMD="$EXTERNAL_TELEMETRY_SERIALPORT_GROUND"
 
            #
            # This starts the telemetry transmission by taking all data received from the ground serial port
            # and piping it to the tx_telemetry command we configured a few lines earlier
            #
            if [ "$DEBUG" == "Y" ]; then
                nice cat $VSERIALPORT > $UPLINK_TX_CMD
            else
                nice cat $VSERIALPORT > $UPLINK_TX_CMD
            fi
        fi
        
        # 
        # If we reach this point, one of the processes has died and we need to kill the other one so we can
        # restart the telemetry stream. This does not seem to be an issue in practice, the processes are quite stable
        #
        ps -ef | nice grep "cat $VSERIALPORT" | nice grep -v grep | awk '{print $2}' | xargs kill -9
        ps -ef | nice grep "tx_telemetry -p 3" | nice grep -v grep | awk '{print $2}' | xargs kill -9
    done
}


# runs on RX (ground pi)
function mspdownlinkrx_function {
    echo

    echo -n "Waiting until video is running ..."

    VIDEORXRUNNING=0
    while [ $VIDEORXRUNNING -ne 1 ]; do
        sleep 0.5
        VIDEORXRUNNING=`pidof $DISPLAY_PROGRAM | wc -w`
        echo -n "."
    done
    
    
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
