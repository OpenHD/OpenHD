function tmessage {
    if [ "$QUIET" == "N" ]; then
        echo $1 "$2"
    fi
}

function create_fifos {
    if [ "$TTY" != "/dev/tty1" ]; then
        return
    fi
    
    mkdir -p /var/run/openhd
    
    mkfifo /var/run/openhd/videofifo1
    mkfifo /var/run/openhd/videofifo2
    mkfifo /var/run/openhd/videofifo3
    mkfifo /var/run/openhd/videofifo4
    mkfifo /var/run/openhd/telemetryfifo1
    mkfifo /var/run/openhd/telemetryfifo2
    mkfifo /var/run/openhd/telemetryfifo3
    mkfifo /var/run/openhd/telemetryfifo4
    mkfifo /var/run/openhd/telemetryfifo5
    mkfifo /var/run/openhd/telemetryfifo6
    mkfifo /var/run/openhd/mspfifo

    touch /var/run/openhd/fifoready
}


function detect_os {
    source /etc/os-release

    if [[ "${VERSION_ID}" == "9" ]]; then
        export OPENHD_VERSION="stretch"
    elif [[ "${VERSION_ID}" == "10" ]]; then
        export OPENHD_VERSION="buster"
    else
        export OPENHD_VERSION="unknown"
    fi
}


function configure_hello_video_args {
    if [[ "$OPENHD_VERSION" == "stretch" ]]; then
        export HELLO_VIDEO_ARGS="0"
    elif [[ "$OPENHD_VERSION" == "buster" ]]; then
        export HELLO_VIDEO_ARGS="1"
    else
        export HELLO_VIDEO_ARGS="0"
    fi
}


function start_microservices {
    if [ "$TTY" != "/dev/tty1" ]; then
        return
    fi
    qstatus "Starting power microservice" 5

    systemctl start openhd_microservice@power

    # gpio service only runs on the air side
    if [ "${CAM}" -ge 1 ]; then 
        qstatus "Starting GPIO microservice" 5
        systemctl start openhd_microservice@gpio
    fi
}


function migration_helper {
    #
    # Force enable QOpenHD because it's the only option on buster
    #
    if [[ "$OPENHD_VERSION" == "buster" ]]; then
        export ENABLE_QOPENHD="Y"
    fi


    #
    # Automatically replace omxh264enc with v4l2h264enc on buster, the former doesn't work right now due to a bug 
    # in gstreamer-omx package in raspbian. 
    #
    # This is only a bandaid for people who have not updated their settings file yet, just to ensure that things work
    #
    if [[ "${OPENHD_VERSION}" == "buster" && "${USBCamera}" != "" ]]; then
        export USBCamera=`echo ${USBCamera} | python3 -c 'import sys, re; s = sys.stdin.read(); s=re.sub("omxh264enc.+\s*\!", "v4l2h264enc !", s); print(s);'`
    fi
}


function detect_memory {
    TOTAL_MEMORY=$(cat /proc/meminfo | grep 'MemTotal' | awk '{print $2}')
}


function detect_hardware {
    HARDWARE=$(cat /proc/cpuinfo | grep 'Revision' | awk '{print $3}')

    echo "Found hardware $HARDWARE..."

    case "$HARDWARE" in
        'a03111')
            ABLE_BAND=ag
            MODEL=Pi4b
        ;;
        'b03111')
            ABLE_BAND=ag
            MODEL=Pi4b
        ;;
        'b03112')
            ABLE_BAND=ag
            MODEL=Pi4b
        ;;
        'c03111')
            ABLE_BAND=ag
            MODEL=Pi4b
        ;;
        'c03112')
            ABLE_BAND=ag
            MODEL=Pi4b
        ;;
        'd03114')
            ABLE_BAND=ag
            MODEL=Pi4b
        ;;
        '29020e0')
            ABLE_BAND=ag
            MODEL=Pi3a+
        ;;
        '2a02082')
            ABLE_BAND=g
            MODEL=Pi3b
        ;;
        '2a22082')
            ABLE_BAND=g
            MODEL=Pi3b
        ;;
        '2a32082')
            ABLE_BAND=g
            MODEL=Pi3b
        ;;		
        '2a52082')
            ABLE_BAND=g
            MODEL=Pi3b
        ;;	
        '2a020d3')
            ABLE_BAND=ag
            MODEL=Pi3b+
        ;;
        '2900092')
            ABLE_BAND=none
            MODEL=PiZero
        ;;
        '2900093')
            ABLE_BAND=none
            MODEL=PiZero
        ;;
        '29000c1')
            ABLE_BAND=ag
            MODEL=PiZeroW
        ;;
        '2920092')
            ABLE_BAND=none
            MODEL=PiZero
        ;;
        '2920093')
            ABLE_BAND=none
            MODEL=PiZero
        ;;
        '2a22042')
            ABLE_BAND=none
            MODEL=Pi2b
        ;;
        '2a21041')
            ABLE_BAND=none
            MODEL=Pi2b
        ;;
        '2a01041')
            ABLE_BAND=none
            MODEL=Pi2b
        ;;
        '2a01040')
            ABLE_BAND=none
            MODEL=Pi2b
        ;;
        *)
            ABLE_BAND=unknown
            MODEL=unknown
        ;;
    esac

    qstatus "Running on $MODEL system" 5

    echo "Running on $MODEL system"
}


