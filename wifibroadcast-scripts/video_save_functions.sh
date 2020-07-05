function MAIN_VIDEO_SAVE_FUNCTION {
    echo "================== SAVE FUNCTION (tty6) ==========================="
    echo
    

    #
    # Only run save function if this is a ground pi
    #
    if [ "$CAM" == "0" ]; then
        echo "Waiting some time until everything else is running ..."
    
        sleep 30
        
        echo "Waiting for USB stick to be plugged in ..."
        
        KILLED=0

        #
        # 3 Megabytes free space limit, the code below will stop saving video if free space falls below
        # this level
        #
        LIMITFREE=3000
        

        while true; do
            if [ ! -f "/tmp/donotsave" ]; then
                if [ -e "/dev/sda" ]; then
                    echo "USB Memory stick detected"
                    save_function
                fi
            fi
            
            #
            # Check if tmp disk is full, if yes, kill cat process
            #
            if [ "${KILLED}" != "1" ]; then
                FREETMPSPACE=`nice df -P /wbc_tmp/ | nice awk 'NR==2 {print $4}'`

                if [ ${FREETMPSPACE} -lt ${LIMITFREE} ]; then
                    if [ "${ENABLE_QOPENHD}" == "Y" ]; then
                        qstatus "Ground recording space full" 4
                    else
                        killall wbc_status > /dev/null 2>&1
                        wbc_status "Ground recording space full" 7 65 0 &
                    fi
                
                    ps -ef | nice grep "cat /var/run/openhd/videofifo3" | nice grep -v grep | awk '{print $2}' | xargs kill -9
                    KILLED=1
                fi
            fi
            
            sleep 1
        done
    fi
    
    echo "Save function not enabled, this is an air pi"
    
    sleep 365d
}


