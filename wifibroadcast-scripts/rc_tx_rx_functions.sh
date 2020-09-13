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
    echo "Starting $RC RC transmission"
    qstatus "Starting $RC RC transmission" 5

    while true; do
        nice -n -5 /usr/local/bin/rctx
        qstatus "Restarting ${RC} RC transmission!" 3
        wbc_status "Restarting ${RC} RC transmission!" 5 55 0 &
        sleep 1
    done
}
