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
                    echo "RAM disk full, killing cat video file writing  process ..."
                
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
        qstatus "Saving to USB. This may take some time ..." 5
    else
        wbc_status "Saving to USB. This may take some time ..." 7 55 0 &
    fi


    echo -n "Accessing file system.. "

    #
    # Some USB drives show up as sda1, others as sda, check for both in order (checking for sda first
    # would always succeed even if sda1 exists)
    #
    if [ -e "/dev/sda1" ]; then
        USBDEV="/dev/sda1"
    else
        USBDEV="/dev/sda"
    fi


    echo "USBDEV: ${USBDEV}"


    if mount ${USBDEV} /media/usb; then
        TELEMETRY_SAVE_PATH="telemetry"
        SCREENSHOT_SAVE_PATH="screenshot"
        VIDEO_SAVE_PATH="video"
        RSSI_SAVE_PATH="rssi"


        if [ -d "/media/usb/${RSSI_SAVE_PATH}" ]; then
            echo "RSSI save path '${RSSI_SAVE_PATH}' found"
        else
            echo "Creating RSSI save path '${RSSI_SAVE_PATH}'"
            mkdir /media/usb/${RSSI_SAVE_PATH}
        fi


        if [ -d "/media/usb/${TELEMETRY_SAVE_PATH}" ]; then
            echo "Telemetry save path '${TELEMETRY_SAVE_PATH}' found"
        else
            echo "Creating Telemetry save path '${TELEMETRY_SAVE_PATH}'"
            mkdir /media/usb/${TELEMETRY_SAVE_PATH}
        fi


        killall rssilogger
        killall syslogger
        killall wifibackgroundscan


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
        cp /wbc_tmp/*.csv /media/usb/${RSSI_SAVE_PATH}/


        #
        # Save the telemetry data to the USB drive
        #
        if [ -s "/wbc_tmp/telemetrydowntmp.raw" ]; then
            cp /wbc_tmp/telemetrydowntmp.raw /media/usb/${TELEMETRY_SAVE_PATH}/telemetrydown`ls /media/usb/${TELEMETRY_SAVE_PATH}/*.raw | wc -l`.raw
            cp /wbc_tmp/telemetrydowntmp.txt /media/usb/${TELEMETRY_SAVE_PATH}/telemetrydown`ls /media/usb/${TELEMETRY_SAVE_PATH}/*.txt | wc -l`.txt
        fi


        #
        # Stop capturing packet data so the file can be saved to USB
        # 
        killall tshark


        #
        # Copy any captured packet data to the USB drive
        #
        cp /wbc_tmp/*.pcap /media/usb/
        cp /wbc_tmp/*.cap /media/usb/

        #
        # Copy the airodump chart to the USB drive
        #
        cp /wbc_tmp/airodump.png /media/usb/


        if [ "${ENABLE_SCREENSHOTS}" == "Y" ]; then
            if [ -d "/media/usb/${SCREENSHOT_SAVE_PATH}" ]; then
                echo "Screenshots save path '${SCREENSHOT_SAVE_PATH}' found"
            else
                echo "Creating screenshots save path '${SCREENSHOT_SAVE_PATH}'"
                mkdir /media/usb/${SCREENSHOT_SAVE_PATH}
            fi
            
            DIR_NAME_SCREENSHOT=/media/usb/${SCREENSHOT_SAVE_PATH}/`ls /media/usb/${SCREENSHOT_SAVE_PATH} | wc -l`
            
            mkdir ${DIR_NAME_SCREENSHOT}
            cp /wbc_tmp/screenshot* ${DIR_NAME_SCREENSHOT} > /dev/null 2>&1
        fi

        #
        # Only save video if the user enabled saving via memory or sdcard
        #
        if [ -s "${VIDEOFILE}" ] && [ "${VIDEO_TMP}" != "none" ]; then

            if [ -d "/media/usb/${VIDEO_SAVE_PATH}" ]; then
                echo "Video save path '${VIDEO_SAVE_PATH}' found"
            else
                echo "Creating video save path '${VIDEO_SAVE_PATH}'"
                mkdir /media/usb/${VIDEO_SAVE_PATH}
            fi


            if [ -z "${VIDEO_SAVE_FORMAT}" ]; then
                VIDEO_SAVE_FORMAT=avi
            fi
            

            USB_VIDEO_FILE=/media/usb/${VIDEO_SAVE_PATH}/video`ls /media/usb/${VIDEO_SAVE_PATH} | wc -l`.${VIDEO_SAVE_FORMAT}


            echo "USB_VIDEO_FILE: ${USB_VIDEO_FILE}"


            nice ffmpeg -framerate ${FPS} -i ${VIDEOFILE} -c:v copy ${USB_VIDEO_FILE} > /dev/null 2>&1 &


            FFMPEGRUNNING=1
            while [ ${FFMPEGRUNNING} -eq 1 ]; do
                FFMPEGRUNNING=`pidof ffmpeg | wc -w`
                #echo "FFMPEGRUNNING: ${FFMPEGRUNNING}"

                sleep 4

                killall wbc_status > /dev/null 2>&1

                if [ "${ENABLE_QOPENHD}" == "Y" ]; then
                    qstatus "Saving - please wait ..." 5
                else
                    wbc_status "Saving - please wait ..." 7 65 0 &
                fi
            done
        fi
        

        #
        # Copy telemetry logs to the USB drive
        #

        #cp /wbc_tmp/tracker.txt /media/usb/
        cp /wbc_tmp/debug.txt /media/usb/
        cp /wbc_tmp/telemetrydowndebug.txt /media/usb/${TELEMETRY_SAVE_PATH}/
        cp /wbc_tmp/telemetryupdebug.txt /media/usb/${TELEMETRY_SAVE_PATH}/


        nice umount /media/usb

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
        killall hello_video.bin.player > /dev/null 2>&1

        #
        # Clear out the temporary directories so we can start saving again
        #
        rm /wbc_tmp/* > /dev/null 2>&1
        rm /video_tmp/* > /dev/null 2>&1
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
        killall hello_video.bin.player > /dev/null 2>&1
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
}
