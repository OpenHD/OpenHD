function tx_function {
    killall wbc_status > /dev/null 2>&1

    if [[ "${ENABLE_NEW_PI_AWB}" != "Y" ]]; then
        vcdbg set awb_mode 0
    fi

    if [ "$LTE" == "Y" ]; then
        source lte_functions.sh
        lte_function
    fi

    #
    # Look for a VEYE camera by probing the i2c bus. Note that this *requires*
    # i2c_vc to already be enabled or the bus won't even be available.
    #
    i2cdetect -y 0 | grep  "30: -- -- -- -- -- -- -- -- -- -- -- 3b -- -- -- --"
    grepRet=$?
    if [[ $grepRet -eq 0 ]] ; then
        echo "VEYE camera detected"

        #
        # Signal to the rest of the system that a VEYE camera was detected
        #
        echo "1" > /tmp/imx290
        IMX290="1"

        #
        # Configure the camera's ISP parameters
        #
        pushd /usr/local/share/veye-raspberrypi
        /usr/local/share/veye-raspberrypi/veye_mipi_i2c.sh -w -f wdrmode -p1 $IMX290_wdrmode > /tmp/imx290log
        /usr/local/share/veye-raspberrypi/veye_mipi_i2c.sh -w -f mirrormode -p1 $IMX290_mirrormode >> /tmp/imx290log
        /usr/local/share/veye-raspberrypi/veye_mipi_i2c.sh -w -f denoise -p1 $IMX290_denoise >> /tmp/imx290log

        if [ "${IMX290_lowlight}" != "" ]; then
            /usr/local/share/veye-raspberrypi/veye_mipi_i2c.sh -w -f lowlight -p1 ${IMX290_lowlight} >> /tmp/imx290log
        else
            # turn it off by default to avoid framerate changing during flight
            /usr/local/share/veye-raspberrypi/veye_mipi_i2c.sh -w -f lowlight -p1 0x00 >> /tmp/imx290log
        fi
        popd
    fi


    if [ "$LoadFlirDriver" == "Y" ]; then
        echo "FLIR enabled"
        qstatus "FLIR enabled" 5

        /usr/local/share/cameracontrol/LoadFlirDriver.sh &
    fi


    /usr/local/bin/sharedmem_init_tx


    if [ "$TXMODE" == "single" ]; then
        echo -n "Waiting for wifi card to become ready"

        COUNTER=0

        #
        # Wait until the card is initialized before continuing 
        #
        while [ $COUNTER -lt 10 ]; do
            sleep 0.5
            echo -n "."

            let "COUNTER++"

            if [ -d "/sys/class/net/wlan0" ]; then
                echo -n "Card ready"
                break
            fi
        done
    else
        #
        # TODO: this is a race condition, we should be looking for cards proactively early on, then
        #       checking for each of them rather than waiting a specific amount of time
        #
        echo -n "Waiting 3 seconds for wifi cards to become ready"
        sleep 3
    fi

    echo
    echo

    dmesg -c >/dev/null 2>/dev/null

    detect_nics

    if [ "$Bandwidth" == "10" ]; then
        qstatus "Using 10MHz channel bandwidth" 5
        echo "HardCode dirty code for tests only. Values are it Hex, to set 10MHz use 0xa (10 in dec)"

        echo 0xa > /sys/kernel/debug/ieee80211/phy0/ath9k_htc/chanbw
        echo 0xa > /sys/kernel/debug/ieee80211/phy1/ath9k_htc/chanbw
        echo 0xa > /sys/kernel/debug/ieee80211/phy2/ath9k_htc/chanbw
        echo 0xa > /sys/kernel/debug/ieee80211/phy3/ath9k_htc/chanbw
        echo 0xa > /sys/kernel/debug/ieee80211/phy4/ath9k_htc/chanbw
    fi

    if [ "$Bandwidth" == "5" ]; then
        qstatus "Using 5MHz channel bandwidth" 5
        echo "HardCode dirty code for tests only. Values are it Hex, to set 10MHz use 0xa (10 in dec)"

        echo 5 > /sys/kernel/debug/ieee80211/phy0/ath9k_htc/chanbw
        echo 5 > /sys/kernel/debug/ieee80211/phy1/ath9k_htc/chanbw
        echo 5 > /sys/kernel/debug/ieee80211/phy2/ath9k_htc/chanbw
        echo 5 > /sys/kernel/debug/ieee80211/phy3/ath9k_htc/chanbw
        echo 5 > /sys/kernel/debug/ieee80211/phy4/ath9k_htc/chanbw
    fi
    

    sleep 1

    echo

    if [ -e "$FC_TELEMETRY_SERIALPORT" ]; then
        echo "Flight controller serial port $FC_TELEMETRY_SERIALPORT found"

    else
        echo "ERROR: $FC_TELEMETRY_SERIALPORT not found!"

        collect_errorlog
        sleep 365d
    fi


    echo


    RALINK=0


    if [ "$TXMODE" == "single" ]; then
        #
        # for single TX setups, the frame type to use depends on the type of wifi chip
        #

        DRIVER=`cat /sys/class/net/$NICS/device/uevent | nice grep DRIVER | sed 's/DRIVER=//'`

        case $DRIVER in
            *881[24]au)
                DRIVER=rtl88xxau
                ;;
        esac

        if [ "$DRIVER" != "ath9k_htc" ]; then
            #
            # in single mode and ralink cards always, use frametype 1 (data)
            #

            VIDEO_FRAMETYPE=0
            if [[ "$DRIVER" == "rtl88xxau" || "$DRIVER" == "rtl88x2bu" ]]; then
                if [ "$CTS_PROTECTION" != "Y" ] && [ "$UseMCS" == "1" ]; then
                    VIDEO_FRAMETYPE=2
                else
                    VIDEO_FRAMETYPE=1
                fi
            fi
            RALINK=1
        fi
    else
        #
        # for normal multiple wifi adapter setups, always use frametype 1
        #
        VIDEO_FRAMETYPE=1
        RALINK=1
    fi


    if [ "$VIDEO_WIFI_BITRATE" == "19.5" ]; then
        #
        # set back to 18 to make sure -d parameter works (supports only 802.11b/g datarates)
        #
        VIDEO_WIFI_BITRATE=18
    fi

    if [ "$VIDEO_WIFI_BITRATE" == "5.5" ]; then
        #
        # set back to 6 to make sure -d parameter works (supports only 802.11b/g datarates)
        #
        VIDEO_WIFI_BITRATE=5
    fi



    DRIVER=`cat /sys/class/net/$NICS/device/uevent | nice grep DRIVER | sed 's/DRIVER=//'`
    case $DRIVER in
        *881[24]au)
            DRIVER=rtl88xxau
        ;;
    esac



    #
    # only enable CTS protection automatically with Atheros, otherwise disable it unless explicitly enabled
    #
    if [ "$CTS_PROTECTION" == "auto" ] && [ "$DRIVER" == "ath9k_htc" ]; then
        echo -n "Checking for other wifi traffic ... "

        WIFIPPS=`/usr/local/bin/wifiscan $NICS`

        echo -n "Detected WiFi packets per second:  $WIFIPPS"

        if [ "$WIFIPPS" != "0" ]; then
            echo "Video CTS: enabled"
            qstatus "Video CTS: enabled" 5

            VIDEO_FRAMETYPE=1
            TELEMETRY_CTS=1
            CTS=Y
        else
            echo "No wifi traffic detected, disabling CTS"
            qstatus "No wifi traffic detected, disabling CTS" 5

            CTS=N
        fi
    else
        if [ "$CTS_PROTECTION" == "N" ]; then
            echo "Video CTS: disabled"
            qstatus "Video CTS: disabled" 5

            CTS=N
        else
            if [[ "$DRIVER" == "ath9k_htc" || "$DRIVER" == "rtl88xxau" || "$DRIVER" == "rtl88x2bu" ]]; then
                echo "Video CTS: enabled"
                qstatus "Video CTS: enabled" 5

                CTS=Y
            else
                echo "Video CTS: not supported on ${DRIVER}"
                qstatus "Video CTS: not supported on ${DRIVER}" 3

                CTS=N
            fi
        fi
    fi


    # 
    # Check if over-temp or under-voltage occured before bitrate measuring
    #
    if vcgencmd get_throttled | nice grep -q -v "0x0"; then
        TEMP=`cat /sys/class/thermal/thermal_zone0/temp`
        TEMP_C=$(($TEMP/1000))
        
        #
        # If the pi is throttled but not becuase of temperature, we assume it must be due to undervolt
        #
        if [ "$TEMP_C" -lt 75 ]; then
            echo
            echo "  ---------------------------------------------------------------------------------------------------"
            echo "  | ERROR: Under-Voltage detected on the AIRPi. Your Pi is not supplied with stable 5 Volts.        |"
            echo "  | Either your power-supply or wiring is not sufficent, check the wiring instructions in the Wiki. |"
            echo "  | Video Bitrate will be reduced to 1000kbit to reduce current consumption!                        |"
            echo "  ---------------------------------------------------------------------------------------------------"
            echo
            
            qstatus "ERROR: Undervoltage detected, check power supply" 3

            
            mount -o remount,rw /boot
            
            echo "  ---------------------------------------------------------------------------------------------------" >> /boot/UNDERVOLTAGE-ERROR!!!.txt
            echo "  | ERROR: Under-Voltage detected on the AIRPi. Your Pi is not supplied with stable 5 Volts.        |" >> /boot/UNDERVOLTAGE-ERROR!!!.txt
            echo "  | Either your power-supply or wiring is not sufficent, check the wiring instructions in the Wiki. |" >> /boot/UNDERVOLTAGE-ERROR!!!.txt
            echo "  | Video Bitrate will be reduced to 1000kbit to reduce current consumption!                        |" >> /boot/UNDERVOLTAGE-ERROR!!!.txt
            echo "  | When you have fixed wiring/power-supply, delete this file and make sure it doesn't re-appear!   |" >> /boot/UNDERVOLTAGE-ERROR!!!.txt
            echo "  ---------------------------------------------------------------------------------------------------" >> /boot/UNDERVOLTAGE-ERROR!!!.txt
            
            mount -o remount,ro /boot
            
            UNDERVOLT=1

            echo "1" > /tmp/undervolt
        else
            #
            # It was either over-temp or both undervolt and over-temp, we set undervolt to 0 anyway, since overtemp can be 
            # seen at the temp display on the rx
            #
            UNDERVOLT=0

            echo "0" > /tmp/undervolt
        fi
    else
        UNDERVOLT=0

        echo "0" > /tmp/undervolt
    fi

    # If a tc358743 HDMI CSI board is detected and framerate is 30fps, cut bitrate in half.
    #
    # This is a fix for an odd bug, it would be nice to find the real cause but this works for now
    #
    if [[ "${HDMI_CSI}" == "1" && "${FPS}" == "30" ]]; then

        echo "Reducing video bitrate by half for HDMI CSI board @ 30fps"
        qstatus "Reducing video bitrate by half for HDMI CSI board @ 30fps" 5

        BITRATE_PERCENT=$(python -c "print(${BITRATE_PERCENT}/2)")

        if [ "${VIDEO_BITRATE}" != "auto" ]; then
            BITRATE=$(python -c "print(${VIDEO_BITRATE}/2)")
        fi
    fi

    #
    # Only do bitrate measurement if no undervolt was detected, to ensure the system boots fully and begins working, even 
    # though it will be in a degraded state
    #
    if [ "$UNDERVOLT" == "0" ]; then
        if [ "$VIDEO_BITRATE" == "auto" ]; then

            echo "-----------------------------------------"
            echo "Running bandwidth measurement...         "
            echo "-----------------------------------------"

            qstatus "Running bandwidth measurement..." 5
            
            BANDWIDTH_MEASURED=$(cat /dev/zero | /usr/local/bin/tx_rawsock -z 1 -p 77 -b $VIDEO_BLOCKS -r $VIDEO_FECS -f $VIDEO_BLOCKLENGTH -t $VIDEO_FRAMETYPE -d $VIDEO_WIFI_BITRATE -M $UseMCS -S $UseSTBC -L $UseLDPC -y 0 $NICS)
            BANDWIDTH_MEASURED_KBIT=$(python -c "print(int(${BANDWIDTH_MEASURED} / 1000.0))")
            echo "Bandwidth available: ${BANDWIDTH_MEASURED_KBIT}Kbit/s"
            qstatus "Bandwidth available: ${BANDWIDTH_MEASURED_KBIT}Kbit/s" 5
            
            FEC_SIZE=$(python -c "print(${VIDEO_BLOCKS} + ${VIDEO_FECS})")
            echo "Video/FEC ratio: ${VIDEO_BLOCKS}/${FEC_SIZE}"
            qstatus "Video/FEC ratio: ${VIDEO_BLOCKS}/${FEC_SIZE}" 5

            AVAILABLE_VIDEO_BANDWIDTH=$(python -c "print(int(${BANDWIDTH_MEASURED} * (float(${VIDEO_BLOCKS}) / float(${FEC_SIZE}))))")
            AVAILABLE_VIDEO_BANDWIDTH_KBIT=$(python -c "print(int(float(${AVAILABLE_VIDEO_BANDWIDTH}) / 1000.0))")
            echo "Bandwidth available for video: ${AVAILABLE_VIDEO_BANDWIDTH_KBIT}Kbit/s"
            qstatus "Bandwidth available for video: ${AVAILABLE_VIDEO_BANDWIDTH_KBIT}Kbit/s" 5
            
            BITRATE=$(python -c "print(int(${AVAILABLE_VIDEO_BANDWIDTH} * ${BITRATE_PERCENT} / 100.0))")
            BITRATE_KBIT=$(($BITRATE/1000))
            BITRATE_MEASURED_KBIT=$(($BANDWIDTH_MEASURED/1000))

            echo "Using average of $BITRATE_PERCENT% of available video bandwidth"
            qstatus "Using average of $BITRATE_PERCENT% of available video bandwidth" 5
            echo "-----------------------------------------"
            echo "Final video bitrate: $BITRATE_KBIT kBit/s"
            qstatus "Final video bitrate: $BITRATE_KBIT kBit/s" 5
            echo "-----------------------------------------"
        else
            BITRATE=$(python -c "print(int(${VIDEO_BITRATE}*1000*1000))")
            BITRATE_KBIT=$(python -c "print(int(${VIDEO_BITRATE}*1000))")
            BITRATE_MEASURED_KBIT=0

            echo "Using fixed $BITRATE_KBIT kBit/s video bitrate"
            qstatus "Using fixed $BITRATE_KBIT kBit/s video bitrate" 5
        fi
    else
        BITRATE=$((1000*1000))
        BITRATE_KBIT=1000
        BITRATE_MEASURED_KBIT=2000

        echo "Using fixed $BITRATE_KBIT kBit/s video bitrate due to undervoltage!"
        qstatus "Using fixed $BITRATE_KBIT kBit/s video bitrate due to undervoltage!" 5
    fi

    #
    # Check again if over-temp or under-voltage occured after bitrate measuring,
    # but only if we haven't already detected undervolt (we don't want to erase the previously
    # detected undervolt condition)
    #
    if [ "$UNDERVOLT" == "0" ]; then
        if vcgencmd get_throttled | nice grep -q -v "0x0"; then
            TEMP=`cat /sys/class/thermal/thermal_zone0/temp`
            TEMP_C=$(($TEMP/1000))

            #
            # If the pi is throttled but not becuase of temperature, we assume it must be due to undervolt
            #
            if [ "$TEMP_C" -lt 75 ]; then
                echo
                echo "  ---------------------------------------------------------------------------------------------------"
                echo "  | ERROR: Under-Voltage detected on the AIRPi. Your Pi is not supplied with stable 5 Volts.        |"
                echo "  | Either your power-supply or wiring is not sufficent, check the wiring instructions in the Wiki. |"
                echo "  | Video Bitrate will be reduced to 1000kbit to reduce current consumption!                        |"
                echo "  ---------------------------------------------------------------------------------------------------"
                echo

                mount -o remount,rw /boot

                echo "  ---------------------------------------------------------------------------------------------------" >> /boot/UNDERVOLTAGE-ERROR!!!.txt
                echo "  | ERROR: Under-Voltage detected on the AIRPi. Your Pi is not supplied with stable 5 Volts.        |" >> /boot/UNDERVOLTAGE-ERROR!!!.txt
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

                echo "Using fixed $BITRATE_KBIT kBit/s video bitrate due to undervoltage!"
            else 
                #
                # It was either over-temp or both undervolt and over-temp, we set undervolt to 0 anyway, since
                # overtemp can be seen at the temp display on the rx
                # 
                UNDERVOLT=0

                echo "0" > /tmp/undervolt
            fi
        else
            UNDERVOLT=0

            echo "0" > /tmp/undervolt
        fi
    fi

    #
    # Check for over-current on USB bus (due to card being powered via usb instead directly)
    #
    if nice dmesg | nice grep -q over-current; then
        echo "ERROR: Over-current detected - potential power supply problems!"
        qstatus  "ERROR: Over-current detected - potential power supply problems!" 3

        collect_errorlog
        sleep 365d
    fi

    #
    # Check for USB disconnects (due to power-supply problems)
    #
    if nice dmesg | nice grep -q disconnect; then
        echo "ERROR: USB disconnect detected - potential power supply problems!"
        qstatus "ERROR: USB disconnect detected - potential power supply problems!" 3

        collect_errorlog
        sleep 365d
    fi


    #
    # Forward the configured bitrate to the OSD so it can be displayed on-screen
    #
    echo $BITRATE_KBIT > /tmp/bitrate_kbit
    echo $BITRATE_MEASURED_KBIT > /tmp/bitrate_measured_kbit


    #
    # Check to see if CTS can actually be enabled, some configurations can't support it
    #
    if [ "$CTS" == "N" ]; then

        echo "0" > /tmp/cts
    else
        if [ "$VIDEO_WIFI_BITRATE" == "11" ] || [ "$VIDEO_WIFI_BITRATE" == "5" ]; then
            #
            # 11Mbit and 5Mbit datarates don't support CTS, so we disable it
            #
            
            echo "0" > /tmp/cts
        else
            #
            # CTS was not explicitly disabled, and we aren't using one of the problematic datarates, so it
            # can be enabled
            #

            echo "1" > /tmp/cts
        fi
    fi

    #
    # Store the boot log up to this point so users can retrieve it from the FAT32/boot partition and provide it to
    # developers when there is a problem
    #
    if [ "$DEBUG" == "Y" ]; then
        collect_debug /boot &
    fi


    /usr/local/share/RemoteSettings/Air/rssitx.sh &

    echo
    echo "Starting video TX, FEC $VIDEO_BLOCKS/$VIDEO_FECS/$VIDEO_BLOCKLENGTH: $WIDTH x $HEIGHT $FPS fps, video bitrate: $BITRATE_KBIT kBit/s, keyframe interval: $KEYFRAMERATE"

    qstatus "Starting video TX" 5
    qstatus "FEC: $VIDEO_BLOCKS/$VIDEO_FECS/$VIDEO_BLOCKLENGTH" 5
    qstatus "Resolution: ${WIDTH}x${HEIGHT}@${FPS}" 5
    qstatus "Video bitrate: $BITRATE_KBIT kBit/s, keyframe interval: $KEYFRAMERATE" 5


    if [ "$IsAudioTransferEnabled" == "1" ]; then
        qstatus "Audio enabled" 5
        /usr/local/share/RemoteSettings/Air/AudioCapture.sh &
        /usr/local/share/RemoteSettings/Air/AudioTX.sh &
    fi

    #
    # Start joystick processes
    #
    if [ "$EncryptionOrRange" == "Encryption" ]; then
        /usr/local/share/RemoteSettings/Air/RxJoystick.sh &
        /usr/local/share/RemoteSettings/Air/processUDP.sh &
    fi

    # 
    # Start the live remote settings system
    #
    if [ "$RemoteSettingsEnabled" == "1" ]; then
        echo "RemoteSettings enabled"
        qstatus "Remote settings enabled" 5

        /usr/local/share/RemoteSettings/RemoteSettingsWFBC_UDP_Air.sh > /dev/null &
        /usr/local/share/RemoteSettings/AirRSSI.sh &
        /usr/bin/python3 /usr/local/share/RemoteSettings/RemoteSettingsAir.py &
    else
        #
        # Check to see if RemoteSettings was enabled with a value other than 0/1/2, which we interpret
        # to mean that RemoteSettings should be enabled for a short time after boot, then disabled again
        #
        # The timer functionality is in RemoteSettingsAir.py, the value is placed in TerminateTimeOut and
        # used from a background thread that stops the RemoteSettings system on the air side if nothing
        # is actually using the RemoteSettings system from the ground side
        # 
        re='^[0-9]+$'
        if ! [[ $RemoteSettingsEnabled =~ $re ]] ; then
                echo "Incorrect timer value provided for RemoteSettings"
        else
            if [ "$RemoteSettingsEnabled" -ne "0" ] && [ "$RemoteSettingsEnabled" -ne "2" ]; then
                    echo "RemoteSettings enabled with timer"

                    /usr/local/share/RemoteSettings/RemoteSettingsWFBC_UDP_Air.sh > /dev/null &
                    /usr/local/share/RemoteSettings/AirRSSI.sh &
                    /usr/bin/python3 /usr/local/share/RemoteSettings/RemoteSettingsAir.py $RemoteSettingsEnabled &
            fi
        fi
    fi

    #
    # Start building the camera configuration script, they are handled this way so they can be easily
    # restarted when camera switching is in use
    #
    echo "#!/bin/bash" > /dev/shm/startReadCameraTransfer.sh
    echo "echo \$\$ > /dev/shm/TXCAMPID" >> /dev/shm/startReadCameraTransfer.sh

    NICS="${NICS//$'\n'/ }"

    echo "#!/bin/bash" >  /dev/shm/startReadCameraTransferExteranlBitrate.sh

    #
    # If tx_functions is running but the GPU says there are no cameras connected (CAM=0), it's
    # likely an IMX290. We could explicitly detect that, but this works for now.
    #
    CAM=`/usr/bin/vcgencmd get_camera | python3 -c 'import sys, re; s = sys.stdin.read(); s=re.sub("supported=\d+ detected=", "", s); print(s);'`
    
    if [ "${CAM}" -ge 1 ];  then

        if [ "$ENABLE_OPENHDVID" == "Y" ]; then
            CAMERA_PROGRAM="openhdvid"

            if [ "$AIR_VIDEO_RECORDING" == "Y" ]; then
                EXTRAPARAMS="${EXTRAPARAMS} --record ${AIR_VIDEO_RECORDING_PATH} -wr ${AIR_VIDEO_RECORDING_WIDTH} -hr ${AIR_VIDEO_RECORDING_HEIGHT}"
            fi
        else
            CAMERA_PROGRAM="raspivid"
        fi

        echo "nice -n -9 ${CAMERA_PROGRAM} -w $WIDTH -h $HEIGHT -fps $FPS -b \$1 -g $KEYFRAMERATE -t 0 $EXTRAPARAMS -o - | nice -n -9 /usr/local/bin/tx_rawsock -p 0 -b $VIDEO_BLOCKS -r $VIDEO_FECS -f $VIDEO_BLOCKLENGTH -t $VIDEO_FRAMETYPE -d $VIDEO_WIFI_BITRATE -M $UseMCS -S $UseSTBC -L $UseLDPC -y 0 $NICS" >> /dev/shm/startReadCameraTransferExteranlBitrate.sh
    else

        echo "nice -n -9 /usr/local/bin/veye_raspivid -w $WIDTH -h $HEIGHT -fps $FPS -b \$1 -g $KEYFRAMERATE -t 0 $EXTRAPARAMS_IMX290 -o - | nice -n -9 /usr/local/bin/tx_rawsock -p 0 -b $VIDEO_BLOCKS -r $VIDEO_FECS -f $VIDEO_BLOCKLENGTH -t $VIDEO_FRAMETYPE -d $VIDEO_WIFI_BITRATE -M $UseMCS -S $UseSTBC -L $UseLDPC -y 0 $NICS" >> /dev/shm/startReadCameraTransferExteranlBitrate.sh
    fi

    #
    # Build the USB camera contol script, used for single USB camera and 2nd cameras using the camera switching
    # system
    #
    echo "#!/bin/bash" > /dev/shm/startReadUSBCamera.sh
    echo $USBCamera >> /dev/shm/startReadUSBCamera.sh


    #
    # Build the IP camera contol scripts, used for 2nd cameras using the camera switching system
    #
    echo "#!/bin/bash" > /dev/shm/startReadIPCameraHiRes.sh
    echo "while [ True ]; do" >> /dev/shm/startReadIPCameraHiRes.sh
    echo $IPCameraHiRes >> /dev/shm/startReadIPCameraHiRes.sh
    echo "sleep 2" >> /dev/shm/startReadIPCameraHiRes.sh
    echo "done" >> /dev/shm/startReadIPCameraHiRes.sh

    echo "#!/bin/bash" > /dev/shm/startReadIPCameraLowRes.sh
    echo "while [ True ]; do" >> /dev/shm/startReadIPCameraLowRes.sh
    echo $IPCameraLowRes >> /dev/shm/startReadIPCameraLowRes.sh
    echo "done" >> /dev/shm/startReadIPCameraLowRes.sh



    # 
    # Create half/quarter bitrate variables that can be used in the camera control scripts. When
    # camera switching is enabled, we cut the bitrate of the main video to allow the 2nd camera
    # to use some of the available bandwidth, (hopefully) without exceeding the limit
    # 
    echo "BitRate_20: "
    echo $BITRATE

    BITRATE_10=$((BITRATE/2))
    echo "BitRate_10: "
    echo $BITRATE_10

    BITRATE_5=$((BITRATE_10/2))
    echo "BitRate_5: "
    echo $BITRATE_5


    #
    # If tx_functions is running but the GPU says there are no cameras connected (CAM=0), it's
    # likely an IMX290. We could explicitly detect that, but this works for now.
    #
    # Note: this should be expanded to support single USB cameras as well
    #
    if [ "${CAM}" -ge 1 ]; then

        #
        # If openhdvid is enabled we mark it as the camera program and pull in the configuration variables it uses
        # 
        # Note that openhdvid is being removed and replaced with the camera microservice
        #
        if [ "$ENABLE_OPENHDVID" == "Y" ]; then
            CAMERA_PROGRAM="openhdvid"

            if [ "$AIR_VIDEO_RECORDING" == "Y" ]; then
                EXTRAPARAMS="${EXTRAPARAMS} --record ${AIR_VIDEO_RECORDING_PATH} -wr ${AIR_VIDEO_RECORDING_WIDTH} -hr ${AIR_VIDEO_RECORDING_HEIGHT}"
            fi
        else
            CAMERA_PROGRAM="raspivid"
        fi

        #
        # When LTE is enabled, the camera stream goes to a splitter rather than directly to tx_rawsock, so that it can be
        # duplicated and sent out through the LTE card as well.
        # 
        if [ "$LTE" == "Y" ]; then
            echo "nice -n -9 ${CAMERA_PROGRAM} -w $WIDTH -h $HEIGHT -fps $FPS -b $BITRATE -g $KEYFRAMERATE -t 0 $EXTRAPARAMS -o - | gst-launch-1.0 -v fdsrc !  tee name=splitter ! queue ! h264parse ! rtph264pay config-interval=10 pt=96 ! udpsink host=$ZT_IP port=8000 splitter. ! queue ! fdsink | nice -n -9 /usr/local/bin/tx_rawsock -p 0 -b $VIDEO_BLOCKS -r $VIDEO_FECS -f $VIDEO_BLOCKLENGTH -t $VIDEO_FRAMETYPE -d $VIDEO_WIFI_BITRATE -M $UseMCS -S $UseSTBC -L $UseLDPC -y 0 $NICS" >> /dev/shm/startReadCameraTransfer.sh
        else
            echo "nice -n -9 ${CAMERA_PROGRAM} -w $WIDTH -h $HEIGHT -fps $FPS -b $BITRATE -g $KEYFRAMERATE -t 0 $EXTRAPARAMS -o - | nice -n -9 /usr/local/bin/tx_rawsock -p 0 -b $VIDEO_BLOCKS -r $VIDEO_FECS -f $VIDEO_BLOCKLENGTH -t $VIDEO_FRAMETYPE -d $VIDEO_WIFI_BITRATE -M $UseMCS -S $UseSTBC -L $UseLDPC -y 0 $NICS" >> /dev/shm/startReadCameraTransfer.sh
        fi

        #
        # Create the half/quarter bandwidth camera scripts, used for camera switching
        # 
        echo "nice -n -9 ${CAMERA_PROGRAM} -w $WIDTH -h $HEIGHT -fps $FPS -b $BITRATE_10 -g $KEYFRAMERATE -t 0 $EXTRAPARAMS -o - | nice -n -9 /usr/local/bin/tx_rawsock -p 0 -b $VIDEO_BLOCKS -r $VIDEO_FECS -f $VIDEO_BLOCKLENGTH -t $VIDEO_FRAMETYPE -d $VIDEO_WIFI_BITRATE -M $UseMCS -S $UseSTBC -L $UseLDPC -y 0 $NICS" >> /dev/shm/startReadCameraTransfer_10.sh
        echo "nice -n -9 ${CAMERA_PROGRAM} -w $WIDTH -h $HEIGHT -fps $FPS -b $BITRATE_5 -g $KEYFRAMERATE -t 0 $EXTRAPARAMS -o - | nice -n -9 /usr/local/bin/tx_rawsock -p 0 -b $VIDEO_BLOCKS -r $VIDEO_FECS -f $VIDEO_BLOCKLENGTH -t $VIDEO_FRAMETYPE -d $VIDEO_WIFI_BITRATE -M $UseMCS -S $UseSTBC -L $UseLDPC -y 0 $NICS" >> /dev/shm/startReadCameraTransfer_5.sh
    else

        #
        # No pi camera was detected, but this is an air pi so we assume it's a VEYE camera that the GPU can't detect
        #

        echo "nice -n -9 /usr/local/bin/veye_raspivid -w $WIDTH -h $HEIGHT -fps $FPS -b $BITRATE -g $KEYFRAMERATE -t 0 $EXTRAPARAMS_IMX290 -o - | nice -n -9 /usr/local/bin/tx_rawsock -p 0 -b $VIDEO_BLOCKS -r $VIDEO_FECS -f $VIDEO_BLOCKLENGTH -t $VIDEO_FRAMETYPE -d $VIDEO_WIFI_BITRATE -M $UseMCS -S $UseSTBC -L $UseLDPC -y 0 $NICS" >> /dev/shm/startReadCameraTransfer.sh
        echo "nice -n -9 /usr/local/bin/veye_raspivid -w $WIDTH -h $HEIGHT -fps $FPS -b $BITRATE_10 -g $KEYFRAMERATE -t 0 $EXTRAPARAMS_IMX290 -o - | nice -n -9 /usr/local/bin/tx_rawsock -p 0 -b $VIDEO_BLOCKS -r $VIDEO_FECS -f $VIDEO_BLOCKLENGTH -t $VIDEO_FRAMETYPE -d $VIDEO_WIFI_BITRATE -M $UseMCS -S $UseSTBC -L $UseLDPC -y 0 $NICS" >> /dev/shm/startReadCameraTransfer_10.sh
        echo "nice -n -9 /usr/local/bin/veye_raspivid -w $WIDTH -h $HEIGHT -fps $FPS -b $BITRATE_5 -g $KEYFRAMERATE -t 0 $EXTRAPARAMS_IMX290 -o - | nice -n -9 /usr/local/bin/tx_rawsock -p 0 -b $VIDEO_BLOCKS -r $VIDEO_FECS -f $VIDEO_BLOCKLENGTH -t $VIDEO_FRAMETYPE -d $VIDEO_WIFI_BITRATE -M $UseMCS -S $UseSTBC -L $UseLDPC -y 0 $NICS" >> /dev/shm/startReadCameraTransfer_5.sh
    fi


    echo $NICS > /tmp/NICS.txt
    echo $VIDEO_WIFI_BITRATE > /tmp/DATARATE.txt

    chmod +x /dev/shm/startReadCameraTransfer.sh
    chmod +x /dev/shm/startReadCameraTransfer_5.sh
    chmod +x /dev/shm/startReadCameraTransfer_10.sh

    chmod +x /dev/shm/startReadCameraTransferExteranlBitrate.sh
    chmod +x /dev/shm/startReadIPCameraHiRes.sh
    chmod +x /dev/shm/startReadIPCameraLowRes.sh
    chmod +x /dev/shm/startReadUSBCamera.sh


    #
    # Check for arducam camera switcher
    # 
    # The code that manages this is in cameracontrolUDP.py
    #
    IsArduCameraV21="0"
    i2cdetect -y 1 | grep  "70: 70"
    grepRet=$?

    if [[ $grepRet -eq 0 ]] ; then
        IsArduCameraV21="21"
    fi

    CameraType="RPi"

    #
    # Check to see if GPIO7 is pulled low, signaling that air side boot was chosen
    #
    if [ -e /tmp/Air ]; then
        CameraType="Secondary"
        
        if [ "$IsBandSwicherEnabled" != "1" ]; then
            /usr/local/share/RemoteSettings/Air/TxBandSwitcher.sh &
        fi

        if [ $SecondaryCamera == "No" ]; then
            echo "SecondaryCamera type is not selected, but RPi forced to boot as Air unit via GPIO. Camera type set to USB"
            qstatus "Camera type set to USB" 5
            SecondaryCamera="USB"
        fi
    fi

    #
    # Start the 2nd camera, currently uses SVPCOM but will be switched to rawsock soon
    #
    if [ $SecondaryCamera != "No" ]; then
        /usr/local/share/RemoteSettings/Air/TxSecondaryCamera.sh &
    fi


    #
    # Start the bandswitcher
    #
    # Note: this is also the camera switching system, they reuse the same control code
    #
    if [ "$IsBandSwicherEnabled" == "1" ]; then
        qstatus "Starting band switcher" 5
        /usr/local/share/RemoteSettings/BandSwitcherAir.sh $SecondaryCamera  $BITRATE &
        /usr/bin/python3 /usr/local/share/RemoteSettings/Air/MessageSorter.py &
    fi

    #
    # No official pi camera is connected, but this is an air pi. If the VEYE is not present,
    # we assume we are using a USB camera and immediately cause the band switcher to enable
    # the proper mode for it
    #
    WithoutNativeRPiCamera="0"
    if [ -e /tmp/CameraNotDetected ]; then
        WithoutNativeRPiCamera="1"
    fi

    # 
    # User selected USB or IP camera mode, so disable official pi camera even if detected
    #
    if [ -e /tmp/Air ]; then
        WithoutNativeRPiCamera="1"
    fi

    #
    # Start the camera control system
    #
    # Note: this is going to become part of the camera microservice soon
    #
    /usr/bin/python /usr/local/share/cameracontrol/cameracontrolUDP.py -IsArduCameraV21 $IsArduCameraV21 -IsCamera1Enabled $IsCamera1Enabled -IsCamera2Enabled $IsCamera2Enabled -IsCamera3Enabled $IsCamera3Enabled -IsCamera4Enabled $IsCamera4Enabled  -Camera1ValueMin $Camera1ValueMin -Camera1ValueMax $Camera1ValueMax -Camera2ValueMin $Camera2ValueMin -Camera2ValueMax $Camera2ValueMax -Camera3ValueMin $Camera3ValueMin -Camera3ValueMax $Camera3ValueMax  -Camera4ValueMin $Camera4ValueMin -Camera4ValueMax $Camera4ValueMax -DefaultCameraId $DefaultCameraId -BitrateMeasured $BITRATE -SecondaryCamera $SecondaryCamera -CameraType $CameraType -WithoutNativeRPiCamera $WithoutNativeRPiCamera -DefaultBandWidthAth9k $Bandwidth


    TX_EXITSTATUS=${PIPESTATUS[1]}
    
    #
    # cameracontrolUDP.py blocks, so if the script gets to this point then either raspivid or tx did not start, or were terminated later
    #
    # Check if NIC has been removed
    #
    NICS2=`ls /sys/class/net/ | nice grep -v eth0 | nice grep -v lo | nice grep -v usb | nice grep -v intwifi | nice grep -v relay | nice grep -v wifihotspot`
    
    if [ "$NICS" == "$NICS2" ]; then
     
        if [ "$TX_EXITSTATUS" != "0" ]; then    
            echo "ERROR: could not start tx or tx terminated!"
            qstatus "ERROR: could not start tx or tx terminated!" 3
        fi
    
        collect_errorlog
    
        sleep 365d
    else        
        echo "ERROR: Wifi card removed!"
        qstatus "ERROR: Wifi card removed!" 3
        
        collect_errorlog
        
        sleep 365d
    fi
}
