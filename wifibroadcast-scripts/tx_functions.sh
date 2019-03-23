function tx_function {
    killall wbc_status > /dev/null 2>&1

    if [ "$DEBUG" == "Y" ]; then
	#Turn text output back on for Airpi.
	con2fbmap 1 0
    fi


    /home/pi/wifibroadcast-base/sharedmem_init_tx

    if [ "$TXMODE" == "single" ]; then
		echo -n "Waiting for wifi card to become ready ..."
		COUNTER=0
		# loop until card is initialized
		while [ $COUNTER -lt 10 ]; do
				sleep 0.5
				echo -n "."
			let "COUNTER++"
				if [ -d "/sys/class/net/wlan0" ]; then
			echo -n "card ready"
			break
				fi
		done
    else
		# just wait some time
		echo -n "Waiting for wifi cards to become ready ..."
		sleep 3
    fi

    echo
    echo
    dmesg -c >/dev/null 2>/dev/null
    detect_nics

    sleep 1
    echo

    if [ -e "$FC_TELEMETRY_SERIALPORT" ]; then
		echo "Configured serial port $FC_TELEMETRY_SERIALPORT found ..."
		else
		echo "ERROR: $FC_TELEMETRY_SERIALPORT not found!"
		collect_errorlog
		sleep 365d
    fi
	
    echo

    RALINK=0

    if [ "$TXMODE" == "single" ]; then
		DRIVER=`cat /sys/class/net/$NICS/device/uevent | nice grep DRIVER | sed 's/DRIVER=//'`

	        case $DRIVER in
	            *881[24]au)
	                DRIVER=rtl88xxau
	            ;;
	        esac

		if [ "$DRIVER" != "ath9k_htc" ]; then # in single mode and ralink cards always, use frametype 1 (data)
			VIDEO_FRAMETYPE=0
		        if [ "$DRIVER" == "rtl88xxau" ]; then
		        	VIDEO_FRAMETYPE=1
			fi
			RALINK=1
		fi
    else # for txmode dual always use frametype 1
		VIDEO_FRAMETYPE=1
		RALINK=1
    fi

    if [ "$VIDEO_WIFI_BITRATE" == "19.5" ]; then # set back to 18 to make sure -d parameter works (supports only 802.11b/g datarates)
		VIDEO_WIFI_BITRATE=18
    fi
    if [ "$VIDEO_WIFI_BITRATE" == "5.5" ]; then # set back to 6 to make sure -d parameter works (supports only 802.11b/g datarates)
		VIDEO_WIFI_BITRATE=5
    fi

    DRIVER=`cat /sys/class/net/$NICS/device/uevent | nice grep DRIVER | sed 's/DRIVER=//'`

    case $DRIVER in
        *881[24]au)
            DRIVER=rtl88xxau
        ;;
    esac

    if [ "$CTS_PROTECTION" == "auto" ] && [ "$DRIVER" == "ath9k_htc" ]; then # only use CTS protection with Atheros
    	echo -n "Checking for other wifi traffic ... "
		WIFIPPS=`/home/pi/wifibroadcast-base/wifiscan $NICS`
		echo -n "$WIFIPPS PPS: "
		if [ "$WIFIPPS" != "0" ]; then # wifi networks detected, enable CTS
			echo "Wifi traffic detected, CTS enabled"
			VIDEO_FRAMETYPE=1
			TELEMETRY_CTS=1
			CTS=Y
		else
			echo "No wifi traffic detected, CTS disabled"
			CTS=N
		fi
    else
		if [ "$CTS_PROTECTION" == "N" ]; then
			echo "CTS Protection disabled in config"
			CTS=N
		else
			if [ "$DRIVER" == "ath9k_htc" ]; then
				echo "CTS Protection enabled in config"
				CTS=Y
			else
				echo "CTS Protection not supported!"
				CTS=N
			fi
		fi
    fi

    # check if over-temp or under-voltage occured before bitrate measuring
    if vcgencmd get_throttled | nice grep -q -v "0x0"; then
		TEMP=`cat /sys/class/thermal/thermal_zone0/temp`
		TEMP_C=$(($TEMP/1000))
		
		if [ "$TEMP_C" -lt 75 ]; then # it must be under-voltage
			echo
			echo "  ---------------------------------------------------------------------------------------------------"
			echo "  | ERROR: Under-Voltage detected on the TX Pi. Your Pi is not supplied with stable 5 Volts.        |"
			echo "  | Either your power-supply or wiring is not sufficent, check the wiring instructions in the Wiki. |"
			echo "  | Video Bitrate will be reduced to 1000kbit to reduce current consumption!                        |"
			echo "  ---------------------------------------------------------------------------------------------------"
			echo
			mount -o remount,rw /boot
			echo "  ---------------------------------------------------------------------------------------------------" >> /boot/UNDERVOLTAGE-ERROR!!!.txt
			echo "  | ERROR: Under-Voltage detected on the TX Pi. Your Pi is not supplied with stable 5 Volts.        |" >> /boot/UNDERVOLTAGE-ERROR!!!.txt
			echo "  | Either your power-supply or wiring is not sufficent, check the wiring instructions in the Wiki. |" >> /boot/UNDERVOLTAGE-ERROR!!!.txt
			echo "  | Video Bitrate will be reduced to 1000kbit to reduce current consumption!                        |" >> /boot/UNDERVOLTAGE-ERROR!!!.txt
			echo "  | When you have fixed wiring/power-supply, delete this file and make sure it doesn't re-appear!   |" >> /boot/UNDERVOLTAGE-ERROR!!!.txt
			echo "  ---------------------------------------------------------------------------------------------------" >> /boot/UNDERVOLTAGE-ERROR!!!.txt
			mount -o remount,ro /boot
			UNDERVOLT=1
			echo "1" > /tmp/undervolt
		else # it was either over-temp or both undervolt and over-temp, we set undervolt to 0 anyway, since overtemp can be seen at the temp display on the rx
			UNDERVOLT=0
			echo "0" > /tmp/undervolt
		fi
    else
		UNDERVOLT=0
		echo "0" > /tmp/undervolt
    fi

    # if yes, we don't do the bitrate measuring to increase chances we "survive"
	if [ "$UNDERVOLT" == "0" ]; then
		if [ "$VIDEO_BITRATE" == "auto" ]; then
			echo -n "Measuring max. available bitrate .. "
			BITRATE_MEASURED=`/home/pi/wifibroadcast-base/tx_measure -p 77 -b $VIDEO_BLOCKS -r $VIDEO_FECS -f $VIDEO_BLOCKLENGTH -t $VIDEO_FRAMETYPE -d $VIDEO_WIFI_BITRATE -y 0 $NICS`
			BITRATE=$((BITRATE_MEASURED*$BITRATE_PERCENT/100))
			BITRATE_KBIT=$((BITRATE/1000))
			BITRATE_MEASURED_KBIT=$((BITRATE_MEASURED/1000))
			echo "$BITRATE_MEASURED_KBIT kBit/s * $BITRATE_PERCENT% = $BITRATE_KBIT kBit/s video bitrate"
		else
			BITRATE=$(($VIDEO_BITRATE*1000))
			echo "Using fixed bitrate: $VIDEO_BITRATE kBit"
		fi
	else
		BITRATE=$((1000*1000))
		BITRATE_KBIT=1000
		BITRATE_MEASURED_KBIT=2000
		echo "Using reduced bitrate: 1000 kBit due to undervoltage!"
	fi


    # check again if over-temp or under-voltage occured after bitrate measuring (but only if it didn't occur before yet)
	if [ "$UNDERVOLT" == "0" ]; then
		if vcgencmd get_throttled | nice grep -q -v "0x0"; then
			TEMP=`cat /sys/class/thermal/thermal_zone0/temp`
			TEMP_C=$(($TEMP/1000))
			if [ "$TEMP_C" -lt 75 ]; then # it must be under-voltage
				echo
				echo "  ---------------------------------------------------------------------------------------------------"
				echo "  | ERROR: Under-Voltage detected on the TX Pi. Your Pi is not supplied with stable 5 Volts.        |"
				echo "  | Either your power-supply or wiring is not sufficent, check the wiring instructions in the Wiki. |"
				echo "  | Video Bitrate will be reduced to 1000kbit to reduce current consumption!                        |"
				echo "  ---------------------------------------------------------------------------------------------------"
				echo
				mount -o remount,rw /boot
				echo "  ---------------------------------------------------------------------------------------------------" >> /boot/UNDERVOLTAGE-ERROR!!!.txt
				echo "  | ERROR: Under-Voltage detected on the TX Pi. Your Pi is not supplied with stable 5 Volts.        |" >> /boot/UNDERVOLTAGE-ERROR!!!.txt
				echo "  | Either your power-supply or wiring is not sufficent, check the wiring instructions in the Wiki. |" >> /boot/UNDERVOLTAGE-ERROR!!!.txt
				echo "  | Video Bitrate will be reduced to 1000kbit to reduce current consumption!                        |" >> /boot/UNDERVOLTAGE-ERROR!!!.txt
				echo "  | When you have fixed wiring/power-supply, delete this file and make sure it doesn't re-appear!   |" >> /boot/UNDERVOLTAGE-ERROR!!!.txt
				echo "  ---------------------------------------------------------------------------------------------------" >> /boot/UNDERVOLTAGE-ERROR!!!.txt
				mount -o remount,ro /boot
				UNDERVOLT=1
				echo "1" > /tmp/undervolt
				BITRATE=$((1000*1000))
				BITRATE_KBIT=1000
				BITRATE_MEASURED_KBIT=2000
			else # it was either over-temp or both undervolt and over-temp, we set undervolt to 0 anyway, since overtemp can be seen at the temp display on the rx
				UNDERVOLT=0
				echo "0" > /tmp/undervolt
			fi
		else
			UNDERVOLT=0
			echo "0" > /tmp/undervolt
		fi
	fi

    # check for over-current on USB bus (due to card being powered via usb instead directly)
    if nice dmesg | nice grep -q over-current; then
        echo "ERROR: Over-current detected - potential power supply problems!"
        collect_errorlog
		sleep 365d
    fi

    # check for USB disconnects (due to power-supply problems)
    if nice dmesg | nice grep -q disconnect; then
        echo "ERROR: USB disconnect detected - potential power supply problems!"
        collect_errorlog
		sleep 365d
    fi

    echo $BITRATE_KBIT > /tmp/bitrate_kbit
    echo $BITRATE_MEASURED_KBIT > /tmp/bitrate_measured_kbit

    if [ "$CTS" == "N" ]; then
		echo "0" > /tmp/cts
    else
		if [ "$VIDEO_WIFI_BITRATE" == "11" ] || [ "$VIDEO_WIFI_BITRATE" == "5" ]; then # 11mbit and 5mbit bitrates don't support CTS, so set to 0
			echo "0" > /tmp/cts
		else
			echo "1" > /tmp/cts
		fi
    fi

    if [ "$DEBUG" == "Y" ]; then
		collect_debug /boot &
    fi

    /home/pi/RemoteSettings/Air/rssitx.sh &

    echo
    echo "Starting transmission in $TXMODE mode, FEC $VIDEO_BLOCKS/$VIDEO_FECS/$VIDEO_BLOCKLENGTH: $WIDTH x $HEIGHT $FPS fps, video bitrate: $BITRATE_KBIT kBit/s, Keyframerate: $KEYFRAMERATE"
