## runs on RX (ground pi)
function osdrx_function {
    if [ "$MirrorDSI_To_HDMI" == "y" ]; then
            if [ -e "/dev/fb0" ]; then
                    echo "/dev/fb0 - found"
                    if [ -e "/dev/fb1" ]; then
                            echo "/dev/fb1 - found. Start framebuffer copy"
                            /home/pi/wifibroadcast-misc/raspi2raspi &
                    fi
            fi
    fi
    echo
    # Convert osdconfig from DOS format to UNIX format
    ionice -c 3 nice dos2unix -n /boot/osdconfig.txt /tmp/osdconfig.txt
    echo
	
    cd /home/pi/wifibroadcast-osd
	
    echo Building OSD:
	
    ionice -c 3 nice make -j2 || {
        echo
        echo "ERROR: Could not build OSD, check osdconfig.txt!"
		
		sleep 5
		
        nice /home/pi/wifibroadcast-status/wbc_status "ERROR: Could not build OSD, check osdconfig.txt for errors." 7 55 0
		
		sleep 5
    }
	
    echo

    while true; do
		killall wbc_status > /dev/null 2>&1

		echo -n "Waiting until video is running ..."
		VIDEORXRUNNING=0
		
		while [ $VIDEORXRUNNING -ne 1 ]; do
			sleep 0.5
			VIDEORXRUNNING=`pidof $DISPLAY_PROGRAM | wc -w`
			echo -n "."
		done
		
		echo
		echo "Video running, starting OSD processes ..."

		if [ "$TELEMETRY_TRANSMISSION" == "wbc" ]; then
			echo "Telemetry transmission WBC chosen, using wbc rx"
			TELEMETRY_RX_CMD="/home/pi/wifibroadcast-base/rx_rc_telemetry_buf -p 1 -o 1 -r 99"
		else
			echo "Telemetry transmission external chosen, using cat from serialport"
			nice stty -F $EXTERNAL_TELEMETRY_SERIALPORT_GROUND $EXTERNAL_TELEMETRY_SERIALPORT_GROUND_STTY_OPTIONS $EXTERNAL_TELEMETRY_SERIALPORT_GROUND_BAUDRATE
			#nice /home/pi/wifibroadcast-base/setupuart -d 0 -s $EXTERNAL_TELEMETRY_SERIALPORT_GROUND -b $EXTERNAL_TELEMETRY_SERIALPORT_GROUND_BAUDRATE
			TELEMETRY_RX_CMD="cat $EXTERNAL_TELEMETRY_SERIALPORT_GROUND"
		fi

		if [ "$ENABLE_SERIAL_TELEMETRY_OUTPUT" == "Y" ]; then
			echo "enable_serial_telemetry_output is Y, sending telemetry stream to $TELEMETRY_OUTPUT_SERIALPORT_GROUND"
			nice stty -F $TELEMETRY_OUTPUT_SERIALPORT_GROUND $TELEMETRY_OUTPUT_SERIALPORT_GROUND_STTY_OPTIONS $TELEMETRY_OUTPUT_SERIALPORT_GROUND_BAUDRATE
			#nice /home/pi/wifibroadcast-base/setupuart -d 1 -s $TELEMETRY_OUTPUT_SERIALPORT_GROUND -b $TELEMETRY_OUTPUT_SERIALPORT_GROUND_BAUDRATE
			nice cat /root/telemetryfifo6 > $TELEMETRY_OUTPUT_SERIALPORT_GROUND &
		fi

		# telemetryfifo1: local display, osd
		# telemetryfifo2: secondary display, hotspot/usb-tethering
		# telemetryfifo3: recording
		# telemetryfifo4: wbc relay
		# telemetryfifo5: mavproxy downlink
		# telemetryfifo6: serial downlink

		ionice -c 3 nice cat /root/telemetryfifo3 >> /wbc_tmp/telemetrydowntmp.raw &
		pause_while
		/tmp/osd >> /wbc_tmp/telemetrydowntmp.txt &

		if [ "$RELAY" == "Y" ]; then
			ionice -c 1 -n 4 nice -n -9 cat /root/telemetryfifo4 | nice /home/pi/wifibroadcast-base/tx_telemetry -p 1 -c $TELEMETRY_CTS -r 2 -x $TELEMETRY_TYPE -d 12 -y 0 relay0 > /dev/null 2>&1 &
		fi

		# update NICS variable in case a NIC has been removed (exclude devices with wlanx)
		NICS=`ls /sys/class/net/ | nice grep -v eth0 | nice grep -v lo | nice grep -v usb | nice grep -v intwifi | nice grep -v wlan | nice grep -v relay | nice grep -v wifihotspot`

		if [ "$TELEMETRY_TRANSMISSION" == "wbc" ]; then
			if [ "$DEBUG" == "Y" ]; then
				$TELEMETRY_RX_CMD -d 1 $NICS 2>/wbc_tmp/telemetrydowndebug.txt | tee >(/home/pi/wifibroadcast-misc/ftee /root/telemetryfifo2 > /dev/null 2>&1) >(/home/pi/wifibroadcast-misc/ftee /root/telemetryfifo3 > /dev/null 2>&1) >(ionice -c 1 nice -n -9 /home/pi/wifibroadcast-misc/ftee /root/telemetryfifo4 > /dev/null 2>&1) >(ionice nice /home/pi/wifibroadcast-misc/ftee /root/telemetryfifo5 > /dev/null 2>&1) >(ionice nice /home/pi/wifibroadcast-misc/ftee /root/telemetryfifo6 > /dev/null 2>&1) | /home/pi/wifibroadcast-misc/ftee /root/telemetryfifo1 > /dev/null 2>&1
			else
				nice -n -5 $TELEMETRY_RX_CMD $NICS | tee >(/home/pi/wifibroadcast-misc/ftee /root/telemetryfifo2 > /dev/null 2>&1) >(/home/pi/wifibroadcast-misc/ftee /root/telemetryfifo3 > /dev/null 2>&1) >(ionice -c 1 nice -n -9 /home/pi/wifibroadcast-misc/ftee /root/telemetryfifo4 > /dev/null 2>&1) >(ionice nice /home/pi/wifibroadcast-misc/ftee /root/telemetryfifo5 > /dev/null 2>&1) >(ionice nice /home/pi/wifibroadcast-misc/ftee /root/telemetryfifo6 > /dev/null 2>&1) | /home/pi/wifibroadcast-misc/ftee /root/telemetryfifo1 > /dev/null 2>&1
			fi
		else
			$TELEMETRY_RX_CMD | tee >(/home/pi/wifibroadcast-misc/ftee /root/telemetryfifo2 > /dev/null 2>&1) >(/home/pi/wifibroadcast-misc/ftee /root/telemetryfifo3 > /dev/null 2>&1) >(ionice -c 1 nice -n -9 /home/pi/wifibroadcast-misc/ftee /root/telemetryfifo4 > /dev/null 2>&1) >(ionice nice /home/pi/wifibroadcast-misc/ftee /root/telemetryfifo5 > /dev/null 2>&1) >(ionice nice /home/pi/wifibroadcast-misc/ftee /root/telemetryfifo6 > /dev/null 2>&1) | /home/pi/wifibroadcast-misc/ftee /root/telemetryfifo1 > /dev/null 2>&1
		fi
		
		echo "ERROR: Telemetry RX has been stopped - restarting RX and OSD ..."
		ps -ef | nice grep "rx -p 1" | nice grep -v grep | awk '{print $2}' | xargs kill -9
		ps -ef | nice grep "ftee /root/telemetryfifo" | nice grep -v grep | awk '{print $2}' | xargs kill -9
		ps -ef | nice grep "/tmp/osd" | nice grep -v grep | awk '{print $2}' | xargs kill -9
		ps -ef | nice grep "cat /root/telemetryfifo3" | nice grep -v grep | awk '{print $2}' | xargs kill -9
		sleep 1
    done
}
