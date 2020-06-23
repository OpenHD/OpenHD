source rx_functions.sh
source tx_functions.sh

function MAIN_TX_RX_FUNCTION {
    ###############################################################################
    # Create /wbc_tmp dynamically based on available ram
    ###############################################################################

    detect_memory

    mkdir -p /wbc_tmp
    
    # use 1/3 of available ram by default for /wbc_tmp, which is used for recording telemetry and video
    # when VIDEO_TMP=memory. We need to do this to avoid crashes or safety issues caused by running out of
    # memory, which is easy to do when the ground station has just 512MB ram to start with and has 128MB set
    # aside for the GPU, like the Pi3a+
    available_for_wbc_tmp=$((${TOTAL_MEMORY} / 3))

    # add a little extra margin 
    available_for_wbc_tmp_final=$((${available_for_wbc_tmp} + 30000))

    mount -t tmpfs -o size=${available_for_wbc_tmp_final}K tmpfs /wbc_tmp


    if [ "$CAM" == "0" ]; then
        rx_function
    else
        tx_function
    fi
}
