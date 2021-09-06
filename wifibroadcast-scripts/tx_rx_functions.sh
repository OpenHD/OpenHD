source rx_functions.sh
source tx_functions.sh
source camera_setup.sh

function MAIN_TX_RX_FUNCTION {
    ###############################################################################
    # Create /wbc_tmp for telemetry storage
    ###############################################################################
    mkdir -p /wbc_tmp
    

    mount -t tmpfs -o size=15024K tmpfs /wbc_tmp
    mkdir -p /wbc_tmp/rssi/

    if [ "$CAM" == "0" ]; then
        rx_function
    else
        camera_setup
        tx_function
    fi
}
