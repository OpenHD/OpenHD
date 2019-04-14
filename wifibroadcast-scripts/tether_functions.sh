function MAIN_TETHER_FUNCTION {
	echo "================== CHECK TETHER (tty7) ==========================="
	
	if [ "$CAM" == "0" ]; then
	    echo "Waiting some time until everything else is running ..."
	    sleep 6
	    tether_check_function
	else
	    echo "Cam found, we are TX, Check tether function disabled"
	    sleep 365d
	fi
}


function tether_check_function {
	while true; do
	    # pause loop while saving is in progress
	    pause_while
	    if [ -d "/sys/class/net/usb0" ]; then
	    	echo
			echo "USB tethering device detected. Configuring IP ..."
		
			nice pump -h wifibrdcast -i usb0 --no-dns --keep-up --no-resolvconf --no-ntp || {
				echo "ERROR: Could not configure IP for USB tethering device!"
				nice killall wbc_status > /dev/null 2>&1
				nice /home/pi/wifibroadcast-status/wbc_status "ERROR: Could not configure IP for USB tethering device!" 7 55 0
				collect_errorlog
				sleep 365d
			}
			
			# find out smartphone IP to send video stream to
			PHONE_IP=`ip route show 0.0.0.0/0 dev usb0 | cut -d\  -f3`
			echo "Android IP: $PHONE_IP"

			nice socat -b $TELEMETRY_UDP_BLOCKSIZE GOPEN:/root/telemetryfifo2 UDP4-SENDTO:$PHONE_IP:$TELEMETRY_UDP_PORT &
			nice /home/pi/wifibroadcast-base/rssi_forward $PHONE_IP 5003 &
			nice /home/pi/wifibroadcast-base/rssi_qgc_forward $PHONE_IP 5154 &

			if [ "$FORWARD_STREAM" == "rtp" ]; then
				ionice -c 1 -n 4 nice -n -5 cat /root/videofifo2 | nice -n -5 gst-launch-1.0 fdsrc ! h264parse ! rtph264pay pt=96 config-interval=5 ! udpsink port=$VIDEO_UDP_PORT host=$PHONE_IP > /dev/null 2>&1 &
			else
				ionice -c 1 -n 4 nice -n -10 socat -b $VIDEO_UDP_BLOCKSIZE GOPEN:/root/videofifo2 UDP4-SENDTO:$PHONE_IP:$VIDEO_UDP_PORT &
			fi

			if cat /boot/osdconfig.txt | grep -q "^#define MAVLINK"; then
				cat /root/telemetryfifo5 > /dev/pts/0 &
				if [ "$MAVLINK_FORWARDER" == "mavlink-routerd" ]; then
					ionice -c 3 nice /home/pi/mavlink-router/mavlink-routerd -e $PHONE_IP:14550 /dev/pts/1:57600 &
				else
					cp /boot/cmavnode.conf /tmp/
					echo "targetip=$PHONE_IP" >> /tmp/cmavnode.conf
					ionice -c 3 nice /home/pi/cmavnode/build/cmavnode --file /tmp/cmavnode.conf &
				fi

				if [ "$DEBUG" == "Y" ]; then
					tshark -i usb0 -f "udp and port 14550" -w /wbc_tmp/mavlink`date +%s`.pcap &
				fi
			fi

			if [ "$TELEMETRY_UPLINK" == "msp" ]; then
				cat /root/mspfifo > /dev/pts/2 &
				#socat /dev/pts/3 tcp-listen:23
				ser2net
			fi

			# kill and pause OSD so we can safeley start wbc_status
			ps -ef | nice grep "osd" | nice grep -v grep | awk '{print $2}' | xargs kill -9

			killall wbc_status > /dev/null 2>&1
			if [ "$QUIET" == "N" ]; then
			    nice /home/pi/wifibroadcast-status/wbc_status "Secondary display connected (USB)" 7 55 0
                        fi
			# re-start osd
			killall wbc_status > /dev/null 2>&1
			OSDRUNNING=`pidof /tmp/osd | wc -w`
			if [ $OSDRUNNING  -ge 1 ]; then
				echo "OSD already running!"
			else
				killall wbc_status > /dev/null 2>&1
				/tmp/osd >> /wbc_tmp/telemetrydowntmp.txt &
			fi

			# check if smartphone has been disconnected
			PHONETHERE=1
			while [  $PHONETHERE -eq 1 ]; do
				if [ -d "/sys/class/net/usb0" ]; then
					PHONETHERE=1
					echo "Android device still connected ..."
				else
					echo "Android device gone"
					# kill and pause OSD so we can safeley start wbc_status
					ps -ef | nice grep "osd" | nice grep -v grep | awk '{print $2}' | xargs kill -9
					killall wbc_status > /dev/null 2>&1
					if [ "$QUIET" == "N" ]; then
					    nice /home/pi/wifibroadcast-status/wbc_status "Secondary display disconnected (USB)" 7 55 0
					fi
					# re-start osd
					OSDRUNNING=`pidof /tmp/osd | wc -w`
					if [ $OSDRUNNING  -ge 1 ]; then
						echo "OSD already running!"
					else
						killall wbc_status > /dev/null 2>&1
						/tmp/osd >> /wbc_tmp/telemetrydowntmp.txt &
					fi
					
					PHONETHERE=0
					
					# kill forwarding of video and osd to secondary display
					ps -ef | nice grep "socat -b $VIDEO_UDP_BLOCKSIZE GOPEN:/root/videofifo2" | nice grep -v grep | awk '{print $2}' | xargs kill -9
					ps -ef | nice grep "gst-launch-1.0 fdsrc" | nice grep -v grep | awk '{print $2}' | xargs kill -9
					ps -ef | nice grep "cat /root/videofifo2" | nice grep -v grep | awk '{print $2}' | xargs kill -9
					ps -ef | nice grep "socat -b $TELEMETRY_UDP_BLOCKSIZE GOPEN:/root/telemetryfifo2" | nice grep -v grep | awk '{print $2}' | xargs kill -9
					ps -ef | nice grep "cat /root/telemetryfifo5" | nice grep -v grep | awk '{print $2}' | xargs kill -9
					ps -ef | nice grep "cmavnode" | nice grep -v grep | awk '{print $2}' | xargs kill -9
					ps -ef | nice grep "mavlink-routerd" | nice grep -v grep | awk '{print $2}' | xargs kill -9
					ps -ef | nice grep "tshark" | nice grep -v grep | awk '{print $2}' | xargs kill -9
					ps -ef | nice grep "rssi_forward" | nice grep -v grep | awk '{print $2}' | xargs kill -9
					ps -ef | nice grep "rssi_qgc_forward" | nice grep -v grep | awk '{print $2}' | xargs kill -9
					
					# kill msp processes
					ps -ef | nice grep "cat /root/mspfifo" | nice grep -v grep | awk '{print $2}' | xargs kill -9
					#ps -ef | nice grep "socat /dev/pts/3" | nice grep -v grep | awk '{print $2}' | xargs kill -9
					ps -ef | nice grep "ser2net" | nice grep -v grep | awk '{print $2}' | xargs kill -9
				fi
				
				sleep 1
			done
	    else
			echo "Android device not detected ..."
	    fi
		
	    sleep 1
	done
}
