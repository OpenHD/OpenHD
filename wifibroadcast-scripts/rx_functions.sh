function rx_function {

    if [ "$AdjustLCDBacklight" == "Y" ]; then
        /home/pi/wifibroadcast-misc/LCD/MouseListener $AutoDimTime $AutoDimValue /dev/input/event0 & > /dev/null 2>&1
    fi
    
    /home/pi/wifibroadcast-base/sharedmem_init_rx

    # start virtual serial port for cmavnode and ser2net
    ionice -c 3 nice socat -lf /wbc_tmp/socat1.log -d -d pty,raw,echo=0 pty,raw,echo=0 & > /dev/null 2>&1
    sleep 1
    ionice -c 3 nice socat -lf /wbc_tmp/socat2.log -d -d pty,raw,echo=0 pty,raw,echo=0 & > /dev/null 2>&1
    sleep 1
    # setup virtual serial ports
    stty -F /dev/pts/0 -icrnl -ocrnl -imaxbel -opost -isig -icanon -echo -echoe -ixoff -ixon 57600
    stty -F /dev/pts/1 -icrnl -ocrnl -imaxbel -opost -isig -icanon -echo -echoe -ixoff -ixon 115200

    echo

    # if USB memory stick is already connected during startup, notify user
    # and pause as long as stick is not removed
    # some sticks show up as sda1, others as sda, check for both
    if [ -e "/dev/sda1" ]; then
		STARTUSBDEV="/dev/sda1"
    else
		STARTUSBDEV="/dev/sda"
    fi

    if [ -e $STARTUSBDEV ]; then
        touch /tmp/donotsave
        STICKGONE=0
	while [ $STICKGONE -ne 1 ]; do
	    killall wbc_status > /dev/null 2>&1
	    nice /home/pi/wifibroadcast-status/wbc_status "USB memory stick detected - please remove and re-plug after flight" 7 65 0 &
	    sleep 4
	    if [ ! -e $STARTUSBDEV ]; then
			STICKGONE=1
			rm /tmp/donotsave
	    fi
	done
    fi

    killall wbc_status > /dev/null 2>&1

    sleep 1
    detect_nics
    
    if [ "$Bandwidth" == "10" ]; then
        echo "HardCode dirty code for tests only. Values are it Hex, to set 10MHz use 0xa (10 in dec)"
        echo 0xa > /sys/kernel/debug/ieee80211/phy0/ath9k_htc/chanbw
        echo 0xa > /sys/kernel/debug/ieee80211/phy1/ath9k_htc/chanbw
        echo 0xa > /sys/kernel/debug/ieee80211/phy2/ath9k_htc/chanbw
        echo 0xa > /sys/kernel/debug/ieee80211/phy3/ath9k_htc/chanbw
        echo 0xa > /sys/kernel/debug/ieee80211/phy4/ath9k_htc/chanbw
    fi

    if [ "$Bandwidth" == "5" ]; then
        echo "HardCode dirty code for tests only. Values are it Hex, to set 10MHz use 0xa (10 in dec)"
        echo 5 > /sys/kernel/debug/ieee80211/phy0/ath9k_htc/chanbw
        echo 5 > /sys/kernel/debug/ieee80211/phy1/ath9k_htc/chanbw
        echo 5 > /sys/kernel/debug/ieee80211/phy2/ath9k_htc/chanbw
        echo 5 > /sys/kernel/debug/ieee80211/phy3/ath9k_htc/chanbw
        echo 5 > /sys/kernel/debug/ieee80211/phy4/ath9k_htc/chanbw
    fi

    sleep 0.5

    # videofifo1: local display, hello_video.bin
    # videofifo2: secondary display, hotspot/usb-tethering
    # videofifo3: recording
    # videofifo4: wbc relay

    if [ "$VIDEO_TMP" == "sdcard" ]; then
		touch /tmp/pausewhile # make sure check_alive doesn't do it's stuff ...
		tmessage "Saving to SDCARD enabled, preparing video storage ..."
		if cat /proc/partitions | nice grep -q mmcblk0p3; then 
			echo
			echo "SDCARD Video partion detected.."
		else
			echo
			echo "SDCARD Video partion NOT detected.."
			echo 

		#	echo -e "n\np\n3\n3674112\n\nw" | fdisk /dev/mmcblk0 > /dev/null 2>&1
		
			echo -e "n\np\n3\n7839744\n\nw" | fdisk /dev/mmcblk0 > /dev/null 2>&1
			partprobe > /dev/null 2>&1
			
			if [ "$VIDEO_FS" == "fat" ]; then
				tmessage "Creating SDCARD FAT filesytem for Video Recording.."
				mkfs.vfat /dev/mmcblk0p3 -n myvideo > /boot/sdcard.txt 2>&1 || {
				tmessage "ERROR: Could not format video storage on SDCARD!"
				collect_errorlog
				sleep 365d 
				}
				mkdir -p /video_tmp
				mount -t vfat /dev/mmcblk0p3 /video_tmp > /dev/null 2>&1 || {
				tmessage "ERROR: Could not mount video storage on SDCARD!"
				collect_errorlog
				sleep 365d
				}
			else
				tmessage "Creating SDCARD EXT4 filesytem for Video Recording.."
				mkfs.ext4 /dev/mmcblk0p3 -L myvideo -F > /dev/null 2>&1 || {
				tmessage "ERROR: Could not format video storage on SDCARD!"
				collect_errorlog
				sleep 365d 
				}				
				e2fsck -p /dev/mmcblk0p3 
				mkdir -p /video_tmp 
				mount -t ext4 /dev/mmcblk0p3 /video_tmp > /dev/null 2>&1 || {
				tmessage "ERROR: Could not mount video storage on SDCARD!"
				collect_errorlog
				sleep 365d
				}
			fi

		fi

		
		VIDEOFILE=/video_tmp/videotmp.raw
		echo "VIDEOFILE=/video_tmp/videotmp.raw" > /tmp/videofile
		rm $VIDEOFILE > /dev/null 2>&1		
    else
		VIDEOFILE=/wbc_tmp/videotmp.raw
		echo "VIDEOFILE=/wbc_tmp/videotmp.raw" > /tmp/videofile
    fi


	# tracker disabled
    #/home/pi/wifibroadcast-base/tracker /wifibroadcast_rx_status_0 >> /wbc_tmp/tracker.txt &
    #sleep 1

    killall wbc_status > /dev/null 2>&1

    if [ "$AIRODUMP" == "Y" ]; then
		touch /tmp/pausewhile # make sure check_alive doesn't do it's stuff ...
		
		echo "AiroDump is enabled, running airodump-ng for $AIRODUMP_SECONDS seconds ..."
		sleep 3
		
		# strip newlines
		NICS_COMMA=`echo $NICS | tr '\n' ' '`
		# strip space at end
		NICS_COMMA=`echo $NICS_COMMA | sed 's/ *$//g'`
		# replace spaces by comma
		NICS_COMMA=${NICS_COMMA// /,}

		if [ "$FREQ" -gt 3000 ]; then
			AIRODUMP_CHANNELS="5180,5200,5220,5240,5260,5280,5300,5320,5500,5520,5540,5560,5580,5600,5620,5640,5660,5680,5700,5745,5765,5785,5805,5825"
		else
			AIRODUMP_CHANNELS="2412,2417,2422,2427,2432,2437,2442,2447,2452,2457,2462,2467,2472"
		fi

		airodump-ng --showack -h --berlin 60 --ignore-negative-one --manufacturer --output-format pcap --write /wbc_tmp/wifiscan --write-interval 2 -C $AIRODUMP_CHANNELS  $NICS_COMMA &
		sleep $AIRODUMP_SECONDS
		sleep 2
		
		ionice -c 3 nice -n 19 /home/pi/wifibroadcast-misc/raspi2png -p /wbc_tmp/airodump.png >> /dev/null
		killall airodump-ng
		
		sleep 1
		
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

    if vcgencmd get_throttled | nice grep -q -v "0x0"; then
		TEMP=`cat /sys/class/thermal/thermal_zone0/temp`
		TEMP_C=$(($TEMP/1000))
		if [ "$TEMP_C" -lt 75 ]; then
			echo "  ---------------------------------------------------------------------------------------------------"
			echo "  | ERROR: Under-Voltage detected on the GROUNDPi. Your Pi is not supplied with stable 5 Volts.        |"
			echo "  |                                                                                                 |"
			echo "  | Either your power-supply or wiring is not sufficent, check the wiring instructions in the Wiki! |"
			echo "  ---------------------------------------------------------------------------------------------------"
			echo "1" >> /tmp/undervolt
			sleep 5
		fi
		echo "0" >> /tmp/undervolt
		else
		echo "0" >> /tmp/undervolt

	fi

    if [ -e "/tmp/pausewhile" ]; then
		rm /tmp/pausewhile # remove pausewhile file to make sure check_alive and everything runs again
    fi

    IsFirstTime=0;
    while true; do
        pause_while
	
		if [ $IsFirstTime -eq 0 ]; then
                        killall omxplayer  > /dev/null 2>/dev/null
                        killall omxplayer.bin  > /dev/null 2>/dev/null
                fi

		ionice -c 1 -n 4 nice -n -10 cat /root/videofifo1 | ionice -c 1 -n 4 nice -n -10 $DISPLAY_PROGRAM > /dev/null 2>&1 &
		ionice -c 3 nice cat /root/videofifo3 >> $VIDEOFILE &

		if [ "$RELAY" == "Y" ]; then
		        /root/wifibroadcast/sharedmem_init_tx
			ionice -c 1 -n 4 nice -n -10 cat /root/videofifo4 | /home/pi/wifibroadcast-base/tx_rawsock -p 0 -b $RELAY_VIDEO_BLOCKS -r $RELAY_VIDEO_FECS -f $RELAY_VIDEO_BLOCKLENGTH -t $VIDEO_FRAMETYPE -d 24 -y 0 relay0 > /dev/null 2>&1 &
		fi

		# update NICS variable in case a NIC has been removed (exclude devices with wlanx)
		NICS=`ls /sys/class/net/ | nice grep -v eth0 | nice grep -v lo | nice grep -v usb | nice grep -v intwifi | nice grep -v wlan | nice grep -v relay | nice grep -v wifihotspot`

		tmessage "Starting RX ... (FEC: $VIDEO_BLOCKS/$VIDEO_FECS/$VIDEO_BLOCKLENGTH)"
		
	#START AUDIO AND REMOTE SETTINGS
		if [ $IsFirstTime -eq 0 ]; then
			if [ "$IsAudioTransferEnabled" == "1" ]; then
			      echo "AUDIO ENABLED..."
				amixer cset numid=3 $DefaultAudioOut
				/home/pi/RemoteSettings/Ground/AudioPlayback.sh &
				/home/pi/RemoteSettings/Ground/RxAudio.sh &
			fi

			if [ "$RemoteSettingsEnabled" != "0" ]; then
				echo "SETTINGS CHANGE MODULE ENABLED..."
				/home/pi/RemoteSettings/ipchecker/iphelper.sh > /dev/null 2>&1 &
				/usr/bin/python3.5 /home/pi/RemoteSettings/RemoteSettings.py > /dev/null 2>&1 &
				/home/pi/RemoteSettings/RemoteSettingsWFBC_UDP.sh > /dev/null 2>&1 &
				/home/pi/RemoteSettings/GroundRSSI.sh &
			fi
			
			#if [ "$IsBandSwicherEnabled" == "1" ]; then
			    echo "BAND SWITCHER ENABLED...."
        			/home/pi/RemoteSettings/BandSwitcher.sh &
			#fi
		fi
		IsFirstTime=1
		#MYADDEND
		
		
		ionice -c 1 -n 3 /home/pi/wifibroadcast-base/rx -p 0 -d 1 -b $VIDEO_BLOCKS -r $VIDEO_FECS -f $VIDEO_BLOCKLENGTH $NICS | ionice -c 1 -n 4 nice -n -10 tee >(ionice -c 1 -n 4 nice -n -10 /home/pi/wifibroadcast-misc/ftee /root/videofifo2 > /dev/null 2>&1) >(ionice -c 1 nice -n -10 /home/pi/wifibroadcast-misc/ftee /root/videofifo4 > /dev/null 2>&1) >(ionice -c 3 nice /home/pi/wifibroadcast-misc/ftee /root/videofifo3 > /dev/null 2>&1) | ionice -c 1 -n 4 nice -n -10 /home/pi/wifibroadcast-misc/ftee /root/videofifo1 > /dev/null 2>&1

		RX_EXITSTATUS=${PIPESTATUS[0]}
		check_exitstatus $RX_EXITSTATUS
		ps -ef | nice grep "$DISPLAY_PROGRAM" | nice grep -v grep | awk '{print $2}' | xargs kill -9
		ps -ef | nice grep "rx -p 0" | nice grep -v grep | awk '{print $2}' | xargs kill -9
		ps -ef | nice grep "ftee /root/videofifo" | nice grep -v grep | awk '{print $2}' | xargs kill -9
		ps -ef | nice grep "cat /root/videofifo" | nice grep -v grep | awk '{print $2}' | xargs kill -9
    done
}