##########################Start mod:

#nice -n -9 raspivid -w $WIDTH -h $HEIGHT -fps $FPS -b $BITRATE -g $KEYFRAMERATE -t 0 $EXTRAPARAMS -o - | nice -n -9 /home/pi/wifibroadcast-base/tx_rawsock -p 0 -b $VIDEO_BLOCKS -r $VIDEO_FECS -f $VIDEO_BLOCKLENGTH -t $VIDEO_FRAMETYPE -d $VIDEO_WIFI_BITRATE -y 0 $NICS

#if [ "$IsAudioTransferEnabled" == "1" ]; then
#        /home/pi/RemoteSettings/Air/AudioCapture.sh &
#        /home/pi/RemoteSettings/Air/AudioTX.sh &
#fi

#if [ "$RemoteSettingsEnabled" == "1" ]; then
#        echo "\n RemoteSettings enabled \n"
#        /home/pi/RemoteSettings/RemoteSettingsWFBC_UDP_Air.sh > /dev/null &
#        /home/pi/RemoteSettings/AirRSSI.sh &
#        /usr/bin/python3.5 /home/pi/RemoteSettings/RemoteSettingsAir.py &
#else
#        re='^[0-9]+$'
#        if ! [[ $RemoteSettingsEnabled =~ $re ]] ; then
#                echo "RemoteSettings - incorrect timer value \n"
#        else
#                if [ "$RemoteSettingsEnabled" -ne "0" ]; then
#                        echo "\n RemoteSettings enabled with timer \n"
#                         /home/pi/RemoteSettings/RemoteSettingsWFBC_UDP_Air.sh > /dev/null &
#                        /home/pi/RemoteSettings/AirRSSI.sh &
#                        /usr/bin/python3.5 /home/pi/RemoteSettings/RemoteSettingsAir.py $RemoteSettingsEnabled &
#                fi
#        fi
#fi

