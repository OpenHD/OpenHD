function MAIN_ALIVE_FUNCTION {
	echo "================== CHECK ALIVE (tty9) ==========================="

	if [ "$CAM" == "0" ]; then
	    echo "Waiting some time until everything else is running ..."
	    sleep 15
		
	    check_alive_function
		
	    echo
	else
	    echo "Cam found, we are TX, check alive function disabled"
	    sleep 365d
	fi
}

function check_alive_function {
    # function to check if packets coming in, if not, re-start hello_video to clear frozen display
    while true; do
		# pause while saving is in progress
		pause_while
		
		ALIVE=`nice /home/pi/wifibroadcast-base/check_alive`
		if [ $ALIVE == "0" ]; then
			echo "no new packets, restarting hello_video and sleeping for 5s ..."
			ps -ef | nice grep "cat /root/videofifo1" | nice grep -v grep | awk '{print $2}' | xargs kill -9
			ps -ef | nice grep "$DISPLAY_PROGRAM" | nice grep -v grep | awk '{print $2}' | xargs kill -9
			ionice -c 1 -n 4 nice -n -10 cat /root/videofifo1 | ionice -c 1 -n 4 nice -n -10 $DISPLAY_PROGRAM > /dev/null 2>&1 &
			sleep 5
		else
			echo "received packets, doing nothing ..."
		fi
    done
}