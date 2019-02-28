function MAIN_HOTSPOT_FUNCTION {
	echo "================== CHECK HOTSPOT (tty8) ==========================="
	if [ "$CAM" == "0" ]; then
	    if [ "$ETHERNET_HOTSPOT" == "Y" ] || [ "$WIFI_HOTSPOT" == "Y" ]; then
			echo
			echo -n "Waiting until video is running ..."
			HVIDEORXRUNNING=0
		
			while [ $HVIDEORXRUNNING -ne 1 ]; do
				sleep 0.5
				HVIDEORXRUNNING=`pidof $DISPLAY_PROGRAM | wc -w`
				echo -n "."
			done
			
			echo
			echo "Video running, starting hotspot processes ..."
			
			sleep 1
			
			hotspot_check_function
	    else
			echo "Check hotspot function not enabled in config file"
			sleep 365d
	    fi
	else
	    echo "Check hotspot function not enabled - we are TX (Air Pi)"
	    sleep 365d
	fi
}


function hotspot_check_function {
    
	
			
	if [ "$ETHERNET_HOTSPOT" == "Y" ]; then
	    # setup hotspot on RPI3 internal ethernet chip
	    # Convert hostap config from DOS format to UNIX format
	    ionice -c 3 nice dos2unix -n /boot/apconfig.txt /tmp/apconfig.txt
	    nice ifconfig eth0 192.168.1.1 up
	    nice udhcpd -I 192.168.1.1 /etc/udhcpd-eth.conf
	fi

	if [ "$WIFI_HOTSPOT" != "N" ]; then	
	         # Find capabilities of this pi
		 # IEEE 802.11bgn = 2.4hz only
                 # IEEE 802.11gn = 2.4hz only
                 # IEEE 802.11agn = 2.4hz + 5hz
		  
		  IW=$(iwconfig wlan0)

                  # capture each parameter in a variable
                  IEEE=$(echo "$IW" | grep -oP '(?<=IEEE ).[^\s]*')
		  
		  case "$IEEE" in
                       '802.11bgn')
		        ABLE_BAND=g
			;;
                  case "$IEEE" in
                       '802.11gn')
		        ABLE_BAND=g
			;;
		  case "$IEEE" in
                       '802.11agn')
		        ABLE_BAND=ag
			;;
	          esac
	
	# CONTINUE IF WE ARE ABLE_BAND IS A or G
	if ["$ABLE_BAND" == "g" ] || ["$ABLE_BAND" == "ag" ]; then
	
	    # Read if hotspot config is auto
	     if [ "$WIFI_HOTSPOT" == "auto"]; then	 
	     
	        # for both a and g ability choose opposite of video	     
	       if [ "$FREQ" -gt "3000" ]; then
	       
	         if ["ABLE_BAND" == "ag"] 
	         HOTSPOT_BAND=a
		 fi
		 
	       else
	         HOTSPOT_BAND=g
	       fi
	     # NOTHING TO DO For user defined use of A (5.8ghz) OR G (2.4ghz) 
	     fi
	     	     		
	    # set A OR G
	    THISCONFIG="/boot/apconfig.txt"
	    SOURCE $THISCONFIG
	    hw_mode=$HOTSPOT_BAND
	    set_config hw_mode $hw_mode
	    
	    # Convert hostap config from DOS format to UNIX format
	    ionice -c 3 nice dos2unix -n /boot/apconfig.txt /tmp/apconfig.txt
		
	    echo "setting up hotspot with mode $HOTSPOT_BAND on channel $HOTSPOT_CHANNEL"
	    nice udhcpd -I 192.168.2.1 /etc/udhcpd-wifi.conf
	    nice -n 5 hostapd -B -d /tmp/apconfig.txt
	  fi   
	fi

	while true; do
	    # pause loop while saving is in progress
	    pause_while
	    pidRX=`ps -ef | nice grep "rx -p 0" | nice grep -v grep | awk '{print $2}'`
	    IP=0
	    if [ "$ETHERNET_HOTSPOT" == "Y" ]; then
			if nice ping -I eth0 -c 1 -W 1 -n -q 192.168.1.2 > /dev/null 2>&1; then
				IP="192.168.1.2"
				echo "Ethernet device detected. IP: $IP"
				
				nice socat -b $TELEMETRY_UDP_BLOCKSIZE GOPEN:/root/telemetryfifo2 UDP4-SENDTO:$IP:$TELEMETRY_UDP_PORT &
				nice /home/pi/wifibroadcast-base/rssi_forward $IP 5003 &
				
				if [ "$FORWARD_STREAM" == "rtp" ]; then
					ionice -c 1 -n 4 nice -n -5 cat /root/videofifo2 | nice -n -5 gst-launch-1.0 fdsrc ! h264parse ! rtph264pay pt=96 config-interval=5 ! udpsink port=$VIDEO_UDP_PORT host=$IP > /dev/null 2>&1 &
				else
					ionice -c 1 -n 4 nice -n -10 socat -b $VIDEO_UDP_BLOCKSIZE GOPEN:/root/videofifo2 UDP4-SENDTO:$IP:$VIDEO_UDP_PORT &
				fi
				
				if cat /boot/osdconfig.txt | grep -q "^#define MAVLINK"; then
					nice cat /root/telemetryfifo5 > /dev/pts/0 &
					if [ "$MAVLINK_FORWARDER" == "mavlink-routerd" ]; then
						ionice -c 3 nice /home/pi/mavlink-router/mavlink-routerd -e $IP:14550 /dev/pts/1:57600 &
					else
						cp /boot/cmavnode.conf /tmp/
						echo "targetip=$IP" >> /tmp/cmavnode.conf
						ionice -c 3 nice /home/pi/cmavnode/build/cmavnode --file /tmp/cmavnode.conf &
					fi
					
					if [ "$DEBUG" == "Y" ]; then
						tshark -i eth0 -f "udp and port 14550" -w /wbc_tmp/mavlink`date +%s`.pcap &
					fi
				fi
				
				if [ "$TELEMETRY_UPLINK" == "msp" ]; then
					cat /root/mspfifo > /dev/pts/2 &
					#socat /dev/pts/3 TCP-LISTEN:23
					ser2net
				fi
			fi
	    fi
		
	    if [ "$WIFI_HOTSPOT" == "Y" ]; then
			if nice ping -I wifihotspot0 -c 2 -W 1 -n -q 192.168.2.2 > /dev/null 2>&1; then
				IP="192.168.2.2"
				echo "Wifi device detected. IP: $IP"
				
				nice socat -b $TELEMETRY_UDP_BLOCKSIZE GOPEN:/root/telemetryfifo2 UDP4-SENDTO:$IP:$TELEMETRY_UDP_PORT &
				nice /home/pi/wifibroadcast-base/rssi_forward $IP 5003 &
				
				if [ "$FORWARD_STREAM" == "rtp" ]; then
					ionice -c 1 -n 4 nice -n -5 cat /root/videofifo2 | nice -n -5 gst-launch-1.0 fdsrc ! h264parse ! rtph264pay pt=96 config-interval=5 ! udpsink port=$VIDEO_UDP_PORT host=$IP > /dev/null 2>&1 &
				else
					ionice -c 1 -n 4 nice -n -10 socat -b $VIDEO_UDP_BLOCKSIZE GOPEN:/root/videofifo2 UDP4-SENDTO:$IP:$VIDEO_UDP_PORT &
				fi
				
				if cat /boot/osdconfig.txt | grep -q "^#define MAVLINK"; then
					cat /root/telemetryfifo5 > /dev/pts/0 &
					
					if [ "$MAVLINK_FORWARDER" == "mavlink-routerd" ]; then
						ionice -c 3 nice /home/pi/mavlink-router/mavlink-routerd -e $IP:14550 /dev/pts/1:57600 &
					else
						cp /boot/cmavnode.conf /tmp/
						echo "targetip=$IP" >> /tmp/cmavnode.conf
						ionice -c 3 nice /home/pi/cmavnode/build/cmavnode --file /tmp/cmavnode.conf &
					fi
					
					if [ "$DEBUG" == "Y" ]; then
						tshark -i wifihotspot0 -f "udp and port 14550" -w /wbc_tmp/mavlink`date +%s`.pcap &
					fi
				fi

				if [ "$TELEMETRY_UPLINK" == "msp" ]; then
					cat /root/mspfifo > /dev/pts/2 &
					#socat /dev/pts/3 TCP-LISTEN:23
					ser2net
				fi
			fi
	    fi
		
	    if [ "$IP" != "0" ]; then
			# kill and pause OSD so we can safeley start wbc_status
			ps -ef | nice grep "osd" | nice grep -v grep | awk '{print $2}' | xargs kill -9

	        killall wbc_status > /dev/null 2>&1
		    if [ "$QUIET" == "N" ]; then
			nice /home/pi/wifibroadcast-status/wbc_status "Secondary display connected (Hotspot)" 7 55 0
                    fi
			# re-start osd
			OSDRUNNING=`pidof /tmp/osd | wc -w`
			if [ $OSDRUNNING  -ge 1 ]; then
				echo "OSD already running!"
			else
				killall wbc_status > /dev/null 2>&1
				/tmp/osd >> /wbc_tmp/telemetrydowntmp.txt &
			fi

			# check if connection is still connected
			IPTHERE=1
			while [  $IPTHERE -eq 1 ]; do
				pidRXNow=`ps -ef | nice grep "rx -p 0" | nice grep -v grep | awk '{print $2}'`
                        	#if ping -c 2 -W 1 -n -q $IP > /dev/null 2>&1 && [ "$pidRX" == "$pidRXNow"  ]; then
				ping -c 3 -W 1 -n -q $IP > /dev/null 2>&1
				if [ $? -eq 0 ] && [ "$pidRX" == "$pidRXNow"  ]; then
					IPTHERE=1
					echo "IP $IP still connected ..."
					else
						echo "IP $IP gone. Check 2..."
						ping -c 3 -W 1 -n -q $IP > /dev/null 2>&1
						if [ $? -ne 0 ] || [ "$pidRX" != "$pidRXNow"  ]; then
							echo "IP $IP gone. Check 2 - gone."
							# kill and pause OSD so we can safeley start wbc_status
							ps -ef | nice grep "osd" | nice grep -v grep | awk '{print $2}' | xargs kill -9

							killall wbc_status > /dev/null 2>&1
							if [ "$QUIET" == "N" ]; then
							   nice /home/pi/wifibroadcast-status/wbc_status "Secondary display disconnected (Hotspot)" 7 55 0
							fi
							# re-start osd
							OSDRUNNING=`pidof /tmp/osd | wc -w`
							if [ $OSDRUNNING  -ge 1 ]; then
								echo "OSD already running!"
							else
								killall wbc_status > /dev/null 2>&1
								OSDRUNNING=`pidof /tmp/osd | wc -w`
								if [ $OSDRUNNING  -ge 1 ]; then
									echo "OSD already running!"
								else
									killall wbc_status > /dev/null 2>&1
									/tmp/osd >> /wbc_tmp/telemetrydowntmp.txt &
								fi
							fi

							IPTHERE=0					
							# kill forwarding of video and telemetry to secondary display
							ps -ef | nice grep "socat -b $VIDEO_UDP_BLOCKSIZE GOPEN:/root/videofifo2" | nice grep -v grep | awk '{print $2}' | xargs kill -9
							ps -ef | nice grep "gst-launch-1.0 fdsrc" | nice grep -v grep | awk '{print $2}' | xargs kill -9
							ps -ef | nice grep "cat /root/videofifo2" | nice grep -v grep | awk '{print $2}' | xargs kill -9
							ps -ef | nice grep "socat -b $TELEMETRY_UDP_BLOCKSIZE GOPEN:/root/telemetryfifo2" | nice grep -v grep | awk '{print $2}' | xargs kill -9
							ps -ef | nice grep "cat /root/telemetryfifo5" | nice grep -v grep | awk '{print $2}' | xargs kill -9
							ps -ef | nice grep "cmavnode" | nice grep -v grep | awk '{print $2}' | xargs kill -9
							ps -ef | nice grep "mavlink-routerd" | nice grep -v grep | awk '{print $2}' | xargs kill -9
							ps -ef | nice grep "tshark" | nice grep -v grep | awk '{print $2}' | xargs kill -9
							ps -ef | nice grep "rssi_forward" | nice grep -v grep | awk '{print $2}' | xargs kill -9

							# kill msp processes
							ps -ef | nice grep "cat /root/mspfifo" | nice grep -v grep | awk '{print $2}' | xargs kill -9
							#ps -ef | nice grep "socat /dev/pts/3" | nice grep -v grep | awk '{print $2}' | xargs kill -9
							ps -ef | nice grep "ser2net" | nice grep -v grep | awk '{print $2}' | xargs kill -9
						fi
				fi
				
				sleep 1
			done
	    else
			echo "No IP detected ..."
	    fi
		
	    sleep 1
	done
}
