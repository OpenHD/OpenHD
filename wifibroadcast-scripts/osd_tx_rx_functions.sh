source osd_rx_functions.sh
source osd_tx_functions.sh

function MAIN_OSD_TX_RX_FUNCTION {
    echo "================== OSD (tty2) ==========================="
    
    if [ "${CAM}" -ge 1 ]; then
        #
        # Run osdtx if pi camera found and telemetry transmission is enabled through Open.HD itself
        #
        if [ "$TELEMETRY_TRANSMISSION" == "wbc" ]; then
            osdtx_function
        fi
    else
        #
        # Run osdrx if no cam found
        #
        osdrx_function
    fi
    
    echo "OSD not enabled in config file"

    sleep 365d
}