function detect_wfb_primary_band {
    lsmod | grep 88XXau
    grepRet=$?
    if [[ $grepRet -eq 0 ]] ; then
        export WFB_PRIMARY_BAND_58="1"
        export WFB_PRIMARY_BAND_24="0"
        echo "58" > /tmp/wfb_primary_band
        return
    fi

    lsmod | grep 88xxau
    grepRet=$?
    if [[ $grepRet -eq 0 ]] ; then
        export WFB_PRIMARY_BAND_58="1"
        export WFB_PRIMARY_BAND_24="0"
        echo "58" > /tmp/wfb_primary_band
        return
    fi

    lsmod | grep 8188eu
    grepRet=$?
    if [[ $grepRet -eq 0 ]] ; then
        export WFB_PRIMARY_BAND_58="0"
        export WFB_PRIMARY_BAND_24="1"
        echo "58" > /tmp/wfb_primary_band
        return
    fi

    lsmod | grep 88x2bu
    grepRet=$?
    if [[ $grepRet -eq 0 ]] ; then
        export WFB_PRIMARY_BAND_58="1"
        export WFB_PRIMARY_BAND_24="0"
        echo "58" > /tmp/wfb_primary_band
        return
    fi

    # every other card we support is 2.4-only, so we default to 2.4
    export WFB_PRIMARY_BAND_58="0"
    export WFB_PRIMARY_BAND_24="1"
    echo "24" > /tmp/wfb_primary_band
}


function auto_frequency_select {
    if [[ "${WFB_PRIMARY_BAND_58}" == "1" ]]; then
        if [[ "${FREQ}" == "auto" ]]; then
            export FREQ="5180"
        fi
    else
        if [[ "${FREQ}" == "auto" ]]; then
            export FREQ="2412"
        fi
    fi
}


#
# If /boot/i2c_vc exists, we check to see if i2c_vc is already enabled in
# config.txt, and if not we enable it and reboot. 
#
# This should not add more than a few seconds to the air boot time, and 
# only needs to be done once, but IMX290 and other 3rd party cameras won't 
# work without it
#
function autoenable_i2c_vc {
    #
    # Only run this on TTY1 to ensure it only runs once, we don't want it running several times and
    # partially overwriting the config.txt file
    #
    if [ "$TTY" != "/dev/tty1" ]; then
        return
    fi

    if [ -e "/boot/i2c_vc" ]; then
        
        I2C_VC_ENABLED=$(cat /boot/config.txt | grep "^dtparam=i2c_vc" | wc -l)
        
        if [ $I2C_VC_ENABLED == "0" ]; then
        
            I2C_VC_DISABLED=$(cat /boot/config.txt | grep "^#dtparam=i2c_vc" | wc -l)
            
            mount -o remount,rw /boot
        
            if [ $I2C_VC_DISABLED == "1" ]; then
                #
                # Present but disabled, we can use sed it to enable it in-place
                #
                sed -i 's/^#dtparam=i2c_vc.*/dtparam=i2c_vc=on/g' /boot/config.txt
            else
                #
                # It's not in the file at all, so we'll have to add it manually, but at the end of the
                # file with an [all] section to ensure that the setting applies to ever model in case
                # the config.txt file has sections like [pi3] [pi0] at the end.
                #
                echo "[all]" >> /boot/config.txt
                echo "dtparam=i2c_vc=on" >> /boot/config.txt
            fi
        
            mount -o remount,ro /boot
        
            reboot
        fi
    fi
}


function check_exitstatus {
    STATUS=$1

    case $STATUS in
        9)
            #
            # RX returned with exit code 9, which means the interface went down
            #
            # The wifi card must have been removed during running, so we check if the wifi card is really gone
            #
            NICS2=`ls /sys/class/net/ | nice grep -v eth0 | nice grep -v lo | nice grep -v usb | nice grep -v intwifi | nice grep -v relay | nice grep -v wifihotspot`
            
            if [ "$NICS" == "$NICS2" ]; then
                #
                # Wifi card has not been removed, something else must've gone wrong
                #

                echo "ERROR: RX stopped, wifi card _not_ removed!"
            else
                #
                # Wifi card has been removed
                #

                echo "ERROR: Wifi card removed!"
            fi
        ;;
        2)
            #
            # Something else that is fatal happened during running
            #
            echo "ERROR: RX chain stopped, but not because the wifi card was removed"
        ;;
        1)
            #
            # something that is fatal went wrong at rx startup
            #

            echo "ERROR: could not start RX"
        ;;
        *)
            if [  $RX_EXITSTATUS -lt 128 ]; then
                # 
                # We don't know why, but it failed
                #
                echo -n ""
            fi
        ;;
    esac
}


