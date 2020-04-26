function MAIN_RC_TX_RX_FUNCTION {
    echo "================== R/C TX (tty3) ==========================="
    
    #
    # Only run rctx if no cam found and rc is not disabled
    #
    if [ "$CAM" == "0" ] && [ "$RC" != "disabled" ]; then
        echo "Running on ground pi, RC enabled"
        rctx_function
    fi
    
    echo "RC not enabled in config file, or running on air pi"
    
    sleep 365d
}

# runs on RX (ground pi)
function rctx_function {    
    echo "Building rctx"
    
    cd /home/pi/wifibroadcast-rc-Ath9k/
    
    ./build.sh

    cp rctx /tmp/
    
    #
    # Wait until video is running to make sure NICS are configured and wifibroadcast_rx_status shared memory is available
    #

    echo
    echo -n "Waiting until nics are configured ..."
    while [ ! -f /tmp/nics_configured ]; do
        sleep 0.5
        echo -n "."
    done
    
    sleep 0.5
        
    pause_while
    
    echo
    echo "Starting RC TX..."

    while true; do
        nice -n -5 /tmp/rctx
        sleep 1
    done
}