#echo "starting remote settins 2 - scripts\n"
#/home/pi/RemoteSettings2/RemoteSettingsWFBC_UDP_Air.sh > /dev/null &
#echo "starting remote settins 2 - python\n"
#/usr/bin/python3.5 /home/pi/RemoteSettings2/src_python/ServerAir.py &


if [ "$IsBandSwicherEnabled" == "1" ]; then
        /home/pi/RemoteSettings/BandSwitcherAir.sh &
fi

echo "#!/bin/bash" > /dev/shm/startReadCameraTransfer.sh
echo "echo \$\$ > /dev/shm/TXCAMPID" >> /dev/shm/startReadCameraTransfer.sh

NICS="${NICS//$'\n'/ }"


echo "BitRate_20: "
echo $BITRATE

BITRATE_10=$((BITRATE/2))
echo "BitRate_10: "
echo $BITRATE_10

BITRATE_5=$((BITRATE_10/2))
echo "BitRate_5: "
echo $BITRATE_5


echo "nice -n -9 raspivid -w $WIDTH -h $HEIGHT -fps $FPS -b $BITRATE -g $KEYFRAMERATE -t 0 $EXTRAPARAMS -o - | nice -n -9 /home/pi/wifibroadcast-base/tx_rawsock -p 0 -b $VIDEO_BLOCKS -r $VIDEO_FECS -f $VIDEO_BLOCKLENGTH -t $VIDEO_FRAMETYPE -d $VIDEO_WIFI_BITRATE -y 0 $NICS" >> /dev/shm/startReadCameraTransfer.sh
echo "nice -n -9 raspivid -w $WIDTH -h $HEIGHT -fps $FPS -b $BITRATE_10 -g $KEYFRAMERATE -t 0 $EXTRAPARAMS -o - | nice -n -9 /home/pi/wifibroadcast-base/tx_rawsock -p 0 -b $VIDEO_BLOCKS -r $VIDEO_FECS -f $VIDEO_BLOCKLENGTH -t $VIDEO_FRAMETYPE -d $VIDEO_WIFI_BITRATE -y 0 $NICS" >> /dev/shm/startReadCameraTransfer_10.sh
echo "nice -n -9 raspivid -w $WIDTH -h $HEIGHT -fps $FPS -b $BITRATE_5 -g $KEYFRAMERATE -t 0 $EXTRAPARAMS -o - | nice -n -9 /home/pi/wifibroadcast-base/tx_rawsock -p 0 -b $VIDEO_BLOCKS -r $VIDEO_FECS -f $VIDEO_BLOCKLENGTH -t $VIDEO_FRAMETYPE -d $VIDEO_WIFI_BITRATE -y 0 $NICS" >> /dev/shm/startReadCameraTransfer_5.sh