function check_lifepowered_pi_attached {
    i2cdetect -y 1 | grep  "43"
    grepRet=$?

    if [[ $grepRet -eq 0 ]] ; then
        export LIFEPO4WERED_PI="1"

        systemctl start lifepo4wered-daemon

        if [ "$TTY" == "/dev/tty1" ]; then
            echo "Detected LiFePO4wered Pi power hat"
            qstatus "Detected LiFePO4wered Pi power hat" 5
        fi
    else
        export LIFEPO4WERED_PI="0"
    fi
}


function check_hdmi_csi_attached {
    i2cdetect -y 0 | grep  "0f"
    grepRet=$?

    if [[ $grepRet -eq 0 ]] ; then
        export HDMI_CSI="1"

        if [ "$TTY" == "/dev/tty1" ]; then
            echo "Detected HDMI CSI input board"
            qstatus "Detected HDMI CSI input board" 5
        fi
    else
        export HDMI_CSI="0"
    fi
}


function check_camera_attached {
    #
    # Check if pi camera is detected to determine if we're going to be air or ground
    #
    # Only do this on one tty so that we don't run vcgencmd multiple times, which could make the GPU hang
    #
    if [ "$TTY" == "/dev/tty1" ]; then
        CAM=`/usr/bin/vcgencmd get_camera | python3 -c 'import sys, re; s = sys.stdin.read(); s=re.sub("supported=\d+ detected=", "", s); print(s);'`
        
        if [ "$CAM" == "0" ]; then
            #
            # Used by cameracontrolUDP.py to restrict video mode change
            #
            echo  "1" > /tmp/CameraNotDetected
        else
            qstatus "Detected ${CAM} official Raspberry Pi camera(s)" 5
        fi


        if [ -e /tmp/Air ]; then
            echo "force boot as Air via GPIO"
            qstatus "Force boot as air" 5
            CAM="1"
        fi


        if [ "$CAM" == "0" ]; then
            #
            # No pi camera detected, but we still might have a VEYE, and the only way to detect
            # it is to have i2c_vc enabled already, and then probe the i2c-0 bus
            #
            i2cdetect -y 0 | grep  "30: -- -- -- -- -- -- -- -- -- -- -- 3b -- -- -- --"
            grepRet=$?

            if [[ $grepRet -eq 0 ]] ; then
                echo  "1" > /tmp/cam

                IMX290="1"

                rm /tmp/CameraNotDetected

                CAM="1"

                qstatus "Detected VEYE camera" 5
            else
                echo  "0" > /tmp/cam

                IMX290="0"
            fi
        else
            # 
            # We found a pi camera, so this is definitely air side
            #
            touch /tmp/TX

            echo ${CAM} > /tmp/cam
        fi
    else
        echo -n "Waiting until TX/RX has been determined"

        while [ ! -f /tmp/cam ]; do
            sleep 0.5
        done
        
        CAM=`cat /tmp/cam`
    fi
}


function set_font_for_resolution {
    #
    # Only run on the ground station
    #
    if [ "$CAM" == "0" ]; then
        # 
        # Set font according to display resolution
        #
        # TODO: this may be a good way to set QOpenHD scaling, if we need to do it manually
        #
        
        if [ "$TTY" = "/dev/tty1" ] || [ "$TTY" = "/dev/tty2" ] || [ "$TTY" = "/dev/tty3" ] || [ "$TTY" = "/dev/tty4" ] || [ "$TTY" = "/dev/tty5" ] || [ "$TTY" = "/dev/tty6" ] || [ "$TTY" = "/dev/tty7" ] || [ "$TTY" = "/dev/tty8" ] || [ "$TTY" = "/dev/tty9" ] || [ "$TTY" = "/dev/tty10" ] || [ "$TTY" = "/dev/tty11" ] || [ "$TTY" = "/dev/tty12" ]; then
            
            H_RES=`tvservice -s | cut -f 2 -d "," | cut -f 2 -d " " | cut -f 1 -d "x"`
            
            if [ "$H_RES" -ge "1680" ]; then

                setfont /usr/share/consolefonts/Lat15-TerminusBold24x12.psf.gz
            else
                if [ "$H_RES" -ge "1280" ]; then

                    setfont /usr/share/consolefonts/Lat15-TerminusBold20x10.psf.gz
                else
                    if [ "$H_RES" -ge "800" ]; then

                        setfont /usr/share/consolefonts/Lat15-TerminusBold14.psf.gz
                    fi
                fi
            fi
        fi
    fi
}


function read_config_file {
    if [ -e "/boot/$CONFIGFILE" ]; then

        dos2unix -n /boot/$CONFIGFILE /tmp/settings.sh > /dev/null 2>&1

        OK=`bash -n /tmp/settings.sh`

        if [ "$?" == "0" ]; then
            source /tmp/settings.sh
        else
            echo "ERROR: openhd-settings file contains syntax error(s)!"
            qstatus "ERROR: openhd-settings file contains syntax error(s)!" 3

            collect_errorlog

            sleep 365d
        fi
    else
        echo "ERROR: openhd-settings file not found!"
        qstatus "ERROR: openhd-settings file not found!" 3

        collect_errorlog
        
        sleep 365d
    fi
}


