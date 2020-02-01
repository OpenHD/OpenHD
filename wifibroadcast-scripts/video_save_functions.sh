function MAIN_VIDEO_SAVE_FUNCTION {
	echo "================== SAVE FUNCTION (tty6) ==========================="
	echo
	
	# only run save function if we are RX
	if [ "$CAM" == "0" ]; then
	    echo "Waiting some time until everything else is running ..."
	
		sleep 30
	    
		echo "Waiting for USB stick to be plugged in ..."
	    
		KILLED=0
	    LIMITFREE=3000 # 3 mbyte
		
	    while true; do
			if [ ! -f "/tmp/donotsave" ]; then
				if [ -e "/dev/sda" ]; then
					echo "USB Memory stick detected"
					save_function
				fi
			fi
			
			# check if tmp disk is full, if yes, kill cat process
			if [ "$KILLED" != "1" ]; then
				FREETMPSPACE=`nice df -P /wbc_tmp/ | nice awk 'NR==2 {print $4}'`
				if [ $FREETMPSPACE -lt $LIMITFREE ]; then
					echo "RAM disk full, killing cat video file writing  process ..."
					ps -ef | nice grep "cat /root/videofifo3" | nice grep -v grep | awk '{print $2}' | xargs kill -9
					KILLED=1
				fi
			fi
			
			sleep 1
	    done
	fi
	
	echo "Save function not enabled, we are TX"
	
	sleep 365d
}


