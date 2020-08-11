## runs on RX (ground pi)
function osdrx_function {    
    while true; do
        killall wbc_status > /dev/null 2>&1

        echo -n "Waiting until video is running ..."
        VIDEORXRUNNING=0

        while [ ${VIDEORXRUNNING} -ne 1 ]; do
            sleep 0.5
            VIDEORXRUNNING=`pidof $DISPLAY_PROGRAM | wc -w`
            echo -n "."
        done

        
        echo
        echo "Video running, starting OSD processes ..."

        if [ "${TELEMETRY_TRANSMISSION}" == "wbc" ]; then
            echo "Telemetry transmission WBC chosen, using wbc rx"

            TELEMETRY_RX_CMD="/usr/local/bin/rx_rc_telemetry_buf -p 1 -o 1 -r 99"
        else
            echo "Telemetry transmission external chosen, reading from serialport: ${EXTERNAL_TELEMETRY_SERIALPORT_GROUND}"
            
            nice stty -F ${EXTERNAL_TELEMETRY_SERIALPORT_GROUND} ${EXTERNAL_TELEMETRY_SERIALPORT_GROUND_STTY_OPTIONS} ${EXTERNAL_TELEMETRY_SERIALPORT_GROUND_BAUDRATE}
                        
            TELEMETRY_RX_CMD="cat ${EXTERNAL_TELEMETRY_SERIALPORT_GROUND}"
        fi

        if [ "${ENABLE_SERIAL_TELEMETRY_OUTPUT}" == "Y" ]; then
            echo "Sending telemetry stream to ${TELEMETRY_OUTPUT_SERIALPORT_GROUND}"
            
            nice stty -F ${TELEMETRY_OUTPUT_SERIALPORT_GROUND} ${TELEMETRY_OUTPUT_SERIALPORT_GROUND_STTY_OPTIONS} ${TELEMETRY_OUTPUT_SERIALPORT_GROUND_BAUDRATE}
                        
            nice cat /var/run/openhd/telemetryfifo6 > ${TELEMETRY_OUTPUT_SERIALPORT_GROUND} &
        fi

        # telemetryfifo1: local display, osd
        # telemetryfifo2: secondary display, hotspot/usb-tethering
        # telemetryfifo3: recording
        # telemetryfifo4: wbc relay
        # telemetryfifo5: mavproxy downlink
        # telemetryfifo6: serial downlink

        ionice -c 3 nice cat /var/run/openhd/telemetryfifo3 >> /wbc_tmp/telemetrydowntmp.raw &
        pause_while
        sleep 5

        systemctl stop openhdboot

        if [ "${DISPLAY_OSD}" == "Y" ]; then
            if [ "${ENABLE_QOPENHD}" == "Y" ]; then
                systemctl start qopenhd
            else
                systemctl start osd
            fi
        fi

        if [ "${RELAY}" == "Y" ]; then
            ionice -c 1 -n 4 nice -n -9 cat /var/run/openhd/telemetryfifo4 | nice /usr/local/bin/tx_telemetry -p 1 -c ${TELEMETRY_CTS} -r 2 -x ${TELEMETRY_TYPE} -d 12 -y 0 relay0 -f ${FORCE_REALTEK_TELEMETRY_DATA_FRAME} > /dev/null 2>&1 &
        fi

        #
        # update NICS variable in case a NIC has been removed (exclude devices with wlanX)
        #
        NICS=`ls /sys/class/net/ | nice grep -v eth0 | nice grep -v lo | nice grep -v usb | nice grep -v intwifi | nice grep -v wlan | nice grep -v relay | nice grep -v wifihotspot`


        if [ "${TELEMETRY_TRANSMISSION}" == "wbc" ]; then
            if [ "${DEBUG}" == "Y" ]; then
                ${TELEMETRY_RX_CMD} -d 1 ${NICS} 2>/wbc_tmp/telemetrydowndebug.txt | tee >(/usr/local/bin/ftee /var/run/openhd/telemetryfifo2 > /dev/null 2>&1) >(/usr/local/bin/ftee /var/run/openhd/telemetryfifo3 > /dev/null 2>&1) >(ionice -c 1 nice -n -9 /usr/local/bin/ftee /var/run/openhd/telemetryfifo4 > /dev/null 2>&1) >(ionice nice /usr/local/bin/ftee /var/run/openhd/telemetryfifo5 > /dev/null 2>&1) >(ionice nice /usr/local/bin/ftee /var/run/openhd/telemetryfifo6 > /dev/null 2>&1) | /usr/local/bin/ftee /var/run/openhd/telemetryfifo1 > /dev/null 2>&1
            else
                nice -n -5 ${TELEMETRY_RX_CMD} ${NICS} | tee >(/usr/local/bin/ftee /var/run/openhd/telemetryfifo2 > /dev/null 2>&1) >(/usr/local/bin/ftee /var/run/openhd/telemetryfifo3 > /dev/null 2>&1) >(ionice -c 1 nice -n -9 /usr/local/bin/ftee /var/run/openhd/telemetryfifo4 > /dev/null 2>&1) >(ionice nice /usr/local/bin/ftee /var/run/openhd/telemetryfifo5 > /dev/null 2>&1) >(ionice nice /usr/local/bin/ftee /var/run/openhd/telemetryfifo6 > /dev/null 2>&1) | /usr/local/bin/ftee /var/run/openhd/telemetryfifo1 > /dev/null 2>&1
            fi
        else
            ${TELEMETRY_RX_CMD} | tee >(/usr/local/bin/ftee /var/run/openhd/telemetryfifo2 > /dev/null 2>&1) >(/usr/local/bin/ftee /var/run/openhd/telemetryfifo3 > /dev/null 2>&1) >(ionice -c 1 nice -n -9 /usr/local/bin/ftee /var/run/openhd/telemetryfifo4 > /dev/null 2>&1) >(ionice nice /usr/local/bin/ftee /var/run/openhd/telemetryfifo5 > /dev/null 2>&1) >(ionice nice /usr/local/bin/ftee /var/run/openhd/telemetryfifo6 > /dev/null 2>&1) | /usr/local/bin/ftee /var/run/openhd/telemetryfifo1 > /dev/null 2>&1
        fi

        
        echo "ERROR: Telemetry RX has been stopped - restarting RX ..."
        ps -ef | nice grep "rx -p 1" | nice grep -v grep | awk '{print $2}' | xargs kill -9
        ps -ef | nice grep "ftee /var/run/openhd/telemetryfifo" | nice grep -v grep | awk '{print $2}' | xargs kill -9
        ps -ef | nice grep "cat /var/run/openhd/telemetryfifo3" | nice grep -v grep | awk '{print $2}' | xargs kill -9
        sleep 1
    done
}
