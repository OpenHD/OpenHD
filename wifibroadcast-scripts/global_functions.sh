function tmessage {
    if [ "$QUIET" == "N" ]; then
        echo $1 "$2"
    fi
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

function detect_memory {
    TOTAL_MEMORY=$(cat /proc/meminfo | grep 'MemTotal' | awk '{print $2}')
}

function detect_hardware {
    HARDWARE=$(cat /proc/cpuinfo | grep 'Revision' | awk '{print $3}')

    echo "Found hardware $HARDWARE..."
          
    case "$HARDWARE" in
        'b03111')
            ABLE_BAND=ag
            MODEL=4b
            ;;
        '29020e0')
            ABLE_BAND=ag
            MODEL=3a+
            ;;
        '2a02082')
            ABLE_BAND=g
            MODEL=3b
            ;;
        '2a22082')
            ABLE_BAND=g
            MODEL=3b
            ;;
        '2a32082')
            ABLE_BAND=g
            MODEL=3b
            ;;		
        '2a52082')
            ABLE_BAND=g
            MODEL=3b
            ;;	
        '2a020d3')
            ABLE_BAND=ag
            MODEL=3b+
            ;;
        '2900092')
            ABLE_BAND=none
            MODEL=Zero
            ;;
        '2900093')
            ABLE_BAND=none
            MODEL=Zero
            ;;
        '29000c1')
            ABLE_BAND=ag
            MODEL=ZeroW
            ;;
        '2920092')
            ABLE_BAND=none
            MODEL=Zero
            ;;
        '2920093')
            ABLE_BAND=none
            MODEL=Zero
            ;;
        '2a22042')
            ABLE_BAND=none
            MODEL=2b
            ;;
        '2a21041')
            ABLE_BAND=none
            MODEL=2b
            ;;
        '2a01041')
            ABLE_BAND=none
            MODEL=2b
            ;;
        '2a01040')
            ABLE_BAND=none
            MODEL=2b
            ;;
        *)
            ABLE_BAND=unknown
            MODEL=unknown
            ;;
    esac
    echo "This pi model is $MODEL..."
}

# If a /boot/i2c_vc exists, we check to see if i2c_vc is already enabled in
# config.txt, and if not we enable it and reboot. This should not add more
# than a few seconds to the air boot time, and only needs to be done once, 
# but IMX290 and other 3rd party cameras won't work without it
function autoenable_i2c_vc {
    # only run this on tty1
    if [ "$TTY" != "/dev/tty1" ]; then
        return
    fi

    if [ -e "/boot/i2c_vc" ]; then
        
        I2C_VC_ENABLED=$(cat /boot/config.txt | grep "^dtparam=i2c_vc" | wc -l)
        
        if [ $I2C_VC_ENABLED == "0" ]; then
        
            I2C_VC_DISABLED=$(cat /boot/config.txt | grep "^#dtparam=i2c_vc" | wc -l)
            
            mount -o remount,rw /boot
        
            if [ $I2C_VC_DISABLED == "1" ]; then
                # it's present but disabled, we can use sed it to enable it in-place
                sed -i 's/^#dtparam=i2c_vc.*/dtparam=i2c_vc=on/g' /boot/config.txt
            else
                # it's not in the file at all so we'll have to add it manually, but at the end of the
                # file with an [all] section to ensure that the setting applies to ever model in case
                # the config.txt file has sections like [pi3] [pi0] at the end.
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
            # rx returned with exit code 9 = the interface went down
            # wifi card must've been removed during running
            # check if wifi card is really gone
            NICS2=`ls /sys/class/net/ | nice grep -v eth0 | nice grep -v lo | nice grep -v usb | nice grep -v intwifi | nice grep -v relay | nice grep -v wifihotspot`
            if [ "$NICS" == "$NICS2" ]; then
                # wifi card has not been removed, something else must've gone wrong
                echo "ERROR: RX stopped, wifi card _not_ removed!             "
            else
                # wifi card has been removed
                echo "ERROR: Wifi card removed!                               "
            fi
            ;;
        2)
            # something else that is fatal happened during running
            echo "ERROR: RX chain stopped wifi card _not_ removed!             "
            ;;
        1)
            # something that is fatal went wrong at rx startup
            echo "ERROR: could not start RX                           "
            #echo "ERROR: could not start RX                           "
            ;;
        *)
            if [  $RX_EXITSTATUS -lt 128 ]; then
                # whatever it was ...
                #echo "RX exited with status: $RX_EXITSTATUS                        "
                echo -n ""
            fi
            ;;
    esac
}

function check_hdmi_csi_attached {
    i2cdetect -y 0 | grep  "0f"
    grepRet=$?

    if [[ $grepRet -eq 0 ]] ; then
        export HDMI_CSI="1"
        echo "Detected HDMI CSI input board"
    else
        export HDMI_CSI="0"
    fi
}