function datarate_to_wifi_settings {
    case $DATARATE in
        1)
            UPLINK_WIFI_BITRATE=11
            TELEMETRY_WIFI_BITRATE=11
            VIDEO_WIFI_BITRATE=5.5
            OSD_HW_BITRATE=5.5

            if [ "$UseMCS" == "Y" ]; then
                UseMCS="1"

                VIDEO_WIFI_BITRATE="0"
            else
                UseMCS="0"
            fi
        ;;
        2)
            UPLINK_WIFI_BITRATE=11
            TELEMETRY_WIFI_BITRATE=11
            VIDEO_WIFI_BITRATE=11
            OSD_HW_BITRATE=11

            if [ "$UseMCS" == "Y" ]; then
                UseMCS="1"

                VIDEO_WIFI_BITRATE="1"
            else
                UseMCS="0"
            fi
        ;;
        3)
            UPLINK_WIFI_BITRATE=11
            TELEMETRY_WIFI_BITRATE=12
            VIDEO_WIFI_BITRATE=12
            OSD_HW_BITRATE=12

            if [ "$UseMCS" == "Y" ]; then
                UseMCS="1"

                VIDEO_WIFI_BITRATE="1"
            else
                UseMCS="0"
            fi
        ;;
        4)
            UPLINK_WIFI_BITRATE=11
            TELEMETRY_WIFI_BITRATE=19.5
            VIDEO_WIFI_BITRATE=19.5
            OSD_HW_BITRATE=19.5

            if [ "$UseMCS" == "Y" ]; then
                UseMCS="1"

                VIDEO_WIFI_BITRATE="2"
            else
                UseMCS="0"
            fi
        ;;
        5)
            UPLINK_WIFI_BITRATE=11
            TELEMETRY_WIFI_BITRATE=24
            VIDEO_WIFI_BITRATE=24
            OSD_HW_BITRATE=24

            if [ "$UseMCS" == "Y" ]; then
                UseMCS="1"

                VIDEO_WIFI_BITRATE="3"
            else
                UseMCS="0"
            fi
        ;;
        6)
            UPLINK_WIFI_BITRATE=12
            TELEMETRY_WIFI_BITRATE=36
            VIDEO_WIFI_BITRATE=36
            OSD_HW_BITRATE=36

            if [ "$UseMCS" == "Y" ]; then
                UseMCS="1"

                VIDEO_WIFI_BITRATE="4"
            else
                UseMCS="0"
            fi
        ;;
    esac


    if [ "$UseSTBC" == "Y" ]; then
        UseSTBC="1"
    else
        UseSTBC="0"
    fi


    if [ "$UseLDPC" == "Y" ]; then
        UseLDPC="1"
    else
        UseLDPC="0"
    fi
}


function set_video_player_based_fps {
    #
    # Select the version of hello_video to use based on the framerate
    #
    # Note that only the 30fps version actually adjusts to match the timing of the video, the 48fps version is just 
    # matching vsync and is named that way because it isn't 30 and it isn't 59.9.
    #
    # For framerates faster than 59.9, we use the special 240fps version.
    # 
    
    if [ "$CAM" == "0" ]; then         
        if [ "$FPS" == "59.9" ]; then

            DISPLAY_PROGRAM=/usr/local/bin/hello_video.bin.48-mm
        else
            if [ "$FPS" -eq 30 ]; then

                DISPLAY_PROGRAM=/usr/local/bin/hello_video.bin.30-mm
            fi

            if [ "$FPS" -lt 60 ]; then

                DISPLAY_PROGRAM=/usr/local/bin/hello_video.bin.48-mm
            fi
            
            if [ "$FPS" -gt 60 ]; then
            
                DISPLAY_PROGRAM=/usr/local/bin/hello_video.bin.240-befi
            fi
        fi
    fi
}



function get_telemetry_settings {
    #
    # This determines how telemetry will be handled in the rest of the system
    # 
    # One of the things it affects is tx_telemetry, which will choose to parse individual messages for
    # mavlink to ensure they get sent out in individual wifi frames
    #
    # The ports here are used when distributing telemetry to hotspot devices using UDPsplitter, with the
    # exception of mavlink which uses mavlink-routerd
    #
    if cat /boot/osdconfig.txt | grep -q "^#define LTM"; then
        TELEMETRY_UDP_PORT=5001
        TELEMETRY_TYPE=1
    fi

    if cat /boot/osdconfig.txt | grep -q "^#define FRSKY"; then
        TELEMETRY_UDP_PORT=5002
        TELEMETRY_TYPE=1
    fi

    if cat /boot/osdconfig.txt | grep -q "^#define SMARTPORT"; then
        TELEMETRY_UDP_PORT=5010
        TELEMETRY_TYPE=1
    fi

    if cat /boot/osdconfig.txt | grep -q "^#define VOT"; then
        TELEMETRY_UDP_PORT=5011
        TELEMETRY_TYPE=1
    fi

    if cat /boot/osdconfig.txt | grep -q "^#define MAVLINK"; then
        TELEMETRY_UDP_PORT=5004
        TELEMETRY_TYPE=0
    fi
}


