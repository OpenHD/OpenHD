
function MAIN_SCREENSHOT_FUNCTION {
    echo "================== SCREENSHOT (tty5) ==========================="
    echo
    
    #
    # Only run screenshot function if cam found and screenshots are enabled
    #
    if [ "$CAM" == "0" ] && [ "$ENABLE_SCREENSHOTS" == "Y" ]; then
        echo "Waiting some time until everything else is running ..."
        
        sleep 20
        
        echo "Screenshots enabled - starting screenshot function ..."
        
        screenshot_function
    fi
    
    echo "Screenshots not enabled in configfile or we are TX"
    
    sleep 365d
}


function screenshot_function {
    while true; do
        #
        # Pause loop while saving is in progress
        #
        pause_while
        

        SCALIVE=`nice /usr/local/bin/check_alive`

        #
        # 3 Megabytes free space limit, the code below will stop saving screenshots if free space falls below
        # this level
        #
        LIMITFREE=3000
        
        #
        # Do nothing if no video being received (so we don't take unnecessary screeshots)
        #
        if [ "$SCALIVE" == "1" ]; then
            #
            # Check if tmp disk is full, if yes, do not save screenshot
            #
            FREETMPSPACE=`df -P /wbc_tmp/ | awk 'NR==2 {print $4}'`
            
            if [ $FREETMPSPACE -gt $LIMITFREE ]; then
                PNG_NAME=/wbc_tmp/screenshot`ls /wbc_tmp/screenshot* | wc -l`.png
            
                echo "Taking screenshot: $PNG_NAME"

                #
                # The raspi2png process takes advantage of the ability to read out the entire video display
                # from hardware *after* all the layers have been scaled and composited, it works well but can
                # only be done every few seconds since it is not efficient enough to run continuously
                #
                ionice -c 3 nice -n 19 /usr/bin/raspi2png -p $PNG_NAME
            else
                echo "RAM disk full - no screenshot taken ..."
            fi
        else
            echo "Video not running - no screenshot taken ..."
        fi
        
        sleep 5
    done
}
