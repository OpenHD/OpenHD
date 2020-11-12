function rx_function {

    if [ "${ENABLE_QOPENHD}" != "Y" ]; then
        if [ "${AdjustLCDBacklight}" == "Y" ]; then
            /usr/local/bin/MouseListener ${AutoDimTime} ${AutoDimValue} /dev/input/event0 & > /dev/null 2>&1
        fi
    fi

    /usr/local/bin/sharedmem_init_rx

    #
    # Start virtual serial port for cmavnode and ser2net
    #
    ionice -c 3 nice socat -lf /wbc_tmp/socat1.log -d -d pty,raw,echo=0,link=/dev/openhd_mavlink2 pty,raw,echo=0,link=/dev/openhd_mavlink1 & > /dev/null 2>&1
    sleep 1
    ionice -c 3 nice socat -lf /wbc_tmp/socat2.log -d -d pty,raw,echo=0,link=/dev/openhd_msp2 pty,raw,echo=0,link=/dev/openhd_msp1 & > /dev/null 2>&1
    sleep 1

    #
    # Setup virtual serial ports
    #
    stty -F /dev/openhd_mavlink2 -icrnl -ocrnl -imaxbel -opost -isig -icanon -echo -echoe -ixoff -ixon 115200
    stty -F /dev/openhd_msp2 -icrnl -ocrnl -imaxbel -opost -isig -icanon -echo -echoe -ixoff -ixon 115200

    echo

    #
    # If USB memory stick is already connected during startup, notify user
    # and pause as long as stick is not removed
    # some sticks show up as sda1, others as sda, check for both
    #
    if [ -e "/dev/sda1" ]; then
        STARTUSBDEV="/dev/sda1"
    else
        STARTUSBDEV="/dev/sda"
    fi

    #
    # Check to see if a USB drive is connected before continuing.
    #
    # This is not strictly necessary and should be changed to allow direct saving to USB. It
    # also may cause false positives, because some USB devices present multiple types of device class to
    # the system. For example many LTE modems have a fake USB storage device inside that is used to store
    # drivers for the device itself. There's really no way around that except to blacklist them by USB ID,
    # or to check if they're actually read-write, most of them may not be and we could detect that.
    #
    if [ -e ${STARTUSBDEV} ]; then
        touch /tmp/donotsave
        STICKGONE=0

        while [ ${STICKGONE} -ne 1 ]; do
            killall wbc_status > /dev/null 2>&1
            
            if [ "${ENABLE_QOPENHD}" == "Y" ]; then
                qstatus "USB memory stick detected - please remove and re-plug after flight" 5
            else
                wbc_status "USB memory stick detected - please remove and re-plug after flight" 7 65 0 &
            fi
            
            sleep 4

            if [ ! -e ${STARTUSBDEV} ]; then
                STICKGONE=1
                rm /tmp/donotsave
            fi
        done
    fi

    killall wbc_status > /dev/null 2>&1


    sleep 1

    detect_nics
    

    if [ "${Bandwidth}" == "10" ]; then
        qstatus "Using 10MHz channel bandwidth" 5
        echo "HardCode dirty code for tests only. Values are it Hex, to set 10MHz use 0xa (10 in dec)"
        echo 0xa > /sys/kernel/debug/ieee80211/phy0/ath9k_htc/chanbw
        echo 0xa > /sys/kernel/debug/ieee80211/phy1/ath9k_htc/chanbw
        echo 0xa > /sys/kernel/debug/ieee80211/phy2/ath9k_htc/chanbw
        echo 0xa > /sys/kernel/debug/ieee80211/phy3/ath9k_htc/chanbw
        echo 0xa > /sys/kernel/debug/ieee80211/phy4/ath9k_htc/chanbw
    fi

    if [ "${Bandwidth}" == "5" ]; then
        qstatus "Using 5MHz channel bandwidth" 5
        echo "HardCode dirty code for tests only. Values are it Hex, to set 10MHz use 0xa (10 in dec)"
        echo 5 > /sys/kernel/debug/ieee80211/phy0/ath9k_htc/chanbw
        echo 5 > /sys/kernel/debug/ieee80211/phy1/ath9k_htc/chanbw
        echo 5 > /sys/kernel/debug/ieee80211/phy2/ath9k_htc/chanbw
        echo 5 > /sys/kernel/debug/ieee80211/phy3/ath9k_htc/chanbw
        echo 5 > /sys/kernel/debug/ieee80211/phy4/ath9k_htc/chanbw
    fi


    if [ "${VIDEO_WIFI_BITRATE}" == "19.5" ]; then
        #
        # Set back to 18 to make sure -d parameter works (supports only 802.11b/g datarates)
        #
        # This matches similar code in tx_functions.sh
        #
        VIDEO_WIFI_BITRATE=18
        OSD_HW_BITRATE=18
    fi

    if [ "${VIDEO_WIFI_BITRATE}" == "5.5" ]; then
        #
        # set back to 6 to make sure -d parameter works (supports only 802.11b/g datarates)
        #
        # This matches similar code in tx_functions.sh
        #
        VIDEO_WIFI_BITRATE=5
        OSD_HW_BITRATE=5
    fi

    #
    # On the ground side, this is read by the old OSD code so we can display it alongside the 
    # current/measured air datarate
    #
    echo ${OSD_HW_BITRATE} > /tmp/DATARATE.txt


    sleep 0.5


    # videofifo1: local display, hello_video.bin
    # videofifo2: secondary display, hotspot/usb-tethering
    # videofifo3: recording
    # videofifo4: wbc relay

    #
    # Check the SD card partitions, we create a separate video partition if needed to avoid
    # filling up the root filesystem.
    #
    if [ "${VIDEO_TMP}" == "sdcard" ]; then
        #
        # Make sure check_alive (in alive_functions.sh) doesn't do anything for now
        #
        touch /tmp/pausewhile

        tmessage "Saving to SD card enabled, preparing video storage"
        qstatus "Saving video to SD card" 5
        
        tmessage "WARNING: SD card video saving may cause instability in the live video stream, this is temporary"
        
        if cat /proc/partitions | nice grep -q mmcblk0p3; then 
            echo
            echo "SD card video partion detected.."
        else
            echo
            echo "SD card video partion NOT detected.."
            echo 

            #
            # This automates the process of creating the correct partition structure by feeding specific 
            # commands to fdisk
            #        
            echo -e "n\np\n3\n7839744\n\nw" | fdisk /dev/mmcblk0 > /dev/null 2>&1

            # 
            # Make the kernel re-detect any partitions now that we created a new one on the SD card
            # 
            partprobe > /dev/null 2>&1
            

            if [ "${VIDEO_FS}" == "fat" ]; then
                tmessage "Creating SD card FAT filesytem for video recording.."

                mkfs.vfat /dev/mmcblk0p3 -n myvideo > /boot/sdcard.txt 2>&1 || {
                    tmessage "ERROR: Could not format video storage on SDCARD!"
                    collect_errorlog
                    sleep 365d 
                }

                mkdir -p /video_tmp > /dev/null 2>&1

                mount -t vfat /dev/mmcblk0p3 /video_tmp > /dev/null 2>&1 || {
                    tmessage "ERROR: Could not mount video partition on SD card!"
                    collect_errorlog
                    sleep 365d
                }
            else
                tmessage "Creating SDCARD EXT4 filesytem for Video Recording.."
                
                mkfs.ext4 /dev/mmcblk0p3 -L myvideo -F > /dev/null 2>&1 || {
                    tmessage "ERROR: Could not format video partition on SD card!"
                    collect_errorlog
                    sleep 365d 
                }

                e2fsck -p /dev/mmcblk0p3 > /dev/null 2>&1
                
                mkdir -p /video_tmp > /dev/null 2>&1
                
                mount -t ext4 /dev/mmcblk0p3 /video_tmp > /dev/null 2>&1 || {
                    tmessage "ERROR: Could not mount video partition on SD card!"
                    collect_errorlog
                    sleep 365d
                }
            fi
        fi

        
        VIDEOFILE=/video_tmp/videotmp.raw

        echo "VIDEOFILE=${VIDEOFILE}" > /tmp/videofile

        rm ${VIDEOFILE} > /dev/null 2>&1

    elif [ "${VIDEO_TMP}" == "memory" ]; then
        detect_memory

        mkdir -p /video_tmp

        # use 1/3 of available ram by default for /video_tmp, which is used for recording telemetry and video
        # when VIDEO_TMP=memory. We need to do this to avoid crashes or safety issues caused by running out of
        # memory, which is easy to do when the ground station has just 512MB ram to start with and has 128MB set
        # aside for the GPU, like the Pi3a+
        available_for_video_tmp=$((${TOTAL_MEMORY} / 3))

        # add a little extra margin 
        available_for_video_tmp_final=$((${available_for_video_tmp} + 30000))

        mount -t tmpfs -o size=${available_for_video_tmp_final}K tmpfs /video_tmp

        VIDEOFILE=/video_tmp/videotmp.raw

        echo "VIDEOFILE=${VIDEOFILE}" > /tmp/videofile

        qstatus "Saving video to memory" 5
    else
        echo "Video save disabled"
        qstatus "Saving video disabled" 5
    fi


    #
    # Tracker disabled temporarily
    #
    #/usr/local/bin//tracker /wifibroadcast_rx_status_0 >> /wbc_tmp/tracker.txt &
    #sleep 1

    killall wbc_status > /dev/null 2>&1

    if [ "${AIRODUMP}" == "Y" ]; then
        #
        # Make sure check_alive (in alive_functions.sh) doesn't do anything for now
        #
        touch /tmp/pausewhile

        
        echo "AiroDump is enabled, running airodump-ng for ${AIRODUMP_SECONDS} seconds ..."
        qstatus "AiroDump is enabled, running airodump-ng for ${AIRODUMP_SECONDS} seconds ..." 5
        
        sleep 3
        
        # strip newlines
        NICS_COMMA=`echo $NICS | tr '\n' ' '`
        # strip space at end
        NICS_COMMA=`echo $NICS_COMMA | sed 's/ *$//g'`
        # replace spaces by comma
        NICS_COMMA=${NICS_COMMA// /,}

        #
        # Decide which channels to look at based on the configured frequency band
        #
        # TODO: this should be pulled out into a separate preconfiguration step so the information is
        #       in a centralized location
        #
        if [ "$FREQ" -gt 3000 ]; then
            AIRODUMP_CHANNELS="5180,5200,5220,5240,5260,5280,5300,5320,5500,5520,5540,5560,5580,5600,5620,5640,5660,5680,5700,5745,5765,5785,5805,5825"
        else
            AIRODUMP_CHANNELS="2412,2417,2422,2427,2432,2437,2442,2447,2452,2457,2462,2467,2472"
        fi

        airodump-ng --showack -h --berlin 60 --ignore-negative-one --manufacturer --output-format pcap --write /wbc_tmp/wifiscan --write-interval 2 -C $AIRODUMP_CHANNELS  $NICS_COMMA &
        sleep $AIRODUMP_SECONDS
        sleep 2
        
        #
        # Use raspi2png to take a screenshot of the rendered airodump display
        #
        ionice -c 3 nice -n 19 /usr/bin/raspi2png -p /wbc_tmp/airodump.png >> /dev/null
        killall airodump-ng
        
        sleep 1
        

        #
        # Set the configured WFB frequency on all configured NICs
        #
        printf "\033c"
        for NIC in $NICS
        do
            iw dev $NIC set freq $FREQ
        done
        
        sleep 1
        
        echo
        echo
        echo
        echo
        echo
        echo
        echo
        echo
        echo
        echo
        echo
        echo
        echo
        echo
        echo
        echo
        echo
        echo
        echo
    fi


    if [ "$DEBUG" == "Y" ]; then
        collect_debug /wbc_tmp &
    fi
    

    wbclogger_function &

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
            echo "  ---------------------------------------------------------------------------------------------------"
            echo "  | ERROR: Under-Voltage detected on the GROUNDPi. Your Pi is not supplied with stable 5 Volts.     |"
            echo "  |                                                                                                 |"
            echo "  | Either your power-supply or wiring is not sufficent, check the wiring instructions in the Wiki! |"
            echo "  ---------------------------------------------------------------------------------------------------"
            
            qstatus "ERROR: Undervoltage detected, check power supply" 3

            UNDERVOLT=1

            echo "1" > /tmp/undervolt

            sleep 5
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


    #
    # Make sure check_alive (in alive_functions.sh) can resume, along with anything else that was waiting
    #
    if [ -e "/tmp/pausewhile" ]; then
        rm /tmp/pausewhile
    fi


    IsFirstTime=0


    #
    # Loop to ensure the video RX process is always running, in case it exits for some reason
    #
    while true; do
        #
        # Make sure check_alive (in alive_functions.sh) can resume, along with anything else that was waiting
        #
        pause_while


        if [ $IsFirstTime -eq 0 ]; then
            killall omxplayer  > /dev/null 2>/dev/null
            killall omxplayer.bin  > /dev/null 2>/dev/null
        fi

        #
        # Temporarily continue using hello_video even if QOpenHD is enabled, there's not much to gain by changing it
        #

        #if [ "$ENABLE_QOPENHD" == "Y" ]; then
        #	if [ "$FORWARD_STREAM" == "rtp" ]; then
        #		ionice -c 1 -n 4 nice -n -10 cat /var/run/openhd/videofifo1 | ionice -c 1 -n 4 nice -n -10 gst-launch-1.0 fdsrc ! h264parse ! rtph264pay pt=96 config-interval=5 ! udpsink port=${VIDEO_UDP_PORT} host=127.0.0.1 &
        #	else
        #		ionice -c 1 -n 4 nice -n -10 cat /var/run/openhd/videofifo1 | ionice -c 1 -n 4 nice -n -10 gst-launch-1.0 fdsrc ! udpsink port=${VIDEO_UDP_PORT} host=127.0.0.1 &
        #	fi
        #else
            ionice -c 1 -n 4 nice -n -10 cat /var/run/openhd/videofifo1 | ionice -c 1 -n 4 nice -n -10 $DISPLAY_PROGRAM ${HELLO_VIDEO_ARGS} > /dev/null 2>&1 &
        #fi


        #
        # Start saving video if one of the saving modes is enabled
        #
        if [ "$VIDEO_TMP" != "none" ]; then
            ionice -c 3 nice cat /var/run/openhd/videofifo3 >> $VIDEOFILE &
        fi

        #
        # Start the WFB relay, using another wifi card to retransmit the video to another ground station
        #
        # Note that this is reportedly not working at the moment, likely because of the NIC detection and setup, it's likely
        # that something is not setting up the interface or treating it like it's just another ground interface
        #
        if [ "$RELAY" == "Y" ]; then
            /usr/local/bin/sharedmem_init_tx
            ionice -c 1 -n 4 nice -n -10 cat /var/run/openhd/videofifo4 | /usr/local/bin/tx_rawsock -p 0 -b $RELAY_VIDEO_BLOCKS -r $RELAY_VIDEO_FECS -f $RELAY_VIDEO_BLOCKLENGTH -t $VIDEO_FRAMETYPE -d 24 -y 0 relay0 > /dev/null 2>&1 &
        fi


        #
        # Update NICS variable in case a NIC has been removed (exclude devices with wlanX)
        #
        NICS=`ls /sys/class/net/ | nice grep -v eth0 | nice grep -v lo | nice grep -v usb | nice grep -v intwifi | nice grep -v wlan | nice grep -v relay | nice grep -v wifihotspot`

        tmessage "Starting video RX, FEC: $VIDEO_BLOCKS/$VIDEO_FECS/$VIDEO_BLOCKLENGTH"
        qstatus "Starting video RX" 5
        qstatus "FEC: $VIDEO_BLOCKS/$VIDEO_FECS/$VIDEO_BLOCKLENGTH" 5

        #
        # Start audio and remote settings
        #
        if [ $IsFirstTime -eq 0 ]; then
            if [ "$IsAudioTransferEnabled" == "1" ]; then
                echo "Audio enabled"
                qstatus "Audio enabled" 5

                amixer cset numid=3 $DefaultAudioOut

                /usr/local/share/RemoteSettings/Ground/AudioPlayback.sh &
                /usr/local/share/RemoteSettings/Ground/RxAudio.sh &
            fi

            if [ "$RemoteSettingsEnabled" != "0" ]; then
                echo "Remote settings enabled"
                qstatus "Remote settings enabled" 5

                /usr/local/share/RemoteSettings/ipchecker/iphelper.sh > /dev/null 2>&1 &
                /usr/bin/python3 /usr/local/share/RemoteSettings/RemoteSettings.py > /dev/null 2>&1 &
                /usr/local/share/RemoteSettings/RemoteSettingsWFBC_UDP.sh > /dev/null 2>&1 &
                /usr/local/share/RemoteSettings/GroundRSSI.sh &
            fi
            
            #if [ "$IsBandSwicherEnabled" == "1" ]; then
                echo "Band switcher enabled"
                qstatus "Band switcher enabled" 5
                /usr/local/share/RemoteSettings/BandSwitcher.sh &
            #fi
        fi
    
        IsFirstTime=1


        #
        # Start the video RX process, piping the data to several fifos to distribute it around the system
        #
        # This will be merged into the new ground recording system
        #
        if [ "$VIDEO_TMP" != "none" ]; then
            ionice -c 1 -n 3 /usr/local/bin/rx -p 0 -d 1 -b $VIDEO_BLOCKS -r $VIDEO_FECS -f $VIDEO_BLOCKLENGTH $NICS | ionice -c 1 -n 4 nice -n -10 tee >(ionice -c 1 -n 4 nice -n -10 /usr/local/bin/ftee /var/run/openhd/videofifo2 > /dev/null 2>&1) >(ionice -c 1 nice -n -10 /usr/local/bin/ftee /var/run/openhd/videofifo4 > /dev/null 2>&1) >(ionice -c 3 nice /usr/local/bin/ftee /var/run/openhd/videofifo3 > /dev/null 2>&1) | ionice -c 1 -n 4 nice -n -10 /usr/local/bin/ftee /var/run/openhd/videofifo1 > /dev/null 2>&1
        else
            ionice -c 1 -n 3 /usr/local/bin/rx -p 0 -d 1 -b $VIDEO_BLOCKS -r $VIDEO_FECS -f $VIDEO_BLOCKLENGTH $NICS | ionice -c 1 -n 4 nice -n -10 tee >(ionice -c 1 -n 4 nice -n -10 /usr/local/bin/ftee /var/run/openhd/videofifo2 > /dev/null 2>&1) >(ionice -c 1 nice -n -10 /usr/local/bin/ftee /var/run/openhd/videofifo4 > /dev/null 2>&1) | ionice -c 1 -n 4 nice -n -10 /usr/local/bin/ftee /var/run/openhd/videofifo1 > /dev/null 2>&1
        fi


        RX_EXITSTATUS=${PIPESTATUS[0]}
        
        check_exitstatus $RX_EXITSTATUS

        #if [ "$ENABLE_QOPENHD" != "Y" ]; then
            ps -ef | nice grep "$DISPLAY_PROGRAM" | nice grep -v grep | awk '{print $2}' | xargs kill -9
        #fi
        ps -ef | nice grep "rx -p 0" | nice grep -v grep | awk '{print $2}' | xargs kill -9
        ps -ef | nice grep "ftee /var/run/openhd/videofifo" | nice grep -v grep | awk '{print $2}' | xargs kill -9
        ps -ef | nice grep "cat /var/run/openhd/videofifo" | nice grep -v grep | awk '{print $2}' | xargs kill -9
    done
}