function save_function {
    # let screenshot and check_alive function know that saving is in progrss
    touch /tmp/pausewhile
	
    # kill OSD so we can safeley start wbc_status
    ps -ef | nice grep "osd" | nice grep -v grep | awk '{print $2}' | xargs kill -9
	
    # kill video and telemetry recording and also local video display
    ps -ef | nice grep "cat /root/videofifo3" | nice grep -v grep | awk '{print $2}' | xargs kill -9
    ps -ef | nice grep "cat /root/telemetryfifo3" | nice grep -v grep | awk '{print $2}' | xargs kill -9
	if [ "$ENABLE_QOPENHD" != "Y" ]; then
    	ps -ef | nice grep "$DISPLAY_PROGRAM" | nice grep -v grep | awk '{print $2}' | xargs kill -9
    	ps -ef | nice grep "cat /root/videofifo1" | nice grep -v grep | awk '{print $2}' | xargs kill -9
	fi

    # kill video rx
    ps -ef | nice grep "rx -p 0" | nice grep -v grep | awk '{print $2}' | xargs kill -9
    ps -ef | nice grep "ftee /root/videofifo" | nice grep -v grep | awk '{print $2}' | xargs kill -9

    if [ "$VIDEO_TMP" != "none" ]; then
        # find out if video is on ramdisk or sd
        source /tmp/videofile
        echo "VIDEOFILE: $VIDEOFILE"

        # start re-play of recorded video ....
        # nice /opt/vc/src/hello_pi/hello_video/hello_video.bin.player $VIDEOFILE $FPS &
        nice /rootfs/home/pi/wifibroadcast-hello_video/hello_video.bin.player $VIDEOFILE $FPS &
    fi

    killall wbc_status > /dev/null 2>&1
	if [ "$ENABLE_QOPENHD" == "Y" ]; then
        qstatus "Saving to USB. This may take some time ..." 3
	else
        wbc_status "Saving to USB. This may take some time ..." 7 55 0 &
	fi

    echo -n "Accessing file system.. "

    # some sticks show up as sda1, others as sda, check for both
    if [ -e "/dev/sda1" ]; then
		USBDEV="/dev/sda1"
    else
		USBDEV="/dev/sda"
    fi

    echo "USBDEV: $USBDEV"

    if mount $USBDEV /media/usb; then
		TELEMETRY_SAVE_PATH="/telemetry"
		SCREENSHOT_SAVE_PATH="/screenshot"
		VIDEO_SAVE_PATH="/video"
		RSSI_SAVE_PATH=/"rssi"

		if [ -d "/media/usb$RSSI_SAVE_PATH" ]; then
			echo "RSSI save path $RSSI_SAVE_PATH found"
		else
			echo "Creating RSSI save path $RSSI_SAVE_PATH.. "
			mkdir /media/usb$RSSI_SAVE_PATH
		fi

		if [ -d "/media/usb$TELEMETRY_SAVE_PATH" ]; then
			echo "Telemetry save path $TELEMETRY_SAVE_PATH found"
		else
			echo "Creating Telemetry save path $TELEMETRY_SAVE_PATH.. "
			mkdir /media/usb$TELEMETRY_SAVE_PATH
		fi

		killall rssilogger
		killall syslogger
		killall wifibackgroundscan

		gnuplot -e "load '/root/gnuplot/videorssi.gp'" &
		gnuplot -e "load '/root/gnuplot/videopackets.gp'"
		gnuplot -e "load '/root/gnuplot/telemetrydownrssi.gp'" &
		gnuplot -e "load '/root/gnuplot/telemetrydownpackets.gp'"

		if [ "$TELEMETRY_UPLINK" != "disabled" ]; then
			gnuplot -e "load '/root/gnuplot/telemetryuprssi.gp'" &
			gnuplot -e "load '/root/gnuplot/telemetryuppackets.gp'"
		fi
		if [ "$RC" != "disabled" ]; then
			gnuplot -e "load '/root/gnuplot/rcrssi.gp'" &
			gnuplot -e "load '/root/gnuplot/rcpackets.gp'"
		fi

		if [ "$DEBUG" == "Y" ]; then
			gnuplot -e "load '/root/gnuplot/wifibackgroundscan.gp'" &
		fi

		cp /wbc_tmp/*.csv /media/usb$RSSI_SAVE_PATH/

		if [ -s "/wbc_tmp/telemetrydowntmp.raw" ]; then
			cp /wbc_tmp/telemetrydowntmp.raw /media/usb$TELEMETRY_SAVE_PATH/telemetrydown`ls /media/usb$TELEMETRY_SAVE_PATH/*.raw | wc -l`.raw
			cp /wbc_tmp/telemetrydowntmp.txt /media/usb$TELEMETRY_SAVE_PATH/telemetrydown`ls /media/usb$TELEMETRY_SAVE_PATH/*.txt | wc -l`.txt
		fi

		killall tshark
		cp /wbc_tmp/*.pcap /media/usb
		cp /wbc_tmp/*.cap /media/usb

		cp /wbc_tmp/airodump.png /media/usb

		if [ "$ENABLE_SCREENSHOTS" == "Y" ]; then
			if [ -d "/media/usb$SCREENSHOT_SAVE_PATH" ]; then
				echo "Screenshots save path $SCREENSHOT_SAVE_PATH found"
			else
				echo "Creating screenshots save path $SCREENSHOT_SAVE_PATH.. "
				mkdir /media/usb$SCREENSHOT_SAVE_PATH
			fi
			
			DIR_NAME_SCREENSHOT=/media/usb$SCREENSHOT_SAVE_PATH/`ls /media/usb$SCREENSHOT_SAVE_PATH | wc -l`
			
			mkdir $DIR_NAME_SCREENSHOT
			cp /wbc_tmp/screenshot* $DIR_NAME_SCREENSHOT > /dev/null 2>&1
		fi
		if [ -s "$VIDEOFILE" ] && [ "$VIDEO_TMP" != "none" ]; then
			if [ -d "/media/usb$VIDEO_SAVE_PATH" ]; then
				echo "Video save path $VIDEO_SAVE_PATH found"
			else
				echo "Creating video save path $VIDEO_SAVE_PATH.. "
				mkdir /media/usb$VIDEO_SAVE_PATH
			fi

			if [ -z "${VIDEO_SAVE_FORMAT}" ]; then
				VIDEO_SAVE_FORMAT=avi
			fi
			
			USB_VIDEO_FILE=/media/usb$VIDEO_SAVE_PATH/video`ls /media/usb$VIDEO_SAVE_PATH | wc -l`.${VIDEO_SAVE_FORMAT}
			echo "USB_VIDEO_FILE: $USB_VIDEO_FILE"
			nice ffmpeg -framerate $FPS -i $VIDEOFILE -c:v copy $USB_VIDEO_FILE > /dev/null 2>&1 &
			FFMPEGRUNNING=1
			while [ $FFMPEGRUNNING -eq 1 ]; do
				FFMPEGRUNNING=`pidof ffmpeg | wc -w`
				#echo "FFMPEGRUNNING: $FFMPEGRUNNING"
				sleep 4
				killall wbc_status > /dev/null 2>&1
				if [ "$ENABLE_QOPENHD" == "Y" ]; then
				    qstatus "Saving - please wait ..." 3
				else
				    wbc_status "Saving - please wait ..." 7 65 0 &
				fi
			done
		fi
		
		#cp /wbc_tmp/tracker.txt /media/usb/
		cp /wbc_tmp/debug.txt /media/usb/
		cp /wbc_tmp/telemetrydowndebug.txt /media/usb$TELEMETRY_SAVE_PATH/
		cp /wbc_tmp/telemetryupdebug.txt /media/usb$TELEMETRY_SAVE_PATH/

		nice umount /media/usb
		STICKGONE=0
		while [ $STICKGONE -ne 1 ]; do
			killall wbc_status > /dev/null 2>&1
			if [ "$ENABLE_QOPENHD" == "Y" ]; then
			    qstatus "Done - USB memory stick can be removed now" 3
			else
			    wbc_status "Done - USB memory stick can be removed now" 7 65 0 &
			fi
			sleep 4
			if [ ! -e "/dev/sda" ]; then
				STICKGONE=1
			fi
		done
		
		killall wbc_status > /dev/null 2>&1
		killall hello_video.bin.player > /dev/null 2>&1
		rm /wbc_tmp/* > /dev/null 2>&1
		rm /video_tmp/* > /dev/null 2>&1
		sync
    else
		STICKGONE=0
		while [ $STICKGONE -ne 1 ]; do
			killall wbc_status > /dev/null 2>&1
			if [ "$ENABLE_QOPENHD" == "Y" ]; then
			    qstatus "ERROR: Could not access USB memory stick!" 5
			else
			    wbc_status "ERROR: Could not access USB memory stick!" 7 65 0 &
			fi
			sleep 4
			if [ ! -e "/dev/sda" ]; then
				STICKGONE=1
			fi
		done
		killall wbc_status > /dev/null 2>&1
		killall hello_video.bin.player > /dev/null 2>&1
    fi

    #killall tracker
    # re-start video/telemetry recording
    if [ "$VIDEO_TMP" != "none" ]; then
        ionice -c 3 nice cat /root/videofifo3 >> $VIDEOFILE &
    fi
    ionice -c 3 nice cat /root/telemetryfifo3 >> /wbc_tmp/telemetrydowntmp.raw &

    killall wbc_status > /dev/null 2>&1
    OSDRUNNING=`pidof /tmp/osd | wc -w`
    if [ $OSDRUNNING  -ge 1 ]; then
		echo "OSD already running!"
    else
		killall wbc_status > /dev/null 2>&1
		if [ "$ENABLE_QOPENHD" != "Y" ]; then
			/tmp/osd >> /wbc_tmp/telemetrydowntmp.txt &
		fi
    fi
	
    # let screenshot function know that it can continue taking screenshots
    rm /tmp/pausewhile
}
