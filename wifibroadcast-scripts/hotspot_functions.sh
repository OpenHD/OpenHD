function MAIN_HOTSPOT_FUNCTION {
	echo "================== CHECK HOTSPOT (tty8) ==========================="
	if [ "$CAM" == "0" ]; then
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
	    echo "Check hotspot function not enabled - we are TX (Air Pi)"
	    sleep 365d
	fi
}


function hotspot_check_function {

	# Convert hostap config from DOS format to UNIX format
	ionice -c 3 nice dos2unix -n /boot/apconfig.txt /tmp/apconfig.txt

            pause_while

         nice cat /root/telemetryfifo5 > /dev/pts/0 &
        /home/pi/mavlink-router/mavlink-routerd  /dev/pts/1:57600 &
        #we still can have USB phone connected anytime. So, start programs anyway
        #Maybe add code inside USB tethering file to check  HOTSPOT is off and phone connected - start....
        #if [ "$ETHERNET_HOTSPOT" == "Y" ] || [ "$WIFI_HOTSPOT" != "N" ]; then
                /home/pi/wifibroadcast-scripts/UDPsplitterhelper.sh 9121 5621 $VIDEO_UDP_PORT2 &  #Secondary video stream
                /home/pi/wifibroadcast-scripts/UDPsplitterhelper.sh 9120 5620 $VIDEO_UDP_PORT &  #Main video stream

                if [ "$FORWARD_STREAM" == "rtp" ]; then
                        echo "ionice -c 1 -n 4 nice -n -5 cat /root/videofifo2 | nice -n -5 gst-launch-1.0 fdsrc ! h264parse ! rtph264pay pt=96 config-interval=5 ! udpsink port=5620 host=127.0.0.1 > /dev/null 2>&1 &" > /tmp/ForwardRTPMainCamera.sh
                else
                        echo "ionice -c 1 -n 4 nice -n -10 socat -b $VIDEO_UDP_BLOCKSIZE GOPEN:/root/videofifo2 UDP4-SENDTO:127.0.0.1:5620 &" > /tmp/ForwardRTPMainCamera.sh
                fi
                chmod +x /tmp/ForwardRTPMainCamera.sh
                /tmp/ForwardRTPMainCamera.sh &
        #fi

        #redirect telemetry to UDP splitter
        nice socat -b $TELEMETRY_UDP_BLOCKSIZE GOPEN:/root/telemetryfifo2 UDP4-SENDTO:127.0.0.1:6610 &
        /home/pi/wifibroadcast-scripts/UDPsplitterhelper.sh 9122 6610 $TELEMETRY_UDP_PORT &


        nice /home/pi/wifibroadcast-base/rssi_forward 127.0.0.1 5003 &
        /home/pi/wifibroadcast-scripts/UDPsplitterhelper.sh 9123 5003 5003 &

        nice /home/pi/wifibroadcast-base/rssi_qgc_forward 127.0.0.1 5154 &
        /home/pi/wifibroadcast-scripts/UDPsplitterhelper.sh 9124 5154 5154 &

	##OpenHD RemoteSettings android app
	/home/pi/wifibroadcast-scripts/UDPsplitterhelper.sh 9125 5116 5115 &

        # used for QOpenHD when running on the ground pi itself
        nice /home/pi/wifibroadcast-base/rssi_qgc_forward 127.0.0.1 5155 &


        #if [ "$TELEMETRY_UPLINK" == "msp" ]; then
        #       cat /root/mspfifo > /dev/pts/2 &
        #       ser2net
        #fi

	if [ "$ETHERNET_HOTSPOT" == "Y" ]; then
	    # setup hotspot on RPI3 internal ethernet chip
	    nice ifconfig eth0 192.168.1.1 up
	    nice /usr/sbin/dnsmasq --conf-file=/etc/dnsmasqEth0.conf
	fi

	if [ "$WIFI_HOTSPOT" != "N" ]; then
			
	         # Detect cpu revision pi
		  detect_hardware

			echo "This Pi model $MODEL with Band $ABLE_BAND"
	
		# CONTINUE IF WE ARE ABLE_BAND IS A or G
		if [ "$ABLE_BAND" != "unknown" ]; then
			echo "Setting up Hotspot..."

	    		# Read if hotspot config is auto
	     		if [ "$WIFI_HOTSPOT" == "auto" ]&&[ "$WIFI_HOTSPOT_NIC" == "internal" ]; then	
				echo "wifihotspot auto..."

	        		# for both a and g ability choose opposite of video	   	         	
				if [ "$ABLE_BAND" == "ag" ]; then
					echo "Dual Band capable..."

					if [ "$FREQ" -gt "3000" ]; then
	         			HOTSPOT_BAND=g
					HOTSPOT_CHANNEL=7
	       				else
	         			HOTSPOT_BAND=a
					HOTSPOT_CHANNEL=36
					fi
	       			
				# for g ability only choose furthest freq from video
				else
					echo "G Band only capable..."
					HOTSPOT_BAND=g

					if [ "$FREQ" -gt "3000" ]; then					
					HOTSPOT_CHANNEL=1
					else	         							   			
					echo "Hotspot Disabled. Not recommended to share same band as video..."	
					echo "If you still want hotspot you must manually set up..."
					#kill the function	
					return 1
					fi
				fi
	     		# NOTHING TO DO For user defined use of A (5.8ghz) OR G (2.4ghz) 

	     		fi

		#populate $hw_mode and channel
		source /tmp/apconfig.txt

		sudo sed -i -e "s/hw_mode=$hw_mode/hw_mode=$HOTSPOT_BAND/g" /tmp/apconfig.txt
		sudo sed -i -e "s/channel=$channel/channel=$HOTSPOT_CHANNEL/g" /tmp/apconfig.txt

	    	echo "setting up hotspot with mode $HOTSPOT_BAND on channel $HOTSPOT_CHANNEL"
		tmessage "setting up hotspot with mode $HOTSPOT_BAND on channel $HOTSPOT_CHANNEL..."

		/usr/sbin/dnsmasq --conf-file=/etc/dnsmasqWifi.conf
	    	nice -n 5 hostapd -B -d /tmp/apconfig.txt
		
			if [ ${HOTSPOT_TXPOWER} != "" ]; then
				iw dev wifihotspot0 set txpower fixed ${HOTSPOT_TXPOWER}
			else
				iw dev wifihotspot0 set txpower fixed 100
			fi

	  	else
	     	echo "NO HOTSPOT CAPABILTY WAS FOUND"
		tmessage "no hotspot hardware found..."
	  	fi 
		
		if [ "$HOTSPOT_TIMEOUT" != "0" ]; then
			nice /home/pi/wifibroadcast-status/wbc_status "Hotspot Shutting Down in $HOTSPOT_TIMEOUT seconds" 7 55 0
    			sleep $HOTSPOT_TIMEOUT
			nice /home/pi/wifibroadcast-status/wbc_status "Hotspot Shutting Down in 10s" 7 55 0
    			sleep 10
   			killall hostapd
			ps -ef | nice grep "wifihotspot" | nice grep -v grep | awk '{print $2}' | xargs kill -9
			nice /home/pi/wifibroadcast-status/wbc_status "Hotspot Shut Down" 7 55 0
		fi
	fi



}
