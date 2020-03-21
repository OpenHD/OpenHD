source osd_rx_functions.sh
source osd_tx_functions.sh

function MAIN_OSD_TX_RX_FUNCTION {
    echo "================== OSD (tty2) ==========================="
    
    if [ "${CAM}" -ge 1 ]; then
        # only run osdtx if cam found, osd enabled and telemetry input is the tx
        if [ "$TELEMETRY_TRANSMISSION" == "wbc" ]; then
            osdtx_function
        fi
    else
        # only run osdrx if no cam found
        osdrx_function
    fi
    
    echo "OSD not enabled in configfile"
    sleep 365d
}