function set_cts_protection {
    if [ "$CTS_PROTECTION" == "Y" ]; then
        #
        # Use standard data frames, so that CTS is generated for Atheros
        #
        VIDEO_FRAMETYPE=1

        TELEMETRY_CTS=1
    else
        #
        # Use RTS frames for video, CTS for telemetry (only atheros)
        #
        # This is also used when CTS is set to auto or disabled
        #
        VIDEO_FRAMETYPE=2

        TELEMETRY_CTS=1
    fi
    
    if [ "$TXMODE" != "single" ]; then
        #
        # Always use standard data frames in dual TX mode, because Ralink beacon injection is broken
        #
        VIDEO_FRAMETYPE=1

        TELEMETRY_CTS=1
    fi
}


function collect_debug {
    sleep 25

    DEBUGPATH=$1

    if [ "$DEBUGPATH" == "/boot" ]; then
        #
        # If debugpath is the /boot partition, make it writeable first and move old logs
        #
        nice mount -o remount,rw /boot
        mv /boot/debug.txt /boot/debug-old.txt > /dev/null 2>&1
    fi

    uptime >>$DEBUGPATH/debug.txt
    echo >>$DEBUGPATH/debug.txt

    echo -n "Camera: " >>$DEBUGPATH/debug.txt
    nice /usr/bin/vcgencmd get_camera >>$DEBUGPATH/debug.txt
    nice dmesg | nice grep disconnect >>$DEBUGPATH/debug.txt
    nice dmesg | nice grep over-current >>$DEBUGPATH/debug.txt
    echo >>$DEBUGPATH/debug.txt

    nice tvservice -s >>$DEBUGPATH/debug.txt
    echo >>$DEBUGPATH/debug.txt
    nice tvservice -m CEA >>$DEBUGPATH/debug.txt
    echo >>$DEBUGPATH/debug.txt
    nice tvservice -m DMT >>$DEBUGPATH/debug.txt
    echo >>$DEBUGPATH/debug.txt

    nice iwconfig >>$DEBUGPATH/debug.txt > /dev/null 2>&1
    echo >>$DEBUGPATH/debug.txt
    nice ifconfig >>$DEBUGPATH/debug.txt
    echo >>$DEBUGPATH/debug.txt

    nice iw reg get >>$DEBUGPATH/debug.txt
    echo >>$DEBUGPATH/debug.txt

    nice iw list >>$DEBUGPATH/debug.txt
    echo >>$DEBUGPATH/debug.txt

    nice ps ax >>$DEBUGPATH/debug.txt
    echo >>$DEBUGPATH/debug.txt

    nice df -h >>$DEBUGPATH/debug.txt
    echo >>$DEBUGPATH/debug.txt

    nice mount >>$DEBUGPATH/debug.txt
    echo >>$DEBUGPATH/debug.txt

    nice fdisk -l /dev/mmcblk0 >>$DEBUGPATH/debug.txt
    echo >>$DEBUGPATH/debug.txt

    nice lsmod >>$DEBUGPATH/debug.txt
    echo >>$DEBUGPATH/debug.txt

    nice lsusb >>$DEBUGPATH/debug.txt
    nice lsusb -v >>$DEBUGPATH/debug.txt
    echo >>$DEBUGPATH/debug.txt
    nice ls -la /dev >>$DEBUGPATH/debug.txt
    echo >>$DEBUGPATH/debug.txt
    nice ls -la /dev/input >>$DEBUGPATH/debug.txt
    echo >>$DEBUGPATH/debug.txt
    nice vcgencmd measure_temp >>$DEBUGPATH/debug.txt
    nice vcgencmd get_throttled >>$DEBUGPATH/debug.txt
    echo >>$DEBUGPATH/debug.txt
    nice vcgencmd get_config int >>$DEBUGPATH/debug.txt

    echo >>$DEBUGPATH/debug.txt
    nice dmesg >>$DEBUGPATH/debug.txt
    echo >>$DEBUGPATH/debug.txt
    echo >>$DEBUGPATH/debug.txt

    nice cat /etc/modprobe.d/rt2800usb.conf >> $DEBUGPATH/debug.txt
    nice cat /etc/modprobe.d/ath9k_htc.conf >> $DEBUGPATH/debug.txt
    nice cat /etc/modprobe.d/ath9k_hw.conf >> $DEBUGPATH/debug.txt

    echo >>$DEBUGPATH/debug.txt
    echo >>$DEBUGPATH/debug.txt
    nice cat /boot/$CONFIGFILE | egrep -v "^(#|$)" >> $DEBUGPATH/debug.txt
    echo >>$DEBUGPATH/debug.txt
    echo >>$DEBUGPATH/debug.txt
    nice cat /boot/osdconfig.txt | egrep -v "^(//|$)" >> $DEBUGPATH/debug.txt
    echo >>$DEBUGPATH/debug.txt
    echo >>$DEBUGPATH/debug.txt
    nice cat /boot/joyconfig.txt | egrep -v "^(//|$)" >> $DEBUGPATH/debug.txt
    echo >>$DEBUGPATH/debug.txt
    echo >>$DEBUGPATH/debug.txt
    nice cat /boot/apconfig.txt | egrep -v "^(#|$)" >> $DEBUGPATH/debug.txt

    nice top -n 3 -b -d 2 >>$DEBUGPATH/debug.txt

    #
    # If debugpath is the /boot partition, sync and remount read-only
    #
    if [ "$DEBUGPATH" == "/boot" ]; then
        sync
        nice mount -o remount,ro /boot
    fi
}


