source osd_rx_functions.sh
source osd_tx_functions.sh

function MAIN_OSD_TX_RX_FUNCTION {
	echo "================== OSD (tty2) ==========================="
	
	# only run osdrx if no cam found
	if [ "$CAM" == "0" ]; then
	    osdrx_function
	else
	    # only run osdtx if cam found, osd enabled and telemetry input is the tx
	    if [ "$CAM" == "1" ] && [ "$TELEMETRY_TRANSMISSION" == "wbc" ]; then
	        osdtx_function
	    fi
	fi
	
    echo "OSD not enabled in configfile"
	sleep 365d
}