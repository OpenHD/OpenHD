function MAIN_ALIVE_FUNCTION {
    echo "================== CHECK ALIVE (tty9) ==========================="

    if [ "${CAM}" == "0" ]; then

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
    #
    # Checks if packets are coming in
    # 
    # If not, re-start hello_video to clear frozen display
    #
    while true; do
        #
        # Wait if save to USB is in progress
        #
        pause_while
        
        ALIVE=`nice /usr/local/bin/check_alive`

        if [ ${ALIVE} == "0" ]; then


            echo "No new packets received, restarting hello_video and sleeping for 5s ..."
                
            ps -ef | nice grep "cat /var/run/openhd/videofifo1" | nice grep -v grep | awk '{print $2}' | xargs kill -9
            ps -ef | nice grep "${DISPLAY_PROGRAM}" | nice grep -v grep | awk '{print $2}' | xargs kill -9
                
            ionice -c 1 -n 4 nice -n -10 cat /var/run/openhd/videofifo1 | ionice -c 1 -n 4 nice -n -10 ${DISPLAY_PROGRAM} ${HELLO_VIDEO_ARGS} > /dev/null 2>&1 &


            sleep 5
        fi
    done
}