function collect_errorlog {
    sleep 3
    echo


    if nice dmesg | nice grep -q over-current; then
        echo "ERROR: Over-current detected - potential power supply problems!"
    fi


    # check for USB disconnects (due to power-supply problems)
    if nice dmesg | nice grep -q disconnect; then
        echo "ERROR: USB disconnect detected - potential power supply problems!"
    fi


    nice mount -o remount,rw /boot

    
    #
    # Check to see if the pi is throttled so we can show a warning on the OSD
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
            echo "  | ERROR: Under-Voltage detected on the TX Pi. Your Pi is not supplied with stable 5 Volts.        |"
            echo "  | Either your power-supply or wiring is not sufficent, check the wiring instructions in the Wiki. |"
            echo "  | Video Bitrate will be reduced to 1000kbit to reduce current consumption!                        |"
            echo "  ---------------------------------------------------------------------------------------------------"
            echo
            echo "  ---------------------------------------------------------------------------------------------------" >> /boot/UNDERVOLTAGE-ERROR!!!.txt
            echo "  | ERROR: Under-Voltage detected on the TX Pi. Your Pi is not supplied with stable 5 Volts.        |" >> /boot/UNDERVOLTAGE-ERROR!!!.txt
            echo "  | Either your power-supply or wiring is not sufficent, check the wiring instructions in the Wiki. |" >> /boot/UNDERVOLTAGE-ERROR!!!.txt
            echo "  | Video Bitrate will be reduced to 1000kbit to reduce current consumption!                        |" >> /boot/UNDERVOLTAGE-ERROR!!!.txt
            echo "  | When you have fixed wiring/power-supply, delete this file and make sure it doesn't re-appear!   |" >> /boot/UNDERVOLTAGE-ERROR!!!.txt
            echo "  ---------------------------------------------------------------------------------------------------" >> /boot/UNDERVOLTAGE-ERROR!!!.txt
        fi
    fi

    mv /boot/errorlog.txt /boot/errorlog-old.txt > /dev/null 2>&1
    mv /boot/errorlog.png /boot/errorlog-old.png > /dev/null 2>&1
    echo -n "Camera: "
    nice /usr/bin/vcgencmd get_camera
    uptime >>/boot/errorlog.txt
    echo >>/boot/errorlog.txt
    echo -n "Camera: " >>/boot/errorlog.txt
    nice /usr/bin/vcgencmd get_camera >>/boot/errorlog.txt
    echo
    nice dmesg | nice grep disconnect
    nice dmesg | nice grep over-current
    nice dmesg | nice grep disconnect >>/boot/errorlog.txt
    nice dmesg | nice grep over-current >>/boot/errorlog.txt
    echo >>/boot/errorlog.txt
    echo


    NICS=`ls /sys/class/net/ | nice grep -v eth0 | nice grep -v lo | nice grep -v usb`


    for NIC in $NICS
    do
        iwconfig $NIC | grep $NIC
    done
    
    echo
    echo "Detected USB devices:"
    lsusb

    nice iwconfig >>/boot/errorlog.txt > /dev/null 2>&1
    echo >>/boot/errorlog.txt
    nice ifconfig >>/boot/errorlog.txt
    echo >>/boot/errorlog.txt

    nice iw reg get >>/boot/errorlog.txt
    echo >>/boot/errorlog.txt

    nice iw list >>/boot/errorlog.txt
    echo >>/boot/errorlog.txt


    nice ps ax >>/boot/errorlog.txt
    echo >>/boot/errorlog.txt

    nice df -h >>/boot/errorlog.txt
    echo >>/boot/errorlog.txt

    nice mount >>/boot/errorlog.txt
    echo >>/boot/errorlog.txt

    nice fdisk -l /dev/mmcblk0 >>/boot/errorlog.txt
    echo >>/boot/errorlog.txt

    nice lsmod >>/boot/errorlog.txt
    echo >>/boot/errorlog.txt

    nice lsusb >>/boot/errorlog.txt
    nice lsusb -v >>/boot/errorlog.txt
    echo >>/boot/errorlog.txt
    nice ls -la /dev >>/boot/errorlog.txt
    nice ls -la /dev/input >>/boot/errorlog.txt
    echo
    nice vcgencmd measure_temp
    nice vcgencmd get_throttled
    echo >>/boot/errorlog.txt
    nice vcgencmd measure_volts >>/boot/errorlog.txt
    nice vcgencmd measure_temp >>/boot/errorlog.txt
    nice vcgencmd get_throttled >>/boot/errorlog.txt
    echo >>/boot/errorlog.txt
    nice vcgencmd get_config int >>/boot/errorlog.txt

    nice /usr/bin/raspi2png -p /boot/errorlog.png
    echo >>/boot/errorlog.txt
    nice dmesg >>/boot/errorlog.txt
    echo >>/boot/errorlog.txt
    echo >>/boot/errorlog.txt

    nice cat /etc/modprobe.d/rt2800usb.conf >> /boot/errorlog.txt
    nice cat /etc/modprobe.d/ath9k_htc.conf >> /boot/errorlog.txt
    nice cat /etc/modprobe.d/ath9k_hw.conf >> /boot/errorlog.txt

    echo >>/boot/errorlog.txt
    echo >>/boot/errorlog.txt
    nice cat /boot/$CONFIGFILE | egrep -v "^(#|$)" >> /boot/errorlog.txt
    echo >>/boot/errorlog.txt
    echo >>/boot/errorlog.txt
    nice cat /boot/osdconfig.txt | egrep -v "^(//|$)" >> /boot/errorlog.txt
    echo >>/boot/errorlog.txt
    echo >>/boot/errorlog.txt
    nice cat /boot/joyconfig.txt | egrep -v "^(//|$)" >> /boot/errorlog.txt
    echo >>/boot/errorlog.txt
    echo >>/boot/errorlog.txt
    nice cat /boot/apconfig.txt | egrep -v "^(#|$)" >> /boot/errorlog.txt

    sync
    nice mount -o remount,ro /boot
}