function check_camera_attached {
    # check if cam is detected to determine if we're going to be RX or TX
    # only do this on one tty so that we don't run vcgencmd multiple times (which may make it hang)
    
    if [ "$TTY" == "/dev/tty1" ]; then
        CAM=`/usr/bin/vcgencmd get_camera | python3 -c 'import sys, re; s = sys.stdin.read(); s=re.sub("supported=1 detected=", "", s); print(s);'`
        
        if [ "$CAM" == "0" ]; then
            #is use by cameracontrolUDP.py to restricted video mode change
            echo  "1" > /tmp/CameraNotDetected
        fi

        if [ -e /tmp/Air ]; then
            echo "force boot as Air via GPIO"
            CAM="1"
        fi
        
        if [ "$CAM" == "0" ]; then # if we are RX ...
            i2cdetect -y 0 | grep  "30: -- -- -- -- -- -- -- -- -- -- -- 3b -- -- -- --"
            grepRet=$?
            if [[ $grepRet -eq 0 ]] ; then
                echo  "1" > /tmp/cam
                IMX290="1"
                rm /tmp/CameraNotDetected
                CAM="1"
            else
                echo  "0" > /tmp/cam
                IMX290="0"
            fi
        else # else we are TX ...
            touch /tmp/TX
            echo ${CAM} > /tmp/cam
        fi
    else
        echo -n "Waiting until TX/RX has been determined"
        while [ ! -f /tmp/cam ]; do
            sleep 0.5
            #echo -n "."
        done
        CAM=`cat /tmp/cam`
    fi
}

function set_font_for_resolution {
    if [ "$CAM" == "0" ]; then 
        # if we are RX ...
        
        # if local TTY, set font according to display resolution
        
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
            collect_errorlog
            sleep 365d
        fi
    else
        echo "ERROR: openhd-settings file not found!"
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
    # mmormota's stutter-free hello_video.bin: "hello_video.bin.30-mm" (for 30fps) or "hello_video.bin.48-mm" (for 48 and 59.9fps)
    # befinitiv's hello_video.bin: "hello_video.bin.240-befi" (for any fps, use this for higher than 59.9fps)
    
    if [ "$CAM" == "0" ]; then 
        # if we are RX ...
        
        if [ "$FPS" == "59.9" ]; then
            DISPLAY_PROGRAM=/opt/vc/src/hello_pi/hello_video/hello_video.bin.48-mm
        else
            if [ "$FPS" -eq 30 ]; then
                DISPLAY_PROGRAM=/opt/vc/src/hello_pi/hello_video/hello_video.bin.30-mm
            fi

            if [ "$FPS" -lt 60 ]; then
                DISPLAY_PROGRAM=/opt/vc/src/hello_pi/hello_video/hello_video.bin.48-mm
            fi
            
            if [ "$FPS" -gt 60 ]; then
                DISPLAY_PROGRAM=/opt/vc/src/hello_pi/hello_video/hello_video.bin.240-befi
            fi
        fi
    fi
}

function get_telemetry_settings {
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
        # use standard data frames, so that CTS is generated for Atheros
        VIDEO_FRAMETYPE=1
        TELEMETRY_CTS=1
    else
        # auto or N
        # use RTS frames for video, CTS for telemetry (only atheros anyway)
        VIDEO_FRAMETYPE=2
        TELEMETRY_CTS=1
    fi
    
    if [ "$TXMODE" != "single" ]; then
        # always type 1 in dual tx mode since ralink beacon injection broken
        VIDEO_FRAMETYPE=1
        TELEMETRY_CTS=1
    fi
}