echo $NICS > /tmp/NICS.txt
echo $VIDEO_WIFI_BITRATE > /tmp/DATARATE.txt

chmod +x /dev/shm/startReadCameraTransfer.sh
chmod +x /dev/shm/startReadCameraTransfer_5.sh
chmod +x /dev/shm/startReadCameraTransfer_10.sh
/usr/bin/python /home/pi/cameracontrol/cameracontrolUDP.py -IsCamera1Enabled $IsCamera1Enabled -IsCamera2Enabled $IsCamera2Enabled -IsCamera3Enabled $IsCamera3Enabled -IsCamera4Enabled $IsCamera4Enabled  -Camera1ValueMin $Camera1ValueMin -Camera1ValueMax $Camera1ValueMax -Camera2ValueMin $Camera2ValueMin -Camera2ValueMax $Camera2ValueMax -Camera3ValueMin $Camera3ValueMin -Camera3ValueMax $Camera3ValueMax  -Camera4ValueMin $Camera4ValueMin -Camera4ValueMax $Camera4ValueMax -DefaultCameraId $DefaultCameraId


###########################END MOD.

# Older video transmit commands, might be usefull for USB webcams...
#    v4l2-ctl -d /dev/video0 --set-fmt-video=width=1280,height=720,pixelformat='H264' -p 48 --set-ctrl video_bitrate=7000000,repeat_sequence_header=1,h264_i_frame_period=7,white_balance_auto_preset=5
#    nice -n -9 cat /dev/video0 | /home/pi/wifibroadcast-base/tx_rawsock -p 0 -b $VIDEO_BLOCKS -r $VIDEO_FECS -f $VIDEO_BLOCKLENGTH -t $VIDEO_FRAMETYPE -d $VIDEO_WIFI_BITRATE -y 0 $NICS

    TX_EXITSTATUS=${PIPESTATUS[1]}
	
    # if we arrive here, either raspivid or tx did not start, or were terminated later
    # check if NIC has been removed
    NICS2=`ls /sys/class/net/ | nice grep -v eth0 | nice grep -v lo | nice grep -v usb | nice grep -v intwifi | nice grep -v relay | nice grep -v wifihotspot`
    if [ "$NICS" == "$NICS2" ]; then
    	# wifi card has not been removed
		if [ "$TX_EXITSTATUS" != "0" ]; then
			echo "ERROR: could not start tx or tx terminated!"
		fi
	
		collect_errorlog
	
		sleep 365d
    else
        # wifi card has been removed
        echo "ERROR: Wifi card removed!"
		
		collect_errorlog
		
		sleep 365d
    fi
}