function save_function {
    #
    # Let screenshot and check_alive function know that saving is in progress
    #
    touch /tmp/pausewhile

    SAVE_IDENTIFIER=$(date '+%Y-%m-%d-%H-%M-%S')
    source /tmp/videofile

     

    #
    # Kill old OSD temporarily
    #   
    if [ "${ENABLE_QOPENHD}" != "Y" ]; then
        systemctl stop osd
    fi
    

    #
    # Stop saving the incoming video transmission while it is being copied to USB
    #
    ps -ef | nice grep "cat /var/run/openhd/videofifo3" | nice grep -v grep | awk '{print $2}' | xargs kill -9

    # Stop saving the incoming telemetry stream while it is being copied to USB
    ps -ef | nice grep "cat /var/run/openhd/telemetryfifo3" | nice grep -v grep | awk '{print $2}' | xargs kill -9


    #
    # Kill any on-screen status display so that we can show another one
    #
    killall wbc_status > /dev/null 2>&1


    if [ "${ENABLE_QOPENHD}" == "Y" ]; then
        qstatus "Saving to USB..." 5
    else
        wbc_status "Saving to USB..." 7 55 0 &
    fi

    #
    # Some USB drives show up as sda1, others as sda, check for both in order (checking for sda first
    # would always succeed even if sda1 exists)
    #
    if [ -e "/dev/sda1" ]; then
        USBDEV="/dev/sda1"
    else
        USBDEV="/dev/sda"
    fi


    if mount ${USBDEV} /media/usb; then
        killall rssilogger > /dev/null 2>&1
        killall syslogger > /dev/null 2>&1
        killall wifibackgroundscan > /dev/null 2>&1


        #
        # Create charts for the video and downstream telemetry
        # 
        gnuplot -e "load '/usr/local/share/openhd/gnuplot/videorssi.gp'" &
        gnuplot -e "load '/usr/local/share/openhd/gnuplot/videopackets.gp'"
        gnuplot -e "load '/usr/local/share/openhd/gnuplot/telemetrydownrssi.gp'" &
        gnuplot -e "load '/usr/local/share/openhd/gnuplot/telemetrydownpackets.gp'"


        #
        # Upstream telemetry was enabled, this creates a chart with the data
        #
        if [ "${TELEMETRY_UPLINK}" != "disabled" ]; then
            gnuplot -e "load '/usr/local/share/openhd/gnuplot/telemetryuprssi.gp'" &
            gnuplot -e "load '/usr/local/share/openhd/gnuplot/telemetryuppackets.gp'"
        fi


        #
        # RC was in use, this creates a chart with the data
        #
        if [ "${RC}" != "disabled" ]; then
            gnuplot -e "load '/usr/local/share/openhd/gnuplot/rcrssi.gp'" &
            gnuplot -e "load '/usr/local/share/openhd/gnuplot/rcpackets.gp'"
        fi


        if [ "${DEBUG}" == "Y" ]; then
            gnuplot -e "load '/usr/local/share/openhd/gnuplot/wifibackgroundscan.gp'" &
        fi


        #
        # Save all the raw chart data to the USB drive
        #
        RSSI_SAVE_PATH="/media/usb/${SAVE_IDENTIFIER}/rssi"
        mkdir -p ${RSSI_SAVE_PATH}

        cp /wbc_tmp/*.csv ${RSSI_SAVE_PATH}/
        cp /wbc_tmp/rssi/*.png ${RSSI_SAVE_PATH}/


        #
        # Save the telemetry data to the USB drive
        #
        TELEMETRY_SAVE_PATH="/media/usb/${SAVE_IDENTIFIER}/telemetry"
        mkdir -p ${TELEMETRY_SAVE_PATH}

        if [ -s "/wbc_tmp/telemetrydowntmp.raw" ]; then
            cp /wbc_tmp/telemetrydowntmp.raw ${TELEMETRY_SAVE_PATH}/telemetrydown.raw
            cp /wbc_tmp/telemetrydowntmp.txt ${TELEMETRY_SAVE_PATH}/telemetrydown.txt > /dev/null 2>&1
        fi

        cp /wbc_tmp/telemetrydowndebug.txt ${TELEMETRY_SAVE_PATH}/telemetrydowndebug.txt > /dev/null 2>&1
        cp /wbc_tmp/telemetryupdebug.txt ${TELEMETRY_SAVE_PATH}/telemetryupdebug.txt > /dev/null 2>&1



        #
        # Stop capturing packet data so the file can be saved to USB
        # 
        killall tshark > /dev/null 2>&1


        #
        # Copy any captured packet data to the USB drive
        #
        CAP_SAVE_PATH="/media/usb/${SAVE_IDENTIFIER}/cap"
        mkdir -p ${CAP_SAVE_PATH}
        cp /wbc_tmp/*.pcap ${CAP_SAVE_PATH}/ > /dev/null 2>&1
        cp /wbc_tmp/*.cap ${CAP_SAVE_PATH}/ > /dev/null 2>&1

        #
        # Copy the airodump chart to the USB drive
        #
        cp /wbc_tmp/airodump.png /media/usb/${SAVE_IDENTIFIER}/ > /dev/null 2>&1


        if [ "${ENABLE_SCREENSHOTS}" == "Y" ]; then
            SCREENSHOT_SAVE_PATH="/media/usb/${SAVE_IDENTIFIER}/screenshot"

            mkdir -p ${SCREENSHOT_SAVE_PATH}
            cp /wbc_tmp/screenshot* ${SCREENSHOT_SAVE_PATH}/ > /dev/null 2>&1
        fi

        #
        # Only save video if the user enabled saving via memory or sdcard
        #
        if [ -s "${VIDEOFILE}" ] && [ "${VIDEO_TMP}" != "none" ]; then
            VIDEO_SAVE_PATH="/media/usb/${SAVE_IDENTIFIER}/video"

            mkdir -p ${VIDEO_SAVE_PATH}

            if [ -z "${VIDEO_SAVE_FORMAT}" ]; then
                VIDEO_SAVE_FORMAT=avi
            fi
            

            USB_VIDEO_FILE=${VIDEO_SAVE_PATH}/video.${VIDEO_SAVE_FORMAT}


            nice ffmpeg -framerate ${FPS} -i ${VIDEOFILE} -c:v copy ${USB_VIDEO_FILE} > /dev/null 2>&1 &

            killall wbc_status > /dev/null 2>&1

            FFMPEGRUNNING=1
            while [ ${FFMPEGRUNNING} -eq 1 ]; do
                FFMPEGRUNNING=`pidof ffmpeg | wc -w`

                sleep 4
        
                if [ -f ${USB_VIDEO_FILE} ]; then
                    ORIGINAL_FILE_SIZE=$(ls -l ${VIDEOFILE} | awk '{print $5}')
                    TARGET_FILE_SIZE=$(ls -l ${USB_VIDEO_FILE} | awk '{print $5}')
                    PERCENT_FINISHED=$(python -c "print(int(float(${TARGET_FILE_SIZE}) / float(${ORIGINAL_FILE_SIZE}) * 100.0))"  )

                    if [ "${ENABLE_QOPENHD}" == "Y" ]; then
                        qstatus "Saving: ${PERCENT_FINISHED}% done" 5
                    else
                        killall wbc_status > /dev/null 2>&1
                        wbc_status "Saving: ${PERCENT_FINISHED}% done" 7 65 0 &
                    fi
                fi
            done
        fi
        
        killall wbc_status > /dev/null 2>&1

        #
        # Copy telemetry logs to the USB drive
        #

        #cp /wbc_tmp/tracker.txt /media/usb/${SAVE_IDENTIFIER}/
        cp /wbc_tmp/debug.txt /media/usb/${SAVE_IDENTIFIER}/debug.txt > /dev/null 2>&1

        sync

        MOUNTED=`mount | grep /media/usb | wc -l`
        while [ $MOUNTED -ge 1 ]; do
            echo "Attempting to unmount USB drive"
            umount -A /media/usb > /dev/null 2>&1
            sleep 1
            MOUNTED=`mount | grep /media/usb | wc -l`
        done

        #
        # Inform the user that saving is done and the USB drive can be removed
        #
        STICKGONE=0
        while [ ${STICKGONE} -ne 1 ]; do
            killall wbc_status > /dev/null 2>&1

            if [ "${ENABLE_QOPENHD}" == "Y" ]; then
                qstatus "Done - USB memory stick can be removed now" 5
            else
                wbc_status "Done - USB memory stick can be removed now" 7 65 0 &
            fi

            sleep 4

            if [ ! -e "/dev/sda" ]; then
                STICKGONE=1
            fi
        done
        
        #
        # Stop any processes left over from the save step
        #
        killall wbc_status > /dev/null 2>&1

        #
        # Clear out the temporary directories so we can start saving again
        #
        rm /wbc_tmp/* > /dev/null 2>&1
        rm /video_tmp/* > /dev/null 2>&1
        rm /wbc_tmp/rssi/* > /dev/null 2>&1

        sync
    else
        STICKGONE=0

        while [ ${STICKGONE} -ne 1 ]; do
            killall wbc_status > /dev/null 2>&1

            if [ "${ENABLE_QOPENHD}" == "Y" ]; then
                qstatus "ERROR: Could not access USB memory stick!" 3
            else
                wbc_status "ERROR: Could not access USB memory stick!" 7 65 0 &
            fi

            sleep 4

            if [ ! -e "/dev/sda" ]; then
                STICKGONE=1
            fi
        done

        #
        # Stop any processes left over from the save step
        #
        killall wbc_status > /dev/null 2>&1
    fi


    #killall tracker

    #
    # Re-start video recording
    #
    if [ "${VIDEO_TMP}" != "none" ]; then
        ionice -c 3 nice cat /var/run/openhd/videofifo3 >> ${VIDEOFILE} &
    fi

    #
    # Re-start raw telemetry recording
    #
    ionice -c 3 nice cat /var/run/openhd/telemetryfifo3 >> /wbc_tmp/telemetrydowntmp.raw &


    #
    # Re-start rssi logging
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

    killall wbc_status > /dev/null 2>&1
    OSDRUNNING=`pidof /usr/local/bin/osd | wc -w`


    if [ ${OSDRUNNING}  -ge 1 ]; then
        echo "OSD already running!"
    else
        killall wbc_status > /dev/null 2>&1

        #
        # Re-start the OSD, we only do this for the old OSD because QOpenHD never stops
        #
        if [ "${ENABLE_QOPENHD}" != "Y" ]; then
            systemctl start osd
        fi
    fi
    
    #
    # Let screenshot function know that it can continue taking screenshots
    #
    rm /tmp/pausewhile

    echo "USB save done"
}