function collect_debug {
    sleep 25

    DEBUGPATH=$1
    if [ "$DEBUGPATH" == "/boot" ]; then
        # if debugpath is boot partition, make it writeable first and move old logs
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
    nice cat /boot/openhd-settings-1.txt | egrep -v "^(#|$)" >> $DEBUGPATH/debug.txt
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

    if [ "$DEBUGPATH" == "/boot" ]; then # if debugpath is boot partition, sync and remount ro
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

    # check if over-temp or under-voltage occured
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

    nice /home/pi/wifibroadcast-misc/raspi2png -p /boot/errorlog.png
    echo >>/boot/errorlog.txt
    nice dmesg >>/boot/errorlog.txt
    echo >>/boot/errorlog.txt
    echo >>/boot/errorlog.txt

    nice cat /etc/modprobe.d/rt2800usb.conf >> /boot/errorlog.txt
    nice cat /etc/modprobe.d/ath9k_htc.conf >> /boot/errorlog.txt
    nice cat /etc/modprobe.d/ath9k_hw.conf >> /boot/errorlog.txt

    echo >>/boot/errorlog.txt
    echo >>/boot/errorlog.txt
    nice cat /boot/openhd-settings-1.txt | egrep -v "^(#|$)" >> /boot/errorlog.txt
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
    if [ "$CAM" == "0" ]; then
        # if we are RX ...

        if [ "$ENABLE_QOPENHD" != "Y" ]; then
            # Waiting until video is running ...
            VIDEORXRUNNING=0
            while [ $VIDEORXRUNNING -ne 1 ]; do
                VIDEORXRUNNING=`pidof $DISPLAY_PROGRAM | wc -w`
                sleep 1
            done
        fi
    
        echo
        
        sleep 5
        
        nice /home/pi/wifibroadcast-base/rssilogger /wifibroadcast_rx_status_0 >> /wbc_tmp/videorssi.csv &
        nice /home/pi/wifibroadcast-base/rssilogger /wifibroadcast_rx_status_1 >> /wbc_tmp/telemetrydownrssi.csv &
        nice /home/pi/wifibroadcast-base/syslogger /wifibroadcast_rx_status_sysair >> /wbc_tmp/system.csv &

        if [ "$TELEMETRY_UPLINK" != "disabled" ]; then
            nice /home/pi/wifibroadcast-base/rssilogger /wifibroadcast_rx_status_uplink >> /wbc_tmp/telemetryuprssi.csv &
        fi

        if [ "$RC" != "disabled" ]; then
            nice /home/pi/wifibroadcast-base/rssilogger /wifibroadcast_rx_status_rc >> /wbc_tmp/rcrssi.csv &
        fi

        if [ "$DEBUG" == "Y" ]; then
            nice /home/pi/wifibroadcast-base/wifibackgroundscan $NICS >> /wbc_tmp/wifibackgroundscan.csv &
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
    
    if [ "$DRIVER" != "rt2800usb" ] && [ "$DRIVER" != "mt7601u" ] && [ "$DRIVER" != "ath9k_htc" ]; then
        tmessage "WARNING: Unsupported or experimental wifi card: $DRIVER"
    fi

    case $DRIVER in
        *881[24]au)
        DRIVER=rtl88xxau
        ;;
    esac

    tmessage -n "Setting up $1: "
    
    if [ "$DRIVER" == "ath9k_htc" ]; then 
        # set bitrates for Atheros via iw
        ifconfig $1 up || {
            echo
            echo "ERROR: Bringing up interface $1 failed!"
            collect_errorlog
            sleep 365d
        }
        sleep 0.2

        if [ "$CAM" == "0" ]; then
            # we are RX, set bitrate to uplink bitrate
            #tmessage -n "bitrate $UPLINK_WIFI_BITRATE Mbit "
            iw dev $1 set bitrates legacy-2.4 $UplinkSpeed || {
                echo
                echo "ERROR: Setting bitrate on $1 failed!"
                collect_errorlog
                sleep 365d
            }
            sleep 0.2
            #tmessage -n "done. "
        else 
            # we are TX, set bitrate to downstream bitrate
            tmessage -n "bitrate "
            
            if [ "$VIDEO_WIFI_BITRATE" != "19.5" ]; then
                # only set bitrate if something else than 19.5 is requested (19.5 is default compiled in ath9k_htc firmware)
                tmessage -n "$VIDEO_WIFI_BITRATE Mbit "
                iw dev $1 set bitrates legacy-2.4 $VIDEO_WIFI_BITRATE || {
                    echo
                    echo "ERROR: Setting bitrate on $1 failed!"
                    collect_errorlog
                    sleep 365d
                }
            else
                tmessage -n "$VIDEO_WIFI_BITRATE Mbit "
            fi
        
            sleep 0.2
            tmessage -n "done. "
        fi

        ifconfig $1 down || {
            echo
            echo "ERROR: Bringing down interface $1 failed!"
            collect_errorlog
            sleep 365d
        }
        sleep 0.2

        tmessage -n "monitor mode.. "
        iw dev $1 set monitor none || {
            echo
            echo "ERROR: Setting monitor mode on $1 failed!"
            collect_errorlog
            sleep 365d
        }
        
        sleep 0.2
        tmessage -n "done. "

        ifconfig $1 up || {
            echo
            echo "ERROR: Bringing up interface $1 failed!"
            collect_errorlog
            sleep 365d
        }
        sleep 0.2

        if [ "$2" != "0" ]; then
            tmessage -n "frequency $2 MHz.. "
            iw dev $1 set freq $2 || {
                echo
                echo "ERROR: Setting frequency $2 MHz on $1 failed!"
                collect_errorlog
                sleep 365d
            }
            tmessage "done!"
        else
            echo
        fi
    fi

    if [ "$DRIVER" == "rt2800usb" ] || [ "$DRIVER" == "mt7601u" ] || [ "$DRIVER" == "rtl8192cu" ] || [ "$DRIVER" == "rtl88xxau" ]; then
        # do not set bitrate for Ralink, Mediatek, Realtek, done through tx parameter
        tmessage -n "monitor mode.. "
        iw dev $1 set monitor none || {
            echo
            echo "ERROR: Setting monitor mode on $1 failed!"
            collect_errorlog
            sleep 365d
        }
        sleep 0.2
        tmessage -n "done. "

        #tmessage -n "bringing up.. "
        ifconfig $1 up || {
            echo
            echo "ERROR: Bringing up interface $1 failed!"
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
            collect_errorlog
            sleep 365d
            }
            tmessage "done!"
        else
            echo
        fi

        if  [ "$DRIVER" == "rtl88xxau" -a -n "$3" ]; then
            tmessage -n "TX power $3.. "
            iw dev $1 set txpower fixed $3 || {
                echo
                echo "ERROR: Setting TX power to $3 on $1 failed!"
                collect_errorlog
                sleep 365d
            }
            sleep 0.2
            tmessage -n "done. "
        fi
    fi
}
