#!/bin/bash

export PATH=/usr/local/bin:${PATH}

if [ "$CAM" == "0" ]; then
    CONFIGFILE=`/usr/local/bin/gpio-config.py`
else
    #
    # Air side has no concept of profiles, it is always just one settings file being overwritten
    #
    CONFIGFILE="openhd-settings-1.txt"
fi

source /usr/local/share/wifibroadcast-scripts/global_functions.sh


function datarate_to_wifi_settings {
    case $DATARATE in
        1)
            UPLINK_WIFI_BITRATE=11
            TELEMETRY_WIFI_BITRATE=11
            VIDEO_WIFI_BITRATE=5.5
        ;;
        2)
            UPLINK_WIFI_BITRATE=11
            TELEMETRY_WIFI_BITRATE=11
            VIDEO_WIFI_BITRATE=11
        ;;
        3)
            UPLINK_WIFI_BITRATE=11
            TELEMETRY_WIFI_BITRATE=12
            VIDEO_WIFI_BITRATE=12
        ;;
        4)
            UPLINK_WIFI_BITRATE=11
            TELEMETRY_WIFI_BITRATE=19.5
            VIDEO_WIFI_BITRATE=19.5
        ;;
        5)
            UPLINK_WIFI_BITRATE=11
            TELEMETRY_WIFI_BITRATE=24
            VIDEO_WIFI_BITRATE=24
        ;;
        6)
            UPLINK_WIFI_BITRATE=12
            TELEMETRY_WIFI_BITRATE=36
            VIDEO_WIFI_BITRATE=36
        ;;
    esac
}