function wbclogger_function {
    #
    # Only run on the ground side
    #
    if [ "$CAM" == "0" ]; then
        #
        # Waiting until video is running ...
        #
        VIDEORXRUNNING=0
        while [ $VIDEORXRUNNING -ne 1 ]; do
            VIDEORXRUNNING=`pidof $DISPLAY_PROGRAM | wc -w`
            sleep 1
        done

    
        echo
        
        sleep 5
        
        #
        # Start saving Open.HD telemetry to the temporary area, so it can be copied to the USB drive after flight
        #
        nice /usr/local/bin/rssilogger /wifibroadcast_rx_status_0 >> /wbc_tmp/videorssi.csv &
        nice /usr/local/bin/rssilogger /wifibroadcast_rx_status_1 >> /wbc_tmp/telemetrydownrssi.csv &
        nice /usr/local/bin/syslogger /wifibroadcast_rx_status_sysair >> /wbc_tmp/system.csv &

        if [ "$TELEMETRY_UPLINK" != "disabled" ]; then
            nice /usr/local/bin/rssilogger /wifibroadcast_rx_status_uplink >> /wbc_tmp/telemetryuprssi.csv &
        fi

        if [ "$RC" != "disabled" ]; then
            nice /usr/local/bin/rssilogger /wifibroadcast_rx_status_rc >> /wbc_tmp/rcrssi.csv &
        fi

        if [ "$DEBUG" == "Y" ]; then
            nice /usr/local/bin/wifibackgroundscan $NICS >> /wbc_tmp/wifibackgroundscan.csv &
        fi

        sleep 365d
    fi
}


function pause_while {
    if [ -f "/tmp/pausewhile" ]; then
        PAUSE=1
        while [ $PAUSE -ne 0 ]; do
            if [ ! -f "/tmp/pausewhile" ]; then
                PAUSE=0
            fi
            sleep 1
        done
    fi
}


function detect_nics {
    NICS=`ls /sys/class/net/ | nice grep -v eth0 | nice grep -v lo | nice grep -v usb | nice grep -v intwifi | nice grep -v relay | nice grep -v wifihotspot`
}


function prepare_nic {
    DRIVER=`cat /sys/class/net/$1/device/uevent | nice grep DRIVER | sed 's/DRIVER=//'`
    
    if [ "$DRIVER" != "rtl88xxau" ] && [ "$DRIVER" != "rtl88XXau" ] && [ "$DRIVER" != "rt2800usb" ] && [ "$DRIVER" != "mt7601u" ] && [ "$DRIVER" != "ath9k_htc" ]; then
        tmessage "WARNING: Unsupported or experimental wifi card: $DRIVER"
        qstatus "WARNING: Unsupported or experimental wifi card: $DRIVER" 4
    fi

    case $DRIVER in
        *881[24]au)
            DRIVER=rtl88XXau
            ;;
        rtl88xxau)
            DRIVER=rtl88XXau
            ;;
    esac

    tmessage -n "Setting up $1: "
    
    if [ "$DRIVER" == "ath9k_htc" ]; then 

        ifconfig $1 up || {
            echo
            echo "ERROR: Bringing up interface $1 failed!"
            qstatus "ERROR: Bringing up interface $1 failed!" 3
            collect_errorlog
            sleep 365d
        }

        sleep 0.2

        if [ "$CAM" == "0" ]; then
            #
            # Running on ground side, set bitrate to uplink bitrate
            #

            iw dev $1 set bitrates legacy-2.4 $UplinkSpeed || {
                echo
                echo "ERROR: Setting bitrate on $1 failed!"
                qstatus "ERROR: Setting bitrate on $1 failed!" 3
                
                collect_errorlog
                
                sleep 365d
            }

            sleep 0.2
            
        else 
            #
            # Running on air side, set bitrate to downstream bitrate
            #
            tmessage -n "bitrate "
            
            if [ "$VIDEO_WIFI_BITRATE" != "19.5" ]; then
                #
                # Only set bitrate if not configured for 19.5 (19.5 is the default in ath9k_htc firmware)
                #
                tmessage -n "$VIDEO_WIFI_BITRATE Mbit "
                
                iw dev $1 set bitrates legacy-2.4 $VIDEO_WIFI_BITRATE || {
                    echo
                    echo "ERROR: Setting bitrate on $1 failed!"
                    qstatus "ERROR: Setting bitrate on $1 failed!" 3

                    collect_errorlog
                    
                    sleep 365d
                }
            else
                tmessage -n "$VIDEO_WIFI_BITRATE Mbit "
            fi
        
            sleep 0.2

            tmessage -n "done. "
        fi


        #
        # Bring the interface down again so we can configure monitor mode
        # 
        ifconfig $1 down || {
            echo
            echo "ERROR: Bringing down interface $1 failed!"
            qstatus "ERROR: Bringing down interface $1 failed!" 3

            collect_errorlog
            
            sleep 365d
        }
        sleep 0.2


        tmessage -n "monitor mode.. "
        iw dev $1 set monitor none || {
            echo
            echo "ERROR: Setting monitor mode on $1 failed!"
            qstatus "ERROR: Setting monitor mode on $1 failed!" 3

            collect_errorlog
            
            sleep 365d
        }

        
        sleep 0.2
        tmessage -n "done. "


        #
        # Now that monitor mode is configured, we bring the interface up again
        # 
        ifconfig $1 up || {
            echo
            echo "ERROR: Bringing up interface $1 failed!"
            qstatus "ERROR: Bringing up interface $1 failed!" 3
            
            collect_errorlog

            sleep 365d
        }
        sleep 0.2

        #
        # Configure the interface with the wifi frequency supplied to this function as the 2nd argument
        #
        if [ "$2" != "0" ]; then
            tmessage -n "frequency $2 MHz.. "

            iw dev $1 set freq $2 || {
                echo
                echo "ERROR: Setting frequency $2 MHz on $1 failed!"
                qstatus "ERROR: Setting frequency $2 Mhz on $1 failed!" 3

                collect_errorlog
                
                sleep 365d
            }

            tmessage "done!"
        else
            echo
        fi
    fi


    if [ "$DRIVER" == "rt2800usb" ] || [ "$DRIVER" == "mt7601u" ] || [ "$DRIVER" == "rtl8192cu" ] || [ "$DRIVER" == "rtl88XXau" ] || [ "$DRIVER" == "rtl88x2bu" ] || [ "$DRIVER" == "8188eu" ] || [ "$DRIVER" == "rtl8188eu" ]; then
        #
        # Do not set the bitrate for Ralink, Mediatek, Realtek, those are handled through tx parameter
        #

        tmessage -n "monitor mode.. "
        
        iw dev $1 set monitor none || {
            echo
            echo "ERROR: Setting monitor mode on $1 failed!"
            qstatus "ERROR: Setting monitor mode on $1 failed!" 3

            collect_errorlog
            
            sleep 365d
        }

        sleep 0.2
        tmessage -n "done. "


        #tmessage -n "bringing up.. "
        ifconfig $1 up || {
            echo
            echo "ERROR: Bringing up interface $1 failed!"
            qstatus "ERROR: Bringing up interfce $1 failed!" 3

            collect_errorlog
            
            sleep 365d
        }

        sleep 0.2
        #tmessage -n "done. "


        if [ "$2" != "0" ]; then
            tmessage -n "frequency $2 MHz.. "

            iw dev $1 set freq $2 || {
                echo
                echo "ERROR: Setting frequency $2 MHz on $1 failed!"
                qstatus "ERROR: Setting frequency $2 MHz on $1 failed!" 3

                collect_errorlog
                
                sleep 365d
            }

            tmessage "done!"
        else
            echo
        fi

        #
        # Configure the interface with the power level supplied to this function as the 3rd argument
        #
        if  [ "$DRIVER" == "rtl88XXau" -a -n "$3" ]; then
            tmessage -n "TX power $3.. "

            iw dev $1 set txpower fixed $3 || {
                echo
                echo "ERROR: Setting TX power to $3 on $1 failed!"
                qstatus "ERROR: Setting TX power to $3 on $1 failed!" 3

                collect_errorlog
                
                sleep 365d
            }

            sleep 0.2
            
            tmessage -n "done. "
        fi
    fi
}