function collect_errorlog {

    sleep 3
    echo
    
    if nice dmesg | nice grep -q over-current; then
        echo "ERROR: Over-current detected - potential power supply problems!"
    fi

    #
    # Check for USB disconnects (due to power-supply problems)
    #
    if nice dmesg | nice grep -q disconnect; then
        echo "ERROR: USB disconnect detected - potential power supply problems!"
    fi

    nice mount -o remount,rw /boot

    #
    # Check if over-temp or under-voltage occured
    #
    if vcgencmd get_throttled | nice grep -q -v "0x0"; then
        TEMP=`cat /sys/class/thermal/thermal_zone0/temp`
        TEMP_C=$(($TEMP/1000))

        if [ "$TEMP_C" -lt 75 ]; then
            #
            # It must be under-voltage
            #
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



function read_config_file {
    if [ -e "/boot/${CONFIGFILE}" ]; then
        dos2unix -n /boot/$CONFIGFILE /tmp/settings.sh > /dev/null 2>&1
    
        OK=`bash -n /tmp/settings.sh`
        
        if [ "$?" == "0" ]; then
            source /tmp/settings.sh
        else
            qstatus "Could not read settings file" 3
            collect_errorlog
            sleep 365d
        fi
    else
        qstatus "Settings file missing" 3
        collect_errorlog
        sleep 365d
    fi
}


function detect_nics {
    qstatus "Setting up wifi cards" 5

    #
    # Set reg domain to DE to allow channel 12 and 13 for hotspot
    #
    iw reg set DE

    NUM_CARDS=-1
    NICSWL=`ls /sys/class/net | nice grep wlan`

    for NIC in $NICSWL
    do
        #
        # Set MTU to 2304
        #
        ifconfig $NIC mtu 2304
        
        #
        # Rename wifi interface to MAC address
        #
        NAME=`cat /sys/class/net/$NIC/address`
        ip link set $NIC name ${NAME//:}

        let "NUM_CARDS++"
        #sleep 0.1
    done

    if [ "$NUM_CARDS" == "-1" ]; then
        qstatus "No wifi cards detected" 1
        collect_errorlog
        sleep 365d
    fi



    if [ "$CAM" == "0" ]; then
        #
        # Get wifi hotspot card out of the way
        #
        if [ "$WIFI_HOTSPOT" != "N" ]; then
            if [ "$WIFI_HOTSPOT_NIC" != "internal" ]; then
                #
                # Only configure it if it's there
                #
                if ls /sys/class/net/ | grep -q $WIFI_HOTSPOT_NIC; then
                    qstatus "Set up card $WIFI_HOTSPOT_NIC for hotspot" 5
                    ip link set $WIFI_HOTSPOT_NIC name wifihotspot0
                    ifconfig wifihotspot0 192.168.2.1 up
                    let "NUM_CARDS--"
                else
                    qstatus "Hotspot card $WIFI_HOTSPOT_NIC not found" 3
                    sleep 0.5
                fi
            else
                #
                # Only configure it if it's there
                #
                if ls /sys/class/net/ | grep -q intwifi0; then
                    qstatus "Set up internal wifi for hotspot" 5
                    ip link set intwifi0 name wifihotspot0
                    ifconfig wifihotspot0 192.168.2.1 up
                else
                    qstatus "Internal wifi card not found, hotspot disabled" 3
                    sleep 0.5
                fi
            fi
        fi
        
        #
        # Get relay card out of the way
        #
        
        if [ "$RELAY" == "Y" ]; then
            # 
            # Only configure it if it's there
            #
            if ls /sys/class/net/ | grep -q $RELAY_NIC; then
                qstatus "Set up card $RELAY_NIC for relay" 5
                ip link set $RELAY_NIC name relay0
                prepare_nic relay0 $RELAY_FREQ
                let "NUM_CARDS--"
            else
                qstatus "Relay card $RELAY_NIC not found" 3
                sleep 0.5
            fi
        fi
    fi

    NICS=`ls /sys/class/net/ | nice grep -v eth0 | nice grep -v lo | nice grep -v usb | nice grep -v intwifi | nice grep -v relay | nice grep -v wifihotspot`
    
    #echo "NICS: $NICS"

    if [ "$TXMODE" != "single" ]; then
        for i in $(eval echo {0..$NUM_CARDS})
        do
            if [ "$CAM" == "0" ]; then
                prepare_nic ${MAC_RX[$i]} ${FREQ_RX[$i]}
            else
                prepare_nic ${MAC_TX[$i]} ${FREQ_TX[$i]}
            fi
            
            sleep 0.1
        done
    else
        #
        # Check if auto scan is enabled, if yes, set freq to 0 to let prepare_nic know not to set channel
        #
       
        if [ "$FREQSCAN" == "Y" ] && [ "$CAM" == "0" ]; then
            for NIC in $NICS
            do
                prepare_nic $NIC 2484
                sleep 0.1
            done
            
            #
            # Make sure check_alive function doesnt restart hello_video while we are still scanning for channel
            #
            touch /tmp/pausewhile
           
            /usr/local/bin/rx -p 0 -d 1 -b $VIDEO_BLOCKS -r $VIDEO_FECS -f $VIDEOBLOCKLENGTH $NICS >/dev/null &
            
            sleep 0.5
            
            qstatus "Scanning for nearby transmitters" 5
            FREQ=0

            if iw list | nice grep -q 5180; then 
                #
                # Cards support 5G and 2.4G
                #
                FREQCMD="/usr/local/bin/channelscan 245 $NICS"
            else
                if iw list | nice grep -q 2312; then 
                    #
                    # Cards support 2.3G and 2.4G
                    #
                    FREQCMD="/usr/local/bin/channelscan 2324 $NICS"
                else 
                    #
                    # Cards support only 2.4G
                    #
                    FREQCMD="/usr/local/bin/channelscan 24 $NICS"
                fi
            fi

            while [ $FREQ -eq 0 ]; do
                FREQ=`$FREQCMD`
            done

            qstatus "Transmitter found on ${FREQ}MHz" 5
            echo
            ps -ef | nice grep "rx -p 0" | nice grep -v grep | awk '{print $2}' | xargs kill -9
            for NIC in $NICS
            do
                qstatus "Setting card $NIC to ${FREQ}MHz" 5
                iw dev $NIC set freq $FREQ
                sleep 0.1
            done
            
            #
            # All done
            #
            rm /tmp/pausewhile
        else
            for NIC in $NICS
            do
                prepare_nic $NIC $FREQ "$TXPOWER"
                sleep 0.1
            done
        fi
    fi

    echo $NICS > /tmp/NICS_LIST

    #
    # Let other processes know nics are setup and ready
    #
    touch /tmp/nics_configured 
}




function prepare_nic {
    DRIVER=`cat /sys/class/net/${1}/device/uevent | nice grep DRIVER | sed 's/DRIVER=//'`
    
    if [ "$DRIVER" != "rtl88xxau" ] && [ "$DRIVER" != "rtl88XXau" ] && [ "$DRIVER" != "rt2800usb" ] && [ "$DRIVER" != "mt7601u" ] && [ "$DRIVER" != "ath9k_htc" ]; then
        qstatus "Unsupported card: $DRIVER" 3
    fi

    case $DRIVER in
        *881[24]au)
            DRIVER=rtl88XXau
            ;;
        rtl88xxau)
            DRIVER=rtl88XXau
            ;;
    esac

    qstatus "Setting up interface $1" 5
    
    if [ "$DRIVER" == "ath9k_htc" ]; then 
        #
        # set bitrates for Atheros via iw
        #
        ifconfig $1 up || {
            qstatus "Setting up card $1 failed" 1
            collect_errorlog
            sleep 365d
        }
        
        sleep 0.2

        if [ "$CAM" == "0" ]; then 
            #
            # We are RX, set bitrate to uplink bitrate
            #
            
            #tmessage -n "bitrate $UPLINK_WIFI_BITRATE Mbit "
            
            iw dev $1 set bitrates legacy-2.4 $UplinkSpeed || {
                qstatus "Setting datarate on wifi card $1 failed" 3
                #collect_errorlog
                #sleep 365d
            }
            sleep 0.2
            #tmessage -n "done. "
        else 
            #
            # We are TX, set bitrate to downstream bitrate
            #
            if [ "$VIDEO_WIFI_BITRATE" != "19.5" ]; then
                #
                # Only set bitrate if something else than 19.5 is requested
                # 
                # 19.5 is default compiled in ath9k_htc firmware
                #
                qstatus "Setting card $1 to ${VIDEO_WIFI_BITRATE}Mbit" 5
                iw dev $1 set bitrates legacy-2.4 $VIDEO_WIFI_BITRATE || {
                    qstatus "Setting card $1 to ${VIDEO_WIFI_BITRATE} failed" 3
                    collect_errorlog
                    sleep 365d
                }
            else
                qstatus "Setting card $1 to ${VIDEO_WIFI_BITRATE}Mbit" 5
            fi
            
            sleep 0.2            
        fi

        qstatus "Disabling card $1 to set monitor mode" 5

        ifconfig $1 down || {
            qstatus "Disabling card $1 failed" 1
            collect_errorlog
            sleep 365d
        }
        
        sleep 0.2
        qstatus "Setting card $1 to monitor mode" 5
        

        iw dev $1 set monitor none || {
            qstatus "Setting card $1 to monitor mode failed" 1
            collect_errorlog
            sleep 365d
        }
        
        sleep 0.2

        ifconfig $1 up || {
            qstatus "Re-enabling card $1 failed" 1
            collect_errorlog
            sleep 365d
        }
        
        sleep 0.2


        if [ "$2" != "0" ]; then
            qstatus "Setting card $1 to ${2}MHz" 5
            
            iw dev $1 set freq $2 || {
                qstatus "Setting card $1 to ${2}MHz failed" 1
                collect_errorlog
                sleep 365d
            }
        else
            echo
        fi
    fi



    if [ "$DRIVER" == "rt2800usb" ] || [ "$DRIVER" == "mt7601u" ] || [ "$DRIVER" == "rtl8192cu" ] || [ "$DRIVER" == "rtl88XXau" ] || [ "$DRIVER" == "rtl88x2bu" ] || [ "$DRIVER" == "8188eu" ] || [ "$DRIVER" == "rtl8188eu" ]; then 
        #
        # do not set bitrate for Ralink, Mediatek, Realtek, done through tx parameter
        #
        qstatus "Setting card $1 to monitor mode" 5
        
        iw dev $1 set monitor none || {
            qstatus "Setting card $1 to monitor mode failed" 1
            collect_errorlog
            sleep 365d
        }
        

        sleep 0.2
        
        #tmessage -n "bringing up.. "
        ifconfig $1 up || {
            qstatus "Re-enabling card $1 failed" 1
            collect_errorlog
            sleep 365d
        }
        
        sleep 0.2
        #tmessage -n "done. "


        if [ "$2" != "0" ]; then
            qstatus "Setting card $1 to ${2}MHz" 5
            
            iw dev $1 set freq $2 || {
                qstatus "Setting card $1 to ${2}MHz failed" 1
                collect_errorlog
                sleep 365d
            }
        fi

        if  [ "$DRIVER" == "rtl88XXau" -a -n "$3" ]; then
            qstatus "Setting card $1 to TX power ${3}" 5
            
            iw dev $1 set txpower fixed $3 || {
                qstatus "Setting card $1 to TX power ${3} failed" 1
                collect_errorlog
                sleep 365d
            }
        
            sleep 0.2
        fi
    fi
}


read_config_file

detect_wfb_primary_band

auto_frequency_select

if [ "$CAM" == "0" ]; then
    if [[ "$MirrorDSI_To_HDMI" == "y" || "$MirrorDSI_To_HDMI" == "Y" ]]; then
        qstatus "Enabling DSI to HDMI mirroring" 5

        if [ -e "/dev/fb0" ]; then
            if [ -e "/dev/fb1" ]; then                
                /usr/local/bin/raspi2raspi &
            else
                qstatus "/dev/fb1 not found, mirroring disabled" 4
            fi
        else
            qstatus "/dev/fb0 not found, mirroring disabled" 4
        fi
    fi
fi

datarate_to_wifi_settings

detect_nics

if [ "$CAM" == "0" ]; then
    if [ "$SmartSyncControlVia" == "GPIO" ]; then
        exit 2
    fi

    if [ "$SmartSyncControlVia" == "joystick" ]; then
        exit 1
    fi
fi

exit 0
